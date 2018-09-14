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

#ifndef SRC_UI_ALLOYWINDOWPANE_H_
#define SRC_UI_ALLOYWINDOWPANE_H_
#include "ui/AlloyAdjustableComposite.h"
#include "ui/AlloyButton.h"
#include "ui/AlloyTextWidget.h"
namespace aly {

class WindowPane: public AdjustableComposite {
protected:
	CompositePtr titleRegion;
	CompositePtr contentRegion;
	bool maximized;
	bool dragging;
	std::shared_ptr<IconButton> maximizeIcon;
	TextLabelPtr label;
public:
	void setMaximize(bool max);
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)
			override;
	WindowPane(const RegionPtr& content);
	virtual void draw(AlloyContext* context) override;
};

typedef std::shared_ptr<WindowPane> WindowPanePtr;

} /* namespace aly */

#endif /* SRC_UI_ALLOYWINDOWPANE_H_ */
