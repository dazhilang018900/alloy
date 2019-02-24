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
#include "AlloyTextWidget.h"
#include "ui/AlloyApplication.h"
#include "ui/AlloyDrawUtil.h"
namespace aly {
const float TextField::PADDING = 2;
std::shared_ptr<TextLabel> MakeTextLabel(const std::string& name,
		const AUnit2D& position, const AUnit2D& dimensions,
		const FontType& fontType, const AUnit1D& fontSize,
		const Color& fontColor, const HorizontalAlignment& halign,
		const VerticalAlignment& valign) {
	std::shared_ptr<TextLabel> region = std::shared_ptr<TextLabel>(
			new TextLabel(name));
	region->position = position;
	region->dimensions = dimensions;
	region->textColor = MakeColor(fontColor);
	region->fontType = fontType;
	region->fontSize = fontSize;
	region->horizontalAlignment = halign;
	region->verticalAlignment = valign;
	return region;
}
std::shared_ptr<TextField> MakeTextField(const std::string& name,
		const AUnit2D& position, const AUnit2D& dimensions,
		const Color& bgColor, const Color& textColor,
		const std::string& value) {
	std::shared_ptr<TextField> region = std::shared_ptr<TextField>(
			new TextField(name));
	region->position = position;
	region->dimensions = dimensions;
	region->backgroundColor = MakeColor(bgColor);
	region->textColor = MakeColor(textColor);
	region->borderColor = MakeColor(bgColor.toDarker(0.5f));
	region->setValue(value);
	return region;
}
pixel2 TextLabel::getTextDimensions(AlloyContext* context) {
	NVGcontext* nvg = context->nvgContext;
	box2px bounds = getBounds();

	float th = fontSize.toPixels(bounds.dimensions.y, context->dpmm.y,
			context->pixelRatio);
	nvgFontSize(nvg, th);
	nvgFontFaceId(nvg, context->getFontHandle(fontType));
	float tw = nvgTextBounds(nvg, 0, 0, label.c_str(), nullptr, nullptr);
	return pixel2(tw, th);
}

pixel2 ModifiableLabel::getTextDimensions(AlloyContext* context) {
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
ModifiableLabel::ModifiableLabel(const std::string& name,
		const AUnit2D& position, const AUnit2D& dimensions, bool modifiable) :
		TextField(name, position, dimensions), truncate(false), fontType(
				FontType::Normal), fontStyle(FontStyle::Normal) {
	this->modifiable = modifiable;
	textAltColor = MakeColor(AlloyApplicationContext()->theme.DARK);
	textColor = MakeColor(AlloyApplicationContext()->theme.LIGHTER);
	fontSize = UnitPX(24);
	setValue(name);
	setRoundCorners(false);
}

void ModifiableLabel::draw(AlloyContext* context) {

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
					bounds.dimensions.x - lineWidth,
					bounds.dimensions.y - lineWidth,
					context->theme.CORNER_RADIUS);
		} else {
			nvgRect(nvg, bounds.position.x + lineWidth * 0.5f,
					bounds.position.y + lineWidth * 0.5f,
					bounds.dimensions.x - lineWidth,
					bounds.dimensions.y - lineWidth);
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
			tw = nvgTextBounds(nvg, 0, 0, value.c_str(), nullptr, nullptr);
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
			nvgStrokeColor(nvg, *textColor);
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
			drawText(nvg, bounds.position + offset, value, fontStyle,
					*textColor, *textAltColor);

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
		const float PAD = 1.0f;
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
}

void TextLabel::draw(AlloyContext* context) {
	NVGcontext* nvg = context->nvgContext;
	box2px bounds = getBounds();
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

	float th = fontSize.toPixels(bounds.dimensions.y, context->dpmm.y,
			context->pixelRatio);
	nvgFontSize(nvg, th);
	nvgFontFaceId(nvg, context->getFontHandle(fontType));

	nvgTextAlign(nvg,
			static_cast<int>(horizontalAlignment)
					| static_cast<int>(verticalAlignment));
	pixel2 offset(0, 0);
	switch (horizontalAlignment) {
	case HorizontalAlignment::Left:
		offset.x = lineWidth;
		break;
	case HorizontalAlignment::Center:
		offset.x = bounds.dimensions.x / 2;
		break;
	case HorizontalAlignment::Right:
		offset.x = bounds.dimensions.x - lineWidth;
		break;
	}

	switch (verticalAlignment) {
	case VerticalAlignment::Top:
		offset.y = lineWidth;
		break;
	case VerticalAlignment::Middle:
		offset.y = bounds.dimensions.y / 2;
		break;
	case VerticalAlignment::Bottom:
	case VerticalAlignment::Baseline:
		offset.y = bounds.dimensions.y - lineWidth;
		break;
	}
	if (truncate) {
		pushScissor(nvg, getCursorBounds());
	}
	drawText(nvg, bounds.position + offset, label, fontStyle, *textColor,
			*textAltColor);
	if (truncate) {
		popScissor(nvg);
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
	const int PAD = 1;
	if (isObjectFocused()) {
		nvgLineJoin(nvg, NVG_MITER);
		nvgBeginPath(nvg);

		if (roundCorners) {
			nvgRoundedRect(nvg, bounds.position.x + PAD,
					bounds.position.y + +PAD, bounds.dimensions.x - 2 * PAD,
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
}

TextLink::TextLink(const std::string& name) :
		TextLabel(name), enabled(true) {
	this->onMouseDown = [this](AlloyContext* context,const InputEvent& e) {
		if(e.button==GLFW_MOUSE_BUTTON_LEFT) {
			setFocus(true);
			if(onClick&&enabled) {
				return onClick();
			} else {
				return false;
			}
		} else if(e.button==GLFW_MOUSE_BUTTON_RIGHT) {
			setFocus(false);
			return false;
		}
	};
}
TextLink::TextLink(const std::string& name, const AUnit2D& pos,
		const AUnit2D& dims) :
		TextLabel(name, pos, dims), enabled(true) {
	this->onMouseDown = [this](AlloyContext* context,const InputEvent& e) {
		if(e.button==GLFW_MOUSE_BUTTON_LEFT) {
			setFocus(true);
			if(onClick&&enabled) {
				return onClick();
			} else {
				return false;
			}
		} else if(e.button==GLFW_MOUSE_BUTTON_RIGHT) {
			setFocus(false);
			return false;
		}
	};
}
void TextLink::draw(AlloyContext* context) {
	NVGcontext* nvg = context->nvgContext;
	box2px bounds = getBounds();
	bool hover = context->isMouseOver(this) && enabled;
	pixel lineWidth = borderWidth.toPixels(bounds.dimensions.y, context->dpmm.y,
			context->pixelRatio);
	if (hover) {
		context->setCursor(&Cursor::Hand);
	}
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

	float th = fontSize.toPixels(bounds.dimensions.y, context->dpmm.y,
			context->pixelRatio);
	nvgFontSize(nvg, th);
	nvgFontFaceId(nvg, context->getFontHandle(fontType));

	nvgTextAlign(nvg,
			static_cast<int>(horizontalAlignment)
					| static_cast<int>(verticalAlignment));
	float tw = nvgTextBounds(nvg, 0, 0, label.c_str(), nullptr, nullptr);
	pixel2 offset(0, 0);
	pixel2 start(0.0f);
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
	if (truncate) {
		pushScissor(nvg, getCursorBounds());
	}
	if (hover) {
		nvgLineCap(nvg, NVG_SQUARE);
		nvgStrokeWidth(nvg, (fontType == FontType::Bold) ? 2.0f : 1.0f);
		nvgStrokeColor(nvg, *textColor);
		nvgBeginPath(nvg);
		nvgMoveTo(nvg, bounds.position.x + start.x,
				bounds.position.y + start.y + th - 2);
		nvgLineTo(nvg, bounds.position.x + start.x + tw,
				bounds.position.y + start.y + th - 2);
		nvgStroke(nvg);
	}
	drawText(nvg, bounds.position + offset, label, fontStyle, *textColor,
			*textAltColor);
	if (truncate) {
		popScissor(nvg);
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
	const int PAD = 1;
	if (isObjectFocused()) {
		nvgLineJoin(nvg, NVG_MITER);
		nvgBeginPath(nvg);
		if (roundCorners) {
			nvgRoundedRect(nvg, bounds.position.x + PAD,
					bounds.position.y + +PAD, bounds.dimensions.x - 2 * PAD,
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
}
pixel2 TextRegion::getTextDimensions(AlloyContext* context) {
	NVGcontext* nvg = context->nvgContext;
	box2px bounds = getBounds();
	std::vector<NVGtextRow> rows(3);
	float lineh = fontSize.toPixels(bounds.dimensions.y, context->dpmm.y,
			context->pixelRatio);
	nvgFontSize(nvg, lineh);
	nvgFontFaceId(nvg, context->getFontHandle(fontType));
	float width = bounds.dimensions.x;
	const char* start = label.c_str();
	const char* end = label.c_str() + label.length();
	float x = 0, y = 0;
	float w = 0, h = lineh;
	while (int nrows = nvgTextBreakLines(nvg, start, end, width, rows.data(), 3)) {
		for (int i = 0; i < nrows; i++) {
			NVGtextRow* row = &rows[i];
			nvgBeginPath(nvg);
			nvgFill(nvg);
			nvgFillColor(nvg, nvgRGBA(255, 255, 255, 255));
			float tw = nvgTextBounds(nvg, x, y, row->start, row->end, nullptr);
			w = std::max(tw, w);
			y += lineh;
			h = std::max(h, y);
		}
		start = rows[nrows - 1].next;
	}
	return pixel2(w, h);
}
void TextRegion::draw(AlloyContext* context) {
	NVGcontext* nvg = context->nvgContext;
	box2px bounds = getBounds();
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

	float th = fontSize.toPixels(bounds.dimensions.y, context->dpmm.y,
			context->pixelRatio);
	nvgFontSize(nvg, th);
	nvgFontFaceId(nvg, context->getFontHandle(fontType));

	nvgTextAlign(nvg,
			static_cast<int>(horizontalAlignment)
					| static_cast<int>(verticalAlignment));
	pixel2 offset(0, 0);
	switch (horizontalAlignment) {
	case HorizontalAlignment::Left:
		offset.x = lineWidth;
		break;
	case HorizontalAlignment::Center:
		offset.x = bounds.dimensions.x / 2;
		break;
	case HorizontalAlignment::Right:
		offset.x = bounds.dimensions.x - lineWidth;
		break;
	}

	switch (verticalAlignment) {
	case VerticalAlignment::Top:
		offset.y = lineWidth;
		break;
	case VerticalAlignment::Middle:
		offset.y = bounds.dimensions.y / 2;
		break;
	case VerticalAlignment::Bottom:
	case VerticalAlignment::Baseline:
		offset.y = bounds.dimensions.y - lineWidth;
		break;
	}
	drawParagraph(nvg, bounds.position + offset, bounds.dimensions.x, th, label,
			fontStyle, *textColor, *textAltColor);
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

}
void TextField::setValue(const std::string& text) {
	this->value = text;
	textStart = 0;
	moveCursorTo((int) text.size());
}

void TextField::erase() {
	int lo = std::min(cursorEnd, cursorStart);
	int hi = std::max(cursorEnd, cursorStart);
	if (hi != lo) {
		value.erase(value.begin() + lo, value.begin() + hi);
	}
	cursorEnd = cursorStart = lo;
	textStart = clamp(cursorStart - 1, 0, textStart);
}

void TextField::dragCursorTo(int index) {
	if (index < 0 || index > (int) value.size())
		throw std::runtime_error(
				MakeString() << name << ": Cursor position out of range.");
	cursorStart = index;
	textStart = clamp(cursorStart - 1, 0, textStart);
}

void TextField::moveCursorTo(int index, bool isShiftHeld) {
	dragCursorTo(index);
	if (!isShiftHeld)
		cursorEnd = cursorStart;
}

void TextField::clear() {
	setValue("");
}

TextField::TextField(const std::string& name) :
		Composite(name), label(name), value(""), modifiable(true) {
	Application::addListener(this);
	modifiable = true;
	lastTime = std::chrono::high_resolution_clock::now();
	backgroundColor = MakeColor(AlloyApplicationContext()->theme.DARK);
	borderColor = MakeColor(AlloyApplicationContext()->theme.LIGHT);
	borderWidth = UnitPX(1.0f);
	setRoundCorners(true);

}
TextField::TextField(const std::string& name, const AUnit2D& position,
		const AUnit2D& dimensions) :
		Composite(name, position, dimensions), label(name), value("") {
	Application::addListener(this);
	modifiable = true;
	lastTime = std::chrono::high_resolution_clock::now();
	backgroundColor = MakeColor(AlloyApplicationContext()->theme.DARK);
	borderColor = MakeColor(AlloyApplicationContext()->theme.LIGHT);
	borderWidth = UnitPX(1.0f);
	setRoundCorners(true);
}

void TextField::handleCharacterInput(AlloyContext* context,
		const InputEvent& e) {
	if (e.codepoint < 128 && isprint(e.codepoint) && !e.isControlDown()) {
		erase();
		value.insert(value.begin() + cursorStart, e.codepoint);
		showCursor = true;
		cursorEnd = ++cursorStart;
		showDefaultLabel = false;
		if (onKeyInput)
			onKeyInput(this);
	}
}
bool TextField::handleKeyInput(AlloyContext* context, const InputEvent& e) {
	showCursor = true;
	if (e.isDown()) {
		switch (e.key) {
		case GLFW_KEY_RIGHT:
			if (cursorStart < (int) value.size()) {
				moveCursorTo(cursorStart + 1, e.isShiftDown());
			} else {
				moveCursorTo((int) value.size(), e.isShiftDown());
			}
			return true;
			break;
		case GLFW_KEY_LEFT:
			if (cursorStart > 0) {
				moveCursorTo(cursorStart - 1, e.isShiftDown());
			} else {
				moveCursorTo(0, e.isShiftDown());
			}
			return true;
			break;
		case GLFW_KEY_END:
			moveCursorTo((int) value.size(), e.isShiftDown());
			return true;
			break;
		case GLFW_KEY_HOME:
			moveCursorTo(0, e.isShiftDown());
			return true;
			break;
		case GLFW_KEY_BACKSPACE:
			if (cursorEnd != cursorStart)
				erase();
			else if (cursorStart > 0) {
				moveCursorTo(cursorStart - 1);
				value.erase(value.begin() + cursorStart);
				showDefaultLabel = false;
				if (onKeyInput)
					onKeyInput(this);
			}
			return true;
			break;
		case GLFW_KEY_A:
			if (e.isControlDown()) {
				cursorEnd = 0;
				cursorStart = (int) (value.size());
				return true;
			}
			break;
		case GLFW_KEY_C:
			if (e.isControlDown()) {
				glfwSetClipboardString(context->window,
						value.substr(std::min(cursorEnd, cursorStart),
								std::abs(cursorEnd - cursorStart)).c_str());
				return true;
			}
			break;
		case GLFW_KEY_X:
			if (e.isControlDown()) {
				glfwSetClipboardString(context->window,
						value.substr(std::min(cursorEnd, cursorStart),
								std::abs(cursorEnd - cursorStart)).c_str());
				erase();
				return true;
			}
			break;
		case GLFW_KEY_V:
			if (e.isControlDown()) {
				const char* pasteText = glfwGetClipboardString(context->window);
				if (pasteText != nullptr) {
					erase();
					value.insert(cursorStart, pasteText);
					moveCursorTo(
							cursorStart + (int) std::string(pasteText).size(),
							e.isShiftDown());
					if (onTextEntered) {
						onTextEntered(this);
					}
				}
				return true;
			}
			break;
		case GLFW_KEY_DELETE:
			if (cursorEnd != cursorStart)
				erase();
			else if (cursorStart < (int) value.size())
				value.erase(value.begin() + cursorStart);
			showDefaultLabel = false;
			if (onKeyInput)
				onKeyInput(this);
			return true;
			break;
		case GLFW_KEY_ENTER:
			if (onTextEntered) {
				onTextEntered(this);
			}
			setFocus(false);
			AlloyApplicationContext()->setCursorFocus(nullptr);
			return true;
			break;
		}
	}
	return false;
}

bool TextField::handleMouseInput(AlloyContext* context, const InputEvent& e) {
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
		setFocus(false);
		showCursor = false;
		int shift = (int) (e.cursor.x - textOffsetX);
		int cursorPos = fontFace->getCursorPosition(value, th, shift);
		moveCursorTo(cursorPos);
		textStart = 0;
		AlloyApplicationContext()->setCursorFocus(nullptr);
		return true;
	}
	return false;
}
bool TextField::handleCursorInput(AlloyContext* context, const InputEvent& e) {
	FontPtr fontFace = context->getFont(FontType::Bold);
	if (dragging) {
		int shift = (int) (e.cursor.x - textOffsetX);
		dragCursorTo(fontFace->getCursorPosition(value, th, shift));
		return true;
	}
	return false;
}
bool TextField::onEventHandler(AlloyContext* context, const InputEvent& e) {
	if (isVisible() && modifiable) {
		if (e.type == InputType::MouseButton && e.isDown()
				&& context->isMouseOver(this, false)) {
			if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
				setFocus(true);
			} else if (e.button == GLFW_MOUSE_BUTTON_RIGHT) {
				if(isObjectFocused()){
					if (onTextEntered) {
						onTextEntered(this);
					}
				}
				setFocus(false);
			}
		}
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
			return handleKeyInput(context, e);
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
void TextField::draw(AlloyContext* context) {
	float ascender, descender, lineh;
	Region::draw(context);
	std::vector<NVGglyphPosition> positions(value.size());
	NVGcontext* nvg = context->nvgContext;
	box2px bounds = getBounds();
	bool f = context->isCursorFocused(this) && modifiable;
	if (!f && isObjectFocused() && onTextEntered) {
		onTextEntered(this);
	}
	setFocus(f && modifiable);
	bool hover = context->isMouseOver(this) && modifiable;
	float x = bounds.position.x;
	float y = bounds.position.y;
	float w = bounds.dimensions.x;
	float h = bounds.dimensions.y;
	if (hover) {
		context->setCursor(&Cursor::TextInsert);
	}
	bool ofocus = isObjectFocused();
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
	float cursorOffset;
	if (!showDefaultLabel) {
		textOffsetX = textOffsetX - positions[textStart].minx;
		cursorOffset = textOffsetX
				+ (cursorStart ? positions[cursorStart - 1].maxx - 1 : 0);
	} else {
		cursorOffset = textOffsetX;
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
	if (!ofocus && value.size() == 0) {
		showDefaultLabel = true;
	}
}
}

