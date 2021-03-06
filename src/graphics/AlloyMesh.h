/*
 * Copyright(C) 2014, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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
#ifndef ALLOYMESH_H_
#define ALLOYMESH_H_

#include <mutex>
#include "graphics/GLComponent.h"
#include "math/AlloyVector.h"
#include "math/AlloyVecMath.h"
#include "image/AlloyImage.h"
#include "ui/AlloyContext.h"
#include <vector>
#include <set>
#include <unordered_set>
#include <list>

namespace aly {
bool SANITY_CHECK_SUBDIVIDE();
class Mesh;
enum class SubDivisionScheme {
	CatmullClark, Loop
};
struct GLMesh: public GLComponent {
public:
	enum class PrimitiveType {
		ALL = 0, QUADS = 4, TRIANGLES = 3, LINES = 2, POINTS = 1
	};
	Mesh& mesh;
	GLuint vao;
	GLuint vertexBuffer;
	GLuint normalBuffer;
	GLuint colorBuffer;
	GLuint lineIndexBuffer;
	GLuint triIndexBuffer;
	GLuint quadIndexBuffer;

	GLuint lineColorBuffer[2];
	GLuint lineVertexBuffer[2];

	GLuint triColorBuffer[3];
	GLuint triVertexBuffer[3];

	GLuint quadVertexBuffer[4];
	GLuint triNormalBuffer[3];

	GLuint quadColorBuffer[4];
	GLuint quadNormalBuffer[4];

	GLuint triTextureBuffer[3];
	GLuint quadTextureBuffer[4];

	GLuint lineCount;
	GLuint triCount;
	GLuint quadCount;
	GLuint vertexCount;

	GLuint lineIndexCount;
	GLuint triIndexCount;
	GLuint quadIndexCount;

	virtual void draw() const override;
	virtual void draw(const PrimitiveType& type, bool forceVertexColor=false) const;
	virtual void update() override;
	void updateVertexColors();
	void updateVertexPositions();
	GLMesh(Mesh& mesh,
			const std::shared_ptr<AlloyContext>& context =
					AlloyDefaultContext());
	virtual ~GLMesh();
};
class Mesh {
private:
	bool dirty = false;
	GLMesh::PrimitiveType type = GLMesh::PrimitiveType::ALL;
protected:
	box3f boundingBox;
public:
	friend struct GLMesh;

	Vector3f vertexLocations;
	Vector3f vertexNormals;
	Vector4f vertexColors;

	Vector4ui quadIndexes;
	Vector3ui triIndexes;
	Vector2ui lineIndexes;
	std::vector<uint32_t> pointIndexes;
	Vector2f textureMap;
	Image4f textureImage;

	std::shared_ptr<GLMesh> glOnScreen;
	float4x4 pose;

	inline void clone(Mesh& mesh) const {
		mesh.boundingBox = boundingBox;
		mesh.vertexLocations = vertexLocations;
		mesh.vertexNormals = vertexNormals;
		mesh.vertexColors = vertexColors;

		mesh.quadIndexes = quadIndexes;
		mesh.triIndexes = triIndexes;
		mesh.lineIndexes = lineIndexes;
		mesh.pointIndexes = pointIndexes;
		mesh.textureMap = textureMap;
		mesh.textureImage = textureImage;
		mesh.pose = pose;
		mesh.type = type;
		mesh.dirty = true;
	}
	void flipNormals();
	bool isEmpty() const {
		return (vertexLocations.size() == 0 && pointIndexes.size() == 0
				&& vertexNormals.size() == 0 && vertexColors.size() == 0
				&& lineIndexes.size() == 0 && quadIndexes.size() == 0
				&& triIndexes.size() == 0 && textureMap.size() == 0);
	}
	template<class Archive> void serialize(Archive & archive) {
		archive(CEREAL_NVP(pose), CEREAL_NVP(vertexLocations),
				CEREAL_NVP(vertexNormals), CEREAL_NVP(vertexColors),
				CEREAL_NVP(quadIndexes), CEREAL_NVP(triIndexes),
				CEREAL_NVP(lineIndexes), CEREAL_NVP(pointIndexes),
				CEREAL_NVP(textureMap), CEREAL_NVP(textureImage));
	}
	void setContext(const std::shared_ptr<AlloyContext>& context);
	Mesh(const std::shared_ptr<AlloyContext>& context);
	Mesh();
	inline void setType(GLMesh::PrimitiveType type) {
		this->type = type;
	}
	inline GLMesh::PrimitiveType getType() {
		return type;
	}
	inline box3f getBoundingBox() const {
		return boundingBox;
	}
	virtual void draw(const GLMesh::PrimitiveType& type, bool froceVertexColor);
	box3f updateBoundingBox();
	void scale(float sc);
	void transform(const float4x4& M);
	float estimateVoxelSize(int stride = 1);
	void update();
	void updateVertexColors();
	void updateVertexPositions();

	void clear();
	void setDirty(bool d);
	bool isDirty() const;
	bool load(const std::string& file);
	void updateVertexNormals(bool flipSign = false, int SMOOTH_ITERATIONS = 0,
			float DOT_TOLERANCE = 0.75f);

	bool convertQuadsToTriangles();
	void mapIntoBoundingBox(float voxelSize);
	void mapOutOfBoundingBox(float voxelSize);
	bool save(const std::string& file);
	virtual ~Mesh();
};
template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const Mesh & m) {
	ss << "Mesh Dimensions:" << m.getBoundingBox().dimensions << std::endl;
	if (m.vertexLocations.size() > 0)
		ss << "\tVertex Locations: " << m.vertexLocations.size() << std::endl;
	if (m.vertexNormals.size() > 0)
		ss << "\tVertex Normals: " << m.vertexNormals.size() << std::endl;
	if (m.vertexColors.size() > 0)
		ss << "\tVertex Colors: " << m.vertexColors.size() << std::endl;
	if (m.quadIndexes.size() > 0)
		ss << "\tQuad Faces: " << m.quadIndexes.size() << std::endl;
	if (m.triIndexes.size() > 0)
		ss << "\tTriangle Faces: " << m.triIndexes.size() << std::endl;
	if (m.lineIndexes.size() > 0)
		ss << "\tLine Indexes: " << m.lineIndexes.size() << std::endl;
	if (m.pointIndexes.size() > 0)
		ss << "\tPoint Indexes: " << m.pointIndexes.size() << std::endl;
	if (m.textureMap.size() > 0)
		ss << "\tTexture Map: " << m.textureMap.size() << std::endl;
	if (m.textureImage.size() > 0)
		ss << "\tTexture Image: " << m.textureImage.dimensions() << std::endl;
	return ss;
}
void ReadMeshFromFile(const std::string& file, Mesh& mesh);
void ReadPlyMeshFromFile(const std::string& file, Mesh& mesh);
void ReadObjMeshFromFile(const std::string& file, std::vector<Mesh>& mesh);
void ReadObjMeshFromFile(const std::string& file, Mesh& mesh);
void WritePlyMeshToFile(const std::string& file, const Mesh& mesh, bool binary =
		true);
void WriteMeshToFile(const std::string& file, const Mesh& mesh);
void WriteObjMeshToFile(const std::string& file, const Mesh& mesh);
typedef std::vector<std::unordered_set<uint32_t>> MeshSetNeighborTable;
typedef std::vector<std::vector<uint32_t>> MeshListNeighborTable;
void CreateVertexNeighborTable(const Mesh& mesh,
		MeshSetNeighborTable& vertNbrs);
void CreateOrderedVertexNeighborTable(const Mesh& mesh,
		MeshListNeighborTable& vertNbrs, bool leaveTail = false);
void CreateFaceNeighborTable(const Mesh& mesh, MeshListNeighborTable& faceNbrs);
//This is the canonical way to describe methods
inline void MakeVertexNeighborTable(const Mesh& mesh,
		MeshSetNeighborTable& vertNbrs) {
	CreateVertexNeighborTable(mesh, vertNbrs);
}
inline void MakeOrderedVertexNeighborTable(const Mesh& mesh,
		MeshListNeighborTable& vertNbrs, bool leaveTail = false) {
	CreateOrderedVertexNeighborTable(mesh, vertNbrs, leaveTail);
}
inline void MakeFaceNeighborTable(const Mesh& mesh,
		MeshListNeighborTable& faceNbrs) {
	CreateFaceNeighborTable(mesh, faceNbrs);
}
void Subdivide(Mesh& mesh, SubDivisionScheme type =
		SubDivisionScheme::CatmullClark);
}
#endif /* MESH_H_ */
