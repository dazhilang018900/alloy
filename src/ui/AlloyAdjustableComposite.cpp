/*
 * AlloyAdjustableComposite.cpp
 *
 *  Created on: Sep 13, 2018
 *      Author: blake
 */

#include "ui/AlloyAdjustableComposite.h"
#include "ui/AlloyDrawUtil.h"
#include "ui/AlloyApplication.h"
namespace aly{
AdjustableComposite::AdjustableComposite(const std::string& name,
		const AUnit2D& pos, const AUnit2D& dims, bool resizeable) :
		Composite(name, pos, dims), resizing(false), winPos(
				WindowPosition::Outside), resizeable(resizeable) {

	windowInitialBounds.dimensions = float2(-1, -1);
	cellPadding = pixel2(10, 10);
	if (resizeable) {
		Application::addListener(this);
	}
}
void AdjustableComposite::draw(AlloyContext* context) {
	pushScissor(context->getNVG(), getCursorBounds());
	Composite::draw(context);
	popScissor(context->getNVG());
	if (windowInitialBounds.dimensions.x < 0
			|| windowInitialBounds.dimensions.y < 0) {
		windowInitialBounds = getBounds(false);
		windowInitialBounds.position -= this->getDragOffset();
	}
	if (context->getCursor() == nullptr) {
		switch (winPos) {
		case WindowPosition::Center:
			if ((dragButton == GLFW_MOUSE_BUTTON_LEFT
					&& context->isLeftMouseButtonDown())
					|| (dragButton == GLFW_MOUSE_BUTTON_RIGHT
							&& context->isRightMouseButtonDown())) {
				if (!context->isMouseDown()) {
					context->setCursor(&Cursor::Position);
				}
			}
			break;
		case WindowPosition::Top:
		case WindowPosition::Bottom:
			context->setCursor(&Cursor::Vertical);
			break;
		case WindowPosition::Left:
		case WindowPosition::Right:
			context->setCursor(&Cursor::Horizontal);
			break;
		case WindowPosition::TopLeft:
		case WindowPosition::BottomRight:
			context->setCursor(&Cursor::SlantDown);
			break;
		case WindowPosition::BottomLeft:
		case WindowPosition::TopRight:
			context->setCursor(&Cursor::SlantUp);
			break;
		default:
			break;
		}

	}
}
bool AdjustableComposite::onEventHandler(AlloyContext* context,
		const InputEvent& e) {
	if (Composite::onEventHandler(context, e))
		return true;
	if (resizeable) {
		bool over = context->isMouseOver(this, true);
		if (e.type == InputType::MouseButton
				&& e.button == GLFW_MOUSE_BUTTON_LEFT && e.isDown() && over) {
			dynamic_cast<Composite*>(this->parent)->putLast(this);
		} else if (!context->isLeftMouseButtonDown()) {
			resizing = false;
		}
		if (e.type == InputType::Cursor) {
			if (!resizing) {
				if (over) {
					box2px bounds = getBounds();
					winPos = WindowPosition::Center;
					if (e.cursor.x <= bounds.position.x + cellPadding.x) {
						if (e.cursor.y <= bounds.position.y + cellPadding.y) {
							winPos = WindowPosition::TopLeft;
						} else if (e.cursor.y
								>= bounds.position.y + bounds.dimensions.y
										- cellPadding.y) {
							winPos = WindowPosition::BottomLeft;
						} else {
							winPos = WindowPosition::Left;
						}
					} else if (e.cursor.x
							>= bounds.position.x + bounds.dimensions.x
									- cellPadding.x) {
						if (e.cursor.y <= bounds.position.y + cellPadding.y) {
							winPos = WindowPosition::TopRight;
						} else if (e.cursor.y
								>= bounds.position.y + bounds.dimensions.y
										- cellPadding.y) {
							winPos = WindowPosition::BottomRight;
						} else {
							winPos = WindowPosition::Right;
						}
					} else if (e.cursor.y
							<= bounds.position.y + cellPadding.y) {
						winPos = WindowPosition::Top;
					} else if (e.cursor.y
							>= bounds.position.y + bounds.dimensions.y
									- cellPadding.y) {
						winPos = WindowPosition::Bottom;
					}
				} else {
					winPos = WindowPosition::Outside;
				}
			}
		}
		if (over && e.type == InputType::MouseButton
				&& e.button == GLFW_MOUSE_BUTTON_LEFT && e.isDown()
				&& winPos != WindowPosition::Center
				&& winPos != WindowPosition::Outside) {
			if (!resizing) {
				cursorDownPosition = e.cursor;
				windowInitialBounds = getBounds(false);
				windowInitialBounds.position -= this->getDragOffset();
			}
			resizing = true;

		}
		if (resizing && e.type == InputType::Cursor) {
			float2 minPt = windowInitialBounds.min();
			float2 maxPt = windowInitialBounds.max();
			pixel2 cursor = box2px(pixel2(0.0f, 0.0f),
					pixel2(context->getScreenWidth() - 1.0f,
							context->getScreenHeight() - 1.0f)).clamp(e.cursor);
			switch (winPos) {
			case WindowPosition::Top:
				minPt.y += cursor.y - cursorDownPosition.y;
				break;
			case WindowPosition::Bottom:
				maxPt.y += cursor.y - cursorDownPosition.y;
				break;
			case WindowPosition::Left:
				minPt.x += cursor.x - cursorDownPosition.x;
				break;
			case WindowPosition::Right:
				maxPt.x += cursor.x - cursorDownPosition.x;
				break;
			case WindowPosition::TopLeft:
				minPt.x += cursor.x - cursorDownPosition.x;
				minPt.y += cursor.y - cursorDownPosition.y;
				break;
			case WindowPosition::BottomRight:
				maxPt.x += cursor.x - cursorDownPosition.x;
				maxPt.y += cursor.y - cursorDownPosition.y;
				break;
			case WindowPosition::BottomLeft:
				minPt.x += cursor.x - cursorDownPosition.x;
				maxPt.y += cursor.y - cursorDownPosition.y;
				break;
			case WindowPosition::TopRight:
				maxPt.x += cursor.x - cursorDownPosition.x;
				minPt.y += cursor.y - cursorDownPosition.y;

				break;
			default:
				break;
			}
			box2px newBounds(aly::min(minPt, maxPt),
					aly::max(maxPt - minPt, float2(50, 50)));
			pixel2 d = newBounds.dimensions;
			pixel2 d1, d2;
			if (aspectRule != AspectRule::Unspecified) {
				switch (winPos) {
				case WindowPosition::Left:
				case WindowPosition::Right:
					newBounds.dimensions = pixel2(d.x,
							d.x / (float) aspectRatio);
					break;
				case WindowPosition::Top:
				case WindowPosition::Bottom:
					newBounds.dimensions = pixel2(d.y * (float) aspectRatio,
							d.y);
					break;
				case WindowPosition::TopLeft:
				case WindowPosition::TopRight:
				case WindowPosition::BottomLeft:
				case WindowPosition::BottomRight:
					d1 = pixel2(d.x, d.x / (float) aspectRatio);
					d2 = pixel2(d.y * (float) aspectRatio, d.y);
					if (d1.x * d1.y > d2.x * d2.y) {
						newBounds.dimensions = d1;
					} else {
						newBounds.dimensions = d2;
					}
					break;
				default:
					break;
				}
			}

			if (clampToParentBounds) {
				pixel2 offset = getDragOffset();
				newBounds.position += offset;
				newBounds.clamp(parent->getBounds());
				newBounds.position -= offset;
			}
			this->position = CoordPX(
					newBounds.position - parent->getBoundsPosition());
			this->dimensions = CoordPX(newBounds.dimensions);
			if (onResize) {
				onResize(this, newBounds);
			}
			context->requestPack();
		}
		return false;
	} else {
		return false;
	}
}
}
