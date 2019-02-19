/*
 * AlloyDragComposite.h
 *
 *  Created on: Feb 10, 2019
 *      Author: blake
 */

#ifndef SRC_UI_ALLOYDRAGCOMPOSITE_H_
#define SRC_UI_ALLOYDRAGCOMPOSITE_H_
#include "ui/AlloyComposite.h"
namespace aly {
struct DragSlot{
	int index=-1;
	Region* region=nullptr;
	float tween=0.0f;
	pixel2 start;
	box2px bounds;
	void draw(aly::AlloyContext* context);
};
class DragComposite:public Composite {
protected:
	std::vector<DragSlot> sourceSlots;
	std::vector<DragSlot> targetSlots;
	Timer timer;
	Region* focusRegion=nullptr;
public:
	virtual void draw(AlloyContext* context) override;
	virtual void pack(const pixel2& pos, const pixel2& dims, const double2& dpmm,double pixelRatio, bool clamp = false) override;
	DragComposite(const std::string& name, const AUnit2D& pos, const AUnit2D& dims,const Orientation& orient);
	virtual void add(const std::shared_ptr<Region>& region,bool appendToTab=false) override;
	virtual ~DragComposite(){}
};
typedef std::shared_ptr<DragComposite> DragCompositePtr;
} /* namespace aly */

#endif /* SRC_UI_ALLOYDRAGCOMPOSITE_H_ */
