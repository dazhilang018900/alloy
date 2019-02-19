/*
 * AlloyScrollPanel.h
 *
 *  Created on: Feb 9, 2019
 *      Author: blake
 */

#ifndef SRC_UI_ALLOYSCROLLPANE_H_
#define SRC_UI_ALLOYSCROLLPANE_H_

#include "ui/AlloyComposite.h"
#include "ui/AlloyButton.h"
namespace aly {


class ScrollPane :public Composite {
protected:
	ArrowButtonPtr leftButton;
	ArrowButtonPtr rightButton;
	ArrowButtonPtr upButton;
	ArrowButtonPtr downButton;
	Timer timer;
public:
	virtual void updateCursor(CursorLocator* cursorLocator) override;
	virtual void draw(AlloyContext* context) override;
	virtual void pack(const pixel2& pos, const pixel2& dims, const double2& dpmm,double pixelRatio, bool clamp = false) override;
	virtual void drawDebug(AlloyContext* context) override;
	ScrollPane(const std::string& name, const AUnit2D& pos, const AUnit2D& dims, const Orientation& orient,float scrollStep=10.0f,float buttonWidth=40.0f);
	virtual ~ScrollPane(){}
};
typedef std::shared_ptr<ScrollPane> ScrollPanelPtr;
} /* namespace aly */
#endif /* SRC_UI_ALLOYSCROLLPANE_H_ */
