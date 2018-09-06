/*
* Copyright(C) 2016, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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
#ifndef INCLUDE_Manifold3D_H_
#define INCLUDE_Manifold3D_H_
#include "GLComponent.h"
#include "AlloyVector.h"
#include "AlloyContext.h"
#include "AlloyFileUtil.h"
#include "AlloyMesh.h"
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/list.hpp>
#include <cereal/types/array.hpp>
#include <AlloyLocator.h>
namespace aly {
	class AlloyContext;
	struct Orphan3D {
		float3 particle;
		float3 normal;
		float3 correspondence;
		Orphan3D(float3 pt,float3 norm,float3 correspondence):particle(pt),normal(norm),correspondence(correspondence){
		}
	};
	class Breadcrumbs3D{
	protected:
		std::vector<std::vector<float3i>> history;
	public:
		template<class Archive> void save(Archive & archive) const {
			archive(CEREAL_NVP(history));
		}
		template<class Archive> void load(Archive & archive) {
			archive(CEREAL_NVP(history));
		}
		const float3i& operator()(const size_t t,const size_t i) const ;
		float3i& operator()(const size_t t,const size_t i);
		void clear();
		std::vector<float3i>& addTime(size_t sz);
		std::vector<float3i>& addTime(const Vector3f& current);
		const std::vector<float3i>& getTime(size_t sz) const;
		std::vector<float3i>& getTime(size_t sz);
		std::vector<float3i>& getLastTime();
		size_t size() const;
		size_t size(size_t t) const;
	};
	class Manifold3D {
	protected:
		bool onScreen;
		std::shared_ptr<AlloyContext> context;
		GLuint vao;
		GLuint vertexBuffer;
		GLuint particleBuffer;
		GLuint labelBuffer;
		std::string file;
		bool dirty;
		int vertexCount;
	public:
		Vector3f vertexes;
		Vector3f normals;
		Vector4f colors;
		Vector3f particles;
		Vector4ui quadIndexes;
		Vector3ui triIndexes;
		Vector3f vertexLocations;
		Vector3f vertexNormals;
		std::vector<int> particleLabels;
		std::vector<int> vertexLabels;
		std::vector<int> particleTracking;
		Vector3f correspondence;
		std::array<Vector3f,4> velocities;
		aly::MeshType meshType;
		void setDirty(bool b) {
			dirty = b;
		}
		bool isDirty() const {
			return dirty;
		}
		void update();
		void draw();
		void stashSpringls(const std::string& file);
		void stashCorrespondence(const std::string& file);
		void stashIsoSurface(const std::string& mesh);
		std::string getFile() const {
			return file;
		}
		~Manifold3D();
		Manifold3D(bool onScreen=true,const std::shared_ptr<AlloyContext>& context=AlloyDefaultContext());
		void setFile(const std::string& file) {
			this->file = file;
		}
		template<class Archive> void save(Archive & archive) const {
			archive(CEREAL_NVP(file),CEREAL_NVP(meshType),CEREAL_NVP(vertexes),CEREAL_NVP(normals),CEREAL_NVP(colors),CEREAL_NVP(particles),CEREAL_NVP(particleTracking),CEREAL_NVP(vertexLocations),CEREAL_NVP(vertexNormals),CEREAL_NVP(triIndexes),CEREAL_NVP(quadIndexes),CEREAL_NVP(vertexLabels), CEREAL_NVP(particleLabels),CEREAL_NVP(correspondence),CEREAL_NVP(velocities));
		}
		template<class Archive> void load(Archive & archive) 
		{
			archive(CEREAL_NVP(file),CEREAL_NVP(meshType),CEREAL_NVP(vertexes),CEREAL_NVP(normals),CEREAL_NVP(colors), CEREAL_NVP(particles),CEREAL_NVP(particleTracking),CEREAL_NVP(vertexLocations),CEREAL_NVP(vertexNormals),CEREAL_NVP(triIndexes),CEREAL_NVP(quadIndexes),CEREAL_NVP(vertexLabels), CEREAL_NVP(particleLabels), CEREAL_NVP(correspondence),CEREAL_NVP(velocities));
		}
		void updateNormals();
		void operator=(const Manifold3D &c);
		Manifold3D(const Manifold3D& c);
	};
	void ReadContourFromFile(const std::string& file, Manifold3D& contour);
	void WriteContourToFile(const std::string& file, Manifold3D& contour);

}
#endif
