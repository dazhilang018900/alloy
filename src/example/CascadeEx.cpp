/*
 * CascadeEx.cpp
 *
 *  Created on: Feb 9, 2019
 *      Author: blake
 */

#include "CascadeEx.h"

namespace aly {

CascadeEx::CascadeEx() :
		Application(800, 600, "Cascade Layout Example") {
}
bool CascadeEx::init(Composite& rootNode) {
	scroller=ScrollPanelPtr(new ScrollPanel("horizontal scroller",CoordPX(0.0f,100.0f),CoordPerPX(1.0f,0.0f,0.0f,400.0f),Orientation::Horizontal));
	int N=16;
	for(int n=0;n<N;n++){
		RegionPtr region=RegionPtr(new Region(MakeString()<<"R"<<n,CoordPX(0.0f,0.0f),CoordPerPX(0.0f,1.0f,200.0f,0.0f)));
		region->setRoundCorners(true);
		region->backgroundColor=MakeColor(Color((-n+N)%N/(float)(N-1),((n*13+3)%N)/(float)(N-1),((n*43+2)%N)/(float)(N-1)));
		scroller->addPanel(region);
	}

	rootNode.add(scroller);
	rootNode.backgroundColor=MakeColor(getContext()->theme.DARKER);
	return true;
}


} /* namespace aly */
