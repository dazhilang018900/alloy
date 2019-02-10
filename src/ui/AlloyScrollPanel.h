/*
 * AlloyScrollPanel.h
 *
 *  Created on: Feb 9, 2019
 *      Author: blake
 */

#ifndef SRC_UI_ALLOYSCROLLPANEL_H_
#define SRC_UI_ALLOYSCROLLPANEL_H_

#include "ui/AlloyComposite.h"
#include "ui/AlloyButton.h"
namespace aly {

class ArrowButton: public Region {
protected:
	Direction dir;
public:
	ArrowButton(const std::string& label, const AUnit2D& position, const AUnit2D& dimensions,const Direction& dir);
	virtual void draw(AlloyContext* context) override;
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event) override;
	virtual inline ~ArrowButton() {}
};
typedef std::shared_ptr<ArrowButton> ArrowButtonPtr;

class ScrollPanel :public Composite {
protected:
	ArrowButtonPtr leftButton;
	ArrowButtonPtr rightButton;
	ArrowButtonPtr upButton;
	ArrowButtonPtr downButton;
	CompositePtr content;
public:
	virtual void draw(AlloyContext* context) override;
	ScrollPanel(const std::string& name, const AUnit2D& pos, const AUnit2D& dims, const Orientation& orient);
	void addPanel(const RegionPtr& ptr);
	virtual ~ScrollPanel(){}
};
typedef std::shared_ptr<ScrollPanel> ScrollPanelPtr;
} /* namespace aly */
#endif /* SRC_UI_ALLOYSCROLLPANEL_H_ */
