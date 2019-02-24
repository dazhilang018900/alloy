/*
 * AlloyRegion.cpp
 *
 *  Created on: Sep 13, 2018
 *      Author: blake
 */

#include "AlloyRegion.h"
#include "ui/AlloyApplication.h"
#include "ui/AlloyDrawUtil.h"
#include "system/AlloyFileUtil.h"
#include <cctype>
namespace aly {
uint64_t Region::REGION_COUNTER = 0;
const RGBA DEBUG_STROKE_COLOR = RGBA(32, 32, 200, 255);
const RGBA DEBUG_HIDDEN_COLOR = RGBA(128, 128, 128, 0);
const RGBA DEBUG_HOVER_COLOR = RGBA(32, 200, 32, 255);
const RGBA DEBUG_DOWN_COLOR = RGBA(200, 64, 32, 255);
const RGBA DEBUG_ON_TOP_COLOR = RGBA(120, 120, 0, 255);
const RGBA DEBUG_ON_TOP_DOWN_COLOR = RGBA(220, 220, 0, 255);
const RGBA DEBUG_ON_TOP_HOVER_COLOR = RGBA(180, 180, 0, 255);
const RGBA DEBUG_OBJECT_FOCUS_DOWN_COLOR = RGBA(255, 64, 242, 255);
const RGBA DEBUG_OBJECT_FOCUS_HOVER_COLOR = RGBA(254, 120, 248, 255);
void TabChain::add(Region* region) {
	region->tabChain.parent = this;
	regions.push_back(region);
}
TabChain::TabChain() :
		parent(nullptr) {
}
void TabChain::focusNext(Region* region) {
	for (auto iter = regions.begin(); iter != regions.end(); iter++) {
		if (*iter == region) {
			iter++;
			if (iter == regions.end()) {
				regions.front()->setFocus(true);
				break;
			} else {
				(*iter)->setFocus(true);
				break;
			}
		}
	}
}
void TabChain::focusPrevious(Region* region) {
	for (auto iter = regions.begin(); iter != regions.end(); iter++) {
		if (*iter == region) {
			if (iter == regions.begin()) {
				regions.back()->setFocus(true);
				break;
			} else {
				iter--;
				(*iter)->setFocus(true);
				break;
			}
		}
	}
}
void TabChain::remove(Region* region) {
	for (auto iter = regions.begin(); iter != regions.end(); iter++) {
		if (*iter == region) {
			regions.erase(iter);
			if (region->tabChain.parent == this)
				region->tabChain.parent = nullptr;
			break;
		}
	}
}
TabChain::~TabChain() {
	clear();
}
void TabChain::clear() {
	for (Region* region : regions) {
		if (region->tabChain.parent == this)
			region->tabChain.parent = nullptr;
	}
	regions.clear();
}
void Region::focusNext() {
	if (tabChain.parent != nullptr) {
		tabChain.parent->focusNext(this);
	}
}
void Region::focusPrevious() {
	if (tabChain.parent != nullptr) {
		tabChain.parent->focusPrevious(this);
	}
}
void Region::printFocus() {
	if (tabChain.parent != nullptr) {
		std::cout << "Tab Chain: " << *tabChain.parent << std::endl;
	} else {
		std::cout << getName() << " has no tab chain." << std::endl;
	}
}
void Region::appendTo(TabChain& chain) {
	chain.add(this);
}
std::shared_ptr<Region> MakeRegion(const std::string& name,
		const AUnit2D& position, const AUnit2D& dimensions,
		const Color& bgColor, const Color& borderColor,
		const AUnit1D& borderWidth) {
	std::shared_ptr<Region> region = std::shared_ptr<Region>(new Region(name));
	region->position = position;
	region->dimensions = dimensions;
	region->backgroundColor = MakeColor(bgColor);
	region->borderColor = MakeColor(borderColor);
	region->borderWidth = borderWidth;
	return region;
}
void Region::updateCursor(CursorLocator* cursorLocator) {
	if (!ignoreCursorEvents)
		cursorLocator->add(this);
}
void Region::removeListener() const {
	Application::removeListener(this);
}
bool Region::isCursorFocused() const {
	return AlloyApplicationContext()->isCursorFocused(this);
}
bool Region::hasRoundedCorners() const{
	return roundCorners;
}
bool Region::isClampedToBounds() const{
	return clampToParentBounds;
}
void Region::setFocus(bool f) {
	if (f) {
		AlloyApplicationContext()->setObjectFocus(this);
		AlloyApplicationContext()->setCursorFocus(this);
	} else {
		Region* ptr = AlloyApplicationContext()->getObjectFocus();
		if (ptr == this) {
			AlloyApplicationContext()->setObjectFocus(nullptr);
		}
	}
}
bool Region::isObjectFocused() const {
	return AlloyApplicationContext()->isObjectFocused(this);
}
pixel2 Composite::getCellSpacing() const{
	return cellSpacing;
}
pixel2 Composite::getCellPadding() const{
	return cellPadding;
}
void Composite::appendToTabChain(Region* region) {
	region->appendTo(tabChain);
}
void Composite::setSlimScroll(bool s){
	slim=s;
}
void Composite::removeListener() const {
	Application::removeListener(this);
	for (RegionPtr child : children) {
		child->removeListener();
	}
}
void Region::pack(const pixel2& pos, const pixel2& dims, const double2& dpmm,
		double pixelRatio, bool clamp) {

	pixel2 computedPos = position.toPixels(dims, dpmm, pixelRatio);
//pixel2 xy = pos + dragOffset + computedPos;
	pixel2 xy = pos + computedPos;
	pixel2 d = dimensions.toPixels(dims, dpmm, pixelRatio);
	if (aspectRatio < 0) {
		aspectRatio = d.x / std::max((float) d.y, 0.0f);
	}
	switch (aspectRule) {
	case AspectRule::FixedWidth:
		bounds.dimensions = pixel2(d.x, d.x / (float) aspectRatio);
		break;
	case AspectRule::FixedHeight:
		bounds.dimensions = pixel2(d.y * (float) aspectRatio, d.y);
		break;
	case AspectRule::Unspecified:
	default:
		bounds.dimensions = d;
	}
	bounds.position = xy;

	if (clamp && parent != nullptr) {
		bounds.dimensions = aly::clamp(bounds.dimensions, pixel2(0, 0),
				parent->bounds.dimensions);
	}

	switch (origin) {
	case Origin::TopLeft:
		bounds.position = xy;
		break;
	case Origin::BottomRight:
		bounds.position = xy - bounds.dimensions;
		break;
	case Origin::MiddleCenter:
		bounds.position = xy - bounds.dimensions / (pixel) 2;
		break;
	case Origin::TopRight:
		bounds.position = xy - pixel2(bounds.dimensions.x, 0);
		break;
	case Origin::BottomLeft:
		bounds.position = xy - pixel2(0, bounds.dimensions.y);
		break;
	case Origin::MiddleLeft:
		bounds.position = xy - pixel2(0, bounds.dimensions.y / (pixel) 2);
		break;
	case Origin::MiddleRight:
		bounds.position = xy
				- pixel2(bounds.dimensions.x, bounds.dimensions.y / (pixel) 2);
		break;
	case Origin::TopCenter:
		bounds.position = xy - pixel2(bounds.dimensions.x / (pixel) 2, 0);
		break;
	case Origin::BottomCenter:
		bounds.position = xy
				- pixel2(bounds.dimensions.x / (pixel) 2, bounds.dimensions.y);
		break;
	}
	if (clamp && parent != nullptr && !parent->isScrollEnabled()) {
		pixel2 ppos = parent->getBoundsPosition();
		pixel2 dims = parent->bounds.dimensions;
		bounds.position = aly::clamp(bounds.position, ppos,
				ppos + dims - bounds.dimensions);
	}
	extents.position = pixel2(0.0f);
	extents.dimensions = bounds.dimensions;
}

bool Region::isVisible() const {
	if (!visible)
		return false;
	if (parent != nullptr) {
		return parent->isVisible();
	}
	return true;
}
void Region::setVisible(bool vis) {
	visible = vis;
	AlloyContext* context = AlloyDefaultContext().get();
	if (context != nullptr) {
		context->requestUpdateCursor();
	}
}
bool Region::onEventHandler(AlloyContext* context, const InputEvent& event) {

	if (isVisible() && onEvent)
		return onEvent(context, event);
	else
		return false;
}
Region* Region::locate(const pixel2& cursor) {
	if (isVisible() && getCursorBounds().contains(cursor)) {
		return this;
	} else {
		return nullptr;
	}
}
void Region::pack(AlloyContext* context) {
	if (parent == nullptr) {
		pack(pixel2(0, 0), pixel2(context->screenDimensions()), context->dpmm,
				context->pixelRatio);
	} else {
		box2px bounds = parent->getBounds(false);
		pack(bounds.position, bounds.dimensions, context->dpmm,
				context->pixelRatio);
	}
}
void Region::pack() {
	pack(AlloyApplicationContext().get());
}
void Region::draw(AlloyContext* context) {
	NVGcontext* nvg = context->nvgContext;
	box2px bounds = getBounds();
	pixel lineWidth = borderWidth.toPixels(bounds.dimensions.y, context->dpmm.y,
			context->pixelRatio);
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
void Region::drawBoundsLabel(AlloyContext* context, const std::string& name,
		int font) {
	bool ontop = context->isOnTop(this);
	box2px bounds = getCursorBounds();
	if ((bounds.dimensions.x <= 4 && bounds.dimensions.y <= 4)
			|| bounds.dimensions.x * bounds.dimensions.y == 0) {
		return;
	}
	NVGcontext* nvg = context->nvgContext;
	pushScissor(nvg, bounds.position.x, bounds.position.y, bounds.dimensions.x,
			bounds.dimensions.y);
	bool hover = context->isMouseOver(this);
	bool down = context->isMouseDown(this) && context->isLeftMouseButtonDown();
	bool focus = context->isObjectFocused(this);
	Color c;
	if (isVisible()) {
		if (focus) {
			if (hover) {
				c = DEBUG_OBJECT_FOCUS_HOVER_COLOR;
			} else {
				c = DEBUG_OBJECT_FOCUS_DOWN_COLOR;
			}
		} else {
			if (down) {
				if (ontop) {
					c = DEBUG_ON_TOP_DOWN_COLOR;
				} else {
					c = DEBUG_DOWN_COLOR;
				}
			} else if (hover) {
				if (ontop) {
					c = DEBUG_ON_TOP_HOVER_COLOR;
				} else {
					c = DEBUG_HOVER_COLOR;
				}
			} else if (ontop) {
				c = DEBUG_ON_TOP_COLOR;
			} else {
				c = DEBUG_STROKE_COLOR;
			}
		}
	} else {
		c = DEBUG_HIDDEN_COLOR;
	}

	nvgBeginPath(nvg);

	nvgLineJoin(nvg, NVG_ROUND);
	nvgRect(nvg, bounds.position.x + 1, bounds.position.y + 1,
			bounds.dimensions.x - 2, bounds.dimensions.y - 2);
	nvgStrokeColor(nvg, c);
	nvgStrokeWidth(nvg, 1.0f);
	nvgStroke(nvg);
	popScissor(nvg);
}
void Region::setDragOffset(const pixel2& cursor, const pixel2& delta) {
	box2px bounds = getBounds();
	pixel2 d = (bounds.position - dragOffset);
	if (!clampToParentBounds) {
		dragOffset = cursor - delta - d;
	} else {
		box2px pbounds = parent->getBounds();
		dragOffset = bounds.clamp(cursor - delta, pbounds) - d;
	}
}
void Region::clampDragOffset() {
	if (clampToParentBounds) {
		box2px pbounds = parent->getBounds();
		dragOffset = bounds.clamp(bounds.position + dragOffset, pbounds)
				- bounds.position;
	}
}

bool Region::acceptDragEvent(const pixel2& cursor) const {
	return true;
}
void Region::setDragEnabled(bool enabled) {
	dragButton = (enabled) ? GLFW_MOUSE_BUTTON_LEFT : -1;
	if (enabled)
		clampToParentBounds = true;
}
void Region::setDragButton(int button) {
	dragButton = button;
	if (dragButton != -1)
		clampToParentBounds = true;
}
void Region::setOrigin(const Origin& org) {
	origin = org;
}
bool Region::isDragEnabled() const {
	return (dragButton != -1);
}
pixel2 Region::getBoundsPosition(bool includeOffset) const {
	return getBounds(includeOffset).position;
}
pixel2 Region::getBoundsDimensions(bool includeOffset) const {
	return getBounds(includeOffset).dimensions;
}
pixel2 Region::getDragOffset() const {
	return dragOffset;
}
pixel Region::getBoundsPositionX(bool includeOffset) const {
	return getBounds(includeOffset).position.x;
}
pixel Region::getBoundsDimensionsX(bool includeOffset) const {
	return getBounds(includeOffset).dimensions.x;
}
pixel Region::getBoundsPositionY(bool includeOffset) const {
	return getBounds(includeOffset).position.y;
}
pixel Region::getBoundsDimensionsY(bool includeOffset) const {
	return getBounds(includeOffset).dimensions.y;
}
Origin Region::getOrigin() const {
	return origin;
}
AspectRule Region::getAspectRule() const {
	return aspectRule;
}
double Region::getAspectRatio() const {
	return aspectRatio;
}

void Region::setIgnoreCursorEvents(bool ignore) {
	ignoreCursorEvents = ignore;
}
void Region::setClampDragToParentBounds(bool clamp) {
	clampToParentBounds = clamp;
}
void Region::setDetached(bool enable) {
	detached = enable;
}
void Region::setDragOffset(const pixel2& offset) {
	dragOffset = offset;
}
int Region::getDragButton() const {
	return dragButton;
}
void Region::setRoundCorners(bool round) {
	this->roundCorners = round;
}
bool Region::hasParent(Region* region) const {
	return (parent != nullptr && (parent == region || parent->hasParent(region)));
}
bool Region::isDetached() const {
	return (parent != nullptr && parent->isDetached()) || detached;
}
bool Region::isScrollEnabled() const {
	return false;
}
std::string Region::getName() const {
	return name;
}
pixel2 Region::getDrawOffset() const {
	if (parent != nullptr) {
		return parent->getDrawOffset();
	} else {
		return pixel2(0, 0);
	}
}

void Region::setAspectRule(const AspectRule& aspect) {
	aspectRule = aspect;
}
void Region::setAspectRatio(double val) {
	aspectRatio = val;
}
void Region::setBounds(const AUnit2D& pt, const AUnit2D& dim) {
	position = pt;
	dimensions = dim;
}
void Region::setBounds(const pixel2& pt, const pixel2& dim) {
	bounds.position = pt;
	bounds.dimensions = dim;
}
box2px Region::setBounds(const box2px& bbox) {
	box2px old=bounds;
	bounds = bbox;
	return old;
}
bool Region::addDragOffset(const pixel2& delta) {
	pixel2 oldOffset = dragOffset;
	box2px bounds = getBounds();
	pixel2 d = (bounds.position - dragOffset);
	box2px pbounds = parent->getBounds();
	if (!clampToParentBounds) {
		dragOffset = bounds.position + delta - d;
	} else {
		dragOffset = bounds.clamp(bounds.position + delta, pbounds) - d;
	}
	return (oldOffset != dragOffset);
}
Region::Region(const std::string& name) :
		position(CoordPX(0, 0)), dimensions(CoordPercent(1, 1)), name(name) {
}
Region::Region(const std::string& name, const AUnit2D& pos, const AUnit2D& dims) :
		position(pos), dimensions(dims), name(name) {
}
Region::~Region() {
	Application::clearEvents(this);
}
void Region::drawDebug(AlloyContext* context) {
	drawBoundsLabel(context, name, context->getFontHandle(FontType::Bold));
}
box2px Region::getBounds(bool includeOffset) const {
	box2px box = bounds;
	if (parent != nullptr && includeOffset) {
		box.position += parent->getDrawOffset();
	}
	box.position += dragOffset;
	return box;
}
box2px Region::getExtents() const {
	return extents;
}
void Region::setExtents(const box2px& bbox){
	extents=bbox;
}
box2px Region::getCursorBounds(bool includeOffset) const {
	box2px box = (isDetached() ? getBounds(includeOffset) : bounds);
	box.position += dragOffset;
	if (parent != nullptr && (!isDetached() && includeOffset)) {
		box.position += parent->getDrawOffset();
		if (AlloyApplicationContext()->getOnTopRegion() != this) {
			box.intersect(parent->getCursorBounds());
		}
	}

	return box;
}

void Draw::draw(AlloyContext* context) {
	if (onDraw) {
		onDraw(context, getBounds());
	}
}
}
