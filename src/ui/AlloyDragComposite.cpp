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
		const AUnit2D& dims, const Orientation& orient,bool clamp) :
		ScrollPane(name, pos, dims,orient),clamp(clamp) {
	setOrientation(orient);
}
void DragComposite::add(const std::shared_ptr<Region>& region,
		bool appendToTab) {
	Composite::add(region, appendToTab);
	region->setDragEnabled(true);
	region->setClampDragToParentBounds(clamp);
}
void DragComposite::draw(AlloyContext* context) {
	NVGcontext* nvg = context->nvgContext;
	box2px bounds = getBounds();
	float w = bounds.dimensions.x;
	float h = bounds.dimensions.y;
	pixel lineWidth = borderWidth.toPixels(bounds.dimensions.y, context->dpmm.y,context->pixelRatio);
	if(isHorizontalScrollHandleVisible()){
		if(leftButton.get()!=nullptr)leftButton->setVisible(scrollPosition.x>0.00001f);
		if(rightButton.get()!=nullptr)rightButton->setVisible(scrollPosition.x<0.9999f);
	} else {
		if(leftButton.get()!=nullptr)leftButton->setVisible(false);
		if(rightButton.get()!=nullptr)rightButton->setVisible(false);
	}
	if(isVerticalScrollHandleVisible()){
		if(upButton.get()!=nullptr)upButton->setVisible(scrollPosition.y>0.00001f);
		if(downButton.get()!=nullptr)downButton->setVisible(scrollPosition.y<0.9999f);
	} else {
		if(upButton.get()!=nullptr)upButton->setVisible(false);
		if(downButton.get()!=nullptr)downButton->setVisible(false);
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
	Region* dragRegion = context->getMouseDownObject();
	bool drawLater = false;
	bool dragging = false;
	for (std::shared_ptr<Region>& region : children) {
		if (context->getCursor() == nullptr
				&& context->isMouseOver(region.get(), true)) {
			context->setCursor(&Cursor::Hand);
		}
		if (dragRegion != region.get()) {
			if (region->isVisible() && focusRegion == nullptr) {
				region->draw(context);
			}
		} else {
			region->setDragOffset(context->cursorPosition,
					context->cursorDownPosition);
			drawLater = true;
			dragging = true;
		}
	}
	int srcIndex = -1, tarIndex = -1;
	if (dragRegion != nullptr) {
		for (DragSlot& slot : sourceSlots) {
			if (slot.region == dragRegion) {
				srcIndex = slot.index;
			}
			if (slot.bounds.contains(
					dragRegion->getBoundsPosition()
							+ 0.5f * dragRegion->getBoundsDimensions()
							- slot.region->getDrawOffset())) {
				tarIndex = slot.index;
			}
		}
		if (srcIndex != -1 && tarIndex != -1) {
			if (targetSlots.size() == 0
					|| targetSlots[tarIndex].index != srcIndex) {
				std::vector<pixel2> starts(targetSlots.size());
				for (int i = 0; i < targetSlots.size(); i++) {
					DragSlot& slot = targetSlots[i];
					starts[slot.index] = mix(slot.start, slot.bounds.position,
							slot.tween);
				}
				targetSlots.clear();
				if (tarIndex > srcIndex) {
					for (DragSlot& s : sourceSlots) {
						if (s.index != srcIndex) {
							targetSlots.push_back(s);

						}
						if (s.index == tarIndex) {
							targetSlots.push_back(sourceSlots[srcIndex]);
						}
					}
				} else {
					for (DragSlot& s : sourceSlots) {
						if (s.index == tarIndex) {
							targetSlots.push_back(sourceSlots[srcIndex]);
						}
						if (s.index != srcIndex) {
							targetSlots.push_back(s);
						}
					}
				}
				//std::cout << srcIndex << ":" << tarIndex << " Order: ";
				for (int i = 0; i < targetSlots.size(); i++) {
					DragSlot& slot = targetSlots[i];
					if (starts.size() > 0)
						slot.start = starts[slot.index]; //start from current location instead of bounds location
					slot.tween = 0.0f;
					slot.bounds = sourceSlots[i].bounds;
					//std::cout << slot.index << " ";
				}
				//std::cout << std::endl;
			}
		}
	}
	bool move = false;
	if (timer.resetAfterElapsed(1 / 30.0f)) {
		move = true;
	}
	for (int idx = 0; idx < targetSlots.size(); idx++) {
		DragSlot& tslot = targetSlots[idx];
		DragSlot mslot = tslot;
		mslot.bounds.position = mix(tslot.start, tslot.bounds.position,
				tslot.tween);
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
		if(!dragRegion->isClampedToBounds()){
			context->setOnTopRegion(dragRegion);
		} else {
			if(dragRegion->isVisible()){
				dragRegion->draw(context);
			}
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
			if(!focusRegion->isClampedToBounds()){
				context->removeOnTopRegion(focusRegion);
			}
			context->requestPack();
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
	sourceSlots.resize(children.size());
	int idx = 0;
	AlloyContext* context=AlloyApplicationContext().get();
	for (std::shared_ptr<Region>& region : children) {
		DragSlot& slot = sourceSlots[idx];
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
		slot.start = slot.bounds.position;
		slot.bounds.position -= region->getDragOffset();
		if (orientation == Orientation::Horizontal) {
			offset.x += cellSpacing.x + cbounds.dimensions.x;
		}
		if (orientation == Orientation::Vertical) {
			offset.y += cellSpacing.y + cbounds.dimensions.y;
		}
		idx++;
		if(context->isOnTop(region.get())){
			cbounds.clamp(bounds);
		}
		scrollExtent = aly::max(cbounds.dimensions + cbounds.position - this->bounds.position,scrollExtent);
	}
	if (focusRegion != nullptr) {
		if (targetSlots.size() == 0) {
			targetSlots = sourceSlots;
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
	if (leftButton.get() != nullptr) {
		leftButton->parent=parent;
		leftButton->pack(bounds.position, bounds.dimensions, dpmm, pixelRatio);
	}
	if (rightButton.get() != nullptr) {
		rightButton->parent=parent;
		rightButton->pack(bounds.position, bounds.dimensions, dpmm, pixelRatio);
	}
	if (upButton.get() != nullptr) {
		upButton->parent=parent;
		upButton->pack(bounds.position, bounds.dimensions, dpmm, pixelRatio);
	}
	if (downButton.get() != nullptr) {
		downButton->parent=parent;
		downButton->pack(bounds.position, bounds.dimensions, dpmm, pixelRatio);
	}
	if (onPack)
		onPack();
}
DragCompositePtr DragBinComposite::getBin(int idx) const{
	return std::dynamic_pointer_cast<DragComposite>(children[idx]);
}
DragCompositePtr DragBinComposite::addBin(const std::string& name, int size){
	DragCompositePtr comp;

	if(orientation==Orientation::Horizontal){
		comp=DragCompositePtr(new DragComposite(name,CoordPX(0.0f,0.0f),CoordPerPX(0.0f,1.0f,(float)size,-Composite::scrollBarSize),Orientation::Vertical,false));
	} else if(orientation==Orientation::Vertical){
		comp=DragCompositePtr(new DragComposite(name,CoordPX(0.0f,0.0f),CoordPerPX(1.0f,0.0f,-Composite::scrollBarSize,(float)size),Orientation::Horizontal,false));
	} else {
		return DragCompositePtr();
	}
	ScrollPane::add(comp);
	return comp;
}

DragBinComposite::DragBinComposite(const std::string& name, const AUnit2D& pos,const AUnit2D& dims, const Orientation& orient) : ScrollPane(name, pos, dims,orient) {
	setCellSpacing(pixel2(0.0f));
	setCellPadding(pixel2(0.0f));
}
} /* namespace aly */
