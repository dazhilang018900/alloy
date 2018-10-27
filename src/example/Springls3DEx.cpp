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
#include "vision/Phantom.h"
#include "graphics/EndlessGrid.h"
#include "image/AlloyGradientVectorFlow.h"
#include "example/Springls3DEx.h"
#include "image/AlloyDistanceField.h"
using namespace aly;

Springls3DEx::Springls3DEx(int exampleIndex) :
		Application(1280,720, "Spring Level Set 3D"),
		matcapShaderIso(getFullPath("images/JG_Red.png")),matcapShaderParticles(getFullPath("images/matcap/00005.png")),
		matcapShaderSpringls(getFullPath("images/JG_Silver.png")),
		springlShader(getFullPath("images/JG_Red.png"),getFullPath("images/JG_Silver.png")),
		imageShader(ImageShader::Filter::SMALL_BLUR),
		running(false), simulation(std::shared_ptr < ManifoldCache3D > (new ManifoldCache3D())) {
	showIsoSurface = true;
	showSpringls = true;
	showParticles = true;
	showTracking = true;
	example=exampleIndex;
}
bool Springls3DEx::init(Composite& rootNode) {
	box3f renderBBox = box3f(float3(-0.5f, -0.5f, -0.5f),
			float3(1.0f, 1.0f, 1.0f));
	lastTime = -1;
	int D;
	if(example==0){
		Volume1f sourceVol;
		isosurface.load(getFullPath("models/armadillo.ply"));
		isosurface.updateVertexNormals(false);
		MeshToLevelSet(isosurface, sourceVol, true, 2.5f, true, 1.5f);
		PhantomBubbles bubbles(sourceVol.rows, sourceVol.cols, sourceVol.slices,4.0f);
		D = std::max(std::max(sourceVol.rows, sourceVol.cols),sourceVol.slices);
		bubbles.setNoiseLevel(0.0f);
		bubbles.setNumberOfBubbles(12);
		bubbles.setFuzziness(0.5f);
		bubbles.setMinRadius(0.2f);
		bubbles.setMaxRadius(0.3f);
		bubbles.setInvertContrast(true);
		Volume1f targetVol = bubbles.solveLevelSet();
		Volume1f edgeVol;
		Volume3f vectorField;
		simulation.setInitialDistanceField(sourceVol);
		simulation.setPressure(targetVol, 0.4f, 0.5f);
		simulation.setCurvature(0.01f);
		simulation.setTemporalScheme(SpringLevelSet3D::TemporalScheme::FirstOrder);
		simulation.setSimulationDuration((3*D)/2);
		simulation.init();
	} else if(example==1){
		D=128;
		PhantomSphere sphereGen(D,D,D);
		const float radius = 0.15f;
		sphereGen.setCenter(float3(0.35f, 0.35f, 0.35f)*2.0f-float3(1.0f));
		sphereGen.setRadius(radius*2);
		Volume1f sourceVol=sphereGen.solveDistanceField();
		simulation.setInitialDistanceField(sourceVol);
		simulation.setAdvection(SpringLevelSet3D::ENRIGHT_FUNCTION);
		simulation.setCurvature(0.01f);
		simulation.setResamplingEnabled(false);
		simulation.setTimeStep(0.32f);
		simulation.setSimulationDuration(D*SpringLevelSet3D::ENRIGHT_PERIOD);
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
				contour = simulation.getManifold();
			}
			isosurface.vertexLocations = contour->vertexLocations;
			isosurface.vertexNormals = contour->vertexNormals;
			isosurface.triIndexes = contour->triIndexes;
			isosurface.quadIndexes = contour->quadIndexes;
			springls.vertexLocations = contour->vertexes;
			springls.setType(
					static_cast<GLMesh::PrimitiveType>(contour->meshType));
			particles.vertexLocations = contour->particles;
			particles.vertexNormals = contour->normals;
			Breadcrumbs3D& crumbs=simulation.crumbs;
			int T=std::min((int)crumbs.size(),timelineSlider->getTimeValue().toInteger()+1);
			trails.vertexLocations.clear();
			for(int t=std::max(1,T-8);t<T;t++) {
				int N=crumbs.size(t);
				for (int n = 0; n < N; n++) {
					float3i p1 = crumbs(t,n);
					if(p1.index>=0){
						float3i p2 = crumbs(t-1,p1.index);
						trails.vertexLocations.push_back(p1);
						trails.vertexLocations.push_back(p2);
					}
				}
			}
			trails.setType(aly::GLMesh::PrimitiveType::LINES);
			trails.setDirty(true);
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
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
	if(showIsoSurface&&showSpringls){
		springlShader.draw(springlsBuffer.getTexture(),isosurfBuffer.getTexture(), camera,
				renderRegion->getBounds() * context->pixelRatio,
				context->getViewport(), RGBAf(1.0f));
	} else {
		if (showIsoSurface) {
			matcapShaderIso.draw(isosurfBuffer.getTexture(), camera,
					renderRegion->getBounds() * context->pixelRatio,
					context->getViewport(), RGBAf(1.0f));
		}
		if (showSpringls) {
			matcapShaderSpringls.draw(springlsBuffer.getTexture(), camera,
					renderRegion->getBounds() * context->pixelRatio,
					context->getViewport(), RGBAf(1.0f));
		}
	}
	if(showTracking){
		box2px bbox=context->getViewport();
		box2px rbbox=renderRegion->getBounds();
		glViewport((int)rbbox.position.x,(int) (bbox.dimensions.y - rbbox.position.y
				- rbbox.dimensions.y),(int)rbbox.dimensions.x,(int)rbbox.dimensions.y);
		lineShader.draw(trails,camera,isosurfBuffer.getViewport(),1.5f,Color(153, 204, 255));
		glViewport(0,0,(int)bbox.dimensions.x,(int)bbox.dimensions.y);
	}
	if (showParticles) {
		matcapShaderParticles.draw(particleBuffer.getTexture(), camera,
				renderRegion->getBounds() * context->pixelRatio,
				context->getViewport(), RGBAf(1.0f));
	}
	camera.setDirty(false);
}

