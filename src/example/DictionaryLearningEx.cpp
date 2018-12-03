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
#include "image/AlloyGradientVectorFlow.h"
#include "image/AlloyDistanceField.h"
#include "graphics/AlloyIsoContour.h"
#include "example/DictionaryLearningEx.h"
#include "vision/DictionaryLearning.h"
#include "vision/SpringLevelSet2D.h"
#include "vision/SpringlsSecondOrder.h"
#include "vision/DeepDictionary.h"
using namespace aly;
DictionaryLearningEx::DictionaryLearningEx() :
		Application(1200, 1000, "Learning Toy", false) {
}
bool DictionaryLearningEx::init(Composite& rootNode) {
	ReadImageFromFile(getFullPath("images/mountains.jpg"), img);
	int patchSize = 16;
	int subsample = 16;
	int sparsity = 3;
	int angleSamples = 8;
	DeepDictionary dictionary({64,64},patchSize,sparsity,angleSamples);
	ImageRGB est;
	aly::Volume3f weights;
	//learning.estimate(img, est, weights, sparsity);
	//WriteImageToFile(MakeDesktopFile("estimate.png"), est);
	//WriteVolumeToFile(MakeDesktopFile("weights.xml"), weights);
	/*
	{
		aly::Image3f scores;
		aly::Image3f pool;
		int count = 0;
		for (FilterBank& b : learning.getFilterBanks()) {
			std::cout << "Writing Filter Bank " << count << std::endl;
			b.score(img, scores);
			learning.normalize(scores,0.05f);
			learning.maxPool(scores,pool,2);
			WriteImageToFile(
					MakeDesktopFile(
							MakeString() << "image_filtered_"
									<< ZeroPad(count, 3) << ".png"), pool);
			count++;
		}
	}
*/
	img = est;
	frameBuffersDirty = true;
	BorderCompositePtr layout = BorderCompositePtr(
			new BorderComposite("UI Layout", CoordPX(0.0f, 0.0f),
					CoordPercent(1.0f, 1.0f), false));
	CompositePtr infoComposite = CompositePtr(
			new Composite("Info", CoordPX(0.0f, 0.0f),
					CoordPercent(1.0f, 1.0f)));
	infoComposite->backgroundColor = MakeColor(getContext()->theme.DARKER);
	infoComposite->borderColor = MakeColor(getContext()->theme.DARK);
	infoComposite->borderWidth = UnitPX(0.0f);
	rootNode.add(layout);
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
	DrawPtr drawContour = DrawPtr(
			new Draw("Contour Draw", CoordPX(0.0f, 0.0f),
					CoordPercent(1.0f, 1.0f),
					[this](AlloyContext* context, const box2px& bounds) {
						/*
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

						 const std::vector<SamplePatch>& banks=learning.getSamplePatch();
						 if(lineWidth.toFloat()*0.1f*scale>0.5f) {
						 nvgStrokeColor(nvg, lineColor);
						 nvgStrokeWidth(nvg, lineWidth.toFloat()*0.1f*scale);
						 for(size_t idx=0;idx<banks.size();idx++) {
						 const SamplePatch& bank=banks[idx];
						 float2 u(1.0f,0.0f);
						 float2 v(0.0f,1.0f);
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
						 const SamplePatch& bank=banks[idx];
						 float2 pt = bank.position;
						 pt.x = pt.x / (float)img.width;
						 pt.y = pt.y / (float)img.height;
						 pt = pt*bounds.dimensions + bounds.position;
						 }
						 }
						 float r=scale*0.5f;
						 if(r>0.5f) {
						 nvgFillColor(nvg, pointColor);
						 for(size_t idx=0;idx<banks.size();idx++) {
						 const SamplePatch& bank=banks[idx];
						 float2 pt =bank.position;
						 pt.x = pt.x / (float)img.width;
						 pt.y = pt.y / (float)img.height;
						 pt = pt*bounds.dimensions + bounds.position;
						 nvgBeginPath(nvg);
						 nvgCircle(nvg,pt.x,pt.y,r);
						 nvgFill(nvg);
						 }
						 }
						 */
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

