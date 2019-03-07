/*
 * AlloyProgressBar.cpp
 *
 *  Created on: Sep 13, 2018
 *      Author: blake
 */

#include "ui/AlloyProgressBar.h"
#include "ui/AlloyApplication.h"
#include "ui/AlloyDrawUtil.h"
namespace aly {

void ProgressBar::draw(AlloyContext* context) {
	box2px bounds = getBounds();
	NVGcontext* nvg = context->nvgContext;
	float x = bounds.position.x;

	float y = bounds.position.y;
	float w = bounds.dimensions.x;
	float h = bounds.dimensions.y;
	const float FADE = 8;
	box2px cbounds = getCursorBounds();
	pushScissor(nvg,cbounds);
	NVGpaint shadowPaint;
	nvgBeginPath(nvg);
	if (round) {
		shadowPaint = nvgBoxGradient(nvg, x, y, //+1
					w, h, (h) / 2, FADE, context->theme.LIGHT, context->theme.DARKEST);
		nvgRoundedRect(nvg, x, y, w, h, h / 2);
	} else {
		shadowPaint = nvgBoxGradient(nvg, x, y,
					w, h, 0.0f, FADE, context->theme.LIGHT, context->theme.DARKEST);
		nvgRect(nvg, x, y, w, h);
	}
	nvgFillPaint(nvg, shadowPaint);
	nvgFill(nvg);

	NVGpaint gradPaint;

	pushScissor(nvg, cbounds.position.x, cbounds.position.y,
			std::min(cbounds.dimensions.x, w * value), cbounds.dimensions.y);
	nvgBeginPath(nvg);
	gradPaint= nvgLinearGradient(nvg, x, y, x, y + h,
			context->theme.NEUTRAL, context->theme.DARK);
	if (round) {
		shadowPaint = nvgBoxGradient(nvg, x,
				y,
				w, h, (h) / 2, FADE, context->theme.LIGHT.toSemiTransparent(0.0f),
				context->theme.DARKEST.toSemiTransparent(1.0f));
		nvgRoundedRect(nvg, x, y, w, h, h / 2);
	} else {
		shadowPaint = nvgBoxGradient(nvg, x,
				y,
				w, h, 0.0f, FADE, context->theme.LIGHT.toSemiTransparent(0.0f),
				context->theme.DARKEST.toSemiTransparent(1.0f));
		nvgRect(nvg, x, y, w, h);
	}
	nvgFillPaint(nvg, gradPaint);
	nvgFill(nvg);

	nvgFillPaint(nvg, shadowPaint);
	nvgFill(nvg);
	nvgTextAlign(nvg, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
	nvgFontFaceId(nvg, context->getFontHandle(FontType::Bold));
	nvgFontSize(nvg, std::max(8.0f, h - FADE));
	drawText(nvg, pixel2(x + 0.5f * w, y + 0.5f * h), label, FontStyle::Normal,
			context->theme.LIGHTER, context->theme.DARK);
	popScissor(nvg);
	float xx = std::max(cbounds.position.x, x + w * value);
	pushScissor(nvg, xx, cbounds.position.y,
			cbounds.dimensions.x + std::max(0.0f, cbounds.position.x - xx),
			cbounds.dimensions.y);
	drawText(nvg, pixel2(x + 0.5f * w, y + 0.5f * h), label, FontStyle::Normal,
			context->theme.DARK, context->theme.LIGHTER);
	popScissor(nvg);
	if (isObjectFocused()) {
		nvgLineJoin(nvg, NVG_MITER);
		nvgBeginPath(nvg);
		if (round) {
			nvgRoundedRect(nvg, x+1, y+1, w-2, h-2, h / 2-1);
		} else {
			nvgRect(nvg, x+1, y+1, w-2, h-2);
		}
		nvgStrokeWidth(nvg, 2.0f);
		nvgStrokeColor(nvg, context->theme.FOCUS);
		nvgStroke(nvg);
	}
	popScissor(nvg);
}
bool ProgressBar::onEventHandler(AlloyContext* context,
		const InputEvent& event) {
	if (event.type == InputType::MouseButton && event.isDown()
			&& context->isMouseOver(this, true)) {
		if (event.button == GLFW_MOUSE_BUTTON_LEFT) {
			setFocus(true);
		} else if (event.button == GLFW_MOUSE_BUTTON_RIGHT) {
			setFocus(false);
		}
	}
	return Region::onEventHandler(context, event);
}
ProgressBar::ProgressBar(const std::string& name, const AUnit2D& pt,
		const AUnit2D& dims, bool round) :
		Region(name, pt, dims), value(0), label(name), round(round) {
	Application::addListener(this);
}

void ProgressCircle::draw(AlloyContext* context) {
	box2px bounds = getBounds();
	NVGcontext* nvg = context->nvgContext;
	pixel lineWidth = borderWidth.toPixels(bounds.dimensions.y, context->dpmm.y,
			context->pixelRatio);
	float x = bounds.position.x;
	float y = bounds.position.y;
	float w = bounds.dimensions.x;
	float h = bounds.dimensions.y;

	float rOuter = aly::round(std::min(w, h) / 2.0f) - 2.0f;
	float rInner = aly::round((1.0f - thickness) * rOuter) - 2.0f;
	float2 center(round(x + w * 0.5f), round(y + w * 0.5f));
	box2px cbounds = getCursorBounds();
	pushScissor(nvg, cbounds);
	nvgFontFaceId(nvg, context->getFontHandle(FontType::Bold));
	float fsz1 = rInner;
	float fsz2 = rInner * 0.3f;
	nvgTextAlign(nvg, NVG_ALIGN_TOP | NVG_ALIGN_CENTER);
	static const float FUDGE = 1.0f / std::sqrt(2.0f);
	if (fsz1 > 8.0f) {
		nvgFontSize(nvg, fsz1);
		drawText(nvg, center + pixel2(0.0f, -rInner * (0.9f * FUDGE)),
				MakeString() << (int) (aly::round(100 * value)) << "%",
				FontStyle::Normal, *textColor);
	}
	if (fsz2 > 8.0f) {
		nvgFontSize(nvg, fsz2);
		drawText(nvg, center + pixel2(0.0f, rInner * (0.8f * FUDGE) - fsz2),
				label, FontStyle::Normal, textColor->toDarker(0.8f));
	}

	nvgBeginPath(nvg);
	nvgArc(nvg, center.x, center.y, rInner, 0.0f, 2 * ALY_PI, NVG_CW);
	nvgArc(nvg, center.x, center.y, rOuter, 2 * ALY_PI, 0.0f, NVG_CCW);
	nvgFillColor(nvg, *backgroundColor);
	nvgFill(nvg);

	const float a0 = -ALY_PI * 0.5f;
	float a1 = value * ALY_PI * 2 - ALY_PI * 0.5f;
	nvgBeginPath(nvg);
	nvgArc(nvg, center.x, center.y, rInner, a0, a1, NVG_CW);
	nvgArc(nvg, center.x, center.y, rOuter, a1, a0, NVG_CCW);
	nvgClosePath(nvg);
	nvgFillColor(nvg, *foregroundColor);
	nvgFill(nvg);

	nvgBeginPath(nvg);
	nvgArc(nvg, center.x, center.y, rInner, -ALY_PI * 0.5f - 1.0f / rInner,
			-ALY_PI * 0.5f + 1.0f / rInner, NVG_CW);
	nvgArc(nvg, center.x, center.y, rOuter, -ALY_PI * 0.5f + 1.0f / rOuter,
			-ALY_PI * 0.5f - 1.0f / rOuter, NVG_CCW);
	nvgFillColor(nvg, backgroundColor->toLighter(1.25f));
	nvgFill(nvg);

	if (isObjectFocused()) {
		nvgBeginPath(nvg);
		nvgCircle(nvg, center.x, center.y, rOuter - 1.0f);
		nvgStrokeColor(nvg, context->theme.FOCUS);
		nvgStrokeWidth(nvg, 2.0f);
		nvgStroke(nvg);
		nvgBeginPath(nvg);
		nvgCircle(nvg, center.x, center.y, rInner);
		nvgStrokeColor(nvg, *borderColor);
		nvgStrokeWidth(nvg, 1.0f);
		nvgStroke(nvg);
	} else {
		nvgBeginPath(nvg);
		nvgCircle(nvg, center.x, center.y, rInner);
		nvgCircle(nvg, center.x, center.y, rOuter);
		nvgStrokeColor(nvg, *borderColor);
		nvgStrokeWidth(nvg, lineWidth);
		nvgStroke(nvg);
	}
	popScissor(nvg);
}
void ProgressCircle::setThickness(float p) {
	thickness = p;
}
bool ProgressCircle::onEventHandler(AlloyContext* context,
		const InputEvent& event) {
	if (event.type == InputType::MouseButton && event.isDown()
			&& context->isMouseOver(this, true)) {
		if (event.button == GLFW_MOUSE_BUTTON_LEFT) {
			setFocus(true);
		} else if (event.button == GLFW_MOUSE_BUTTON_RIGHT) {
			setFocus(false);
		}
	}
	return Region::onEventHandler(context, event);
}
ProgressCircle::ProgressCircle(const std::string& name, const AUnit2D& pt,
		const AUnit2D& dims) :
		Region(name, pt, dims), value(0), label(name), thickness(0.25f) {
	foregroundColor = MakeColor(AlloyTheme().LINK);
	backgroundColor = MakeColor(AlloyTheme().LIGHT);
	borderColor = MakeColor(AlloyTheme().LIGHT);
	borderWidth = UnitPX(1.0f);
	textColor = MakeColor(AlloyTheme().LIGHTEST);
	Application::addListener(this);
}

} /* namespace aly */
