/*
 * AlloyGlyph.cpp
 *
 *  Created on: Sep 13, 2018
 *      Author: blake
 */

#include "ui/AlloyGlyph.h"
#include "ui/AlloyApplication.h"
#include "ui/AlloyDrawUtil.h"

namespace aly{
void GlyphRegion::drawDebug(AlloyContext* context) {
	drawBoundsLabel(context, name,
			(glyph->type == GlyphType::Awesome) ?
					context->getFontHandle(FontType::Icon) :
					context->getFontHandle(FontType::Bold));
}

void GlyphRegion::draw(AlloyContext* context) {
	NVGcontext* nvg = context->getNVG();
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
	if (glyph.get() != nullptr) {
		box2px b = bounds;
		b.position.x = bounds.position.x + lineWidth * 0.5f;
		b.position.y = bounds.position.y + lineWidth * 0.5f;
		b.dimensions.x = bounds.dimensions.x - lineWidth;
		b.dimensions.y = bounds.dimensions.y - lineWidth;
		glyph->draw(b, *foregroundColor, *backgroundColor, context);
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
}

std::shared_ptr<GlyphRegion> MakeGlyphRegion(
		const std::shared_ptr<AwesomeGlyph>& glyph, const AUnit2D& position,
		const AUnit2D& dimensions, const Color& bgColor, const Color& fgColor,
		const Color& borderColor, const AUnit1D& borderWidth) {
	std::shared_ptr<GlyphRegion> region = std::shared_ptr<GlyphRegion>(
			new GlyphRegion(glyph->name, glyph));

	region->position = position;
	region->dimensions = dimensions;
	region->backgroundColor = MakeColor(bgColor);
	region->foregroundColor = MakeColor(fgColor);
	region->borderColor = MakeColor(borderColor);
	region->borderWidth = borderWidth;
	region->setAspectRule(AspectRule::FixedHeight);
	region->setAspectRatio(glyph->width / (float) glyph->height);
	return region;
}
std::shared_ptr<GlyphRegion> MakeGlyphRegion(
		const std::shared_ptr<AwesomeGlyph>& glyph, const std::string& name,
		const AUnit2D& position, const AUnit2D& dimensions,
		const Color& bgColor, const Color& fgColor, const Color& borderColor,
		const AUnit1D& borderWidth) {
	std::shared_ptr<GlyphRegion> region = std::shared_ptr<GlyphRegion>(
			new GlyphRegion(name, glyph));

	region->position = position;
	region->dimensions = dimensions;
	region->backgroundColor = MakeColor(bgColor);
	region->foregroundColor = MakeColor(fgColor);
	region->borderColor = MakeColor(borderColor);
	region->borderWidth = borderWidth;
	region->setAspectRule(AspectRule::FixedHeight);
	region->setAspectRatio(glyph->width / (float) glyph->height);
	return region;
}
std::shared_ptr<GlyphRegion> MakeGlyphRegion(
		const std::shared_ptr<ImageGlyph>& glyph, const std::string& name,
		const AUnit2D& position, const AUnit2D& dimensions,
		const AspectRule& aspectRatio, const Color& bgColor,
		const Color& fgColor, const Color& borderColor,
		const AUnit1D& borderWidth) {
	std::shared_ptr<GlyphRegion> region = std::shared_ptr<GlyphRegion>(
			new GlyphRegion(name, glyph));

	region->position = position;
	region->dimensions = dimensions;
	region->backgroundColor = MakeColor(bgColor);
	region->foregroundColor = MakeColor(fgColor);
	region->borderColor = MakeColor(borderColor);
	region->borderWidth = borderWidth;
	region->setAspectRule(aspectRatio);
	region->setAspectRatio(glyph->width / (float) glyph->height);
	return region;
}
std::shared_ptr<GlyphRegion> MakeGlyphRegion(
		const std::shared_ptr<ImageGlyph>& glyph, const AUnit2D& position,
		const AUnit2D& dimensions, const AspectRule& aspectRatio,
		const Color& bgColor, const Color& fgColor, const Color& borderColor,
		const AUnit1D& borderWidth) {
	std::shared_ptr<GlyphRegion> region = std::shared_ptr<GlyphRegion>(
			new GlyphRegion(glyph->name, glyph));

	region->position = position;
	region->dimensions = dimensions;
	region->backgroundColor = MakeColor(bgColor);
	region->foregroundColor = MakeColor(fgColor);
	region->borderColor = MakeColor(borderColor);
	region->borderWidth = borderWidth;
	region->setAspectRule(aspectRatio);
	region->setAspectRatio(glyph->width / (float) glyph->height);
	return region;
}
}
