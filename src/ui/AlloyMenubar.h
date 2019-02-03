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
#ifndef SRC_UI_ALLOYMENUBAR_H_
#define SRC_UI_ALLOYMENUBAR_H_
#include "ui/AlloyComposite.h"
namespace aly{
class MenuBar;
class MenuItem: public Composite {
protected:

	std::mutex showLock;
	std::shared_ptr<MenuItem> currentSelected;
	std::shared_ptr<MenuItem> requestedSelected;
	std::shared_ptr<MenuItem> currentVisible;
	std::shared_ptr<TimerTask> showTimer;
	std::string hint;
	const int MENU_DISPLAY_DELAY = 250;
public:
	MenuItem* getSelectedItem();
	MenuBar* menuBar = nullptr;
	FontStyle fontStyle = FontStyle::Normal;
	FontType fontType = FontType::Normal;
	AUnit1D fontSize = UnitPX(24.0f);
	AColor textColor = MakeColor(COLOR_WHITE);
	AColor textAltColor = MakeColor(COLOR_BLACK);
	std::function<void()> onSelect;
	virtual bool isMenu() const {
		return false;
	}
	void setHint(const std::string& str){
		hint=str;
	}
	std::string getHint() const{
		return hint;
	}
	virtual void setVisible(bool visible) override;
	MenuItem(const std::string& name);
	MenuItem(const std::string& name, const AUnit2D& position,
			const AUnit2D& dimensions);

	virtual void setVisibleItem(const std::shared_ptr<MenuItem>& item,
			bool forceShow = false);
};

class Menu: public MenuItem {
protected:
	int selectedIndex = -1;
	std::shared_ptr<MenuItem> label;
	int maxDisplayEntries = 8;
	int selectionOffset = 0;
	bool scrollingDown = false, scrollingUp = false;
	std::shared_ptr<TimerTask> downTimer, upTimer;
	std::shared_ptr<AwesomeGlyph> downArrow, upArrow;
	std::vector<std::shared_ptr<MenuItem>> options;
	bool fireEvent(int selectedIndex);
public:
	virtual bool isMenu() const override {
		return true;
	}
	virtual void setVisible(bool visible) override;
	void setMaxDisplayEntries(int mx) {
		maxDisplayEntries = mx;
	}
	virtual box2px getBounds(bool includeBounds = true) const override;
	std::string getItem(int index) {
		return (selectedIndex >= 0) ? options[selectedIndex]->name : name;
	}
	int getSelectedIndex() const {
		return selectedIndex;
	}
	inline void setSelectionOffset(bool offset) {
		selectionOffset = offset;
	}
	void setSelectedIndex(int index);
	void draw(AlloyContext* context) override;
	std::shared_ptr<MenuItem> addItem(const std::string& selection,const std::string& hint="") {
		std::shared_ptr<MenuItem> item = std::shared_ptr<MenuItem>(
				new MenuItem(selection));
		item->setHint(hint);
		options.push_back(item);
		return item;
	}
	void addItem(const std::shared_ptr<MenuItem>& selection);
	virtual void clear() override;
	Menu(const std::string& name, float menuWidth = 200.0f,
			const std::vector<std::shared_ptr<MenuItem>>& options = std::vector<
					std::shared_ptr<MenuItem>>());
};
class MenuHeader: public MenuItem {
public:
	AColor backgroundAltColor;
	std::shared_ptr<Menu> menu;
	bool isMenuVisible() const {
		return menu->isVisible();
	}
	MenuHeader(const std::shared_ptr<Menu>& menu, const AUnit2D& position,
			const AUnit2D& dimensions);
	virtual void draw(AlloyContext* context) override;
	virtual inline ~MenuHeader() {
	}
};
class MenuBar: public MenuItem {
protected:
	std::list<std::shared_ptr<MenuHeader>> headers;
	std::shared_ptr<Composite> barRegion;
	virtual void setVisibleItem(const std::shared_ptr<MenuItem>& item,
			bool forceShow = false) override;
	bool active;
public:
	void addMenu(const std::shared_ptr<Menu>& menu);
	void hideMenus();
	MenuBar(const std::string& name, const AUnit2D& position,
			const AUnit2D& dimensions);
};
typedef std::shared_ptr<MenuItem> MenuItemPtr;
typedef std::shared_ptr<Menu> MenuPtr;
typedef std::shared_ptr<MenuHeader> MenuHeaderPtr;
typedef std::shared_ptr<MenuBar> MenuBarPtr;
}
#endif /* SRC_UI_ALLOYMENUBAR_H_ */
