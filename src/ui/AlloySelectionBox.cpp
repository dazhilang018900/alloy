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
#include "ui/AlloySelectionBox.h"
#include "ui/AlloyApplication.h"
#include "ui/AlloyDrawUtil.h"
namespace aly {
bool Selection::handleMouseClick(AlloyContext* context,
		const InputEvent& event) {
	if (event.button == GLFW_MOUSE_BUTTON_LEFT) {
		setFocus(true);
		if(!selectionBox->isVisible()){
			box2px bounds = getBounds(false);
			selectionBox->pack(bounds.position, bounds.dimensions, context->dpmm,
					context->pixelRatio);
			int current = getSelectedIndex();
			if (current >= 0) {
				selectionBox->setSelectedIndex(current);
				selectionBox->setSelectionOffset(current);
			} else {
				selectionBox->setSelectionOffset(0);
				selectionBox->setSelectedIndex(0);
			}
			show(context);
		} else {
			hide(context);
		}
		return true;
	} else if (event.button == GLFW_MOUSE_BUTTON_RIGHT) {
		hide(context);
		setFocus(false);
	}
	return false;
}


box2px SelectionBox::getBounds(bool includeBounds) const {
	box2px bounds = Region::getBounds(includeBounds);
	AlloyContext* context = AlloyApplicationContext().get();
	int elements =
			(maxDisplayEntries > 0) ?
					std::min(maxDisplayEntries, (int) options.size()) :
					(int) options.size();
	float entryHeight = std::min(context->getScreenHeight() / (float) elements,
			bounds.dimensions.y);
	float boxHeight = (elements) * entryHeight;
	float parentHeight =
			(parent != nullptr) ? parent->getBoundsDimensionsY() : 0.0f;
	float yOffset = std::min(bounds.position.y + boxHeight + parentHeight,
			(float) context->getScreenHeight()) - boxHeight;
	box2px bbox;

	bbox.position = pixel2(bounds.position.x, yOffset);
	bbox.dimensions = pixel2(bounds.dimensions.x, boxHeight);
	return bbox;
}
void SelectionBox::draw(AlloyContext* context) {

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
	for (std::string label : options) {
		tw = std::max(tw,
				nvgTextBounds(nvg, 0, 0, label.c_str(), nullptr, nullptr));
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
		if (context->isMouseContainedIn(bounds.position + offset,
				pixel2(bounds.dimensions.x, entryHeight))) {
			newSelectedIndex = index;
			break;
		}
		offset.y += entryHeight;
	}
	if (newSelectedIndex >= 0) {
		selectedIndex = newSelectedIndex;
	}
	offset = pixel2(0, 0);

	for (index = selectionOffset; index < N; index++) {
		std::string& label = options[index];
		if (index == selectedIndex) {
			nvgBeginPath(nvg);
			nvgRect(nvg, bounds.position.x + offset.x,
					bounds.position.y + offset.y, bounds.dimensions.x,
					entryHeight);
			nvgFillColor(nvg, context->theme.NEUTRAL);
			nvgFill(nvg);
			nvgFillColor(nvg, *textAltColor);
		} else {
			nvgFillColor(nvg, *textColor);
		}
		pushScissor(nvg, sbounds);
		nvgText(nvg,
				bounds.position.x + offset.x + lineWidth + TextField::PADDING,
				bounds.position.y + entryHeight / 2 + offset.y, label.c_str(),
				nullptr);
		popScissor(nvg);
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

}
std::string SelectionBox::getSelection(int index) {
	return (index >= 0 && index < (int) options.size()) ? options[index] : name;
}
void SelectionBox::setSelectedIndex(int index) {
	selectedIndex = index;
	if (index < 0) {
		label = name;
		selectionOffset = 0;
	} else {
		if (selectedIndex < (int) options.size()) {
			label = options[selectedIndex];
		} else {
			label = getName();
		}

	}
}
SelectionBox::SelectionBox(const std::string& name,
		const std::vector<std::string>& labels) :
		SelectionBox(name, CoordPX(0.0f, 0.0f), CoordPercent(1.0f, 1.0f),
				labels) {

}
void SelectionBox::setSelectionOffset(int offset) {
	selectionOffset = aly::clamp(offset, 0,
			std::max((int) options.size() - maxDisplayEntries, 0));
}
SelectionBox::SelectionBox(const std::string& name, const AUnit2D& pos,
		const AUnit2D& dims, const std::vector<std::string>& labels) :
		Region(name, pos, dims), label(name), options(labels) {

	downArrow = AlloyApplicationContext()->createAwesomeGlyph(0xf0ab,
			FontStyle::Normal, 14);
	upArrow = AlloyApplicationContext()->createAwesomeGlyph(0xf0aa,
			FontStyle::Normal, 14);
	onEvent =
			[this](AlloyContext* context, const InputEvent& event) {
				if (context->isOnTop(this) && this->isVisible()) {
					if (event.type == InputType::Key&&event.isDown()) {
						if (event.key == GLFW_KEY_UP) {
							if (selectedIndex < 0) {
								this->setSelectedIndex((int)options.size() - 1);
							}
							else {
								this->setSelectedIndex(std::max(0, selectedIndex - 1));
							}
							if (maxDisplayEntries >= 0 && selectedIndex>0 && (selectedIndex < selectionOffset || selectedIndex >= selectionOffset + maxDisplayEntries)) {
								selectionOffset = std::max(0, selectedIndex + 1 - maxDisplayEntries);
							}
							return true;
						}
						else if (event.key == GLFW_KEY_DOWN) {
							if (selectedIndex < 0) {
								this->setSelectedIndex(0);
							}
							else {
								this->setSelectedIndex(std::min((int)options.size() - 1, selectedIndex + 1));
							}
							if (maxDisplayEntries >= 0 && selectedIndex>0 && (selectedIndex < selectionOffset || selectedIndex >= selectionOffset + maxDisplayEntries)) {
								selectionOffset = std::max(0, selectedIndex + 1 - maxDisplayEntries);
							}
							return true;
						}
						else if (event.key == GLFW_KEY_PAGE_UP) {
							if (maxDisplayEntries > 0) {
								selectionOffset = 0;
								scrollingUp = false;
								return true;
							}
						}
						else if (event.key == GLFW_KEY_PAGE_DOWN) {
							if (maxDisplayEntries > 0) {
								selectionOffset = (int)options.size() - maxDisplayEntries;
								scrollingDown = false;
								return true;
							}
						}
						else if (event.key == GLFW_KEY_ESCAPE) {
							setSelectedIndex(-1);
							AlloyApplicationContext()->removeOnTopRegion(this);
							this->setVisible(false);
						}
						else if (event.key == GLFW_KEY_ENTER) {
							if (selectedIndex >= 0) {
								if (this->onSelect) {
									return this->onSelect(this);
								}
							}
						}
					}
					else if (event.type == InputType::MouseButton&&event.isDown() && event.button == GLFW_MOUSE_BUTTON_LEFT) {
						if (AlloyApplicationContext()->isMouseOver(this)) {
							if (selectedIndex >= 0) {
								if (this->onSelect) {
									return this->onSelect(this);
								}
							}
							return true;
						}
						else {
							setSelectedIndex(-1);
							AlloyApplicationContext()->removeOnTopRegion(this);
							this->setVisible(false);
						}
					}
					else if (event.type == InputType::MouseButton&&event.isDown() && event.button == GLFW_MOUSE_BUTTON_RIGHT) {
						setSelectedIndex(-1);
						AlloyApplicationContext()->removeOnTopRegion(this);
						this->setVisible(false);
					}
					else if (event.type == InputType::Scroll) {
						if (maxDisplayEntries >= 0) {
							if ((int)options.size() > maxDisplayEntries) {
								if (downTimer.get() != nullptr) {
									scrollingDown = false;
									downTimer.reset();
								}
								if (upTimer.get() != nullptr) {
									scrollingUp = false;
									upTimer.reset();
								}
								selectionOffset = aly::clamp(selectionOffset - (int)event.scroll.y, 0, (int)options.size() - maxDisplayEntries);
								return true;
							}
						}
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
												}, nullptr, 500, 30));
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
												}, nullptr, 500, 30));
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
				}

				return false;
			};
	Application::addListener(this);
}
bool Selection::onEventHandler(AlloyContext* context, const InputEvent& event){
	if(event.type==InputType::Key&&event.isDown()){
		if(isObjectFocused()&&!selectionBox->isVisible()){
			if(event.key==GLFW_KEY_UP){
				int sz=getSelectionSize();
				if(sz>0){
					setSelectedIndex((getSelectedIndex()-1+sz)%sz);
					return true;
				}
			} else if(event.key==GLFW_KEY_DOWN){
				setSelectedIndex((getSelectedIndex()+1)%getSelectionSize());
				return true;
			}
		}
	}
	return Composite::onEventHandler(context,event);
}
void Selection::hide(AlloyContext* context) {
	context->removeOnTopRegion(selectionBox.get());
	selectionBox->setVisible(false);
}
void Selection::show(AlloyContext* context) {
	context->setOnTopRegion(selectionBox.get());
	selectionBox->setVisible(true);
}
void Selection::setTextColor(const AColor& c) {
	selectionLabel->textColor = c;
	selectionLabel->textAltColor = c;
	arrowLabel->textColor = c;
	arrowLabel->textAltColor = c;
}
Selection::Selection(const std::string& label, const AUnit2D& position,
		const AUnit2D& dimensions, const std::vector<std::string>& options) :
		Composite(label), selectedIndex(-1) {
	this->position = position;
	this->dimensions = dimensions;
	borderColor = MakeColor(AlloyApplicationContext()->theme.NEUTRAL);
	borderWidth=UnitPX(0.0f);
	backgroundColor = MakeColor(AlloyApplicationContext()->theme.LIGHTER);
	setRoundCorners(true);
	CompositePtr valueContainer = MakeComposite(label,
			CoordPerPX(0.0f, 0.0f, 5.0f, 5.0f),
			CoordPerPX(1.0f, 1.0f, -10.0f, -10.0f));
	selectionLabel = MakeTextLabel(label, CoordPercent(0.0f, 0.0f),
			CoordPercent(1.0f, 1.0f), FontType::Bold, UnitPercent(1.0f),
			AlloyApplicationContext()->theme.DARK.toRGBA(),
			HorizontalAlignment::Left, VerticalAlignment::Middle);
	arrowLabel = MakeTextLabel(CodePointToUTF8(0xf13a),
			CoordPercent(1.0f, 0.0f), CoordPercent(0.0f, 1.0f), FontType::Icon,
			UnitPercent(1.0f), AlloyApplicationContext()->theme.DARK.toRGBA(),
			HorizontalAlignment::Center, VerticalAlignment::Middle);
	selectionBox = SelectionBoxPtr(new SelectionBox(label, options));
	selectionBox->onRemoveFromOnTop=[this](){
		selectionBox->setVisible(false);
	};
	selectionBox->setDetached(true);
	selectionBox->setVisible(false);
	selectionBox->position = CoordPercent(0.0f, 0.0f);
	selectionBox->dimensions = CoordPercent(1.0f, 0.8f);
	selectionBox->backgroundColor = MakeColor(
			AlloyApplicationContext()->theme.DARK);
	selectionBox->borderColor = MakeColor(
			AlloyApplicationContext()->theme.LIGHTER);
	selectionBox->borderWidth = UnitPX(1.0f);
	selectionBox->textColor = MakeColor(
			AlloyApplicationContext()->theme.LIGHTER);
	selectionBox->textAltColor = MakeColor(
			AlloyApplicationContext()->theme.LIGHTER);
	arrowLabel->setAspectRatio(1.0f);
	arrowLabel->setOrigin(Origin::TopRight);
	arrowLabel->setAspectRule(AspectRule::FixedHeight);
	valueContainer->add(selectionLabel);
	valueContainer->add(arrowLabel);
	add(valueContainer);
	add(selectionBox);

	Region::onMouseDown =
			[this](AlloyContext* context, const InputEvent& event) {
				return handleMouseClick(context, event);
			};
	selectionLabel->onMouseDown =
			[this](AlloyContext* context, const InputEvent& event) {
				return handleMouseClick(context, event);
			};
	arrowLabel->onMouseDown =
			[this](AlloyContext* context, const InputEvent& event) {
				return handleMouseClick(context, event);
			};
	selectionBox->onMouseUp =
			[this](AlloyContext* context, const InputEvent& event) {
		/*
				if (event.button == GLFW_MOUSE_BUTTON_LEFT) {
					hide(context);
					int newSelection = selectionBox->getSelectedIndex();
					if (newSelection < 0) {
						selectionBox->setSelectedIndex(selectedIndex);
					}
					else {
						selectedIndex = selectionBox->getSelectedIndex();
						selectionBox->setSelectedIndex(selectedIndex);

					}
					if (selectionBox->onSelect) {
						selectionBox->onSelect(selectionBox.get());
					}
					selectionLabel->setLabel( this->getValue());
					return true;
				}
				else
				*/
				if (event.button == GLFW_MOUSE_BUTTON_RIGHT) {
					hide(context);
				}
				return false;
			};
	selectionBox->onSelect = [this](SelectionBox* box) {
		AlloyApplicationContext()->removeOnTopRegion(selectionBox.get());
		selectionBox->setVisible(false);
		int newSelection = selectionBox->getSelectedIndex();
		if (newSelection < 0) {
			selectionBox->setSelectedIndex(selectedIndex);
		}
		else {
			selectedIndex = selectionBox->getSelectedIndex();
			selectionBox->setSelectedIndex(selectedIndex);
		}
		selectionLabel->setLabel( this->getValue());
		if (this->onSelect) {
			this->onSelect(selectedIndex);
		}
		return true;
	};
	Application::addListener(this);
}
void Selection::setValue(int selection) {
	selectedIndex = selection;
	selectionBox->setSelectedIndex(selection);
	selectionLabel->setLabel(this->getValue());
	if (onSelect)
		onSelect(selectedIndex);
}
void Selection::addSelection(const std::string& selection) {
	selectionBox->addSelection(selection);
}
size_t Selection::getSelectionSize() const {
	return selectionBox->getSelectionSize();
}
void Selection::setSelectedIndex(int selection) {
	selectedIndex = selection;
	selectionBox->setSelectedIndex(selection);
	selectionLabel->setLabel(this->getValue());
	if (onSelect)
		onSelect(selectedIndex);
}
void Selection::draw(AlloyContext* context) {
	bool hover = context->isMouseContainedIn(this);
	if(hover){
		context->setCursor(&Cursor::Normal);
	}
	//if (!hover && selectionBox->isVisible()&& !context->isLeftMouseButtonDown()) {
	//	hide(context);
	//}
	Composite::draw(context);
	box2px bounds = getBounds();
	NVGcontext* nvg=context->nvgContext;
	const int PAD = 1.0f;
	if (isObjectFocused()) {
		nvgLineJoin(nvg, NVG_MITER);
		nvgBeginPath(nvg);
			if (roundCorners) {
				nvgRoundedRect(nvg, bounds.position.x + PAD,
						bounds.position.y  + PAD,
						bounds.dimensions.x - 2 * PAD,
						bounds.dimensions.y - 2 * PAD,
						context->theme.CORNER_RADIUS);
			} else {
				nvgRect(nvg, bounds.position.x  + PAD,
						bounds.position.y + PAD,
						bounds.dimensions.x - 2 * PAD,
						bounds.dimensions.y - 2 * PAD);
			}
		nvgStrokeWidth(nvg, 2.0f);
		nvgStrokeColor(nvg, context->theme.FOCUS);
		nvgStroke(nvg);
	}
}


