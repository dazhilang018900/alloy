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
#include "ui/AlloyNumberWidget.h"
#include "ui/AlloyApplication.h"
#include "ui/AlloyDrawUtil.h"

namespace aly {

const float NumberField::PADDING = 2;
pixel2 ModifiableNumber::getTextDimensions(AlloyContext* context) {
	NVGcontext* nvg = context->nvgContext;
	box2px bounds = getBounds();
	float th = fontSize.toPixels(bounds.dimensions.y, context->dpmm.y,
			context->pixelRatio);
	nvgFontSize(nvg, th);
	nvgFontFaceId(nvg, context->getFontHandle(fontType));
	float tw;
	if (showDefaultLabel || value.size() == 0) {
		tw = nvgTextBounds(nvg, 0, 0, label.c_str(), nullptr, nullptr);
	} else {
		tw = nvgTextBounds(nvg, 0, 0, value.c_str(), nullptr, nullptr);
	}
	return pixel2(tw, th);
}
ModifiableNumber::ModifiableNumber(const std::string& name,
		const AUnit2D& position, const AUnit2D& dimensions,
		const NumberType& type, bool modifiable) :
		NumberField(name, position, dimensions, type), truncate(false), fontType(
				FontType::Normal), fontStyle(FontStyle::Normal) {
	this->modifiable = modifiable;
	textAltColor = MakeColor(AlloyApplicationContext()->theme.DARK);
	textColor = MakeColor(AlloyApplicationContext()->theme.LIGHTER);
	setRoundCorners(false);
	fontSize = UnitPX(24);
}

void ModifiableNumber::draw(AlloyContext* context) {
	float ascender, descender, lineh;
	std::vector<NVGglyphPosition> positions(value.size());
	NVGcontext* nvg = context->nvgContext;
	box2px bounds = getBounds();
	bool hover = context->isMouseOver(this) && modifiable;
	float x = bounds.position.x;
	float y = bounds.position.y;
	float w = bounds.dimensions.x;
	float h = bounds.dimensions.y;
	bool f = context->isCursorFocused(this) && modifiable;
	std::string formatedValue;

	if (!f && isObjectFocused() && onTextEntered) {
		onTextEntered(this);
	}
	setFocus(f);
	bool ofocus = isObjectFocused();
	if (hover) {
		context->setCursor(&Cursor::TextInsert);
	}
	pixel lineWidth = borderWidth.toPixels(bounds.dimensions.y, context->dpmm.y,
			context->pixelRatio);
	if (backgroundColor->a > 0) {
		nvgBeginPath(nvg);
		if (roundCorners) {
			nvgRoundedRect(nvg, bounds.position.x + lineWidth * 0.5f,
					bounds.position.y + lineWidth * 0.5f,
					bounds.dimensions.x - lineWidth,
					bounds.dimensions.y - lineWidth,
					context->theme.CORNER_RADIUS);
		} else {
			nvgRect(nvg, bounds.position.x + lineWidth * 0.5f,
					bounds.position.y + lineWidth * 0.5f,
					bounds.dimensions.x - lineWidth,
					bounds.dimensions.y - lineWidth);
		}
		nvgFillColor(nvg, *backgroundColor);
		nvgFill(nvg);
	}

	if (ofocus) {
		nvgFillColor(nvg, context->theme.LIGHTER);
		nvgBeginPath(nvg);
		if (roundCorners) {
			nvgRoundedRect(nvg, bounds.position.x + lineWidth * 0.5f,
					bounds.position.y + lineWidth * 0.5f,
					bounds.dimensions.x - lineWidth ,
					bounds.dimensions.y - lineWidth ,
					context->theme.CORNER_RADIUS);
		} else {
			nvgRect(nvg, bounds.position.x + lineWidth * 0.5f ,
					bounds.position.y + lineWidth * 0.5f ,
					bounds.dimensions.x - lineWidth ,
					bounds.dimensions.y - lineWidth );
		}
		nvgFill(nvg);
	}
	auto currentTime = std::chrono::high_resolution_clock::now();
	double elapsed =
			std::chrono::duration<double>(currentTime - lastTime).count();
	if (elapsed >= 0.5f) {
		showCursor = !showCursor;
		lastTime = currentTime;
	}
	textOffsetX = x + 2.0f * lineWidth + PADDING;
	float textY = y;

	if (ofocus) {
		th = std::min(std::max(8.0f, h - 4 * PADDING),
				this->fontSize.toPixels(bounds.dimensions.y, context->dpmm.y,
						context->pixelRatio));
	} else {
		th = this->fontSize.toPixels(bounds.dimensions.y, context->dpmm.y,
				context->pixelRatio);
	}
	if (!valid && !showDefaultLabel && ofocus) {
		nvgBeginPath(nvg);
		nvgRect(nvg, x + PADDING, y + PADDING, w - 2 * PADDING,
				h - 2 * PADDING);
		nvgFillColor(nvg, *invalidNumberColor);
		nvgFill(nvg);

	}
	nvgFontSize(nvg, th);
	nvgFontFaceId(nvg, context->getFontHandle(fontType));
	if (ofocus) {
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
		float cursorOffset = textOffsetX
				+ (cursorStart ? positions[cursorStart - 1].maxx - 1 : 0);
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
		nvgFillColor(nvg, context->theme.DARK.toDarker(0.5f));
		if (showDefaultLabel) {
			nvgText(nvg, textOffsetX, textY + h / 2, label.c_str(), NULL);
		} else {
			nvgText(nvg, textOffsetX, textY + h / 2, value.c_str(), NULL);
		}
		if (showCursor) {
			nvgBeginPath(nvg);
			nvgMoveTo(nvg, cursorOffset, textY + h / 2 - lineh / 2 + PADDING);
			nvgLineTo(nvg, cursorOffset, textY + h / 2 + lineh / 2 - PADDING);
			nvgStrokeWidth(nvg, 1.0f);
			nvgLineCap(nvg, NVG_ROUND);
			nvgStrokeColor(nvg, context->theme.DARKEST);
			nvgStroke(nvg);
		}
		popScissor(nvg);
	} else {
		if (labelFormatter) {
			formatedValue = labelFormatter(numberValue);
		} else {
			formatedValue = this->value;
		}
		if (truncate) {
			pushScissor(nvg, getCursorBounds());
		}
		nvgTextAlign(nvg,
				static_cast<int>(horizontalAlignment)
						| static_cast<int>(verticalAlignment));
		pixel2 offset(0.0f);
		pixel2 start(0.0f);
		float tw;
		if (showDefaultLabel) {
			tw = nvgTextBounds(nvg, 0, 0, label.c_str(), nullptr, nullptr);
		} else {
			tw = nvgTextBounds(nvg, 0, 0, formatedValue.c_str(), nullptr,
					nullptr);
		}
		switch (horizontalAlignment) {
		case HorizontalAlignment::Left:
			offset.x = lineWidth;
			start.x = offset.x;
			break;
		case HorizontalAlignment::Center:
			offset.x = bounds.dimensions.x / 2;
			start.x = offset.x - tw / 2;
			break;
		case HorizontalAlignment::Right:
			offset.x = bounds.dimensions.x - lineWidth;
			start.x = offset.x - tw;
			break;
		}

		switch (verticalAlignment) {
		case VerticalAlignment::Top:
			offset.y = lineWidth;
			start.y = offset.y;
			break;
		case VerticalAlignment::Middle:
			offset.y = bounds.dimensions.y / 2;
			start.y = offset.y - th / 2;
			break;
		case VerticalAlignment::Bottom:
		case VerticalAlignment::Baseline:
			offset.y = bounds.dimensions.y - lineWidth;
			start.y = offset.y - th;
			break;
		}
		if (hover) {
			nvgLineCap(nvg, NVG_SQUARE);
			nvgStrokeWidth(nvg, (fontType == FontType::Bold) ? 2.0f : 1.0f);
			nvgStrokeColor(nvg,
					(!valid && !ofocus && !showDefaultLabel) ?
							*invalidNumberColor : *textColor);
			nvgBeginPath(nvg);
			nvgMoveTo(nvg, bounds.position.x + start.x,
					bounds.position.y + start.y + th - 2);
			nvgLineTo(nvg, bounds.position.x + start.x + tw,
					bounds.position.y + start.y + th - 2);
			nvgStroke(nvg);
		}
		if (showDefaultLabel) {
			drawText(nvg, bounds.position + offset, label, fontStyle,
					*textColor, *textAltColor);

		} else {
			drawText(nvg, bounds.position + offset, formatedValue, fontStyle,
					(!valid && !ofocus) ? *invalidNumberColor : *textColor,
					*textAltColor);

		}
		if (truncate) {
			popScissor(nvg);
		}
	}

	if (borderColor->a > 0) {
		nvgLineJoin(nvg, NVG_ROUND);
		nvgBeginPath(nvg);
		if (roundCorners) {
			nvgRoundedRect(nvg, bounds.position.x + lineWidth * 0.5f,
					bounds.position.y + lineWidth * 0.5f,
					bounds.dimensions.x - lineWidth,
					bounds.dimensions.y - lineWidth,
					context->theme.CORNER_RADIUS);
		} else {
			nvgRect(nvg, bounds.position.x + lineWidth * 0.5f,
					bounds.position.y + lineWidth * 0.5f,
					bounds.dimensions.x - lineWidth,
					bounds.dimensions.y - lineWidth);
		}
		nvgStrokeColor(nvg, *borderColor);
		nvgStrokeWidth(nvg, lineWidth);
		nvgStroke(nvg);
		nvgLineJoin(nvg, NVG_MITER);
	}
	if (!ofocus && value.size() == 0) {
		showDefaultLabel = true;
	}
	if (ofocus) {
		const float PAD = 1;
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
		nvgStrokeWidth(nvg,2.0f);
		nvgStrokeColor(nvg, context->theme.FOCUS);
		nvgStroke(nvg);
	}
}
bool NumberField::setNumberValue(const Number& val) {
	return setValue(MakeString() << val);
}
bool NumberField::validate() {
	int dotCount = 0;
	int index = 0;
	try {
		switch (numberType) {
		case NumberType::Boolean:
			if (value.length() == 1) {
				if (value[0] != '0' && value[0] != '1') {
					dotCount = 2;
				}
			} else {
				dotCount = 2;
			}
			if (dotCount < 2) {
				valid = true;
			} else {
				valid = false;
			}
			numberValue.setValue(std::stoi(value) != 0);
			break;
		case NumberType::Integer:
			for (char c : value) {
				if (index == 0 && c == '-') {
					//negative is first character, skip.
					continue;
				}
				if (!isdigit(c)) {
					//Case for any other letter. we ignore exponential notation.
					dotCount += 2;
					break;
				}
				index++;
			}
			if (dotCount < 2) {
				valid = true;
			} else {
				valid = false;
			}
			try {
				numberValue.setValue(std::stoi(value));
			} catch (...) {

			}
			break;
		case NumberType::Double:
		case NumberType::Float:
			for (char c : value) {
				if (index == 0 && c == '-') {
					//negative is first character, skip.
					continue;
				}
				if (!isdigit(c)) {
					if (c == '.') {
						//decimal is first character, must be an error.
						if (index == 0) {
							dotCount += 2;
							break;
						} else {
							dotCount++;
						}
					} else {
						//Case for any other letter. we ignore exponential notation.
						dotCount += 2;
						break;
					}
				}
				index++;
			}
			if (dotCount < 2) {
				valid = true;
			} else {
				valid = false;
			}
			try {
				if (numberType == NumberType::Float) {
					numberValue.setValue(std::stof(value));
				} else {
					numberValue.setValue(std::stod(value));
				}
			} catch (...) {

			}
			break;
		default:
			valid = false;
		}
	} catch (std::invalid_argument&) {
		valid = false;
	} catch (std::out_of_range&) {
		valid = false;
	}
	return valid;
}
bool NumberField::setValue(const std::string& text) {

	this->value = text;
	validate();
	textStart = 0;
	moveCursorTo((int) text.size());
	return valid;
}

void NumberField::erase() {
	int lo = std::min(cursorEnd, cursorStart);
	int hi = std::max(cursorEnd, cursorStart);
	if (hi != lo) {
		value.erase(value.begin() + lo, value.begin() + hi);
		validate();
	}
	cursorEnd = cursorStart = lo;
	textStart = clamp(cursorStart - 1, 0, textStart);
}

void NumberField::dragCursorTo(int index) {
	if (index < 0 || index > (int) value.size())
		throw std::runtime_error(
				MakeString() << name << ": Cursor position out of range.");
	cursorStart = index;
	textStart = clamp(cursorStart - 1, 0, textStart);
}

void NumberField::moveCursorTo(int index, bool isShiftHeld) {
	dragCursorTo(index);
	if (!isShiftHeld)
		cursorEnd = cursorStart;
}
void NumberField::clear() {
	if (numberType == NumberType::Double || numberType == NumberType::Float) {
		setValue("0.0");
	} else {
		setValue("0");
	}
}
NumberField::NumberField(const std::string& name, const NumberType& numberType) :
		Composite(name), label(name), value(""), th(0.0f), textOffsetX(0.0f), numberType(
				numberType), modifiable(true) {
	Application::addListener(this);
	modifiable = true;
	lastTime = std::chrono::high_resolution_clock::now();
	numberValue = MakeNumber(numberType, 0);
	invalidNumberColor = MakeColor(AlloyApplicationContext()->theme.INVALID);
	backgroundColor = MakeColor(AlloyApplicationContext()->theme.LIGHTER);
	borderColor = MakeColor(AlloyApplicationContext()->theme.LIGHT);
	borderWidth = UnitPX(1.0f);
	setRoundCorners(true);
	clear();
}
NumberField::NumberField(const std::string& name, const AUnit2D& position,
		const AUnit2D& dimensions, const NumberType& numberType) :
		Composite(name, position, dimensions), label(name), value(""), th(0.0f), textOffsetX(
				0.0f), numberType(numberType) {
	modifiable = true;
	Application::addListener(this);
	invalidNumberColor = MakeColor(255, 128, 128, 255);
	lastTime = std::chrono::high_resolution_clock::now();
	numberValue = MakeNumber(numberType, 0);
	backgroundColor = MakeColor(AlloyApplicationContext()->theme.LIGHTER);
	borderColor = MakeColor(AlloyApplicationContext()->theme.LIGHT);
	borderWidth = UnitPX(1.0f);
	setRoundCorners(true);
	clear();
}
void NumberField::handleCharacterInput(AlloyContext* context,
		const InputEvent& e) {
	if (e.codepoint < 128 && isprint(e.codepoint) && !e.isControlDown()) {
		erase();
		value.insert(value.begin() + cursorStart, e.codepoint);
		validate();
		showCursor = true;
		cursorEnd = ++cursorStart;
		showDefaultLabel = false;
		if (onKeyInput)
			onKeyInput(this);
	}
}
void NumberField::handleKeyInput(AlloyContext* context, const InputEvent& e) {
	showCursor = true;
	if (e.isDown()) {
		switch (e.key) {
		case GLFW_KEY_RIGHT:
			if (cursorStart < (int) value.size()) {
				moveCursorTo(cursorStart + 1, e.isShiftDown());
			} else {
				moveCursorTo((int) value.size(), e.isShiftDown());
			}
			break;
		case GLFW_KEY_LEFT:
			if (cursorStart > 0) {
				moveCursorTo(cursorStart - 1, e.isShiftDown());
			} else {
				moveCursorTo(0, e.isShiftDown());
			}
			break;
		case GLFW_KEY_END:
			moveCursorTo((int) value.size(), e.isShiftDown());
			break;
		case GLFW_KEY_HOME:
			moveCursorTo(0, e.isShiftDown());
			break;
		case GLFW_KEY_BACKSPACE:
			if (cursorEnd != cursorStart)
				erase();
			else if (cursorStart > 0) {
				moveCursorTo(cursorStart - 1);
				value.erase(value.begin() + cursorStart);
				validate();
				showDefaultLabel = false;
				if (onKeyInput)
					onKeyInput(this);
			}
			break;
		case GLFW_KEY_A:
			if (e.isControlDown()) {
				cursorEnd = 0;
				cursorStart = (int) (value.size());
			}
			break;
		case GLFW_KEY_C:
			if (e.isControlDown()) {
				glfwSetClipboardString(context->window,
						value.substr(std::min(cursorEnd, cursorStart),
								std::abs(cursorEnd - cursorStart)).c_str());
			}
			break;
		case GLFW_KEY_X:
			if (e.isControlDown()) {
				glfwSetClipboardString(context->window,
						value.substr(std::min(cursorEnd, cursorStart),
								std::abs(cursorEnd - cursorStart)).c_str());
				erase();
			}
			break;
		case GLFW_KEY_V:
			if (e.isControlDown()) {
				const char* pasteText = glfwGetClipboardString(context->window);
				if (pasteText != nullptr) {
					erase();
					value.insert(cursorStart, pasteText);
					validate();
					moveCursorTo(
							cursorStart + (int) std::string(pasteText).size(),
							e.isShiftDown());
				}
			}
			break;
		case GLFW_KEY_DELETE:
			if (cursorEnd != cursorStart)
				erase();
			else if (cursorStart < (int) value.size()) {
				value.erase(value.begin() + cursorStart);
			}
			showDefaultLabel = false;
			if (onKeyInput)
				onKeyInput(this);
			break;
		case GLFW_KEY_ENTER:
			if (onTextEntered) {
				onTextEntered(this);
			}
			setFocus(false);
			AlloyApplicationContext()->setCursorFocus(nullptr);
			break;
		}
	}
}

bool NumberField::handleMouseInput(AlloyContext* context, const InputEvent& e) {
	FontPtr fontFace = context->getFont(FontType::Bold);
	if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
		if (e.isDown()) {
			showCursor = true;
			showDefaultLabel = false;
			int shift = (int) (e.cursor.x - textOffsetX);
			int cursorPos = fontFace->getCursorPosition(value, th, shift);
			moveCursorTo(cursorPos);
			textStart = 0;
			dragging = true;
		} else {
			dragging = false;
		}
		return true;
	} else if (e.button == GLFW_MOUSE_BUTTON_RIGHT) {
		if (onTextEntered) {
			onTextEntered(this);
		}
		showCursor = false;
		int shift = (int) (e.cursor.x - textOffsetX);
		int cursorPos = fontFace->getCursorPosition(value, th, shift);
		moveCursorTo(cursorPos);
		textStart = 0;
		setFocus(false);
		AlloyApplicationContext()->setCursorFocus(nullptr);
		return true;
	}
	return false;
}
bool NumberField::handleCursorInput(AlloyContext* context,
		const InputEvent& e) {
	FontPtr fontFace = context->getFont(FontType::Bold);
	if (dragging) {
		int shift = (int) (e.cursor.x - textOffsetX);
		dragCursorTo(fontFace->getCursorPosition(value, th, shift));
		return true;
	}
	return false;
}
bool NumberField::onEventHandler(AlloyContext* context, const InputEvent& e) {
	if (isVisible() && modifiable) {
		if (!isObjectFocused() || th <= 0)
			return false;
		switch (e.type) {
		case InputType::MouseButton:
			if (handleMouseInput(context, e))
				return true;
			break;
		case InputType::Character:
			handleCharacterInput(context, e);
			break;
		case InputType::Key:
			handleKeyInput(context, e);
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
	}
	return Region::onEventHandler(context, e);
}

void NumberField::draw(AlloyContext* context) {
	float ascender, descender, lineh;
	Region::draw(context);
	std::vector<NVGglyphPosition> positions(value.size());
	NVGcontext* nvg = context->nvgContext;
	box2px bounds = getBounds();
	bool hover = context->isMouseOver(this) && modifiable;
	float x = bounds.position.x;
	float y = bounds.position.y;
	float w = bounds.dimensions.x;
	float h = bounds.dimensions.y;
	bool f = context->isCursorFocused(this) && modifiable;
	if (!f && isObjectFocused() && onTextEntered) {
		onTextEntered(this);
	}
	setFocus(f);
	bool ofocus = isObjectFocused();
	if (hover) {
		context->setCursor(&Cursor::TextInsert);
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
			context->theme.CORNER_RADIUS, 4, context->theme.LIGHTEST,
			context->theme.DARKEST);
	nvgBeginPath(nvg);
	nvgRoundedRect(nvg, x + PADDING, y + PADDING,
			std::max(0.0f, w - 2 * PADDING), std::max(0.0f, h - 2 * PADDING),
			context->theme.CORNER_RADIUS);
	nvgFillPaint(nvg, bg);
	nvgFill(nvg);
	if (!valid && !showDefaultLabel) {
		nvgBeginPath(nvg);
		nvgRoundedRect(nvg, x + PADDING, y + PADDING,
				std::max(0.0f, w - 2 * PADDING),
				std::max(0.0f, h - 2 * PADDING), context->theme.CORNER_RADIUS);
		nvgFillColor(nvg, *invalidNumberColor);
		nvgFill(nvg);

	}
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
	float cursorOffset = textOffsetX
			+ (cursorStart ? positions[cursorStart - 1].maxx - 1 : 0);
	if (cursorEnd != cursorStart && f) {
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
		nvgFillColor(nvg, textColor->toSemiTransparent(0.5f));
		nvgText(nvg, textOffsetX, textY + h / 2, label.c_str(), NULL);
	} else {
		nvgFillColor(nvg, *textColor);
		nvgText(nvg, textOffsetX, textY + h / 2, value.c_str(), NULL);
	}
	if (ofocus && showCursor) {
		nvgBeginPath(nvg);
		nvgMoveTo(nvg, cursorOffset, textY + h / 2 - lineh / 2 + PADDING);
		nvgLineTo(nvg, cursorOffset, textY + h / 2 + lineh / 2 - PADDING);
		nvgStrokeWidth(nvg, 1.0f);
		nvgLineCap(nvg, NVG_ROUND);
		nvgStrokeColor(nvg, context->theme.DARKEST);
		nvgStroke(nvg);
	}
	popScissor(nvg);
	if (ofocus) {
		const int PAD = 2.0f;
		nvgLineJoin(nvg, NVG_MITER);
		nvgBeginPath(nvg);
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

std::shared_ptr<NumberField> MakeNumberField(const std::string& name,
		const AUnit2D& position, const AUnit2D& dimensions,
		const NumberType& numberType, const Color& bgColor,
		const Color& textColor, const Color& invalidColor) {
	std::shared_ptr<NumberField> region = std::shared_ptr<NumberField>(
			new NumberField(name, numberType));
	region->position = position;
	region->dimensions = dimensions;
	region->invalidNumberColor = MakeColor(invalidColor);
	region->backgroundColor = MakeColor(bgColor);
	region->textColor = MakeColor(textColor);
	region->borderColor = MakeColor(bgColor.toDarker(0.5f));

	return region;
}

}
