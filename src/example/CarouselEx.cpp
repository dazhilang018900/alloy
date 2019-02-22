/*
 * CascadeEx.cpp
 *
 *  Created on: Feb 9, 2019
 *      Author: blake
 */

#include "CarouselEx.h"0.0f, 0.0f, 0.0f
#include "ui/AlloyDragComposite.h"
#include "ui/AlloyDrawUtil.h"
namespace aly {
CarouselEx::CarouselEx() :
		Application(1200, 800, "Carousel Layout Example") {
}
bool CarouselEx::init(Composite& rootNode) {
	hscroller = DragCompositePtr(
			new DragComposite("horizontal scroller", CoordPX(0.0f, 0.0f),
					CoordPerPX(1.0f, 0.0f, 0.0f, 200.0f),
					Orientation::Horizontal));
	vscroller = DragCompositePtr(
			new DragComposite("vertical scroller",
					CoordPX( 0.0f, 200.0f),
					CoordPerPX(0.0f, 1.0f, 200.0f, -200.0f),
					Orientation::Vertical));
	hvscroller = DragBinCompositePtr(
			new DragBinComposite("vertical scroller",
					CoordPX(200.0f, 200.0f),
					CoordPerPX(1.0f, 1.0f, -200.0f, -200.0f),
					Orientation::Horizontal));
	hscroller->setSlimScroll(true);
	rootNode.add(hscroller);
	hscroller->backgroundColor = MakeColor(getContext()->theme.DARKER);
	const int N = 16;
	for (int n = 0; n < N; n++) {
		DrawPtr region = DrawPtr(
				new Draw(MakeString() << "[" << (char) ('A' + n) << "]",
						CoordPX(0.0f, 0.0f),
						CoordPerPX(0.0f, 1.0f, 200.0f, 0.0f)));
		region->onDraw =
				[=,n](AlloyContext* context,const box2px& bounds) {
					NVGcontext* nvg = context->nvgContext;
					nvgBeginPath(nvg);
					nvgRoundedRect(nvg, bounds.position.x + 3, bounds.position.y + 3,
							bounds.dimensions.x - 6, bounds.dimensions.y - 6,context->theme.CORNER_RADIUS);
					nvgStrokeWidth(nvg, 2.0f);
					nvgFillColor(nvg,Color((-n+N)%N/(float)(N-1),((n*13+3)%N)/(float)(N-1),((n*43+2)%N)/(float)(N-1)));
					nvgStrokeColor(nvg, Color(1.0f, 0.0f, 0.0f));
					nvgFill(nvg);
					nvgTextAlign(nvg, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
					nvgFontSize(nvg, 30.0f);
					nvgFontFaceId(nvg, context->getFontHandle(FontType::Normal));
					float2 offset(0.0f,Composite::scrollBarSize);
					drawText(nvg, bounds.position + (bounds.dimensions-offset)*0.5f, region->getName(),FontStyle::Outline);
				};
		hscroller->add(region);
	}
	rootNode.add(vscroller);
	vscroller->backgroundColor = MakeColor(getContext()->theme.DARK);
	for (int n = 0; n < N; n++) {
		DrawPtr region = DrawPtr(
				new Draw(MakeString() << "[" << (char) ('A' + n) << "]",
						CoordPX(0.0f, 0.0f),
						CoordPerPX(1.0f, 0.0f, 0.0f, 200.0f-Composite::scrollBarSize)));
		region->onDraw =
				[=,n](AlloyContext* context,const box2px& bounds) {
					NVGcontext* nvg = context->nvgContext;
					nvgBeginPath(nvg);
					nvgRoundedRect(nvg, bounds.position.x + 2, bounds.position.y + 2,
							bounds.dimensions.x - 4-Composite::scrollBarSize, bounds.dimensions.y - 4,context->theme.CORNER_RADIUS);
					nvgStrokeWidth(nvg, 2.0f);
					nvgFillColor(nvg,Color((-n+N)%N/(float)(N-1),((n*13+3)%N)/(float)(N-1),((n*43+2)%N)/(float)(N-1)));
					nvgStrokeColor(nvg, Color(1.0f, 0.0f, 0.0f));
					nvgFill(nvg);
					nvgTextAlign(nvg, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
					nvgFontSize(nvg, 30.0f);
					nvgFontFaceId(nvg, context->getFontHandle(FontType::Normal));
					float2 offset(Composite::scrollBarSize,0.0f);
					drawText(nvg, bounds.position + (bounds.dimensions-offset)*0.5f, region->getName(),FontStyle::Outline);
				};
		vscroller->add(region);
	}
	rootNode.add(hvscroller);
	hvscroller->backgroundColor = MakeColor(getContext()->theme.DARKER);
	rootNode.backgroundColor= MakeColor(getContext()->theme.DARKEST);
	std::vector<int> binSizes = { 1, 2, 0, 4, 3,7,8,3,2,1,6,5,4 };
	for (int n = 0; n < binSizes.size(); n++) {
		DragCompositePtr bin = hvscroller->addBin(
				MakeString() << "[" << (char) ('A' + n) << "]", 100);
		for (int m = 0; m < binSizes[n]; m++) {
			DrawPtr region = DrawPtr(
					new Draw(MakeString() << "[" << (char) ('A' + n) <<":"<<m<< "]",
							CoordPX(0.0f, 0.0f),
							CoordPerPX(1.0f, 0.0f, 0.0f, 100.0f)));
			region->onDraw =
					[=,n,m](AlloyContext* context,const box2px& bounds) {
						NVGcontext* nvg = context->nvgContext;
						nvgBeginPath(nvg);
						nvgRoundedRect(nvg, bounds.position.x + 3, bounds.position.y + 3,bounds.dimensions.x - 6, bounds.dimensions.y - 6,context->theme.CORNER_RADIUS);
						nvgStrokeWidth(nvg, 2.0f);
						nvgFillColor(nvg,Color((-n+N)%N/(float)(N-1),((m*13+3)%N)/(float)(N-1),((((m+1)*(n+1))*43+2)%N)/(float)(N-1)));
						nvgStrokeColor(nvg, Color(1.0f, 0.0f, 0.0f));
						nvgFill(nvg);
						nvgTextAlign(nvg, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
						nvgFontSize(nvg, 30.0f);
						nvgFontFaceId(nvg, context->getFontHandle(FontType::Normal));
						float2 offset(Composite::scrollBarSize,0.0f);
						drawText(nvg, bounds.position + (bounds.dimensions-offset)*0.5f, region->getName(),FontStyle::Outline);
					};
			bin->add(region);
		}
	}
	return true;
}

} /* namespace aly */