void FilterBox::setTextColor(const AColor& c) {
	filterIcon->iconColor = c;
	filterField->textColor = c;
	filterIcon->borderColor = c;
}
FilterBox::FilterBox(const std::string& name, const AUnit2D& pos,
		const AUnit2D& dims) :
		BorderComposite(name, pos, dims) {
	backgroundColor = MakeColor(AlloyApplicationContext()->theme.LIGHTER);
	borderColor = MakeColor(AlloyApplicationContext()->theme.LIGHTER);
	borderWidth = UnitPX(0.0f);
	setRoundCorners(true);
	filterField = std::shared_ptr<TextField>(
			new TextField("Filter", CoordPX(0, 0), CoordPercent(1.0f, 1.0f)));
	filterField->borderColor = MakeColor(0, 0, 0, 0);
	filterField->borderWidth = UnitPX(0.0f);
	filterField->backgroundColor = MakeColor(0, 0, 0, 0);
	filterField->textColor = MakeColor(AlloyApplicationContext()->theme.DARK);
	filterField->setValue("");
	filterIcon = std::shared_ptr<IconButton>(
			new IconButton(0xf0b0, CoordPX(0.0f, 2.0f),
					CoordPerPX(1.0f, 1.0f, -2.0f, -4.0f)));
	filterIcon->foregroundColor = MakeColor(COLOR_NONE);
	filterIcon->borderColor = MakeColor(AlloyApplicationContext()->theme.DARK);
	filterIcon->borderWidth = UnitPX(0.0f);
	filterIcon->backgroundColor = MakeColor(COLOR_NONE);
	filterIcon->iconColor = MakeColor(AlloyApplicationContext()->theme.DARK);
	setCenter(filterField);
	setEast(filterIcon, UnitPX(25.0f));
	filterField->onTextEntered = [this](TextField* textField) {
		if (onChange)onChange(filterField->getValue());
	};
	filterIcon->onMouseDown =
			[this](AlloyContext* context, const InputEvent& event) {
				if (event.button == GLFW_MOUSE_BUTTON_LEFT) {
					if(onChange)onChange(filterField->getValue());
					return true;
				}
				return false;
			};
}
void FilterBox::setValue(const std::string& file) {
	filterField->setValue(file);
}
void SearchBox::setTextColor(const AColor& c) {
	searchIcon->iconColor = c;
	searchField->textColor = c;
	searchIcon->borderColor = c;
}
SearchBox::SearchBox(const std::string& name, const AUnit2D& pos,
		const AUnit2D& dims) :
		BorderComposite(name, pos, dims) {
	backgroundColor = MakeColor(AlloyApplicationContext()->theme.LIGHTER);
	borderColor = MakeColor(AlloyApplicationContext()->theme.LIGHTER);
	borderWidth = UnitPX(0.0f);
	setRoundCorners(true);
	searchField = std::shared_ptr<TextField>(
			new TextField("Search", CoordPX(0, 0), CoordPercent(1.0f, 1.0f)));
	searchField->borderColor = MakeColor(0, 0, 0, 0);
	searchField->borderWidth = UnitPX(0.0f);
	searchField->backgroundColor = MakeColor(0, 0, 0, 0);
	searchField->textColor = MakeColor(AlloyApplicationContext()->theme.DARK);
	searchField->setValue("");
	searchIcon = std::shared_ptr<IconButton>(
			new IconButton(0xf002, CoordPX(0.0f, 2.0f),
					CoordPerPX(1.0f, 1.0f, -2.0f, -4.0f)));
	searchIcon->foregroundColor = MakeColor(COLOR_NONE);
	searchIcon->borderColor = MakeColor(AlloyApplicationContext()->theme.DARK);
	searchIcon->borderWidth = UnitPX(0.0f);
	searchIcon->backgroundColor = MakeColor(COLOR_NONE);
	searchIcon->iconColor = MakeColor(AlloyApplicationContext()->theme.DARK);
	setCenter(searchField);
	setEast(searchIcon, UnitPX(30.0f));
	searchField->onTextEntered = [this](TextField* textField) {
		if (onChange)onChange(searchField->getValue());
	};
	searchIcon->onMouseDown =
			[this](AlloyContext* context, const InputEvent& event) {
				if (event.button == GLFW_MOUSE_BUTTON_LEFT) {
					if(onChange)onChange(searchField->getValue());
					return true;
				}
				return false;
			};
}
void SearchBox::setValue(const std::string& file) {
	searchField->setValue(file);
}

}

