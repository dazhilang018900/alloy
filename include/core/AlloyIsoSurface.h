// ref http://paulbourke.net/geometry/polygonise/ 
// File Name: IsoSurface.h
// Last Modified: 5/8/2000
// Author: Raghavendra Chandrashekara (basesd on source code
// provided by Paul Bourke and Cory Gene Bloyd)
// Email: rc99@doc.ic.ac.uk, rchandrashekara@hotmail.com
//
// Description: This is the interface file for the IsoSurface class.
// IsoSurface can be used to construct an isosurface from a scalar
// field.

#ifndef INCLUDE_ALLOYISOSURFACE_H_
#define INCLUDE_ALLOYISOSURFACE_H_
#include <AlloyMesh.h>
#include <AlloyVolume.h>
#include <map>
#include <unordered_map>
#include <vector>
#include <math.h>
#include <stdint.h>
#include <grid/EndlessGrid.h>
namespace aly {
struct EdgeInfo {
	float3 point;
	bool winding = false;
};
struct EdgeSplit {
private:
	int row, col, slice;
	int64_t index;

	/** The grid point reference less than the level set value. */
	int3 voxelIndex1;

	/** The grid point reference greater than the level set value. */
	int3 voxelIndex2;

public:
	/** The interpolated point on the target level set. */
	aly::float3 point;
	aly::float4 color;
	/** The vertex id for the interpolated point. */
	size_t vertexId;

	EdgeSplit() :
			index(0), row(0), col(0), slice(0), vertexId(0), point(), voxelIndex1(
					0), voxelIndex2(0), color() {

	}
	/**
	 * Instantiates a new edge split.
	 *
	 * @param pt1
	 *            the lower grid point
	 * @param pt2
	 *            the upper grid point
	 */
	EdgeSplit(const int3& pt1, const int3& pt2, const int& r, const int& c,
			const int& s) :
			voxelIndex1(pt1), voxelIndex2(pt2), index(r * c * s), row(r), col(
					c), slice(s), vertexId(0), point(), color() {

	}

	EdgeSplit(const EdgeSplit& edgeSplit) :
			voxelIndex1(edgeSplit.voxelIndex1), voxelIndex2(
					edgeSplit.voxelIndex2), index(edgeSplit.index), row(
					edgeSplit.row), col(edgeSplit.col), slice(edgeSplit.slice), point(
					edgeSplit.point), vertexId(edgeSplit.vertexId), color(
					edgeSplit.color) {

	}

	EdgeSplit& operator=(const EdgeSplit& edgeSplit) {
		if (this != &edgeSplit) {
			voxelIndex1 = edgeSplit.voxelIndex1;
			voxelIndex2 = edgeSplit.voxelIndex2;
			index = edgeSplit.index;
			row = edgeSplit.row;
			col = edgeSplit.col;
			slice = edgeSplit.slice;
			point = edgeSplit.point;
			vertexId = edgeSplit.vertexId;
			color = edgeSplit.color;
		}
		return *this;
	}

	/** * Compare two edge splits.
	 *
	 * @param split
	 *            the split
	 *
	 * @return true, if successful ** */
	//bool equals(const EdgeSplit& split)
	bool operator==(const EdgeSplit& split) {
		return ((voxelIndex1 == split.voxelIndex1
				&& voxelIndex2 == split.voxelIndex2)
				|| (voxelIndex1 == split.voxelIndex2
						&& voxelIndex2 == split.voxelIndex1));
	}

	/**	* Generate hash value for grid point.
	 *
	 * @param pt the grid point
	 * @return the hash value ** */
	inline int64_t hashValue(const int3& pt) {
		return (pt.z + 1) * (row + 2) * (col + 2) + (row + 2) * (pt.y + 1)
				+ (pt.x + 1);
	}

	/**	* Hash value that uniquely identifies edge split.
	 *
	 * @return the hash value ** */
	inline int64_t hashValue() {
		int64_t h1 = hashValue(voxelIndex1);
		int64_t h2 = hashValue(voxelIndex2);
		if (h1 < h2) {
			return h1 + index * h2;
		} else {
			return h2 + index * h1;
		}
	}
	inline int4 bigHashValue() {
		if (voxelIndex1 < voxelIndex2) {
			int3 diff=(voxelIndex2-voxelIndex1)+1;
			return int4(voxelIndex1,diff.x+diff.y*3+diff.z*9);
		} else {
			int3 diff=(voxelIndex1-voxelIndex2)+1;
			return int4(voxelIndex2,diff.x+diff.y*3+diff.z*9);
		}
	}
};

/**
 * The Class IsoTriangle stores vertex ids.
 */
struct IsoTriangle {

	/** The vertex ids. */
public:
	int vertexIds[3];
	IsoTriangle() {
		vertexIds[0] = 0;
		vertexIds[1] = 0;
		vertexIds[2] = 0;
	}

	IsoTriangle(const IsoTriangle &tri) {

		vertexIds[0] = tri.vertexIds[0];
		vertexIds[1] = tri.vertexIds[1];
		vertexIds[2] = tri.vertexIds[2];
	}

	IsoTriangle& operator=(const IsoTriangle& tri) {
		vertexIds[0] = tri.vertexIds[0];
		vertexIds[1] = tri.vertexIds[1];
		vertexIds[2] = tri.vertexIds[2];
		return *this;
	}

