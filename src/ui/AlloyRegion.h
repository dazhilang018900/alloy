/*
 * AlloyRegion.h
 *
 *  Created on: Sep 13, 2018
 *      Author: blake
 */

#ifndef SRC_UI_ALLOYREGION_H_
#define SRC_UI_ALLOYREGION_H_
#include "math/AlloyNumber.h"
#include "ui/AlloyContext.h"
#include "ui/AlloyWorker.h"
#include "ui/AlloyEnum.h"
#include  "graphics/GLTexture.h"
#include "math/AlloyVecMath.h"
#include "common/AlloyUnits.h"
#include "common/nanovg.h"
#include <iostream>
#include <memory>
#include <list>
#include <array>
#include <vector>

namespace aly {
bool SANITY_CHECK_UI();

class Composite;
class BorderComposite;
class Region: public EventHandler {
private:
	pixel2 dragOffset = pixel2(0, 0);
protected:
	box2px bounds;
	box2px extents;
	void drawBoundsLabel(AlloyContext* context, const std::string& name,
			int font);
	Region* mouseOverRegion = nullptr;
	Region* mouseDownRegion = nullptr;
	static uint64_t REGION_COUNTER;
	bool visible = true;
	bool ignoreCursorEvents = false;
	Origin origin = Origin::TopLeft;
	AspectRule aspectRule = AspectRule::Unspecified;
	double aspectRatio = -1.0; //Less than zero indicates undetermined. Will be computed at next pack() event.
	int dragButton = -1;
	bool roundCorners = false;
	bool detached = false;
	bool clampToParentBounds = false;
public:
	AUnit2D position = CoordPercent(0.0f, 0.0f);
	AUnit2D dimensions = CoordPercent(1.0f, 1.0f);
	friend class Composite;
	friend class BorderComposite;
	const std::string name;
	void setIgnoreCursorEvents(bool ignore) {
		ignoreCursorEvents = ignore;
	}
	void setClampDragToParentBounds(bool clamp) {
		clampToParentBounds = clamp;
	}
	void setDetached(bool enable) {
		detached = enable;
	}
	void setDragOffset(const pixel2& offset) {
		dragOffset = offset;
	}
	int getDragButton() const {
		return dragButton;
	}
	void clampDragOffset();
	inline void setRoundCorners(bool round) {
		this->roundCorners = round;
	}
	inline bool hasParent(Region* region) const {
		return (parent != nullptr
				&& (parent == region || parent->hasParent(region)));
	}
	inline bool isDetached() const {
		return (parent != nullptr && parent->isDetached()) || detached;
	}
	std::function<bool(AlloyContext*, const InputEvent& event)> onEvent;

	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)
			override;
	virtual inline bool isScrollEnabled() const {
		return false;
	}
	virtual std::string getName() const override {
		return name;
	}
	virtual inline pixel2 getDrawOffset() const {
		if (parent != nullptr) {
			return parent->getDrawOffset();
		} else {
			return pixel2(0, 0);
		}
	}

	virtual Region* locate(const pixel2& cursor);
	inline void setAspectRule(const AspectRule& aspect) {
		aspectRule = aspect;
	}
	inline void setAspectRatio(double val) {
		aspectRatio = val;
	}
	inline void setBounds(const AUnit2D& pt, const AUnit2D& dim) {
		position = pt;
		dimensions = dim;
	}
	inline void setBounds(const pixel2& pt, const pixel2& dim) {
		bounds.position = pt;
		bounds.dimensions = dim;
	}
	inline void setBounds(const box2px& bbox) {
		bounds = bbox;
	}
	AColor backgroundColor = MakeColor(COLOR_NONE);
	AColor borderColor = MakeColor(COLOR_NONE);
	AUnit1D borderWidth = UnitPX(2);
	std::function<void()> onPack;
	std::function<void()> onRemoveFromOnTop;
	std::function<bool(AlloyContext* context, const InputEvent& event)> onMouseDown;
	std::function<bool(AlloyContext* context, const InputEvent& event)> onMouseUp;
	std::function<bool(AlloyContext* context, const InputEvent& event)> onMouseOver;
	std::function<bool(AlloyContext* context, const InputEvent& event)> onScroll;
	std::function<bool(AlloyContext* context, const InputEvent& event)> onMouseDrag;
	void setDragOffset(const pixel2& cursor, const pixel2& delta);
	virtual bool acceptDragEvent(const pixel2& cursor) const {
		return true;
	}
	bool addDragOffset(const pixel2& delta);

