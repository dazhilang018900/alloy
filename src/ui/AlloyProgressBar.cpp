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
	NVGpaint shadowPaint = nvgBoxGradient(nvg, x, y, //+1
			w, h, (h) / 2, FADE, context->theme.LIGHT, context->theme.DARKEST);
	nvgBeginPath(nvg);
	nvgRoundedRect(nvg, x, y, w, h, h / 2);
	nvgFillPaint(nvg, shadowPaint);
	nvgFill(nvg);

	NVGpaint gradPaint = nvgLinearGradient(nvg, x, y, x, y + h,
			context->theme.NEUTRAL, context->theme.DARK);
	pushScissor(nvg, cbounds.position.x, cbounds.position.y,
			std::min(cbounds.dimensions.x, w * value), cbounds.dimensions.y);
	nvgBeginPath(nvg);
	nvgRoundedRect(nvg, x, y, w, h, h / 2);
	nvgFillPaint(nvg, gradPaint);
	nvgFill(nvg);
	shadowPaint = nvgBoxGradient(nvg, x,
			y, //+1
			w, h, (h) / 2, FADE, context->theme.LIGHT.toSemiTransparent(0.0f),
			context->theme.DARKEST.toSemiTransparent(1.0f));
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
		nvgRoundedRect(nvg, x, y, w, h, h / 2);
		nvgStrokeWidth(nvg, 2.0f);
		nvgStrokeColor(nvg, context->theme.FOCUS);
		nvgStroke(nvg);
	}
}
bool ProgressBar::onEventHandler(AlloyContext* context, const InputEvent& event){
	if(event.type==InputType::MouseButton&&event.isDown()&&context->isMouseOver(this,true)){
		if(event.button==GLFW_MOUSE_BUTTON_LEFT){
			setFocus(true);
		} else if(event.button==GLFW_MOUSE_BUTTON_RIGHT){
			setFocus(false);
		}
	}
	return Composite::onEventHandler(context,event);
}
ProgressBar::ProgressBar(const std::string& name, const AUnit2D& pt,
		const AUnit2D& dims) :
		Composite(name, pt, dims), value(0), label(name) {
	Application::addListener(this);
}



} /* namespace aly */
