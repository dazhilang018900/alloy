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

#ifndef SHADOWCAST_EX_H_
#define SHADOWCAST_EX_H_

#include "AlloyApplication.h"
#include "CommonShaders.h"
#include "ShadowCastShader.h"
class ShadowCastEx: public aly::Application {
protected:
	int lightWidth,lightHeight;
	aly::RegionPtr renderRegion;
	aly::GLFrameBuffer depthFrameBuffer;
	aly::GLFrameBuffer positionFrameBuffer;
	aly::GLFrameBuffer lightDepthBuffer;
	aly::GLFrameBuffer colorFrameBuffer;
	aly::ImageShader imageShader;
	aly::DepthAndNormalShader depthAndNormalShader;
	aly::MeshPositionShader positionShader;
	aly::PhongShader phongShader;
	aly::WireframeShader wireframeShader;
	aly::ShadowCastShader shadowCastShader;
	aly::Mesh monkey;
	aly::Mesh grid;
	aly::Camera camera;
	aly::Camera lightCamera;
public:
	ShadowCastEx();
	bool init(aly::Composite& rootNode);
	void draw(aly::AlloyContext* context);
};

#endif 
