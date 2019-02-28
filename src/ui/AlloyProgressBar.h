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

#ifndef SRC_UI_ALLOYPROGRESSBAR_H_
#define SRC_UI_ALLOYPROGRESSBAR_H_
#include "ui/AlloyComposite.h"
#include "ui/AlloyTextWidget.h"
namespace aly {


class ProgressBar: public Region {
private:
	float value;
	std::string label;
public:
	virtual void draw(AlloyContext* context) override;
	inline void setValue(float p) {
		value = clamp(p, 0.0f, 1.0f);
	}
	inline float getValue() const {
		return value;
	}
	inline std::string getLabel() const {
		return label;
	}
	inline void setValue(const std::string& l) {
		label = l;
	}
	inline void setValue(const std::string& l, float p) {
		label = l;
		value = clamp(p, 0.0f, 1.0f);
	}
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)
			override;
	ProgressBar(const std::string& name, const AUnit2D& pt,
			const AUnit2D& dims);
};


typedef std::shared_ptr<ProgressBar> ProgressBarPtr;

class ProgressCircle: public Region{
private:
	float value;
	std::string label;
	float thickness;
public:
	AColor foregroundColor;
	AColor textColor;
	void setThickness(float p);
	virtual void draw(AlloyContext* context) override;
	inline void setValue(float p) {
		value = clamp(p, 0.0f, 1.0f);
	}
	inline float getValue() const {
		return value;
	}
	inline std::string getLabel() const {
		return label;
	}
	inline void setValue(const std::string& l) {
		label = l;
	}
	inline void setValue(const std::string& l, float p) {
		label = l;
		value = clamp(p, 0.0f, 1.0f);
	}
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)
			override;
	ProgressCircle(const std::string& name, const AUnit2D& pt,
			const AUnit2D& dims);
};


typedef std::shared_ptr<ProgressCircle> ProgressCirclePtr;

} /* namespace aly */

#endif /* SRC_UI_ALLOYPROGRESSBAR_H_ */