	~IsoTriangle() {
	}
};

/*** The winding order for triangle vertices.*/
enum class Winding { /** The CLOCKWISE. */
	CLOCKWISE,
	/** The COUNTE r_ clockwise. */
	COUNTER_CLOCKWISE
};
enum class MeshType {
	TRIANGLE, QUAD
};
class IsoSurface {
private:
	bool skipHidden;
	size_t triangleCount;
	void regularize(const float* data, Mesh& mesh);
	void regularize(const EndlessGridFloat& grid, Mesh& mesh);
	aly::float4 getImageColor(const float4* image, int i, int j, int k);
	size_t getSafeIndex(int i, int j, int k);
	size_t getIndex(int i, int j, int k);
	aly::float4 interpolateColor(const float4 *pRGB, float x, float y, float z);
	aly::float3 interpolateNormal(const float *pVolMat, float x, float y,
			float z);
	float interpolate(const float *data, float x, float y, float z);
	float getImageValue(const float* image, int i, int j, int k);
	void triangulateUsingMarchingCubes(const float* pVolMat,
			std::map<int64_t, EdgeSplit>& splits,
			std::vector<IsoTriangle>& triangles, int x, int y, int z,
			size_t& vertexCount);
	void triangulateUsingMarchingCubes(const float* pVolMat,
			std::map<int4, EdgeSplit>& splits,
			std::vector<IsoTriangle>& triangles,
			int x, int y, int z,
			int ox, int oy, int oz, size_t& vertexCount);
	void triangulateUsingMarchingCubes(const float* pVolMat,
			std::vector<EdgeSplit>& splits, std::vector<IsoTriangle>& triangles,
			int x, int y, int z, size_t& vertexCount);

	static std::vector<std::vector<int>> buildVertexNeighborTable(
			const int vertexCount, const int* indexes, const int indexCount,
			Winding direction);

	static std::vector<std::vector<int>> buildVertexNeighborTable(
			const int vertexCount, const int* indexes, const int indexCount);

	static std::vector<int> buildFaceNeighborTable(int vertexCount,
			const int* indexes, const int indexCount);
	void findActiveVoxels(const float* vol, const std::vector<int3>& indexList,
			std::set<int3>& activeVoxels,
			std::map<int4, EdgeInfo>& activeEdges);
	void generateVertexData(const float* data, const std::set<int3>& voxels,
			const std::map<int4, EdgeInfo>& edges,
			std::map<int3, uint32_t>& vertexIndices, Mesh& buffer);
	void generateTriangles(const std::map<int4, EdgeInfo>& edges,
			const std::map<int3, uint32_t>& vertexIndices, Mesh& buffer);
	void solveQuad(const float* data, const int& rows, const int& cols,
			const int& slices, const std::vector<int3>& indexList, Mesh& mesh, const float& isoLevel = 0.0f);
	void solveQuad(const EndlessGridFloat& grid,Mesh& mesh,const float& isoLevel);
	void generateVertexData(const EndlessGridFloat& grid,
			const std::set<int3>& voxels, const std::map<int4, EdgeInfo>& edges,
			std::map<int3, uint32_t>& vertexIndices, Mesh& buffer);
	void solveTri(const float* data, const int& rows, const int& cols,
			const int& slices, const std::vector<int3>& indexList,
			Mesh& mesh, const float& isoLevel = 0);
	void solveTri(const EndlessGridFloat& grid,
			Mesh& mesh,
			const float& isoLevel = 0);
	void findActiveVoxels(
			const EndlessGridFloat& grid,
			const std::vector<EndlessNodeFloatPtr>& leafs,
			std::set<int3>& activeVoxels,
			std::map<int4, EdgeInfo>& activeEdges);
public:
	IsoSurface();
	~IsoSurface();

	void solve(
			const EndlessGridFloat& grid,
			Mesh& mesh,
			const MeshType& type,
			bool regularizeTest,
			const float& isoLevel);
	void solve(const Volume1f& data, const std::vector<int3>& indexList,
			Mesh& mesh, const MeshType& type = MeshType::TRIANGLE,
			bool regularize = true, const float& isoLevel = 0);
	void solve(const Volume1f& data,
			Mesh& mesh, const MeshType& type = MeshType::TRIANGLE,
			bool regularize = true, const float& isoLevel = 0);

	void solve(const float* data, const int& rows, const int& cols,
			const int& slices, const std::vector<int3>& indexList, Mesh& mesh,
			const MeshType& type = MeshType::TRIANGLE, bool regularize = true,
			const float& isoLevel = 0);
	void project(aly::float3* points, const int& numPoints,
			aly::float3* normals, float* levelset, const aly::box3f& bbox,
			const int& rows, const int& cols, const int& slices, int maxIters,
			float errorThresh);
	void project(aly::float3* points, const int& numPoints,
			aly::float3* normals, float* levelset, const int& rows,
			const int& cols, const int& slices, int maxIters,
			float errorThresh);
	void computeNormals(const std::vector<aly::float3>& points,
			const std::vector<int>& indexes, std::vector<aly::float3>& normals);
	void computeColors(const std::vector<aly::float3>& points,
			std::vector<aly::float4>& colors, const float4* pfRgb,
			aly::box3f& bbox, const int& rows, const int& cols,
			const int& slices);
	/*
	 static void Regularize(char (&statusMessage)[STATUS_MESSAGE_LENGTH], std::vector<aly::float4>& vPoints,
	 const std::vector<std::vector<int>>& nbrs, const float lambda, const int iters);
	 */
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
	float isoLevel;			// = 0;
	int rows;			// = 0
	int cols;			// = 0,
	int slices;			// = 0;
	float getValue(const float* pVolMat, int i, int j, int k);
	aly::float3 getNormal(const float *pVolMat, int i, int j, int k);
	float getOffset(const float* pVolMat, const int3& v1, const int3& v2);
};
}
#endif
