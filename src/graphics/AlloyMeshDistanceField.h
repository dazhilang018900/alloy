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
void SANITY_CHECK_MESHDISTANCEFIELD();
class AlloyMeshDistanceField {
protected:
	typedef Indexable<float> VertexIndexed;
	std::list<VertexIndexed> indexes;
	MeshListNeighborTable nbrTable;
	float ComputeDistanceForAcute(float Ta, float Tb, float a,float b, float c, float Fc);
	float ComputeDistance(size_t vIDc, size_t vIDa, size_t vIDb,const  aly::Mesh& mesh, std::vector<float>& distanceField,std::vector<uint8_t>& label, float Fc, BinaryMinHeap<float>& heap);
public:
	static const uint8_t ACTIVE;
	static const uint8_t NARROW_BAND;
	static const uint8_t FAR_AWAY;
	static const float MAX_VALUE;

	AlloyMeshDistanceField(){}
	float solve(const aly::Mesh& mesh, const std::vector<size_t>& zeroIndexes, std::vector<float>& distanceField, float maxDistance = -1);
	virtual ~AlloyMeshDistanceField(){}
};

} /* namespace intel */

#endif /* SRC_GRAPHICS_ALLOYMESHDISTANCEFIELD_H_ */
