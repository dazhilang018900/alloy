/*
 * EndlessGrid.cpp
 *
 *  Created on: Nov 21, 2017
 *      Author: blake
 */

#include "grid/EndlessGrid.h"
#include "AlloyVolume.h"
#include <queue>
namespace aly {
void WriteGridToFile(const std::string& dir, const EndlessGrid<float>& grid) {
	std::vector<std::pair<int3, EndlessNodeFloatPtr>> nodes = grid.getNodes();
	int dim = grid.getNodeSize();
	Volume1f data(dim, dim, dim);
	Volume1ub depth(dim, dim, dim);
	for (auto pr : nodes) {
		int3 pos = pr.first;
		EndlessNodeFloatPtr node = pr.second;
		std::string file = MakeString() << dir << ALY_PATH_SEPARATOR<<"grid_"<<pos.x<<"_"<<pos.y<<"_"<<pos.z<<".xml";
		data.set(grid.getBackgroundValue());
		for (int z = 0; z < dim; z++) {
			for (int y = 0; y < dim; y++) {
				for (int x = 0; x < dim; x++) {
					data(x, y, z).x = grid.getMultiResolutionValue(node, x, y,
							z);
				}
			}
		}
		std::cout << "Write " << file << std::endl;
		WriteVolumeToFile(file, data);
		/*
		 file = MakeString() << dir << ALY_PATH_SEPARATOR<<"tree_"<<pos.x<<"_"<<pos.y<<"_"<<pos.z<<".xml";
		 depth.set(grid.getBackgroundValue());
		 for (int z = 0; z < dim; z++) {
		 for (int y = 0; y < dim; y++) {
		 for (int x = 0; x < dim; x++) {
		 depth(x, y, z).x = grid.getTreeDepth(node, x, y, z);
		 }
		 }
		 }
		 std::cout<<"Write "<<file<<std::endl;
		 WriteVolumeToFile(file, depth);
		 for (int d = 0; d < grid.getTreeDepth(); d++) {
		 file = MakeString() << dir << ALY_PATH_SEPARATOR<<"tree_"<<pos.x<<"_"<<pos.y<<"_"<<pos.z<<"-"<<d<<".xml";
		 data.set(grid.getBackgroundValue());
		 for (int z = 0; z < dim; z++) {
		 for (int y = 0; y < dim; y++) {
		 for (int x = 0; x < dim; x++) {
		 data(x, y, z).x = grid.getDepthValue(node, x, y, z, d);
		 }
		 }
		 }
		 std::cout<<"Write "<<file<<std::endl;
		 WriteVolumeToFile(file, data);
		 }
		 */
	}

}
void MeshToLevelSet(const Mesh& mesh, EndlessGrid<float>& grid) {

	const int nbr6X[6] = { 1, -1, 0, 0, 0, 0 };
	const int nbr6Y[6] = { 0, 0, 1, -1, 0, 0 };
	const int nbr6Z[6] = { 0, 0, 0, 0, 1, -1 };

	const int nbr26X[26] = { 1, 0, -1, 1, 0, -1, 1, 0, -1, 1, 0, -1, 1, -1, 1,
			0, -1, 1, 0, -1, 1, 0, -1, 1, 0, -1 };
	const int nbr26Y[26] = { 1, 1, 1, 0, 0, 0, -1, -1, -1, 1, 1, 1, 0, 0, -1,
			-1, -1, 1, 1, 1, 0, 0, 0, -1, -1, -1 };
	const int nbr26Z[26] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
			-1, -1, -1, -1, -1, -1, -1, -1, -1 };

