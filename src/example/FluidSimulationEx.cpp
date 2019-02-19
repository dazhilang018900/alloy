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
#include "example/FluidSimulationEx.h"
#include "image/AlloyGradientVectorFlow.h"
#include "image/AlloyDistanceField.h"
#include "graphics/AlloyIsoContour.h"
#include "vision/SpringLevelSet2D.h"
#include "vision/SpringlsSecondOrder.h"
using namespace aly;
FluidSimulationEx::FluidSimulationEx(int width,int height) :Application(1200, 800, "Fluid Simulation", false), simWidth(width),simHeight(height),parametersDirty(true),frameBuffersDirty(true) {
}
bool FluidSimulationEx::init(Composite& rootNode) {
	const int w = simWidth;
	const int h = simHeight;
	cache = std::shared_ptr<ManifoldCache2D>(new ManifoldCache2D());
	simulation = std::shared_ptr<FluidSimulation>(
			new FluidSimulation(int2(w, h), 1.0f / h, MotionScheme::IMPLICIT,
					cache));
	simulation->onUpdate =
			[this](uint64_t iteration, bool lastIteration) {
				if (lastIteration || (int)iteration == timelineSlider->getMaxValue().toInteger()) {
					stopButton->setVisible(false);
					playButton->setVisible(true);
					running = false;
				}
				AlloyApplicationContext()->addDeferredTask([this]() {
							timelineSlider->setUpperValue((int)simulation->getSimulationIteration());
							timelineSlider->setTimeValue((int)simulation->getSimulationIteration());
						});
			};
	simulation->init();
	parametersDirty = true;
	frameBuffersDirty = true;
	BorderCompositePtr layout = BorderCompositePtr(
			new BorderComposite("UI Layout", CoordPX(0.0f, 0.0f),
					CoordPercent(1.0f, 1.0f), false));
	ParameterPanePtr controls = ParameterPanePtr(
			new ParameterPane("Controls", CoordPX(0.0f, 0.0f),
					CoordPercent(1.0f, 1.0f)));
	BorderCompositePtr controlLayout = BorderCompositePtr(
			new BorderComposite("Control Layout", CoordPX(0.0f, 0.0f),
					CoordPercent(1.0f, 1.0f), true));
	controls->onChange =
			[this](const std::string& label, const AnyInterface& value) {
				parametersDirty = true;
			};

	lineWidth = Float(2.0f);
	lineColor = Color(0.7f, 0.2f, 0.2f, 1.0f);
	particleColor = Color(0.7f, 0.2f, 0.2f, 0.5f);
	vecfieldColor = Color(0.3f, 0.6f, 0.3f, 0.5f);
	controls->setAlwaysShowVerticalScrollBar(false);
	controls->setScrollEnabled(false);
	controls->backgroundColor = MakeColor(getContext()->theme.DARKER);
	controls->borderColor = MakeColor(getContext()->theme.DARK);
	controls->borderWidth = UnitPX(1.0f);

	controlLayout->backgroundColor = MakeColor(getContext()->theme.DARKER);
	controlLayout->borderWidth = UnitPX(0.0f);
	CompositePtr renderRegion = CompositePtr(
			new Composite("View", CoordPX(0.0f, 0.0f),
					CoordPercent(1.0f, 1.0f)));
	layout->setWest(controlLayout, UnitPX(400.0f));
	controlLayout->setCenter(controls);
	layout->setCenter(renderRegion);
	CompositePtr infoComposite = CompositePtr(
			new Composite("Info", CoordPX(0.0f, 0.0f),
					CoordPercent(1.0f, 1.0f)));
	infoComposite->backgroundColor = MakeColor(getContext()->theme.DARKER);
	infoComposite->borderColor = MakeColor(getContext()->theme.DARK);
	infoComposite->borderWidth = UnitPX(0.0f);
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
					cache->clear();
					int maxIteration = (int)std::ceil(simulation->getSimulationDuration() / simulation->getTimeStep());
					timelineSlider->setTimeValue(0);
					timelineSlider->setMaxValue(maxIteration);
					timelineSlider->setVisible(true);
					context->addDeferredTask([this]() {
								simulation->init();
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
	infoComposite->add(playButton);
	infoComposite->add(stopButton);
	controlLayout->setSouth(infoComposite, UnitPX(80.0f));
	rootNode.add(layout);

	controls->addGroup("Simulation", true);
	simulation->setup(controls);
	controls->addGroup("Visualization", true);
	controls->addCheckBox("Show Contour", showContour);
	controls->addCheckBox("Show Particles", showParticles);
	controls->addCheckBox("Show Velocity", showVelocity);
	controls->addCheckBox("Show Pressure", showPressure);
	controls->addColorField("Particle", particleColor);
	controls->addColorField("Velocity", vecfieldColor);
	controls->addColorField("Line", lineColor);
	controls->addNumberField("Line Width", lineWidth, Float(1.0f), Float(20.0f),
			6.0f);

	timelineSlider = TimelineSliderPtr(
			new TimelineSlider("Timeline", CoordPerPX(0.0f, 1.0f, 0.0f, -80.0f),
					CoordPerPX(1.0f, 0.0f, 0.0f, 80.0f), Integer(0), Integer(0),
					Integer(0)));
	CompositePtr viewRegion = CompositePtr(
			new Composite("View", CoordPX(0.0f, 0.0f),
					CoordPerPX(1.0f, 1.0f, 0.0f, -80.0f)));
	timelineSlider->backgroundColor = MakeColor(
			AlloyApplicationContext()->theme.DARKER);
	timelineSlider->borderColor = MakeColor(
			AlloyApplicationContext()->theme.DARK);
	timelineSlider->borderWidth = UnitPX(0.0f);
	timelineSlider->onChangeEvent =
			[this](const Number& timeValue, const Number& lowerValue, const Number& upperValue) {

			};
	timelineSlider->setMajorTick(100);
	timelineSlider->setMinorTick(10);
	timelineSlider->setLowerValue(0);
	timelineSlider->setUpperValue(0);
	int maxIteration = (int) std::ceil(
			simulation->getSimulationDuration() / simulation->getTimeStep());
	timelineSlider->setMaxValue(maxIteration);
	timelineSlider->setVisible(true);
	timelineSlider->setModifiable(false);
	renderRegion->add(viewRegion);
	renderRegion->add(timelineSlider);
	const Image1ub& img = simulation->getLabels();
	float downScale = std::min(
			(getContext()->getScreenWidth() - 350.0f) / img.width,
			(getContext()->getScreenHeight() - 80.0f) / img.height);
	resizeableRegion = AdjustableCompositePtr(
			new AdjustableComposite("Image",
					CoordPerPX(0.5, 0.5, -img.width * downScale * 0.5f,
							-img.height * downScale * 0.5f),
					CoordPX(img.width * downScale, img.height * downScale)));
	Application::addListener(resizeableRegion.get());
	overlayGlyph = AlloyApplicationContext()->createImageGlyph(
			simulation->getManifold()->overlay, false);
	DrawPtr drawContour =
			DrawPtr(
					new Draw("Contour Draw", CoordPX(0.0f, 0.0f),
							CoordPercent(1.0f, 1.0f),
							[this](AlloyContext* context, const box2px& bounds) {
								int currentTime=timelineSlider->getTimeValue().toInteger();
								std::shared_ptr<CacheElement2D> elem = this->cache->get(currentTime);
								Manifold2D* contour;
								if (elem.get() != nullptr) {
									contour = elem->getManifold().get();
								}
								else {
									contour = simulation->getManifold();
								}
								if (currentTime != lastSimTime&&contour->overlay.size()>0) {
									overlayGlyph->set(contour->overlay, context);
									lastSimTime = currentTime;
								}
								NVGcontext* nvg = context->nvgContext;
								nvgLineCap(nvg, NVG_ROUND);
								nvgLineJoin(nvg,NVG_ROUND);
								const Image1f& img=simulation->getPressure();
								float scale = bounds.dimensions.x / (float)img.width;
								if(showVelocity) {
									if (0.2f*scale > 0.5f) {
										const Image2f& vecField=contour->fluidParticles.velocityImage;
										nvgFillColor(nvg, vecfieldColor);
										nvgStrokeWidth(nvg, 0.05f*scale);
										const float aH=0.5f*std::sqrt(2.0f);
										const float aW= 0.25f*std::sqrt(2.0f);
										for (int i = 0;i < vecField.width;i++) {
											for (int j = 0;j < vecField.height;j++) {
												float2 v = vecField(i, j);
												float2 pt1 = float2(i + 0.5f + 0.7f*aH*v.x, j + 0.5f + 0.7f*aH*v.y);
												float2 pt2 = float2(i + 0.5f - 0.5f*aW*v.y - 0.3f*aH*v.x, j + 0.5f + 0.5f*aW*v.x- 0.3f*aH*v.y);
												float2 pt3 = float2(i + 0.5f + 0.5f*aW*v.y - 0.3f*aH*v.x, j + 0.5f - 0.5f*aW*v.x - 0.3f*aH*v.y);

												pt1.x = pt1.x / (float)img.width;
												pt1.y = pt1.y / (float)img.height;
												pt1 = pt1*bounds.dimensions + bounds.position;

												pt2.x = pt2.x / (float)img.width;
												pt2.y = pt2.y / (float)img.height;
												pt2 = pt2*bounds.dimensions + bounds.position;

												pt3.x = pt3.x / (float)img.width;
												pt3.y = pt3.y / (float)img.height;
												pt3 = pt3*bounds.dimensions + bounds.position;

												nvgBeginPath(nvg);
												nvgMoveTo(nvg, pt1.x, pt1.y);
												nvgLineTo(nvg, pt2.x, pt2.y);
												nvgLineTo(nvg, pt3.x, pt3.y);
												nvgClosePath(nvg);
												nvgFill(nvg);
											}
										}
										if (0.05f*scale > 0.5f) {
											nvgStrokeColor(nvg, Color(0.4f, 0.4f, 0.4f, 0.5f));
											nvgBeginPath(nvg);
											for (int i = 0;i < img.width;i++) {
												float2 pt = float2(0.5f + i, 0.5f);
												pt.x = pt.x / (float)img.width;
												pt.y = pt.y / (float)img.height;
												pt = pt*bounds.dimensions + bounds.position;
												nvgMoveTo(nvg, pt.x, pt.y);
												pt = float2(0.5f + i, 0.5f + img.height - 1.0f);
												pt.x = pt.x / (float)img.width;
												pt.y = pt.y / (float)img.height;
												pt = pt*bounds.dimensions + bounds.position;
												nvgLineTo(nvg, pt.x, pt.y);
											}
											for (int j = 0;j < img.height;j++) {
												float2 pt = float2(0.5f, 0.5f + j);
												pt.x = pt.x / (float)img.width;
												pt.y = pt.y / (float)img.height;
												pt = pt*bounds.dimensions + bounds.position;
												nvgMoveTo(nvg, pt.x, pt.y);
												pt = float2(0.5f + img.width - 1.0f, 0.5f + j);
												pt.x = pt.x / (float)img.width;
												pt.y = pt.y / (float)img.height;
												pt = pt*bounds.dimensions + bounds.position;
												nvgLineTo(nvg, pt.x, pt.y);
											}
											nvgStroke(nvg);
										}
									}
								}
								float fluidVoxelSize=simulation->getFluidVoxelSize();
								float2 gridScale=1.0f/(fluidVoxelSize*float2(img.dimensions()));
								if(showContour) {
									nvgStrokeColor(nvg, lineColor);
									nvgStrokeWidth(nvg, lineWidth.toFloat());
									for (int n = 0;n < (int)contour->indexes.size();n++) {
										std::vector<uint32_t> curve = contour->indexes[n];
										nvgPathWinding(nvg, NVG_CW);
										nvgBeginPath(nvg);
										bool firstTime = true;
										for (uint32_t idx : curve) {
											float2 pt = contour->vertexLocations[idx]*gridScale;
											pt = pt*bounds.dimensions + bounds.position;
											if (firstTime) {
												nvgMoveTo(nvg, pt.x, pt.y);
											}
											else {
												nvgLineTo(nvg, pt.x, pt.y);
											}
											firstTime = false;
										}
										nvgStroke(nvg);
									}
								}
								if(showParticles) {
									float fluidParticleRadius=contour->fluidParticles.radius;
									if (fluidParticleRadius*scale > 0.5f&&contour->fluidParticles.particles.size()>0) {
										nvgFillColor(nvg,particleColor);
										const Vector2f& fluidParticles=contour->fluidParticles.particles;
										for (int n = 0;n < (int)fluidParticles.size();n++) {
											float2 pt = fluidParticles[n];
											pt.x = pt.x / (float)img.width;
											pt.y = pt.y / (float)img.height;
											pt = pt*bounds.dimensions + bounds.position;
											nvgBeginPath(nvg);
											nvgEllipse(nvg, pt.x, pt.y,fluidParticleRadius*scale, fluidParticleRadius*scale);
											nvgFill(nvg);
										}
									}
								}
								nvgFillColor(nvg, Color(32,32,32));
								for(SimulationObjectPtr obj:simulation->getWallObjects()) {
									BoxObject* ptr=dynamic_cast<BoxObject*>(obj.get());
									if(ptr) {
										float2 minPt = ptr->mMin*gridScale;
										minPt = minPt*bounds.dimensions + bounds.position;
										float2 maxPt = ptr->mMax*gridScale;
										maxPt = maxPt*bounds.dimensions + bounds.position;
										nvgBeginPath(nvg);
										nvgRect(nvg,minPt.x,minPt.y,maxPt.x-minPt.x,maxPt.y-minPt.y);
										nvgFill(nvg);
									} else {
										SphereObject* ptr=dynamic_cast<SphereObject*>(obj.get());
										if(ptr) {
											float2 minPt = ptr->mCenter*gridScale;
											minPt = minPt*bounds.dimensions + bounds.position;
											float2 rad=ptr->mRadius*bounds.dimensions*gridScale;
											nvgBeginPath(nvg);
											nvgEllipse(nvg,minPt.x,minPt.y,rad.x,rad.y);
											nvgFill(nvg);
										}
									}
								}

							}));
	glyphRegion = GlyphRegionPtr(
			new GlyphRegion("Image Region", overlayGlyph, CoordPX(0.0f, 0.0f),
					CoordPercent(1.0f, 1.0f)));
	glyphRegion->setAspectRule(AspectRule::Unspecified);
	glyphRegion->foregroundColor = MakeColor(COLOR_NONE);
	glyphRegion->backgroundColor = MakeColor(COLOR_NONE);
	glyphRegion->borderColor = MakeColor(COLOR_NONE);
	drawContour->onScrollWheel =
			[this](AlloyContext* context, const InputEvent& event)
			{
				box2px bounds = resizeableRegion->getBounds(false);
				pixel scaling = (pixel)(1 - 0.1f*event.scroll.y);
				pixel2 newBounds = bounds.dimensions*scaling;
				pixel2 cursor = context->cursorPosition;
				pixel2 relPos = (cursor - bounds.position) / bounds.dimensions;
				pixel2 newPos = cursor - relPos*newBounds;
				bounds.position = newPos;
				bounds.dimensions = newBounds;
				resizeableRegion->setDragOffset(pixel2(0, 0));
				resizeableRegion->position = CoordPX(bounds.position - resizeableRegion->parent->getBoundsPosition());
				resizeableRegion->dimensions = CoordPX(bounds.dimensions);
				float2 dims = float2(simulation->dimensions());
				cursor = aly::clamp(dims*(event.cursor - bounds.position) / bounds.dimensions, float2(0.0f), dims);

				context->requestPack();
				return true;
			};
	resizeableRegion->backgroundColor = MakeColor(255,255,255, 255);
	resizeableRegion->add(glyphRegion);
	resizeableRegion->add(drawContour);
	resizeableRegion->setAspectRatio(img.width / (float) img.height);
	resizeableRegion->setAspectRule(AspectRule::FixedHeight);
	resizeableRegion->setDragEnabled(true);
	resizeableRegion->setClampDragToParentBounds(false);
	resizeableRegion->borderWidth = UnitPX(2.0f);
	resizeableRegion->borderColor = MakeColor(
			AlloyApplicationContext()->theme.LIGHTER);
	glyphRegion->onMouseDown = [=](AlloyContext* context, const InputEvent& e) {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
			//Bring component to top by setting it to be drawn last.
			dynamic_cast<Composite*>(resizeableRegion->parent)->putLast(resizeableRegion);
			resizeableRegion->borderColor = MakeColor(AlloyApplicationContext()->theme.LIGHTEST);
		}
		return false;
	};
	glyphRegion->onMouseUp =
			[=](AlloyContext* context, const InputEvent& e) {
				resizeableRegion->borderColor = MakeColor(AlloyApplicationContext()->theme.LIGHTER);
				return false;
			};

	viewRegion->backgroundColor = MakeColor(getContext()->theme.DARKER);
	viewRegion->borderColor = MakeColor(getContext()->theme.DARK);
	viewRegion->borderWidth = UnitPX(1.0f);
	viewRegion->add(resizeableRegion);
	return true;
}
void FluidSimulationEx::draw(AlloyContext* context) {
	glyphRegion->setVisible(showPressure);
	if (running) {
		if (!simulation->step()) {
			running = false;
		}
	}
}

