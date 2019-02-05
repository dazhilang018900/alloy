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
#ifndef SRC_UI_ALLOYSELECTIONBOX_H_
#define SRC_UI_ALLOYSELECTIONBOX_H_
#include "ui/AlloyBorderComposite.h"
#include "ui/AlloyTextWidget.h"
#include "ui/AlloyButton.h"
namespace aly {
class SelectionBox: public Region {
protected:
	int selectedIndex = -1;
	std::string label;
	int maxDisplayEntries = 8;
	int selectionOffset = 0;
	bool scrollingDown = false, scrollingUp = false;
	std::shared_ptr<TimerTask> downTimer, upTimer;
	std::shared_ptr<AwesomeGlyph> downArrow, upArrow;

public:
	FontStyle fontStyle = FontStyle::Normal;
	FontType fontType = FontType::Normal;
	AColor textColor = MakeColor(COLOR_WHITE);
	AColor textAltColor = MakeColor(COLOR_WHITE);
	std::function<bool(SelectionBox*)> onSelect;
	std::vector<std::string> options;
	void setMaxDisplayEntries(int mx) {
		maxDisplayEntries = mx;
	}
	virtual box2px getBounds(bool includeBounds = true) const override;
	std::string getSelection(int index);
	int getSelectedIndex() const {
		return selectedIndex;
	}

	void setSelectionOffset(int offset);
	void setSelectedIndex(int index);
	void draw(AlloyContext* context) override;
	void addSelection(const std::string& selection) {
		options.push_back(selection);
	}
	size_t getSelectionSize() const {
		return options.size();
	}
	void clearSelections() {
		selectedIndex = -1;
		selectionOffset = 0;
		options.clear();
	}
	void eraseSelection(int index) {
		options.erase(options.begin() + index);
	}
	SelectionBox(const std::string& name,
			const std::vector<std::string>& options =
					std::vector<std::string>());
	SelectionBox(const std::string& name, const AUnit2D& pos,
			const AUnit2D& dims, const std::vector<std::string>& options =
					std::vector<std::string>());
};

class Selection: public Composite {
private:
	TextLabelPtr selectionLabel;
	TextLabelPtr arrowLabel;
	std::shared_ptr<SelectionBox> selectionBox;
	int selectedIndex;
	void hide(AlloyContext* context);
	void show(AlloyContext* context);
	bool handleMouseClick(AlloyContext* context, const InputEvent& event);
public:
	void setTextColor(const AColor& c);
	std::function<void(int)> onSelect;
	//bool onEventHandler(AlloyContext* context, const InputEvent& event)override;
	void clearSelections() {
		selectionBox->clearSelections();
	}
	inline int getSelectedIndex() const {
		return selectedIndex;
	}
	void setMaxDisplayEntries(int mx) {
		selectionBox->setMaxDisplayEntries(mx);
	}
	std::string getValue() {
		return selectionBox->getSelection(selectedIndex);
	}
	std::string getSelection() {
		return selectionBox->getSelection(selectedIndex);
	}

	std::string getSelection(int index) {
		return selectionBox->getSelection(index);
	}
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)
			override;
	void setValue(int selection);
	void addSelection(const std::string& selection) ;
	size_t getSelectionSize() const ;
	void setSelectedIndex(int selection);
	virtual void draw(AlloyContext* context) override;
	Selection(const std::string& label, const AUnit2D& position,
			const AUnit2D& dimensions, const std::vector<std::string>& options =
					std::vector<std::string>());
};
class SearchBox : public BorderComposite {
private:
	std::shared_ptr<TextField> searchField;
	std::shared_ptr<IconButton> searchIcon;
protected:
	virtual void appendTo(TabChain& chain) override;
public:
	void setTextColor(const AColor& c);
	std::function<void(const std::string& query)> onChange;
	SearchBox(const std::string& name, const AUnit2D& pos,const AUnit2D& dims);
	void setValue(const std::string& file);
	std::string getValue() {
		return searchField->getValue();
	}
};
class FilterBox : public BorderComposite {
private:
	std::shared_ptr<TextField> filterField;
	std::shared_ptr<IconButton> filterIcon;
public:
	void setTextColor(const AColor& c);
	std::function<void(const std::string& query)> onChange;
	FilterBox(const std::string& name, const AUnit2D& pos,const AUnit2D& dims);
	void setValue(const std::string& file);
	std::string getValue() {
		return filterField->getValue();
	}
};
typedef std::shared_ptr<SearchBox> SearchBoxPtr;
typedef std::shared_ptr<FilterBox> FilterBoxPtr;
typedef std::shared_ptr<SelectionBox> SelectionBoxPtr;
typedef std::shared_ptr<Selection> SelectionPtr;
}

#endif /* SRC_UI_ALLOYSELECTIONBOX_H_ */
