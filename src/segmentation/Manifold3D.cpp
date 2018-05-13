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
#include "segmentation/Manifold3D.h"
#include "AlloyFileUtil.h"
#include "AlloyContext.h"
#include <cereal/archives/xml.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>
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

void Manifold3D::operator=(const Manifold3D &c) {
	onScreen = c.onScreen;
	context = c.context;
	particles = c.particles;
	correspondence = c.correspondence;
	vertexLabels = c.vertexLabels;
	vertexColors=c.vertexColors;
	vertexLocations=c.vertexLocations;
	vertexNormals=c.vertexNormals;
	triIndexes=c.triIndexes;
	quadIndexes=c.quadIndexes;
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
	vertexColors=c.vertexColors;
	vertexLocations=c.vertexLocations;
	vertexNormals=c.vertexNormals;
	triIndexes=c.triIndexes;
	quadIndexes=c.quadIndexes;
	file = c.file;
	particleLabels = c.particleLabels;
}
Manifold3D::Manifold3D(bool onScreen,
		const std::shared_ptr<AlloyContext>& context) :
		onScreen(onScreen), context(context), vao(0), vertexBuffer(0), particleBuffer(
				0), labelBuffer(0), dirty(false), vertexCount(0) {
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
				particleLabels.ptr(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	} else {
		if (glIsBuffer(labelBuffer) == GL_TRUE)
			glDeleteBuffers(1, &labelBuffer);
	}
	context->end();
	dirty = false;
}

}
