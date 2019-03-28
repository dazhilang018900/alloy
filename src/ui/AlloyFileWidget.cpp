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
#include "ui/AlloyFileWidget.h"
#include "ui/AlloyApplication.h"
#include "ui/AlloyDrawUtil.h"
#include <cctype>
namespace aly {
bool ListEntry::onEventHandler(AlloyContext* context, const InputEvent& event) {
	return Composite::onEventHandler(context, event);
}
ListEntry::ListEntry(ListBox* listBox, const std::string& name,
		float entryHeight) :
		Composite(name), dialog(listBox), entryHeight(entryHeight) {
	this->backgroundColor = MakeColor(AlloyApplicationContext()->theme.NEUTRAL);
	this->borderColor = MakeColor(COLOR_NONE);
	this->selected = false;
	iconCodeString = "";
	setLabel(name);
	this->onMouseDown = [this](AlloyContext* context, const InputEvent& e) {
		return dialog->onMouseDown(this, context, e);
	};
}
void ListEntry::setLabel(const std::string& label) {
	this->label = label;
	float th = entryHeight - 2 * TextField::PADDING;
	fontSize = UnitPX(th);
	AlloyContext* context = AlloyApplicationContext().get();
	NVGcontext* nvg = context->getNVG();
	nvgFontSize(nvg, th);
	nvgTextAlign(nvg, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT);
	nvgFontFaceId(nvg, context->getFontHandle(FontType::Bold));
	float tw = nvgTextBounds(nvg, 0, 0, label.c_str(), nullptr, nullptr);
	if (iconCodeString.size() > 0) {
		nvgFontFaceId(nvg, context->getFontHandle(FontType::Icon));
		tw += nvgTextBounds(nvg, 0, 0, iconCodeString.c_str(), nullptr, nullptr)
				+ 3 * context->theme.SPACING.x;
	} else {
		tw += 2 * context->theme.SPACING.x;
	}
	position = CoordPX(0.0f, 0.0f);
	dimensions = CoordPX(tw, entryHeight);
	bounds.dimensions = pixel2(tw, entryHeight);
}
void ListEntry::setIcon(int icon) {
	this->iconCodeString = CodePointToUTF8(icon);
	float th = entryHeight - 2 * TextField::PADDING;
	fontSize = UnitPX(th);
	AlloyContext* context = AlloyApplicationContext().get();
	NVGcontext* nvg = context->getNVG();
	nvgFontSize(nvg, th);
	nvgTextAlign(nvg, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT);
	nvgFontFaceId(nvg, context->getFontHandle(FontType::Bold));
	float tw = nvgTextBounds(nvg, 0, 0, label.c_str(), nullptr, nullptr);
	if (iconCodeString.size() > 0) {
		nvgFontFaceId(nvg, context->getFontHandle(FontType::Icon));
		tw += nvgTextBounds(nvg, 0, 0, iconCodeString.c_str(), nullptr, nullptr)
				+ 3 * context->theme.SPACING.x;
	} else {
		tw += 2 * context->theme.SPACING.x;
	}
	position = CoordPX(0.0f, 0.0f);
	dimensions = CoordPX(tw, entryHeight);
}
bool ListBox::onMouseDown(ListEntry* entry, AlloyContext* context,
		const InputEvent& e) {
	if (e.isDown()) {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
			if (enableMultiSelection) {
				if (entry->isSelected() && e.clicks == 1) {
					entry->setSelected(false);
					for (auto iter = lastSelected.begin();
							iter != lastSelected.end(); iter++) {
						if (*iter == entry) {
							lastSelected.erase(iter);
							break;
						}
					}
				} else if (e.isShiftDown()) {
					int startIndex = -1;
					int endIndex = -1;
					if (lastSelected.size() > 0) {
						auto lastEntry = lastSelected.back();
						int index = 0;
						for (auto le : listEntries) {
							if (le.get() == lastEntry) {
								startIndex = index;
							}
							if (le.get() == entry) {
								endIndex = index;
							}
							index++;
						}
						if (startIndex > endIndex) {
							std::swap(startIndex, endIndex);
						}
						for (int i = startIndex; i <= endIndex; i++) {
							if (!listEntries[i]->isSelected()) {
								listEntries[i]->setSelected(true);
								lastSelected.push_back(listEntries[i].get());
								if (listEntries[i].get() != entry) {
									if (onSelect)
										onSelect(entry, e);
								}
							}
						}
					} else {
						if (!entry->isSelected()) {
							entry->setSelected(true);
							lastSelected.push_back(entry);
						}
					}
				} else {
					if (!entry->isSelected()) {
						entry->setSelected(true);
						lastSelected.push_back(entry);
					}
				}

			} else {
				if (!entry->isSelected()) {
					for (ListEntry* child : lastSelected) {
						child->setSelected(false);
					}
					entry->setSelected(true);
					lastSelected.clear();
					lastSelected.push_back(entry);
				}
			}
			if (onSelect)
				onSelect(entry, e);
			return true;
		} else if (e.button == GLFW_MOUSE_BUTTON_RIGHT) {
			for (ListEntry* child : lastSelected) {
				child->setSelected(false);
			}
			if (onSelect)
				onSelect(nullptr, e);
			return true;
		}
	}
	return false;
}
FileField::FileField(const std::string& name, const AUnit2D& position,
		const AUnit2D& dimensions, bool directoryInput) :
		TextField(name, position, dimensions), directoryInput(directoryInput), preferredFieldSize(
				32) {
	showDefaultLabel = true;
	autoSuggest = true;
	selectionBox = SelectionBoxPtr(new SelectionBox(label));
	selectionBox->setDetached(true);
	selectionBox->setVisible(false);
	selectionBox->position = CoordPerPX(0.0f, 0.0f, 2.0f, 0.0f);
	selectionBox->dimensions = CoordPerPX(1.0f, 0.8f, -4.0f, 0.0f);
	selectionBox->backgroundColor = MakeColor(
			AlloyApplicationContext()->theme.DARK);
	selectionBox->borderColor = MakeColor(
			AlloyApplicationContext()->theme.LIGHTER);
	selectionBox->borderWidth = UnitPX(1.0f);
	selectionBox->textColor = MakeColor(
			AlloyApplicationContext()->theme.LIGHTER);
	selectionBox->textAltColor = MakeColor(
			AlloyApplicationContext()->theme.LIGHTER);
	add(selectionBox);
	selectionBox->onSelect = [this](SelectionBox* box) {
		selectionBox->setVisible(false);
		AlloyApplicationContext()->removeOnTopRegion(box);
		std::string path = GetParentDirectory(lastValue);
		this->setValue(path + box->getSelection(box->getSelectedIndex()));
		if (onTextEntered) {
			onTextEntered(this);
		}
		return false;
	};
}
void FileField::setValue(const std::string& text) {

	if (text != this->value) {
		this->value = text;
		segmentedPath = SplitPath(value);
		textStart = 0;
		moveCursorTo((int) value.size());
	}
	if (text.size() == 0) {
		label = name;
	} else {
		if ((int) text.size() > preferredFieldSize) {
			if (directoryInput) {
				this->label = std::string("..") + ALY_PATH_SEPARATOR+ GetFileName(RemoveTrailingSlash(text))+ALY_PATH_SEPARATOR;
			} else {
				this->label = std::string("..") + ALY_PATH_SEPARATOR + GetFileName(text);
			}
		}
		else {
			this->label = text;
		}
	}
}
void FileField::updateSuggestionBox(AlloyContext* context, bool forceValue) {
	showCursor = true;
	std::string root = GetParentDirectory(value);
	std::vector<std::string> listing = GetDirectoryListing(root);
	std::vector<std::string> suggestions = AutoComplete(value, listing);
	if (!autoSuggest) {
		context->removeOnTopRegion(selectionBox.get());
		selectionBox->setVisible(false);
	} else {
		if (suggestions.size() == 1 && forceValue) {
			if (IsDirectory(suggestions[0])) {
				this->setValue(
						RemoveTrailingSlash(suggestions[0]) + ALY_PATH_SEPARATOR);
			}
			else {
				this->setValue(suggestions[0]);
			}
			context->removeOnTopRegion(selectionBox.get());
			selectionBox->setVisible(false);
		}
		else {
			std::vector<std::string>& labels = selectionBox->options;
			labels.clear();
			for (std::string f : suggestions) {
				if (IsDirectory(f)) {
					labels.push_back(
							GetFileName(f) + ALY_PATH_SEPARATOR);
				}
				else {
					labels.push_back(GetFileName(f));
				}
			}
			if (labels.size() > 0) {
				context->setOnTopRegion(selectionBox.get());
				lastValue = this->getValue();
				box2px bounds = getBounds(false);
				selectionBox->pack(bounds.position,
						bounds.dimensions, context->dpmm,
						context->pixelRatio);
				selectionBox->setVisible(true);
				selectionBox->setSelectionOffset(0);
				selectionBox->setSelectedIndex(0);
			}
			else {
				context->removeOnTopRegion(selectionBox.get());
				selectionBox->setVisible(false);
			}
		}
	}
}
bool FileField::onEventHandler(AlloyContext* context, const InputEvent& e) {
	if (isVisible() && modifiable) {
		if (!context->isCursorFocused(this) || th <= 0)
			return false;
		switch (e.type) {
		case InputType::MouseButton:
			if (handleMouseInput(context, e))
				return true;
			break;
		case InputType::Character:
			handleCharacterInput(context, e);
			setValue(value);
			showTimer.reset();
			if (showTimer.get() == nullptr) {
				showTimer = std::shared_ptr<TimerTask>(
						new TimerTask([this, context] {
							updateSuggestionBox(context, false);
						}, nullptr, 500, 30));
			}
			showTimer->execute();
			break;
		case InputType::Key:
			if (e.isDown()) {
				if (e.key == GLFW_KEY_TAB) {
					updateSuggestionBox(context, true);
					break;
				} else if (e.key == GLFW_KEY_ENTER) {
					selectionBox->setVisible(false);
					showTimer.reset();
				}
			}
			handleKeyInput(context, e);
			setValue(value);
			if (e.isDown()) {
				if (e.key == GLFW_KEY_BACKSPACE) {
					showTimer.reset();
					if (showTimer.get() == nullptr) {
						showTimer = std::shared_ptr<TimerTask>(
								new TimerTask([this, context] {
									updateSuggestionBox(context, false);
								}, nullptr, 500, 30));
					}
					showTimer->execute();
				}
			}
			break;
		case InputType::Cursor:
			if (handleCursorInput(context, e))
				return true;
			break;
		case InputType::Unspecified:
			break;
		case InputType::Scroll:
			break;
		}
		segmentedPath = SplitPath(value);
	}
	return Region::onEventHandler(context, e);
}
void FileField::draw(AlloyContext* context) {
	Region::draw(context);
	float ascender, descender, lineh;
	std::vector<NVGglyphPosition> positions(value.size());
	NVGcontext* nvg = context->getNVG();
	box2px bounds = getBounds();
	float x = bounds.position.x;
	float y = bounds.position.y;
	float w = bounds.dimensions.x;
	float h = bounds.dimensions.y;
	bool f = context->isCursorFocused(this);
	if (!f && isObjectFocused() && onTextEntered) {
		onTextEntered(this);
	}
	setFocus(f);
	bool ofocus = isObjectFocused();
	if (!ofocus) {
		showDefaultLabel = true;
	}
	pixel lineWidth = borderWidth.toPixels(bounds.dimensions.y, context->dpmm.y,
			context->pixelRatio);

	auto currentTime = std::chrono::high_resolution_clock::now();
	double elapsed =
			std::chrono::duration<double>(currentTime - lastTime).count();
	if (elapsed >= 0.5f) {
		showCursor = !showCursor;
		lastTime = currentTime;
	}
	textOffsetX = x + 2.0f * lineWidth + PADDING;
	float textY = y;
	NVGpaint bg = nvgBoxGradient(nvg, x + 1, y + 3,
			std::max(0.0f, w - 2 * PADDING), std::max(0.0f, h - 2 * PADDING),
			context->theme.CORNER_RADIUS, 4,
			context->theme.LIGHTEST.toSemiTransparent(0.5f),
			context->theme.DARKEST.toSemiTransparent(0.5f));
	nvgBeginPath(nvg);
	nvgRoundedRect(nvg, x + PADDING, y + PADDING,
			std::max(0.0f, w - 2 * PADDING), std::max(0.0f, h - 2 * PADDING),
			context->theme.CORNER_RADIUS);
	nvgFillPaint(nvg, bg);
	nvgFill(nvg);
	fontSize = UnitPX(th = std::max(8.0f, h - 4 * PADDING));
	nvgFontSize(nvg, th);
	nvgFontFaceId(nvg, context->getFontHandle(FontType::Bold));
	nvgTextMetrics(nvg, &ascender, &descender, &lineh);
	box2px clipBounds = getCursorBounds();
	clipBounds.intersect(
			box2px(pixel2(x + PADDING, y),
					pixel2(std::max(0.0f, w - 2 * PADDING), h)));
	pushScissor(nvg, clipBounds.position.x, clipBounds.position.y,
			clipBounds.dimensions.x, clipBounds.dimensions.y);
	nvgTextAlign(nvg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
	positions.resize(value.size() + 1);
	nvgTextGlyphPositions(nvg, 0, textY, value.data(),
			value.data() + value.size(), positions.data(),
			(int) positions.size());

	float fwidth = (w - 3.0f * PADDING - 2.0f * lineWidth);
	if (cursorStart > 0) {
		if (positions[cursorStart - 1].maxx - positions[textStart].minx
				> fwidth) {
			while (positions[cursorStart - 1].maxx
					> positions[textStart].minx + fwidth) {
				if (textStart >= (int) positions.size() - 1)
					break;
				textStart++;
			}
		}
	}
	if (!showDefaultLabel) {
		textOffsetX = textOffsetX - positions[textStart].minx;
	}

	if (cursorEnd != cursorStart && ofocus) {
		int lo = std::min(cursorEnd, cursorStart);
		int hi = std::max(cursorEnd, cursorStart);
		float x0 = textOffsetX + (lo ? positions[lo - 1].maxx - 1 : 0);
		float x1 = textOffsetX + (hi ? positions[hi - 1].maxx - 1 : 0);
		nvgBeginPath(nvg);
		nvgRect(nvg, x0, textY + (h - lineh) / 2 + PADDING, x1 - x0,
				lineh - 2 * PADDING);
		nvgFillColor(nvg,
				ofocus ?
						context->theme.DARK.toSemiTransparent(0.5f) :
						context->theme.DARK.toSemiTransparent(0.25f));
		nvgFill(nvg);
	}
	if (showDefaultLabel) {
		nvgFillColor(nvg, context->theme.DARK);
		nvgText(nvg, textOffsetX, textY + h / 2, label.c_str(), NULL);
	} else {
		float xOffset = textOffsetX;
		std::stringstream path;
		for (std::string comp : segmentedPath) {
			path << comp;
			if (comp == ALY_PATH_SEPARATOR) {
				nvgFillColor(nvg, context->theme.DARK);
			}
			else {
				if (FileExists(path.str())) {
					nvgFillColor(nvg, context->theme.LINK);
				}
				else {
					nvgFillColor(nvg, context->theme.DARK);
				}
			}
			nvgText(nvg, xOffset, textY + h / 2, comp.c_str(), NULL);
			xOffset += nvgTextBounds(nvg, 0, textY, comp.c_str(), nullptr,
					nullptr);
		}
	}

	if (ofocus && showCursor) {
		nvgBeginPath(nvg);
		float xOffset = textOffsetX
				+ (cursorStart ? positions[cursorStart - 1].maxx - 1 : 0);
		nvgMoveTo(nvg, xOffset, textY + h / 2 - lineh / 2 + PADDING);
		nvgLineTo(nvg, xOffset, textY + h / 2 + lineh / 2 - PADDING);
		nvgStrokeWidth(nvg, 1.0f);
		nvgLineCap(nvg, NVG_ROUND);
		nvgStrokeColor(nvg, context->theme.DARKEST);
		nvgStroke(nvg);
	}
	popScissor(nvg);
	if (ofocus) {
		const int PAD = 2.0f;
		nvgBeginPath(nvg);
		nvgLineJoin(nvg, NVG_MITER);
		nvgRoundedRect(nvg, bounds.position.x + PAD, bounds.position.y + PAD,
				bounds.dimensions.x - 2 * PAD, bounds.dimensions.y - 2 * PAD,
				context->theme.CORNER_RADIUS);
		nvgStrokeWidth(nvg, (float) PAD);
		nvgStrokeColor(nvg, context->theme.FOCUS);
		nvgStroke(nvg);
	}
	if (!isObjectFocused() && value.size() == 0) {
		showDefaultLabel = true;
	}
}
void FileSelector::setValue(const std::string& file) {
	fileLocation->setValue(file);
	fileDialog->setValue(file);
}
void FileSelector::addFileExtensionRule(const std::string& name,
		const std::string& extension) {
	fileDialog->addFileExtensionRule(name, extension);
}
void FileSelector::addFileExtensionRule(const FileFilterRule& rule) {
	fileDialog->addFileExtensionRule(rule);
}
void FileSelector::addFileExtensionRule(const std::string& name,
		const std::initializer_list<std::string>& extension) {
	fileDialog->addFileExtensionRule(name, extension);
}

void FileSelector::setFileExtensionRule(int index) {
	fileDialog->setFileExtensionRule(index);
}
std::string FileSelector::getValue() {
	return fileLocation->getValue();
}
void FileSelector::openFileDialog(AlloyContext* context,
		const std::string& workingDirectory) {
	if (!fileDialog->isVisible()) {
		fileDialog->setVisible(true);
		context->getGlassPane()->setVisible(true);
	} else {
		fileDialog->setVisible(false);
		context->getGlassPane()->setVisible(false);
	}
	fileDialog->setValue(workingDirectory);
}
void FileDialog::updateDirectoryList() {
	setSelectedFile(fileLocation->getValue());
}
bool FileDialog::updateValidity() {
	if (type == FileDialogType::SelectDirectory) {
		std::string file = fileLocation->getValue();
		if (IsDirectory(file)) {
			valid = true;
			actionButton->backgroundColor = MakeColor(
					AlloyApplicationContext()->theme.LIGHTER);
		} else {
			actionButton->backgroundColor = MakeColor(
					AlloyApplicationContext()->theme.LIGHTER.toDarker(0.5f));
			valid = false;
		}
	} else if (type == FileDialogType::SelectMultiDirectory) {
		std::string file = fileLocation->getValue();
		valid = true;
		int count = 0;
		for (std::shared_ptr<ListEntry> entry : directoryList->getEntries()) {
			if (entry->isSelected()) {
				count++;
				std::string file =
						dynamic_cast<FileEntry*>(entry.get())->fileDescription.fileLocation;
				if (!IsDirectory(file)) {
					valid = false;
					break;
				}
			}
		}
		valid &= (count > 0);
		if (valid) {
			actionButton->backgroundColor = MakeColor(
					AlloyApplicationContext()->theme.LIGHTER);
		} else {
			actionButton->backgroundColor = MakeColor(
					AlloyApplicationContext()->theme.LIGHTER.toDarker(0.5f));
		}
	} else {
		FileFilterRule* rule =
				(fileTypeSelect->getSelectedIndex() >= 0) ?
						filterRules[fileTypeSelect->getSelectedIndex()].get() :
						nullptr;
		if (type == FileDialogType::SaveFile) {
			std::string file = fileLocation->getValue();
			std::string fileName = GetFileName(file);
			if (fileName.size() > 0 && !IsDirectory(file)
					&& (rule == nullptr || rule->accept(file))) {
				valid = true;
				actionButton->backgroundColor = MakeColor(
						AlloyApplicationContext()->theme.LIGHTER);
			} else {
				actionButton->backgroundColor = MakeColor(
						AlloyApplicationContext()->theme.LIGHTER.toDarker(
								0.5f));
				valid = false;
			}
		} else if (type == FileDialogType::OpenFile) {
			std::string file = fileLocation->getValue();
			if (FileExists(file) && IsFile(file)
					&& (rule == nullptr || rule->accept(file))) {
				valid = true;
				actionButton->backgroundColor = MakeColor(
						AlloyApplicationContext()->theme.LIGHTER);
			} else {
				actionButton->backgroundColor = MakeColor(
						AlloyApplicationContext()->theme.LIGHTER.toDarker(
								0.5f));
				valid = false;
			}
		} else if (type == FileDialogType::OpenMultiFile) {
			valid = true;
			int count = 0;
			for (std::shared_ptr<ListEntry> entry : directoryList->getEntries()) {
				if (entry->isSelected()) {
					count++;
					std::string file =
							dynamic_cast<FileEntry*>(entry.get())->fileDescription.fileLocation;
					if (FileExists(file) && IsFile(file)
							&& (rule == nullptr || rule->accept(file))) {
					} else {
						valid = false;
						break;
					}
				}
			}
			valid &= (count > 0);
			if (valid) {
				actionButton->backgroundColor = MakeColor(
						AlloyApplicationContext()->theme.LIGHTER);
			} else {
				actionButton->backgroundColor = MakeColor(
						AlloyApplicationContext()->theme.LIGHTER.toDarker(
								0.5f));
			}
		}
	}
	return valid;
}
void FileButton::addFileExtensionRule(const std::string& name,
		const std::string& extension) {
	fileDialog->addFileExtensionRule(name, extension);
}
void FileButton::addFileExtensionRule(const std::string& name,
		const std::initializer_list<std::string>& extension) {
	fileDialog->addFileExtensionRule(name, extension);
}
void FileButton::addFileExtensionRule(const FileFilterRule& rule) {
	fileDialog->addFileExtensionRule(rule);
}
void FileButton::setFileExtensionRule(int index) {
	fileDialog->setFileExtensionRule(index);
}
std::string FileButton::getValue() {
	return fileDialog->getValue();
}
FileButton::FileButton(const std::string& name, const AUnit2D& pos,
		const AUnit2D& dims, const FileDialogType& type) :
		IconButton((type == FileDialogType::SaveFile) ? 0xF0C7 : 0xf115, pos,
				dims) {

	std::shared_ptr<Composite> &glassPanel =
			AlloyApplicationContext()->getGlassPane();
	fileDialog = std::shared_ptr<FileDialog>(
			new FileDialog("File Dialog",
					CoordPerPX(0.5, 0.5, -350 - 15, -250 - 15),
					CoordPX(700 + 30, 500 + 30), type));
	fileDialog->setVisible(false);
	glassPanel->add(fileDialog);

	if (type == FileDialogType::SaveFile) {
		fileDialog->onSelect = [this](const std::vector<std::string>& file) {
			if (onSave)onSave(file.front());
		};
	} else {
		fileDialog->onSelect = [this](const std::vector<std::string>& file) {
			if (onOpen)onOpen(file);
		};
	}
	foregroundColor = MakeColor(AlloyApplicationContext()->theme.LIGHTER);
	iconColor = MakeColor(AlloyApplicationContext()->theme.DARK);
	borderColor = MakeColor(0, 0, 0, 0);
	borderWidth = UnitPX(2.0f);
	backgroundColor = MakeColor(COLOR_NONE);
	setRoundCorners(true);
	onMouseDown = [this](AlloyContext* context, const InputEvent& event) {
		if (event.button == GLFW_MOUSE_BUTTON_LEFT) {
			std::string file = getValue();
			AlloyApplicationContext()->setCursorFocus(nullptr);
			if (FileExists(file)) {
				openFileDialog(context, file);
			}
			else {
				std::string parent = GetParentDirectory(file);
				if (FileExists(parent)) {
					openFileDialog(context, parent);
				} else {
					openFileDialog(context, GetCurrentWorkingDirectory());
				}
			}
			return true;
		}
		return false;
	};
}
void FileButton::setValue(const std::string& file) {

	fileDialog->setValue(file);
}
void FileButton::openFileDialog(AlloyContext* context,
		const std::string& workingDirectory) {
	if (!fileDialog->isVisible()) {
		fileDialog->setVisible(true);
		context->getGlassPane()->setVisible(true);
	} else {
		fileDialog->setVisible(false);
		context->getGlassPane()->setVisible(false);
	}
	fileDialog->setValue(workingDirectory);
}
void FileDialog::setSelectedFile(const std::string& file,
		bool changeDirectory) {
	std::string dir;
	bool select = false;
	newFolderField->setVisible(false);
	if (changeDirectory) {
		if (IsDirectory(file)) {
			dir = file;
		} else {
			dir = RemoveTrailingSlash(GetParentDirectory(file));
			select = true;
		}
	} else {
		dir = RemoveTrailingSlash(GetParentDirectory(file));
		select = true;
	}
	std::vector<FileDescription> descriptions = GetDirectoryDescriptionListing(
			dir);
	int i = 0;
	if (type == FileDialogType::SelectDirectory
			|| type == FileDialogType::SelectMultiDirectory) {
		if (!AlloyApplicationContext()->hasDeferredTasks()) {
			if (dir != lastDirectory) {
				directoryList->clearEntries();
				//Fixes bug in padding out entry width.
				AlloyApplicationContext()->getGlassPane()->pack();
				for (FileDescription& fd : descriptions) {
					if (!aly::IsDirectory(fd.fileLocation)) {
						continue;
					}
					FileEntry* entry = new FileEntry(this,
							MakeString() << "File Entry " << i,
							fileEntryHeight);
					directoryList->addEntry(std::shared_ptr<FileEntry>(entry));
					entry->setValue(fd);
					if (select && entry->fileDescription.fileLocation == file) {
						entry->setSelected(true);
					}
					i++;
				}
			}
			lastDirectory = dir;
			updateValidity();
		}
	} else {
		FileFilterRule* rule =
				(fileTypeSelect->getSelectedIndex() >= 0) ?
						filterRules[fileTypeSelect->getSelectedIndex()].get() :
						nullptr;
		if (!AlloyApplicationContext()->hasDeferredTasks()) {
			if (dir != lastDirectory) {
				directoryList->clearEntries();
				//Fixes bug in padding out entry width.
				AlloyApplicationContext()->getGlassPane()->pack();
				for (FileDescription& fd : descriptions) {
					if (rule != nullptr && fd.fileType == FileType::File
							&& !rule->accept(fd.fileLocation)) {
						continue;
					}
					FileEntry* entry = new FileEntry(this,
							MakeString() << "File Entry " << i,
							fileEntryHeight);
					directoryList->addEntry(std::shared_ptr<FileEntry>(entry));
					entry->setValue(fd);
					if (select && entry->fileDescription.fileLocation == file) {
						entry->setSelected(true);
					}
					i++;
				}
			}
			lastDirectory = dir;
			updateValidity();
		}
	}
}

void FileSelector::setTextColor(const AColor& c) {
	openIcon->iconColor = c;
	fileLocation->textColor = c;
	openIcon->borderColor = c;
}
void FileSelector::appendTo(TabChain& chain) {
	chain.add(fileLocation.get());
	chain.add(openIcon.get());
}
FileSelector::FileSelector(const std::string& name, const AUnit2D& pos,
		const AUnit2D& dims, bool directoryInput) :
		BorderComposite(name, pos, dims), directoryInput(directoryInput) {
	backgroundColor = MakeColor(AlloyApplicationContext()->theme.LIGHTER);
	borderColor = MakeColor(AlloyApplicationContext()->theme.LIGHTER);
	borderWidth = UnitPX(0.0f);
	setRoundCorners(true);
	std::shared_ptr<Composite> &glassPanel =
			AlloyApplicationContext()->getGlassPane();
	fileDialog = std::shared_ptr<FileDialog>(
			new FileDialog("Open File",
					CoordPerPX(0.5, 0.5, -350 - 15, -250 - 15),
					CoordPX(700 + 30, 500 + 30),
					directoryInput ?
							FileDialogType::SelectDirectory :
							FileDialogType::OpenFile));
	fileDialog->setVisible(false);
	glassPanel->add(fileDialog);
	fileLocation = std::shared_ptr<FileField>(
			new FileField("None", CoordPX(0, 0), CoordPercent(1.0f, 1.0f),
					directoryInput));
	fileLocation->borderColor = MakeColor(0, 0, 0, 0);
	fileLocation->backgroundColor = MakeColor(0, 0, 0, 0);
	fileLocation->borderWidth = UnitPX(0.0f);
	fileDialog->onSelect = [this](const std::vector<std::string>& file) {
		fileLocation->setValue(file.front());

		if (onChange)onChange(file.front());
	};
	openIcon = std::shared_ptr<IconButton>(
			new IconButton(0xf115, CoordPX(2.0f, 2.0f),
					CoordPerPX(1.0f, 1.0f, -2.0f, -2.0f)));
	openIcon->foregroundColor = MakeColor(COLOR_NONE);
	openIcon->borderColor = MakeColor(AlloyApplicationContext()->theme.DARK);
	openIcon->borderWidth = UnitPX(0.0f);
	openIcon->backgroundColor = MakeColor(COLOR_NONE);
	openIcon->setRoundCorners(true);
	openIcon->iconColor = MakeColor(AlloyApplicationContext()->theme.DARK);
	setCenter(fileLocation);
	setEast(openIcon, UnitPX(30.0f));
	openIcon->onMouseDown =
			[this](AlloyContext* context, const InputEvent& event) {
				if (event.button == GLFW_MOUSE_BUTTON_LEFT) {
					fileLocation->hideDropDown(context);
					std::string file = getValue();
					AlloyApplicationContext()->setCursorFocus(nullptr);
					if (FileExists(file)) {
						openFileDialog(context, file);
					}
					else {
						std::string parent = GetParentDirectory(file);
						if (FileExists(parent)) {
							openFileDialog(context, parent);
						}
						else {
							openFileDialog(context, GetCurrentWorkingDirectory());
						}
					}
					return true;
				}
				return false;
			};
	fileLocation->setValue(GetCurrentWorkingDirectory());
	fileLocation->onTextEntered = [this](TextField* field) {
		fileDialog->setValue(field->getValue());
		if (onChange)onChange(field->getValue());
	};
}

FileDialog::FileDialog(const std::string& name, const AUnit2D& pos,
		const AUnit2D& dims, const FileDialogType& type, pixel fileEntryHeight) :
		AdjustableComposite(name, pos, dims), type(type), fileEntryHeight(
				fileEntryHeight) {
	AdjustableComposite::setDragEnabled(true);
	cellPadding = pixel2(7, 7);
	containerRegion = std::shared_ptr<BorderComposite>(
			new BorderComposite("Container", CoordPX(15, 15),
					CoordPerPX(1.0, 1.0, -30, -30)));
	actionButton = std::shared_ptr<TextIconButton>(
			new TextIconButton(
					(type == FileDialogType::SaveFile) ?
							"Save" :
							((type == FileDialogType::SelectDirectory) ?
									"Select" : "Open"), 0xf115,
					CoordPerPX(1.0f, 0.0f, -10.0f, 5.0f), CoordPX(100, 30)));
	actionButton->setRoundCorners(true);
	actionButton->onMouseDown =
			[this](AlloyContext* context, const InputEvent& event) {
				if (event.button == GLFW_MOUSE_BUTTON_LEFT) {
					if (valid) {
						if (this->onSelect)
						{
							std::vector<std::string> files;
							if (this->type != FileDialogType::OpenMultiFile&&this->type != FileDialogType::SelectMultiDirectory) {
								files.push_back(this->getValue());
							}
							else {
								for (std::shared_ptr<ListEntry> entry : directoryList->getEntries()) {
									if (entry->isSelected()) {
										files.push_back(dynamic_cast<FileEntry*>(entry.get())->fileDescription.fileLocation);
									}
								}
							}
							if (files.size() > 0)this->onSelect(files);
						}
						this->setVisible(false);
						context->getGlassPane()->setVisible(glassState);
						return true;
					}
					else {
						return false;
					}
				}
				return false;
			};
	if (type != FileDialogType::SelectDirectory
			&& type != FileDialogType::SelectMultiDirectory) {
		fileTypeSelect = std::shared_ptr<Selection>(
				new Selection("File Type", CoordPerPX(0.0f, 0.0f, 10.0f, 5.0f),
						CoordPerPX(1.0f, 0.0f, -125.0f, 30.0f)));
		std::shared_ptr<FileFilterRule> filterRule = std::shared_ptr<
				FileFilterRule>(new FileFilterRule("All Files"));
		filterRules.push_back(filterRule);
		fileTypeSelect->addSelection(filterRule->toString());
		fileTypeSelect->setSelectedIndex(0);
		fileTypeSelect->onSelect = [this](int index) {
			this->update();
		};
	}
	actionButton->setOrigin(Origin::TopRight);
	fileLocation = std::shared_ptr<FileField>(
			new FileField("File Location", CoordPX(10, 7),
					CoordPerPX(1.0f, 0.0f, -86.0f, 30.0f)));
	fileLocation->setPreferredFieldSize(60);
	fileLocation->borderWidth = UnitPX(2.0f);
	fileLocation->borderColor = MakeColor(
			AlloyApplicationContext()->theme.DARK);
	if (type == FileDialogType::SaveFile
			|| type == FileDialogType::OpenMultiFile
			|| type == FileDialogType::SelectMultiDirectory) {
		fileLocation->setEnableAutoSugest(false);
	}
	fileLocation->backgroundColor = MakeColor(
			AlloyApplicationContext()->theme.LIGHT);

	fileLocation->onKeyInput = [this](TextField* field) {
		this->updateValidity();
	};
	fileLocation->onTextEntered = [this](TextField* field) {
		this->updateDirectoryList();

	};
	upDirButton = std::shared_ptr<IconButton>(
			new IconButton(0xf062, CoordPerPX(1.0, 0.0, -40, 7),
					CoordPX(30, 30)));
	upDirButton->foregroundColor = MakeColor(
			AlloyApplicationContext()->theme.LIGHTER);
	upDirButton->borderWidth = UnitPX(0.0f);
	upDirButton->backgroundColor = MakeColor(0, 0, 0, 0);
	upDirButton->setRoundCorners(true);
	upDirButton->iconColor = MakeColor(AlloyApplicationContext()->theme.DARK);
	upDirButton->borderColor = MakeColor(AlloyApplicationContext()->theme.DARK);
	upDirButton->onMouseDown =
			[this](AlloyContext* context, const InputEvent& event) {
				if(event.button==GLFW_MOUSE_BUTTON_LEFT) {
					std::string file=RemoveTrailingSlash(this->getValue());
					if(IsFile(file)) {
						this->setValue(GetParentDirectory(RemoveTrailingSlash(GetParentDirectory(file))));
					} else {
						this->setValue(GetParentDirectory(file));
					}
					return true;
				}
				return false;
			};

	makeDirButton = std::shared_ptr<IconButton>(
			new IconButton(0xF07B, CoordPerPX(1.0, 0.0, -73, 7),
					CoordPX(30, 30)));
	makeDirButton->foregroundColor = MakeColor(
			AlloyApplicationContext()->theme.LIGHTER);
	makeDirButton->borderWidth = UnitPX(0.0f);
	makeDirButton->backgroundColor = MakeColor(0, 0, 0, 0);
	makeDirButton->setRoundCorners(true);
	makeDirButton->iconColor = MakeColor(AlloyApplicationContext()->theme.DARK);
	makeDirButton->borderColor = MakeColor(
			AlloyApplicationContext()->theme.DARK);
	makeDirButton->onMouseDown =
			[this](AlloyContext* context, const InputEvent& event) {
				if(event.button==GLFW_MOUSE_BUTTON_LEFT) {
					newFolderField->setValue("");
					newFolderField->setFocus(true);
					newFolderField->setShowDefaultLabel(true);
					newFolderField->setVisible(true);
					return true;
				}
				return false;
			};

	newFolderField = std::shared_ptr<TextField>(
			new TextField("Folder Name",
					CoordPerPX(1.0, 0.0, -150 - 25.0f, 55.0f),
					CoordPX(150, 30)));
	newFolderField->onTextEntered = [this](TextField* field) {
		if(field->isVisible()) {
			std::string file=RemoveTrailingSlash(this->getValue());
			std::string name=field->getValue();
			std::string f;
			if(IsFile(file)) {
				f=RemoveTrailingSlash(GetParentDirectory(file));
			} else {
				f=RemoveTrailingSlash(file);
			}
			if(name.size()>0) {
				f=MakeString()<<f<<ALY_PATH_SEPARATOR<<name;
				if(MakeDirectory(f)) {
					this->setValue(RemoveTrailingSlash(f));
					this->update();
					return true;
				}
			}
			field->setVisible(false);
			newFolderField->setFocus(false);
			return true;
		} else {
			newFolderField->setFocus(false);
			return false;
		}
	};
	newFolderField->backgroundColor = MakeColor(
			AlloyApplicationContext()->theme.DARK);
	newFolderField->borderColor = MakeColor(
			AlloyApplicationContext()->theme.DARK);
	newFolderField->borderWidth = UnitPX(2.0f);
	newFolderField->textColor = MakeColor(
			AlloyApplicationContext()->theme.DARKER);
	newFolderField->setFocus(false);
	newFolderField->setRoundCorners(false);
	cancelButton = std::shared_ptr<IconButton>(
			new IconButton(0xf00d, CoordPerPX(1.0, 0.0, -30, 30),
					CoordPX(30, 30), IconType::CIRCLE));
	cancelButton->setOrigin(Origin::BottomLeft);
	cancelButton->borderColor = MakeColor(
			AlloyApplicationContext()->theme.LIGHTEST);
	cancelButton->backgroundColor = MakeColor(COLOR_NONE);
	cancelButton->onMouseDown =
			[this](AlloyContext* context, const InputEvent& event) {
				this->setVisible(false);
				context->getGlassPane()->setVisible(glassState);
				return true;
			};
	CompositePtr southRegion = MakeComposite("File Options", CoordPX(0, 0),
			CoordPercent(1.0f, 1.0f));
	CompositePtr northRegion = MakeComposite("Selection Bar", CoordPX(0, 0),
			CoordPercent(1.0f, 1.0f));
	southRegion->add(actionButton);
	if (fileTypeSelect.get() != nullptr)
		southRegion->add(fileTypeSelect);
	northRegion->add(fileLocation);
	northRegion->add(makeDirButton);
	northRegion->add(upDirButton);
	std::vector<std::string> drives = GetDrives();
	float offset = (drives.size() > 6) ? Composite::scrollBarSize + 2.0f : 2.0f;
	directoryTree = std::shared_ptr<Composite>(
			new Composite("Container", CoordPX(10, 0),
					CoordPerPX(1.0, 1.0, -10, 0)));

	TextIconButtonPtr homeDir = TextIconButtonPtr(
			new TextIconButton("Home", 0xf015, CoordPX(1.0f, 0.0f),
					CoordPerPX(1.0f, 0.0f, -offset, 30.0f),
					HorizontalAlignment::Left));
	TextIconButtonPtr docsDir = TextIconButtonPtr(
			new TextIconButton("Documents", 0xf115, CoordPX(1.0f, 0.0f),
					CoordPerPX(1.0f, 0.0f, -offset, 30.0f),
					HorizontalAlignment::Left));
	TextIconButtonPtr downloadDir = TextIconButtonPtr(
			new TextIconButton("Downloads", 0xf019, CoordPX(1.0f, 0.0f),
					CoordPerPX(1.0f, 0.0f, -offset, 30.0f),
					HorizontalAlignment::Left));
	TextIconButtonPtr desktopDir = TextIconButtonPtr(
			new TextIconButton("Desktop", 0xf108, CoordPX(1.0f, 0.0f),
					CoordPerPX(1.0f, 0.0f, -offset, 30.0f),
					HorizontalAlignment::Left));
	homeDir->setRoundCorners(true);
	docsDir->setRoundCorners(true);
	downloadDir->setRoundCorners(true);
	desktopDir->setRoundCorners(true);

	homeDir->onMouseDown = [this](AlloyContext* context, const InputEvent& e) {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
			this->setValue(GetHomeDirectory());
			return true;
		}
		return false;
	};
	directoryTree->add(homeDir);
	docsDir->onMouseDown = [this](AlloyContext* context, const InputEvent& e) {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
			this->setValue(GetDocumentsDirectory());
			return true;
		}
		return false;
	};
	directoryTree->add(docsDir);
	downloadDir->onMouseDown =
			[this](AlloyContext* context, const InputEvent& e) {
				if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
					this->setValue(GetDownloadsDirectory());
					return true;
				}
				return false;
			};
	directoryTree->add(downloadDir);
	desktopDir->onMouseDown =
			[this](AlloyContext* context, const InputEvent& e) {
				if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
					this->setValue(GetDesktopDirectory());
					return true;
				}
				return false;
			};
	directoryTree->add(desktopDir);

	for (std::string file : drives) {
		TextIconButtonPtr diskDir = TextIconButtonPtr(
				new TextIconButton(
						GetFileName(
								RemoveTrailingSlash(file)) + ALY_PATH_SEPARATOR,
						0xf0a0, CoordPX(1.0f, 0.0f),
						CoordPerPX(1.0f, 0.0f, -offset, 30.0f),
						HorizontalAlignment::Left));
		diskDir->onMouseDown =
				[this, file](AlloyContext* context, const InputEvent& e) {
					if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
						this->setValue(file);
						return true;
					}
					return false;
				};
		diskDir->setRoundCorners(true);
		directoryTree->add(diskDir);
	}
	directoryList = std::shared_ptr<ListBox>(
			new ListBox("Container", CoordPX(0, 0),
					CoordPerPX(1.0f, 1.0, -10.0f, 0.0f)));
	directoryList->setEnableMultiSelection(
			type == FileDialogType::OpenMultiFile
					|| type == FileDialogType::SelectMultiDirectory);
	directoryList->onSelect =
			[this](ListEntry* lentry, const InputEvent& e) {
				if (e.type==InputType::MouseButton&&e.clicks == 2&&this->type!=FileDialogType::SelectMultiDirectory) {
					actionButton->onMouseDown(AlloyApplicationContext().get(), e);
				}
				else {
					if (lentry != nullptr) {
						FileEntry* entry = dynamic_cast<FileEntry*>(lentry);
						if(e.type==InputType::MouseButton) {
							if (this->type == FileDialogType::OpenMultiFile||this->type==FileDialogType::SelectMultiDirectory) { //Should only happen on double click
								if (entry->fileDescription.fileType == FileType::Directory) {
									std::string fileName = entry->fileDescription.fileLocation;
									setSelectedFile(fileName,!(e.type==InputType::MouseButton&&e.clicks==1&&this->type==FileDialogType::SelectMultiDirectory));
									fileLocation->setValue(fileName);
								}
								updateValidity();
							}
							else if (this->type == FileDialogType::OpenFile||this->type == FileDialogType::SelectDirectory) {
								std::string fileName = entry->fileDescription.fileLocation;
								setSelectedFile(fileName);
								fileLocation->setValue(fileName);
								updateValidity();
							}
							else if (this->type == FileDialogType::SaveFile) {
								if (entry->fileDescription.fileType == FileType::Directory) {
									std::string fileName = entry->fileDescription.fileLocation;
									setSelectedFile(fileName);
									fileLocation->setValue(fileName);
								}
								else {
									fileLocation->setValue(entry->fileDescription.fileLocation);
								}
								updateValidity();
							}
						} else {
							updateValidity();
						}
					}
					else {
						if (this->type != FileDialogType::OpenMultiFile && this->type!=FileDialogType::SelectMultiDirectory) {
							ListEntry* entry = directoryList->getLastSelected();
							if (entry != nullptr) {
								fileLocation->setValue(
										GetParentDirectory(dynamic_cast<FileEntry*>(entry)->fileDescription.fileLocation));
							}
						}
						updateValidity();
					}
				}
			};
	this->onEvent = [this](AlloyContext* context, const InputEvent& e) {
		if(e.type==InputType::Key&&isVisible()) {
			if(e.key==GLFW_KEY_ESCAPE) {
				this->setVisible(false);
				context->getGlassPane()->setVisible(glassState);
				return true;
			}
			/*
			 if(e.key==GLFW_KEY_ENTER) {

			 if (valid) {
			 if (this->onSelect)
			 {
			 std::vector<std::string> files;
			 if (this->type != FileDialogType::OpenMultiFile&&this->type!=FileDialogType::SelectMultiDirectory) {
			 files.push_back(this->getValue());
			 }
			 else {
			 for (std::shared_ptr<ListEntry> entry : directoryList->getEntries()) {
			 if (entry->isSelected()) {
			 files.push_back(dynamic_cast<FileEntry*>(entry.get())->fileDescription.fileLocation);
			 }
			 }
			 }
			 if (files.size() > 0)this->onSelect(files);
			 }
			 this->setVisible(false);
			 context->getGlassPane()->setVisible(false);
			 return true;
			 }
			 }

			 */
		}
		return false;
	};
	directoryTree->borderColor = MakeColor(
			AlloyApplicationContext()->theme.DARK);
	directoryTree->borderWidth = UnitPX(1.0f);
	directoryTree->setOrientation(Orientation::Vertical);
	directoryTree->setScrollEnabled(true);
	containerRegion->setNorth(northRegion, UnitPX(40));
	containerRegion->setSouth(southRegion, UnitPX(40));
	containerRegion->setWest(directoryTree, UnitPX(140.0f));
	containerRegion->setCenter(directoryList);
	Application::addListener(this);
	add(containerRegion);
	add(cancelButton);
	add(newFolderField);
	appendToTabChain(fileLocation.get());
	if (fileTypeSelect.get() != nullptr)
		appendToTabChain(fileTypeSelect.get());
	appendToTabChain(actionButton.get());
}
std::string FileFilterRule::toString() {
	std::stringstream ss;
	if (extensions.size() == 0) {
		ss << name << " (*.*)";
		return ss.str();
	}
	ss << name << " (";
	int index = 0;
	for (std::string ext : extensions) {
		ss << "*." << ext;
		if (index < (int) extensions.size() - 1) {
			ss << ", ";
		}
		index++;
	}
	ss << ")";
	return ss.str();
}
bool FileFilterRule::accept(const std::string& file) {
	if (extensions.size() == 0)
		return true;
	std::string ext = GetFileExtension(file);
	for (char& c : ext) {
		c = tolower(c);
	}
	for (std::string extension : extensions) {
		if (ext == extension)
			return true;
	}
	return false;
}
bool FileDialog::acceptDragEvent(const pixel2& cursor) const {
	return dragAccept;
}
void FileDialog::addFileExtensionRule(const std::string& name,
		const std::string& extension) {
	using extensions = std::initializer_list<std::string>;
	filterRules.push_back(
			std::shared_ptr<FileFilterRule>(
					new FileFilterRule(name, extensions { extension })));
	if (fileTypeSelect.get() != nullptr)
		fileTypeSelect->addSelection(filterRules.back()->toString());
}
void FileDialog::addFileExtensionRule(const std::string& name,
		const std::initializer_list<std::string> & extension) {
	filterRules.push_back(
			std::shared_ptr<FileFilterRule>(
					new FileFilterRule(name, extension)));
	if (fileTypeSelect.get() != nullptr)
		fileTypeSelect->addSelection(filterRules.back()->toString());
}
void FileDialog::addFileExtensionRule(const FileFilterRule& rule) {
	filterRules.push_back(
			std::shared_ptr<FileFilterRule>(new FileFilterRule(rule)));
	if (fileTypeSelect.get() != nullptr)
		fileTypeSelect->addSelection(filterRules.back()->toString());
}
void FileDialog::setValue(const std::string& file) {
	fileLocation->setValue(file);
	if (isVisible())
		setSelectedFile(file);
}
std::string FileDialog::getValue() const {
	return fileLocation->getValue();
}
void FileDialog::update() {
	lastDirectory = "";
	updateDirectoryList();
}
void FileDialog::setVisible(bool v) {
	if (v) {
		glassState=AlloyApplicationContext()->getGlassPane()->isVisible();
		actionButton->setFocus(true);
	}
	AdjustableComposite::setVisible(v);
}
void FileDialog::draw(AlloyContext* context) {
	NVGcontext* nvg = context->getNVG();
	box2px bounds = this->getBounds();
	bool isOver = false;
	if (context->isMouseOver(this, false)) {
		nvgBeginPath(nvg);
		nvgRoundedRect(nvg, bounds.position.x, bounds.position.y,
				bounds.dimensions.x, bounds.dimensions.y, 15.0f);
		nvgFillColor(nvg, context->theme.LIGHT.toSemiTransparent(0.5f));
		nvgFill(nvg);
		isOver = true;
	}
	bounds = containerRegion->getBounds();

	NVGpaint shadowPaint = nvgBoxGradient(nvg, bounds.position.x,
			bounds.position.y, bounds.dimensions.x, bounds.dimensions.y,
			context->theme.CORNER_RADIUS, 8, context->theme.DARKEST,
			context->theme.LIGHTEST.toSemiTransparent(0.0f));

	nvgBeginPath(nvg);
	nvgFillPaint(nvg, shadowPaint);

	nvgRoundedRect(nvg, bounds.position.x + 2, bounds.position.y + 2,
			bounds.dimensions.x + 2, bounds.dimensions.y + 2,
			context->theme.CORNER_RADIUS);
	nvgFill(nvg);

	nvgBeginPath(nvg);
	nvgRoundedRect(nvg, bounds.position.x, bounds.position.y,
			bounds.dimensions.x, bounds.dimensions.y,
			context->theme.CORNER_RADIUS);
	nvgFillColor(nvg, context->theme.DARK);
	nvgFill(nvg);

	nvgBeginPath(nvg);
	nvgRoundedRect(nvg, bounds.position.x, bounds.position.y,
			bounds.dimensions.x, bounds.dimensions.y,
			context->theme.CORNER_RADIUS);
	pixel lineWidth = borderWidth.toPixels(bounds.dimensions.y, context->dpmm.y,
			context->pixelRatio);
	nvgStrokeWidth(nvg, lineWidth);
	nvgStrokeColor(nvg, context->theme.LIGHT);
	nvgStroke(nvg);

	AdjustableComposite::draw(context);
	if (isOver && context->getCursor() == nullptr) {
		context->setCursor(&aly::Cursor::Position);
		dragAccept = true;
	} else {
		if (!context->isMouseDown()) {
			dragAccept = false;
		}
	}
}
FileEntry::FileEntry(FileDialog* dialog, const std::string& name,
		float fontHeight) :
		ListEntry(dialog->directoryList.get(), name, fontHeight), fileDescription() {
}
void FileEntry::setValue(const FileDescription& description) {
	this->fileDescription = description;
	iconCodeString =
			(fileDescription.fileType == FileType::Directory) ?
					CodePointToUTF8(0xf07b) : CodePointToUTF8(0xf15b);
	fileSize = FormatSize(fileDescription.fileSize);
	creationTime = FormatDateAndTime(fileDescription.creationTime);
	lastAccessTime = FormatDateAndTime(fileDescription.lastModifiedTime);
	lastModifiedTime = FormatDateAndTime(fileDescription.lastModifiedTime);
	setLabel(GetFileName(fileDescription.fileLocation));
}

