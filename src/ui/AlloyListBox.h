/*
 * Copyright(C) 2018, Blake C. Lucas, Ph.D. (img.science@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef SRC_UI_ALLOYLISTBOX_H_
#define SRC_UI_ALLOYLISTBOX_H_
#include "ui/AlloyComposite.h"
#include "ui/AlloyTextWidget.h"
#include "ui/AlloyNumberWidget.h"
#include "ui/AlloyButton.h"
#include "ui/AlloySlider.h"
#include "ui/AlloyBorderComposite.h"
namespace aly{
class ListBox;
class ListEntry: public Composite {
protected:
	std::string iconCodeString;
	std::string label;
	bool selected;
	ListBox* dialog;
	float entryHeight;
	AUnit1D fontSize;
public:
	friend class ListBox;
	void setSelected(bool selected);
	bool isSelected();
	virtual void setLabel(const std::string& label);
	void setIcon(int icon);
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)
			override;
	ListEntry(ListBox* listBox, const std::string& name, float entryHeight);
	virtual void draw(AlloyContext* context) override;
};
class NumberEntry : public aly::ListEntry {
protected:
	ModifiableNumberPtr valueRegion;
public:
        NumberEntry(aly::ListBox* listBox,const NumberType&  number, float entryHeight=30.0f);
        void setValue(const Number& num);
        Number getValue() const {
            return valueRegion->getValue();
        }
        ModifiableNumberPtr getRegion() const {
            return valueRegion;
        }
        virtual void draw(aly::AlloyContext* context) override;
};
class ListBox: public Composite {
protected:
	bool enableMultiSelection;
	bool enableDelete;
	box2px dragBox;
	int startItem;
	int endItem;
	float downOffsetPosition;
	std::shared_ptr<TimerTask> downTimer, upTimer;
	bool scrollingDown;
	bool scrollingUp;
	bool dirty;
	std::vector<std::shared_ptr<ListEntry>> listEntries;
	std::list<ListEntry*> lastSelected;

	void addToActiveList(ListEntry* entry) {
		lastSelected.push_back(entry);
	}
	void clearActiveList() {
		lastSelected.clear();
	}
public:
	void update();
	bool removeSelected();
	bool removeAll();
	bool addVerticalScrollPosition(int c);
	virtual void scrollToTop() override;
	virtual void scrollToBottom() override;
	virtual void pack(const pixel2& pos, const pixel2& dims, const double2& dpmm,double pixelRatio, bool clamp) override;
	box2px getDragBox() const {
		return dragBox;
	}
	std::vector<std::shared_ptr<ListEntry>>& getEntries() {
		return listEntries;
	}
	const std::vector<std::shared_ptr<ListEntry>>& getEntries() const {
		return listEntries;
	}
	void addEntry(const std::shared_ptr<ListEntry>& entry);
	void setEnableDelete(bool val);
	ListEntry* getLastSelected();
	void setEnableMultiSelection(bool enable);
	void clearEntries();
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& e) override;

	bool isDraggingOver(ListEntry* entry);
	ListBox(const std::string& name, const AUnit2D& pos, const AUnit2D& dims);
	virtual void draw(AlloyContext* context) override;

	bool onMouseDown(ListEntry* entry, AlloyContext* context,
			const InputEvent& e);
	std::function<void(ListEntry*, const InputEvent&)> onSelect;
	std::function<void(const std::vector<std::shared_ptr<ListEntry>>& deleteList)> onDeleteEntry;
};

class NumberListBox : public aly::Composite {
protected:
        std::shared_ptr<aly::ListBox> valueRegion;
        std::shared_ptr<aly::IconButton> addButton;
        std::shared_ptr<aly::IconButton> upButton;
        std::shared_ptr<aly::IconButton> downButton;
        std::shared_ptr<aly::IconButton> eraseButton;
        float entryHeight;
        std::vector<Number> values;
        void fireEvent();
public:
        std::function<void(const std::vector<Number>& numbers)> onChange;
        void update();
        void clearEntries();
        void addNumbers(const std::vector<Number>& numbers);
        NumberListBox(const std::string& name, const aly::AUnit2D& pos, const aly::AUnit2D& dims, const NumberType& type, float entryHeight = 30.0f);
};


typedef std::shared_ptr<ListBox> ListBoxPtr;

typedef std::shared_ptr<ListEntry> ListEntryPtr;
typedef std::shared_ptr<NumberListBox>  NumberListBoxPtr;
typedef std::shared_ptr<NumberEntry> NumberEntryPtr;
}

#endif /* SRC_UI_ALLOYLISTBOX_H_ */
