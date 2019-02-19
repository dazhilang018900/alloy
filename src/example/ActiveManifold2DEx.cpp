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

#include "example/ActiveManifold2DEx.h"
#include "image/AlloyGradientVectorFlow.h"
#include "image/AlloyDistanceField.h"
#include "graphics/AlloyIsoContour.h"
#include "vision/SpringLevelSet2D.h"
#include "vision/SpringlsSecondOrder.h"
#include "ui/AlloyDrawUtil.h"
using namespace aly;
ActiveManifold2DEx::ActiveManifold2DEx(int example) :
		Application(1300, 1000, "Active Contour 2D", false), currentIso(0.0f), example(
				example) {
}
void ActiveManifold2DEx::createTextLevelSet(aly::Image1f& distField,
		aly::Image1f& gray, int w, int h, const std::string& text,
		float textSize, float maxDistance) {
	GLFrameBuffer renderBuffer;
	//Render text to image
	NVGcontext* nvg = getContext()->nvgContext;
	renderBuffer.initialize(w, h);
	renderBuffer.begin(RGBAf(1.0f, 1.0f, 1.0f, 1.0f));
	nvgBeginFrame(nvg, w, h, 1.0f);
	nvgFontFaceId(nvg, getContext()->getFont(FontType::Bold)->handle);
	nvgFillColor(nvg, Color(0, 0, 0));
	nvgFontSize(nvg, textSize);
	nvgTextAlign(nvg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
	nvgText(nvg, w * 0.5f, h * 0.5f, text.c_str(), nullptr);
	nvgEndFrame(nvg);
	renderBuffer.end();
	ImageRGBAf img = renderBuffer.getTexture().read();
	FlipVertical(img);
	ConvertImage(img, gray);
	gray -= float1(0.5f);
	DistanceField2f df;
	df.solve(gray, distField, maxDistance);
	gray = (-gray + float1(0.5f));
}
aly::Image1f ActiveManifold2DEx::createCircleLevelSet(int w, int h,
		float2 center, float r) {
	aly::Image1f levelSet(w, h);
	for (int j = 0; j < h; j++) {
		for (int i = 0; i < w; i++) {
			float l = distance(float2((float) i, (float) j), center) - r;
			levelSet(i, j).x = l;
		}
	}
	return levelSet;
}
void ActiveManifold2DEx::createRotationField(aly::Image2f& vecField, int w,
		int h) {
	vecField.resize(w, h);
	float2 center = float2(0.5f * w, 0.5f * h);
	float r = std::max(0.5f * w, 0.5f * h);
	for (int i = 0; i < w; i++) {
		for (int j = 0; j < h; j++) {
			float2 diff = (float2((float) i, (float) j) - center) / r;
			vecField(i, j) = float2(-diff.y, diff.x);
		}
	}
}
bool ActiveManifold2DEx::init(Composite& rootNode) {
	int w = 128;
	int h = 128;
	Image1f distField;
	float maxDistance = 32;
	createTextLevelSet(distField, gray, w, h, "A", 196.0f, maxDistance);
	ConvertImage(gray, img);
	cache = std::shared_ptr < ManifoldCache2D > (new ManifoldCache2D());
	if (example == 0) {
		simulation = std::shared_ptr < ActiveManifold2D
				> (new ActiveManifold2D(cache));
		simulation->setCurvature(2.0f);
	} else if (example == 1) {
		simulation = std::shared_ptr < ActiveManifold2D
				> (new SpringLevelSet2D(cache));
		simulation->setCurvature(0.1f);
	} else if (example == 2) {
		simulation = std::shared_ptr < ActiveManifold2D
				> (new SpringlsSecondOrder(cache));
		simulation->setCurvature(0.1f);
	} else {
		return false;
	}
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
	Image1f init = createCircleLevelSet(w, h, float2(0.5f * w, 0.5f * h),
			std::min(w, h) * 0.35f);
	SolveGradientVectorFlow(distField, vecField, distField.height, true);
	//createRotationField(vecField, w, h);
	simulation->setInitialDistanceField(init);
	simulation->setVectorField(vecField, 0.9f);
	simulation->setPressure(gray, 0.01f, 0.5f);
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

	lineWidth = Float(4.0f);
	particleSize = Float(0.2f);

	lineColor = Color(0.0f, 0.5f, 0.5f, 1.0f);
	pointColor = Color(1.0f, 0.8f, 0.0f, 1.0f);
	springlColor = Color(0.5f, 0.5f, 0.5f, 1.0f);
	matchColor = Color(0.5f, 0.5f, 1.0f, 0.75f);
	particleColor = Color(0.6f, 0.0f, 0.0f, 1.0f);
	normalColor = Color(0.0f, 0.8f, 0.0f, 0.5f);
	vecfieldColor = Color(0.8f, 0.4f, 0.8f, 0.5f);
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
					simulation->crumbs.clear();
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
	controls->addNumberField("Line Width", lineWidth, Float(1.0f), Float(20.0f),
			6.0f);
	if (example > 0) {
		controls->addNumberField("Particle Size", particleSize, Float(0.0f),
				Float(1.0f), 6.0f);
		controls->addColorField("Element", springlColor);
		controls->addColorField("Particle", particleColor);
		controls->addColorField("Point", pointColor);
		controls->addColorField("Normal", normalColor);
		controls->addColorField("Line", lineColor);
		controls->addColorField("Correspondence", matchColor);
	}
	controls->addColorField("Vector Field", vecfieldColor);
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

	float downScale = std::min(
			(getContext()->getScreenWidth() - 350.0f) / img.width,
			(getContext()->getScreenHeight() - 80.0f) / img.height);
	resizeableRegion = AdjustableCompositePtr(
			new AdjustableComposite("Image",
					CoordPerPX(0.5, 0.5, -img.width * downScale * 0.5f,
							-img.height * downScale * 0.5f),
					CoordPX(img.width * downScale, img.height * downScale)));
	Application::addListener(resizeableRegion.get());
	ImageGlyphPtr imageGlyph = AlloyApplicationContext()->createImageGlyph(img,
			false);
	DrawPtr drawContour =
			DrawPtr(
					new Draw("Contour Draw", CoordPX(0.0f, 0.0f),
							CoordPercent(1.0f, 1.0f),
							[this](AlloyContext* context, const box2px& bounds) {
								std::shared_ptr<CacheElement2D> elem = this->cache->get(timelineSlider->getTimeValue().toInteger());
								Manifold2D* contour;
								if (elem.get() != nullptr) {
									contour = elem->getManifold().get();
								}
								else {
									contour = simulation->getManifold();

								}
								NVGcontext* nvg = context->nvgContext;
								nvgLineCap(nvg, NVG_ROUND);
								nvgLineJoin(nvg,NVG_ROUND);
								float scale = bounds.dimensions.x / (float)img.width;
								if (0.05f*scale > 0.5f) {

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

								if (0.04f*scale > 0.5f&&contour->particles.size()== contour->correspondence.size()) {
									nvgStrokeColor(nvg, matchColor);
									nvgStrokeWidth(nvg, 0.04f*scale);
									for (int n = 0;n < (int)contour->particles.size();n++) {
										float2 qt = contour->correspondence[n];
										if (!std::isinf(qt.x)) {
											float2 pt = contour->particles[n] + float2(0.5f);
											pt.x = pt.x / (float)img.width;
											pt.y = pt.y / (float)img.height;
											pt = pt*bounds.dimensions + bounds.position;
											nvgBeginPath(nvg);
											nvgMoveTo(nvg, pt.x, pt.y);
											pt = qt + float2(0.5f);
											pt.x = pt.x / (float)img.width;
											pt.y = pt.y / (float)img.height;
											pt = pt*bounds.dimensions + bounds.position;
											nvgLineTo(nvg, pt.x, pt.y);
											nvgStroke(nvg);
										}
									}
								}

								nvgFillColor(nvg, lineColor.toSemiTransparent(0.5f));
								nvgStrokeColor(nvg, lineColor);
								nvgStrokeWidth(nvg, lineWidth.toFloat());
								for (int n = 0;n < (int)contour->indexes.size();n++) {
									std::vector<uint32_t> curve = contour->indexes[n];
									nvgPathWinding(nvg, NVG_CW);
									nvgBeginPath(nvg);
									bool firstTime = true;
									for (uint32_t idx : curve) {
										float2 pt = contour->vertexLocations[idx] + float2(0.5f);
										pt.x = pt.x / (float)img.width;
										pt.y = pt.y / (float)img.height;
										pt = pt*bounds.dimensions + bounds.position;
										if (firstTime) {
											nvgMoveTo(nvg, pt.x, pt.y);
										}
										else {
											nvgLineTo(nvg, pt.x, pt.y);
										}
										firstTime = false;
									}
									nvgClosePath(nvg);
									nvgFill(nvg);
									nvgStroke(nvg);
								}
								if (0.04f*scale > 0.5f&&simulation->crumbs.size()>0) {
									Breadcrumbs2D& crumbs=simulation->crumbs;
									int T=std::min((int)crumbs.size(),timelineSlider->getTimeValue().toInteger()+1);
									nvgFillColor(nvg, Color(220,220,220));
									nvgStrokeColor(nvg, Color(200,200,200));
									nvgStrokeWidth(nvg, 0.03f*scale);
									for(int t=std::max(1,T-8);t<T;t++) {
										int N=crumbs.size(t);
										for (int n = 0; n < N; n++) {
											float2i qt = crumbs(t,n);
											if (qt.index>=0) {
												float2 pt = qt + float2(0.5f);
												pt.x = pt.x / (float)img.width;
												pt.y = pt.y / (float)img.height;
												pt = pt*bounds.dimensions + bounds.position;
												nvgBeginPath(nvg);
												nvgMoveTo(nvg, pt.x, pt.y);
												qt = crumbs(t-1,qt.index);
												pt = qt + float2(0.5f);
												pt.x = pt.x / (float)img.width;
												pt.y = pt.y / (float)img.height;
												pt = pt*bounds.dimensions + bounds.position;
												nvgLineTo(nvg, pt.x, pt.y);
												nvgStroke(nvg);
											}
										}
									}
									for(int t=std::max(0,T-8);t<T;t++) {
										int N=crumbs.size(t);
										for (int n = 0;n < N;n++) {
											float2i qt = crumbs(t,n);
											if (qt.index>=0) {
												float2 pt = qt + float2(0.5f);
												pt.x = pt.x / (float)img.width;
												pt.y = pt.y / (float)img.height;
												pt = pt*bounds.dimensions + bounds.position;
												nvgBeginPath(nvg);
												nvgRect(nvg, pt.x-0.02f*scale, pt.y-0.02f*scale, 0.04f*scale, 0.04f*scale);
												nvgFill(nvg);
											}
										}
									}
								}
								if (0.1f*scale > 0.5f) {
									nvgStrokeColor(nvg, springlColor);
									nvgStrokeWidth(nvg, 0.1f*scale);
									for (int n = 0;n < (int)contour->vertexes.size();n += 2) {
										float2 pt = contour->vertexes[n] + float2(0.5f);
										pt.x = pt.x / (float)img.width;
										pt.y = pt.y / (float)img.height;
										pt = pt*bounds.dimensions + bounds.position;
										nvgBeginPath(nvg);
										nvgMoveTo(nvg, pt.x, pt.y);

										pt = contour->particles[n/2] + float2(0.5f);
										pt.x = pt.x / (float)img.width;
										pt.y = pt.y / (float)img.height;
										pt = pt*bounds.dimensions + bounds.position;
										nvgLineTo(nvg, pt.x, pt.y);

										pt = contour->vertexes[n + 1] + float2(0.5f);
										pt.x = pt.x / (float)img.width;
										pt.y = pt.y / (float)img.height;
										pt = pt*bounds.dimensions + bounds.position;

										nvgLineTo(nvg, pt.x, pt.y);
										nvgStroke(nvg);
									}
								}

								if (0.05f*scale > 0.5f) {
									nvgStrokeColor(nvg, normalColor);
									nvgStrokeWidth(nvg, 0.05f*scale);
									for (int n = 0;n < (int)contour->normals.size();n++) {
										float2 pt = contour->particles[n] + float2(0.5f);
										pt.x = pt.x / (float)img.width;
										pt.y = pt.y / (float)img.height;
										pt = pt*bounds.dimensions + bounds.position;
										nvgBeginPath(nvg);
										nvgMoveTo(nvg, pt.x, pt.y);
										pt = contour->particles[n] + SpringLevelSet2D::EXTENT*contour->normals[n] + float2(0.5f);
										pt.x = pt.x / (float)img.width;
										pt.y = pt.y / (float)img.height;
										pt = pt*bounds.dimensions + bounds.position;
										nvgLineTo(nvg, pt.x, pt.y);
										nvgStroke(nvg);
									}
								}
								if (0.05f*scale > 0.5f) {
									nvgFillColor(nvg, pointColor);
									for (int n = 0;n < (int)contour->vertexes.size();n++) {
										float2 pt = contour->vertexes[n] + float2(0.5f);
										pt.x = pt.x / (float)img.width;
										pt.y = pt.y / (float)img.height;
										pt = pt*bounds.dimensions + bounds.position;
										nvgBeginPath(nvg);
										nvgEllipse(nvg, pt.x, pt.y, 0.05f*scale, 0.05f*scale);
										nvgFill(nvg);
									}
								}

								if (0.1f*scale > 0.5f) {
									nvgFillColor(nvg, particleColor);
									for (int n = 0;n < (int)contour->particles.size();n++) {
										float2 pt = contour->particles[n] + float2(0.5f);
										pt.x = pt.x / (float)img.width;
										pt.y = pt.y / (float)img.height;
										pt = pt*bounds.dimensions + bounds.position;
										nvgBeginPath(nvg);
										nvgEllipse(nvg, pt.x, pt.y, 0.1f*scale, 0.1f*scale);
										nvgFill(nvg);
									}
								}
							}));
	GlyphRegionPtr glyphRegion = GlyphRegionPtr(
			new GlyphRegion("Image Region", imageGlyph, CoordPX(0.0f, 0.0f),
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

				float2 dims = float2(img.dimensions());
				cursor = aly::clamp(dims*(event.cursor - bounds.position) / bounds.dimensions, float2(0.0f), dims);

				context->requestPack();
				return true;
			};

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
void ActiveManifold2DEx::draw(AlloyContext* context) {
	if (running) {
		if (!simulation->step()) {
			running = false;
		}
	}
}

