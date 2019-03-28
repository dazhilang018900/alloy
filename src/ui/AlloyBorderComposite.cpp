/*
 * AlloyBorderComposite.cpp
 *
 *  Created on: Sep 13, 2018
 *      Author: blake
 */

#include "ui/AlloyBorderComposite.h"
#include "ui/AlloyApplication.h"
#include "ui/AlloyDrawUtil.h"
namespace aly {

Region* BorderComposite::locate(const pixel2& cursor) {
	if (isVisible()) {
		for (auto iter = children.rbegin(); iter != children.rend(); iter++) {
			if (iter->get() == nullptr)
				continue;
			Region* r = (*iter)->locate(cursor);
			if (r != nullptr)
				return r;
		}
		if (getCursorBounds().contains(cursor)) {
			return this;
		}
	}
	return nullptr;
}

void BorderComposite::setNorth(const std::shared_ptr<Region>& region,
		const AUnit1D& fraction) {
	if (region->parent != nullptr)
		throw std::runtime_error(
				MakeString() << "Cannot add child node [" << region->name
						<< "] to [" << name
						<< "] because it already has a parent ["
						<< region->parent->name << "].");
	northRegion = region;
	northRegion->parent = this;
	northFraction = fraction;
}
void BorderComposite::setSouth(const std::shared_ptr<Region>& region,
		const AUnit1D& fraction) {
	if (region->parent != nullptr)
		throw std::runtime_error(
				MakeString() << "Cannot add child node [" << region->name
						<< "] to [" << name
						<< "] because it already has a parent ["
						<< region->parent->name << "].");
	southRegion = region;
	southRegion->parent = this;
	southFraction = fraction;
}
void BorderComposite::setEast(const std::shared_ptr<Region>& region,
		const AUnit1D& fraction) {
	if (region->parent != nullptr)
		throw std::runtime_error(
				MakeString() << "Cannot add child node [" << region->name
						<< "] to [" << name
						<< "] because it already has a parent ["
						<< region->parent->name << "].");
	eastRegion = region;
	eastRegion->parent = this;
	eastFraction = fraction;
}
void BorderComposite::setWest(const std::shared_ptr<Region>& region,
		const AUnit1D& fraction) {
	if (region->parent != nullptr)
		throw std::runtime_error(
				MakeString() << "Cannot add child node [" << region->name
						<< "] to [" << name
						<< "] because it already has a parent ["
						<< region->parent->name << "].");
	westRegion = region;
	westRegion->parent = this;
	westFraction = fraction;
}
void BorderComposite::setCenter(const std::shared_ptr<Region>& region) {
	if (region->parent != nullptr)
		throw std::runtime_error(
				MakeString() << "Cannot add child node [" << region->name
						<< "] to [" << name
						<< "] because it already has a parent ["
						<< region->parent->name << "].");
	centerRegion = region;
	centerRegion->parent = this;
}
void BorderComposite::draw(AlloyContext* context) {
	if (context->getCursor() == nullptr) {
		switch (winPos) {
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
	NVGcontext* nvg = context->getNVG();
	box2px bounds = getBounds();
	pixel lineWidth = borderWidth.toPixels(bounds.dimensions.y, context->dpmm.y,
			context->pixelRatio);
	if (isScrollEnabled()) {
		pushScissor(nvg, getCursorBounds());
	}
	if (backgroundColor->a > 0) {
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
		nvgFillColor(nvg, *backgroundColor);
		nvgFill(nvg);
	}

	for (std::shared_ptr<Region>& region : children) {
		if (region.get() == nullptr)
			continue;
		if (region->isVisible()) {
			region->draw(context);
		}
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

	if (isScrollEnabled()) {
		popScissor(nvg);
	}
}
bool BorderComposite::onEventHandler(AlloyContext* context,
		const InputEvent& e) {
	if (resizeable) {
		bool over = context->isMouseOver(this, true);
		if (!context->isLeftMouseButtonDown()) {
			resizing = false;
		}
		if (e.type == InputType::Cursor) {
			//Add center component to this!
			if (!resizing) {
				if (over) {
					winPos = WindowPosition::Center;

					if (westRegion.get() != nullptr) {
						box2px bounds = westRegion->getBounds();
						if (bounds.contains(e.cursor)) {
							if (e.cursor.x
									>= bounds.position.x + bounds.dimensions.x
											- cellPadding.x) {
								winPos = WindowPosition::Left;
							}
						}
					}
					if (eastRegion.get() != nullptr) {
						box2px bounds = eastRegion->getBounds();
						if (bounds.contains(e.cursor)) {
							if (e.cursor.x
									<= bounds.position.x + cellPadding.x) {
								winPos = WindowPosition::Right;
							}
						}
					}
					if (northRegion.get() != nullptr) {
						box2px bounds = northRegion->getBounds();
						if (bounds.contains(e.cursor)) {
							if (e.cursor.y
									>= bounds.position.y + bounds.dimensions.y
											- cellPadding.y) {
								winPos = WindowPosition::Top;
							}
						}
					}
					if (southRegion.get() != nullptr) {
						box2px bounds = southRegion->getBounds();
						if (bounds.contains(e.cursor)) {
							if (e.cursor.y
									<= bounds.position.y + cellPadding.y) {
								winPos = WindowPosition::Bottom;
							}
						}
					}
					if (centerRegion.get() != nullptr) {
						box2px bounds = centerRegion->getBounds();
						if (bounds.contains(e.cursor)) {
							if (e.cursor.x
									>= bounds.position.x + bounds.dimensions.x
											- cellPadding.x
									&& eastRegion.get() != nullptr) {
								winPos = WindowPosition::Right;
							} else if (e.cursor.x
									<= bounds.position.x + cellPadding.x
									&& westRegion.get() != nullptr) {
								winPos = WindowPosition::Left;
							} else if (e.cursor.y
									>= bounds.position.y + bounds.dimensions.y
											- cellPadding.y
									&& southRegion.get() != nullptr) {
								winPos = WindowPosition::Bottom;
							} else if (e.cursor.y
									<= bounds.position.y + cellPadding.y
									&& northRegion.get() != nullptr) {
								winPos = WindowPosition::Top;
							}
						}
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
				windowInitialBounds = currentBounds;
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
			default:
				break;
			}
			box2px bounds = getBounds(false);
			bounds.position -= this->getDragOffset();
			pixel2 upper = bounds.max() - float2(50.0f, 50.0f);
			pixel2 lower = bounds.min() + float2(50.0f, 50.0f);
			if (minPt.x > maxPt.x) {
				std::swap(minPt.x, maxPt.x);
			}
			if (minPt.y > maxPt.y) {
				std::swap(minPt.y, maxPt.y);
			}
			minPt = aly::max(lower, minPt);
			maxPt = aly::min(upper, maxPt);
			box2px nbounds(minPt, maxPt - minPt);
			switch (winPos) {
			case WindowPosition::Top:
				northFraction = UnitPX(nbounds.position.y - bounds.position.y);
				break;
			case WindowPosition::Bottom:
				southFraction = UnitPX(
						bounds.position.y + bounds.dimensions.y
								- nbounds.position.y - nbounds.dimensions.y);
				break;
			case WindowPosition::Left:
				westFraction = UnitPX(nbounds.position.x - bounds.position.x);
				break;
			case WindowPosition::Right:
				eastFraction = UnitPX(
						bounds.position.x + bounds.dimensions.x
								- nbounds.position.x - nbounds.dimensions.x);
				break;
			default:
				break;
			}
			//Create adjustable region component.
			context->requestPack();
		}
		return false;
	} else {
		return false;
	}
}
BorderComposite::BorderComposite(const std::string& name, const AUnit2D& pos,
		const AUnit2D& dims, bool resizeable) :
		Region(name, pos, dims), northRegion(children[0]), southRegion(
				children[1]), eastRegion(children[2]), westRegion(children[3]), centerRegion(
				children[4]), winPos(WindowPosition::Outside), resizeable(
				resizeable) {
	northFraction = UnitPX(0.0f);
	southFraction = UnitPX(0.0f);
	eastFraction = UnitPX(0.0f);
	westFraction = UnitPX(0.0f);
	cellPadding = pixel2(5.0, 5.0f);
	resizing = false;
	cursorDownPosition = pixel2(-1, -1);
	windowInitialBounds.dimensions = float2(-1, -1);
	if (resizeable) {
		Application::addListener(this);
	}
}
void BorderComposite::updateCursor(CursorLocator* cursorLocator) {
	if (!ignoreCursorEvents)
		cursorLocator->add(this);
	for (std::shared_ptr<Region>& region : children) {
		if (region.get() == nullptr)
			continue;
		region->updateCursor(cursorLocator);
	}
}

void BorderComposite::drawDebug(AlloyContext* context) {
	drawBoundsLabel(context, name, context->getFontHandle(FontType::Bold));
	for (std::shared_ptr<Region>& region : children) {
		if (region.get() == nullptr)
			continue;
		region->drawDebug(context);
	}
}

void BorderComposite::pack(const pixel2& pos, const pixel2& dims,
		const double2& dpmm, double pixelRatio, bool clamp) {
	Region::pack(pos, dims, dpmm, pixelRatio);
	box2px bounds = getBounds(false);
	bounds.position -= this->getDragOffset();
	pixel north = northFraction.toPixels(bounds.dimensions.y, dpmm.y,
			pixelRatio);
	pixel south = southFraction.toPixels(bounds.dimensions.y, dpmm.y,
			pixelRatio);
	pixel west = westFraction.toPixels(bounds.dimensions.x, dpmm.x, pixelRatio);
	pixel east = eastFraction.toPixels(bounds.dimensions.x, dpmm.x, pixelRatio);
	if (northRegion.get() != nullptr) {
		northRegion->pack(bounds.position, pixel2(bounds.dimensions.x, north),
				dpmm, pixelRatio);
	}
	if (southRegion.get() != nullptr) {
		southRegion->pack(
				bounds.position + pixel2(0, bounds.dimensions.y - south),
				pixel2(bounds.dimensions.x, south), dpmm, pixelRatio);
	}
	if (westRegion.get() != nullptr)
		westRegion->pack(bounds.position + pixel2(0.0f, north),
				pixel2(west, bounds.dimensions.y - north - south), dpmm,
				pixelRatio);
	if (eastRegion.get() != nullptr)
		eastRegion->pack(
				bounds.position + pixel2(bounds.dimensions.x - east, north),
				pixel2(east, bounds.dimensions.y - north - south), dpmm,
				pixelRatio);
	if (centerRegion.get() != nullptr)
		centerRegion->pack(bounds.position + pixel2(west, north),
				pixel2(bounds.dimensions.x - east - west,
						bounds.dimensions.y - north - south), dpmm, pixelRatio);
	currentBounds = box2px(bounds.position + pixel2(west, north),
			pixel2(bounds.dimensions.x - east - west,
					bounds.dimensions.y - north - south));
	for (std::shared_ptr<Region>& region : children) {
		if (region.get() == nullptr)
			continue;
		if (region->onPack)
			region->onPack();
	}
	if (onPack)
		onPack();
}
void BorderComposite::draw() {
	draw(AlloyApplicationContext().get());
}
}
