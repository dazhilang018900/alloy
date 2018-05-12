/*
 * Copyright(C) 2018, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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



// ref http://paulbourke.net/geometry/polygonise/ 
// File Name: MultiIsoSurface.h
// Last Modified: 5/8/2000
// Author: Raghavendra Chandrashekara (basesd on source code
// provided by Paul Bourke and Cory Gene Bloyd)
// Email: rc99@doc.ic.ac.uk, rchandrashekara@hotmail.com
//
// Description: This is the interface file for the MultiIsoSurface class.
// MultiIsoSurface can be used to construct an isosurface from a scalar
// field.

#ifndef INCLUDE_MULTIISOSURFACE_H_
#define INCLUDE_MULTIISOSURFACE_H_
#include <AlloyMesh.h>
#include <AlloyVolume.h>
#include <AlloyMath.h>
#include <AlloyEnum.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <math.h>
#include <stdint.h>
#include <AlloyIsoSurface.h>
#include "grid/EndlessGrid.h"
namespace aly {
class MultiIsoSurface {
private:
	float backgroundValue;
	bool skipHidden;
	size_t triangleCount;
	void regularize(const float* data, Mesh& mesh,int label);
	void regularize(const EndlessGridFloatInt& grid, Mesh& mesh,int label);
	aly::float4 getImageColor(const float4* image, int i, int j, int k);
	size_t getSafeIndex(int i, int j, int k);
	size_t getIndex(int i, int j, int k);
	aly::float4 interpolateColor(const float4 *pRGB, float x, float y, float z);
	aly::float3 interpolateNormal(const float *pVolMat, float x, float y,
			float z);
	float interpolate(const float *data, float x, float y, float z);
	float getImageValue(const float* image, int i, int j, int k);
	void triangulateUsingMarchingCubes(const float* pVolMat,const int* labels,
			std::unordered_map<uint64_t, EdgeSplit3D>& splits,
			std::vector<IsoTriangle>& triangles, int x, int y, int z,int label,
			size_t& vertexCount);
	void triangulateUsingMarchingCubes(const float* pVolMat,const int* labels,
			std::unordered_map<int4, EdgeSplit3D>& splits,
			std::vector<IsoTriangle>& triangles,
			int x, int y, int z,int label,
			int ox, int oy, int oz, size_t& vertexCount);
	void triangulateUsingMarchingCubes(const float* pVolMat,const int* labels,
			std::vector<EdgeSplit3D>& splits,
			std::vector<IsoTriangle>& triangles,
			int x, int y, int z,int label, size_t& vertexCount);

	static std::vector<std::vector<int>> buildVertexNeighborTable(
			const int vertexCount, const int* indexes, const int indexCount,
			Winding direction);

	static std::vector<std::vector<int>> buildVertexNeighborTable(
			const int vertexCount, const int* indexes, const int indexCount);

	static std::vector<int> buildFaceNeighborTable(int vertexCount,
			const int* indexes, const int indexCount);
	void findActiveVoxels(const float* vol,const int* labels, const std::vector<int3>& indexList,
			std::unordered_set<int3>& activeVoxels,
			std::unordered_map<int4, EdgeInfo>& activeEdges,int label);
	void generateVertexData(
			const float* data,
			const int* labels,
			const std::unordered_set<int3>& voxels,
			const std::unordered_map<int4, EdgeInfo>& edges,
			std::unordered_map<int3, uint32_t>& vertexIndices,
			Mesh& buffer,int label);
	void generateTriangles(const std::unordered_map<int4, EdgeInfo>& edges,
			const std::unordered_map<int3, uint32_t>& vertexIndices, Mesh& buffer);
	void solveQuad(const float* data,const int* labels,
			const int& rows, const int& cols,
			const int& slices,
			const std::vector<int3>& indexList, Mesh& mesh,
			int label);
	void solveQuad(const EndlessGridFloatInt& grid,Mesh& mesh,int label);
	void generateVertexData(
			const EndlessGridFloatInt& grid,
			const std::unordered_set<int3>& voxels,
			const std::unordered_map<int4, EdgeInfo>& edges,
			std::unordered_map<int3, uint32_t>& vertexIndices,
			Mesh& buffer,
			int label);
	void solveTri(const float* data,const int* labels,
			const int& rows, const int& cols,
			const int& slices, const std::vector<int3>& indexList,
			Mesh& mesh,int label);
	void solveTri(const EndlessGridFloatInt& grid,
			Mesh& mesh,int label);
	void findActiveVoxels(
			const EndlessGridFloatInt& grid,
			const std::list<EndlessNodeFloatInt*>& leafs,
			std::unordered_set<int3>& activeVoxels,
			std::unordered_map<int4, EdgeInfo>& activeEdges,
			int label);
public:
	MultiIsoSurface();
	~MultiIsoSurface();
	void solve(
			const EndlessGridFloatInt& grid,
			Mesh& mesh,
			const MeshType& type,
			bool regularizeTest,
			int label);
	void solve(const Volume1f& data,
			const Volume1i& labels, const std::vector<int3>& indexList,
			Mesh& mesh, const MeshType& type,
			bool regularize, int label);
	void solve(const Volume1f& data,const Volume1i& labels,
			Mesh& mesh, const MeshType& type,
			bool regularize, int label);
	void solve(const float* data, const int* labels, const int& rows, const int& cols,
			const int& slices, const std::vector<int3>& indexList, Mesh& mesh,
			const MeshType& type ,
			bool regularize ,
			int label);
	void project(aly::float3* points, const int& numPoints,
			aly::float3* normals, float* levelset, const aly::box3f& bbox,
			const int& rows, const int& cols, const int& slices, int maxIters,
			float errorThresh);
	void project(aly::float3* points, const int& numPoints,
			aly::float3* normals, float* levelset, const int& rows,
			const int& cols, const int& slices, int maxIters,
			float errorThresh);
	inline static const int* getTriangleConnectionTable() {
		return triangleConnectionTable;
	}
	inline static int getConnectionTableRows() {
		return 256;
	}
	inline static int getConnectionTableCols() {
		return 16;
	}

	/* a2fEdgeDirection lists the direction vector (vertex1-vertex0) for each edge in the cube. */
	static const float edgeDirection[12][3];

	/* Lists the positions, relative to vertex 0, of each of the 8 vertices of a cube. */
	static const int vertexOffset[8][3];

	/* Lists the index of the endpoint vertices for each of the 12 edges of the cube. */
	static const int edgeConnection[12][2];

	/* Lists the index of the endpoint vertices for each of the 6 edges of the tetrahedron. */
	static const int tetrahedronEdgeConnection[6][2];

	/* Lists the index of verticies from a cube that made up each of the six tetrahedrons within the cube. */
	static const int tetrahedronsInACube[6][4];

	/**
	 * For each of the possible vertex states listed in aiTetrahedronEdgeFlags
	 * there is a specific triangulation of the edge intersection points.
	 * a2iTetrahedronTriangles lists all of them in the form of 0-2 edge triples
	 * with the list terminated by the invalid value -1.
	 */
	static const int tetrahedronTriangles[16][7];

	/**
	 * For any edge, if one vertex is inside of the surface and the other is
	 * outside of the surface then the edge intersects the surface For each of
	 * the 8 vertices of the cube can be two possible states : either inside or
	 * outside of the surface For any cube the are 2^8=256 possible sets of
	 * vertex states This table lists the edges intersected by the surface for
	 * all 256 possible vertex states There are 12 edges. For each entry in the
	 * table, if edge #n is intersected, then bit #n is set to 1
	 */
	static const int cubeEdgeFlags[256];

	/** The Constant aiCubeEdgeFlagsCC618. */
	static const int cubeEdgeFlagsCC618[256];

	/** The Constant aiCubeEdgeFlagsCC626. */
	static const int cubeEdgeFlagsCC626[256];

	/** Look-up table for marching cubes. */
	static const int triangleConnectionTable[4096];

	static const int3 AXIS_OFFSET[3];
	static const int3 EDGE_NODE_OFFSETS[3][4];
	static const uint32_t ENCODED_EDGE_NODE_OFFSETS[12];
	static const uint32_t ENCODED_EDGE_OFFSETS[12];
protected:
	Winding winding;			// = Winding.CLOCKWISE;
	int rows;			// = 0
	int cols;			// = 0,
	int slices;			// = 0;
	float getValue(const float* pVolMat, const int* labels, int i, int j, int k,int l);
	aly::float3 getNormal(const float *pVolMat, int i, int j, int k);
	float getOffset(const float* pVolMat,const int* labels, const int3& v1, const int3& v2,int l);
};
}
#endif
