/*
 * EndlessGrid.cpp
 *
 *  Created on: Nov 21, 2017
 *      Author: blake
 */

#include "grid/EndlessGrid.h"
#include "AlloyVolume.h"
namespace aly {
void WriteGridToFile(const std::string& dir, EndlessGrid<float>& grid) {
	std::vector<std::pair<int3, EndlessNodeFloatPtr>> nodes = grid.getNodes();
	int dim = grid.getNodeSize();
	Volume1f data(dim, dim, dim);
	for (auto pr : nodes) {
		int3 pos = pr.first;
		EndlessNodeFloatPtr node = pr.second;
		std::string file = MakeString() << dir << ALY_PATH_SEPARATOR<<"grid_"<<pos.x<<"_"<<pos.y<<"_"<<pos.z<<".xml";
		data.set(grid.getBackgroundValue());
		std::vector<int3> positions;
		std::vector<EndlessNodeFloatPtr> result;
		//std::cout<<"ROOT "<<node->location<<" "<<node->dim<<" "<<(size_t)node->parent<<std::endl;
		node->getLeafNodes(positions, result, grid.getCellSizes());
		for (int n = 0; n < positions.size(); n++) {
			int3 offset = positions[n];
			//std::cout << n << ") position=" << offset << std::endl;
			EndlessNodeFloatPtr leaf = result[n];
			int D = leaf->dim;
			for (int k = 0; k < D; k++) {
				for (int j = 0; j < D; j++) {
					for (int i = 0; i < D; i++) {
						data(i + offset.x, j + offset.y, k + offset.z).x =
								(*leaf)(i, j, k);
					}
				}
			}
		}
		WriteVolumeToFile(file, data);
	}

}
void MeshToLevelSet(const Mesh& mesh, EndlessGrid<float>& grid) {
	grid.clear();
	grid.setBackgroundValue(3.0f);
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
	float res = std::sqrt(averageSize);
	int3 minIndex = int3(bbox.position / res) - int3(2, 2, 2);
	std::cout << "Splats " << splats.size() << std::endl;
	for (int n = 0; n < (int) splats.size(); n++) {
		box3f box = splats[n];
		int3 dims = int3(aly::ceil(box.dimensions / res) + float3(2, 2, 2));
		int3 pos = int3(aly::floor(box.position / res) - float3(1, 1, 1));
		EndlessLocation gpos;
		uint3 tri = mesh.triIndexes[n];
		float3 v1 = mesh.vertexLocations[tri.x];
		float3 v2 = mesh.vertexLocations[tri.y];
		float3 v3 = mesh.vertexLocations[tri.z];
		float3 closestPoint;
		for (int k = 0; k <= dims.z; k++) {
			for (int j = 0; j <= dims.y; j++) {
				for (int i = 0; i <= dims.x; i++) {
					float& value = grid.getValue(i + pos.x - minIndex.x,
							j + pos.y - minIndex.y, k + pos.z - minIndex.z);
					float3 pt = float3(res * (i + pos.x), res * (j + pos.y),
							res * (k + pos.z));
					value = std::min(value,
							std::sqrt(
									DistanceToTriangleSqr(pt, v1, v2, v3,
											&closestPoint))/res);
					//std::shared_ptr<EndlessNode<float>> node=grid.getLocation(,gpos);
					//std::cout<<gpos<<std::endl;
				}
			}
		}
	}
}

}
