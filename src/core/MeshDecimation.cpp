/*
 * MeshProcessing.cpp
 *
 *  Created on: Dec 19, 2017
 *      Author: blake
 */

#include <MeshDecimation.h>
namespace aly {
void DeadTriangle::set(DeadVertex *v0, DeadVertex *v1, DeadVertex *v2) {
	//assert(v0 != v1 && v1 != v2 && v2 != v0);
	vertex[0] = v0;
	vertex[1] = v1;
	vertex[2] = v2;

	vertex[0]->faces.insert(this);
	vertex[1]->faces.insert(this);
	vertex[2]->faces.insert(this);

	vertex[0]->neighbors.insert(vertex[1]);
	vertex[0]->neighbors.insert(vertex[2]);

	vertex[1]->neighbors.insert(vertex[2]);
	vertex[1]->neighbors.insert(vertex[0]);

	vertex[2]->neighbors.insert(vertex[0]);
	vertex[2]->neighbors.insert(vertex[1]);

}
void DeadVertex::remove(DeadTriangle* tri) {
	auto ptr = faces.find(tri);
	if (ptr != faces.end()) {
		faces.erase(ptr);
	}
}
void DeadVertex::remove(DeadVertex* nbr) {
	auto ptr = neighbors.find(nbr);
	if (ptr != neighbors.end()) {
		neighbors.erase(ptr);
	}
}
bool DeadTriangle::hasVertex(DeadVertex *v) {
	return (v == vertex[0] || v == vertex[1] || v == vertex[2]);
}
float3 DeadTriangle::updateNormal(Mesh& mesh) {
	return normal = computeNormal(mesh);
}
float3 DeadTriangle::computeNormal(Mesh& mesh) {
	float3 v0 = mesh.vertexLocations[vertex[0]->id];
	float3 v1 = mesh.vertexLocations[vertex[1]->id];
	float3 v2 = mesh.vertexLocations[vertex[2]->id];
	return normalize(cross(v2 - v0, v1 - v0));
}
void DeadTriangle::remove() {
	if (valid) {
		valid = false;
		for (int i = 0; i < 3; i++) {
			if (vertex[i]) {
				vertex[i]->remove(this);
			}
		}
		for (int i = 0; i < 3; i++) {
			int i2 = (i + 1) % 3;
			if (!vertex[i] || !vertex[i2])
				continue;
			vertex[i]->removeIfNonNeighbor(vertex[i2]);
			vertex[i2]->removeIfNonNeighbor(vertex[i]);
		}
	}
}
void DeadTriangle::replaceVertex(Mesh& mesh, DeadVertex *vold,
		DeadVertex *vnew) {
	//assert(vold && vnew);
	//assert(vold == vertex[0] || vold == vertex[1] || vold == vertex[2]);
	//assert(vnew != vertex[0] && vnew != vertex[1] && vnew != vertex[2]);
	if (vold == vertex[0]) {
		vertex[0] = vnew;
	} else if (vold == vertex[1]) {
		vertex[1] = vnew;
	} else {
		vertex[2] = vnew;
	}
	vold->remove(this);
	vnew->faces.insert(this);
	for (int i = 0; i < 3; i++) {
		vold->removeIfNonNeighbor(vertex[i]);
		vertex[i]->removeIfNonNeighbor(vold);
	}

	vertex[0]->neighbors.insert(vertex[1]);
	vertex[0]->neighbors.insert(vertex[2]);

	vertex[1]->neighbors.insert(vertex[2]);
	vertex[1]->neighbors.insert(vertex[0]);

	vertex[2]->neighbors.insert(vertex[0]);
	vertex[2]->neighbors.insert(vertex[1]);

	updateNormal(mesh);
}
bool DeadVertex::hasNeighbor(DeadVertex* nbr) {
	return (neighbors.find(nbr) != neighbors.end());
}
void DeadVertex::removeIfNonNeighbor(DeadVertex *n) {
	auto ptr = neighbors.find(n);
	if (ptr == neighbors.end())
		return;
	for (DeadTriangle* f : faces) {	//Why do we need this check?
		if (f->hasVertex(n))
			return;
	}
	neighbors.erase(ptr);
}
void DeadVertex::remove() {
	if (valid) {
		valid = false;
		for (DeadVertex* v : neighbors) {
			v->remove(this);
		}
	}
}

void MeshDecimation::solve(Mesh& mesh, float decimationAmount, bool flipNormals,const std::function<bool(const std::string& message, float progress)>& monitor) {
	if(decimationAmount<=0.0f)return;
	if (flipNormals) {
		mesh.flipNormals();
	}
	Vector3f& vertexLocations = mesh.vertexLocations;
	Vector3f& vertexNormals = mesh.vertexNormals;
	Vector4f& vertexColors = mesh.vertexColors;
	Vector3ui& triIndexes = mesh.triIndexes;
	size_t vertexCount = vertexLocations.size();
	size_t triCount = triIndexes.size();
	heap.reserve(vertexCount);
	vertexes.resize(vertexCount);
	triangles.resize(triCount);
	for (size_t i = 0; i < vertexCount; i++) {
		vertexes[i].id = i;
	}
	for (size_t i = 0; i < triCount; i++) {
		uint3 tri = triIndexes[i];
		triangles[i].set(&vertexes[tri.x], &vertexes[tri.y], &vertexes[tri.z]);
		triangles[i].updateNormal(mesh);
	}
	int targetRemoveCount = static_cast<int>(decimationAmount * vertexCount);
	size_t lastRemoveCount = -1;
	size_t removeCount = 0;

	size_t validCount = vertexCount;
	while (vertexCount - validCount < targetRemoveCount) {
		float amt = (targetRemoveCount- (vertexCount - mesh.vertexLocations.size()))/ (float) mesh.vertexLocations.size();
		size_t removeCount= decimateInternal(mesh, amt,(int)validCount, (int)targetRemoveCount, monitor);
		validCount = 0;
		for (const DeadVertex& v : vertexes) {
			if (v.isValid())
				validCount++;
		}
		removeCount = vertexCount - validCount;
		if (removeCount == lastRemoveCount)
			break;
		lastRemoveCount = removeCount;
	}
	if (vertexColors.size() > 0) {
		Vector3f vertexLocationsCopy = vertexLocations;
		Vector3f vertexNormalsCopy = vertexNormals;
		Vector4f vertexColorCopy = vertexColors;
		vertexLocations.clear();
		vertexNormals.clear();
		vertexColors.clear();
		vertexColors.data.reserve(vertexCount);
		vertexNormals.data.reserve(vertexCount);
		vertexLocations.data.reserve(vertexCount);
		for (size_t i = 0; i < vertexCount; i++) {
			DeadVertex& vert = vertexes[i];
			if (vert.isValid()&&vert.neighbors.size()>0) {
				size_t id = vert.id;
				vert.index = vertexLocations.size();
				vertexLocations.push_back(vertexLocationsCopy[id]);
				vertexNormals.push_back(vertexNormalsCopy[id]);
				vertexColors.push_back(vertexColorCopy[id]);

			}
		}
	} else {
		Vector3f vertexLocationsCopy = vertexLocations;
		Vector3f vertexNormalsCopy = vertexNormals;
		vertexLocations.clear();
		vertexNormals.clear();
		vertexNormals.data.reserve(vertexCount);
		vertexLocations.data.reserve(vertexCount);
		for (size_t i = 0; i < vertexCount; i++) {
			DeadVertex& vert = vertexes[i];
			if (vert.isValid()&&vert.neighbors.size()>0) {
				size_t id = vert.id;
				vert.index = vertexLocations.size();
				vertexLocations.push_back(vertexLocationsCopy[id]);
				vertexNormals.push_back(vertexNormalsCopy[id]);
			}
		}
	}
	size_t faceCount = triangles.size();
	triIndexes.clear();
	for (int n = 0; n < faceCount; n++) {
		DeadTriangle& tri = triangles[n];
		if (tri.isValid()) {
			triIndexes.push_back(
					uint3((uint32_t)tri.vertex[0]->index, (uint32_t)tri.vertex[1]->index,
							(uint32_t)tri.vertex[2]->index));
		}
	}
	if (flipNormals) {
		mesh.flipNormals();
	}
}

size_t MeshDecimation::decimateInternal(Mesh& mesh, float threshold,
		int validCount, int targetCount,const std::function<bool(const std::string& message, float progress)>& monitor) {
	size_t vertexCount =vertexes.size();
	heap.reserve(vertexCount);
	for (size_t i = 0; i < vertexCount; i++) {
		DeadVertex* vert=&vertexes[i];
		if(vert->isValid()){
			computeEdgeCostAtVertex(mesh,vert);
			heap.add(vert);
		} else {
			vert->collapse=nullptr;
		}
	}
	size_t removeCount = vertexCount-validCount;
	while (vertexes.size() > 0 && !heap.isEmpty()) {
		if (removeCount >= targetCount)
			break;
		if(monitor){
			monitor("Decimate",removeCount/(float)targetCount);
		}
		// get the next vertex to collapse
		DeadVertex *mn = static_cast<DeadVertex*>(heap.remove());
		if (mn->isValid()&&collapseEdge(mesh, mn, mn->collapse)) {
			removeCount++;
		}

	}
	heap.clear();
	return removeCount;
}
bool MeshDecimation::collapseEdge(Mesh& mesh, DeadVertex *u, DeadVertex *v) {
	// Collapse the edge uv by moving vertex u onto v
	// Actually remove tris on uv, then update tris that
	// have u to have v, and then remove u.
	if (!v) {
		// u is a vertex all by itself so just delete it
		u->remove();
		heap.remove(u);
		return true;
	}
	float3& V = mesh.vertexLocations[v->id];
	float3& U = mesh.vertexLocations[u->id];
	float3 mid = 0.5f * (U + V);
	float3 oldU = U;
	float3 oldV = V;
	U = mid;
	V = mid;
	bool flip = false;
	for (DeadTriangle* tri : v->faces) {
		float3 oldNormal = tri->normal;
		float3 n = tri->updateNormal(mesh);
		if (dot(n, mesh.vertexNormals[v->id]) < 0) {
			flip = true;
			break;
		}
	}
	if (!flip) {
		for (DeadTriangle *tri : u->faces) {
			float3 n = tri->updateNormal(mesh);
			if (dot(n, mesh.vertexNormals[u->id]) < 0) {
				flip = true;
				break;
			}
		}
	}
	if (flip) {
		V = oldV;
		U = oldU;
		return false;
	}
	size_t faceCount = 0;
	for (DeadTriangle* tri : u->faces) {
		if (tri->hasVertex(v)) {
			faceCount++;
		}
	}
	if (faceCount != 2 || v->neighbors.size() >= MAX_VALENCE
			|| u->neighbors.size() >= MAX_VALENCE) {
		V = oldV;
		U = oldU;
		return false;
	}
	// make tmp a Array of all the neighbors of u
	std::set<DeadVertex *> tmp = u->neighbors;
	// delete triangles on edge uv:
	std::set<DeadTriangle*> tmpNbrs = u->faces;
	for (DeadTriangle* tri : tmpNbrs) {
		if (tri->hasVertex(v))
			tri->remove();
	}
	tmpNbrs = u->faces;
	for (DeadTriangle* tri : tmpNbrs) {
		tri->replaceVertex(mesh, u, v);
	}
	u->remove();
	heap.remove(u);
// recompute the edge collapse costs for neighboring vertices
	for (DeadVertex *vert : tmp) {
		computeEdgeCostAtVertex(mesh, vert);
		heap.change(vert->index, vert->value);
	}
	return true;
}

float MeshDecimation::computeEdgeCollapseCost(Mesh& mesh, DeadVertex *u,
		DeadVertex *v) {
// if we collapse edge uv by moving u to v then how
// much different will the model change, i.e. how much "error".
// Texture, vertex normal, and border vertex code was removed
// to keep this demo as simple as possible.
// The method of determining cost was designed in order
// to exploit small and coplanar regions for
// effective polygon reduction.
// Is is possible to add some checks here to see if "folds"
// would be generated.  i.e. normal of a remaining face gets
// flipped.  I never seemed to run into this problem and
// therefore never added code to detect this case.
	float edgelength = distance(mesh.vertexLocations[v->id],
			mesh.vertexLocations[u->id]);
	float curvature = 0;
// find the "sides" triangles that are on the edge uv
	std::vector<DeadTriangle *> sides;
	for (DeadTriangle* tri : u->faces) {
		if (tri->hasVertex(v)) {
			sides.push_back(tri);
		}
	}
// use the triangle facing most away from the sides
// to determine our curvature term
	for (DeadTriangle* tri : u->faces) {
		float mincurv = 1; // curve for face i and closer side to it
		for (unsigned int j = 0; j < sides.size(); j++) {
			float dotprod = dot(tri->normal, sides[j]->normal); // use Dot product of face normals.
			mincurv = std::min(mincurv, (1 - dotprod) / 2.0f);
		}
		curvature = std::max(curvature, mincurv);
	}
// the more coplanar the lower the curvature term
	return edgelength * curvature;
}
void MeshDecimation::computeEdgeCostAtVertex(Mesh& mesh, DeadVertex *v) {
// compute the edge collapse cost for all edges that start
// from vertex v.  Since we are only interested in reducing
// the object by selecting the min cost edge at each step, we
// only cache the cost of the least cost edge at this vertex
// (in member variable collapse) as well as the value of the
// cost (in member variable objdist).
	if (v->neighbors.size() == 0) {
		// v doesn't have neighbors so it costs nothing to collapse
		v->collapse = nullptr;
		v->value = -0.01f;
		return;
	}
	v->value = std::numeric_limits<float>::max();
	v->collapse = nullptr;
// search all neighboring edges for "least cost" edge
	for (DeadVertex* vert : v->neighbors) {
		float dist;
		dist = computeEdgeCollapseCost(mesh, v, vert);
		if (dist < v->value) {
			v->collapse = vert;  // candidate for edge collapse
			v->value = dist;             // cost of the collapse
		}
	}
}
}

