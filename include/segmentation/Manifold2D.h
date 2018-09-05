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
#ifndef INCLUDE_Manifold2D_H_
#define INCLUDE_Manifold2D_H_
#include "GLComponent.h"
#include "AlloyVector.h"
#include "AlloyContext.h"
#include "AlloyFileUtil.h"
#include "segmentation/FluidParticles2D.h"
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/list.hpp>
#include <array>
#include <AlloyLocator.h>
namespace aly {
	class AlloyContext;
	class Breadcrumbs2D{
	protected:
		std::vector<std::vector<float2i>> history;
	public:
		template<class Archive> void save(Archive & archive) const {
			archive(CEREAL_NVP(history));
		}
		template<class Archive> void load(Archive & archive) {
			archive(CEREAL_NVP(history));
		}
		const float2i& operator()(const size_t t,const size_t i) const {
			return history[t][i];
		}
		float2i& operator()(const size_t t,const size_t i) {
			return history[t][i];
		}
		std::vector<float2i>& addTime(size_t sz){
			history.push_back(std::vector<float2i>(sz,float2i(float2(0.0f),-1)));
			return history.back();
		}
		std::vector<float2i>& addTime(const Vector2f& current){
			std::vector<float2i> next(current.size());
			for(int i=0;i<current.size();i++){
				next[i]=float2i(current,i);
			}
			history.push_back(next);
			return history.back();
		}
		const std::vector<float2i>& getTime(size_t sz) const {
			return history[sz];
		}
		std::vector<float2i>& getTime(size_t sz){
			return history[sz];
		}
		std::vector<float2i>& getLastTime(){
			return history.back();
		}
		size_t size() const {
			return history.size();
		}
		size_t size(size_t t) const {
			return history[t].size();
		}
	};
	class Manifold2D {
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
		Breadcrumbs2D crumbs;
		std::vector<std::vector<uint32_t>> indexes;
		Vector2f vertexLocations;
		Vector2f particles;
		Vector2f vertexes;
		Vector2f normals;
		Vector1i particleLabels;
		Vector1i vertexLabels;
		ImageRGBA overlay;
		FluidParticles2D fluidParticles;
		std::array<Vector2f,4> velocities;
		Vector2f correspondence;
		Vector2f clusterCenters;
		Vector3f clusterColors;
		void setDirty(bool b) {
			dirty = b;
		}
		bool isDirty() const {
			return dirty;
		}
		void update();
		void draw();
		std::string getFile() const {
			return file;
		}
		~Manifold2D();
		Manifold2D(bool onScreen=true,const std::shared_ptr<AlloyContext>& context=AlloyDefaultContext());
		void updateNormals();
		void setFile(const std::string& file) {
			this->file = file;
		}
		template<class Archive> void save(Archive & archive) const {
			if (overlay.size() > 0) {
				std::string imageFile = GetFileWithoutExtension(file) + ".png";
				WriteImageToFile(imageFile, overlay);
			}
			archive( CEREAL_NVP(vertexLocations),CEREAL_NVP(file), CEREAL_NVP(indexes), CEREAL_NVP(particles), CEREAL_NVP(vertexes),  CEREAL_NVP(normals), CEREAL_NVP(vertexLabels), CEREAL_NVP(particleLabels),CEREAL_NVP(correspondence), CEREAL_NVP(clusterCenters),CEREAL_NVP(clusterColors),CEREAL_NVP(fluidParticles));
		}
		template<class Archive> void load(Archive & archive) 
		{
			archive(CEREAL_NVP(vertexLocations), CEREAL_NVP(file), CEREAL_NVP(indexes), CEREAL_NVP(particles), CEREAL_NVP(vertexes), CEREAL_NVP(normals), CEREAL_NVP(vertexLabels), CEREAL_NVP(particleLabels), CEREAL_NVP(correspondence),CEREAL_NVP(clusterCenters), CEREAL_NVP(clusterColors),CEREAL_NVP(fluidParticles));
			std::string imageFile = GetFileWithoutExtension(file) + ".png";
			if (FileExists(imageFile)) {
				ReadImageFromFile(imageFile, overlay);
			}
		}
		void operator=(const Manifold2D &c);
		Manifold2D(const Manifold2D& c);
	};
	void ReadContourFromFile(const std::string& file, Manifold2D& contour);
	void WriteContourToFile(const std::string& file, Manifold2D& contour);

}
#endif
