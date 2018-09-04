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
#include "segmentation/Phantom.h"
#include "grid/EndlessGrid.h"
#include <AlloyGradientVectorFlow.h>
#include <example/Springls3DEx.h>
#include <AlloyDistanceField.h>
using namespace aly;
Springls3DEx::Springls3DEx() :
		Application(1024 * 2, 600 * 2, "Spring Level Set 3D"), matcapShader(
				getFullPath("images/JG_Silver.png")), imageShader(
				ImageShader::Filter::SMALL_BLUR), running(false), simulation(
				std::shared_ptr < ManifoldCache3D > (new ManifoldCache3D())) {
	showIsoSurface = true;
	showSpringls = true;
	showParticles = false;
	showTracking = false;
	color = Color(255, 0, 0);
}
bool Springls3DEx::init(Composite& rootNode) {
	box3f renderBBox = box3f(float3(-0.5f, -0.5f, -0.5f),
			float3(1.0f, 1.0f, 1.0f));
	lastTime = -1;
	/*
	 mesh.load(getFullPath("models/armadillo.ply"));
	 mesh.updateVertexNormals(false);
	 Volume1f vol;
	 MeshToLevelSet(mesh,vol,true,2.5f,true,0.75f);
	 IsoSurface isoSurf;
	 isoSurf.solve(vol,mesh,MeshType::Quad);
	 RebuildDistanceFieldFast(vol,8.0f);
	 WriteVolumeToFile(MakeDesktopFile("armadillo.xml"),vol);
	 //WriteMeshToFile(MakeDesktopFile("horse_level.ply"),mesh);
	 */
	/*
	 EndlessGrid<float> vol({32,4,4}, 0.0f);
	 MeshToLevelSet(mesh, vol, 2.5f, false, 0.75f);
	 IsoSurface isoSurf;
	 isoSurf.solve(vol,mesh,MeshType::Quad);
	 WriteMeshToFile(MakeDesktopFile("horse_level.ply"),mesh);
	 WriteGridToFile(MakeDesktopFile("horse.xml"),vol);
	 int D=128;
	 */

	int D;
	{
		Volume1f sourceVol;
		/*
		 PhantomCube cube(D, D, D, 4.0f);
		 cube.setCenter(float3(0.0f));
		 cube.setWidth(1.21);
		 sourceVol= cube.solveDistanceField();
		 */
		isosurface.load(getFullPath("models/armadillo.ply"));
		isosurface.updateVertexNormals(false);
		std::cout << "Build Level Set" << std::endl;
		MeshToLevelSet(isosurface, sourceVol, true, 2.5f, true, 1.5f);
		std::cout << "Done " << sourceVol.dimensions() << std::endl;
		PhantomBubbles bubbles(sourceVol.rows, sourceVol.cols, sourceVol.slices,
				4.0f);
		D = std::max(std::max(sourceVol.rows, sourceVol.cols),
				sourceVol.slices);
		bubbles.setNoiseLevel(0.0f);
		bubbles.setNumberOfBubbles(12);
		bubbles.setFuzziness(0.5f);
		bubbles.setMinRadius(0.2f);
		bubbles.setMaxRadius(0.3f);
		bubbles.setInvertContrast(true);
		Volume1f targetVol = bubbles.solveLevelSet();
		Volume1f edgeVol;
		Volume3f vectorField;
		//std::cout<<"Solve Edges"<<std::endl;
		//SolveEdgeFilter(targetVol, edgeVol, 1);
		//std::cout<<"Solve GVF"<<std::endl;
		//SolveGradientVectorFlow(edgeVol, vectorField, 0.1f, 16, true);
		//std::cout<<"Done"<<std::endl;
		//WriteVolumeToFile(MakeDesktopFile("gvf.xml"),vectorField);
		//WriteVolumeToFile(MakeDesktopFile("target.xml"),targetVol);
		//WriteVolumeToFile(MakeDesktopFile("source.xml"),sourceVol);
		simulation.setInitialDistanceField(sourceVol);
		simulation.setPressure(targetVol, 0.4f, 0.5f);
		simulation.setCurvature(0.01f);
		//simulation.setVectorField(vectorField, 0.2f);
		simulation.init();
	}
	simulation.onUpdate =
			[this](uint64_t iteration, bool lastIteration) {
				if (lastIteration || (int)iteration == timelineSlider->getMaxValue().toInteger()) {
					stopButton->setVisible(false);
					playButton->setVisible(true);
					running = false;
				}
				AlloyApplicationContext()->addDeferredTask([this]() {
							timelineSlider->setUpperValue((int)simulation.getSimulationIteration());
							timelineSlider->setTimeValue((int)simulation.getSimulationIteration());
						});
			};

	//Initialize depth buffer to store the render
	int w = getContext()->getScreenWidth();
	int h = getContext()->getScreenHeight();

	//Set up camera
	camera.setNearFarPlanes(-5.0f, 5.0f);
	camera.setZoom(1.0f);
	camera.setCameraType(CameraType::Orthographic);
	//Map object geometry into unit bounding box for draw.
	camera.setPose(MakeTransform(box3f(float3(0.0f), float3(D)), renderBBox));
	//Add listener to respond to mouse manipulations
	addListener(&camera);
	setOnResize(
			[this](const int2& dims) {
				if(!getContext()->hasDeferredTasks()) {
					getContext()->addDeferredTask([this]() {
								box2f bbox = renderRegion->getBounds();
								springlsBuffer.initialize(bbox.dimensions.x, bbox.dimensions.y);
								isosurfBuffer.initialize(bbox.dimensions.x, bbox.dimensions.y);
								particleBuffer.initialize(bbox.dimensions.x, bbox.dimensions.y);
								camera.setDirty(true);
							});
				}
			});
	playButton = IconButtonPtr(
			new IconButton(0xf144, CoordPerPX(0.5f, 0.5f, -35.0f, -35.0f),
					CoordPX(70.0f, 70.0f)));
	stopButton = IconButtonPtr(
			new IconButton(0xf28d, CoordPerPX(0.5f, 0.5f, -35.0f, -35.0f),
					CoordPX(70.0f, 70.0f)));
	playButton->borderWidth = UnitPX(0.0f);
	stopButton->borderWidth = UnitPX(0.0f);
	playButton->backgroundColor = MakeColor(getContext()->theme.DARKER);
	stopButton->backgroundColor = MakeColor(getContext()->theme.DARKER);
	playButton->foregroundColor = MakeColor(0, 0, 0, 0);
	stopButton->foregroundColor = MakeColor(0, 0, 0, 0);
	playButton->iconColor = MakeColor(getContext()->theme.LIGHTER);
	stopButton->iconColor = MakeColor(getContext()->theme.LIGHTER);
	playButton->borderColor = MakeColor(getContext()->theme.LIGHTEST);
	stopButton->borderColor = MakeColor(getContext()->theme.LIGHTEST);
	playButton->onMouseDown =
			[this](AlloyContext* context, const InputEvent& e) {
				if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
					stopButton->setVisible(true);
					playButton->setVisible(false);
					int maxIteration = (int)std::ceil(simulation.getSimulationDuration() / simulation.getTimeStep());
					timelineSlider->setTimeValue(0);
					timelineSlider->setMaxValue(maxIteration);
					timelineSlider->setVisible(true);

					context->addDeferredTask([this]() {
								simulation.init();
								running = true;
							});
					return true;
				}
				return false;
			};
	stopButton->onMouseDown =
			[this](AlloyContext* context, const InputEvent& e) {
				if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
					stopButton->setVisible(false);
					playButton->setVisible(true);
					running = false;
					return true;
				}
				return false;
			};
	stopButton->setVisible(false);
	CompositePtr infoComposite = CompositePtr(
			new Composite("Info", CoordPerPX(0.0f, 1.0f, 0.0f, -80.0f),
					CoordPX(80.0f, 80.0f)));
	infoComposite->backgroundColor = MakeColor(getContext()->theme.DARKER);
	infoComposite->borderColor = MakeColor(getContext()->theme.DARK);
	infoComposite->borderWidth = UnitPX(0.0f);

	infoComposite->add(playButton);
	infoComposite->add(stopButton);

	timelineSlider = TimelineSliderPtr(
			new TimelineSlider("Timeline",
					CoordPerPX(0.0f, 1.0f, 80.0f, -80.0f),
					CoordPerPX(1.0f, 0.0f, -80.0f, 80.0f), Integer(0),
					Integer(0), Integer(0)));
	timelineSlider->backgroundColor = MakeColor(
			AlloyApplicationContext()->theme.DARKER);
	timelineSlider->borderColor = MakeColor(
			AlloyApplicationContext()->theme.DARK);
	timelineSlider->borderWidth = UnitPX(0.0f);
	timelineSlider->onChangeEvent =
			[this](const Number& timeValue, const Number& lowerValue, const Number& upperValue) {
				camera.setDirty(true);
			};
	timelineSlider->setMajorTick(100);
	timelineSlider->setMinorTick(10);
	timelineSlider->setLowerValue(0);
	timelineSlider->setUpperValue(0);
	int maxIteration = (int) std::ceil(
			simulation.getSimulationDuration() / simulation.getTimeStep());
	timelineSlider->setMaxValue(maxIteration);
	timelineSlider->setVisible(true);
	timelineSlider->setModifiable(false);

	ParameterPanePtr controls = ParameterPanePtr(
			new ParameterPane("Controls", CoordPX(0.0f, 0.0f),
					CoordPerPX(0.0f, 1.0f, 400.0f, -80.0f)));
	controls->onChange =
			[this](const std::string& label, const AnyInterface& value) {
				//parametersDirty = true;
			};
	renderRegion = RegionPtr(
			new aly::Region("Render Region", CoordPX(400.0f, 0.0f),
					CoordPerPX(1.0f, 1.0f, -400.0f, -80.0f)));

	controls->addGroup("Simulation", true);
	simulation.setup(controls);
	controls->addGroup("Visualization", true);
	controls->addCheckBox("IsoSurface", showIsoSurface);
	controls->addCheckBox("Springls", showSpringls);
	controls->addCheckBox("Particles", showParticles);
	controls->addCheckBox("Tracking", showTracking);
	controls->addColorField("Color", color);
	controls->setAlwaysShowVerticalScrollBar(false);
	controls->setScrollEnabled(false);
	controls->backgroundColor = MakeColor(getContext()->theme.DARKER);
	controls->borderColor = MakeColor(getContext()->theme.DARK);
	controls->borderWidth = UnitPX(1.0f);
	rootNode.add(controls);
	rootNode.add(renderRegion);
	rootNode.add(infoComposite);
	rootNode.add(timelineSlider);
	camera.setActiveRegion(renderRegion.get());

	return true;
}
void Springls3DEx::draw(AlloyContext* context) {
	if (springlsBuffer.getWidth() == 0) {
		box2f bbox = renderRegion->getBounds();
		springlsBuffer.initialize(bbox.dimensions.x, bbox.dimensions.y);
		isosurfBuffer.initialize(bbox.dimensions.x, bbox.dimensions.y);
		particleBuffer.initialize(bbox.dimensions.x, bbox.dimensions.y);
	}
	if (running) {
		if (!simulation.step()) {
			running = false;
		} else {
			camera.setDirty(true);
		}
	}
	if (camera.isDirty()) {

		int currentTime = timelineSlider->getTimeValue().toInteger();
		if (currentTime != lastTime) {
			std::shared_ptr<CacheElement3D> elem = simulation.getCache()->get(
					currentTime);
			Manifold3D* contour;
			if (elem.get() != nullptr) {
				contour = elem->getContour().get();
			} else {
				contour = simulation.getSurface();
			}
			isosurface.vertexLocations = contour->vertexLocations;
			isosurface.vertexNormals = contour->vertexNormals;
			isosurface.triIndexes = contour->triIndexes;
			isosurface.quadIndexes = contour->quadIndexes;
			springls.vertexLocations = contour->vertexes;
			springls.setType(
					static_cast<GLMesh::PrimitiveType>(contour->meshType));
			particles.vertexLocations = contour->particles;
			particles.vertexNormals = contour->vertexNormals;
			isosurface.setDirty(true);
			springls.setDirty(true);
			particles.setDirty(true);
			lastTime = currentTime;
		}
		depthAndNormalShader.draw(isosurface, camera, isosurfBuffer);
		depthAndNormalShader.draw(springls, camera, springlsBuffer);
		particleShader.draw(particles,camera,particleBuffer,2*SpringLevelSet3D::PARTICLE_RADIUS);
	}
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	if (showIsoSurface) {
		matcapShader.draw(isosurfBuffer.getTexture(), camera,
				renderRegion->getBounds() * context->pixelRatio,
				context->getViewport(), RGBAf(1.0f));
	}
	if (showSpringls) {
		matcapShader.draw(springlsBuffer.getTexture(), camera,
				renderRegion->getBounds() * context->pixelRatio,
				context->getViewport(), RGBAf(1.0f));
	}
	if (showParticles) {
		matcapShader.draw(particleBuffer.getTexture(), camera,
				renderRegion->getBounds() * context->pixelRatio,
				context->getViewport(), RGBAf(1.0f));
	}
	camera.setDirty(false);
}

