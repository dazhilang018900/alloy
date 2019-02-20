/*
 * AlloyComposite.h
 *
 *  Created on: Sep 13, 2018
 *      Author: blake
 */

#ifndef SRC_UI_ALLOYCOMPOSITE_H_
#define SRC_UI_ALLOYCOMPOSITE_H_
#include "ui/AlloyRegion.h"
namespace aly{
class ScrollTrack;
class ScrollHandle;
class Composite: public Region {
protected:
	Orientation orientation = Orientation::Unspecified;
	bool scrollEnabled = false;
	bool alwaysShowVerticalScrollBar = false;
	bool alwaysShowHorizontalScrollBar = false;
	float horizontalScrollExtent = 0;
	pixel2 scrollPosition = pixel2(0, 0);
	std::shared_ptr<ScrollTrack> verticalScrollTrack, horizontalScrollTrack;
	std::shared_ptr<ScrollHandle> verticalScrollHandle, horizontalScrollHandle;
	std::vector<std::shared_ptr<Region>> children;
	typedef std::shared_ptr<Region> ValueType;
	pixel2 cellPadding = pixel2(0, 0);
	pixel2 cellSpacing = pixel2(0, 0);
	void updateExtents();
public:
	void appendToTabChain(Region* region);
	virtual void removeListener() const override;
	void erase(const std::shared_ptr<Region>& node);
	void erase(Region* node);
	bool isVerticalScrollVisible() const;
	bool isVerticalScrollHandleVisible() const;

	Orientation getOrientation() const ;
	bool isHorizontalScrollVisible() const;
	bool isHorizontalScrollHandleVisible() const;

	void setCellPadding(const pixel2& pix);
	void setCellSpacing(const pixel2& pix);
	void setAlwaysShowVerticalScrollBar(bool show);
	void setAlwaysShowHorizontalScrollBar(bool show);
	void resetScrollPosition();
	static const float scrollBarSize;
	typedef std::vector<ValueType>::iterator iterator;
	typedef std::vector<ValueType>::const_iterator const_iterator;
	std::function<void(const aly::pixel2& scroll)> onScroll;
	virtual void clear();
	std::vector<std::shared_ptr<Region>>& getChildren() {
		return children;
	}
	iterator begin() {
		return children.begin();
	}
	iterator end() {
		return children.end();
	}
	const_iterator cbegin() const {
		return children.cbegin();
	}
	const_iterator cend() const {
		return children.cend();
	}
	bool addVerticalScrollPosition(float pix);
	virtual void scrollToBottom();
	virtual void scrollToTop();
	virtual void scrollToLeft();
	virtual void scrollToRight();
	bool addHorizontalScrollPosition(float pix);
	void putLast(const std::shared_ptr<Region>& region);
	void putFirst(const std::shared_ptr<Region>& region);
	void putLast(Region* region);
	void putFirst(Region* region);
	Composite(const std::string& name = MakeString() << "c" << std::setw(8)<< std::setfill('0') << (REGION_COUNTER++));
	Composite(const std::string& name, const AUnit2D& pos, const AUnit2D& dims);
	virtual Region* locate(const pixel2& cursor) override;
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)override;
	void setOrientation(const Orientation& orient, pixel2 cellSpacing =pixel2(5, 5), pixel2 cellPadding = pixel2(0, 0));
	virtual bool isScrollEnabled() const override;
	void setScrollEnabled(bool enabled);

	virtual pixel2 getDrawOffset() const override;
	virtual void draw(AlloyContext* context) override;
	virtual void drawDebug(AlloyContext* context) override;
	virtual void updateCursor(CursorLocator* cursorLocator) override;
	virtual void pack(const pixel2& pos, const pixel2& dims, const double2& dpmm,
			double pixelRatio, bool clamp = false) override;
	virtual void add(const std::shared_ptr<Region>& region,bool appendToTab=false);
	virtual void insertAtFront(const std::shared_ptr<Region>& region);
	virtual void pack() override;
	virtual void pack(AlloyContext* context) override;
	void draw();
	virtual ~Composite();
};
std::shared_ptr<Composite> MakeComposite(const std::string& name,
		const AUnit2D& position, const AUnit2D& dimensions,
		const Color& bgColor = COLOR_NONE, const Color& lineColor = COLOR_NONE,
		const AUnit1D& lineWidth = UnitPX(2.0f),
		const Orientation& orientation = Orientation::Unspecified);
template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const Composite& region) {
	ss << "Composite: " << region.name << std::endl;
	ss << "\tOrigin: " << region.getOrigin() << std::endl;
	ss << "\tOrientation: " << region.getOrientation() << std::endl;
	ss << "\tRelative Position: " << region.position << std::endl;
	ss << "\tRelative Dimensions: " << region.dimensions << std::endl;
	ss << "\tBackground Color: " << region.backgroundColor << std::endl;
	ss << "\tBounds: " << region.getBounds() << std::endl;
	return ss;
}
typedef std::shared_ptr<Composite> CompositePtr;
}
#endif /* SRC_UI_ALLOYCOMPOSITE_H_ */
