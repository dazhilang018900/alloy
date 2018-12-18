/*
 * Copyright(C) 2018, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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

#ifndef SRC_UI_ALLOYTEXTWIDGET_H_
#define SRC_UI_ALLOYTEXTWIDGET_H_
#include "ui/AlloyComposite.h"
namespace aly{

class TextLabel: public Region {
protected:
	std::string label;
	bool truncate;
public:
	virtual pixel2 getTextDimensions(AlloyContext* context);
	HorizontalAlignment horizontalAlignment = HorizontalAlignment::Left;
	VerticalAlignment verticalAlignment = VerticalAlignment::Top;
	FontStyle fontStyle = FontStyle::Normal;
	FontType fontType = FontType::Normal;
	AUnit1D fontSize = UnitPX(24);
	AColor textColor = MakeColor(Theme::Default.LIGHTER);
	AColor textAltColor = MakeColor(Theme::Default.DARKER);
	void setAlignment(const HorizontalAlignment& horizontalAlignment,
			const VerticalAlignment& verticalAlignment) {
		this->horizontalAlignment = horizontalAlignment;
		this->verticalAlignment = verticalAlignment;
	}
	void setTruncate(bool t) {
		truncate = t;
	}
	void setLabel(const std::string& label) {
		this->label = label;
	}
	void setText(const std::string& label) {
		this->label = label;
	}
	std::string getText() const {
		return label;
	}
	std::string getLabel() const {
		return label;
	}
	TextLabel(
			const std::string& name = MakeString() << "t" << std::setw(8)
					<< std::setfill('0') << (REGION_COUNTER++)) :
			Region(name), label(name),truncate(true) {
	}
	TextLabel(const std::string& name, const AUnit2D& pos, const AUnit2D& dims) :
			Region(name, pos, dims), label(name) {
	}

	virtual void draw(AlloyContext* context) override;
};
class TextLink: public TextLabel {
protected:
	bool enabled;
public:
	std::function<bool()> onClick;
	void setEnabled(bool m){
		enabled=m;
	}
	TextLink(
			const std::string& name = MakeString() << "t" << std::setw(8)
					<< std::setfill('0') << (REGION_COUNTER++));
	TextLink(const std::string& name, const AUnit2D& pos, const AUnit2D& dims);
	virtual void draw(AlloyContext* context) override;
};
class TextRegion: public TextLabel {

public:
	virtual pixel2 getTextDimensions(AlloyContext* context) override;

	TextRegion(
			const std::string& name = MakeString() << "t" << std::setw(8)
					<< std::setfill('0') << (REGION_COUNTER++)) :
			TextLabel(name) {
	}
	TextRegion(const std::string& name, const AUnit2D& pos, const AUnit2D& dims) :
			TextLabel(name, pos, dims) {
	}

	virtual void draw(AlloyContext* context) override;
};

class TextField: public Composite {
protected:
	bool showDefaultLabel = false;
	bool focused = false;
	std::string label;
	std::string value;
	float th=0;
	float textOffsetX = 0;
	bool showCursor = false;
	std::chrono::high_resolution_clock::time_point lastTime;
	void clear();
	void erase();
	bool handleCursorInput(AlloyContext* context, const InputEvent& e);
	bool handleMouseInput(AlloyContext* context, const InputEvent& e);
	void handleKeyInput(AlloyContext* context, const InputEvent& e);
	void handleCharacterInput(AlloyContext* context, const InputEvent& e);
	void moveCursorTo(int index, bool isShiftHeld = false);
	void dragCursorTo(int index);
	int cursorStart = 0, cursorEnd = 0, textStart = 0;
	bool dragging = false;
	std::string lastValue;
	bool modifiable;
public:
	static const float PADDING;
	AUnit1D fontSize;
	AColor textColor = MakeColor(Theme::Default.LIGHTER);
	void setModifiable(bool m) {
		modifiable = m;
	}
	bool isFocused() const {
		return focused;
	}
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)
			override;
	virtual inline ~TextField() {
	}
	void setShowDefaultLabel(bool show) {
		showDefaultLabel = show;
	}
	void setLabel(const std::string& l) {
		label = l;
	}
	TextField(
			const std::string& name = MakeString() << "t" << std::setw(8)
					<< std::setfill('0') << (REGION_COUNTER++));
	TextField(const std::string& name, const AUnit2D& position,
			const AUnit2D& dimensions);
	virtual void draw(AlloyContext* context) override;
	virtual void setValue(const std::string& value);
	std::string getValue() const {
		return value;
	}
	std::function<void(TextField*)> onTextEntered;
	std::function<void(TextField*)> onKeyInput;
};
class ModifiableLabel : public TextField {
protected:
	bool truncate;
public:
	FontType fontType;
	FontStyle fontStyle;
	AColor textAltColor;
	HorizontalAlignment horizontalAlignment = HorizontalAlignment::Left;
	VerticalAlignment verticalAlignment = VerticalAlignment::Top;
	void setAlignment(const HorizontalAlignment& horizontalAlignment,
		const VerticalAlignment& verticalAlignment) {
		this->horizontalAlignment = horizontalAlignment;
		this->verticalAlignment = verticalAlignment;
	}
	void setTruncate(bool b){
		truncate=b;
	}
	virtual pixel2 getTextDimensions(AlloyContext* context);
	ModifiableLabel(const std::string& name, const AUnit2D& position,const AUnit2D& dimensions,bool modifiable=true);
	virtual void draw(AlloyContext* context) override;
};
template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const TextLabel & region) {
	ss << "Text Label: " << region.name << std::endl;
	ss << "\tOrigin: " << region.getOrigin() << std::endl;
	ss << "\tHorizontal Alignment: " << region.horizontalAlignment << std::endl;
	ss << "\tVertical Alignment: " << region.verticalAlignment << std::endl;
	ss << "\tRelative Position: " << region.position << std::endl;
	ss << "\tRelative Dimensions: " << region.dimensions << std::endl;
	ss << "\tBounds: " << region.getBounds() << std::endl;
	ss << "\tFont Type: " << region.fontType << std::endl;
	ss << "\tFont Size: " << region.fontSize << std::endl;
	ss << "\tFont Color: " << region.textColor << std::endl;
	if (region.parent != nullptr)
		ss << "\tParent: " << region.parent->name << std::endl;
	return ss;
}


std::shared_ptr<TextLabel> MakeTextLabel(const std::string& name,
		const AUnit2D& position, const AUnit2D& dimensions,
		const FontType& fontType, const AUnit1D& fontSize = UnitPT(14.0f),
		const Color& fontColor = COLOR_WHITE,
		const HorizontalAlignment& halign = HorizontalAlignment::Left,
		const VerticalAlignment& valign = VerticalAlignment::Top);
std::shared_ptr<TextField> MakeTextField(const std::string& name,
		const AUnit2D& position, const AUnit2D& dimensions,
		const Color& bgColor = Theme::Default.DARK, const Color& textColor =
				Theme::Default.LIGHTER, const std::string& value = "");
typedef std::shared_ptr<TextLabel> TextLabelPtr;
typedef std::shared_ptr<TextLink> TextLinkPtr;
typedef std::shared_ptr<TextRegion> TextRegionPtr;
typedef std::shared_ptr<TextField> TextFieldPtr;
typedef std::shared_ptr<ModifiableLabel> ModifiableLabelPtr;

}
#endif /* SRC_UI_ALLOYTEXTWIDGET_H_ */
