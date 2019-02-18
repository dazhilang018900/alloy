/*
 * CascadeEx.cpp
 *
 *  Created on: Feb 9, 2019
 *      Author: blake
 */

#include "CarouselEx.h"
#include "ui/AlloyDragComposite.h"
#include "ui/AlloyDrawUtil.h"
namespace aly {
CarouselEx::CarouselEx() :
		Application(1200, 800, "Carousel Layout Example") {
}
bool CarouselEx::init(Composite& rootNode) {
	hscroller=ScrollPanelPtr(new ScrollPane("horizontal scroller",CoordPX(0.0f,25.0f),CoordPerPX(1.0f,0.0f,0.0f,200.0f),Orientation::Horizontal));
	vscroller=ScrollPanelPtr(new ScrollPane("vertical scroller",CoordPerPX(0.5f,0.0f,-100.0f,250.0f),CoordPerPX(0.0f,1.0f,200.0f,-250.0f),Orientation::Vertical));
	DragCompositePtr hcomp=DragCompositePtr(new DragComposite("horizontal drag",CoordPX(0.0f,0.0f),CoordPercent(1.0f,1.0f),Orientation::Horizontal));
	DragCompositePtr vcomp=DragCompositePtr(new DragComposite("vertical drag",CoordPX(0.0f,0.0f),CoordPercent(1.0f,1.0f),Orientation::Vertical));
	hscroller->add(hcomp);
	vscroller->add(vcomp);
	rootNode.add(hscroller);
	rootNode.backgroundColor=MakeColor(getContext()->theme.DARKER);
	hscroller->setOrientation(Orientation::Horizontal);
	hscroller->borderWidth=UnitPX(1.0f);
	hscroller->borderColor=MakeColor(getContext()->theme.LIGHTER);
	const int N=16;
	for(int n=0;n<N;n++){
		DrawPtr region=DrawPtr(new Draw(MakeString()<<"["<<(char)('A'+n)<<"]",CoordPX(0.0f,0.0f),CoordPerPX(0.0f,1.0f,200.0f,0.0f)));
		region->onDraw=[=,n](AlloyContext* context,const box2px& bounds){
			NVGcontext* nvg = context->nvgContext;
			nvgBeginPath(nvg);
			nvgRoundedRect(nvg, bounds.position.x + 2, bounds.position.y + 2,
					bounds.dimensions.x - 4, bounds.dimensions.y - 4,context->theme.CORNER_RADIUS);
			nvgStrokeWidth(nvg, 2.0f);
			nvgFillColor(nvg,Color((-n+N)%N/(float)(N-1),((n*13+3)%N)/(float)(N-1),((n*43+2)%N)/(float)(N-1)));
			nvgStrokeColor(nvg, Color(1.0f, 0.0f, 0.0f));
			nvgFill(nvg);
			nvgTextAlign(nvg, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
			nvgFontSize(nvg, 30.0f);
			nvgFontFaceId(nvg, context->getFontHandle(FontType::Normal));
			drawText(nvg, bounds.position + bounds.dimensions*0.5f, region->getName(),FontStyle::Outline);
		};
		hcomp->add(region);
	}
	rootNode.add(vscroller);
	rootNode.backgroundColor=MakeColor(getContext()->theme.DARKER);
	vscroller->setOrientation(Orientation::Vertical);
	vscroller->borderWidth=UnitPX(1.0f);
	vscroller->borderColor=MakeColor(getContext()->theme.LIGHTER);
	for(int n=0;n<N;n++){
		DrawPtr region=DrawPtr(new Draw(MakeString()<<"["<<(char)('A'+n)<<"]",CoordPX(0.0f,0.0f),CoordPerPX(1.0f,0.0f,0.0f,200.0f)));
		region->onDraw=[=,n](AlloyContext* context,const box2px& bounds){
			NVGcontext* nvg = context->nvgContext;
			nvgBeginPath(nvg);
			nvgRoundedRect(nvg, bounds.position.x + 2, bounds.position.y + 2,
					bounds.dimensions.x - 4, bounds.dimensions.y - 4,context->theme.CORNER_RADIUS);
			nvgStrokeWidth(nvg, 2.0f);
			nvgFillColor(nvg,Color((-n+N)%N/(float)(N-1),((n*13+3)%N)/(float)(N-1),((n*43+2)%N)/(float)(N-1)));
			nvgStrokeColor(nvg, Color(1.0f, 0.0f, 0.0f));
			nvgFill(nvg);
			nvgTextAlign(nvg, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
			nvgFontSize(nvg, 30.0f);
			nvgFontFaceId(nvg, context->getFontHandle(FontType::Normal));
			drawText(nvg, bounds.position + bounds.dimensions*0.5f, region->getName(),FontStyle::Outline);
		};
		vcomp->add(region);
	}

	return true;
}


} /* namespace aly */
