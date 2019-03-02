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
#ifndef SRC_UI_ALLOYMENU_H_
#define SRC_UI_ALLOYMENU_H_
#include "ui/AlloyComposite.h"
namespace aly{
class MenuContainer;
struct MenuContainer {
	virtual void hide()=0;
	bool rightSide=true;
	virtual ~MenuContainer(){}
};
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
	MenuContainer* container = nullptr;
	FontStyle fontStyle = FontStyle::Normal;
	FontType fontType = FontType::Normal;
	AUnit1D fontSize = UnitPX(24.0f);
	AColor textColor = MakeColor(COLOR_WHITE);
	AColor textAltColor = MakeColor(COLOR_BLACK);
	std::function<void()> onSelect;

	void setContainer(MenuContainer* cont);
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
	int menuWidth=0;
	bool scrollingDown = false, scrollingUp = false;
	std::shared_ptr<TimerTask> downTimer, upTimer;
	std::shared_ptr<AwesomeGlyph> downArrow, upArrow;
	std::vector<std::shared_ptr<MenuItem>> options;
	bool fireEvent(int selectedIndex);
public:

	virtual void setVisible(bool visible) override;

	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event);
	virtual box2px getBounds(bool includeBounds = true) const override;
	void setMaxDisplayEntries(int mx);
	virtual bool isMenu() const override ;
	std::string getItem(int index);
	int getSelectedIndex() const;
	void setSelectionOffset(bool offset);
	std::shared_ptr<MenuItem> addItem(const std::string& selection,const std::string& hint="");
	void setSelectedIndex(int index);
	void draw(AlloyContext* context) override;

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
class MenuBar: public MenuItem,MenuContainer {
protected:
	std::list<std::shared_ptr<MenuHeader>> headers;
	std::shared_ptr<Composite> barRegion;
	virtual void setVisibleItem(const std::shared_ptr<MenuItem>& item,
			bool forceShow = false) override;
	bool active;
public:
	void addMenu(const std::shared_ptr<Menu>& menu);
	virtual void hide() override;

	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)override;
	MenuBar(const std::string& name, const AUnit2D& position,
			const AUnit2D& dimensions);
};

class MenuPopup:public Menu, MenuContainer{
public:
	MenuPopup(const std::string& name, float menuWidth = 200.0f,
			const std::vector<std::shared_ptr<MenuItem>>& options = std::vector<
					std::shared_ptr<MenuItem>>());
	virtual void hide() override;
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)override;
};
typedef std::shared_ptr<MenuItem> MenuItemPtr;
typedef std::shared_ptr<Menu> MenuPtr;
typedef std::shared_ptr<MenuHeader> MenuHeaderPtr;
typedef std::shared_ptr<MenuBar> MenuBarPtr;
typedef std::shared_ptr<MenuPopup> MenuPopupBar;
}
#endif /* SRC_UI_ALLOYMENU_H_ */