	grid.clear();
	float narrowBand = 1.5f;
	float backgroundValue = 100.0f;
	grid.setBackgroundValue(backgroundValue);
	box3f bbox = mesh.getBoundingBox();
	std::vector<box3f> splats;
	splats.reserve(mesh.triIndexes.size());
	double averageSize = 0;
	for (uint3 tri : mesh.triIndexes.data) {
		float3 v1 = mesh.vertexLocations[tri.x];
		float3 v2 = mesh.vertexLocations[tri.y];
		float3 v3 = mesh.vertexLocations[tri.z];
		averageSize += 0.5f * std::abs(crossMag(v2 - v1, v3 - v1));
		float3 minPt = aly::min(aly::min(v1, v2), v3);
		float3 maxPt = aly::max(aly::max(v1, v2), v3);
		splats.push_back(box3f(minPt, maxPt - minPt));
	}
	averageSize /= mesh.triIndexes.size();
	float res = 0.75f * std::sqrt(averageSize);
	int3 minIndex = int3(bbox.position / res - narrowBand * 8.0f);
	for (int n = 0; n < (int) splats.size(); n++) {
		box3f box = splats[n];
		int3 dims = int3(aly::round(box.dimensions / res) + 4.0f);
		int3 pos = int3(aly::floor(box.position / res) - 2.0f);
		uint3 tri = mesh.triIndexes[n];
		float3 v1 = mesh.vertexLocations[tri.x];
		float3 v2 = mesh.vertexLocations[tri.y];
		float3 v3 = mesh.vertexLocations[tri.z];
		float3 norm = normalize(cross(v2 - v1, v3 - v1));
		float3 closestPoint;
		for (int k = 0; k <= dims.z; k++) {
			for (int j = 0; j <= dims.y; j++) {
				for (int i = 0; i <= dims.x; i++) {
					float3 pt = float3(res * (i + pos.x), res * (j + pos.y),
							res * (k + pos.z));
					float d = std::sqrt(
							DistanceToTriangleSqr(pt, v1, v2, v3,
									&closestPoint)) / res;
					if (d <= narrowBand) {
						float& value = grid.getLeafValue(i + pos.x - minIndex.x,j + pos.y - minIndex.y, k + pos.z - minIndex.z);
						if (d < std::abs(value)) {
							value = aly::sign(dot(pt - closestPoint, norm)) * d;
						}
					}
				}
			}
		}
	}
	//Need to remove small connected components
	grid.allocateInternalNodes();
	std::set<EndlessNodeFloat*> parents;
	std::vector<EndlessNodeFloatPtr> leafs = grid.getLeafNodes();
	for (int i = 0; i < (int) leafs.size(); i++) {
		EndlessNodeFloatPtr leaf = leafs[i];
		int3 location = leaf->location;
		std::vector<float>& data = leaf->data;
		int dim = leaf->dim;
		int pdim = leaf->parent->dim;
		std::queue<int3> posQ, negQ;
		//(*leaf->parent)(location.x, location.y, location.z) = 0;
		for (int k = 0; k < dim; k++) {
			for (int j = 0; j < dim; j++) {
				for (int i = 0; i < dim; i++) {
					float& val = data[i + (j + k * dim) * dim];
					if (val == backgroundValue) {
						for (int n = 0; n < 6; n++) {
							int3 nbr = int3(i + nbr6X[n], j + nbr6Y[n],
									k + nbr6Z[n]);
							if (nbr.x >= 0 && nbr.x < dim && nbr.y >= 0
									&& nbr.y < dim && nbr.z >= 0
									&& nbr.z < dim) {
								float cval = data[nbr.x
										+ (nbr.y + nbr.z * dim) * dim];
								if (cval != backgroundValue) {
									val = sign(cval) * narrowBand;
									if (cval < 0.0f) {
										negQ.push(int3(i, j, k));
									} else if (cval > 0.0f) {
										posQ.push(int3(i, j, k));
									}
									break;
								}
							}
						}
					}
				}
			}
		}
		while (!negQ.empty()) {
			int3 pos = negQ.front();
			negQ.pop();
			for (int n = 0; n < 6; n++) {
				int3 nbr = pos + int3(nbr6X[n], nbr6Y[n], nbr6Z[n]);
				if (nbr.x >= 0 && nbr.x < dim && nbr.y >= 0 && nbr.y < dim
						&& nbr.z >= 0 && nbr.z < dim) {
					float& val = data[nbr.x + (nbr.y + nbr.z * dim) * dim];
					if (val == backgroundValue) {
						val = -narrowBand;
						negQ.push(nbr);
					}
				}
			}
		}
		while (!posQ.empty()) {
			int3 pos = posQ.front();
			posQ.pop();
			for (int n = 0; n < 6; n++) {
				int3 nbr = pos + int3(nbr6X[n], nbr6Y[n], nbr6Z[n]);
				if (nbr.x >= 0 && nbr.x < dim && nbr.y >= 0 && nbr.y < dim
						&& nbr.z >= 0 && nbr.z < dim) {
					float& val = data[nbr.x + (nbr.y + nbr.z * dim) * dim];
					if (val == backgroundValue) {
						val = narrowBand;
						posQ.push(nbr);
					}
				}
			}
		}
		parents.insert(leaf->parent);
	}
	while (parents.size() > 0) {
		std::set<EndlessNodeFloat*> newParents;
		for (EndlessNodeFloat* node : parents) {
			std::vector<float>& data = node->data;
			std::vector<int>& indexes = node->indexes;
			int dim = node->dim;
			std::queue<int3> posQ, negQ;
			for (int k = 0; k < dim; k++) {
				for (int j = 0; j < dim; j++) {
					for (int i = 0; i < dim; i++) {
						int idx = indexes[i + (j + k * dim) * dim];
						if (idx >= 0) {
							//data[i + (j + k * dim) * dim] = 0;
							auto child = node->children[idx];
							for (int n = 0; n < 6; n++) {
								int3 nbr = int3(i + nbr6X[n], j + nbr6Y[n],
										k + nbr6Z[n]);
								if (nbr.x >= 0 && nbr.x < dim && nbr.y >= 0
										&& nbr.y < dim && nbr.z >= 0
										&& nbr.z < dim) {
									float& nval = data[nbr.x
											+ (nbr.y + nbr.z * dim) * dim];
									if (nval == backgroundValue) {
										float val = child->getSideValue(n);
										nval = sign(val) * narrowBand;
										if (nval < 0.0f) {
											negQ.push(nbr);
										} else if (nval > 0.0f) {
											posQ.push(nbr);
										}
									}
								} else {
									if (node->parent != nullptr
											&& node->parent->parent
													!= nullptr) {
										int ldim = child->dim;
										nbr = child->location + ldim / 2
												+ nbr * ldim;
										EndlessNodeFloatPtr result;
										float& nval =
												grid.getMultiResolutionValue(
														nbr.x, nbr.y, nbr.z,
														result);
										if (nval == backgroundValue) {
											float val = child->getSideValue(n);
											if (val != 0) {
												nval = sign(val) * narrowBand;
												if (result.get() != nullptr)
													newParents.insert(
															result.get());
											}
										}
									}
								}
							}
						}
					}
				}
			}
			while (!negQ.empty()) {
				int3 pos = negQ.front();
				negQ.pop();
				for (int n = 0; n < 6; n++) {
					int3 nbr = pos + int3(nbr6X[n], nbr6Y[n], nbr6Z[n]);
					if (nbr.x >= 0 && nbr.x < dim && nbr.y >= 0 && nbr.y < dim
							&& nbr.z >= 0 && nbr.z < dim) {
						float& val = data[nbr.x + (nbr.y + nbr.z * dim) * dim];
						if (val == backgroundValue) {
							val = -narrowBand;
							negQ.push(nbr);
						}
					}
				}
			}
			while (!posQ.empty()) {
				int3 pos = posQ.front();
				posQ.pop();
				for (int n = 0; n < 6; n++) {
					int3 nbr = pos + int3(nbr6X[n], nbr6Y[n], nbr6Z[n]);
					if (nbr.x >= 0 && nbr.x < dim && nbr.y >= 0 && nbr.y < dim
							&& nbr.z >= 0 && nbr.z < dim) {
						float& val = data[nbr.x + (nbr.y + nbr.z * dim) * dim];
						if (val == backgroundValue) {
							val = narrowBand;
							posQ.push(nbr);
						}
					}
				}
			}
			if (node->parent != nullptr)
				newParents.insert(node->parent);
		}
		parents = newParents;
	}
	std::cout << "Done Grid Construction" << std::endl;
}
}
