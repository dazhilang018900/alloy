/*
 * MeshProcessing.h
 *
 *  Created on: Dec 19, 2017
 *      Author: blake
 */

#ifndef INCLUDE_GRID_MESHPROCESSING_H_
#define INCLUDE_GRID_MESHPROCESSING_H_

#include "AlloyMesh.h"
#include "BinaryMinHeap.h"
namespace aly {

struct DeadTriangle;
struct DeadVertex;

struct DeadVertex: public IndexablePtr<float> {
protected:
	bool valid;
public:

	std::set<DeadVertex*> neighbors; 	// adjacent vertices
	std::set<DeadTriangle*> faces;    	// adjacent triangles
	DeadVertex *collapse;					// candidate vertex for collapse
	size_t id;
	DeadVertex(size_t id = -1) : IndexablePtr<float>(0.0f), collapse(nullptr), id(id), valid(true) {
	}
	virtual ~DeadVertex(){}
	inline bool isValid() const {
		return valid;
	}
	bool hasNeighbor(DeadVertex* nbr);
	void remove(DeadTriangle* tri);
	void remove(DeadVertex* nbr);
	void remove();
	void removeIfNonNeighbor(DeadVertex *n);
	bool operator<(const DeadVertex &rhs) const {
		return (value < rhs.value);
	}

	bool operator<=(const DeadVertex &rhs) const {
		return (value <= rhs.value);
	}

	bool operator>(const DeadVertex &rhs) const {
		return (value > rhs.value);
	}

	bool operator>=(const DeadVertex &rhs) const {
		return (value >= rhs.value);
	}

	bool operator==(const DeadVertex &rhs) const {
		return (index == rhs.index);
	}

};

struct DeadTriangle {
	bool valid;
public:
	float3 normal;
	DeadVertex* vertex[3];	// the 3 points that make this tri
	DeadTriangle() :
			valid(true) {
	}
	void set(DeadVertex *v0, DeadVertex *v1, DeadVertex *v2);
	virtual ~DeadTriangle() {
	}
	float3 updateNormal(Mesh& mesh);
	float3 computeNormal(Mesh& mesh);
	void replaceVertex(Mesh& mesh, DeadVertex *vold, DeadVertex *vnew);
	bool hasVertex(DeadVertex *v);
	void remove();
	inline bool isValid() const {
		return valid;
	}
};
class MeshDecimation {
protected:
	static const int MAX_VALENCE = 9;
	std::vector<DeadTriangle> triangles;
	std::vector<DeadVertex> vertexes;
	BinaryMinHeapPtr<float> heap;
	void decimateInternal(Mesh& mesh, float threshold, int totalCount,
			int targetCount);
	bool collapseEdge(Mesh& mesh, DeadVertex *u, DeadVertex *v);
	void computeEdgeCostAtVertex(Mesh& mesh, DeadVertex *v);
	float computeEdgeCollapseCost(Mesh& mesh, DeadVertex *u, DeadVertex *v);
public:
	friend class DeadTriangle;
	void solve(Mesh& mesh, float amount);
};
}

#endif /* INCLUDE_GRID_MESHPROCESSING_H_ */
