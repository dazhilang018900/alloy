/*
 * AlloyComposite.cpp
 *
 *  Created on: Sep 13, 2018
 *      Author: blake
 */

#include "ui/AlloyComposite.h"
#include "ui/AlloyApplication.h"
#include "ui/AlloyDrawUtil.h"
#include "ui/AlloySlider.h"
namespace aly {
const float Composite::scrollBarSize = 15.0f;
const float Composite::slimScrollBarSize = 2.0f;
std::shared_ptr<Composite> MakeComposite(const std::string& name,
		const AUnit2D& position, const AUnit2D& dimensions,
		const Color& bgColor, const Color& borderColor,
		const AUnit1D& borderWidth, const Orientation& orientation) {
	std::shared_ptr<Composite> composite = std::shared_ptr<Composite>(
			new Composite(name));
	composite->position = position;
	composite->dimensions = dimensions;
	composite->backgroundColor = MakeColor(bgColor);
	composite->borderColor = MakeColor(borderColor);
	composite->borderWidth = borderWidth;
	composite->setOrientation(orientation);
	return composite;
}
void Composite::setCellPadding(const pixel2& pix) {
	cellPadding = pix;
}
void Composite::setCellSpacing(const pixel2& pix) {
	cellSpacing = pix;
}
void Composite::setAlwaysShowVerticalScrollBar(bool show) {
	alwaysShowVerticalScrollBar = show;
	scrollEnabled |= show;
}
void Composite::setAlwaysShowHorizontalScrollBar(bool show) {
	alwaysShowHorizontalScrollBar = show;
	scrollEnabled |= show;
}
bool Composite::isVerticalScrollVisible() const {
	if (verticalScrollTrack.get() == nullptr) {
		return false;
	}
	return verticalScrollTrack->isVisible();
}
bool Composite::isVerticalScrollHandleVisible() const {
	if (verticalScrollHandle.get() == nullptr) {
		return false;
	}
	return verticalScrollHandle->isVisible();
}
Orientation Composite::getOrientation() const {
	return orientation;
}
bool Composite::isHorizontalScrollHandleVisible() const {
	if (horizontalScrollHandle.get() == nullptr) {
		return false;
	}
	return horizontalScrollHandle->isVisible();
}
bool Composite::isHorizontalScrollVisible() const {
	if (horizontalScrollTrack.get() == nullptr) {
		return false;
	}
	return horizontalScrollTrack->isVisible();
}
void Composite::insert(size_t idx,const std::shared_ptr<Region>& region,bool appendToTab){
	if (region.get() == nullptr) {
		throw std::runtime_error(
				MakeString() << "Could not add nullptr region to composite ["
						<< getName() << "]");
	}
	children.insert(children.begin()+idx,region);
	if (region->parent != nullptr)
		throw std::runtime_error(
				MakeString() << "Cannot add child node [" << region->name
						<< "] to [" << name
						<< "] because it already has a parent ["
						<< region->parent->name << "].");
	region->parent = this;
	if (appendToTab) {
		appendToTabChain(region.get());
	}
}
void Composite::add(const std::shared_ptr<Region>& region, bool appendToTab) {
	if (region.get() == nullptr) {
		throw std::runtime_error(
				MakeString() << "Could not add nullptr region to composite ["
						<< getName() << "]");
	}
	children.push_back(region);

	if (region->parent != nullptr)
		throw std::runtime_error(
				MakeString() << "Cannot add child node [" << region->name
						<< "] to [" << name
						<< "] because it already has a parent ["
						<< region->parent->name << "].");
	region->parent = this;
	if (appendToTab) {
		appendToTabChain(region.get());
	}
}
void Composite::insertAtFront(const std::shared_ptr<Region>& region) {
	children.insert(children.begin(), region);
	if (region->parent != nullptr)
		throw std::runtime_error(
				MakeString() << "Cannot add child node [" << region->name
						<< "] to [" << name
						<< "] because it already has a parent ["
						<< region->parent->name << "].");
	region->parent = this;
}

Composite::Composite(const std::string& name) :
		Region(name), cellPadding(0, 0), cellSpacing(5, 5) {

}
Composite::Composite(const std::string& name, const AUnit2D& pos,
		const AUnit2D& dims) :
		Region(name, pos, dims), cellPadding(0, 0), cellSpacing(5, 5) {
}
bool Composite::onEventHandler(AlloyContext* context, const InputEvent& event) {
	if (isVisible() && isDragEnabled()) {
		//Steal dragging priority.
		Region* mouseDownRegion = context->getMouseDownObject();
		if (mouseDownRegion != nullptr && !mouseDownRegion->isDragEnabled()) {
			if (mouseDownRegion->hasParent(this)) {
				context->setMouseDownObject(this);
			}
		}
	}
	if (isVisible() && event.type == InputType::Scroll && isScrollEnabled()) {
		box2px bounds = getBounds();
		if (bounds.contains(event.cursor)) {
			if (event.scroll.y != 0 && verticalScrollHandle.get() != nullptr
					&& verticalScrollHandle->isVisible()) {
				verticalScrollHandle->addDragOffset(
						pixel2(-10.0f * event.scroll));
				this->scrollPosition.y =
						(this->verticalScrollHandle->getBoundsPositionY()
								- this->verticalScrollTrack->getBoundsPositionY())
								/ std::max(1.0f,
										(float) this->verticalScrollTrack->getBoundsDimensionsY()
												- (float) this->verticalScrollHandle->getBoundsDimensionsY());
				updateExtents();
				context->requestPack();
				return true;
			}
			if (event.scroll.x != 0 && horizontalScrollHandle.get() != nullptr
					&& horizontalScrollHandle->isVisible()) {
				horizontalScrollHandle->addDragOffset(
						pixel2(-10.0f * event.scroll));
				this->scrollPosition.x =
						(this->horizontalScrollHandle->getBoundsPositionX()
								- this->horizontalScrollTrack->getBoundsPositionX())
								/ std::max(1.0f,
										(float) this->horizontalScrollTrack->getBoundsDimensionsX()
												- (float) this->horizontalScrollHandle->getBoundsDimensionsX());
				updateExtents();
				context->requestPack();
				return true;
			}
		}
	}
	return Region::onEventHandler(context, event);
}

void Composite::setOrientation(const Orientation& orient, pixel2 cellSpacing,
		pixel2 cellPadding) {
	orientation = orient;
	this->cellSpacing = cellSpacing;
	this->cellPadding = cellPadding;
}
bool Composite::isScrollEnabled() const {
	return scrollEnabled;
}
void Composite::setScrollEnabled(bool enabled) {
	scrollEnabled = enabled;
}

pixel2 Composite::getDrawOffset() const {
	pixel2 offset = getExtents().position;
	if (parent != nullptr)
		offset += parent->getDrawOffset();
	return offset;
}
void Composite::updateCursor(CursorLocator* cursorLocator) {
	if (!ignoreCursorEvents)
		cursorLocator->add(this);
	for (std::shared_ptr<Region>& region : children) {
		region->updateCursor(cursorLocator);
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
void Composite::erase(const std::shared_ptr<Region>& node) {
	for (auto iter = children.begin(); iter != children.end(); iter++) {
		if (node.get() == iter->get()) {
			children.erase(iter);
			node->parent = nullptr;
			AlloyDefaultContext()->clearEvents(node.get());
			break;
		}
	}
	tabChain.remove(node.get());
	if(onErase){
		onErase(node.get());
	}
}
void Composite::erase(Region* node) {
	for (auto iter = children.begin(); iter != children.end(); iter++) {
		if (node == iter->get()) {
			children.erase(iter);
			node->parent = nullptr;
			AlloyDefaultContext()->clearEvents(node);
			break;
		}
	}
	tabChain.remove(node);
	if(onErase){
		onErase(node);
	}
}
void Composite::putLast(const std::shared_ptr<Region>& region) {
	size_t idx = 0;
	size_t pivot = children.size() - 1;
	std::vector<std::shared_ptr<Region>> newList;
	for (RegionPtr& child : children) {
		if (child.get() == region.get()) {
			pivot = idx;
		} else {
			newList.push_back(child);
		}
		idx++;
	}
	newList.push_back(children[pivot]);
	children = newList;
	AlloyApplicationContext()->requestUpdateCursorLocator();
}
void Composite::putFirst(const std::shared_ptr<Region>& region) {
	size_t idx = 0;
	size_t pivot = 0;
	std::vector<std::shared_ptr<Region>> newList;
	for (RegionPtr& child : children) {
		if (child.get() == region.get()) {
			pivot = idx;
		} else {
			newList.push_back(child);
		}
		idx++;
	}
	newList.insert(newList.begin(), children[pivot]);
	children = newList;
	AlloyApplicationContext()->requestUpdateCursorLocator();
}
void Composite::putLast(Region* region) {
	size_t idx = 0;
	size_t pivot = children.size() - 1;
	std::vector<std::shared_ptr<Region>> newList;
	for (RegionPtr& child : children) {
		if (child.get() == region) {
			pivot = idx;
		} else {
			newList.push_back(child);
		}
		idx++;
	}
	newList.push_back(children[pivot]);
	children = newList;
	AlloyApplicationContext()->requestUpdateCursorLocator();
}
void Composite::putFirst(Region* region) {
	size_t idx = 0;
	size_t pivot = 0;
	std::vector<std::shared_ptr<Region>> newList;
	for (RegionPtr& child : children) {
		if (child.get() == region) {
			pivot = idx;
		} else {
			newList.push_back(child);
		}
		idx++;
	}
	newList.insert(newList.begin(), children[pivot]);
	children = newList;
	AlloyApplicationContext()->requestUpdateCursorLocator();
}
std::shared_ptr<Region> Composite::getChild(size_t index) const {
	return children[index];
}
std::vector<std::shared_ptr<Region>>& Composite::getChildren() {
	return children;
}
const std::vector<std::shared_ptr<Region>>& Composite::getChildren() const{
	return children;
}
size_t Composite::getChildrenSize() const{
	return children.size();
}
void Composite::clear() {
	AlloyDefaultContext()->clearEvents(this);
	setDragOffset(pixel2(0, 0));
	for (RegionPtr node : children) {
		node->removeListener();
		node->parent = nullptr;
	}
	tabChain.clear();
	children.clear();
}
Region* Composite::locate(const pixel2& cursor) {
	if (isVisible()) {
		for (auto iter = children.rbegin(); iter != children.rend(); iter++) {
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

void Composite::draw(AlloyContext* context) {
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
void Composite::drawDebug(AlloyContext* context) {
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
	}
}
Composite::~Composite() {
	for (RegionPtr child : children) {
		Application::removeListener(child.get());
	}
}
void Composite::draw() {
	draw(AlloyApplicationContext().get());
}

bool Composite::addVerticalScrollPosition(float t) {
	if (verticalScrollHandle.get() != nullptr
			&& verticalScrollHandle->addDragOffset(pixel2(0.0f, t))) {
		this->scrollPosition.y =
				(this->verticalScrollHandle->getBoundsPositionY()
						- this->verticalScrollTrack->getBoundsPositionY())
						/ std::max(1.0f,
								(float) this->verticalScrollTrack->getBoundsDimensionsY()
										- (float) this->verticalScrollHandle->getBoundsDimensionsY());
		updateExtents();
		AlloyApplicationContext()->requestPack();
		if (onScroll) {
			onScroll(scrollPosition);
		}
		return true;
	}
	return false;
}
void Composite::scrollToBottom() {
	if (verticalScrollHandle.get() != nullptr) {
		float shift =
				this->verticalScrollTrack->getBoundsPositionY()
						+ std::max(0.0f,
								this->verticalScrollTrack->getBoundsDimensionsY()
										- (float) this->verticalScrollHandle->getBoundsDimensionsY());
		verticalScrollHandle->setDragOffset(pixel2(0.0f, shift));
		this->scrollPosition.y =
				(this->verticalScrollHandle->getBoundsPositionY()
						- this->verticalScrollTrack->getBoundsPositionY())
						/ std::max(1.0f,
								(float) this->verticalScrollTrack->getBoundsDimensionsY()
										- (float) this->verticalScrollHandle->getBoundsDimensionsY());
		updateExtents();
		AlloyApplicationContext()->requestPack();
		if (onScroll) {
			onScroll(scrollPosition);
		}

	}
}

void Composite::scrollToTop() {
	if (verticalScrollHandle.get() != nullptr) {
		verticalScrollHandle->setDragOffset(pixel2(0.0f, 0.0f));
		this->scrollPosition.y =
				(this->verticalScrollHandle->getBoundsPositionY()
						- this->verticalScrollTrack->getBoundsPositionY())
						/ std::max(1.0f,
								(float) this->verticalScrollTrack->getBoundsDimensionsY()
										- (float) this->verticalScrollHandle->getBoundsDimensionsY());
		updateExtents();
		AlloyApplicationContext()->requestPack();
		if (onScroll) {
			onScroll(scrollPosition);
		}

	}
}
void Composite::scrollToLeft() {
	if (horizontalScrollHandle.get() != nullptr) {
		horizontalScrollHandle->setDragOffset(pixel2(0.0f, 0.0f));
		this->scrollPosition.x =
				(this->horizontalScrollHandle->getBoundsPositionX()
						- this->horizontalScrollTrack->getBoundsPositionX())
						/ std::max(1.0f,
								(float) this->horizontalScrollTrack->getBoundsDimensionsX()
										- (float) this->horizontalScrollHandle->getBoundsDimensionsX());
		updateExtents();
		AlloyApplicationContext()->requestPack();
		if (onScroll) {
			onScroll(scrollPosition);
		}
	}
}
void Composite::scrollToRight() {
	if (horizontalScrollHandle.get() != nullptr) {
		float shift =
				this->horizontalScrollTrack->getBoundsPositionX()
						+ std::max(0.0f,
								this->horizontalScrollTrack->getBoundsDimensionsX()
										- (float) this->horizontalScrollHandle->getBoundsDimensionsX());
		horizontalScrollHandle->setDragOffset(pixel2(shift, 0.0f));
		this->scrollPosition.x =
				(this->horizontalScrollHandle->getBoundsPositionX()
						- this->horizontalScrollTrack->getBoundsPositionX())
						/ std::max(1.0f,
								(float) this->horizontalScrollTrack->getBoundsDimensionsX()
										- (float) this->horizontalScrollHandle->getBoundsDimensionsX());
		updateExtents();
		AlloyApplicationContext()->requestPack();
		if (onScroll) {
			onScroll(scrollPosition);
		}
	}
}
bool Composite::addHorizontalScrollPosition(float t) {
	if (horizontalScrollHandle.get() != nullptr
			&& horizontalScrollHandle->addDragOffset(pixel2(t, 0.0f))) {
		this->scrollPosition.x =
				(this->horizontalScrollHandle->getBoundsPositionX()
						- this->horizontalScrollTrack->getBoundsPositionX())
						/ std::max(1.0f,
								(float) this->horizontalScrollTrack->getBoundsDimensionsX()
										- (float) this->horizontalScrollHandle->getBoundsDimensionsX());
		updateExtents();
		AlloyApplicationContext()->requestPack();
		if (onScroll) {
			onScroll(scrollPosition);
		}
		return true;
	}
	return false;
}
void Composite::resetScrollPosition() {
	if (verticalScrollHandle.get() != nullptr) {
		verticalScrollHandle->setDragOffset(pixel2(0, 0));
	}
	if (horizontalScrollHandle.get() != nullptr) {
		horizontalScrollHandle->setDragOffset(pixel2(0, 0));
	}
	if (onScroll) {
		onScroll(scrollPosition);
	}
	updateExtents();
}
void Composite::updateExtents() {
	extents.position = aly::round(
			-scrollPosition
					* aly::max(pixel2(0, 0),
							extents.dimensions - bounds.dimensions));
}
void Composite::pack() {
	Region::pack();
}
void Composite::pack(AlloyContext* context) {
	Region::pack(context);
}
void Composite::pack(const pixel2& pos, const pixel2& dims, const double2& dpmm,
		double pixelRatio, bool clamp) {
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
		verticalScrollTrack->handle=verticalScrollHandle.get();
		verticalScrollHandle->track=verticalScrollTrack.get();
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
		horizontalScrollTrack->handle=horizontalScrollHandle.get();
		horizontalScrollHandle->track=horizontalScrollTrack.get();
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
	AlloyContext* context=AlloyApplicationContext().get();
	for (std::shared_ptr<Region>& region : children) {
		if (!region->isVisible()) {
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
		box2px cbounds = region->getBounds();
		if (orientation == Orientation::Horizontal) {
			offset.x += cellSpacing.x + cbounds.dimensions.x;

		}
		if (orientation == Orientation::Vertical) {
			offset.y += cellSpacing.y + cbounds.dimensions.y;
		}
			scrollExtent = aly::max(
					cbounds.dimensions + cbounds.position - this->bounds.position,
					scrollExtent);

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
}

