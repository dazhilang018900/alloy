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
#include "ui/AlloyMenubar.h"
#include "ui/AlloyApplication.h"
#include "ui/AlloyDrawUtil.h"

namespace aly{
box2px Menu::getBounds(bool includeBounds) const {
	box2px bounds = Region::getBounds(includeBounds);
	AlloyContext* context = AlloyApplicationContext().get();
	int elements =
			(maxDisplayEntries > 0) ?
					std::min(maxDisplayEntries, (int) options.size()) :
					(int) options.size();
	float entryHeight = fontSize.toPixels(bounds.dimensions.y, context->dpmm.y,
			context->pixelRatio);
	float boxHeight = (elements) * entryHeight;
	float yOffset = std::min(bounds.position.y + boxHeight,
			(float) context->getScreenHeight()) - boxHeight;
	box2px bbox;
	bbox.position = pixel2(bounds.position.x, yOffset);
	bbox.dimensions = pixel2(bounds.dimensions.x, boxHeight);
	return bbox;
}
void Menu::draw(AlloyContext* context) {
	context->setDragObject(this);
	NVGcontext* nvg = context->nvgContext;
	box2px bounds = getBounds();
	box2px sbounds = bounds;
	sbounds.position.x += TextField::PADDING;
	sbounds.dimensions.x -= 2 * TextField::PADDING;
	pixel lineWidth = borderWidth.toPixels(bounds.dimensions.y, context->dpmm.y,
			context->pixelRatio);
	int elements =
			(maxDisplayEntries > 0) ?
					std::min(maxDisplayEntries, (int) options.size()) :
					(int) options.size();

	float entryHeight = bounds.dimensions.y / elements;
	if (backgroundColor->a > 0) {
		nvgBeginPath(nvg);
		nvgRect(nvg, bounds.position.x, bounds.position.y, bounds.dimensions.x,
				bounds.dimensions.y);
		nvgFillColor(nvg, *backgroundColor);
		nvgFill(nvg);
	}

	float th = entryHeight - TextField::PADDING;
	nvgFontSize(nvg, th);
	nvgFillColor(nvg, *textColor);

	float tw = 0.0f;
	for (MenuItemPtr& label : options) {
		tw = std::max(tw,
				nvgTextBounds(nvg, 0, 0, label->name.c_str(), nullptr,
						nullptr));
	}
	nvgTextAlign(nvg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
	pixel2 offset(0, 0);

	nvgFillColor(nvg, context->theme.DARK);
	int index = 0;
	nvgFontFaceId(nvg, context->getFontHandle(FontType::Normal));

	int N = (int) options.size();

	if (maxDisplayEntries >= 0) {
		N = std::min(selectionOffset + maxDisplayEntries, (int) options.size());
	}
	int newSelectedIndex = -1;
	for (index = selectionOffset; index < N; index++) {
		std::shared_ptr<MenuItem> current = options[index];
		if (context->isMouseContainedIn(bounds.position + offset,
				pixel2(bounds.dimensions.x, entryHeight))) {
			newSelectedIndex = index;
			if (current->isMenu()) {
				current->position = CoordPX(offset.x + bounds.dimensions.x,
						offset.y);
			}
		}
		offset.y += entryHeight;
	}
	if (newSelectedIndex >= 0) {
		setVisibleItem(options[newSelectedIndex]);
		selectedIndex = newSelectedIndex;
	} else {
		currentSelected = nullptr;
	}
	offset = pixel2(0, 0);
	const std::string rightArrow = CodePointToUTF8(0xf0da);
	for (index = selectionOffset; index < N; index++) {
		MenuItemPtr& label = options[index];
		if (index == selectedIndex) {
			nvgBeginPath(nvg);
			nvgRect(nvg, bounds.position.x + offset.x,
					bounds.position.y + offset.y, bounds.dimensions.x,
					entryHeight);
			nvgFillColor(nvg, context->theme.DARK);
			nvgFill(nvg);

			nvgFillColor(nvg, *textAltColor);
		} else {
			nvgFillColor(nvg, *textColor);
		}
		pushScissor(nvg, sbounds);
		nvgFontFaceId(nvg, context->getFontHandle(FontType::Normal));
		nvgTextAlign(nvg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
		nvgFontSize(nvg, th);
		nvgText(nvg,
				bounds.position.x + offset.x + lineWidth + TextField::PADDING,
				bounds.position.y + entryHeight / 2 + offset.y,
				label->name.c_str(), nullptr);

		std::string hint=label->getHint();
		if(hint.size()>0){
			if(index==selectedIndex){
				nvgFillColor(nvg,Color(mix(textAltColor->toRGBAf(),backgroundColor->toRGBAf(),0.5f)));
			} else {
				nvgFillColor(nvg, Color(mix(textColor->toRGBAf(),backgroundColor->toRGBAf(),0.5f)));
			}
			nvgFontSize(nvg, th-4);
			nvgFontFaceId(nvg, context->getFontHandle(FontType::Normal));
			nvgTextAlign(nvg, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE);
			nvgText(nvg,bounds.position.x + bounds.dimensions.x - lineWidth- TextField::PADDING + offset.x,bounds.position.y + entryHeight / 2 + offset.y,
					hint.c_str(), nullptr);
		}

		popScissor(nvg);
		if (label->isMenu()) {
			nvgFontSize(nvg, th);
			if(index==selectedIndex){
				nvgFillColor(nvg, *textAltColor);
			} else {
				nvgFillColor(nvg, *textColor);
			}
			nvgFontFaceId(nvg, context->getFontHandle(FontType::Icon));
			nvgTextAlign(nvg, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE);
			nvgText(nvg,
					bounds.position.x + bounds.dimensions.x - lineWidth
							- TextField::PADDING + offset.x,
					bounds.position.y + entryHeight / 2 + offset.y,
					rightArrow.c_str(), nullptr);
			nvgTextAlign(nvg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
			nvgFontFaceId(nvg, context->getFontHandle(FontType::Normal));
		}
		//Right carret 0xf0da
		offset.y += entryHeight;
	}

	if (borderColor->a > 0) {
		nvgBeginPath(nvg);
		nvgRect(nvg, bounds.position.x + lineWidth * 0.5f,
				bounds.position.y + lineWidth * 0.5f,
				bounds.dimensions.x - lineWidth,
				bounds.dimensions.y - lineWidth);
		nvgStrokeColor(nvg, *borderColor);
		nvgStrokeWidth(nvg, lineWidth);
		nvgStroke(nvg);
	}

	box2px downArrowBox(
			pixel2(
					bounds.position.x + bounds.dimensions.x
							- downArrow->width / 2,
					bounds.position.y + bounds.dimensions.y
							- downArrow->height / 2),
			pixel2(downArrow->width, downArrow->height));
	box2px upArrowBox(
			pixel2(
					bounds.position.x + bounds.dimensions.x
							- downArrow->width / 2,
					bounds.position.y - downArrow->height / 2),
			pixel2(downArrow->width, downArrow->height));

	if (maxDisplayEntries >= 0 && (int) options.size() > maxDisplayEntries) {
		if (selectionOffset > 0) {
			nvgBeginPath(nvg);
			nvgFillColor(nvg, context->theme.DARK);
			nvgCircle(nvg, bounds.position.x + bounds.dimensions.x,
					bounds.position.y, upArrow->height / 2);
			nvgFill(nvg);
			upArrow->draw(upArrowBox, context->theme.LIGHTEST, COLOR_NONE,
					context);
			if (upArrowBox.contains(
					AlloyApplicationContext()->cursorPosition)) {
				if (selectionOffset > 0) {
					selectionOffset--;
				}
			}
		}

		if (selectionOffset < (int) options.size() - maxDisplayEntries) {
			nvgBeginPath(nvg);
			nvgFillColor(nvg, context->theme.DARK);
			nvgCircle(nvg, bounds.position.x + bounds.dimensions.x,
					bounds.position.y + bounds.dimensions.y,
					downArrow->height / 2);
			nvgFill(nvg);
			downArrow->draw(downArrowBox, context->theme.LIGHTEST, COLOR_NONE,
					context);
			if (downArrowBox.contains(
					AlloyApplicationContext()->cursorPosition)) {
				if (selectionOffset
						< (int) options.size() - maxDisplayEntries) {
					selectionOffset++;
				}
			}
		}
	}
	for (std::shared_ptr<Region>& region : children) {
		if (region->isVisible()) {
			region->draw(context);
		}
	}
}
void MenuItem::setVisibleItem(const std::shared_ptr<MenuItem>& item,
		bool forceShow) {
	std::lock_guard<std::mutex> lockMe(showLock);
	currentSelected = item;
	if (item.get() != requestedSelected.get()) {
		if (forceShow) {
			showTimer.reset();
			if (currentVisible.get() != nullptr
					&& currentVisible.get() != item.get()) {
				currentVisible->setVisible(false);
			}
			item->setVisible(true);
			currentVisible = item;
			AlloyApplicationContext()->requestPack();
		} else {
			showTimer =
					std::shared_ptr<TimerTask>(
							new TimerTask(
									[=] {
										std::lock_guard<std::mutex> lockMe(showLock);
										if (currentSelected.get() == item.get()) {
											if (currentVisible.get() != nullptr&&currentVisible.get() != item.get()) {
												currentVisible->setVisible(false);
											}
											item->setVisible(true);
											currentVisible = item;
										}
										if (item.get() == requestedSelected.get()) {
											requestedSelected = nullptr;
										}
										AlloyApplicationContext()->requestPack();
									},
									[=]() {
										if (item.get() == requestedSelected.get()) {
											requestedSelected = nullptr;
										}
									}, MENU_DISPLAY_DELAY, 30));
			requestedSelected = item;
			showTimer->execute();
		}
	}
}
void MenuBar::setVisibleItem(const std::shared_ptr<MenuItem>& item,
		bool forceShow) {
	std::lock_guard<std::mutex> lockMe(showLock);
	currentSelected = item;
	if (item.get() != requestedSelected.get()) {
		if (forceShow) {
			showTimer.reset();
			if (currentVisible.get() != nullptr
					&& currentVisible.get() != item.get()) {
				AlloyApplicationContext()->removeOnTopRegion(
						currentVisible.get());
			}
			AlloyApplicationContext()->setOnTopRegion(item.get());
			item->setVisible(true);
			currentVisible = item;
			AlloyApplicationContext()->requestPack();
		} else {
			showTimer =
					std::shared_ptr<TimerTask>(
							new TimerTask(
									[=] {
										std::lock_guard<std::mutex> lockMe(showLock);
										if (currentSelected.get() == item.get()) {
											if (currentVisible.get() != nullptr&&currentVisible.get() != item.get()) {
												AlloyApplicationContext()->removeOnTopRegion(currentVisible.get());
											}
											AlloyApplicationContext()->setOnTopRegion(item.get());
											item->setVisible(true);
											currentVisible = item;
											AlloyApplicationContext()->requestPack();
										}
										if (item.get() == requestedSelected.get()) {
											requestedSelected = nullptr;
										}
									},
									[=]() {
										if (item.get() == requestedSelected.get()) {
											requestedSelected = nullptr;
										}
									}, MENU_DISPLAY_DELAY, 30));
			requestedSelected = item;
			showTimer->execute();
		}
	}
}
void Menu::addItem(const std::shared_ptr<MenuItem>& selection) {
	options.push_back(selection);
	if (selection->isMenu()) {
		Composite::add(selection);
	}
}
void Menu::setSelectedIndex(int index) {
	selectedIndex = index;
	if (index < 0) {
		label = MenuItemPtr();
		selectionOffset = 0;
	} else {
		label = options[selectedIndex];
	}
}
bool Menu::fireEvent(int selectedIndex) {
	if (selectedIndex >= 0) {
		if (options[selectedIndex]->onSelect) {
			options[selectedIndex]->onSelect();
			return true;
		}
	}
	return false;
}
void MenuItem::setVisible(bool visible) {
	if (!visible) {
		if (isVisible()) {
			currentVisible = nullptr;
			currentSelected = nullptr;
			requestedSelected = nullptr;
			if (parent != nullptr) {
				MenuItem* mitem = dynamic_cast<MenuItem*>(parent);
				if (mitem) {
					if (mitem->currentVisible.get() == this) {
						mitem->currentVisible = nullptr;
					}
					if (mitem->requestedSelected.get() == this) {
						mitem->requestedSelected = nullptr;
					}
				}
			}
		}
	} else if (!isVisible()) {
		currentVisible = nullptr;
		currentSelected = nullptr;
		requestedSelected = nullptr;
	}
	Composite::setVisible(visible);
}
void Menu::setVisible(bool visible) {
	if (!visible) {
		setSelectedIndex(-1);
		for (MenuItemPtr& item : options) {
			if (item->isMenu()) {
				item->setVisible(false);
			}
		}
	}
	MenuItem::setVisible(visible);
}
void MenuBar::hideMenus() {
	for (MenuHeaderPtr header : headers) {
		AlloyApplicationContext()->removeOnTopRegion(header->menu.get());
	}
}
Menu::Menu(const std::string& name, float menuWidth,
		const std::vector<std::shared_ptr<MenuItem>>& labels) :
		MenuItem(name, CoordPerPX(0.0f, 0.0f, 0.0f, 0.0f),
				CoordPerPX(0.0f, 0.0f, menuWidth, 0.0f)), options(labels) {
	setDetached(true);
	setVisible(false);
	backgroundColor = MakeColor(AlloyApplicationContext()->theme.LIGHTEST);
	borderColor = MakeColor(AlloyApplicationContext()->theme.LIGHTEST);
	borderWidth = UnitPX(1.0f);
	textColor = MakeColor(AlloyApplicationContext()->theme.DARK);
	textAltColor = MakeColor(AlloyApplicationContext()->theme.LIGHTER);
	downArrow = AlloyApplicationContext()->createAwesomeGlyph(0xf0ab,
			FontStyle::Normal, 14);
	upArrow = AlloyApplicationContext()->createAwesomeGlyph(0xf0aa,
			FontStyle::Normal, 14);
	onRemoveFromOnTop = [=] {
		setVisible(false);
	};
	onEvent =
			[this](AlloyContext* context, const InputEvent& event) {
				if (context->isOnTop(this)&&this->isVisible()) {
					if (event.type == InputType::MouseButton&&event.isUp() && event.button == GLFW_MOUSE_BUTTON_LEFT) {
						if (getSelectedIndex() >= 0) {
							if (fireEvent(selectedIndex)) {
								if (menuBar != nullptr) {
									menuBar->hideMenus();
								}
							}
						}
					} else if (event.type == InputType::MouseButton&&event.isDown() && event.button == GLFW_MOUSE_BUTTON_LEFT) {
						if (getSelectedIndex()<0) {
							if (menuBar != nullptr) {
								MenuItem* selected = getSelectedItem();
								if (selected == nullptr||!selected->isMenu()) {
									setSelectedIndex(-1);
									AlloyApplicationContext()->removeOnTopRegion(this);
								}
							}
						}
					}
					else if (event.type == InputType::MouseButton&&event.isDown() && event.button == GLFW_MOUSE_BUTTON_RIGHT) {
						setSelectedIndex(-1);
						AlloyApplicationContext()->removeOnTopRegion(this);
					}
					else if (event.type == InputType::Cursor && (int)options.size() > maxDisplayEntries) {
						box2px bounds = this->getBounds();
						int elements =
						(maxDisplayEntries > 0) ? std::min(maxDisplayEntries, (int)options.size()) : (int)options.size();
						float entryHeight = bounds.dimensions.y / elements;

						box2px lastBounds = bounds, firstBounds = bounds;
						lastBounds.position.y = bounds.position.y + bounds.dimensions.y - entryHeight;
						lastBounds.dimensions.y = entryHeight;
						firstBounds.dimensions.y = entryHeight;
						if (lastBounds.contains(event.cursor)) {
							if (downTimer.get() == nullptr) {
								downTimer = std::shared_ptr<TimerTask>(new TimerTask([this] {
													double deltaT = 200;
													scrollingDown = true;
													while (scrollingDown&&selectionOffset < (int)options.size() - maxDisplayEntries) {
														this->selectionOffset++;
														std::this_thread::sleep_for(std::chrono::milliseconds((long)deltaT));
														deltaT = std::max(30.0, 0.75*deltaT);
													}
												}, nullptr, MENU_DISPLAY_DELAY, 30));
								downTimer->execute();
							}
						}
						else {
							if (downTimer.get() != nullptr) {
								scrollingDown = false;
								downTimer.reset();
							}
						}
						if (firstBounds.contains(event.cursor)) {
							if (upTimer.get() == nullptr) {
								upTimer = std::shared_ptr<TimerTask>(new TimerTask([this] {
													double deltaT = 200;
													scrollingUp = true;
													while (scrollingUp&&selectionOffset > 0) {
														this->selectionOffset--;
														std::this_thread::sleep_for(std::chrono::milliseconds((long)deltaT));
														deltaT = std::max(30.0, 0.75*deltaT);
													}
												}, nullptr, MENU_DISPLAY_DELAY, 30));
								upTimer->execute();
							}
						}
						else {
							if (upTimer.get() != nullptr) {
								scrollingUp = false;
								upTimer.reset();
							}
						}
					}
					else if (event.type == InputType::Cursor) {
						if (!context->isMouseOver(this)) {
							setSelectedIndex(-1);
						}
					}
				}

				return false;
			};
	Application::addListener(this);
}
MenuBar::MenuBar(const std::string& name, const AUnit2D& position,
		const AUnit2D& dimensions) :
		MenuItem(name, position, dimensions) {

	active = false;
	barRegion = CompositePtr(
			new Composite("Bar Region", CoordPX(0.0f, 0.0f),
					CoordPercent(1.0f, 1.0f)));
	Composite::add(barRegion);
	barRegion->setOrientation(Orientation::Horizontal);
	//barRegion->cellSpacing.x = 2;
	//barRegion->cellPadding.x = 2;
	this->backgroundColor = MakeColor(AlloyApplicationContext()->theme.DARK);
}
void MenuBar::addMenu(const std::shared_ptr<Menu>& menu) {
	MenuHeaderPtr header = MenuHeaderPtr(
			new MenuHeader(menu, CoordPercent(0.0f, 0.0f),
					CoordPerPX(0.0f, 1.0f, 100.0f, 0.0f)));
	menu->position = CoordPercent(0.0f, 1.0f);
	menu->setDetached(true);
	menu->menuBar = this;
	Composite::add(menu);

	headers.push_back(header);
	barRegion->add(header);
	header->onMouseOver = [=](AlloyContext* context, const InputEvent& e) {
		if(currentVisible.get()!=nullptr) {
			setVisibleItem(menu);
		}
		return true;
	};
	header->onMouseDown = [=](AlloyContext* context, const InputEvent& e) {
		setVisibleItem(menu, true);
		return true;
	};
}
MenuItem::MenuItem(const std::string& name) :
		Composite(name) {

}

MenuItem* MenuItem::getSelectedItem() {
	if (isMenu()) {
		if (currentSelected.get() != nullptr) {
			return currentSelected.get();
		} else if (currentVisible.get() != nullptr) {
			return currentVisible->getSelectedItem();
		}
		return nullptr;
	} else {
		return nullptr;
	}
}
MenuItem::MenuItem(const std::string& name, const AUnit2D& position,
		const AUnit2D& dimensions) :
		Composite(name, position, dimensions) {
}

MenuHeader::MenuHeader(const std::shared_ptr<Menu>& menu,
		const AUnit2D& position, const AUnit2D& dimensions) :
		MenuItem(menu->name, position, dimensions), menu(menu) {
	backgroundAltColor = MakeColor(AlloyApplicationContext()->theme.LIGHTEST);
	backgroundColor = MakeColor(0, 0, 0, 0);
	textAltColor = MakeColor(AlloyApplicationContext()->theme.DARK);
	textColor = MakeColor(AlloyApplicationContext()->theme.LIGHTER);
	borderColor = MakeColor(COLOR_NONE);
	borderWidth = UnitPX(0.0f);
	fontSize = UnitPerPX(1.0f, -10);
	this->aspectRule = AspectRule::FixedHeight;
}

void MenuHeader::draw(AlloyContext* context) {
	bool hover = context->isMouseOver(this) || menu->isVisible();
	bool down = context->isMouseOver(this) && context->isLeftMouseButtonDown();
	NVGcontext* nvg = context->nvgContext;
	box2px bounds = getBounds();
	menu->position = CoordPX(bounds.position.x,
			bounds.position.y + bounds.dimensions.y);

	int xoff = 0;
	int yoff = 0;
	int vshift = 0;
	if (down && hover) {
		xoff = 0;
		yoff = 2;
	}
	if (hover) {
		nvgBeginPath(nvg);
		nvgRect(nvg, bounds.position.x + xoff,
				bounds.position.y + yoff + vshift, bounds.dimensions.x,
				bounds.dimensions.y);
		nvgFillColor(nvg, *backgroundAltColor);
		nvgFill(nvg);

	} else {
		nvgBeginPath(nvg);
		nvgRect(nvg, bounds.position.x + 1, bounds.position.y + 1 + vshift,
				bounds.dimensions.x - 2, bounds.dimensions.y - 2);
		nvgFillColor(nvg, *backgroundColor);
		nvgFill(nvg);
	}
	float th = fontSize.toPixels(bounds.dimensions.y, context->dpmm.y,
			context->pixelRatio);
	nvgFontSize(nvg, th);
	if (hover) {
		nvgFillColor(nvg, *textAltColor);
	} else {
		nvgFillColor(nvg, *textColor);
	}
	nvgFontFaceId(nvg, context->getFontHandle(FontType::Bold));
	float tw = nvgTextBounds(nvg, 0, 0, name.c_str(), nullptr, nullptr);
	double old = this->aspectRatio;
	this->aspectRatio = (tw + 10.0f) / (th + 10.0f);
	if (old != aspectRatio) {
		context->requestPack();
	}
	nvgTextAlign(nvg, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
	pixel2 offset(0, 0);
	nvgText(nvg, bounds.position.x + bounds.dimensions.x / 2 + xoff,
			bounds.position.y + bounds.dimensions.y / 2 + yoff + vshift,
			name.c_str(), nullptr);
}
}
