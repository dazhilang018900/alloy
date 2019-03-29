/*
* Copyright(C) 2017, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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
*
*      Author: Blake Lucas
*/
#ifndef POINTSHADER_H_
#define POINTSHADER_H_
#include "graphics/GLShader.h"
#include "graphics/GLFrameBuffer.h"
namespace aly {

class PointColorShader: public GLShader {
public:
	PointColorShader( const std::shared_ptr<AlloyContext>& context =AlloyDefaultContext());
	void draw(Mesh* mesh, CameraParameters& camera, const box2px& bounds,const box2px& viewport,float scale=1.0f,bool twoSided=false);
};
class PointDepthShader: public GLShader {
public:
	PointDepthShader(const std::shared_ptr<AlloyContext>& context =AlloyDefaultContext());
	void draw(Mesh* mesh, CameraParameters& camera, const box2px& bounds,const box2px& viewport,float scale=1.0f,bool twoSided=false);
};
class PointPositionShader: public GLShader {
public:
	PointPositionShader( const std::shared_ptr<AlloyContext>& context =AlloyDefaultContext());
	void draw(Mesh* mesh, CameraParameters& camera, const box2px& bounds,const box2px& viewport,float scale=1.0f,bool twoSided=false);
};
class PointFaceIdShader: public GLShader {
public:
	PointFaceIdShader(const std::shared_ptr<AlloyContext>& context =AlloyDefaultContext());
	void draw(Mesh* mesh, CameraParameters& camera, const box2px& bounds,const box2px& viewport,float scale=1.0f,bool twoSided=false);
	void read(GLFrameBuffer& framebuffer,Image1i& faceIdMap);
};
} /* namespace intel */

#endif /* PointSHADER_H_ */
