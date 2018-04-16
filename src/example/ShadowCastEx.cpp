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

#include <example/ShadowCastEx.h>
#include <AlloyMeshPrimitives.h>
#include "Alloy.h"
using namespace aly;
ShadowCastEx::ShadowCastEx() :
		Application(800, 600, "Shadow Cast Shader Example"), phongShader(1) {
	lightWidth=1024;
	lightHeight=1024;
}
bool ShadowCastEx::init(Composite& rootNode) {
	box3f renderBBox = box3f(float3(-0.5f, -0.5f, -0.5f),float3(1.0f, 1.0f, 1.0f));

	monkey.load(getFullPath("models/monkey.ply"));
	Subdivide(monkey,SubDivisionScheme::Loop);
	monkey.updateVertexNormals();
	//Make region on screen to render 3d view
	renderRegion=MakeRegion("Render View",CoordPX(0.0f,0.0f),CoordPercent(1.0f,1.0f),COLOR_NONE,COLOR_WHITE,UnitPX(1.0f));
	//Initialize depth buffer to store the render
	depthFrameBuffer.initialize(getContext()->getScreenWidth(),getContext()->getScreenHeight());
	colorFrameBuffer.initialize(getContext()->getScreenWidth(),getContext()->getScreenHeight());
	positionFrameBuffer.initialize(getContext()->getScreenWidth(),getContext()->getScreenHeight());
	lightDepthBuffer.initialize(lightWidth,lightHeight);
	//Set up camera
	camera.setNearFarPlanes(0.1f, 10.0f);
	camera.setZoom(0.6f);
	camera.rotateModelY(ToRadians(-40.0f));
	camera.rotateModelX(ToRadians(35.0f));
	camera.setCameraType(CameraType::Perspective);
	camera.setDirty(true);

	lightCamera.setNearFarPlanes(0.1f, 10.0f);
	lightCamera.setZoom(0.2f);
	lightCamera.setFieldOfView(80.0f);
	lightCamera.rotateModelY(ToRadians(30.0f));
	lightCamera.rotateModelX(ToRadians(45.0f));
	lightCamera.setCameraType(CameraType::Perspective);
	lightCamera.setDirty(true);

	//Add listener to respond to mouse manipulations
	addListener(&camera);

	//Setup light. "w" channel used to configure contributions of different lighting terms.
	phongShader[0] = SimpleLight(Color(1.0f, 0.2f, 0.2f, 0.25f),
		Color(1.0f, 1.0f, 1.0f, 0.25f), Color(0.9f, 0.1f, 0.1f, 0.5f),
		Color(0.9f, 0.1f, 0.1f, 0.5f), 16.0f, float3(0, 0.0, 2.0),
		float3(0, 1, 0));
	phongShader[0].moveWithCamera = false;

	wireframeShader.setFaceColor(Color(0.5f, 0.5f, 0.5f, 1.0f));
	wireframeShader.setEdgeColor(Color(0.8f, 0.8f, 0.8f, 1.0f));
	wireframeShader.setLineWidth(2.0f);
	wireframeShader.setSolid(true);
	Grid(3.0f,3.0f,4,4).clone(grid);
	grid.transform(MakeTranslation(float3(0.0f,-0.25f,0.0f))*MakeRotationX(ALY_PI*0.5f));
	monkey.transform(MakeTranslation(float3(0.0f,0.15f,0.25f))*MakeTransform(monkey.getBoundingBox(), renderBBox));
	//Add render component to root node so it is relatively positioned.
	rootNode.add(renderRegion);
	return true;
}
void ShadowCastEx::draw(AlloyContext* context){
	if (camera.isDirty()) {
		positionShader.draw({&grid,&monkey}, camera, positionFrameBuffer,false,true);
		depthAndNormalShader.draw({&grid,&monkey}, lightCamera, lightDepthBuffer,false);
		depthAndNormalShader.draw(monkey, camera, depthFrameBuffer);
		colorFrameBuffer.begin();
		wireframeShader.draw(grid,camera,colorFrameBuffer.getViewport());
		phongShader.draw(depthFrameBuffer.getTexture(), camera, colorFrameBuffer.getViewport(), getContext()->getViewport());
		colorFrameBuffer.end();
	}
	glClearColor(0.81f,0.9f,0.9f,1.0f);
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	shadowCastShader.draw(colorFrameBuffer.getTexture(),positionFrameBuffer.getTexture(),lightDepthBuffer.getTexture(),camera,lightCamera,3.0f,0.01f,0.8f,64);

	camera.setDirty(false);
}

