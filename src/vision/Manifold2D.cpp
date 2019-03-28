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
#include "vision/Manifold2D.h"
#include "system/AlloyFileUtil.h"
#include "ui/AlloyContext.h"
#include "common/cereal/archives/xml.hpp"
#include "common/cereal/archives/json.hpp"
#include "common/cereal/archives/portable_binary.hpp"
namespace aly {
const float2i& Breadcrumbs2D::operator()(const size_t t,const size_t i) const {
	return history[t][i];
}
float2i& Breadcrumbs2D::operator()(const size_t t,const size_t i){
	return history[t][i];
}
std::vector<float2i>& Breadcrumbs2D::addTime(size_t sz) {
	history.push_back(std::vector < float2i > (sz, float2i(float2(0.0f), -1)));
	return history.back();
}
std::vector<float2i>& Breadcrumbs2D::addTime(const Vector2f& current) {
	std::vector<float2i> next(current.size());
	for (int i = 0; i < current.size(); i++) {
		next[i] = float2i(current[i], i);
	}
	history.push_back(next);
	return history.back();
}
void Breadcrumbs2D::clear(){
	history.clear();
}
const std::vector<float2i>& Breadcrumbs2D::getTime(size_t sz) const {
	return history[sz];
}
std::vector<float2i>& Breadcrumbs2D::getTime(size_t sz) {
	return history[sz];
}
std::vector<float2i>& Breadcrumbs2D::getLastTime() {
	return history.back();
}
size_t Breadcrumbs2D::size() const {
	return history.size();
}
size_t Breadcrumbs2D::size(size_t t) const {
	return history[t].size();
}
void Manifold2D::operator=(const Manifold2D &c) {
	onScreen = c.onScreen;
	context = c.context;
	clusterColors = c.clusterColors;
	clusterCenters = c.clusterCenters;
	indexes = c.indexes;
	vertexLocations = c.vertexLocations;
	particles = c.particles;
	vertexes = c.vertexes;
	normals = c.normals;
	velocities = c.velocities;
	correspondence = c.correspondence;
	vertexLabels = c.vertexLabels;
	file = c.file;
	overlay = c.overlay;
	particleLabels = c.particleLabels;
	fluidParticles = c.fluidParticles;

}
Manifold2D::Manifold2D(const Manifold2D& c) :
		vao(0), vertexBuffer(0), particleBuffer(0), labelBuffer(0), dirty(
				false), vertexCount(0) {
	onScreen = c.onScreen;
	context = c.context;
	clusterColors = c.clusterColors;
	clusterCenters = c.clusterCenters;
	indexes = c.indexes;
	vertexLocations = c.vertexLocations;
	particles = c.particles;
	vertexes = c.vertexes;
	normals = c.normals;
	velocities = c.velocities;
	correspondence = c.correspondence;
	vertexLabels = c.vertexLabels;
	file = c.file;
	overlay = c.overlay;
	particleLabels = c.particleLabels;
	fluidParticles = c.fluidParticles;

}
Manifold2D::Manifold2D(bool onScreen,
		const std::shared_ptr<AlloyContext>& context) :
		onScreen(onScreen), context(context), vao(0), vertexBuffer(0), particleBuffer(
				0), labelBuffer(0), dirty(false), vertexCount(0) {
}
Manifold2D::~Manifold2D() {
	if (vao != 0) {
		context->begin();
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
void Manifold2D::draw() {
	if (dirty) {
		update();
	}
	context->begin();
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
			glDrawArrays(GL_POINTS, 0, (GLsizei) (vertexes.size() / 2));
			glDisableVertexAttribArray(2);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
	}
	glBindVertexArray(0);
	context->end();
}
void Manifold2D::update() {
	context->begin();
	if (vao == 0)
		glGenVertexArrays(1, &vao);
	if (vertexes.size() > 0) {
		if (vertexCount != (int) vertexes.size()) {
			if (glIsBuffer(vertexBuffer) == GL_TRUE)
				glDeleteBuffers(1, &vertexBuffer);
			glGenBuffers(1, &vertexBuffer);
		}
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		if (glIsBuffer(vertexBuffer) == GL_FALSE)
			throw std::runtime_error("Error: Unable to create vertex buffer");
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * vertexes.size(),
				vertexes.ptr(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		vertexCount = (int) vertexes.size();
	} else {
		if (glIsBuffer(vertexBuffer) == GL_TRUE)
			glDeleteBuffers(1, &vertexBuffer);
		vertexCount = 0;
	}

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

void Manifold2D::updateNormals() {
	normals.resize(vertexes.size() / 2);
#pragma omp parallel for
	for (int i = 0; i < (int) vertexes.size(); i += 2) {
		float2 norm = normalize(vertexes[i + 1] - vertexes[i]);
		normals[i / 2] = float2(-norm.y, norm.x);
	}
}
void ReadContourFromFile(const std::string& file, Manifold2D& params) {
	std::string ext = GetFileExtension(file);
	if (ext == "json") {
		std::ifstream os(file);
		cereal::JSONInputArchive archive(os);
		archive(cereal::make_nvp("contour", params));
	} else if (ext == "xml") {
		std::ifstream os(file);
		cereal::XMLInputArchive archive(os);
		archive(cereal::make_nvp("contour", params));
	} else {
		std::ifstream os(file, std::ios::binary);
		cereal::PortableBinaryInputArchive archive(os);
		archive(cereal::make_nvp("contour", params));
	}
}
void WriteContourToFile(const std::string& file, Manifold2D& params) {
	params.setFile(file);
	std::string ext = GetFileExtension(file);
	if (ext == "json") {
		std::ofstream os(file);
		cereal::JSONOutputArchive archive(os);
		archive(cereal::make_nvp("contour", params));
	} else if (ext == "xml") {
		std::ofstream os(file);
		cereal::XMLOutputArchive archive(os);
		archive(cereal::make_nvp("contour", params));
	} else {
		std::ofstream os(file, std::ios::binary);
		cereal::PortableBinaryOutputArchive archive(os);
		archive(cereal::make_nvp("contour", params));
	}
	params.setFile(file);
}

}
