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

#include "Alloy.h"
#include "example/MeshDistanceFieldEx.h"
#include "graphics/AlloyMeshDistanceField.h"
#include "graphics/MeshDecimation.h"
using namespace aly;
MeshDistanceFieldEx::MeshDistanceFieldEx() :
		Application(800, 600, "Mesh Surface Distance Field Example"), matcapShader(
				getFullPath("images/JG_Silver.png")) {
}
bool MeshDistanceFieldEx::init(Composite& rootNode) {
	box3f renderBBox = box3f(float3(-0.5f, -0.5f, -0.5f),
			float3(1.0f, 1.0f, 1.0f));

	mesh.load(getFullPath("models/armadillo.ply"));
	{
		MeshDecimation decimate;
		decimate.solve(mesh,0.3f);
		mesh.updateVertexNormals();
	}
	size_t N=mesh.vertexLocations.size();
	{
		AlloyMeshDistanceField df;
		std::vector<size_t> zeroSet(4);
		for(size_t& index:zeroSet){
			index=RandomUniform((int)0,(int)N);
		}
		std::vector<float> distanceField;
		float maxDist=df.solve(mesh,zeroSet,distanceField);
		Vector4f& colors=mesh.vertexColors;
		colors.resize(distanceField.size());
		for(size_t i=0;i<colors.size();i++){
			float d=clamp(distanceField[i]/maxDist,0.0f,1.0f);
			colors[i]=HSVAtoRGBAf(HSVA(0.98f*d,0.7f,aly::sqr(1.0f-d)*0.75f+0.25f,1.0f));
		}
	}
	box3f box = mesh.getBoundingBox();
	//Make region on screen to render 3d view
	renderRegion = MakeRegion("Render View", CoordPerPX(0.5f, 0.5f, -400, -300),
			CoordPX(800, 600), COLOR_NONE, COLOR_WHITE, UnitPX(1.0f));
	//Initialize depth buffer to store the render
	depthFrameBuffer.initialize(800,600);
	colorFrameBuffer.initialize(800,600);
	//Set up camera
	camera.setNearFarPlanes(-2.0f, 2.0f);
	camera.setZoom(0.75f);
	camera.setCameraType(CameraType::Orthographic);
	camera.setDirty(true);
	//Map object geometry into unit bounding box for draw.
	camera.setPose(MakeTransform(mesh.getBoundingBox(), renderBBox));
	//Add listener to respond to mouse manipulations
	addListener(&camera);
	//Add render component to root node so it is relatively positioned.
	rootNode.add(renderRegion);
	return true;
}
void MeshDistanceFieldEx::draw(AlloyContext* context) {
	if (camera.isDirty()) {
		//Compute depth and normals only when camera view changes.
		depthAndNormalShader.draw(mesh, camera, depthFrameBuffer);
		colorVertexShader.draw(mesh,camera,colorFrameBuffer);
	}
	//Recompute lighting at every draw pass.
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	matcapShader.draw(depthFrameBuffer.getTexture(), camera, context->pixelRatio*renderRegion->getBounds(), context->getViewport(), RGBAf(1.0f));
	imageShader.draw(colorFrameBuffer.getTexture(), context->pixelRatio*renderRegion->getBounds(), 0.5f, false);
	camera.setDirty(false);
}

