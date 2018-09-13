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
#include "ui/AlloyContext.h"
#include "vision/Manifold3D.h"
#include "system/AlloyFileUtil.h"
#include "common/cereal/archives/xml.hpp"
#include "common/cereal/archives/json.hpp"
#include "common/cereal/archives/portable_binary.hpp"

namespace aly {

void ReadContourFromFile(const std::string& file, Manifold3D& params) {
	std::string ext = GetFileExtension(file);
	if (ext == "json") {
		std::ifstream os(file);
		cereal::JSONInputArchive archive(os);
		archive(cereal::make_nvp("surface", params));
	} else if (ext == "xml") {
		std::ifstream os(file);
		cereal::XMLInputArchive archive(os);
		archive(cereal::make_nvp("surface", params));
	} else {
		std::ifstream os(file, std::ios::binary);
		cereal::PortableBinaryInputArchive archive(os);
		archive(cereal::make_nvp("surface", params));
	}
}

void WriteContourToFile(const std::string& file, Manifold3D& params) {
	params.setFile(file);
	std::string ext = GetFileExtension(file);
	if (ext == "json") {
		std::ofstream os(file);
		cereal::JSONOutputArchive archive(os);
		archive(cereal::make_nvp("surface", params));
	} else if (ext == "xml") {
		std::ofstream os(file);
		cereal::XMLOutputArchive archive(os);
		archive(cereal::make_nvp("surface", params));
	} else {
		std::ofstream os(file, std::ios::binary);
		cereal::PortableBinaryOutputArchive archive(os);
		archive(cereal::make_nvp("surface", params));
	}
	params.setFile(file);
}
const float3i& Breadcrumbs3D::operator()(const size_t t,const size_t i) const {
	return history[t][i];
}
float3i& Breadcrumbs3D::operator()(const size_t t,const size_t i){
	return history[t][i];
}
std::vector<float3i>& Breadcrumbs3D::addTime(size_t sz) {
	history.push_back(std::vector < float3i > (sz, float3i(float3(0.0f), -1)));
	return history.back();
}
std::vector<float3i>& Breadcrumbs3D::addTime(const Vector3f& current) {
	std::vector<float3i> next(current.size());
	for (int i = 0; i < current.size(); i++) {
		next[i] = float3i(current[i], i);
	}
	history.push_back(next);
	return history.back();
}
void Breadcrumbs3D::clear(){
	history.clear();
}
const std::vector<float3i>& Breadcrumbs3D::getTime(size_t sz) const {
	return history[sz];
}
std::vector<float3i>& Breadcrumbs3D::getTime(size_t sz) {
	return history[sz];
}
std::vector<float3i>& Breadcrumbs3D::getLastTime() {
	return history.back();
}
size_t Breadcrumbs3D::size() const {
	return history.size();
}
size_t Breadcrumbs3D::size(size_t t) const {
	return history[t].size();
}
void Manifold3D::operator=(const Manifold3D &c) {
	onScreen = c.onScreen;
	context = c.context;
	particles = c.particles;
	correspondence = c.correspondence;
	vertexLabels = c.vertexLabels;
	vertexNormals=c.vertexNormals;
	vertexLocations=c.vertexLocations;
	meshType=c.meshType;
	colors=c.colors;
	vertexes=c.vertexes;
	normals=c.normals;
	triIndexes=c.triIndexes;
	quadIndexes=c.quadIndexes;
	particleTracking=c.particleTracking;
	file = c.file;
	particleLabels = c.particleLabels;
}
Manifold3D::Manifold3D(const Manifold3D& c) :
		vao(0), vertexBuffer(0), particleBuffer(0), labelBuffer(0), dirty(
				false), vertexCount(0) {
	onScreen = c.onScreen;
	context = c.context;
	particles = c.particles;
	correspondence = c.correspondence;
	vertexLabels = c.vertexLabels;
	vertexNormals=c.vertexNormals;
	vertexLocations=c.vertexLocations;
	colors=c.colors;
	vertexes=c.vertexes;
	normals=c.normals;
	meshType=c.meshType;
	triIndexes=c.triIndexes;
	quadIndexes=c.quadIndexes;
	particleTracking=c.particleTracking;
	file = c.file;
	particleLabels = c.particleLabels;
}
void Manifold3D::updateNormals() {
	int N=(int)vertexes.size();
	normals.resize(particles.size());
	if(meshType==MeshType::Triangle){
#pragma omp parallel for
		for (int i = 0; i < N; i+=3) {
			float3 norm;
			float3 v1 = vertexes[i];
			float3 v2 = vertexes[i+1];
			float3 v3 = vertexes[i+2];
			float3 p=particles[i/3];
			norm=   cross((v1 - p),(v2 - p));
			norm += cross((v2 - p),(v3 - p));
			norm += cross((v3 - p),(v1 - p));
			normals[i/3] = normalize(norm);
		}
	} else if(meshType==MeshType::Quad){
#pragma omp parallel for
		for (int i = 0; i < N; i+=4) {
			float3 norm;
			float3 v1 = vertexes[i];
			float3 v2 = vertexes[i+1];
			float3 v3 = vertexes[i+2];
			float3 v4 = vertexes[i+3];
			float3 p=particles[i/4];
			norm=   cross((v1 - p),(v2 - p));
			norm += cross((v2 - p),(v3 - p));
			norm += cross((v3 - p),(v4 - p));
			norm += cross((v4 - p),(v1 - p));
			normals[i/4] = normalize(norm);
		}
	}
}
Manifold3D::Manifold3D(bool onScreen,
		const std::shared_ptr<AlloyContext>& context) :
		onScreen(onScreen), context(context), vao(0), vertexBuffer(0), particleBuffer(
				0), labelBuffer(0), dirty(false), vertexCount(0),meshType(MeshType::Triangle) {
}
Manifold3D::~Manifold3D() {
	if (vao != 0) {
		context->begin(onScreen);
		if (glIsBuffer(vertexBuffer) == GL_TRUE)
			glDeleteBuffers(1, &vertexBuffer);
		if (glIsBuffer(particleBuffer) == GL_TRUE)
			glDeleteBuffers(1, &particleBuffer);
		if (glIsBuffer(labelBuffer) == GL_TRUE)
			glDeleteBuffers(1, &labelBuffer);
		glDeleteVertexArrays(1, &vao);
		context->end();
	}
}
void Manifold3D::stashCorrespondence(const std::string& file){
	Mesh mesh;
	mesh.vertexLocations=correspondence;
	WriteMeshToFile(file,mesh);
}
void Manifold3D::stashSpringls(const std::string& file){
	Mesh mesh;
	size_t N=particles.size();
	if(meshType==MeshType::Quad){
		mesh.quadIndexes.resize(N);
		for(size_t n=0;n<N;n++){
			mesh.quadIndexes[n]=uint4(n*4,n*4+1,n*4+2,n*4+3);
		}
	} else {
		mesh.triIndexes.resize(N);
		for(size_t n=0;n<N;n++){
			mesh.triIndexes[n]=uint3(n*3,n*3+1,n*3+2);
		}
	}
	mesh.vertexLocations=vertexes;
	//std::cout<<"Springls:\n"<<mesh<<std::endl;
	WriteMeshToFile(file,mesh);
}
void Manifold3D::stashIsoSurface(const std::string& file){
	aly::Mesh mesh;
	mesh.vertexLocations=vertexLocations;
	mesh.triIndexes=triIndexes;
	mesh.quadIndexes=quadIndexes;
	//std::cout<<"Iso-Surface:\n"<<mesh<<std::endl;
	WriteMeshToFile(file,mesh);
}
void Manifold3D::draw() {
	if (dirty) {
		update();
	}
	context->begin(onScreen);
	if (vao > 0) {
		glBindVertexArray(vao);
		if (vertexBuffer > 0) {
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
			if (particleBuffer > 0) {
				glEnableVertexAttribArray(1);
				glBindBuffer(GL_ARRAY_BUFFER, particleBuffer);
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
			}
			if (labelBuffer > 0) {
				glEnableVertexAttribArray(2);
				glBindBuffer(GL_ARRAY_BUFFER, labelBuffer);
				glVertexAttribPointer(2, 1, GL_INT, GL_FALSE, 0, 0);
			}
			glDrawArrays(GL_POINTS, 0, (GLsizei) (particles.size() / 2));
			glDisableVertexAttribArray(2);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
	}
	glBindVertexArray(0);
	context->end();
}
void Manifold3D::update() {
	context->begin(onScreen);
	if (vao == 0)
		glGenVertexArrays(1, &vao);
	if (particles.size() > 0) {
		if (vertexCount != (int) particles.size()) {
			if (glIsBuffer(particleBuffer) == GL_TRUE)
				glDeleteBuffers(1, &particleBuffer);
			glGenBuffers(1, &particleBuffer);
		}
		glBindBuffer(GL_ARRAY_BUFFER, particleBuffer);
		if (glIsBuffer(particleBuffer) == GL_FALSE)
			throw std::runtime_error("Error: Unable to create vertex buffer");
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * particles.size(),
				particles.ptr(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	} else {
		if (glIsBuffer(particleBuffer) == GL_TRUE)
			glDeleteBuffers(1, &particleBuffer);
	}

	if (particleLabels.size() > 0) {
		if (vertexCount != (int) particleLabels.size()) {
			if (glIsBuffer(labelBuffer) == GL_TRUE)
				glDeleteBuffers(1, &labelBuffer);
			glGenBuffers(1, &labelBuffer);
		}
		glBindBuffer(GL_ARRAY_BUFFER, labelBuffer);
		if (glIsBuffer(labelBuffer) == GL_FALSE)
			throw std::runtime_error("Error: Unable to create label buffer");
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLint) * particleLabels.size(),
				particleLabels.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	} else {
		if (glIsBuffer(labelBuffer) == GL_TRUE)
			glDeleteBuffers(1, &labelBuffer);
	}
	context->end();
	dirty = false;
}

}
