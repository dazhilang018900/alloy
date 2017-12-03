/*
 * Copyright(C) 2017, Blake C. Lucas, Ph.D. (img.science@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "grid/EndlessGrid.h"
#include "AlloyVolume.h"

#include "AlloyIsoSurface.h"
#include <queue>
namespace aly {
void WriteGridToFile(const std::string& root, const EndlessGrid<float>& grid) {
	std::vector<std::pair<int3, EndlessNodeFloatPtr>> nodes = grid.getNodes();
	int dim = grid.getNodeSize();
	Volume1f data(dim, dim, dim);
	Volume1ub depth(dim, dim, dim);
	std::string name = GetFileNameWithoutExtension(root);
	std::string dir = GetFileDirectoryPath(root);
	for (auto pr : nodes) {
		int3 pos = pr.first;
		//if (pos != int3(0, 0, 0))continue;
		EndlessNodeFloatPtr node = pr.second;
		std::string volFile = MakeString() << dir << ALY_PATH_SEPARATOR<<name<<"_"<<pos.x<<"_"<<pos.y<<"_"<<pos.z<<".xml";
		std::string meshFile = MakeString() << dir << ALY_PATH_SEPARATOR<<name<<"_"<<pos.x<<"_"<<pos.y<<"_"<<pos.z<<".ply";

		data.set(grid.getBackgroundValue());
		std::vector<int3> narrowBandList;
		for (int z = 0; z < dim; z++) {
			for (int y = 0; y < dim; y++) {
				for (int x = 0; x < dim; x++) {
					float val = grid.getMultiResolutionValue(node, x, y, z);
					data(x, y, z).x = val;
					if(std::abs(val)<1.75){
						narrowBandList.push_back(int3(x,y,z));
					}
				}
			}
		}
		IsoSurface isosurf;
		Mesh mesh;
		isosurf.solve(data,mesh,narrowBandList);

		std::cout << "Write " << volFile << std::endl;
		WriteVolumeToFile(volFile, data);

		std::cout << "Write " << meshFile << std::endl;
		WriteMeshToFile(meshFile, mesh);

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
		 std::cout << "Write " << file << std::endl;
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

void WriteGridToFile(const std::string& root, const EndlessGrid<int>& grid) {
	std::vector<std::pair<int3, EndlessNodeIntPtr>> nodes = grid.getNodes();
	int dim = grid.getNodeSize();
	Volume1i data(dim, dim, dim);
	std::string name = GetFileNameWithoutExtension(root);
	std::string dir = GetFileDirectoryPath(root);
	for (auto pr : nodes) {
		int3 pos = pr.first;
		EndlessNodeIntPtr node = pr.second;
		std::string file = MakeString() << dir << ALY_PATH_SEPARATOR<<name<<"_"<<pos.x<<"_"<<pos.y<<"_"<<pos.z<<".xml";
		data.set(grid.getBackgroundValue());
		for (int z = 0; z < dim; z++) {
			for (int y = 0; y < dim; y++) {
				for (int x = 0; x < dim; x++) {
					data(x, y, z).x = grid.getMultiResolutionValue(node, x, y,
							z);
				}
			}
		}
		WriteVolumeToFile(file, data);

	}
}
void FloodFill(EndlessGrid<float>& grid, float narrowBand) {
	const int nbr6X[6] = { 1, -1, 0, 0, 0, 0 };
	const int nbr6Y[6] = { 0, 0, 1, -1, 0, 0 };
	const int nbr6Z[6] = { 0, 0, 0, 0, 1, -1 };
	const int nbr26X[26] = { 1, 0, -1, 1, 0, -1, 1, 0, -1, 1, 0, -1, 1, -1, 1,
			0, -1, 1, 0, -1, 1, 0, -1, 1, 0, -1 };
	const int nbr26Y[26] = { 1, 1, 1, 0, 0, 0, -1, -1, -1, 1, 1, 1, 0, 0, -1,
			-1, -1, 1, 1, 1, 0, 0, 0, -1, -1, -1 };
	const int nbr26Z[26] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
			-1, -1, -1, -1, -1, -1, -1, -1, -1 };
	float backgroundValue = grid.getBackgroundValue();
	std::set<EndlessNodeFloat*> parents;
	std::vector<EndlessNodeFloatPtr> leafs = grid.getLeafNodes();
	for (int i = 0; i < (int) leafs.size(); i++) {
		EndlessNodeFloatPtr leaf = leafs[i];
		leaf->parent->data[leaf->parent->getIndex(leaf.get())] = 0;
		int3 location = leaf->location;
		std::vector<float>& data = leaf->data;
		int dim = leaf->dim;
		std::queue<int3> posQ, negQ;
		std::set<int3> posO, negO;
		for (int k = 0; k < dim; k++) {
			for (int j = 0; j < dim; j++) {
				for (int i = 0; i < dim; i++) {
					float& val = data[i + (j + k * dim) * dim];
					if (val != backgroundValue) {
						for (int n = 0; n < 26; n++) {
							int3 nbr = int3(i + nbr26X[n], j + nbr26Y[n],
									k + nbr26Z[n]);
							if (nbr.x >= 0 && nbr.x < dim && nbr.y >= 0
									&& nbr.y < dim && nbr.z >= 0
									&& nbr.z < dim) {
								float& nval = data[nbr.x
										+ (nbr.y + nbr.z * dim) * dim];
								if (nval == backgroundValue) {
									nval = sign(val) * narrowBand;
									if (nval < 0.0f) {
										negQ.push(nbr);
									} else if (nval > 0.0f) {
										posQ.push(nbr);
									}
								}
							} else {
								nbr = location + nbr;
								if (val > 0) {
									posO.insert(nbr);
								} else if (val < 0) {
									negO.insert(nbr);
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
			float maxVal = narrowBand * 2;
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

		for (int3 nbr : posO) {
			EndlessNodeFloatPtr result;
			float* nval = nullptr;
			grid.getMultiResolutionValue(nbr.x, nbr.y, nbr.z, result, nval);
			if (nval != nullptr && *nval == backgroundValue) {
				*nval = narrowBand;
			}
		}
		for (int3 nbr : negO) {
			EndlessNodeFloatPtr result;
			float* nval = nullptr;
			grid.getMultiResolutionValue(nbr.x, nbr.y, nbr.z, result, nval);
			if (nval != nullptr && *nval == backgroundValue) {
				*nval = -narrowBand;
			}
		}

		parents.insert(leaf->parent);
	}

	int treeDepth = grid.getTreeDepth() - 2;
	while (parents.size() > 0) {
		std::set<EndlessNodeFloat*> newParents;
		for (EndlessNodeFloat* node : parents) {
			if (treeDepth < 0) {
				parents.clear();
				break;
			}
			if (node->parent != nullptr)
				node->parent->data[node->parent->getIndex(node)] = 0;
			int3 location = node->location;
			std::vector<float>& data = node->data;
			int dim = node->dim;
			int c = grid.getCellSizes()[treeDepth];
			std::queue<int3> posQ, negQ;
			std::set<int3> posO, negO;
			for (int k = 0; k < dim; k++) {
				for (int j = 0; j < dim; j++) {
					for (int i = 0; i < dim; i++) {
						float& val = data[i + (j + k * dim) * dim];
						if (val != backgroundValue) {
							for (int n = 0; n < 26; n++) {
								int3 nbr = int3(i + nbr26X[n], j + nbr26Y[n],
										k + nbr26Z[n]);
								if (nbr.x >= 0 && nbr.x < dim && nbr.y >= 0
										&& nbr.y < dim && nbr.z >= 0
										&& nbr.z < dim) {
									float& nval = data[nbr.x
											+ (nbr.y + nbr.z * dim) * dim];
									if (nval == backgroundValue) {
										nval = sign(val) * narrowBand;
										if (nval < 0.0f) {
											negQ.push(nbr);
										} else if (nval > 0.0f) {
											posQ.push(nbr);
										}
									}
								} else {
									nbr = location + c * nbr;
									if (val > 0) {
										posO.insert(nbr);
									} else if (val < 0) {
										negO.insert(nbr);
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
			for (int3 nbr : posO) {
				EndlessNodeFloatPtr result;
				float* nval = nullptr;
				grid.getMultiResolutionValue(nbr.x, nbr.y, nbr.z, result, nval);
				if (nval != nullptr && *nval == backgroundValue) {
					*nval = narrowBand;
				}
			}
			for (int3 nbr : negO) {
				EndlessNodeFloatPtr result;
				float* nval = nullptr;
				grid.getMultiResolutionValue(nbr.x, nbr.y, nbr.z, result, nval);
				if (nval != nullptr && *nval == backgroundValue) {
					*nval = -narrowBand;
				}
			}
			if (node->parent != nullptr)
				newParents.insert(node->parent);
		}
		treeDepth--;
		parents = newParents;
	}

}
float MeshToLevelSet(const Mesh& mesh, EndlessGrid<float>& grid,
		float narrowBand, bool flipSign, float voxelScale) {
	const int nbr6X[6] = { 1, -1, 0, 0, 0, 0 };
	const int nbr6Y[6] = { 0, 0, 1, -1, 0, 0 };
	const int nbr6Z[6] = { 0, 0, 0, 0, 1, -1 };
	const int nbr26X[26] = { 1, 0, -1, 1, 0, -1, 1, 0, -1, 1, 0, -1, 1, -1, 1,
			0, -1, 1, 0, -1, 1, 0, -1, 1, 0, -1 };
	const int nbr26Y[26] = { 1, 1, 1, 0, 0, 0, -1, -1, -1, 1, 1, 1, 0, 0, -1,
			-1, -1, 1, 1, 1, 0, 0, 0, -1, -1, -1 };
	const int nbr26Z[26] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
			-1, -1, -1, -1, -1, -1, -1, -1, -1 };
	std::set<EndlessNodeFloat*> parents;
	const float trustDistance = 1.25f;
	narrowBand = std::max(trustDistance + 1.0f, narrowBand);
	float sgn = ((flipSign) ? -1.0f : 1.0f);
	grid.clear();
	float backgroundValue = (narrowBand + 0.5f) * sgn;
	grid.setBackgroundValue(backgroundValue);
	box3f bbox = mesh.getBoundingBox();
	std::vector<box3f> splats;
	splats.reserve(mesh.triIndexes.size());
	double averageSize = 0;
	//Calculate average size of triangle to scale mesh to grid accordingly.
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
	float res = voxelScale * std::sqrt(averageSize);
	int3 minIndex = int3(bbox.position / res - narrowBand * 3.0f);
	//Splat regions around triangles
	for (int n = 0; n < (int) splats.size(); n++) {
		box3f box = splats[n];
		int3 dims = int3(
				aly::round(box.dimensions / res) + 0.5f + 2 * narrowBand);
		int3 pos = int3(aly::floor(box.position / res) - 0.25f - narrowBand);
		uint3 tri = mesh.triIndexes[n];
		float3 v1 = mesh.vertexLocations[tri.x];
		float3 v2 = mesh.vertexLocations[tri.y];
		float3 v3 = mesh.vertexLocations[tri.z];
		float3 n1 = mesh.vertexNormals[tri.x];
		float3 n2 = mesh.vertexNormals[tri.y];
		float3 n3 = mesh.vertexNormals[tri.z];
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
						int3 loc = int3(i + pos.x - minIndex.x,
								j + pos.y - minIndex.y, k + pos.z - minIndex.z);
						float& value = grid.getLeafValue(loc.x, loc.y, loc.z);
						if (d < std::abs(value)) {
							if (d <= trustDistance) {
								//Interpolate normal to compute sign. Addresses cusp issue.
								value =
										d * sgn
												* (int) aly::sign(
														dot(pt - closestPoint,
																FromBary(
																		ToBary(
																				closestPoint,
																				v1,
																				v2,
																				v3),
																		n1, n2,
																		n3)));
							} else {
								value = d * sgn;
							}
						}
					}
				}
			}
		}
	}
	splats.clear();
	std::vector<EndlessNodeFloatPtr> leafs = grid.getLeafNodes();
	grid.allocateInternalNodes();
	//Flood fill leaf nodes with narrowband value.
	for (int i = 0; i < (int) leafs.size(); i++) {
		EndlessNodeFloatPtr leaf = leafs[i];
		leaf->parent->data[leaf->parent->getIndex(leaf.get())] = 0;
		int3 location = leaf->location;
		std::vector<float>& data = leaf->data;
		int dim = leaf->dim;
		std::queue<int3> posQ;
		for (int k = 0; k < dim; k++) {
			for (int j = 0; j < dim; j++) {
				for (int i = 0; i < dim; i++) {
					float& val = data[i + (j + k * dim) * dim];
					if (val != backgroundValue) {
						for (int n = 0; n < 26; n++) {
							int3 nbr = int3(i + nbr26X[n], j + nbr26Y[n],
									k + nbr26Z[n]);
							if (nbr.x >= 0 && nbr.x < dim && nbr.y >= 0
									&& nbr.y < dim && nbr.z >= 0
									&& nbr.z < dim) {
								float& nval = data[nbr.x
										+ (nbr.y + nbr.z * dim) * dim];
								if (nval == backgroundValue) {
									nval = narrowBand * sgn;
									posQ.push(nbr);
								}
							}
						}
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
						val = narrowBand * sgn;
						posQ.push(nbr);
					}
				}
			}
		}

	}
	//Find extremety leaf nodes.
	EndlessNodeFloatPtr minX, minY, minZ, maxX, maxZ, maxY;
	std::queue<std::pair<int3, float>> queue;
	int3 maxPt = int3(-100000000, -100000000, -100000000);
	int3 minPt = int3(100000000, 100000000, 100000000);
	for (EndlessNodeFloatPtr leaf : leafs) {
		if (leaf->location.x > maxPt.x) {
			maxX = leaf;
			maxPt.x = leaf->location.x;

		}
		if (leaf->location.y > maxPt.y) {
			maxY = leaf;
			maxPt.y = leaf->location.y;

		}
		if (leaf->location.z > maxPt.z) {
			maxZ = leaf;
			maxPt.z = leaf->location.z;
		}
		if (leaf->location.x < minPt.x) {
			minX = leaf;
			minPt.x = leaf->location.x;

		}
		if (leaf->location.y < minPt.y) {
			minY = leaf;
			minPt.y = leaf->location.y;

		}
		if (leaf->location.z < minPt.z) {
			minZ = leaf;
			minPt.z = leaf->location.z;
		}
	}
	{
		int dim = maxX->dim;
		int3 loc = maxX->location;
		for (int y = 0; y < dim; y++) {
			for (int z = 0; z < dim; z++) {
				int3 pos = int3(loc.x + dim - 1, loc.y + y, loc.z + z);
				float& value = (*maxX)(dim - 1, y, z);
				queue.push( { pos, value });
				value = -value;
			}
		}
	}
	{
		int dim = maxY->dim;
		int3 loc = maxY->location;
		for (int x = 0; x < dim; x++) {
			for (int z = 0; z < dim; z++) {
				int3 pos = int3(loc.x + x, loc.y + dim - 1, loc.z + z);
				float& value = (*maxY)(x, dim - 1, z);
				queue.push( { pos, value });
				value = -value;
			}
		}
	}
	{
		int dim = maxZ->dim;
		int3 loc = maxZ->location;
		for (int x = 0; x < dim; x++) {
			for (int y = 0; y < dim; y++) {
				int3 pos = int3(loc.x, loc.y + y, loc.z + dim - 1);
				float& value = (*maxZ)(x, y, dim - 1);
				queue.push( { pos, value });
				value = -value;
			}
		}
	}

	{
		int dim = minX->dim;
		int3 loc = minX->location;
		for (int y = 0; y < dim; y++) {
			for (int z = 0; z < dim; z++) {
				int3 pos = int3(loc.x, loc.y + y, loc.z + z);
				float& value = (*minX)(0, y, z);
				queue.push( { pos, value });
				value = -value;
			}
		}
	}
	{
		int dim = minY->dim;
		int3 loc = minY->location;
		for (int x = 0; x < dim; x++) {
			for (int z = 0; z < dim; z++) {
				int3 pos = int3(loc.x + x, loc.y, loc.z + z);
				float& value = (*minY)(x, 0, z);
				queue.push( { pos, value });
				value = -value;
			}
		}
	}
	{
		int dim = minZ->dim;
		int3 loc = minZ->location;
		for (int x = 0; x < dim; x++) {
			for (int y = 0; y < dim; y++) {
				int3 pos = int3(loc.x, loc.y + y, loc.z);
				float& value = (*minZ)(x, y, 0);
				queue.push( { pos, value });
				value = -value;
			}
		}
	}
	EndlessNodeFloatPtr result;
	float* nval;
	//Flood extremity leaf nodes with sign value.
	while (queue.size() > 0) {
		std::pair<int3, float> pr = queue.front();
		float value = pr.second;
		queue.pop();
		for (int n = 0; n < 6; n++) {
			int3 nbr = pr.first + int3(nbr6X[n], nbr6Y[n], nbr6Z[n]);
			grid.getMultiResolutionValue(nbr.x, nbr.y, nbr.z, result, nval);
			if (nval != nullptr && std::abs(*nval) > trustDistance
					&& sgn * (*nval) > 0) {
				if (result->isLeaf()) {
					queue.push( { nbr, *nval });
					*nval = -*nval;
				}
			}
		}
	}
	//Flood fill rest of space using signed level set values, propagating the proper sign in the process.
	FloodFill(grid, narrowBand);
	return res;
}

}
