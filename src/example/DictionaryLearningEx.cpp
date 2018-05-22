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
#include <AlloyGradientVectorFlow.h>
#include "Alloy.h"
#include "AlloyDistanceField.h"
#include "AlloyIsoContour.h"
#include "example/DictionaryLearningEx.h"
#include "ml/DictionaryLearning.h"
#include "segmentation/SpringLevelSet2D.h"
#include "segmentation/SpringlsSecondOrder.h"
using namespace aly;
DictionaryLearningEx::DictionaryLearningEx() :
		Application(1200, 1000, "Learning Toy", false) {
}
bool DictionaryLearningEx::init(Composite& rootNode) {
	ReadImageFromFile(getFullPath("images/stereo_left.png"), img);
	int patchSize = 32;
	int subsample = 32;
	int filters = 32;
	int sparsity = 4;
	/*
	ImageRGBA tmp;
	*/
	/*
	std::vector<std::string> outputFiles = GetDirectoryFileListing(
			"/home/blake/Downloads/gt_training_lowres", "png","GT02");
	std::vector<std::string> imageFiles = GetDirectoryFileListing(
			"/home/blake/Downloads/input_training_lowres", "png","GT02");
	std::string outputFilterFile = MakeString() << GetDesktopDirectory()
			<< ALY_PATH_SEPARATOR<<"alpha_filterbanks.xml";
	std::string inputFilterFile = MakeString() << GetDesktopDirectory()
			<< ALY_PATH_SEPARATOR<<"image_filterbanks.xml";
*/
	 /*
	 std::vector<ImageRGBA> pyramid(5);
	 ReadImageFromFile(getFullPath("images/blake.png"),pyramid[0]);
	 for(int p=1;p<pyramid.size();p++){
	 DownSample3x3(pyramid[p-1],pyramid[p]);
	 }

	std::vector<ImageRGBA> outputImages(outputFiles.size());
	std::vector<ImageRGBA> inputImages(inputFilterFile.size());
	int n = 0;
	for (std::string imgFile : outputFiles) {
		ReadImageFromFile(imgFile, outputImages[n]);
		n++;
	}
	n = 0;
	for (std::string imgFile : imageFiles) {
		ReadImageFromFile(imgFile, inputImages[n]);
		n++;
	}
	std::cout << "Segmentation Images " << outputImages.size() << std::endl;

	std::vector<FilterBank> inputFilters,outputFilters;
	if (!FileExists(outputFilterFile)) {
		learning.train(outputImages, filters, subsample, patchSize, patchSize);
		learning.writeFilterBanks(
				MakeString() << GetDesktopDirectory() << ALY_PATH_SEPARATOR<<"output_filters.png");
		outputFilters=learning.filterBanks;
	} else {
		ReadFilterBanksFromFile(outputFilterFile,outputFilters);
	}
	if (!FileExists(inputFilterFile)) {
		learning.train(inputImages, filters, subsample, patchSize, patchSize);
		learning.writeFilterBanks(
				MakeString() << GetDesktopDirectory() << ALY_PATH_SEPARATOR<<"input_filters.png");
		inputFilters=learning.filterBanks;
		learning.write(inputFilterFile);
	} else {
		ReadFilterBanksFromFile(inputFilterFile,inputFilters);
	}
*/

	//learning.writeEstimatedPatches(MakeString()<<GetDesktopDirectory()<<ALY_PATH_SEPARATOR<<"reconstruction.png",samples[0].width/patchSize,samples[0].height/patchSize);


	 learning.train({img},filters,subsample,patchSize,patchSize,sparsity);
	 learning.writeFilterBanks(MakeString()<<GetDesktopDirectory()<<ALY_PATH_SEPARATOR<<"filters1.png");
	 learning.writeEstimatedPatches(MakeString()<<GetDesktopDirectory()<<ALY_PATH_SEPARATOR<<"reconstruction.png",img.width/subsample,img.height/subsample);

	 /*
	 DownSample3x3(img,tmp);
	 learning.train({tmp},32,patchSize,patchSize,patchSize);
	 learning.writeFilterBanks(MakeString()<<GetDesktopDirectory()<<ALY_PATH_SEPARATOR<<"filters2.png");
	 learning.writeEstimatedPatches(MakeString()<<GetDesktopDirectory()<<ALY_PATH_SEPARATOR<<"reconstruction2.png",tmp.width/patchSize,tmp.height/patchSize);

	 DownSample3x3(tmp,img);
	 learning.train({img},32,patchSize,patchSize,patchSize);
	 learning.writeFilterBanks(MakeString()<<GetDesktopDirectory()<<ALY_PATH_SEPARATOR<<"filters3.png");
	 learning.writeEstimatedPatches(MakeString()<<GetDesktopDirectory()<<ALY_PATH_SEPARATOR<<"reconstruction3.png",img.width/patchSize,img.height/patchSize);

	 DownSample3x3(img,tmp);
	 learning.train({tmp},32,patchSize,patchSize,patchSize);
	 learning.writeFilterBanks(MakeString()<<GetDesktopDirectory()<<ALY_PATH_SEPARATOR<<"filters4.png");
	 learning.writeEstimatedPatches(MakeString()<<GetDesktopDirectory()<<ALY_PATH_SEPARATOR<<"reconstruction4.png",tmp.width/patchSize,tmp.height/patchSize);

	 DownSample3x3(tmp,img);
	 learning.train({img},32,patchSize,patchSize,patchSize);
	 learning.writeFilterBanks(MakeString()<<GetDesktopDirectory()<<ALY_PATH_SEPARATOR<<"filters5.png");
	 learning.writeEstimatedPatches(MakeString()<<GetDesktopDirectory()<<ALY_PATH_SEPARATOR<<"reconstruction5.png",img.width/patchSize,img.height/patchSize);
	 */
	 std::exit(0);
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

	lineWidth = Float(1.0f);
	particleSize = Float(0.2f);

	lineColor = Color(0.0f, 0.2f, 0.5f, 1.0f);
	pointColor = Color(1.0f, 0.0f, 0.0f, 1.0f);
	springlColor = Color(0.5f, 0.5f, 0.5f, 1.0f);
	matchColor = Color(0.5f, 0.5f, 1.0f, 0.75f);
	particleColor = Color(0.6f, 0.0f, 0.0f, 1.0f);
	normalColor = Color(0.0f, 0.8f, 0.0f, 1.0f);
	vecfieldColor = Color(0.8f, 0.4f, 0.8f, 0.5f);
	controls->setAlwaysShowVerticalScrollBar(false);
	controls->setScrollEnabled(false);
	controls->backgroundColor = MakeColor(getContext()->theme.DARKER);
	controls->borderColor = MakeColor(getContext()->theme.DARK);
	controls->borderWidth = UnitPX(1.0f);

	controlLayout->backgroundColor = MakeColor(getContext()->theme.DARKER);
	controlLayout->borderWidth = UnitPX(0.0f);
	layout->setWest(controlLayout, UnitPX(400.0f));
	controlLayout->setCenter(controls);
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
	controls->addGroup("Visualization", true);
	controls->addNumberField("Line Width", lineWidth, Float(1.0f), Float(20.0f),
			6.0f);
	CompositePtr viewRegion = CompositePtr(
			new Composite("View", CoordPX(0.0f, 0.0f),
					CoordPercent(1.0f, 1.0f)));
	layout->setCenter(viewRegion);
	float downScale = 1.5f
			* std::min((getContext()->getScreenWidth() - 350.0f) / img.width,
					((float) getContext()->getScreenHeight()) / img.height);
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

								const std::vector<OrientedPatch>& banks=learning.patches;
								if(lineWidth.toFloat()*0.1f*scale>0.5f) {
									nvgStrokeColor(nvg, lineColor);
									nvgStrokeWidth(nvg, lineWidth.toFloat()*0.1f*scale);
									for(size_t idx=0;idx<banks.size();idx++) {
										const OrientedPatch& bank=banks[idx];
										float2 u=MakeOrthogonalComplement(bank.normal)*(float)bank.width*0.5f;
										float2 v=bank.normal*(float)bank.height*0.5f;
										nvgBeginPath(nvg);

										float2 pt = bank.position+u+v;
										pt.x = pt.x / (float)img.width;
										pt.y = pt.y / (float)img.height;
										pt = pt*bounds.dimensions + bounds.position;
										nvgMoveTo(nvg, pt.x, pt.y);

										pt = bank.position+u-v;
										pt.x = pt.x / (float)img.width;
										pt.y = pt.y / (float)img.height;
										pt = pt*bounds.dimensions + bounds.position;
										nvgLineTo(nvg, pt.x, pt.y);

										pt = bank.position-u-v;
										pt.x = pt.x / (float)img.width;
										pt.y = pt.y / (float)img.height;
										pt = pt*bounds.dimensions + bounds.position;
										nvgLineTo(nvg, pt.x, pt.y);

										pt = bank.position-u+v;
										pt.x = pt.x / (float)img.width;
										pt.y = pt.y / (float)img.height;
										pt = pt*bounds.dimensions + bounds.position;
										nvgLineTo(nvg, pt.x, pt.y);
										nvgClosePath(nvg);
										nvgStroke(nvg);
									}
								}
								if(scale*0.1f>0.5f) {
									nvgStrokeColor(nvg, normalColor);
									nvgStrokeWidth(nvg, scale*0.1f);
									for(size_t idx=0;idx<banks.size();idx++) {
										const OrientedPatch& bank=banks[idx];
										float2 pt = bank.position;
										pt.x = pt.x / (float)img.width;
										pt.y = pt.y / (float)img.height;
										pt = pt*bounds.dimensions + bounds.position;
										nvgBeginPath(nvg);
										nvgMoveTo(nvg, pt.x, pt.y);
										pt =bank.position + bank.normal*4.0f;
										pt.x = pt.x / (float)img.width;
										pt.y = pt.y / (float)img.height;
										pt = pt*bounds.dimensions + bounds.position;
										nvgLineTo(nvg, pt.x, pt.y);
										nvgStroke(nvg);
									}
								}
								float r=scale*0.5f;
								if(r>0.5f) {
									nvgFillColor(nvg, pointColor);
									for(size_t idx=0;idx<banks.size();idx++) {
										const OrientedPatch& bank=banks[idx];
										float2 pt =bank.position;
										pt.x = pt.x / (float)img.width;
										pt.y = pt.y / (float)img.height;
										pt = pt*bounds.dimensions + bounds.position;
										nvgBeginPath(nvg);
										nvgCircle(nvg,pt.x,pt.y,r);
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
	drawContour->onScroll =
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
void DictionaryLearningEx::draw(AlloyContext* context) {

}

