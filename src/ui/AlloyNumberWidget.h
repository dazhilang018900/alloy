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

#ifndef SRC_UI_ALLOYNUMBERWIDGET_H_
#define SRC_UI_ALLOYNUMBERWIDGET_H_
#include "ui/AlloyComposite.h"
namespace aly{

class NumberField: public Composite {
protected:
	bool showDefaultLabel = false;
	std::string label;
	std::string value;
	float th;
	float textOffsetX;
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
	bool valid = true;
	//bool focused=false;
	std::string lastValue;
	Number numberValue;
	NumberType numberType;
	bool modifiable;
public:
	AUnit1D fontSize;
	AColor invalidNumberColor;
	bool isValid() const {
		return valid;
	}
	/*
	bool isFocused() const {
		return focused;
	}
	*/
	void setModifiable(bool m) {
		modifiable = m;
	}
	static const float PADDING;
	AColor textColor = MakeColor(Theme::Default.LIGHTER);
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)
			override;
	virtual inline ~NumberField() {
	}
	void setShowDefaultLabel(bool show) {
		showDefaultLabel = show;
	}
	void setLabel(const std::string& l) {
		label = l;
	}
	NumberField(const std::string& name, const NumberType& numberType);
	NumberField(const std::string& name, const AUnit2D& position,
			const AUnit2D& dimensions, const NumberType& numberType);
	virtual void draw(AlloyContext* context) override;
	virtual bool setValue(const std::string& value);
	bool setNumberValue(const Number& val);
	bool validate();
	Number getValue() const {
		return numberValue;
	}
	Number& getValue() {
		return numberValue;
	}
	std::function<void(NumberField*)> onTextEntered;
	std::function<void(NumberField*)> onKeyInput;
};
class ModifiableNumber : public NumberField {
protected:
	bool truncate;
public:
	FontType fontType;
	FontStyle fontStyle;
	AColor textAltColor;
	std::function<std::string(const Number& value)> labelFormatter;
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
	ModifiableNumber(const std::string& name, const AUnit2D& position, const AUnit2D& dimensions, const NumberType& type,bool modifiable=true);
	virtual void draw(AlloyContext* context) override;
};
std::shared_ptr<NumberField> MakeNumberField(const std::string& name,
		const AUnit2D& position, const AUnit2D& dimensions,
		const NumberType& type, const Color& bgColor = Theme::Default.DARK,
		const Color& textColor = Theme::Default.LIGHTER,
		const Color& invalidColor = Color(220, 64, 64));

typedef std::shared_ptr<ModifiableNumber> ModifiableNumberPtr;
typedef std::shared_ptr<NumberField> NumberFieldPtr;
}
#endif /* SRC_UI_ALLOYNUMBERWIDGET_H_ */
