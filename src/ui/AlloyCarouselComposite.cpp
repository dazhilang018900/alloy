/*
 * AlloyScrollPanel.cpp
 *
 *  Created on: Feb 9, 2019
 *      Author: blake
 */

#include "AlloyCarouselComposite.h"

#include "AlloyDrawUtil.h"
#include "AlloyApplication.h"
namespace aly {

void CarouselComposite::updateCursor(CursorLocator* cursorLocator) {
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

void CarouselComposite::draw(AlloyContext* context) {
	NVGcontext* nvg = context->nvgContext;
	box2px bounds = getBounds();
	float w = bounds.dimensions.x;
	float h = bounds.dimensions.y;
	pixel lineWidth = borderWidth.toPixels(bounds.dimensions.y, context->dpmm.y,
			context->pixelRatio);
	if (isHorizontalScrollHandleVisible()) {
		if (leftButton.get() != nullptr)
			leftButton->setVisible(scrollPosition.x > 0.00001f);
		if (rightButton.get() != nullptr)
			rightButton->setVisible(scrollPosition.x < 0.9999f);
	} else {
		if (leftButton.get() != nullptr)
			leftButton->setVisible(false);
		if (rightButton.get() != nullptr)
			rightButton->setVisible(false);
	}
	if (isVerticalScrollHandleVisible()) {
		if (upButton.get() != nullptr)
			upButton->setVisible(scrollPosition.y > 0.00001f);
		if (downButton.get() != nullptr)
			downButton->setVisible(scrollPosition.y < 0.9999f);
	} else {
		if (upButton.get() != nullptr)
			upButton->setVisible(false);
		if (downButton.get() != nullptr)
			downButton->setVisible(false);
	}
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
	if (verticalScrollTrack.get() != nullptr
			|| horizontalScrollTrack.get() != nullptr) {
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
			if (leftButton.get() != nullptr && leftButton->isVisible())
				leftButton->draw(context);
			if (rightButton.get() != nullptr && rightButton->isVisible())
				rightButton->draw(context);
			if (upButton.get() != nullptr && upButton->isVisible())
				upButton->draw(context);
			if (downButton.get() != nullptr && downButton->isVisible())
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
void CarouselComposite::pack(const pixel2& pos, const pixel2& dims,
		const double2& dpmm, double pixelRatio, bool clamp) {
	Composite::pack(pos, dims, dpmm, pixelRatio, clamp);
	box2px bounds = getBounds(false);
	if(verticalScrollTrack.get()!=nullptr){
		verticalScrollHandle->setSlim(slim);
		verticalScrollTrack->setSlim(slim);
	}
	if(horizontalScrollTrack.get()!=nullptr){
		horizontalScrollHandle->setSlim(slim);
		horizontalScrollTrack->setSlim(slim);
	}
	if (leftButton.get() != nullptr) {
		leftButton->parent = parent;
		leftButton->pack(bounds.position, bounds.dimensions, dpmm, pixelRatio);
	}
	if (rightButton.get() != nullptr) {
		rightButton->parent = parent;
		rightButton->pack(bounds.position, bounds.dimensions, dpmm, pixelRatio);
	}
	if (upButton.get() != nullptr) {
		upButton->parent = parent;
		upButton->pack(bounds.position, bounds.dimensions, dpmm, pixelRatio);
	}
	if (downButton.get() != nullptr) {
		downButton->parent = parent;
		downButton->pack(bounds.position, bounds.dimensions, dpmm, pixelRatio);
	}
}
void CarouselComposite::drawDebug(AlloyContext* context) {
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

CarouselComposite::CarouselComposite(const std::string& name, const AUnit2D& pos,
		const AUnit2D& dims, const Orientation& orient, float scrollStep,
		float buttonWidth) :
		Composite(name, pos, dims){
	setScrollEnabled(true);
	setOrientation(orient);
	if (orient == Orientation::Horizontal) {
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
