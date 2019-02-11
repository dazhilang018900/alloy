/*
 * AlloyScrollPanel.cpp
 *
 *  Created on: Feb 9, 2019
 *      Author: blake
 */

#include "AlloyScrollPane.h"

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
	if (event.type == InputType::MouseButton && context->isMouseOver(this, true)
			&& event.isDown()) {
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
	float xoff = 0;
	float yoff = 0;
	if (hover) {
		if (down) {
			if (onMousePressed)
				onMousePressed();
			tcolor = context->theme.LIGHT;
		} else {
			tcolor = context->theme.LIGHTEST;
		}
		bgcolor = backgroundColor->toSemiTransparent(0.7f);
		shcolor = backgroundColor->toSemiTransparent(0.0f);
	} else {
		if (down) {
			tcolor = context->theme.LIGHT;
		} else {
			tcolor = context->theme.LIGHTER;
		}
		bgcolor = backgroundColor->toSemiTransparent(0.5f);
		shcolor = backgroundColor->toSemiTransparent(0.0f);
	}
	switch (dir) {
	case Direction::Left:
		code = 0xF053;
		if (down)
			xoff = -2;
		bg = nvgLinearGradient(nvg, x, y, x + w, y, bgcolor, shcolor);
		break;
	case Direction::Right:
		code = 0xF054;
		if (down)
			xoff = 2;
		bg = nvgLinearGradient(nvg, x + w, y, x, y, bgcolor, shcolor);
		break;
	case Direction::Up:
		code = 0xF077;
		if (down)
			yoff = -2;
		bg = nvgLinearGradient(nvg, x, y, x, y + h, bgcolor, shcolor);
		break;
	case Direction::Down:
		code = 0xF078;
		if (down)
			yoff = 2;
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
	drawText(nvg,
			bounds.position + float2(xoff, yoff) + HALF_PIX(bounds.dimensions),
			CodePointToUTF8(code), FontStyle::Normal, tcolor, *backgroundColor,
			nullptr);

}
void ScrollPane::updateCursor(CursorLocator* cursorLocator) {
	if (!ignoreCursorEvents)
		cursorLocator->add(this);
	for (std::shared_ptr<Region>& region : children) {
		region->updateCursor(cursorLocator);
	}

	if (leftButton.get() != nullptr) {
		leftButton->updateCursor(cursorLocator);
	}
	if (rightButton.get() != nullptr) {
		rightButton->updateCursor(cursorLocator);
	}
	if (upButton.get() != nullptr) {
		upButton->updateCursor(cursorLocator);
	}
	if (downButton.get() != nullptr) {
		downButton->updateCursor(cursorLocator);
	}

	if (verticalScrollTrack.get() != nullptr) {
		verticalScrollTrack->updateCursor(cursorLocator);
	}
	if (verticalScrollHandle.get() != nullptr) {
		verticalScrollHandle->updateCursor(cursorLocator);
	}
	if (horizontalScrollTrack.get() != nullptr) {
		horizontalScrollTrack->updateCursor(cursorLocator);
	}
	if (horizontalScrollHandle.get() != nullptr) {
		horizontalScrollHandle->updateCursor(cursorLocator);
	}

}
void ScrollPane::draw(AlloyContext* context) {
	NVGcontext* nvg = context->nvgContext;
	box2px bounds = getBounds();
	float w = bounds.dimensions.x;
	float h = bounds.dimensions.y;
	pixel lineWidth = borderWidth.toPixels(bounds.dimensions.y, context->dpmm.y,
			context->pixelRatio);
	if(leftButton.get()!=nullptr)leftButton->setVisible(scrollPosition.x>0.00001f);
	if(rightButton.get()!=nullptr)rightButton->setVisible(scrollPosition.x<0.9999f);
	if(upButton.get()!=nullptr)upButton->setVisible(scrollPosition.y>0.00001f);
	if(downButton.get()!=nullptr)downButton->setVisible(scrollPosition.y<0.9999f);
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
			if (leftButton.get() != nullptr&&leftButton->isVisible())
				leftButton->draw(context);
			if (rightButton.get() != nullptr&&rightButton->isVisible())
				rightButton->draw(context);
			if (upButton.get() != nullptr&&upButton->isVisible())
				upButton->draw(context);
			if (downButton.get() != nullptr&&downButton->isVisible())
				downButton->draw(context);
		}
	}
	if (isScrollEnabled()) {
		popScissor(nvg);
	}
	if (borderColor->a > 0) {

		nvgLineJoin(nvg, NVG_ROUND);
		nvgBeginPath(nvg);
		if (roundCorners) {
			nvgRoundedRect(nvg, bounds.position.x + lineWidth * 0.5f,
					bounds.position.y + lineWidth * 0.5f,
					bounds.dimensions.x - lineWidth,
					bounds.dimensions.y - lineWidth,
					context->theme.CORNER_RADIUS);
		} else {
			nvgRect(nvg, bounds.position.x + lineWidth * 0.5f,
					bounds.position.y + lineWidth * 0.5f,
					bounds.dimensions.x - lineWidth,
					bounds.dimensions.y - lineWidth);
		}
		nvgStrokeColor(nvg, *borderColor);
		nvgStrokeWidth(nvg, lineWidth);
		nvgStroke(nvg);
		nvgLineJoin(nvg, NVG_MITER);
	}

}
void ScrollPane::pack(const pixel2& pos, const pixel2& dims,
		const double2& dpmm, double pixelRatio, bool clamp) {
	Composite::pack(pos, dims, dpmm, pixelRatio, clamp);
	box2px bounds = getBounds();
	if (leftButton.get() != nullptr) {
		leftButton->pack(bounds.position, bounds.dimensions, dpmm, pixelRatio);
	}
	if (rightButton.get() != nullptr) {
		rightButton->pack(bounds.position, bounds.dimensions, dpmm, pixelRatio);
	}
	if (upButton.get() != nullptr) {
		upButton->pack(bounds.position, bounds.dimensions, dpmm, pixelRatio);
	}
	if (downButton.get() != nullptr) {
		downButton->pack(bounds.position, bounds.dimensions, dpmm, pixelRatio);
	}
}
void ScrollPane::drawDebug(AlloyContext* context) {
	drawBoundsLabel(context, name, context->getFontHandle(FontType::Bold));
	for (std::shared_ptr<Region>& region : children) {
		region->drawDebug(context);
	}
	if (isVisible()) {
		if (verticalScrollTrack.get()) {
			verticalScrollTrack->drawDebug(context);
		}
		if (verticalScrollHandle.get()) {
			verticalScrollHandle->drawDebug(context);
		}
		if (horizontalScrollTrack.get()) {
			horizontalScrollTrack->drawDebug(context);
		}
		if (horizontalScrollHandle.get()) {
			horizontalScrollHandle->drawDebug(context);
		}
		if (leftButton.get() != nullptr) {
			leftButton->drawDebug(context);
		}
		if (rightButton.get() != nullptr) {
			rightButton->drawDebug(context);
		}
		if (upButton.get() != nullptr) {
			upButton->drawDebug(context);
		}
		if (downButton.get() != nullptr) {
			downButton->drawDebug(context);
		}
	}
}
ScrollPane::ScrollPane(const std::string& name, const AUnit2D& pos,
		const AUnit2D& dims, const Orientation& orient,float scrollStep, float buttonWidth) :
		Composite(name, pos, dims) {
	setScrollEnabled(true);
	if (orient == Orientation::Horizontal) {
		setAlwaysShowHorizontalScrollBar(true);
		leftButton = ArrowButtonPtr(
				new ArrowButton("Left Button", CoordPX(0.0f, 0.0f),
						CoordPerPX(0.0f, 1.0f, buttonWidth, 0.0f),
						Direction::Left));
		rightButton = ArrowButtonPtr(
				new ArrowButton("Right Button",
						CoordPerPX(1.0f, 0.0f, -buttonWidth, 0.0f),
						CoordPerPX(0.0f, 1.0f, buttonWidth, 0.0f),
						Direction::Right));
		leftButton->onMousePressed = [this,scrollStep]() {
			if(timer.resetAfterElapsed(0.05f)) {
				this->addHorizontalScrollPosition(-scrollStep);
			}
		};
		rightButton->onMousePressed = [this,scrollStep]() {
			if(timer.resetAfterElapsed(0.05f)) {
				this->addHorizontalScrollPosition(scrollStep);
			}
		};
	} else if (orient == Orientation::Vertical) {
		setAlwaysShowVerticalScrollBar(true);
		upButton = ArrowButtonPtr(
				new ArrowButton("Up Button", CoordPX(0.0f, 0.0f),
						CoordPerPX(1.0f, 0.0f, 0.0f, buttonWidth),
						Direction::Up));
		downButton = ArrowButtonPtr(
				new ArrowButton("Down Button",
						CoordPerPX(0.0f, 1.0f, 0.0f, -buttonWidth),
						CoordPerPX(1.0f, 0.0f, 0.0f, buttonWidth),
						Direction::Down));
		upButton->onMousePressed = [this,scrollStep]() {
			if(timer.resetAfterElapsed(0.05f)) {
				this->addVerticalScrollPosition(-scrollStep);
			}
		};
		downButton->onMousePressed = [this,scrollStep]() {
			if(timer.resetAfterElapsed(0.05f)) {
				this->addVerticalScrollPosition(scrollStep);
			}
		};
	}
}

} /* namespace aly */
