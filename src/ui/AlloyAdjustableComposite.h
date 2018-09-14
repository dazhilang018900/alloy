/*
 * AlloyAdjustableComposite.h
 *
 *  Created on: Sep 13, 2018
 *      Author: blake
 */

#ifndef SRC_UI_ALLOYADJUSTABLECOMPOSITE_H_
#define SRC_UI_ALLOYADJUSTABLECOMPOSITE_H_
#include "ui/AlloyComposite.h"
namespace aly {
class AdjustableComposite: public Composite {
protected:
	pixel2 cursorDownPosition;
	box2px windowInitialBounds;
	bool resizing;
	WindowPosition winPos;
	bool resizeable;
public:
	bool isResizing() const {
		return resizing;
	}
	bool isResizeable() const {
		return resizeable;
	}
	virtual bool isDragEnabled() const override {
		if (resizeable) {
			return ((dragButton!=-1) && winPos == WindowPosition::Center);
		} else {
			return (dragButton!=-1);
		}
	}
	std::function<void(AdjustableComposite* composite, const box2px& bounds)> onResize;
	AdjustableComposite(const std::string& name, const AUnit2D& pos,
			const AUnit2D& dims, bool resizeable = true);
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)
			override;
	virtual void draw(AlloyContext* context) override;
};

typedef std::shared_ptr<AdjustableComposite> AdjustableCompositePtr;
}

#endif /* SRC_UI_ALLOYADJUSTABLECOMPOSITE_H_ */
