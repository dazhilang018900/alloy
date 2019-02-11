/*
 * CascadeEx.cpp
 *
 *  Created on: Feb 9, 2019
 *      Author: blake
 */

#include "CarouselEx.h"
namespace aly {
CarouselEx::CarouselEx() :
		Application(800, 600, "Carousel Layout Example") {
}
bool CarouselEx::init(Composite& rootNode) {
	hscroller=ScrollPanelPtr(new ScrollPane("horizontal scroller",CoordPX(0.0f,25.0f),CoordPerPX(1.0f,0.0f,0.0f,200.0f),Orientation::Horizontal));
	rootNode.add(hscroller);
	rootNode.backgroundColor=MakeColor(getContext()->theme.DARKER);
	hscroller->setOrientation(Orientation::Horizontal);
	hscroller->borderWidth=UnitPX(1.0f);
	hscroller->borderColor=MakeColor(getContext()->theme.LIGHTER);
	const int N=16;
	for(int n=0;n<N;n++){
		RegionPtr region=RegionPtr(new Region(MakeString()<<"R"<<n,CoordPX(0.0f,0.0f),CoordPerPX(0.0f,1.0f,200.0f,0.0f)));
		region->setRoundCorners(true);
		region->backgroundColor=MakeColor(Color((-n+N)%N/(float)(N-1),((n*13+3)%N)/(float)(N-1),((n*43+2)%N)/(float)(N-1)));
		hscroller->add(region);
	}
	vscroller=ScrollPanelPtr(new ScrollPane("vertical scroller",CoordPerPX(0.5f,0.0f,-100.0f,250.0f),CoordPerPX(0.0f,1.0f,200.0f,-250.0f),Orientation::Vertical));
	rootNode.add(vscroller);
	rootNode.backgroundColor=MakeColor(getContext()->theme.DARKER);
	vscroller->setOrientation(Orientation::Vertical);
	vscroller->borderWidth=UnitPX(1.0f);
	vscroller->borderColor=MakeColor(getContext()->theme.LIGHTER);
	for(int n=0;n<N;n++){
		RegionPtr region=RegionPtr(new Region(MakeString()<<"R"<<n,CoordPX(0.0f,0.0f),CoordPerPX(1.0f,0.0f,0.0f,200.0f)));
		region->setRoundCorners(true);
		region->backgroundColor=MakeColor(Color((-n+N)%N/(float)(N-1),((n*13+3)%N)/(float)(N-1),((n*43+2)%N)/(float)(N-1)));
		vscroller->add(region);
	}
	return true;
}


} /* namespace aly */