	virtual void setDragEnabled(bool enabled) {
		dragButton = (enabled)?GLFW_MOUSE_BUTTON_LEFT:-1;
		if (enabled)
			clampToParentBounds = true;
	}
	virtual void setDragButton(int button) {
		dragButton = button;
		if (dragButton!=-1)
			clampToParentBounds = true;
	}
	inline void setOrigin(const Origin& org) {
		origin = org;
	}
	virtual bool isDragEnabled() const {
		return (dragButton!=-1);
	}
	virtual box2px getBounds(bool includeOffset = true) const;
	virtual box2px getExtents() const;
	virtual box2px getCursorBounds(bool includeOffset = true) const;
	pixel2 getBoundsPosition(bool includeOffset = true) const {
		return getBounds(includeOffset).position;
	}
	pixel2 getBoundsDimensions(bool includeOffset = true) const {
		return getBounds(includeOffset).dimensions;
	}
	pixel2 getDragOffset() const {
		return dragOffset;
	}
	pixel getBoundsPositionX(bool includeOffset = true) const {
		return getBounds(includeOffset).position.x;
	}
	pixel getBoundsDimensionsX(bool includeOffset = true) const {
		return getBounds(includeOffset).dimensions.x;
	}
	pixel getBoundsPositionY(bool includeOffset = true) const {
		return getBounds(includeOffset).position.y;
	}
	pixel getBoundsDimensionsY(bool includeOffset = true) const {
		return getBounds(includeOffset).dimensions.y;
	}
	Origin getOrigin() const {
		return origin;
	}
	AspectRule getAspectRule() const {
		return aspectRule;
	}
	double getAspectRatio() const {
		return aspectRatio;
	}
	virtual void setVisible(bool vis);
	Region* parent = nullptr;
	Region(
			const std::string& name = MakeString() << "r" << std::setw(8)
					<< std::setfill('0') << (REGION_COUNTER++));
	Region(const std::string& name, const AUnit2D& pos, const AUnit2D& dims);
	virtual void pack(const pixel2& pos, const pixel2& dims,
			const double2& dpmm, double pixelRatio, bool clamp = false);
	virtual void pack(AlloyContext* context);
	virtual void pack();
	virtual void draw(AlloyContext* context);
	virtual void updateCursor(CursorLocator* cursorLocator);
	virtual void drawDebug(AlloyContext* context);
	virtual void removeListener() const;
	bool isVisible() const;
	virtual ~Region();
};

class Draw: public Region {
public:
	std::function<void(AlloyContext* context, const box2px& bounds)> onDraw;
	Draw(const std::string& name, const AUnit2D& pos, const AUnit2D& dims,
			const std::function<
					void(AlloyContext* context, const box2px& bounds)>& func =
					nullptr) :
			Region(name, pos, dims), onDraw(func) {
	}
	virtual void draw(AlloyContext* context) override;
};
class ScrollHandle: public Region {
public:
	const Orientation orientation;
	ScrollHandle(const std::string& name, Orientation orient) :
			Region(name), orientation(orient) {
	}
	virtual void draw(AlloyContext* context) override;
};

typedef std::shared_ptr<Draw> DrawPtr;
typedef std::shared_ptr<Region> RegionPtr;
}
#endif /* SRC_UI_ALLOYREGION_H_ */
