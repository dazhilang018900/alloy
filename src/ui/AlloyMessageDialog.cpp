/*
 * AlloyMessageDialog.cpp
 *
 *  Created on: Sep 13, 2018
 *      Author: blake
 */

#include "ui/AlloyMessageDialog.h"
#include "ui/AlloyApplication.h"
#include "ui/AlloyDrawUtil.h"
namespace aly {
MessageDialog::MessageDialog(const std::string& name, const AUnit2D& pos,
		const AUnit2D& dims, bool wrap, const MessageOption& option,
		const MessageType& type) :
		Composite(name, pos, dims), option(option), type(type) {
	setVisible(false);
	containerRegion = std::shared_ptr<Composite>(
			new Composite("Container", CoordPX(0, 15),
					CoordPerPX(1.0, 1.0, -15, -15)));

	TextButtonPtr actionButton = std::shared_ptr<TextButton>(
			new TextButton(
					(option == MessageOption::Okay
							|| option == MessageOption::OkayCancel) ?
							"Okay" : "Yes",
					CoordPerPX(0.5f, 1.0f,
							(option == MessageOption::Okay) ? 0.0f : -5.0f,
							-40.0f), CoordPX(100, 30)));
	actionButton->setAspectRule(AspectRule::Unspecified);
	actionButton->onMouseDown =
			[this](AlloyContext* context, const InputEvent& event) {
				this->returnValue=true;
				this->setVisible(false);
				if(onSelect) {
					onSelect(this);
				}
				return true;
			};
	containerRegion->add(actionButton);
	if (option == MessageOption::Okay) {
		actionButton->setOrigin(Origin::TopCenter);

	} else {
		actionButton->setOrigin(Origin::TopRight);

	}
	if (option == MessageOption::OkayCancel || option == MessageOption::YesNo) {
		TextButtonPtr inactionButton = std::shared_ptr<TextButton>(
				new TextButton(
						(option == MessageOption::OkayCancel) ? "Cancel" : "No",
						CoordPerPX(0.5f, 1.0f, 5.0f, -40.0f),
						CoordPX(100, 30)));
		inactionButton->setAspectRule(AspectRule::Unspecified);
		inactionButton->onMouseDown =
				[this](AlloyContext* context, const InputEvent& event) {
					this->returnValue=false;
					this->setVisible(false);
					if(onSelect) {
						onSelect(this);
					}
					return true;
				};
		containerRegion->add(inactionButton);

	}
	int code = 0;
	switch (type) {
	case MessageType::Error:
		code = 0xf056;
		break;
	case MessageType::Warning:
		code = 0xf06a;
		break;
	case MessageType::Information:
		code = 0xf05a;
		break;
	case MessageType::Question:
		code = 0xf059;
		break;
	default:
		code = 0;
	}
	GlyphRegionPtr glyphRegion = GlyphRegionPtr(
			new GlyphRegion("icon",
					AlloyApplicationContext()->createAwesomeGlyph(code,
							FontStyle::Normal, 50.0f),
					CoordPerPX(0.0f, 0.5f, 10.0f, (wrap) ? -50.0f : -40.0f),
					CoordPX(50.0f, 50.0f)));

	glyphRegion->setAspectRule(AspectRule::FixedHeight);
	glyphRegion->setOrigin(Origin::TopLeft);

	IconButtonPtr cancelButton = std::shared_ptr<IconButton>(
			new IconButton(0xf00d, CoordPerPX(1.0, 0.0, -30, 30),
					CoordPX(30, 30), IconType::CIRCLE));
	cancelButton->setOrigin(Origin::BottomLeft);
	cancelButton->backgroundColor = MakeColor(COLOR_NONE);
	cancelButton->borderColor = MakeColor(
			AlloyApplicationContext()->theme.LIGHTEST);
	cancelButton->onMouseDown =
			[this](AlloyContext* context, const InputEvent& event) {
				returnValue=false;
				this->setVisible(false);
				return true;
			};
	containerRegion->add(glyphRegion);
	if (wrap) {
		textLabel = TextRegionPtr(
				new TextRegion(name, CoordPerPX(0.0f, 0.5f, 60.0f, -50.0f),
						CoordPerPX(1.0f, 0.0f, -70.0f, 50.0f)));
		containerRegion->add(textLabel);
	} else {
		textLabel = TextLabelPtr(
				new TextLabel(name, CoordPerPX(0.0f, 0.5f, 60.0f, -40.0f),
						CoordPerPX(1.0f, 0.0f, -70.0f, 50.0f)));
		containerRegion->add(textLabel);
		textLabel->verticalAlignment = VerticalAlignment::Middle;
	}
	add(containerRegion);
	add(cancelButton);
	this->onEvent = [=](AlloyContext* context, const InputEvent& e) {
		if(e.type==InputType::Key&&this->isVisible()) {
			if(e.key==GLFW_KEY_ESCAPE) {
				this->setVisible(false);
				context->getGlassPane()->setVisible(false);
				return true;
			}
		}
		return false;
	};

	Application::addListener(this);
}
void MessageDialog::draw(AlloyContext* context) {
	NVGcontext* nvg = context->nvgContext;
	box2px bounds = containerRegion->getBounds();
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

	Composite::draw(context);
}
void MessageDialog::setVisible(bool visible) {
	if (!Composite::isVisible()) {
		AlloyApplicationContext()->getGlassPane()->setVisible(true);
	} else {
		AlloyApplicationContext()->getGlassPane()->setVisible(false);
	}
	Composite::setVisible(visible);
}
MessageDialog::MessageDialog(const std::string& name, bool wrap,
		const MessageOption& option, const MessageType& type) :
		MessageDialog(name, CoordPerPX(0.5, 0.5, -200 + 7.5f, -100 - 7.5f),
				CoordPX(400, 200), wrap, option, type) {

}

} /* namespace aly */
