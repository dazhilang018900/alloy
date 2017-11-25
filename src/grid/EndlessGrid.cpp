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
	for (auto pr : nodes) {
		int3 pos = pr.first;
		EndlessNodeFloatPtr node = pr.second;
		std::string file = MakeString() << dir << ALY_PATH_SEPARATOR<<"grid_"<<pos.x<<"_"<<pos.y<<"_"<<pos.z<<".xml";
		data.set(grid.getBackgroundValue());
		std::vector<int3> positions;
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
void MeshToLevelSet(const Mesh& mesh, EndlessGrid<float>& grid) {
	grid.clear();
	float narrowBand = 2.0f;
	float backgroundValue = 100.0f;
	grid.setBackgroundValue(backgroundValue);
	box3f bbox = mesh.getBoundingBox();
	std::vector<box3f> splats;
	splats.reserve(mesh.triIndexes.size());
	double averageSize = 0;
	std::cout << "Bounding Box: " << bbox << std::endl;
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
	std::cout << "Average Area " << averageSize << " " << std::sqrt(averageSize)
			<< std::endl;
	float res = 0.8f * std::sqrt(averageSize);
	int3 minIndex = int3(bbox.position / res - narrowBand * 8.0f);
	std::cout << "Splats " << splats.size() << std::endl;
	for (int n = 0; n < (int) splats.size(); n++) {
		box3f box = splats[n];
		int3 dims = int3(aly::ceil(box.dimensions / res) + 6.0f);
		int3 pos = int3(aly::floor(box.position / res) - 3.0f);
		EndlessLocation gpos;
		uint3 tri = mesh.triIndexes[n];
		float3 v1 = mesh.vertexLocations[tri.x];
		float3 v2 = mesh.vertexLocations[tri.y];
		float3 v3 = mesh.vertexLocations[tri.z];
		float3 norm = normalize(cross(v2 - v1, v3 - v1));
		float3 center = 0.33333f * (v1 + v2 + v3);
		float3 closestPoint;
		for (int k = 0; k <= dims.z; k++) {
			for (int j = 0; j <= dims.y; j++) {
				for (int i = 0; i <= dims.x; i++) {
					float& value = grid.getLeafValue(i + pos.x - minIndex.x,
							j + pos.y - minIndex.y, k + pos.z - minIndex.z);
					float3 pt = float3(res * (i + pos.x), res * (j + pos.y),
							res * (k + pos.z));
					float d = DistanceToTriangleSqr(pt, v1, v2, v3,
							&closestPoint) / res;
					if (d < std::abs(value)) {
						float sgn = aly::sign(dot(pt - center, norm));
						value = sgn * d;
					}
				}
			}
		}
	}
	grid.allocateInternalNodes();
	std::set<EndlessNodeFloat*> parents;
	const int nbrX[6] = { 1, -1, 0, 0, 0, 0 };
	const int nbrY[6] = { 0, 0, 1, -1, 0, 0 };
	const int nbrZ[6] = { 0, 0, 0, 0, 1, -1 };

	const int nbr26X[26] = { 1, 0, -1, 1, 0, -1, 1, 0, -1, 1, 0, -1, 1, -1, 1,
			0, -1, 1, 0, -1, 1, 0, -1, 1, 0, -1 };
	const int nbr26Y[26] = { 1, 1, 1, 0, 0, 0, -1, -1, -1, 1, 1, 1, 0, 0, -1,
			-1, -1, 1, 1, 1, 0, 0, 0, -1, -1, -1 };
	const int nbr26Z[26] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,-1, -1, -1, -1, -1, -1, -1, -1, -1 };
	std::vector<EndlessNodeFloatPtr> leafs = grid.getLeafNodes();
	for (int i = 0; i < (int) leafs.size(); i++) {
		EndlessNodeFloatPtr leaf = leafs[i];
		int3 location = leaf->location;
		std::vector<float>& data = leaf->data;
		int dim = leaf->dim;
		int pdim = leaf->parent->dim;
		std::queue<int3> posQ, negQ;
		(*leaf->parent)(location.x, location.y, location.z) = 0;
		for (int k = 0; k < dim; k++) {
			for (int j = 0; j < dim; j++) {
				for (int i = 0; i < dim; i++) {
					float& val = data[i + (j + k * dim) * dim];
					if (val != backgroundValue) {
						if (std::abs(val) > narrowBand) {
							val = backgroundValue;
						} else {
							if (val < 0) {
								negQ.push(int3(i, j, k));
							} else if (val > 0) {
								posQ.push(int3(i, j, k));
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
				int3 nbr = pos + int3(nbrX[n], nbrY[n], nbrZ[n]);
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
				int3 nbr = pos + int3(nbrX[n], nbrY[n], nbrZ[n]);
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
		//if(*(leaf)(i,j,k)=)
		//(*leaf->parent)(location.x+1,location.y,location.z);
		//parents.insert(leaf->parent);

	}
	/*
	std::vector<int3> positions;
	std::vector<EndlessNodeFloatPtr> result;
	grid.getLeafNodes(positions, result);
	for (int i = 0; i < positions.size(); i++) {
		EndlessNodeFloatPtr pnode;
		int3 pos = positions[i];
		EndlessNodeFloatPtr node = result[i];
		int dim = node->dim;
		pos += dim / 2;
		std::vector<float>& data = node->data;
		//test value for center side
		for (int n = 0; n < 26; n++) {
			int3 nbr;
			nbr.x = clamp(dim / 2 + dim * nbr26X[n], 0, dim - 1);
			nbr.y = clamp(dim / 2 + dim * nbr26Y[n], 0, dim - 1);
			nbr.z = clamp(dim / 2 + dim * nbr26Z[n], 0, dim - 1);
			float tval = data[nbr.x + (nbr.y + dim * nbr.z) * dim];
			//std::cout<<n<<") NBR POS "<<pos.x + dim * nbr26X[n]<<","<<pos.y + dim * nbr26Y[n]<<","<<pos.z + dim * nbr26Z[n]<<std::endl;
			float& val = grid.getMultiResolutionValue(pos.x + dim * nbr26X[n],pos.y + dim * nbr26Y[n], pos.z + dim * nbr26Z[n], pnode);
			if (val == backgroundValue) {
				val = aly::sign(tval) * narrowBand;
			}
		}
	}
*/
	/*
	 for (int d = grid.getTreeDepth() - 2; d >= 0; d--) {
	 auto nodes = grid.getNodesAtDepth(d);
	 std::cout << d << ") " << nodes.size() << std::endl;
	 for (EndlessNodeFloatPtr node : nodes) {
	 std::vector<int>& indexes = node->indexes;
	 std::vector<float>& data = node->data;
	 int dim = node->dim;
	 std::queue<int3> posQ, negQ;
	 for (int k = 0; k < dim; k++) {
	 for (int j = 0; j < dim; j++) {
	 for (int i = 0; i < dim; i++) {
	 float& cval = data[i + (j + k * dim) * dim];
	 if (cval == backgroundValue) {
	 for (int n = 0; n < 6; n++) {
	 int3 nbr = int3(i + nbrX[n], j + nbrY[n],
	 k + nbrZ[n]);
	 if (nbr.x >= 0 && nbr.x < dim && nbr.y >= 0
	 && nbr.y < dim && nbr.z >= 0
	 && nbr.z < dim) {
	 int idx = nbr.x
	 + (nbr.y + nbr.z * dim) * dim;
	 if (indexes[idx] >= 0) {
	 auto child =
	 node->children[indexes[idx]];
	 std::vector<float>& ldata = child->data;
	 int ldim = child->dim;
	 //test value for center side
	 nbr.x = clamp(ldim / 2 - ldim * nbrX[n],
	 0, ldim - 1);
	 nbr.y = clamp(ldim / 2 - ldim * nbrY[n],
	 0, ldim - 1);
	 nbr.z = clamp(ldim / 2 - ldim * nbrZ[n],
	 0, ldim - 1);
	 float tval = ldata[nbr.x
	 + (nbr.y + ldim * nbr.z) * ldim];
	 if (aly::sign(tval) > 0) {
	 cval = narrowBand;
	 posQ.push(int3(i, j, k));
	 } else if (aly::sign(tval) < 0) {
	 cval = -narrowBand;
	 negQ.push(int3(i, j, k));
	 }
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
	 int3 nbr = pos + int3(nbrX[n], nbrY[n], nbrZ[n]);
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
	 while (!negQ.empty()) {
	 int3 pos = negQ.front();
	 negQ.pop();
	 for (int n = 0; n < 6; n++) {
	 int3 nbr = pos + int3(nbrX[n], nbrY[n], nbrZ[n]);
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
	 }
	 }
	 */
	std::cout << "Done Grid Construction" << std::endl;
}

}
