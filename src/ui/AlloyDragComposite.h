/*
 * AlloyDragComposite.h
 *
 *  Created on: Feb 10, 2019
 *      Author: blake
 */

#ifndef SRC_UI_ALLOYDRAGCOMPOSITE_H_
#define SRC_UI_ALLOYDRAGCOMPOSITE_H_
#include "AlloyCarouselComposite.h"
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
class DragComposite:public CarouselComposite {
protected:
	std::vector<DragSlot> sourceSlots;
	std::vector<DragSlot> targetSlots;
	Timer timer;
	Region* focusRegion=nullptr;
	Region* dragRegion=nullptr;
	bool dragging=false;
	float slotOffset=0.0f;
	bool clamp;
public:
	std::function<void(const std::shared_ptr<Region>& region)> onDrop;
	std::function<void(Region* region)> onDragOver;
	const std::vector<DragSlot>& getSlots() const;
	void setEmptySlot(const box2px& box);
	virtual void draw(AlloyContext* context) override;
	virtual void pack(const pixel2& pos, const pixel2& dims, const double2& dpmm,double pixelRatio, bool clamp = false) override;
	DragComposite(const std::string& name, const AUnit2D& pos, const AUnit2D& dims,const Orientation& orient,bool clamp=true);
	virtual void add(const std::shared_ptr<Region>& region,bool appendToTab=false) override;
	virtual void insert(size_t idx,const std::shared_ptr<Region>& region,bool appendToTab=false) override;
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& e) override;
	virtual ~DragComposite(){}
};
typedef std::shared_ptr<DragComposite> DragCompositePtr;


class DragBinComposite:public DragComposite {
public:
	std::function<void(DragComposite* bin,Region* r)> onAddItem;
	std::function<void(DragComposite* bin,Region* r)> onRemoveItem;
	std::function<void(DragComposite* bin,Region* r)> onMoveItem;
	std::function<void(DragComposite* bin)> onAddBin;
	std::function<void(DragComposite* bin)> onRemoveBin;
	std::function<void(DragComposite* bin)> onMoveBin;
	void handleDrop(const std::shared_ptr<Region>& region);
	void handleDragOver(Region* region);
	DragBinComposite(const std::string& name, const AUnit2D& pos, const AUnit2D& dims,const Orientation& orient);
	DragCompositePtr addBin(const std::string& name,int size,Composite* after=nullptr);
	DragCompositePtr getBin(int idx) const ;
	void removeBin(DragComposite* region);
	void removeItem(Region* region);
	size_t size() const;
	virtual ~DragBinComposite(){}
};

typedef std::shared_ptr<DragBinComposite> DragBinCompositePtr;
class DragBinTab:public Composite{
protected:
	Orientation orientation;
	aly::IconButtonPtr addButton;
	aly::IconButtonPtr delButton;
public:
	static float tabSize;
	friend class DragBinComposite;
	DragBinTab(const std::string& name, const AUnit2D& pos, const AUnit2D& dims,const Orientation& orient);
	virtual void draw(AlloyContext* context) override;
};
typedef std::shared_ptr<DragBinTab> DragBinTabPtr;

} /* namespace aly */

#endif /* SRC_UI_ALLOYDRAGCOMPOSITE_H_ */
