/*
 * EndlessGrid.cpp
 *
 *  Created on: Nov 21, 2017
 *      Author: blake
 */

#include "grid/EndlessGrid.h"

namespace aly {
void MeshToLevelSet(const Mesh& mesh, EndlessGrid<float>& grid) {
	grid.clear();
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
	std::cout << "Average Area " << averageSize << " " << std::sqrt(averageSize)<< std::endl;
	float res = 0.5f * std::sqrt(averageSize);
	std::cout << "Splats " << splats.size() << std::endl;
	for (int n = 0; n < (int) splats.size(); n+=100) {
		box3f box = splats[n];
		int3 dims =int3(aly::ceil(box.dimensions / res) + float3(2,2,2));
		int3 pos = int3(aly::floor(box.position / res) - float3(1,1,1));
		std::cout<<"Splat "<<pos<<" "<<dims<<std::endl;
		EndlessLocation gpos;
		for (int k = 0; k <= dims.z; k++) {
			for (int j = 0; j <= dims.y; j++) {
				for (int i = 0; i <= dims.x; i++) {
					float& value=grid.getValue(i+pos.x,j+pos.y,k+pos.z);
					value=1;
					//std::shared_ptr<EndlessNode<float>> node=grid.getLocation(,gpos);
					//std::cout<<gpos<<std::endl;
				}
			}
		}
	}
}

}
