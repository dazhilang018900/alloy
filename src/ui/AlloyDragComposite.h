/*
 * AlloyDragComposite.h
 *
 *  Created on: Feb 10, 2019
 *      Author: blake
 */

#ifndef SRC_UI_ALLOYDRAGCOMPOSITE_H_
#define SRC_UI_ALLOYDRAGCOMPOSITE_H_
#include "ui/AlloyComposite.h"
#include "ui/AlloyScrollPane.h"
namespace aly {
struct DragSlot{
	int index=-1;
	Region* region=nullptr;
	float tween=0.0f;
	pixel2 start;
	box2px bounds;
	void draw(aly::AlloyContext* context);
};
class DragComposite:public ScrollPane {
protected:
	std::vector<DragSlot> sourceSlots;
	std::vector<DragSlot> targetSlots;
	Timer timer;
	Region* focusRegion=nullptr;
	bool clamp;
public:
	virtual void draw(AlloyContext* context) override;
	virtual void pack(const pixel2& pos, const pixel2& dims, const double2& dpmm,double pixelRatio, bool clamp = false) override;
	DragComposite(const std::string& name, const AUnit2D& pos, const AUnit2D& dims,const Orientation& orient,bool clamp=true);
	virtual void add(const std::shared_ptr<Region>& region,bool appendToTab=false) override;
	virtual ~DragComposite(){}
};
typedef std::shared_ptr<DragComposite> DragCompositePtr;

class DragBinComposite:public ScrollPane {
protected:
public:
	DragBinComposite(const std::string& name, const AUnit2D& pos, const AUnit2D& dims,const Orientation& orient);
	DragCompositePtr addBin(const std::string& name,int size);
	DragCompositePtr getBin(int idx) const ;
	virtual ~DragBinComposite(){}
};
typedef std::shared_ptr<DragBinComposite> DragBinCompositePtr;

} /* namespace aly */

#endif /* SRC_UI_ALLOYDRAGCOMPOSITE_H_ */
