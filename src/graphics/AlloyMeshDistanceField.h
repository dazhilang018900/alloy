/*
 * AlloyMeshDistanceField.h
 *
 *  Created on: Nov 2, 2018
 *      Author: blake
 */

#ifndef SRC_GRAPHICS_ALLOYMESHDISTANCEFIELD_H_
#define SRC_GRAPHICS_ALLOYMESHDISTANCEFIELD_H_
#include "graphics/AlloyMesh.h"
#include "image/AlloyMinHeap.h"
namespace aly {

class AlloyMeshDistanceField {
protected:
	typedef IndexablePtr<float> VertexIndexed;
	std::vector<VertexIndexed> indexes;
	MeshListNeighborTable nbrTable;
	float ComputeDistanceForAcute(float Ta, float Tb, float a,float b, float c, float Fc);
	float ComputeDistance(int vIDc, int vIDa, int vIDb,const  aly::Mesh& mesh, std::vector<float>& distanceField,std::vector<uint8_t>& label, float Fc, BinaryMinHeapPtr<float>& heap);
public:
	static const uint8_t ACTIVE;
	static const uint8_t NARROW_BAND;
	static const uint8_t FAR_AWAY;
	static const float MAX_VALUE;

	AlloyMeshDistanceField();
	void solve(const aly::Mesh& mesh, const std::vector<size_t>& zeroIndexes, std::vector<float>& distanceField, float maxDistance = 2.5f);
	virtual ~AlloyMeshDistanceField();
};

} /* namespace intel */

#endif /* SRC_GRAPHICS_ALLOYMESHDISTANCEFIELD_H_ */
