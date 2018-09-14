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

#include "ui/AlloyWindowPane.h"
#include "ui/AlloyApplication.h"
namespace aly {
void WindowPane::setMaximize(bool max) {
	maximized = max;
	if (this->maximized) {
		maximizeIcon->setIcon(0xf066);
	} else {
		maximizeIcon->setIcon(0xf065);
	}
}
void WindowPane::draw(AlloyContext* context) {
	AdjustableComposite::draw(context);
	if (context->getCursor() == nullptr) {
		if (context->isMouseOver(label.get())) {
			context->setCursor(&Cursor::Position);
		}
	}
}
bool WindowPane::onEventHandler(AlloyContext* context, const InputEvent& e) {
	if (dragging && e.type == InputType::Cursor && !isResizing()) {
		box2px pbounds = parent->getBounds();
		this->setDragOffset(pbounds.clamp(e.cursor), cursorDownPosition);
		setMaximize(false);
		context->requestPack();
	} else if (e.type == InputType::MouseButton && e.isUp()) {
		context->requestPack();
		dragging = false;
	}
	return AdjustableComposite::onEventHandler(context, e);
}

WindowPane::WindowPane(const RegionPtr& content) :
		AdjustableComposite(content->name, content->position,
				content->dimensions, true), maximized(false), dragging(false) {
	cellSpacing = pixel2(2, 2);
	cellPadding = pixel2(8, 8);
	titleRegion = CompositePtr(
			new Composite("Title", CoordPX(cellPadding.x, cellPadding.y),
					CoordPerPX(1.0f, 0.0f, -2.0f * cellPadding.x, 30.0f)));
	//titleRegion->backgroundColor = MakeColor(AlloyApplicationContext()->theme.LIGHT);
	label = TextLabelPtr(
			new TextLabel(content->name, CoordPX(0.0f, 0.0f),
					CoordPerPX(1.0f, 1.0f, 0.0f, 0.0f)));
	label->textColor = MakeColor(AlloyApplicationContext()->theme.LIGHTER);
	titleRegion->add(label);
	contentRegion = CompositePtr(
			new Composite("Content",
					CoordPX(cellPadding.x,
							30.0f + cellSpacing.y + cellPadding.y),
					CoordPerPX(1.0f, 1.0f, -2.0f * cellPadding.x,
							-30.0f - 2 * cellPadding.y - cellSpacing.y)));
	contentRegion->setScrollEnabled(true);
	contentRegion->add(content);
	content->position = CoordPX(0.0f, 0.0f);
	content->dimensions = CoordPercent(1.0f, 1.0f);
	backgroundColor = MakeColor(AlloyApplicationContext()->theme.DARK);
	setRoundCorners(true);
	Composite::add(titleRegion);
	Composite::add(contentRegion);
	this->setClampDragToParentBounds(false);
	label->onMouseDown = [this](AlloyContext* context, const InputEvent& e) {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT&&!isResizing()) {
			cursorDownPosition = e.cursor - this->getBoundsPosition();
			dragging = true;
		}
		return false;
	};
	onResize = [this](AdjustableComposite* composite,const box2px& bounds) {
		setMaximize(false);
	};
	maximizeIcon = IconButtonPtr(
			new IconButton(0xf0fe, CoordPerPX(1.0f, 0.0f, -24.0f, 0.0f),
					CoordPX(24.0f, 24.0f)));
	maximizeIcon->borderWidth = UnitPX(0.0f);
	maximizeIcon->borderColor = MakeColor(COLOR_NONE);
	maximizeIcon->foregroundColor = MakeColor(COLOR_NONE);
	maximizeIcon->iconColor = MakeColor(
			AlloyApplicationContext()->theme.LIGHTER);
	titleRegion->add(maximizeIcon);
	maximizeIcon->onMouseDown =
			[this](AlloyContext* context, const InputEvent& event) {
				if (event.button == GLFW_MOUSE_BUTTON_LEFT) {
					setMaximize(!this->maximized);
					if (maximized) {
						windowInitialBounds = getBounds(false);
						this->setDragOffset(pixel2(0.0f, 0.0f));
						this->position = CoordPX(0.0f,0.0f);
						this->dimensions = CoordPercent(1.0f,1.0f);
						dynamic_cast<Composite*>(this->parent)->resetScrollPosition();
						dynamic_cast<Composite*>(this->parent)->putLast(this);
						context->requestPack();
					}
					else {
						this->position = CoordPX(windowInitialBounds.position);
						this->dimensions = CoordPX(windowInitialBounds.dimensions);
						context->requestPack();
					}
					return true;
				}
				return false;
			};
	setMaximize(false);
	Application::addListener(this);
}
} /* namespace aly */
