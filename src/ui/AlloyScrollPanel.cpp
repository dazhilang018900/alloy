/*
 * AlloyScrollPanel.cpp
 *
 *  Created on: Feb 9, 2019
 *      Author: blake
 */

#include "AlloyScrollPanel.h"
#include "AlloyDrawUtil.h"
#include "AlloyApplication.h"
namespace aly {
ArrowButton::ArrowButton(const std::string& label, const AUnit2D& position,
		const AUnit2D& dimensions, const Direction& dir) :
		Region(label, position, dimensions), dir(dir) {
	backgroundColor = MakeColor(AlloyApplicationContext()->theme.DARKER);
	borderColor = MakeColor(AlloyApplicationContext()->theme.LIGHT);
	borderWidth = UnitPX(0.0f);
	setRoundCorners(false);
	Application::addListener(this);
}
bool ArrowButton::onEventHandler(AlloyContext* context,
		const InputEvent& event) {
	if (event.type == InputType::MouseButton && event.isDown()
			&& context->isMouseOver(this, true)) {
		if (event.button == GLFW_MOUSE_BUTTON_LEFT) {
			setFocus(true);
		} else if (event.button == GLFW_MOUSE_BUTTON_RIGHT) {
			setFocus(false);
		}
	} else if (event.type == InputType::Key && event.key == GLFW_KEY_ENTER) {
		if (event.isDown() && isObjectFocused()) {
			context->setMouseDownObject(this);
			context->setLeftMouseButtonDown(true);
			if (onMouseDown) {
				InputEvent e = event;
				e.type = InputType::MouseButton;
				e.clicks = 1;
				e.action = GLFW_PRESS;
				e.button = GLFW_MOUSE_BUTTON_LEFT;
				return onMouseDown(context, e);
			}
		} else if (!event.isDown()) {
			if (context->getMouseDownObject() == this) {
				context->setMouseDownObject(nullptr);
				context->setLeftMouseButtonDown(false);
				if (onMouseUp) {
					InputEvent e = event;
					e.type = InputType::MouseButton;
					e.clicks = 1;
					e.action = GLFW_RELEASE;
					e.button = GLFW_MOUSE_BUTTON_LEFT;
					return onMouseUp(context, e);
				}
			}
		}
	}
	return Region::onEventHandler(context, event);
}
void ArrowButton::draw(AlloyContext* context) {
	bool hover = context->isMouseOver(this);
	bool down = context->isMouseDown(this) && context->isLeftMouseButtonDown();
	NVGcontext* nvg = context->nvgContext;
	box2px bounds = getBounds();
	const float x = bounds.position.x;
	const float y = bounds.position.y;
	const float w = bounds.dimensions.x;
	const float h = bounds.dimensions.y;
	float th = min(w, h) / 2;
	NVGpaint bg;
	uint32_t code = 0;
	Color bgcolor, shcolor, tcolor;
	if (hover) {
		tcolor =  context->theme.LIGHTEST;
		bgcolor = backgroundColor->toSemiTransparent(0.7f);
		shcolor = backgroundColor->toSemiTransparent(0.0f);
	} else {
		tcolor = context->theme.LIGHTER;
		bgcolor = backgroundColor->toSemiTransparent(0.5f);
		shcolor = backgroundColor->toSemiTransparent(0.0f);
	}
	switch (dir) {
	case Direction::Left:
		code = 0xF053;
		bg = nvgLinearGradient(nvg, x, y, x + w, y, bgcolor, shcolor);
		break;
	case Direction::Right:
		code = 0xF054;
		bg = nvgLinearGradient(nvg, x + w, y, x, y, bgcolor, shcolor);
		break;
	case Direction::Up:
		code = 0xF077;
		bg = nvgLinearGradient(nvg, x, y, x, y + h, bgcolor, shcolor);
		break;
	case Direction::Down:
		code = 0xF078;
		bg = nvgLinearGradient(nvg, x, y + h, x, y, bgcolor, shcolor);
		break;
	}

	nvgBeginPath(nvg);
	nvgRect(nvg, bounds.position.x, bounds.position.y, bounds.dimensions.x,
			bounds.dimensions.y);

	nvgFillPaint(nvg, bg);
	nvgFill(nvg);

	nvgFontSize(nvg, th);
	nvgFontFaceId(nvg, context->getFontHandle(FontType::Icon));
	nvgTextAlign(nvg, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
	drawText(nvg, bounds.position + HALF_PIX(bounds.dimensions),
			CodePointToUTF8(code), FontStyle::Normal, tcolor, *backgroundColor,
			nullptr);

}
void ScrollPanel::draw(AlloyContext* context) {
	NVGcontext* nvg = context->nvgContext;
	box2px bounds = getBounds();
	float w = bounds.dimensions.x;
	float h = bounds.dimensions.y;
	if (isScrollEnabled()) {
		pushScissor(nvg, getCursorBounds());
	}
	if (backgroundColor->a > 0) {
		nvgBeginPath(nvg);
		if (roundCorners) {
			nvgRoundedRect(nvg, bounds.position.x, bounds.position.y,
					bounds.dimensions.x, bounds.dimensions.y,
					context->theme.CORNER_RADIUS);
		} else {
			nvgRect(nvg, bounds.position.x, bounds.position.y,
					bounds.dimensions.x, bounds.dimensions.y);
		}
		nvgFillColor(nvg, *backgroundColor);
		nvgFill(nvg);
	}

	for (std::shared_ptr<Region>& region : children) {
		if (region->isVisible()) {
			region->draw(context);
		}
	}
	if (verticalScrollTrack.get() != nullptr) {
		if (isScrollEnabled()) {
			if (extents.dimensions.y > h) {
				verticalScrollTrack->draw(context);
				verticalScrollHandle->draw(context);
			} else {
				verticalScrollTrack->draw(context);
			}
			if (extents.dimensions.x > w) {
				horizontalScrollTrack->draw(context);
				horizontalScrollHandle->draw(context);
			} else {
				horizontalScrollTrack->draw(context);
			}
		}
	}
	if (isScrollEnabled()) {
		popScissor(nvg);
	}
}
void ScrollPanel::addPanel(const RegionPtr& ptr) {
	content->add(ptr);
}
ScrollPanel::ScrollPanel(const std::string& name, const AUnit2D& pos,
		const AUnit2D& dims, const Orientation& orient) :
		Composite(name, pos, dims) {
	content = CompositePtr(
			new Composite("Scroller Content", CoordPX(0.0f, 0.0f),
					CoordPercent(1.0f, 1.0f)));
	content->setOrientation(orient);
	content->setScrollEnabled(true);
	content->setAlwaysShowHorizontalScrollBar(true);
	add(content);
	const float BUTTON_SZ = 40.0f;
	const float SZ = 0.0f; //Composite::scrollBarSize;
	if (orient == Orientation::Horizontal) {
		leftButton = ArrowButtonPtr(
				new ArrowButton("Left Button", CoordPX(0.0f, 0.0f),
						CoordPerPX(0.0f, 1.0f, BUTTON_SZ, -SZ),
						Direction::Left));
		rightButton = ArrowButtonPtr(
				new ArrowButton("Right Button",
						CoordPerPX(1.0f, 0.0f, -BUTTON_SZ, 0.0f),
						CoordPerPX(0.0f, 1.0f, BUTTON_SZ, -SZ),
						Direction::Right));
		add(leftButton);
		add(rightButton);
	} else if (orient == Orientation::Vertical) {
		upButton = ArrowButtonPtr(
				new ArrowButton("Up Button", CoordPX(0.0f, 0.0f),
						CoordPerPX(1.0f, 0.0f, -SZ, BUTTON_SZ), Direction::Up));
		downButton = ArrowButtonPtr(
				new ArrowButton("Down Button",
						CoordPerPX(0.0f, 1.0f, 0.0f, -BUTTON_SZ),
						CoordPerPX(1.0f, 0.0f, -SZ, BUTTON_SZ),
						Direction::Down));
		add(upButton);
		add(downButton);
	}
}

} /* namespace aly */
