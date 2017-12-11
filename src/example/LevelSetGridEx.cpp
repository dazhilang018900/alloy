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
 */

#include "Alloy.h"
#include "AlloyReconstruction.h"
#include "AlloyVolume.h"
#include "AlloyIsoSurface.h"
#include "grid/EndlessGrid.h"
#include "../../include/example/LevelSetGridEx.h"
using namespace aly;
LevelSetGridEx::LevelSetGridEx() :
		Application(1200, 800, "Endless Grid Example") {

}
bool LevelSetGridEx::init(Composite& rootNode) {
	srand((unsigned int) time(nullptr));
	//mesh.load(getFullPath("models/eagle.ply"));

	mesh.load(getFullPath("models/horse.ply"));

	/*
	std::chrono::steady_clock::time_point lastTime;
	std::chrono::steady_clock::time_point currentTime;
	IsoSurface isosurf;
	double elapsed;
	std::string voxelFile=MakeString() << GetDesktopDirectory() << ALY_PATH_SEPARATOR<<"points.xml";
	box3f bbox=mesh.getBoundingBox();
	float res=std::max(std::max(bbox.dimensions.x,bbox.dimensions.y),bbox.dimensions.z)/512;

	std::cout<<"Points to level set ..."<<std::endl;
	float4x4 T=PointsToLevelSet(mesh, grid, res,3.0f*res);

	std::cout<<"Write grid to file ..."<<std::endl;
	WriteGridToFile(voxelFile,grid);
	isosurf.solve(grid,mesh,MeshType::TRIANGLE,false,0.0f);

	std::string meshFile = MakeString() << GetDesktopDirectory()<< ALY_PATH_SEPARATOR<<"points_tri.ply";
	WriteMeshToFile(meshFile,mesh);

	isosurf.solve(grid,mesh,MeshType::QUAD,false,0.0f);
	meshFile = MakeString() << GetDesktopDirectory()<< ALY_PATH_SEPARATOR<<"points_quad.ply";
	WriteMeshToFile(meshFile,mesh);

	voxelFile=MakeString() << GetDesktopDirectory() << ALY_PATH_SEPARATOR<<"signed.xml";
	std::cout<<"Convert to level set"<<std::endl;
	T=MeshToLevelSet(mesh, grid, 2.5f, true, 0.8f);
*/
	/*
	std::cout<<"Convert grid to dense block"<<std::endl;
	WriteGridToFile(voxelFile,grid);
	Volume1f data;
	voxelFile=MakeString() << GetDesktopDirectory() << ALY_PATH_SEPARATOR<<"signed_0_0_0.xml";
	ReadVolumeFromFile(voxelFile, data);
	std::vector<int3> narrowBandList;
	for (int z = 0; z < data.slices; z++) {
		for (int y = 0; y < data.cols; y++) {
			for (int x = 0; x < data.rows; x++) {
				float val = data(x, y, z).x;
				if (std::abs(val) < 1.75f) {
					narrowBandList.push_back(int3(x, y, z));
				}
			}
		}
	}
	*/

	objectBBox = mesh.getBoundingBox();
	displayIndex = 0;
	colorReconstruction = false;
	parametersDirty = true;
	frameBuffersDirty = true;
	renderFrameBuffer.reset(new GLFrameBuffer());
	pointColorFrameBuffer.reset(new GLFrameBuffer());
	pointCloudDepthBuffer.reset(new GLFrameBuffer());
	depthFrameBuffer.reset(new GLFrameBuffer());
	wireframeFrameBuffer.reset(new GLFrameBuffer());
	colorFrameBuffer.reset(new GLFrameBuffer());
	compositeShader.reset(new CompositeShader());
	depthAndNormalShader.reset(new DepthAndNormalShader());
	wireframeShader.reset(new WireframeShader());
	particleDepthShader.reset(new ParticleDepthShader());
	particleMatcapShader.reset(new ParticleMatcapShader());
	matcapShader.reset(new MatcapShader());
	imageShader.reset(new ImageShader());
	colorVertexShader.reset(new ColorVertexShader());
	matcapImageFile = getFullPath("images/JG_Silver.png");
	box3f renderBBox = box3f(float3(-0.5f, -0.5f, -0.5f),
			float3(1.0f, 1.0f, 1.0f));
	BorderCompositePtr layout = BorderCompositePtr(
			new BorderComposite("UI Layout", CoordPX(0.0f, 0.0f),
					CoordPercent(1.0f, 1.0f), false));
	ParameterPanePtr controls = ParameterPanePtr(
			new ParameterPane("Controls", CoordPX(0.0f, 0.0f),
					CoordPercent(1.0f, 1.0f)));
	CompositePtr buttons = CompositePtr(
			new Composite("Buttons", CoordPX(0.0f, 0.0f),
					CoordPercent(1.0f, 1.0f)));
	TextIconButtonPtr executeButton = TextIconButtonPtr(
			new TextIconButton("Execute", 0xf085,
					CoordPerPX(0.5f, 0.5f, -120.0f, -30.0f),
					CoordPX(240.0f, 60.0f)));
	BorderCompositePtr controlLayout = BorderCompositePtr(
			new BorderComposite("Control Layout", CoordPX(0.0f, 0.0f),
					CoordPercent(1.0f, 1.0f), true));
	textLabel = TextLabelPtr(
			new TextLabel("", CoordPX(5, 5),
					CoordPerPX(1.0f, 0.0f, -10.0f, 30.0f)));
	textLabel->fontSize = UnitPX(24.0f);
	textLabel->fontType = FontType::Bold;
	textLabel->fontStyle = FontStyle::Outline;
	buttons->add(executeButton);
	executeButton->setRoundCorners(true);
	executeButton->onMouseDown =
			[this](aly::AlloyContext* context, const InputEvent& e) {
				if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
					context->addDeferredTask([this]() {
								solve();
							});
					return true;
				}
				return false;
			};
	controls->onChange =
			[this](const std::string& label, const AnyInterface& value) {
				parametersDirty = true;
			};
	float aspect = 6.0f;

	lineColor = Color(32, 32, 32, 128);
	faceColor = Color(255, 255, 255);

	displayIndex = 0;
	lineWidth = Float(1.0f);

	controls->setAlwaysShowVerticalScrollBar(false);
	controls->setScrollEnabled(false);
	controls->backgroundColor = MakeColor(getContext()->theme.DARKER);
	controlLayout->backgroundColor = MakeColor(getContext()->theme.DARKER);
	controlLayout->borderWidth = UnitPX(1.0f);
	controlLayout->borderColor = MakeColor(getContext()->theme.LIGHT);
	renderRegion = CompositePtr(
			new Composite("Render View", CoordPX(0.0f, 0.0f),
					CoordPercent(1.0f, 1.0f)));
	renderRegion->add(textLabel);
	layout->setWest(controlLayout, UnitPX(400.0f));
	controlLayout->setCenter(controls);
	controlLayout->setSouth(buttons, UnitPX(80.0f));
	layout->setCenter(renderRegion);
	buttons->borderWidth = UnitPX(1.0f);
	buttons->borderColor = MakeColor(getContext()->theme.DARK);
	buttons->backgroundColor = MakeColor(getContext()->theme.DARKER);
	rootNode.add(layout);
	matcapShader->setTextureImage(getFullPath("images/JG_Silver.png"));
	particleMatcapShader->setTextureImage(getFullPath("images/JG_Silver.png"));
	camera.setNearFarPlanes(-2.0f, 2.0f);
	camera.setZoom(0.75f);
	camera.setCameraType(CameraType::Orthographic);
	controls->addGroup("Visualization", true);
	displayIndexField =
			controls->addSelectionField("Display", displayIndex,
					std::vector<std::string> { "Solid", "Solid & Wireframe",
							"Wireframe" }, aspect);
	controls->addSelectionField("Camera", cameraType, std::vector<std::string> {
			"Orthographic", "Perspective" }, aspect);
	lineWidthField = controls->addNumberField("Line Width", lineWidth,
			Float(0.5f), Float(5.0f), 5.5f);
	faceColorField = controls->addColorField("Face", faceColor);
	lineColorField = controls->addColorField("Line", lineColor);

	meshType=0;
	topoConfig=1;
	regularize=true;
	voxelScale=Float(0.75f);
	controls->addGroup("Grid Settings", true);
	controls->addSelectionField("Topology",topoConfig,std::vector<std::string>{"256","32/4/4","5/4/6/4"});
	controls->addSelectionField("Type",meshType,std::vector<std::string>{"Triangle","Quad"});
	controls->addNumberField("Voxel Scale",voxelScale,Float(0.25f),Float(2.0f));
	controls->addCheckBox("Regularize",regularize);

	ReconstructionParameters params;
	float4x4 MT = MakeTransform(objectBBox, renderBBox);
	camera.setPose(MT);
	addListener(&camera);
	renderRegion->onPack = [this]() {
		camera.setDirty(true);
		frameBuffersDirty = true;
	};
	camera.setActiveRegion(renderRegion.get(), false);
	wireframeShader->setFaceColor(Color(0.1f, 0.1f, 1.0f, 0.5f));
	wireframeShader->setEdgeColor(Color(1.0f, 0.8f, 0.1f, 1.0f));
	wireframeShader->setLineWidth(lineWidth.toFloat() * 8.0f);
	return true;
}
void LevelSetGridEx::solve() {
	parametersDirty = true;
	mesh.load(getFullPath("models/horse.ply"));
	 if (worker.get() != nullptr) {
	 worker->cancel();
	 } else {
	 worker.reset(
	 new WorkerTask(
	 [this]() {
		 	IsoSurface isosurf;
		 	std::vector<int> config;
			std::chrono::steady_clock::time_point lastTime=std::chrono::steady_clock::now();
			double elapsed;
		 	if(topoConfig==0){
		 		config={256};
		 	} else if(topoConfig==1){
		 		config={32,4,4};
		 	} else if(topoConfig==2){
		 		config={5,4,6,4};
		 	}
			EndlessGrid<float> grid(config, 0.0f);
			textLabel->setLabel("Converting mesh to level set...");
			float4x4 T=MeshToLevelSet(mesh,grid,2.5f,true,voxelScale.toFloat(),[this](const std::string& label,float progress){
				textLabel->setLabel(MakeString()<<label<<" ["<<(int)(progress*100)<<"%]");
				AlloyApplicationContext()->requestPack();
				return true;
			});
			mesh.clear();
			Mesh tmp;
			textLabel->setLabel("Solving for iso-surface...");
			AlloyApplicationContext()->requestPack();
			isosurf.solve(grid,tmp,(this->meshType==0)?MeshType::TRIANGLE:MeshType::QUAD,regularize,0.0f);
			std::chrono::steady_clock::time_point currentTime=std::chrono::steady_clock::now();
			elapsed = std::chrono::duration<double>(currentTime - lastTime).count();
			tmp.transform(T);
			tmp.updateBoundingBox();
			tmp.clone(mesh);
			textLabel->setLabel(MakeString()<<"Elapsed Time: "<<elapsed<<" sec");
			AlloyApplicationContext()->requestPack();
	 },
	 [this]() {
		 mesh.setDirty(true);
	 }));
	 }
	 worker->execute();


}
void LevelSetGridEx::initializeFrameBuffers(aly::AlloyContext* context) {
	float2 dims = renderRegion->getBounds().dimensions;
	int w = (int) dims.x;
	int h = (int) dims.y;
	renderFrameBuffer->initialize(w, h);
	pointColorFrameBuffer->initialize(w, h);
	colorFrameBuffer->initialize(w, h);
	depthFrameBuffer->initialize(w, h);
	pointCloudDepthBuffer->initialize(w, h);
	wireframeFrameBuffer->initialize(w, h);
}
void LevelSetGridEx::draw(AlloyContext* context) {
	const float MIN_ELAPSED_TIME = 0.25f;
	if (frameBuffersDirty) {
		initializeFrameBuffers(context);
		frameBuffersDirty = false;
	}
	if (parametersDirty) {
		if (cameraType == 0) {
			camera.setNearFarPlanes(-2.0f, 2.0f);
			camera.setCameraType(CameraType::Orthographic);
		} else {
			camera.setNearFarPlanes(0.01f, 10.0f);
			camera.setCameraType(CameraType::Perspective);
		}
	}
	std::list<std::pair<aly::Mesh*, aly::float4x4>> drawList;
	drawList.push_back(std::pair<aly::Mesh*, aly::float4x4>(&mesh, mesh.pose));
	if (camera.isDirty() || parametersDirty) {
		wireframeShader->setLineWidth(lineWidth.toFloat());
		switch (displayIndex) {
		case 0: //Solid
			if (colorReconstruction) {
				colorVertexShader->draw(drawList, camera, *colorFrameBuffer);
			}
			depthAndNormalShader->draw(drawList, camera, *depthFrameBuffer);
			wireframeShader->setSolid(false);
			break;
		case 1: //Solid & Wireframe
			if (colorReconstruction) {
				colorVertexShader->draw(drawList, camera, *colorFrameBuffer);
			}
			depthAndNormalShader->draw(drawList, camera, *depthFrameBuffer);
			wireframeShader->setSolid(true);
			wireframeShader->setFaceColor(Color(0, 0, 0, 0));
			wireframeShader->setEdgeColor(lineColor);
			wireframeShader->draw(drawList, camera, *wireframeFrameBuffer);
			break;
		case 2: // Wireframe
			wireframeShader->setSolid(false);
			wireframeShader->setFaceColor(Color(0, 0, 0, 0));
			wireframeShader->setEdgeColor(lineColor);
			wireframeShader->draw(drawList, camera, *wireframeFrameBuffer);
			break;
		}
	}
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	const RGBAf bgColor = context->theme.DARKEST.toRGBAf();
	glClearColor(bgColor.x, bgColor.y, bgColor.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	box2px rbbox = renderRegion->getBounds();
	renderFrameBuffer->begin();
	switch (displayIndex) {
	case 0: //Solid
		if (colorReconstruction) {
			imageShader->draw(colorFrameBuffer->getTexture(),
					context->getViewport(), 1.0f, false);
		} else {
			matcapShader->draw(depthFrameBuffer->getTexture(), camera,
					context->getViewport(), context->getViewport(),
					faceColor.toRGBAf());
		}
		break;
	case 1: ///Solid & Wireframe
		if (colorReconstruction) {
			imageShader->draw(colorFrameBuffer->getTexture(),
					context->getViewport(), 1.0f, false);
		} else {
			matcapShader->draw(depthFrameBuffer->getTexture(), camera,
					context->getViewport(), context->getViewport(),
					faceColor.toRGBAf());
		}
		imageShader->draw(wireframeFrameBuffer->getTexture(),
				context->getViewport(), 1.0f, false);
		break;
	case 2: //Wireframe
		imageShader->draw(wireframeFrameBuffer->getTexture(),
				context->getViewport(), 1.0f, false);
		break;
	default:
		break;

	}
	renderFrameBuffer->end();
	imageShader->draw(renderFrameBuffer->getTexture(),
			context->pixelRatio * rbbox, 1.0f, false);
	camera.setDirty(false);
	parametersDirty = false;
}

