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
float DragBinTab::tabSize = 20.0f;
DragComposite::DragComposite(const std::string& name, const AUnit2D& pos,
		const AUnit2D& dims, const Orientation& orient, bool clamp) :
		ScrollPane(name, pos, dims, orient), clamp(clamp) {
	setOrientation(orient);
	Application::addListener(this);
}
bool DragComposite::onEventHandler(AlloyContext* context, const InputEvent& e) {
	dragRegion = context->getMouseDownObject();
	if (e.type == InputType::Cursor || e.type == InputType::MouseButton) {
		dragging = false;
		if (dragRegion != nullptr) {
			for (auto r : children) {
				if (r.get() == dragRegion) {
					dragging = true;
					break;
				}
			}
		}
	}
	if (dragging && e.type == InputType::Cursor) {
		dragRegion->setDragOffset(e.cursor, context->cursorDownPosition);
		context->requestPack();
	} else if (e.type == InputType::MouseButton && e.isUp()) {
		context->requestPack();
		dragging = false;
	}
	return ScrollPane::onEventHandler(context, e);
}
void DragComposite::add(const std::shared_ptr<Region>& region,
		bool appendToTab) {
	ScrollPane::add(region, appendToTab);
	region->setClampDragToParentBounds(clamp);
}
void DragComposite::insert(size_t idx, const std::shared_ptr<Region>& region,
		bool appendToTab) {
	ScrollPane::insert(idx, region, appendToTab);
	region->setClampDragToParentBounds(clamp);
}
void DragComposite::setEmptySlot(const box2px& box) {
	slotOffset =
			(orientation == Orientation::Vertical) ?
					box.dimensions.y : box.dimensions.x;
}
const std::vector<DragSlot>& DragComposite::getSlots() const {
	return sourceSlots;
}
void DragComposite::draw(AlloyContext* context) {
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
	if(slim){
		if(orientation==Orientation::Vertical&&isVerticalScrollVisible()){
			nvgBeginPath(nvg);
			nvgRect(nvg, bounds.position.x, bounds.position.y,Composite::slimScrollBarSize, bounds.dimensions.y);
			nvgFillColor(nvg,context->theme.DARKEST);
			nvgFill(nvg);
		} else 	if(orientation==Orientation::Horizontal&&isHorizontalScrollVisible()){
			nvgBeginPath(nvg);
			nvgRect(nvg, bounds.position.x, bounds.position.y,bounds.dimensions.x,Composite::slimScrollBarSize);
			nvgFillColor(nvg,context->theme.DARKEST);
			nvgFill(nvg);
		}
	}
	int srcIndex = -1;
	int tarIndex = -1;
	float dragOffset = 0.0f;
	for (const DragSlot& slot : sourceSlots) {
		if (dragRegion == slot.region || focusRegion == slot.region) {
			srcIndex = slot.index;
			dragOffset = (
					(orientation == Orientation::Vertical) ?
							slot.bounds.dimensions.y + cellSpacing.y :
							slot.bounds.dimensions.x + cellSpacing.x);
		}
		box2px lbounds = slot.bounds;
		lbounds.position += slot.region->getDrawOffset();
		lbounds.dimensions.y +=
				(orientation == Orientation::Vertical) ?
						cellSpacing.y : cellSpacing.x;
		if (context->isMouseContainedIn(lbounds)) {
			tarIndex = slot.index;
		}
	}
	std::vector<pixel2> starts(targetSlots.size());
	for (int i = 0; i < targetSlots.size(); i++) {
		DragSlot& slot = targetSlots[i];
		starts[i] = mix(slot.start, slot.bounds.position, slot.tween);
	}

	pixel2 cursor = context->cursorPosition;
	float offset = slotOffset;
	for (int i = 0; i < targetSlots.size(); i++) {
		const DragSlot& sslot = sourceSlots[i];
		DragSlot& tslot = targetSlots[i];
		if (starts.size() > 0) {
			tslot.start = starts[i]; //start from current location instead of bounds location
		}
		tslot.tween = 0.0f;
		float2 drawOffset = sslot.region->getDrawOffset();
		if (orientation == Orientation::Vertical) {
			if (dragging) {
				float off = 0.0f;
				if (srcIndex < tarIndex) {
					if (sslot.index > srcIndex) {
						off = -dragOffset;
					}
					if (tarIndex >= 0 && sslot.index > tarIndex) {
						off += dragOffset;
					}
				} else {
					if (sslot.index >= srcIndex) {
						off = -dragOffset;
					}
					if (tarIndex >= 0 && sslot.index >= tarIndex) {
						off += dragOffset;
					}
				}
				tslot.bounds.position.y = sslot.bounds.position.y + off;
			} else {
				if (sslot.bounds.position.y + drawOffset.y
						+ sslot.bounds.dimensions.y >= cursor.y) {
					tslot.bounds.position.y = sslot.bounds.position.y
							+ slotOffset;
				} else {
					//std::cout <<getName()<<": "<< children[i]->getName() <<std::endl;
					tslot.bounds.position.y = sslot.bounds.position.y;
				}
			}
		} else {
			if (dragging) {
				float off = 0.0f;
				if (srcIndex < tarIndex) {
					if (sslot.index > srcIndex) {
						off = -dragOffset;
					}
					if (tarIndex >= 0 && sslot.index > tarIndex) {
						off += dragOffset;
					}
				} else {
					if (sslot.index >= srcIndex) {
						off = -dragOffset;
					}
					if (tarIndex >= 0 && sslot.index >= tarIndex) {
						off += dragOffset;
					}
				}
				tslot.bounds.position.x = sslot.bounds.position.x + off;
			} else {
				if (sslot.bounds.position.x + drawOffset.x
						+ sslot.bounds.dimensions.x >= cursor.x) {
					tslot.bounds.position.x = sslot.bounds.position.x
							+ slotOffset;
				} else {
					tslot.bounds.position.x = sslot.bounds.position.x;
				}
			}
		}
		//std::cout << slot.index << " ";
	}
	bool move = false;
	if (timer.resetAfterElapsed(1 / 30.0f)) {
		move = true;
	}
	for (int idx = 0; idx < targetSlots.size(); idx++) {
		DragSlot& tslot = targetSlots[idx];
		DragSlot& sslot = sourceSlots[idx];
		DragSlot mslot = tslot;
		mslot.bounds.position = mix(tslot.start, tslot.bounds.position,
				tslot.tween);
		if (move) {
			tslot.tween = std::min(tslot.tween + 0.1f, 1.0f);
		}
		if (dragRegion != mslot.region) {
			if (dragging || offset != 0) {
				mslot.region->setDragOffset(
						mslot.bounds.position - sslot.bounds.position);
				mslot.region->draw(context);
			} else {
				tslot.bounds = sslot.bounds;
				mslot.region->setDragOffset(pixel2(0.0f));
				mslot.region->draw(context);
			}
		}
	}
	if (dragging) {
		if (focusRegion == nullptr) {
			timer.reset();
			focusRegion = dragRegion;
		}
		if (!dragRegion->isClampedToBounds()) {
			context->setOnTopRegion(dragRegion);
		} else {
			if (dragRegion->isVisible()) {
				dragRegion->draw(context);
			}
		}
		if (onDragOver) {
			onDragOver(focusRegion);
		}
		context->requestPack();
	} else {
		if (focusRegion != nullptr) {
			RegionPtr drop;
			if (tarIndex >= 0) {
				std::vector<aly::RegionPtr> newRegions;
				for (int i = 0; i < children.size(); i++) {
					auto r = children[i];
					r->setDragOffset(pixel2(0.0f));
					if (i == tarIndex && srcIndex >= tarIndex) {
						newRegions.push_back(children[srcIndex]);
					}
					if (i != srcIndex) {
						newRegions.push_back(r);
					}
					if (i == tarIndex && srcIndex < tarIndex) {
						newRegions.push_back(children[srcIndex]);
					}
					if (r.get() == focusRegion) {
						drop = r;
					}
				}
				children = newRegions;
			} else {
				for (int i = 0; i < children.size(); i++) {
					auto r = children[i];
					r->setDragOffset(pixel2(0.0f));
					if (r.get() == focusRegion) {
						drop = r;
					}
				}
			}
			targetSlots.clear();
			if (!focusRegion->isClampedToBounds()) {
				context->removeOnTopRegion(focusRegion);
				if (onDrop && drop.get() != nullptr) {
					onDrop(drop);
				}
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
		verticalScrollHandle->setSlim(slim);
		verticalScrollTrack->setSlim(slim);
		verticalScrollTrack->handle = verticalScrollHandle.get();
		verticalScrollHandle->track = verticalScrollTrack.get();
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
		horizontalScrollHandle->setSlim(slim);
		horizontalScrollTrack->setSlim(slim);
		horizontalScrollTrack->handle = horizontalScrollHandle.get();
		horizontalScrollHandle->track = horizontalScrollTrack.get();
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
	AlloyContext* context = AlloyApplicationContext().get();
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
		slot.bounds.position -= region->getDragOffset();
		slot.start = slot.bounds.position;
		if (orientation == Orientation::Horizontal) {
			offset.x += cellSpacing.x + cbounds.dimensions.x;
		}
		if (orientation == Orientation::Vertical) {
			offset.y += cellSpacing.y + cbounds.dimensions.y;
		}
		idx++;
		if (context->isOnTop(region.get())) {
			cbounds.clamp(bounds);
		}
		scrollExtent = aly::max(
				cbounds.dimensions + cbounds.position - this->bounds.position,
				scrollExtent);
	}
	if (targetSlots.size() != sourceSlots.size()) {
		targetSlots = sourceSlots;
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
	if (onPack)
		onPack();
}
DragCompositePtr DragBinComposite::getBin(int idx) const {
	return std::dynamic_pointer_cast<DragComposite>(
			std::dynamic_pointer_cast<Composite>(children[idx])->getChild(0));
}

void DragBinComposite::handleDragOver(Region* region) {
	AlloyContext* context = AlloyApplicationContext().get();
	aly::pixel2 cursor = context->getCursorPosition();
	for (int n = 0; n < children.size(); n++) {
		auto bin = getBin(n);
		bool found = false;
		Composite* comp = dynamic_cast<Composite*>(region->parent);
		if (comp != bin.get()) {
			box2px bounds = bin->getBounds();
			if (bin->isVisible() && context->isMouseContainedIn(bounds)) {
				std::vector<DragSlot> slots = bin->getSlots();
				pixel2 cellSpacing = bin->getCellSpacing();
				for (int m = 0; m < slots.size(); m++) {
					box2px bbox = slots[m].bounds;
					if (orientation == Orientation::Horizontal) {
						cellSpacing.x = 0;
					} else {
						cellSpacing.y = 0;
					}
					bbox.dimensions += cellSpacing;
					bbox.position += slots[m].region->getDrawOffset();
					if (bbox.contains(cursor)) {
						bin->setEmptySlot(bbox);
						found = true;
						break;
					}
				}
			}

			if (!found) {
				bin->setEmptySlot(box2px(pixel2(0.0f), pixel2(0.0f)));
			}
		}
	}
}

void DragBinComposite::handleDrop(const std::shared_ptr<Region>& region) {
	AlloyContext* context = AlloyApplicationContext().get();
	aly::pixel2 cursor = context->getCursorPosition();
	for (int n = 0; n < children.size(); n++) {
		auto bin = getBin(n);
		bool found = false;
		bin->setEmptySlot(box2px(pixel2(0.0f), pixel2(0.0f)));
		box2px bounds = bin->getBounds();
		if (bin->isVisible() && context->isMouseContainedIn(bounds)) {
			Composite* comp = dynamic_cast<Composite*>(region->parent);
			if (comp != bin.get()) {
				if (comp) {
					comp->erase(region);
				}
				const std::vector<DragSlot>& slots = bin->getSlots();
				pixel2 cellSpacing = bin->getCellSpacing();
				if (orientation == Orientation::Horizontal) {
					cellSpacing.x = 0;
				} else {
					cellSpacing.y = 0;
				}
				for (int m = 0; m < slots.size(); m++) {
					box2px bbox = slots[m].bounds;
					bbox.dimensions += cellSpacing;
					bbox.position += slots[m].region->getDrawOffset();
					if (bbox.contains(cursor)) {
						bin->insert(m, region);
						found = true;
						break;
					}
				}
				if (!found) {
					bin->add(region);
					found = true;
				}
			}
		}
	}
}
DragCompositePtr DragBinComposite::addBin(const std::string& name, int size) {
	DragCompositePtr dcomp;
	DragBinTabPtr comp;
	if (orientation == Orientation::Horizontal) {
		dcomp = DragCompositePtr(
				new DragComposite(name, CoordPX(0.0f, DragBinTab::tabSize),
						CoordPerPX(1.0f, 1.0f, 0.0f,
								-DragBinTab::tabSize
										- Composite::scrollBarSize),
						Orientation::Vertical, false));
		dcomp->onDrop = [this](const std::shared_ptr<Region>& region) {
			handleDrop(region);
		};
		dcomp->onDragOver = [this](Region* region) {
			handleDragOver(region);
		};
		comp = DragBinTabPtr(
				new DragBinTab(name + "_tab", CoordPX(0.0f, 0.0f),
						CoordPerPX(0.0f, 1.0f, (float) size, 0.0f),
						Orientation::Vertical));
		comp->add(dcomp);
	} else if (orientation == Orientation::Vertical) {
		dcomp = DragCompositePtr(
				new DragComposite(name, CoordPX(DragBinTab::tabSize, 0.0f),
						CoordPerPX(1.0f, 1.0f,
								-DragBinTab::tabSize - Composite::scrollBarSize,
								0.0f), Orientation::Horizontal, false));
		dcomp->onDrop = [this](const std::shared_ptr<Region>& region) {
			handleDrop(region);
		};
		dcomp->onDragOver = [this](Region* region) {
			handleDragOver(region);
		};
		comp = DragBinTabPtr(
				new DragBinTab(name + "_tab", CoordPX(0.0f, 0.0f),
						CoordPerPX(1.0f, 0.0f, 0.0f, (float) size),
						Orientation::Horizontal));
		comp->add(dcomp);
	} else {
		return DragCompositePtr();
	}
	dcomp->setSlimScroll(true);
	dcomp->setCellPadding(pixel2(3.0f));
	DragComposite::add(comp);
	return dcomp;
}
DragBinComposite::DragBinComposite(const std::string& name, const AUnit2D& pos,
		const AUnit2D& dims, const Orientation& orient) :
		DragComposite(name, pos, dims, orient) {
	setCellSpacing(pixel2(3.0f));
	setCellPadding(pixel2(3.0f));
}
DragBinTab::DragBinTab(const std::string& name, const AUnit2D& pos,
		const AUnit2D& dims, const Orientation& orient) :
		Composite(name, pos, dims), orient(orient) {

}
void DragBinTab::draw(AlloyContext* context) {
	box2px bounds = getBounds();
	NVGcontext* nvg = context->nvgContext;
	nvgBeginPath(nvg);
	if (orient == Orientation::Vertical) {
		nvgRoundedRect(nvg, bounds.position.x+Composite::slimScrollBarSize,bounds.position.y + context->theme.CORNER_RADIUS,
				bounds.dimensions.x-Composite::slimScrollBarSize,tabSize + context->theme.CORNER_RADIUS,
				context->theme.CORNER_RADIUS);
	} else if (orient == Orientation::Horizontal) {
		nvgRoundedRect(nvg, bounds.position.x + context->theme.CORNER_RADIUS,
				bounds.position.y+Composite::slimScrollBarSize,
				tabSize + context->theme.CORNER_RADIUS,
				bounds.dimensions.y-Composite::slimScrollBarSize, context->theme.CORNER_RADIUS);
	}
	if (context->isMouseDown(this, false)) {
		if (context->getCursor() == nullptr) {
			context->setCursor(&Cursor::Grab);
		}
		nvgFillColor(nvg, context->theme.LIGHTEST);
	} else if (context->isMouseOver(this, false)) {
		if (context->getCursor() == nullptr) {
			context->setCursor(&Cursor::Hand);
		}
		nvgFillColor(nvg, context->theme.LIGHTEST);
	} else {
		nvgFillColor(nvg, context->theme.LIGHTER);
	}
	nvgFill(nvg);
	Composite::draw(context);
}
} /* namespace aly */
