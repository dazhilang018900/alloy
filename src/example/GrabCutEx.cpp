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

#include <example/GrabCutEx.h>
#include "Alloy.h"
#include "AlloyGaussianMixture.h"
#include "AlloyMaxFlow.h"
#include "AlloyMath.h"
using namespace aly;
GrabCutEx::GrabCutEx() : Application(900, 600, "Grab Cut Example") {
	cycle = 0;
	colorDiff = 0.03f;
	maxDist = 6.0f;
}
void GrabCutEx::initSolver(aly::ImageRGBA& image, const aly::box2f& region) {
	const int nbrX[] = { 0, 0, 1, -1 };
	const int nbrY[] = { 1, -1, 0, 0 };
	GaussianMixtureRGB fgModel, bgModel;
	std::vector<RGBf> fgSamples, bgSamples;
	for (int j = 0; j < image.height; j++) {
		for (int i = 0; i < image.width; i++) {
			if (region.contains(float2(i, j))) {
				fgSamples.push_back(ToRGBf(image(i, j)));
			} else {
				bgSamples.push_back(ToRGBf(image(i, j)));
			}
		}
	}
	fgModel.solve(fgSamples, 5, 128, 32);
	bgModel.solve(bgSamples, 5, 128, 32);
	maxFlow.reset();
	maxFlow.resize(image.width * image.height);
	for (int j = 0; j < image.height; j++) {
		for (int i = 0; i < image.width; i++) {
			RGBf c = ToRGBf(image(i, j));
			float2 sc;
			sc.x = fgModel.distanceMahalanobis(c);
			sc.y = bgModel.distanceMahalanobis(c);
			int id = i + j * image.width;

			for (int k = 0; k < 4; k++) {
				int ii = i + nbrX[k];
				int jj = j + nbrY[k];
				if (ii >= 0 && jj >= 0 && ii < image.width
						&& jj < image.height) {
					aly::RGBf cc = ToRGBf(image(ii, jj));
					float w = std::exp(-lengthL1(c - cc) * 0.3333f / colorDiff);
					maxFlow.addEdge(id, ii + image.width * jj, w, w);
				}
			}
			maxFlow.addSourceCapacity(id,
					aly::clamp(sc.x, 0.0f, maxDist) / maxDist);
			maxFlow.addSinkCapacity(id,
					aly::clamp(sc.y, 0.0f, maxDist) / maxDist);
		}
	}
	maxFlow.initialize();
}
void GrabCutEx::initSolver(aly::ImageRGBA& image) {

	const int nbrX[] = { 0, 0, 1, -1 };
	const int nbrY[] = { 1, -1, 0, 0 };
	GaussianMixtureRGB fgModel, bgModel;
	std::vector<RGBf> fgSamples, bgSamples;
	for (int j = 0; j < image.height; j++) {
		for (int i = 0; i < image.width; i++) {
			RGBAf c = ToRGBAf(image(i, j));
			if (c.w > 0.5f) {
				fgSamples.push_back(c.xyz());
			} else {
				bgSamples.push_back(c.xyz());
			}
		}
	}
	fgModel.solve(fgSamples, 5, 128, 32);
	bgModel.solve(bgSamples, 5, 128, 32);
	maxFlow.reset();
	maxFlow.resize(image.width * image.height);
	for (int j = 0; j < image.height; j++) {
		for (int i = 0; i < image.width; i++) {
			RGBf c = ToRGBf(image(i, j));
			float2 sc;
			sc.x = fgModel.distanceMahalanobis(c);
			sc.y = bgModel.distanceMahalanobis(c);
			int id = i + j * image.width;

			for (int k = 0; k < 4; k++) {
				int ii = i + nbrX[k];
				int jj = j + nbrY[k];
				if (ii >= 0 && jj >= 0 && ii < image.width
						&& jj < image.height) {
					aly::RGBf cc = ToRGBf(image(ii, jj));
					float w = std::exp(-lengthL1(c - cc) * 0.3333f / colorDiff);
					maxFlow.addEdge(id, ii + image.width * jj, w, w);
				}
			}
			maxFlow.addSourceCapacity(id,
					aly::clamp(sc.x, 0.0f, maxDist) / maxDist);
			maxFlow.addSinkCapacity(id,
					aly::clamp(sc.y, 0.0f, maxDist) / maxDist);
		}
	}
	maxFlow.initialize();
}
bool GrabCutEx::init(Composite& rootNode) {
	ReadImageFromFile(getFullPath("images/flowers.png"), image);
	/*
	{
		ImageRGBA tmp;
		DownSample3x3(image, tmp);
		image = tmp;
	}
*/
	float2 dims = float2(image.dimensions());
	selectedRegion = box2f(float2(25.0f, 25.0f), float2(460.0f, 320.0f));
	initSolver(image, selectedRegion);
	DrawPtr drawRegion =
			MakeShared<Draw>("Draw", CoordPX(0, 0), CoordPercent(1.0f, 1.0f),
					[this,dims](AlloyContext* context, const box2px& bounds) {
						NVGcontext* nvg=context->nvgContext;
						nvgStrokeWidth(nvg,3.0f);
						nvgStrokeColor(nvg,Color(100,100,200));
						nvgBeginPath(nvg);
						float2 pos=bounds.position+selectedRegion.position*bounds.dimensions/dims;
						float2 sz=selectedRegion.dimensions*bounds.dimensions/dims;
						nvgRect(nvg,pos.x,pos.y,sz.x,sz.y);
						nvgStroke(nvg);

						if(cycle<MAX_CYCLES) {
							int iter = 0;
							while (maxFlow.step()&&iter<4*image.width) {
								iter++;
							}
							if(iter>0) {
								std::vector<MaxFlow::Node>& nodes = maxFlow.getNodes();
								for (int n = 0; n < nodes.size(); n++) {
									MaxFlow::Node& node = nodes[n];
									image[n].w = static_cast<uint8_t>(node.type)*255;
								}
								imageGlyph->set(image,context);
							} else {
								initSolver(image);
								cycle++;
							}
						}
					});
	drawRegion->setAspectRatio(image.width / (float) image.height);
	drawRegion->setAspectRule(AspectRule::FixedHeight);
	//Use nanovg to draw
	GlyphRegionPtr imageRegion = MakeGlyphRegion(
			imageGlyph = createImageGlyph(image), CoordPX(0.0f, 0.0f),
			CoordPercent(1.0f, 1.0f), AspectRule::FixedHeight, COLOR_NONE,
			COLOR_NONE, Color(200, 200, 200, 255), UnitPX(1.0f));
	imageRegion->setAspectRatio(image.width / (float) image.height);
	imageRegion->setAspectRule(AspectRule::FixedHeight);
	imageRegion->backgroundColor = MakeColor(Color(100, 200, 100));
	rootNode.add(imageRegion);
	rootNode.add(drawRegion);
	return true;
}

