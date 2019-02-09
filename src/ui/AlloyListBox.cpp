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

#include "ui/AlloyListBox.h"
#include "ui/AlloyApplication.h"
#include "ui/AlloyDrawUtil.h"
namespace aly {

void ListBox::addEntry(const std::shared_ptr<ListEntry>& entry) {
	listEntries.push_back(entry);
	appendToTabChain(entry.get());
	dirty = true;
}
void ListBox::setEnableDelete(bool val) {
	enableDelete = val;
}
ListEntry* ListBox::getLastSelected() {
	if (lastSelected.size() > 0)
		return lastSelected.back();
	else
		return nullptr;
}
void ListBox::setEnableMultiSelection(bool enable) {
	enableMultiSelection = enable;
}
void ListBox::pack(const pixel2& pos, const pixel2& dims, const double2& dpmm,
		double pixelRatio, bool clamp) {
	if (dirty) {
		update();
	}
	AlloyContext* context = AlloyApplicationContext().get();
	Region::pack(pos, dims, dpmm, pixelRatio, clamp);
	pixel2 maxDim = pixel2(this->getBoundsDimensionsX(), 0.0f);
	NVGcontext* nvg = context->nvgContext;
	box2px bounds = getBounds();

	for (std::shared_ptr<ListEntry> entry : listEntries) {
		float th = entry->fontSize.toPixels(bounds.dimensions.y,
				context->dpmm.y, context->pixelRatio);
		nvgFontSize(nvg, th);
		nvgFontFaceId(nvg, context->getFontHandle(FontType::Bold));
		float tw = nvgTextBounds(nvg, 0, 0, entry->getName().c_str(), nullptr,
				nullptr) + 10;
		maxDim = aly::max(pixel2(tw, entry->entryHeight), maxDim);
	}
	for (std::shared_ptr<ListEntry> entry : listEntries) {
		entry->dimensions = CoordPX(maxDim);
	}
	Composite::pack(pos, dims, dpmm, pixelRatio, clamp);
}
void ListBox::update() {
	clear();
	lastSelected.clear();
	AlloyContext* context = AlloyApplicationContext().get();
	for (std::shared_ptr<ListEntry> entry : listEntries) {
		if (entry->parent == nullptr) {
			add(entry, true);
		}
		if (entry->isSelected()) {
			lastSelected.push_back(entry.get());
		}
	}
	dirty = false;
	context->requestPack();
}
void ListBox::clearEntries() {
	for (ListEntryPtr entry : listEntries) {
		entry->parent = nullptr;
	}
	tabChain.clear();
	listEntries.clear();
	lastSelected.clear();
	dirty = true;
}
ListBox::ListBox(const std::string& name, const AUnit2D& pos,
		const AUnit2D& dims) :
		Composite(name, pos, dims) {
	dirty = false;
	enableDelete = false;
	enableMultiSelection = false;
	scrollingDown = false;
	scrollingUp = false;
	startItem = -1;
	endItem = -1;
	downOffsetPosition = 0;
	backgroundColor = MakeColor(AlloyApplicationContext()->theme.LIGHTER);
	borderColor = MakeColor(AlloyApplicationContext()->theme.DARK);
	borderWidth = UnitPX(1.0f);
	setOrientation(Orientation::Vertical, pixel2(0, 2), pixel2(0, 2));
	setScrollEnabled(true);
	dragBox = box2px(float2(0, 0), float2(0, 0));
	Application::addListener(this);
}
bool ListBox::removeAll() {
	if (!enableDelete) {
		std::cerr << "Could not delete from list box [" << getName()
				<< "] because delete is not enabled" << std::endl;
		return false;
	}
	std::vector<ListEntryPtr>& entries = getEntries();
	std::vector<ListEntryPtr> removalList = entries;
	entries.clear();
	if (removalList.size() > 0) {
		update();
		AlloyApplicationContext()->requestPack();
		if (onDeleteEntry) {
			onDeleteEntry(removalList);
		}
		return true;
	}
	return false;
}

void ListEntry::setSelected(bool selected) {
	this->selected = selected;
}
bool ListEntry::isSelected() {
	return selected;
}
void ListEntry::draw(AlloyContext* context) {
	box2px bounds = getBounds();
	NVGcontext* nvg = context->nvgContext;
	bool hover = context->isMouseOver(this);
	bool down = context->isMouseDown(this);
	bool selected = this->selected || dialog->isDraggingOver(this);
	int xoff = 0;
	int yoff = 0;
	if (down) {
		xoff = 2;
		yoff = 2;
	}
	if (hover || down) {
		nvgBeginPath(nvg);
		nvgRoundedRect(nvg, bounds.position.x + xoff, bounds.position.y + yoff,
				bounds.dimensions.x, bounds.dimensions.y,
				context->theme.CORNER_RADIUS);
		if (selected) {
			nvgFillColor(nvg, context->theme.LINK);
		} else {
			nvgFillColor(nvg, *backgroundColor);
		}
		nvgFill(nvg);
	} else {
		nvgBeginPath(nvg);
		nvgRoundedRect(nvg, bounds.position.x + 1, bounds.position.y + 1,
				bounds.dimensions.x - 2, bounds.dimensions.y - 2,
				context->theme.CORNER_RADIUS);
		if (selected) {
			nvgFillColor(nvg, context->theme.LINK);
		} else {
			nvgFillColor(nvg, Color(0, 0, 0, 0));
		}
		nvgFill(nvg);
	}
	float th = fontSize.toPixels(bounds.dimensions.y, context->dpmm.y,
			context->pixelRatio);
	nvgFontSize(nvg, th);
	nvgTextAlign(nvg, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT);
	nvgFontFaceId(nvg, context->getFontHandle(FontType::Icon));
	float iw =
			(iconCodeString.size() > 0) ?
					nvgTextBounds(nvg, 0, 0, iconCodeString.c_str(), nullptr,
							nullptr)
							+ AlloyApplicationContext()->theme.SPACING.x :
					0;
	pixel2 offset(0, 0);

	if (selected) {
		if (hover) {

			nvgFillColor(nvg, context->theme.LIGHTEST);
		} else {
			nvgFillColor(nvg, context->theme.LIGHTER);
		}
	} else {
		if (hover) {

			nvgFillColor(nvg, context->theme.LIGHTEST);
		} else {
			nvgFillColor(nvg, context->theme.DARK);
		}
	}

	box2px labelBounds = getCursorBounds();
	pushScissor(nvg, labelBounds);
	if (iconCodeString.size() > 0) {
		nvgFontFaceId(nvg, context->getFontHandle(FontType::Icon));
		nvgText(nvg,
				AlloyApplicationContext()->theme.SPACING.x + bounds.position.x
						+ xoff,
				bounds.position.y + bounds.dimensions.y / 2 + yoff,
				iconCodeString.c_str(), nullptr);
	}
	nvgFontFaceId(nvg, context->getFontHandle(FontType::Bold));
	nvgText(nvg,
			AlloyApplicationContext()->theme.SPACING.x + bounds.position.x + iw
					+ xoff, bounds.position.y + bounds.dimensions.y / 2 + yoff,
			label.c_str(), nullptr);
	popScissor(nvg);

	const int PAD = 1.0f;
	if (isObjectFocused()) {
		nvgLineJoin(nvg, NVG_MITER);
		nvgBeginPath(nvg);
		if (hover || down) {
			nvgRoundedRect(nvg, bounds.position.x + xoff,
					bounds.position.y + yoff, bounds.dimensions.x,
					bounds.dimensions.y, context->theme.CORNER_RADIUS);
		} else {
			nvgRoundedRect(nvg, bounds.position.x + 1, bounds.position.y + 1,
					bounds.dimensions.x - 2, bounds.dimensions.y - 2,
					context->theme.CORNER_RADIUS);
		}
		nvgStrokeWidth(nvg, 2.0f);
		nvgStrokeColor(nvg, context->theme.FOCUS);
		nvgStroke(nvg);
	}

}

bool ListBox::addVerticalScrollPosition(int c) {
	if (listEntries.size() > 0) {
		float t = c * (cellSpacing.y + listEntries.front()->entryHeight)
				* (this->verticalScrollTrack->getBoundsDimensionsY()
						- this->verticalScrollHandle->getBoundsDimensionsY())
				/ std::max(1E-6f, extents.dimensions.y - bounds.dimensions.y);
		if (verticalScrollHandle->addDragOffset(pixel2(0.0f, t))) {
			this->scrollPosition.y =
					(this->verticalScrollHandle->getBoundsPositionY()
							- this->verticalScrollTrack->getBoundsPositionY())
							/ std::max(1.0f,
									(float) this->verticalScrollTrack->getBoundsDimensionsY()
											- (float) this->verticalScrollHandle->getBoundsDimensionsY());
			updateExtents();
			AlloyApplicationContext()->requestPack();
			return true;
		}
	}
	return false;
}
void ListBox::scrollToBottom() {
	float t =
			this->verticalScrollTrack->getBoundsPositionY()
					+ std::max(0.0f,
							this->verticalScrollTrack->getBoundsDimensionsY()
									- (float) this->verticalScrollHandle->getBoundsDimensionsY());
	if (verticalScrollHandle.get() != nullptr
			&& verticalScrollHandle->addDragOffset(pixel2(0.0f, t))) {
		this->scrollPosition.y =
				(this->verticalScrollHandle->getBoundsPositionY()
						- this->verticalScrollTrack->getBoundsPositionY())
						/ std::max(1.0f,
								(float) this->verticalScrollTrack->getBoundsDimensionsY()
										- (float) this->verticalScrollHandle->getBoundsDimensionsY());
		updateExtents();
		AlloyApplicationContext()->requestPack();
	}
}
void ListBox::scrollToTop() {
	if (verticalScrollHandle.get() != nullptr) {
		verticalScrollHandle->setDragOffset(pixel2(0.0f, 0.0f));
		this->scrollPosition.y =
				(this->verticalScrollHandle->getBoundsPositionY()
						- this->verticalScrollTrack->getBoundsPositionY())
						/ std::max(1.0f,
								(float) this->verticalScrollTrack->getBoundsDimensionsY()
										- (float) this->verticalScrollHandle->getBoundsDimensionsY());
		updateExtents();
		AlloyApplicationContext()->requestPack();
	}
}
bool ListBox::removeSelected() {
	if (!enableDelete) {
		std::cerr << "Could not delete from list box [" << getName()
				<< "] because delete is not enabled" << std::endl;
		return false;
	}
	std::vector<ListEntryPtr>& entries = getEntries();
	bool removed = false;
	ListEntryPtr next;
	std::vector<ListEntryPtr> removalList;
	for (int i = 0; i < (int) entries.size(); i++) {
		ListEntryPtr entry = entries[i];
		if (entry->isSelected()) {
			if (i < (int) entries.size() - 1) {
				next = entries[i + 1];
			}
			entries.erase(entries.begin() + i);
			removalList.push_back(entry);
			removed = true;
			i--;
		}
	}
	if (next.get() != nullptr) {
		next->setSelected(true);
	}
	if (removed) {
		update();
		AlloyApplicationContext()->requestPack();
		if (onDeleteEntry) {
			onDeleteEntry(removalList);
		}
		return true;
	}
	return false;
}
bool ListBox::onEventHandler(AlloyContext* context, const InputEvent& e) {
	if (!isVisible())
		return false;
	if (!context->isMouseOver(this, true)) {
		if (!Composite::onEventHandler(context, e)) {
			bool ret = false;
			for (auto entry : listEntries) {
				if (entry->onEventHandler(context, e)) {
					ret = true;
				}
			}
			return ret;
		} else {
			return true;
		}
	}
	Region* mouseDownRegion = context->getMouseDownObject();
	if (mouseDownRegion == nullptr) {
		bool click = false;
		for (auto entry : listEntries) {
			if (entry->isSelected()
					&& context->isMouseOver(entry.get(), true)) {
				context->setMouseDownObject(entry.get());
				entry->setFocus(true);
				break;
			}
		}
	}
	if (e.type == InputType::Key) {
		if (e.isDown() && e.key == GLFW_KEY_SPACE) {
			for (auto entry : listEntries) {
				if (entry->isObjectFocused()) {
					if (enableMultiSelection) {
						if (entry->isSelected()) {
							entry->setSelected(false);
							for (auto iter = lastSelected.begin();
									iter != lastSelected.end(); iter++) {
								if (*iter == entry.get()) {
									lastSelected.erase(iter);
									break;
								}
							}
						} else {
							entry->setSelected(true);
							lastSelected.push_back(entry.get());
						}
					} else {
						for (ListEntry* child : lastSelected) {
							child->setSelected(false);
						}
						lastSelected.clear();
						if (entry->isSelected()) {
							entry->setSelected(false);
						} else {
							entry->setSelected(true);
							lastSelected.push_back(entry.get());
						}
					}
					if (onSelect)
						onSelect(entry.get(), e);
					break;
				}
			}
		} else if (e.isDown() && e.isControlDown() && e.key == GLFW_KEY_A
				&& enableMultiSelection) {
			for (auto entry : listEntries) {
				if (!entry->isSelected()) {
					entry->setSelected(true);
					lastSelected.push_back(entry.get());
					if (onSelect)
						onSelect(entry.get(), e);
				}
			}
		} else if (e.isUp() && enableDelete && e.key == GLFW_KEY_DELETE) {
			if (e.isControlDown()) {
				removeAll();
			} else {
				removeSelected();
			}
		} else if (e.key == GLFW_KEY_DOWN && e.isDown()) {
			return addVerticalScrollPosition(1);
		} else if (e.key == GLFW_KEY_UP && e.isDown()) {
			return addVerticalScrollPosition(-1);
		} else if (e.key == GLFW_KEY_PAGE_DOWN && e.isDown()) {
			return addVerticalScrollPosition(5);
		} else if (e.key == GLFW_KEY_PAGE_UP && e.isDown()) {
			return addVerticalScrollPosition(-5);
		} else if (e.key == GLFW_KEY_HOME && e.isDown()) {
			scrollToTop();
			return true;
		} else if (e.key == GLFW_KEY_END && e.isDown()) {
			scrollToBottom();
			return true;
		}
	}
	if (e.type == InputType::Cursor || e.type == InputType::MouseButton) {
		if (context->isMouseDrag() && e.button == GLFW_MOUSE_BUTTON_LEFT) {
			if (enableMultiSelection) {
				if (startItem < 0) {
					downOffsetPosition = extents.position.y;
				}
				float2 cursorDown = context->getCursorDownPosition();
				int index = 0;
				if (startItem < 0) {
					for (std::shared_ptr<ListEntry> entry : listEntries) {
						if (entry->getBounds().contains(cursorDown)) {
							startItem = index;
							break;
						}
						index++;
					}
				}
				index = 0;
				for (std::shared_ptr<ListEntry> entry : listEntries) {
					if (entry->getBounds().contains(e.cursor)) {
						endItem = index;
						break;
					}
					index++;
				}

			}
		} else if (!context->isMouseDown()
				&& e.type == InputType::MouseButton) {
			if (enableMultiSelection) {
				int index = 0;
				for (std::shared_ptr<ListEntry> entry : listEntries) {
					if (entry->getBounds().contains(e.cursor)) {
						endItem = index;
						break;
					}
					index++;
				}
				if (endItem < startItem) {
					std::swap(startItem, endItem);
				}
				if (startItem >= 0 && e.button == GLFW_MOUSE_BUTTON_LEFT) {
					for (int i = startItem; i <= endItem; i++) {
						std::shared_ptr<ListEntry> entry = listEntries[i];
						if (!entry->isSelected()) {
							entry->setSelected(true);
							lastSelected.push_back(entry.get());
							entry->setFocus(true);
							if (onSelect)
								onSelect(entry.get(), e);
						}
					}
				}
			}
			if (e.button == GLFW_MOUSE_BUTTON_RIGHT) {
				for (std::shared_ptr<ListEntry> entry : listEntries) {
					entry->setSelected(false);
					entry->setFocus(false);
				}
				lastSelected.clear();
				if (onSelect) {
					onSelect(nullptr, e);
				}
			}
			dragBox = box2px(float2(0, 0), float2(0, 0));
			startItem = -1;
			endItem = -1;
		} else {
			dragBox = box2px(float2(0, 0), float2(0, 0));
			startItem = -1;
			endItem = -1;

		}
	}
	if (e.type == InputType::Cursor) {
		box2px bounds = this->getBounds();
		box2px lastBounds = bounds, firstBounds = bounds;
		float entryHeight = 30;
		lastBounds.position.y = bounds.position.y + bounds.dimensions.y
				- entryHeight;
		lastBounds.dimensions.y = entryHeight;
		firstBounds.dimensions.y = entryHeight;
		if ((!isHorizontalScrollVisible() && lastBounds.contains(e.cursor))
				|| (dragBox.dimensions.x * dragBox.dimensions.y > 0
						&& e.cursor.y > bounds.dimensions.y + bounds.position.y)) {
			if (downTimer.get() == nullptr) {
				downTimer =
						std::shared_ptr<TimerTask>(
								new TimerTask(
										[this] {
											double deltaT = 200;
											scrollingDown = true;
											while (scrollingDown) {
												if (!addVerticalScrollPosition(10.0f))break;
												std::this_thread::sleep_for(std::chrono::milliseconds((long)deltaT));
												deltaT = std::max(30.0, 0.75*deltaT);
											}
										}, nullptr, 500, 30));
				downTimer->execute();
			}
		} else {
			if (downTimer.get() != nullptr) {
				scrollingDown = false;
				downTimer.reset();
			}
		}
		if (firstBounds.contains(e.cursor)
				|| (dragBox.dimensions.x * dragBox.dimensions.y > 0
						&& e.cursor.y < bounds.position.y)) {
			if (upTimer.get() == nullptr) {
				upTimer =
						std::shared_ptr<TimerTask>(
								new TimerTask(
										[this] {
											double deltaT = 200;
											scrollingUp = true;
											while (scrollingUp) {
												if (!addVerticalScrollPosition(-10.0f))break;
												std::this_thread::sleep_for(std::chrono::milliseconds((long)deltaT));
												deltaT = std::max(30.0, 0.75*deltaT);
											}
										}, nullptr, 500, 30));
				upTimer->execute();
			}
		} else {
			if (upTimer.get() != nullptr) {
				scrollingUp = false;
				upTimer.reset();
			}
		}
	}
	if (!Composite::onEventHandler(context, e)) {
		bool ret = false;
		for (auto entry : listEntries) {
			if (entry->onEventHandler(context, e)) {
				ret = true;
			}
		}
		return ret;
	} else {
		return true;
	}
}
void ListBox::draw(AlloyContext* context) {
	if (startItem >= 0) {
		float2 cursorDown = context->getCursorDownPosition();
		float2 cursorPos = context->getCursorPosition();
		cursorDown.y += extents.position.y - downOffsetPosition;
		float2 stPt = aly::min(cursorDown, cursorPos);
		float2 endPt = aly::max(cursorDown, cursorPos);
		dragBox.position = stPt;
		dragBox.dimensions = endPt - stPt;
		dragBox.intersect(getBounds());
	}
	pushScissor(context->nvgContext, getCursorBounds());
	Composite::draw(context);
	popScissor(context->nvgContext);
	NVGcontext* nvg = context->nvgContext;
	const int PAD = 1.0f;
	if (isObjectFocused()) {
		nvgLineJoin(nvg, NVG_MITER);
		nvgBeginPath(nvg);
		if (roundCorners) {
			nvgRoundedRect(nvg, bounds.position.x + PAD,
					bounds.position.y + PAD, bounds.dimensions.x - 2 * PAD,
					bounds.dimensions.y - 2 * PAD,
					context->theme.CORNER_RADIUS);
		} else {
			nvgRect(nvg, bounds.position.x + PAD, bounds.position.y + PAD,
					bounds.dimensions.x - 2 * PAD,
					bounds.dimensions.y - 2 * PAD);
		}
		nvgStrokeWidth(nvg, 2.0f);
		nvgStrokeColor(nvg, context->theme.FOCUS);
		nvgStroke(nvg);
	}
	if (dragBox.dimensions.x > 0 && dragBox.dimensions.y > 0) {
		nvgBeginPath(nvg);
		nvgRect(nvg, dragBox.position.x, dragBox.position.y,
				dragBox.dimensions.x, dragBox.dimensions.y);
		nvgFillColor(nvg, context->theme.DARK.toSemiTransparent(0.5f));
		nvgFill(nvg);

		nvgBeginPath(nvg);
		nvgRect(nvg, dragBox.position.x, dragBox.position.y,
				dragBox.dimensions.x, dragBox.dimensions.y);
		nvgStrokeWidth(nvg, 2.0f);
		nvgStrokeColor(nvg, context->theme.DARK);
		nvgStroke(nvg);
	}
}
bool ListBox::isDraggingOver(ListEntry* entry) {
	if (entry->isSelected() || dragBox.intersects(entry->getBounds())) {
		return true;
	} else {
		return false;
	}
}

NumberEntry::NumberEntry(ListBox* listBox, const NumberType& type,
		float entryHeight) :
		ListEntry(listBox, "", entryHeight) {
	float w = (type == NumberType::Double) ? 5 * entryHeight : 3 * entryHeight;
	valueRegion = ModifiableNumberPtr(
			new ModifiableNumber("Value",
					CoordPerPX(1.0f, 0.0f, -w - 15.0f, 1.0f),
					CoordPX(w, entryHeight - 2.0f), type));
	setLabel("Number");
	backgroundColor = MakeColor(AlloyApplicationContext()->theme.LIGHT);
	valueRegion->backgroundColor = MakeColor(0, 0, 0, 64);
	valueRegion->borderColor = MakeColor(0, 0, 0, 0);
	valueRegion->borderWidth = UnitPX(0.0f);
	valueRegion->setRoundCorners(false);
	valueRegion->textColor = MakeColor(AlloyApplicationContext()->theme.DARK);
	valueRegion->setAlignment(HorizontalAlignment::Center,
			VerticalAlignment::Middle);
	add(valueRegion);
}
void NumberEntry::setValue(const Number& number) {
	valueRegion->setNumberValue(number);
}
void NumberEntry::draw(AlloyContext* context) {
	box2px bounds = getBounds();
	NVGcontext* nvg = context->nvgContext;
	bool hover = context->isMouseOver(this);
	bool down = context->isMouseDown(this);
	bool selected = this->selected || dialog->isDraggingOver(this);
	int xoff = 0;
	int yoff = 0;
	if (down) {
		xoff = 2;
		yoff = 2;
	}
	if (hover || down) {
		nvgBeginPath(nvg);
		nvgRoundedRect(nvg, bounds.position.x + xoff, bounds.position.y + yoff,
				bounds.dimensions.x, bounds.dimensions.y,
				context->theme.CORNER_RADIUS);
		if (selected) {
			nvgFillColor(nvg, context->theme.LINK);
		} else {
			nvgFillColor(nvg, *backgroundColor);
		}
		nvgFill(nvg);
	} else {
		nvgBeginPath(nvg);
		nvgRoundedRect(nvg, bounds.position.x + 1, bounds.position.y + 1,
				bounds.dimensions.x - 2, bounds.dimensions.y - 2,
				context->theme.CORNER_RADIUS);
		if (selected) {
			nvgFillColor(nvg, context->theme.LINK);
		} else {
			nvgFillColor(nvg, Color(0, 0, 0, 0));
		}
		nvgFill(nvg);
	}
	float th = fontSize.toPixels(bounds.dimensions.y, context->dpmm.y,
			context->pixelRatio);
	nvgFontSize(nvg, th);
	nvgTextAlign(nvg, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT);
	nvgFontFaceId(nvg, context->getFontHandle(FontType::Icon));
	float iw =
			(iconCodeString.size() > 0) ?
					nvgTextBounds(nvg, 0, 0, iconCodeString.c_str(), nullptr,
							nullptr)
							+ AlloyApplicationContext()->theme.SPACING.x :
					0;
	pixel2 offset(0, 0);

	if (selected) {
		if (hover) {
			nvgFillColor(nvg, context->theme.LIGHTEST);
			valueRegion->textColor = MakeColor(context->theme.LIGHTEST);
		} else {
			nvgFillColor(nvg, context->theme.LIGHTER);
			valueRegion->textColor = MakeColor(context->theme.LIGHTER);
		}
	} else {
		if (hover) {
			nvgFillColor(nvg, context->theme.LIGHTEST);
			valueRegion->textColor = MakeColor(context->theme.LIGHTEST);
		} else {
			nvgFillColor(nvg, context->theme.DARK);
			valueRegion->textColor = MakeColor(context->theme.DARK);
		}
	}

	box2px labelBounds = getCursorBounds();
	pushScissor(nvg, labelBounds);
	if (iconCodeString.size() > 0) {
		nvgFontFaceId(nvg, context->getFontHandle(FontType::Icon));
		nvgText(nvg,
				AlloyApplicationContext()->theme.SPACING.x + bounds.position.x
						+ xoff,
				bounds.position.y + bounds.dimensions.y / 2 + yoff,
				iconCodeString.c_str(), nullptr);
	}
	nvgFontFaceId(nvg, context->getFontHandle(FontType::Bold));
	nvgText(nvg,
			AlloyApplicationContext()->theme.SPACING.x + bounds.position.x + iw
					+ xoff, bounds.position.y + bounds.dimensions.y / 2 + yoff,
			label.c_str(), nullptr);
	for (RegionPtr child : children) {
		child->draw(context);
	}
	popScissor(nvg);

}

void NumberListBox::clearEntries() {
	valueRegion->clearEntries();
}
void NumberListBox::update() {
	std::vector<std::shared_ptr<ListEntry>> &entries =
			valueRegion->getEntries();
	values.resize(valueRegion->getEntries().size());
	for (int i = 0; i < (int) entries.size(); i++) {
		entries[i]->setLabel(MakeString() << "[" << i << "]");
		NumberEntryPtr numEntry = std::dynamic_pointer_cast<NumberEntry>(
				entries[i]);
		values[i] = numEntry->getValue();

	}
	valueRegion->update();
	for (int i = 0; i < (int) entries.size(); i++) {
		NumberEntryPtr numEntry = std::dynamic_pointer_cast<NumberEntry>(
				entries[i]);
		Application::addListener(numEntry->getRegion().get());
	}
}
void NumberListBox::fireEvent() {
	if (onChange) {
		std::vector<std::shared_ptr<ListEntry>> &entries =
				valueRegion->getEntries();
		values.resize(entries.size());
		int index = 0;
		for (ListEntryPtr entry : entries) {
			NumberEntryPtr numEntry = std::dynamic_pointer_cast<NumberEntry>(
					entry);
			values[index++] = numEntry->getValue();
		}
		onChange(values);
	}
}
void NumberListBox::addNumbers(const std::vector<Number>& numbers) {
	for (const Number& number : numbers) {
		NumberEntryPtr entry = NumberEntryPtr(
				new NumberEntry(valueRegion.get(), number.type(),
						this->entryHeight));
		entry->setValue(number);
		entry->getRegion()->onTextEntered = [this](NumberField* field) {
			update();
			fireEvent();
		};
		valueRegion->addEntry(entry);
	}
	update();
	fireEvent();
}

NumberListBox::NumberListBox(const std::string& name, const AUnit2D& pos,
		const AUnit2D& dims, const NumberType& type, float entryHeight) :
		Composite(name, pos, dims), entryHeight(entryHeight) {
	valueRegion = ListBoxPtr(
			new ListBox(name, CoordPX(0.0f, 0.0f),
					CoordPerPX(1.0f, 1.0f, -entryHeight - 3.0f, 0.0f)));
	valueRegion->setEnableDelete(true);
	RegionPtr bgRegion = RegionPtr(
			new Region(name, CoordPerPX(1.0f, 0.0f, -entryHeight - 3, 0.0f),
					CoordPerPX(0.0f, 1.0f, 2.0f, 0.0f)));
	bgRegion->backgroundColor = MakeColor(AlloyDefaultContext()->theme.DARK);
	bgRegion->setRoundCorners(false);
	bgRegion->setIgnoreCursorEvents(true);
	addButton = IconButtonPtr(
			new IconButton(0xf067, CoordPerPX(1.0f, 0.0f, -entryHeight, 1.0f),
					CoordPX(entryHeight - 2, entryHeight - 2)));
	upButton = IconButtonPtr(
			new IconButton(0xf0d8,
					CoordPerPX(1.0f, 0.0f, -entryHeight, entryHeight + 1.0f),
					CoordPX(entryHeight - 2, entryHeight - 2)));
	downButton = IconButtonPtr(
			new IconButton(0xf0d7,
					CoordPerPX(1.0f, 0.0f, -entryHeight,
							2 * entryHeight + 1.0f),
					CoordPX(entryHeight - 2, entryHeight - 2)));
	eraseButton = IconButtonPtr(
			new IconButton(0xf00d,
					CoordPerPX(1.0f, 0.0f, -entryHeight,
							3 * entryHeight + 1.0f),
					CoordPX(entryHeight - 2, entryHeight - 2)));
	valueRegion->setRoundCorners(true);
	valueRegion->borderWidth = UnitPX(0.0f);
	valueRegion->backgroundColor = MakeColor(0, 0, 0, 0);
	addButton->backgroundColor = MakeColor(0, 0, 0, 0);
	addButton->foregroundColor = MakeColor(0, 0, 0, 0);
	addButton->borderWidth = UnitPX(0.0f);
	addButton->borderColor = MakeColor(AlloyDefaultContext()->theme.DARK);
	addButton->iconColor = MakeColor(AlloyDefaultContext()->theme.DARK);
	upButton->backgroundColor = MakeColor(0, 0, 0, 0);
	upButton->foregroundColor = MakeColor(0, 0, 0, 0);
	upButton->iconColor = MakeColor(AlloyDefaultContext()->theme.DARK);
	upButton->borderWidth = UnitPX(0.0f);
	upButton->borderColor = MakeColor(AlloyDefaultContext()->theme.DARK);

	downButton->backgroundColor = MakeColor(0, 0, 0, 0);
	downButton->foregroundColor = MakeColor(0, 0, 0, 0);
	downButton->iconColor = MakeColor(AlloyDefaultContext()->theme.DARK);
	downButton->borderWidth = UnitPX(0.0f);
	downButton->borderColor = MakeColor(AlloyDefaultContext()->theme.DARK);

	eraseButton->backgroundColor = MakeColor(0, 0, 0, 0);
	eraseButton->foregroundColor = MakeColor(0, 0, 0, 0);
	eraseButton->iconColor = MakeColor(AlloyDefaultContext()->theme.DARK);
	eraseButton->borderWidth = UnitPX(0.0f);
	eraseButton->borderColor = MakeColor(AlloyDefaultContext()->theme.DARK);
	borderWidth = UnitPX(0.0f);
	backgroundColor = MakeColor(AlloyDefaultContext()->theme.LIGHTER);
	setRoundCorners(true);

	Composite::add(valueRegion);
	Composite::add(addButton);
	Composite::add(upButton);
	Composite::add(downButton);
	Composite::add(eraseButton);
	Composite::add(bgRegion);
	addButton->onMouseDown =
			[this,type](AlloyContext* context, const InputEvent& e) {
				if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
					NumberEntryPtr entry = NumberEntryPtr(new NumberEntry(valueRegion.get(),type, this->entryHeight));
					Number defaultValue;
					switch(type) {
						case NumberType::Integer:defaultValue=Integer(0);break;
						case NumberType::Float:defaultValue=Float(0);break;
						case NumberType::Double:defaultValue=Double(0);break;
						case NumberType::Boolean:defaultValue=Boolean(0);break;
					}
					entry->setValue(defaultValue);
					entry->getRegion()->onTextEntered=[this](NumberField* field) {
						update();
						fireEvent();
					};
					valueRegion->addEntry(entry);
					update();
					return true;
				}
				return false;
			};
	eraseButton->onMouseDown =
			[this](AlloyContext* context, const InputEvent& e) {
				return valueRegion->removeAll();
			};
	valueRegion->onDeleteEntry =
			[this](const std::vector<ListEntryPtr>& removalList) {
				fireEvent();
			};

	upButton->onMouseDown = [this](AlloyContext* context, const InputEvent& e) {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
			std::vector<ListEntryPtr>& entries = valueRegion->getEntries();
			int N = (int)entries.size();
			for (int i = 1;i < N;i++) {
				if (entries[i]->isSelected()) {
					std::swap(entries[std::max(i - 1, 0)], entries[i]);
					update();
					context->requestPack();
					fireEvent();
					break;
				}
			}
			return true;
		}
		return false;
	};
	downButton->onMouseDown =
			[this](AlloyContext* context, const InputEvent& e) {
				std::vector<ListEntryPtr>& entries = valueRegion->getEntries();
				int N = (int)entries.size();
				for (int i = 0;i < N - 1;i++) {
					if (entries[i]->isSelected()) {
						std::swap(entries[std::min(i + 1, N - 1)], entries[i]);
						update();
						context->requestPack();
						fireEvent();
						break;
					}
				}
				return false;
			};
}

}
