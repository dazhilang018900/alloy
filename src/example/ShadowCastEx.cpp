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

#include "example/ShadowCastEx.h"
#include "graphics/AlloyMeshPrimitives.h"
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
	renderRegion=MakeComposite("Render View",CoordPX(0.0f,0.0f),CoordPercent(1.0f,1.0f),COLOR_NONE,COLOR_WHITE,UnitPX(1.0f));
	spread=Float(2.0f);
	samples=Integer(64);
	pitch=Integer(40);
	angle=Integer(33);

	pitchSlider = HorizontalSliderPtr(
			new HorizontalSlider("Zenith Angle", CoordPX(5.0f, 5.0f),
					CoordPX(200.0f, 45.0f), true, Integer(5), Integer(90),
					pitch));
	angleSlider = HorizontalSliderPtr(
			new HorizontalSlider("Azimuth Angle", CoordPX(5.0f, 55.0f),
					CoordPX(200.0f, 45.0f), true, Integer(-180), Integer(180),
					angle));
	samplesSlider = HorizontalSliderPtr(
			new HorizontalSlider("Samples", CoordPX(5.0f, 105.0f),
					CoordPX(200.0f, 45.0f), true, Integer(1), Integer(256),
					samples));
	spreadSlider = HorizontalSliderPtr(
			new HorizontalSlider("Spread", CoordPX(5.0f, 155.0f),
					CoordPX(200.0f, 45.0f), true, Float(0.0f), Float(8.0f),
					spread));

	pitchSlider->backgroundColor = MakeColor(
			getContext()->theme.DARK.toSemiTransparent(0.5f));
	angleSlider->backgroundColor = MakeColor(
			getContext()->theme.DARK.toSemiTransparent(0.5f));

	samplesSlider->backgroundColor = MakeColor(
			getContext()->theme.DARK.toSemiTransparent(0.5f));
	spreadSlider->backgroundColor = MakeColor(
			getContext()->theme.DARK.toSemiTransparent(0.5f));

	renderRegion->add(pitchSlider);
	renderRegion->add(angleSlider);
	renderRegion->add(samplesSlider);
	renderRegion->add(spreadSlider);
	pitchSlider->setOnChangeEvent(
			[this](const Number& val) {
				lightCamera.setModelRotation(float4x4::identity());
				lightCamera.rotateModelY(ToRadians(angle.toFloat()));
				lightCamera.rotateModelX(ToRadians(pitch.toFloat()));

			});
	angleSlider->setOnChangeEvent(
			[this](const Number& val) {
				lightCamera.setModelRotation(float4x4::identity());
				lightCamera.rotateModelY(ToRadians(angle.toFloat()));
				lightCamera.rotateModelX(ToRadians(pitch.toFloat()));

		});
	//Initialize depth buffer to store the render
	depthFrameBuffer.initialize(getContext()->getScreenWidth(),getContext()->getScreenHeight());
	colorFrameBuffer.initialize(getContext()->getScreenWidth(),getContext()->getScreenHeight());
	positionFrameBuffer.initialize(getContext()->getScreenWidth(),getContext()->getScreenHeight());
	lightDepthBuffer.initialize(lightWidth,lightHeight);
	//Set up camera
	camera.setNearFarPlanes(0.1f, 10.0f);
	camera.setZoom(0.3f);
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
	monkey.transform(MakeTranslation(float3(0.0f,0.15f,0.5f))*MakeTransform(monkey.getBoundingBox(), renderBBox));
	//Add render component to root node so it is relatively positioned.
	rootNode.add(renderRegion);
	return true;
}
void ShadowCastEx::draw(AlloyContext* context){
	if (camera.isDirty()||lightCamera.isDirty()) {
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
	shadowCastShader.draw(colorFrameBuffer.getTexture(),positionFrameBuffer.getTexture(),lightDepthBuffer.getTexture(),camera,lightCamera,spread.toFloat(),0.005f,0.8f,samples.toInteger());
	camera.setDirty(false);
	lightCamera.setDirty(false);
}

