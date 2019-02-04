/*
 * AlloyButton.h
 *
 *  Created on: Sep 13, 2018
 *      Author: blake
 */

#ifndef SRC_UI_ALLOYBUTTON_H_
#define SRC_UI_ALLOYBUTTON_H_
#include "ui/AlloyComposite.h"
namespace aly{
class TextButton: public Region {
protected:
	bool truncate;
public:
	AColor textColor;
	AUnit1D fontSize;
	void setTruncate(bool t) {
		truncate = t;
	}
	TextButton(const std::string& label, const AUnit2D& position,
			const AUnit2D& dimensions,bool truncate=true);
	virtual void draw(AlloyContext* context) override;
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)
			override;
	virtual inline ~TextButton() {
	}
};

class TextIconButton: public Composite {
private:
	std::string iconCodeString;
	IconAlignment iconAlignment;
	HorizontalAlignment alignment;
	bool truncate;
	std::string label;
public:
	AColor textColor;
	AUnit1D fontSize;
	void setTruncate(bool t) {
		truncate = t;
	}
	void setLabel(const std::string& label);
	void setIcon(int code);
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)
			override;
	TextIconButton(const std::string& label, int iconCode,
			const AUnit2D& position, const AUnit2D& dimensions,
			const HorizontalAlignment& alignment = HorizontalAlignment::Center,
			const IconAlignment& iconAlignment = IconAlignment::Left,bool truncate=true);
	virtual void draw(AlloyContext* context) override;
	virtual inline ~TextIconButton() {
	}
};

enum class IconType {
	CIRCLE, SQUARE
};
class IconButton: public Composite {
private:
	std::string iconCodeString;
	IconType iconType;
	bool truncate;
	bool rescale;
	pixel2 nudge;
	pixel nudgeSize;
public:
	AColor foregroundColor;
	AColor iconColor;
	void setNudgeSize(pixel nz) {
		nudgeSize = nz;
	}
	void setNudgePosition(pixel2 n) {
		nudge = n;
	}
	void setTruncate(bool t) {
		truncate = t;
	}
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)
			override;
	IconButton(int iconCode, const AUnit2D& position, const AUnit2D& dimensions,
			IconType iconType = IconType::SQUARE,bool truncate=true);
	void setIcon(int iconCode);
	void setRescaleOnHover(bool r){
		rescale=r;
	}
	virtual void draw(AlloyContext* context) override;
	virtual inline ~IconButton() {
	}
};
typedef std::shared_ptr<TextButton> TextButtonPtr;
typedef std::shared_ptr<TextIconButton> TextIconButtonPtr;
typedef std::shared_ptr<IconButton> IconButtonPtr;
}

#endif /* SRC_UI_ALLOYBUTTON_H_ */
