/*
 * AlloyDragComposite.cpp
 *
 *  Created on: Feb 10, 2019
 *      Author: blake
 */

#include "AlloyDragComposite.h"
#include "AlloySlider.h"
#include "AlloyDrawUtil.h"
#include "AlloyApplication.h"
namespace aly {
DragComposite::DragComposite(const std::string& name, const AUnit2D& pos,
		const AUnit2D& dims, const Orientation& orient) :
		Composite(name, pos, dims) {
	setOrientation(orient);
}
void DragComposite::add(const std::shared_ptr<Region>& region,
		bool appendToTab) {
	Composite::add(region, appendToTab);
	region->setClampDragToParentBounds(true);
	region->setDragEnabled(true);
	region->onMouseDrag =
			[=](AlloyContext* context, const InputEvent& event) {
				region->setDragOffset(context->cursorPosition,context->cursorDownPosition);
				return true;
			};
}
void DragComposite::draw(AlloyContext* context) {
	NVGcontext* nvg = context->nvgContext;
	box2px bounds = getBounds();
	float w = bounds.dimensions.x;
	float h = bounds.dimensions.y;
	pixel lineWidth = borderWidth.toPixels(bounds.dimensions.y, context->dpmm.y,
			context->pixelRatio);
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
	Region* dragRegion = context->getMouseDownObject();
	bool drawLater = false;
	bool dragging = false;
	for (std::shared_ptr<Region>& region : children) {
		if (dragRegion != region.get()) {
			if (region->isVisible() && focusRegion == nullptr) {
				region->draw(context);
			}
		} else {
			drawLater = true;
			dragging = true;

		}
	}
	int srcIndex = -1, tarIndex = -1;
	if (dragRegion != nullptr) {
		for (DragSlot& slot : currentSlots) {
			if (slot.region == dragRegion) {
				srcIndex = slot.index;
			}
			if (slot.bounds.contains(
					context->getCursorPosition()
							- slot.region->getDrawOffset())) {
				tarIndex = slot.index;
			}
		}
		if (srcIndex != -1 && tarIndex != -1) {
			if (targetSlots.size() == 0
					|| targetSlots[tarIndex].index != srcIndex) {
				targetSlots.clear();
				if (tarIndex > srcIndex) {
					for (DragSlot& s : currentSlots) {
						if (s.index != srcIndex) {
							targetSlots.push_back(s);

						}
						if (s.index == tarIndex) {
							targetSlots.push_back(currentSlots[srcIndex]);
						}
					}
				} else {
					for (DragSlot& s : currentSlots) {
						s.tween = 0.0f;
						if (s.index == tarIndex) {
							targetSlots.push_back(currentSlots[srcIndex]);
						}
						if (s.index != srcIndex) {
							targetSlots.push_back(s);
						}
					}
				}
				std::cout << srcIndex << ":" << tarIndex << " Order: ";
				for (int i = 0; i < targetSlots.size(); i++) {
					DragSlot& slot = targetSlots[i];
					slot.tween =0.0f;// (slot.index == tarIndex||slot.index == tarIndex) ? 0.0f : 1.0f;
					slot.bounds = currentSlots[i].bounds;
					std::cout << slot.index << " ";
				}
				std::cout << std::endl;
			}
		}
	}
	bool move = false;
	if (timer.resetAfterElapsed(1 / 30.0f)) {
		move = true;
	}
	for (int idx = 0; idx < targetSlots.size(); idx++) {

		DragSlot& tslot = targetSlots[idx];
		DragSlot& sslot = currentSlots[tslot.index];
		DragSlot mslot = tslot;
		mslot.bounds.position = mix(sslot.bounds.position,
				tslot.bounds.position, tslot.tween);
		if (move) {
			tslot.tween = std::min(tslot.tween + 0.1f, 1.0f);
		}
		if (dragRegion != mslot.region) {
			mslot.region->setBounds(mslot.bounds);
			mslot.region->draw(context);
		}
	}
	if (drawLater) {
		if (focusRegion == nullptr) {
			timer.reset();
		}
		focusRegion = dragRegion;
		if (dragRegion->isVisible()) {
			dragRegion->draw(context);
		}
	} else {
		if (focusRegion != nullptr) {
			std::vector<aly::RegionPtr> newRegions(children.size());
			for (int i = 0; i < children.size(); i++) {
				auto r = children[targetSlots[i].index];
				r->setDragOffset(pixel2(0.0f));
				newRegions[i] = r;
			}
			children = newRegions;
			targetSlots.clear();
		}
		focusRegion = nullptr;
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
void DragSlot::draw(aly::AlloyContext* context) {
	NVGcontext* nvg = context->nvgContext;
	box2px bounds = this->bounds;
	if (region)
		bounds.position += region->getDrawOffset();
	nvgBeginPath(nvg);
	nvgRect(nvg, bounds.position.x + 2, bounds.position.y + 2,
			bounds.dimensions.x - 4, bounds.dimensions.y - 4);
	nvgStrokeWidth(nvg, 2.0f);
	nvgStrokeColor(nvg, Color(1.0f, 0.0f, 0.0f));
	nvgStroke(nvg);
	nvgTextAlign(nvg, NVG_ALIGN_TOP | NVG_ALIGN_LEFT);
	nvgFontSize(nvg, 20.0f);
	nvgFontFaceId(nvg, context->getFontHandle(FontType::Normal));
	drawText(nvg, bounds.position + pixel(4.0f), region->getName(),
			FontStyle::Outline);
}
void DragComposite::pack(const pixel2& pos, const pixel2& dims,
		const double2& dpmm, double pixelRatio, bool clamp) {
	Region::pack(pos, dims, dpmm, pixelRatio);
	box2px bounds = getBounds(false);
	if (verticalScrollTrack.get() == nullptr && isScrollEnabled()) {
		verticalScrollTrack = std::shared_ptr<ScrollTrack>(
				new ScrollTrack("Vert Track", Orientation::Vertical));
		verticalScrollTrack->position = CoordPercent(1.0f, 0.0f);
		verticalScrollTrack->dimensions = CoordPerPX(0.0, 1.0f, scrollBarSize,
				0.0f);
		verticalScrollTrack->setOrigin(Origin::TopRight);
		verticalScrollTrack->parent = parent;
		verticalScrollHandle = std::shared_ptr<ScrollHandle>(
				new ScrollHandle("Vert Handle", Orientation::Vertical));
		verticalScrollHandle->position = CoordPX(0.0f, 0.0f);
		verticalScrollHandle->dimensions = CoordPerPX(1.0f, 0.0f, 0.0f,
				scrollBarSize);
		verticalScrollHandle->parent = verticalScrollTrack.get();
		verticalScrollHandle->setDragEnabled(true);
		verticalScrollTrack->onMouseDown =
				[this](AlloyContext* context, const InputEvent& event) {
					if (event.button == GLFW_MOUSE_BUTTON_LEFT) {
						if (this->verticalScrollHandle->isVisible()) {
							this->verticalScrollHandle->setDragOffset(event.cursor, this->verticalScrollHandle->getBoundsDimensions() * 0.5f);
							context->setDragObject(verticalScrollHandle.get());
							this->scrollPosition.y = (this->verticalScrollHandle->getBoundsPositionY() - this->verticalScrollTrack->getBoundsPositionY()) /
							std::max(1.0f, (float)this->verticalScrollTrack->getBoundsDimensionsY() - (float)this->verticalScrollHandle->getBoundsDimensionsY());
							updateExtents();
							context->requestPack();
							return false;
						}
						else {
							return false;
						}
					}
					return false;
				};
		verticalScrollHandle->onMouseDrag =
				[this](AlloyContext* context, const InputEvent& event) {
					if (context->isLeftMouseButtonDown()) {
						this->verticalScrollHandle->setDragOffset(event.cursor, context->getRelativeCursorDownPosition());
						this->scrollPosition.y = (this->verticalScrollHandle->getBoundsPositionY() - this->verticalScrollTrack->getBoundsPositionY()) /
						std::max(1.0f, (float)this->verticalScrollTrack->getBoundsDimensionsY() - (float)this->verticalScrollHandle->getBoundsDimensionsY());
						updateExtents();
					}
					return false;
				};
		horizontalScrollTrack = std::shared_ptr<ScrollTrack>(
				new ScrollTrack("Horiz Track", Orientation::Horizontal));
		horizontalScrollTrack->position = CoordPercent(0.0f, 1.0f);
		horizontalScrollTrack->dimensions = CoordPerPX(1.0, 0.0f, 0.0f,
				scrollBarSize);
		horizontalScrollTrack->setOrigin(Origin::BottomLeft);
		verticalScrollTrack->parent = parent;
		horizontalScrollHandle = std::shared_ptr<ScrollHandle>(
				new ScrollHandle("Horiz Handle", Orientation::Horizontal));
		horizontalScrollHandle->position = CoordPX(0.0f, 0.0f);
		horizontalScrollHandle->dimensions = CoordPerPX(0.0f, 1.0f,
				scrollBarSize, 0.0f);
		horizontalScrollHandle->parent = horizontalScrollTrack.get();
		horizontalScrollHandle->setDragEnabled(true);
		horizontalScrollTrack->onMouseDown =
				[this](AlloyContext* context, const InputEvent& event) {
					if (event.button == GLFW_MOUSE_BUTTON_LEFT) {
						if (this->horizontalScrollHandle->isVisible()) {
							this->horizontalScrollHandle->setDragOffset(event.cursor, this->horizontalScrollHandle->getBoundsDimensions() * 0.5f);
							context->setDragObject(horizontalScrollHandle.get());
							this->scrollPosition.x = (this->horizontalScrollHandle->getBoundsPositionX() - this->horizontalScrollTrack->getBoundsPositionX()) /
							std::max(1.0f, (float)this->horizontalScrollTrack->getBoundsDimensionsX() - (float)this->horizontalScrollHandle->getBoundsDimensionsX());
							updateExtents();
							context->requestPack();
							return false;
						}
						else {
							return false;
						}
					}
					return false;
				};
		horizontalScrollHandle->onMouseDrag =
				[this](AlloyContext* context, const InputEvent& event) {
					if (context->isLeftMouseButtonDown()) {
						this->horizontalScrollHandle->setDragOffset(event.cursor, context->getRelativeCursorDownPosition());
						this->scrollPosition.x = (this->horizontalScrollHandle->getBoundsPositionX() - this->horizontalScrollTrack->getBoundsPositionX()) /
						std::max(1.0f, (float)this->horizontalScrollTrack->getBoundsDimensionsX() - (float)this->horizontalScrollHandle->getBoundsDimensionsX());
						updateExtents();
					}
					return false;
				};
		Application::addListener(this);
	}
	pixel2 offset = cellPadding;
	pixel2 scrollExtent = pixel2(0.0f);
	currentSlots.resize(children.size());
	int idx = 0;
	for (std::shared_ptr<Region>& region : children) {
		DragSlot& slot = currentSlots[idx];
		slot.index = idx;
		slot.region = region.get();
		if (!region->isVisible()) {
			slot.bounds = box2px();
			idx++;
			continue;
		}
		if (orientation == Orientation::Vertical) {
			pixel2 pix = region->position.toPixels(bounds.dimensions, dpmm,
					pixelRatio);
			region->position = CoordPX(pix.x, offset.y);
		}
		if (orientation == Orientation::Horizontal) {
			pixel2 pix = region->position.toPixels(bounds.dimensions, dpmm,
					pixelRatio);
			region->position = CoordPX(offset.x, pix.y);
		}
		region->pack(bounds.position, bounds.dimensions, dpmm, pixelRatio);
		box2px cbounds = region->getBounds(false);
		slot.bounds = cbounds;
		slot.bounds.position -= region->getDragOffset();
		if (orientation == Orientation::Horizontal) {
			offset.x += cellSpacing.x + cbounds.dimensions.x;
		}
		if (orientation == Orientation::Vertical) {
			offset.y += cellSpacing.y + cbounds.dimensions.y;
		}
		idx++;
		scrollExtent = aly::max(
				cbounds.dimensions + cbounds.position - this->bounds.position,
				scrollExtent);
	}
	if (focusRegion != nullptr) {
		if (targetSlots.size() == 0) {
			targetSlots = currentSlots;
		}
	} else {
		targetSlots.clear();
	}
	extents.dimensions = scrollExtent;
	if (!isScrollEnabled()) {
		if (orientation == Orientation::Horizontal) {
			extents.dimensions.x = this->bounds.dimensions.x =
					bounds.dimensions.x = std::max(bounds.dimensions.x,
							offset.x - cellSpacing.x + cellPadding.x);
		}
		if (orientation == Orientation::Vertical) {
			extents.dimensions.y = this->bounds.dimensions.y =
					bounds.dimensions.y = std::max(bounds.dimensions.y,
							offset.y - cellSpacing.y + cellPadding.y);
		}

	}
	if (verticalScrollTrack.get() != nullptr
			|| horizontalScrollTrack.get() != nullptr) {
		bool showY = extents.dimensions.y > bounds.dimensions.y
				|| alwaysShowVerticalScrollBar;
		bool showX = extents.dimensions.x > bounds.dimensions.x
				|| alwaysShowHorizontalScrollBar;
		float nudge = (showX && showY) ? -scrollBarSize : 0;

		verticalScrollTrack->dimensions = CoordPerPX(0.0f, 1.0f, scrollBarSize,
				nudge);
		verticalScrollTrack->pack(bounds.position, bounds.dimensions, dpmm,
				pixelRatio);
		horizontalScrollTrack->dimensions = CoordPerPX(1.0f, 0.0f, nudge,
				scrollBarSize);
		horizontalScrollTrack->pack(bounds.position, bounds.dimensions, dpmm,
				pixelRatio);

		verticalScrollHandle->dimensions = CoordPX(scrollBarSize,
				std::max(scrollBarSize,
						(verticalScrollTrack->getBoundsDimensionsY()
								* bounds.dimensions.y) / extents.dimensions.y));
		verticalScrollHandle->pack(verticalScrollTrack->getBoundsPosition(),
				verticalScrollTrack->getBoundsDimensions(), dpmm, pixelRatio,
				true);
		verticalScrollHandle->clampDragOffset();
		verticalScrollHandle->pack(verticalScrollTrack->getBoundsPosition(),
				verticalScrollTrack->getBoundsDimensions(), dpmm, pixelRatio,
				true);
		this->scrollPosition.y =
				(this->verticalScrollHandle->getBoundsPositionY()
						- this->verticalScrollTrack->getBoundsPositionY())
						/ std::max(1.0f,
								(float) this->verticalScrollTrack->getBoundsDimensionsY()
										- (float) this->verticalScrollHandle->getBoundsDimensionsY());

		horizontalScrollHandle->dimensions = CoordPX(
				std::max(scrollBarSize,
						(horizontalScrollTrack->getBoundsDimensionsX()
								* bounds.dimensions.x) / extents.dimensions.x),
				scrollBarSize);
		horizontalScrollHandle->pack(horizontalScrollTrack->getBoundsPosition(),
				horizontalScrollTrack->getBoundsDimensions(), dpmm, pixelRatio,
				true);
		horizontalScrollHandle->clampDragOffset();
		horizontalScrollHandle->pack(horizontalScrollTrack->getBoundsPosition(),
				horizontalScrollTrack->getBoundsDimensions(), dpmm, pixelRatio,
				true);
		this->scrollPosition.x =
				(this->horizontalScrollHandle->getBoundsPositionX()
						- this->horizontalScrollTrack->getBoundsPositionX())
						/ std::max(1.0f,
								(float) this->horizontalScrollTrack->getBoundsDimensionsX()
										- (float) this->horizontalScrollHandle->getBoundsDimensionsX());

		updateExtents();
		if (isScrollEnabled()) {
			if (this->verticalScrollHandle->getBoundsDimensionsY()
					< this->verticalScrollTrack->getBoundsDimensionsY() - 1) { //subtract one to avoid round off error in determination track bar size!
				verticalScrollTrack->setVisible(true);
				verticalScrollHandle->setVisible(true);
			} else {
				verticalScrollTrack->setVisible(alwaysShowVerticalScrollBar);
				verticalScrollHandle->setVisible(false);
			}
			if (this->horizontalScrollHandle->getBoundsDimensionsX()
					< this->horizontalScrollTrack->getBoundsDimensionsX() - 1) { //subtract one to avoid round off error in determination track bar size!
				horizontalScrollTrack->setVisible(true);
				horizontalScrollHandle->setVisible(true);
			} else {
				horizontalScrollTrack->setVisible(
						alwaysShowHorizontalScrollBar);
				horizontalScrollHandle->setVisible(false);
			}
		} else {
			horizontalScrollHandle->setVisible(false);
			verticalScrollHandle->setVisible(false);
			horizontalScrollTrack->setVisible(alwaysShowHorizontalScrollBar);
			verticalScrollTrack->setVisible(alwaysShowVerticalScrollBar);
		}
	}
	for (std::shared_ptr<Region>& region : children) {
		if (region->onPack)
			region->onPack();
	}

	if (onPack)
		onPack();
}

} /* namespace aly */