MultiFileEntry::MultiFileEntry(ListBox* listBox, const std::string& name,
		float fontHeight) :
		ListEntry(listBox, name, fontHeight) {
}
void MultiFileEntry::setValue(const std::string& file) {
	this->fileName = file;
}
void MultiFileSelector::clearEntries() {
	valueRegion->clearEntries();
}
void MultiFileSelector::update() {
	for (ListEntryPtr entry : valueRegion->getEntries()) {
		entry->parent = nullptr;
	}
	valueRegion->update();
}
void MultiFileSelector::appendTo(TabChain& chain) {
	chain.add(valueRegion.get());
	chain.add(openFileButton.get());
}
void MultiFileSelector::addFileExtensionRule(const std::string& name,
		const std::string& extension) {
	openFileButton->addFileExtensionRule(name, extension);
}
void MultiFileSelector::addFileExtensionRule(const std::string& name,
		const std::initializer_list<std::string>& extension) {
	openFileButton->addFileExtensionRule(name, extension);
}
void MultiFileSelector::addFileExtensionRule(const FileFilterRule& rule) {
	openFileButton->addFileExtensionRule(rule);
}
void MultiFileSelector::setFileExtensionRule(int index) {
	openFileButton->setFileExtensionRule(index);
}
void MultiFileSelector::addFiles(const std::vector<std::string>& newFiles) {
	for (std::string file : newFiles) {
		MultiFileEntryPtr entry = MultiFileEntryPtr(
				new MultiFileEntry(valueRegion.get(), GetFileName(file),
						this->entryHeight));
		entry->setValue(file);
		valueRegion->addEntry(entry);
	}
	update();
	fireEvent();
}
void MultiFileSelector::fireEvent() {
	if (onChange) {
		std::vector<std::string> files;
		for (ListEntryPtr entry : valueRegion->getEntries()) {
			MultiFileEntryPtr newEntry = std::dynamic_pointer_cast<
					MultiFileEntry>(entry);
			files.push_back(newEntry->getValue());
		}
		onChange(files);
	}
}
MultiFileSelector::MultiFileSelector(const std::string& name,
		const AUnit2D& pos, const AUnit2D& dims, bool directoryInput,
		float entryHeight) :
		Composite(name, pos, dims), directoryInput(directoryInput), entryHeight(
				entryHeight) {
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
	openFileButton = FileButtonPtr(
			new FileButton("Open Multi-File",
					CoordPerPX(1.0f, 0.0f, -entryHeight, 1.0f),
					CoordPX(entryHeight - 2, entryHeight - 2),
					(directoryInput) ?
							FileDialogType::SelectMultiDirectory :
							FileDialogType::OpenMultiFile));
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
	openFileButton->backgroundColor = MakeColor(0, 0, 0, 0);
	openFileButton->foregroundColor = MakeColor(0, 0, 0, 0);
	openFileButton->borderWidth = UnitPX(0.0f);
	openFileButton->iconColor = MakeColor(AlloyDefaultContext()->theme.DARK);
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
	Composite::add(openFileButton);
	Composite::add(upButton);
	Composite::add(downButton);
	Composite::add(eraseButton);
	Composite::add(bgRegion);
	valueRegion->onDeleteEntry =
			[this](const std::vector<ListEntryPtr>& removalList) {
				fireEvent();
			};
	eraseButton->onMouseDown =
			[this](AlloyContext* context, const InputEvent& e) {
				return valueRegion->removeAll();
			};
	upButton->onMouseDown = [this](AlloyContext* context, const InputEvent& e) {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
			std::vector<ListEntryPtr>& entries = valueRegion->getEntries();
			int N =(int) entries.size();
			for (int i = 1;i < N;i++) {
				if (entries[i]->isSelected()) {
					std::swap(entries[std::max(i-1 , 0)], entries[i]);
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
				for (int i = 0;i < N-1;i++) {
					if (entries[i]->isSelected()) {
						std::swap(entries[std::min(i +1 ,N-1)], entries[i]);
						update();
						context->requestPack();
						fireEvent();
						break;
					}
				}
				return false;
			};
	openFileButton->onOpen = [this](const std::vector<std::string>& newFiles) {
		addFiles(newFiles);
	};
}

}
