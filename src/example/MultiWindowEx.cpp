/*
 * MultiWindowEx.cpp
 *
 *  Created on: Mar 29, 2019
 *      Author: blake
 */

#include "MultiWindowEx.h"

namespace aly {
MultiWindowEx::MultiWindowEx() :
		Application("Primary Window", 800, 600) {
	WindowPtr main=getMainWindow();
	main->setLocation(100,100);
	WindowPtr win=addWindow("Second Window", 400, 400, true);
	win->setLocation(950,150);
}
bool MultiWindowEx::init(aly::Composite& rootNode) {
	//init() called on each window. Identify windows by composite name.
	if (rootNode.getName() == "Second Window") {
		rootNode.backgroundColor=MakeColor(0.5f,0.5f,0.5f);
		TextLabelPtr label = TextLabelPtr(
				new TextLabel("Secondary",
						CoordPerPX(0.5f, 0.5f, -100.0f, -30.0f), CoordPX(200, 60)));
		label->setAlignment(HorizontalAlignment::Center,VerticalAlignment::Middle);
		label->backgroundColor = MakeColor(Color(1.0f, 0.8f, 0.2f));
		label->textColor = MakeColor(Color(0.1f, 0.1f, 0.1f));
		rootNode.add(label);
	} else {
		rootNode.backgroundColor=MakeColor(0.8f,0.8f,0.8f);
		TextLabelPtr label = TextLabelPtr(
				new TextLabel("Primary",
						CoordPerPX(0.5f, 0.5f, -100.0f, -30.0f), CoordPX(200, 60)));
		label->setAlignment(HorizontalAlignment::Center,VerticalAlignment::Middle);
		label->backgroundColor = MakeColor(Color(0.2f, 0.8f, 1.0f));
		label->textColor = MakeColor(Color(0.1f, 0.1f, 0.1f));
		rootNode.add(label);
	}
	return true;
}
void MultiWindowEx::draw(aly::AlloyContext* context) {

}

} /* namespace aly */
