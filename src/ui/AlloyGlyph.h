/*
 * AlloyGlyph.h
 *
 *  Created on: Sep 13, 2018
 *      Author: blake
 */

#ifndef SRC_UI_ALLOYGLYPH_H_
#define SRC_UI_ALLOYGLYPH_H_
#include "ui/AlloyRegion.h"
namespace aly{
class GlyphRegion: public Region {
public:
	AColor foregroundColor = MakeColor(Theme::Default.LIGHTER);
	std::shared_ptr<Glyph> glyph;
	GlyphRegion(const std::string& name, const std::shared_ptr<Glyph>& glyph) :
			Region(name), glyph(glyph) {
		aspectRule = AspectRule::FixedHeight;
	}
	GlyphRegion(const std::string& name, const std::shared_ptr<Glyph>& glyph,
			const AUnit2D& pos, const AUnit2D& dims) :
			Region(name, pos, dims), glyph(glyph) {
		aspectRule = AspectRule::FixedHeight;
	}
	;
	virtual void drawDebug(AlloyContext* context) override;
	virtual void draw(AlloyContext* context) override;
};
template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const GlyphRegion & region) {
	ss << "Glyph Region: " << region.name << std::endl;
	if (region.glyph.get() != nullptr)
		ss << "\t" << *region.glyph << std::endl;
	ss << "\tOrigin: " << region.getOrigin() << std::endl;
	ss << "\tRelative Position: " << region.position << std::endl;
	ss << "\tRelative Dimensions: " << region.dimensions << std::endl;
	ss << "\tBounds: " << region.getBounds() << std::endl;
	ss << "\tAspect Ratio: " << region.getAspectRule() << " (" << region.getAspectRatio()
			<< ")" << std::endl;
	if (region.parent != nullptr)
		ss << "\tParent: " << region.parent->name << std::endl;
	return ss;
}

std::shared_ptr<GlyphRegion> MakeGlyphRegion(
		const std::shared_ptr<ImageGlyph>& glyph, const AUnit2D& position,
		const AUnit2D& dimensions, const AspectRule& aspectRatio =
				AspectRule::Unspecified, const Color& bgColor = COLOR_NONE,
		const Color& fgColor = COLOR_NONE,
		const Color& borderColor = COLOR_NONE, const AUnit1D& borderWidth =
				UnitPX(2));
std::shared_ptr<GlyphRegion> MakeGlyphRegion(
		const std::shared_ptr<ImageGlyph>& glyph, const std::string& name,
		const AUnit2D& position, const AUnit2D& dimensions,
		const AspectRule& aspectRatio = AspectRule::Unspecified,
		const Color& bgColor = COLOR_NONE, const Color& fgColor = COLOR_NONE,
		const Color& borderColor = COLOR_NONE, const AUnit1D& borderWidth =
				UnitPX(2));
std::shared_ptr<GlyphRegion> MakeGlyphRegion(
		const std::shared_ptr<AwesomeGlyph>& glyph, const AUnit2D& position,
		const AUnit2D& dimensions, const Color& bgColor = COLOR_NONE,
		const Color& fgColor = COLOR_NONE,
		const Color& borderColor = COLOR_NONE, const AUnit1D& borderWidth =
				UnitPX(2));
std::shared_ptr<GlyphRegion> MakeGlyphRegion(
		const std::shared_ptr<AwesomeGlyph>& glyph, const std::string& name,
		const AUnit2D& position, const AUnit2D& dimensions,
		const Color& bgColor = COLOR_NONE, const Color& fgColor = COLOR_NONE,
		const Color& borderColor = COLOR_NONE, const AUnit1D& borderWidth =
				UnitPX(2));
typedef std::shared_ptr<GlyphRegion> GlyphRegionPtr;
}
#endif /* SRC_UI_ALLOYGLYPH_H_ */
