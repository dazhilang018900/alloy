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
class Region;
class BorderComposite;
struct TabChain{
	TabChain* parent;
	std::list<Region*> regions;
	TabChain();
	void add(Region* region);
	void remove(Region* region);
	void focusNext(Region* region);
	void focusPrevious(Region* region);
	void clear();
	virtual ~TabChain();
};
class Region: public EventHandler {
private:
	pixel2 dragOffset = pixel2(0, 0);
protected:
	friend class TabChain;
	box2px bounds;
	box2px extents;
	TabChain tabChain;
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
	virtual void appendTo(TabChain& chain);
public:
	AUnit2D position = CoordPercent(0.0f, 0.0f);
	AUnit2D dimensions = CoordPercent(1.0f, 1.0f);
	friend class Composite;
	friend class BorderComposite;
	const std::string name;
	void focusNext();
	void focusPrevious();
	void printFocus();
	bool hasRoundedCorners() const;
	bool isClampedToBounds() const;
	std::function<bool(AlloyContext*, const InputEvent& event)> onEvent;
	virtual Region* locate(const pixel2& cursor);
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)override;

	virtual bool isCursorFocused() const;
	virtual void setFocus(bool f);
	virtual bool isObjectFocused() const;
	void clampDragOffset();
	void setIgnoreCursorEvents(bool ignore);
	void setClampDragToParentBounds(bool clamp);
	void setDetached(bool enable);
	void setDragOffset(const pixel2& offset);
	int getDragButton() const;
	void setRoundCorners(bool round);
	bool hasParent(Region* region) const;
	bool isDetached() const;
	virtual inline bool isScrollEnabled() const;
	virtual std::string getName() const override;
	virtual pixel2 getDrawOffset() const;
	void setAspectRule(const AspectRule& aspect);
	void setAspectRatio(double val);
	void setBounds(const AUnit2D& pt, const AUnit2D& dim);
	void setBounds(const pixel2& pt, const pixel2& dim);
	void setBounds(const box2px& bbox);
	AColor backgroundColor = MakeColor(COLOR_NONE);
	AColor borderColor = MakeColor(COLOR_NONE);
	AUnit1D borderWidth = UnitPX(2);
	std::function<void()> onPack;
	std::function<void()> onRemoveFromOnTop;
	std::function<bool(AlloyContext* context, const InputEvent& event)> onMouseDown;
	std::function<bool(AlloyContext* context, const InputEvent& event)> onMouseUp;
	std::function<bool(AlloyContext* context, const InputEvent& event)> onMouseOver;
	std::function<bool(AlloyContext* context, const InputEvent& event)> onScrollWheel;
	std::function<bool(AlloyContext* context, const InputEvent& event)> onMouseDrag;
	void setDragOffset(const pixel2& cursor, const pixel2& delta);
	bool addDragOffset(const pixel2& delta);
	virtual bool acceptDragEvent(const pixel2& cursor) const;
	virtual void setDragEnabled(bool enabled);
	virtual void setDragButton(int button);
	void setOrigin(const Origin& org);
	virtual bool isDragEnabled() const;
	pixel2 getBoundsPosition(bool includeOffset = true) const;
	pixel2 getBoundsDimensions(bool includeOffset = true) const;
	pixel2 getDragOffset() const;
	pixel getBoundsPositionX(bool includeOffset = true) const;
	pixel getBoundsDimensionsX(bool includeOffset = true) const;
	pixel getBoundsPositionY(bool includeOffset = true) const;
	pixel getBoundsDimensionsY(bool includeOffset = true) const;
	Origin getOrigin() const;
	AspectRule getAspectRule() const;
	double getAspectRatio() const;
	virtual box2px getBounds(bool includeOffset = true) const;
	virtual box2px getExtents() const;
	virtual box2px getCursorBounds(bool includeOffset = true) const;

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
template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const TabChain& tabs) {
	for(Region* region:tabs.regions){
		if(AlloyDefaultContext()->isObjectFocused(region)){
			if(region==tabs.regions.back()){
				ss<<"[**"<<region->getName()<<"**]";
			} else {
				ss<<"[**"<<region->getName()<<"**] -> ";
			}
		} else {
			if(region==tabs.regions.back()){
				ss<<"["<<region->getName()<<"]";
			} else {
				ss<<"["<<region->getName()<<"] -> ";
			}
		}
	}
	return ss;
}
typedef std::shared_ptr<Draw> DrawPtr;
typedef std::shared_ptr<Region> RegionPtr;
}
#endif /* SRC_UI_ALLOYREGION_H_ */
