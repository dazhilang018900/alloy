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

#ifndef ALLOYTOGGLEWIDGET_H_
#define ALLOYTOGGLEWIDGET_H_

#include "ui/AlloyComposite.h"
#include "ui/AlloyTextWidget.h"
namespace aly {

class CheckBox: public Composite {
private:
	TextLabelPtr checkLabel;
	TextLabelPtr valueLabel;
	bool checked;
	bool handleMouseDown(AlloyContext* context, const InputEvent& event);
public:
	std::function<void(bool)> onChange;
	inline bool getValue() const {
		return checked;
	}
	void setValue(bool value);
	CheckBox(const std::string& label, const AUnit2D& position,
			const AUnit2D& dimensions, bool checked=true ,bool showText=true);
	virtual void draw(AlloyContext* context) override;
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)
			override;
};
class ToggleBox: public Composite {
private:
	TextLabelPtr toggleLabel;
	TextLabelPtr onLabel;
	TextLabelPtr offLabel;
	CompositePtr clickRegion;
	bool toggledOn;
	bool handleMouseDown(AlloyContext* context, const InputEvent& e);
public:
	inline bool getValue() const {
		return toggledOn;
	}
	std::function<void(bool)> onChange;

	void setValue(bool value);
	ToggleBox(const std::string& label, const AUnit2D& position,
			const AUnit2D& dimensions, bool toggledOn ,bool showText=true);
	virtual void draw(AlloyContext* context) override;
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)
			override;
};



typedef std::shared_ptr<CheckBox> CheckBoxPtr;
typedef std::shared_ptr<ToggleBox> ToggleBoxPtr;



}

#endif /* ALLOYWIDGET_H_ */
