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
#include "AlloyAnisotropicFilter.h"
#include "../../include/example/AnisotropicFilterEx.h"
using namespace aly;
AnisotropicFilterEx::AnisotropicFilterEx() :
		Application(970, 550, "Anisotropic Filter Example"){
}
bool AnisotropicFilterEx::init(Composite& rootNode) {
	ImageRGBAf img,out;
	ReadImageFromFile(getFullPath("images/sfmarket.png"),out);
	DownSample(out,img);
	AnisotropicDiffusion(img,out,3,AnisotropicKernel::Gaussian,0.02f,1.0f);
	imageGlyph=createImageGlyph(out);
	imageRegion = MakeGlyphRegion(imageGlyph, CoordPerPX(0.5f,0.5f,-out.width/2,-out.height/2), CoordPX(out.width, out.height), AspectRule::FixedHeight,COLOR_NONE, COLOR_NONE, Color(200, 200, 200, 255), UnitPX(1.0f));
	imageRegion->setDragEnabled(true);
	imageRegion->setClampDragToParentBounds(false);
	imageRegion->onScroll = [this](AlloyContext* context, const InputEvent& event)
	{
		box2px bounds = imageRegion->getBounds(false);
		pixel scaling = (pixel)(1 - 0.1f*event.scroll.y);
		pixel2 newBounds = bounds.dimensions*scaling;
		pixel2 cursor = context->cursorPosition;
		pixel2 relPos = (cursor - bounds.position) / bounds.dimensions;
		pixel2 newPos = cursor - relPos*newBounds;
		bounds.position = newPos;
		bounds.dimensions = newBounds;
		imageRegion->setDragOffset(pixel2(0, 0));
		imageRegion->position = CoordPX(bounds.position - imageRegion->parent->getBoundsPosition());
		imageRegion->dimensions = CoordPX(bounds.dimensions);
		float2 dims = float2(imageGlyph->width,imageGlyph->height);
		cursor = aly::clamp(dims*(event.cursor - bounds.position) / bounds.dimensions, float2(0.0f), dims);
		context->requestPack();
		return true;
	};
	rootNode.add(imageRegion);
	return true;
}
void AnisotropicFilterEx::draw(aly::AlloyContext* context) {
}

