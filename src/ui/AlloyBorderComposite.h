/*
 * AlloyBorderComposite.h
 *
 *  Created on: Sep 13, 2018
 *      Author: blake
 */

#ifndef SRC_UI_ALLOYBORDERCOMPOSITE_H_
#define SRC_UI_ALLOYBORDERCOMPOSITE_H_

#include "AlloyComposite.h"
namespace aly{
class BorderComposite: public Region {
protected:
	std::array<std::shared_ptr<Region>, 5> children;
	std::shared_ptr<Region>& northRegion;
	std::shared_ptr<Region>& southRegion;
	std::shared_ptr<Region>& eastRegion;
	std::shared_ptr<Region>& westRegion;
	AUnit1D northFraction, southFraction, eastFraction, westFraction;
	std::shared_ptr<Region>& centerRegion;
	bool resizing;
	WindowPosition winPos;
	bool resizeable;
	pixel2 cellPadding;
	box2px windowInitialBounds;
	box2px currentBounds;
	pixel2 cursorDownPosition;
public:
	bool isResizeable() const {
		return resizeable;
	}
	bool isResizing() const {
		return resizing;
	}
	void setCellPadding(const pixel2& padding){
		cellPadding=padding;
	}
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)
			override;
	BorderComposite(const std::string& name, const AUnit2D& pos,
			const AUnit2D& dims, bool resizeable = false);
	virtual Region* locate(const pixel2& cursor) override;
	virtual void draw(AlloyContext* context) override;
	virtual void drawDebug(AlloyContext* context) override;
	virtual void updateCursor(CursorLocator* cursorLocator) override;
	void pack(const pixel2& pos, const pixel2& dims, const double2& dpmm,
			double pixelRatio, bool clamp = false) override;
	void setNorth(const std::shared_ptr<Region>& region,
			const AUnit1D& fraction);
	void setSouth(const std::shared_ptr<Region>& region,
			const AUnit1D& fraction);
	void setEast(const std::shared_ptr<Region>& region,
			const AUnit1D& fraction);
	void setWest(const std::shared_ptr<Region>& region,
			const AUnit1D& fraction);
	inline void setNorth(const std::shared_ptr<Region>& region,
			float fraction) {
		setNorth(region, UnitPercent(fraction));
	}
	inline void setSouth(const std::shared_ptr<Region>& region,
			float fraction) {
		setSouth(region, UnitPercent(fraction));
	}
	inline void setEast(const std::shared_ptr<Region>& region, float fraction) {
		setEast(region, UnitPercent(fraction));
	}
	inline void setWest(const std::shared_ptr<Region>& region, float fraction) {
		setWest(region, UnitPercent(fraction));
	}

	void setCenter(const std::shared_ptr<Region>& region);
	virtual void pack() override {
		Region::pack();
	}
	virtual void pack(AlloyContext* context) override {
		Region::pack(context);
	}
	void draw();
};

typedef std::shared_ptr<BorderComposite> BorderCompositePtr;
}
#endif /* SRC_UI_ALLOYBORDERCOMPOSITE_H_ */
