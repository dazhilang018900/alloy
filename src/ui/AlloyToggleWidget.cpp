/*
 * Copyright(C) 2015, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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

#include "AlloyApplication.h"
#include "AlloyDrawUtil.h"
#include "AlloyContext.h"
#include "AlloyToggleWidget.h"
#include <future>
#include <cstdlib>
#include <cctype>
using namespace std;
namespace aly {

bool CheckBox::handleMouseDown(AlloyContext* context, const InputEvent& event) {
	if (event.button == GLFW_MOUSE_BUTTON_LEFT) {
		setFocus(true);
		this->checked = !(this->checked);
		this->valueLabel->textColor =
				(this->checked) ?
						MakeColor(AlloyApplicationContext()->theme.LIGHTER) :
						MakeColor(AlloyApplicationContext()->theme.DARK);
		if (onChange)
			onChange(this->checked);
		return true;
	} else if (event.button == GLFW_MOUSE_BUTTON_RIGHT) {
		setFocus(false);
		return true;
	}
	return false;
}
CheckBox::CheckBox(const std::string& label, const AUnit2D& position,
		const AUnit2D& dimensions, bool checked, bool showText) :
		Composite(label, position, dimensions), checked(checked) {
	this->aspectRatio = 4.0f;
	backgroundColor = MakeColor(AlloyApplicationContext()->theme.DARK);
	borderColor = MakeColor(AlloyApplicationContext()->theme.LIGHT);
	borderWidth = UnitPX(1.0f);
	setRoundCorners(true);
	CompositePtr valueContainer = MakeComposite("Check Bounds",
			CoordPerPX(0.0f, 0.0f, 5.0f, 5.0f),
			CoordPerPX(1.0f, 1.0f, -10.0f, -10.0f));

	DrawPtr checkBoundsDraw;
	if (showText) {
		checkLabel = MakeTextLabel(label, CoordPercent(0.0f, 0.0f),
				CoordPercent(1.0f, 1.0f), FontType::Bold, UnitPercent(1.0f),
				AlloyApplicationContext()->theme.LIGHTER.toRGBA(),
				HorizontalAlignment::Left, VerticalAlignment::Middle);
		valueLabel = MakeTextLabel(CodePointToUTF8(0xf00c),
				CoordPercent(1.0f, 0.0f), CoordPercent(0.0f, 1.0f),
				FontType::Icon, UnitPercent(1.0f),
				AlloyApplicationContext()->theme.LIGHTER.toRGBA(),
				HorizontalAlignment::Center, VerticalAlignment::Middle);
		valueLabel->setAspectRatio(1.0f);
		valueLabel->setOrigin(Origin::TopRight);
		valueLabel->setAspectRule(AspectRule::FixedHeight);
		checkBoundsDraw = DrawPtr(
				new Draw("Check Bounds", CoordPercent(1.0f, 0.0f),
						CoordPercent(0.0f, 1.0f)));
		checkBoundsDraw->setAspectRatio(1.0f);
		checkBoundsDraw->setOrigin(Origin::TopRight);
		checkBoundsDraw->setAspectRule(AspectRule::FixedHeight);
	} else {
		valueLabel = MakeTextLabel(CodePointToUTF8(0xf00c),
				CoordPercent(0.5f, 0.0f), CoordPercent(0.0f, 1.0f),
				FontType::Icon, UnitPercent(1.0f),
				AlloyApplicationContext()->theme.LIGHTER.toRGBA(),
				HorizontalAlignment::Center, VerticalAlignment::Middle);
		valueLabel->setAspectRatio(1.0f);
		valueLabel->setOrigin(Origin::TopCenter);
		valueLabel->setAspectRule(AspectRule::FixedHeight);
		checkBoundsDraw = DrawPtr(
				new Draw("Check Bounds", CoordPercent(0.5f, 0.0f),
						CoordPercent(0.0f, 1.0f)));
		checkBoundsDraw->setAspectRatio(1.0f);
		checkBoundsDraw->setOrigin(Origin::TopCenter);
		checkBoundsDraw->setAspectRule(AspectRule::FixedHeight);
	}
	checkBoundsDraw->onDraw =
			[this](AlloyContext* context, const box2px& clickbox) {
				NVGcontext* nvg = context->nvgContext;
				nvgBeginPath(nvg);
				nvgFillColor(nvg, context->theme.NEUTRAL);
				nvgRoundedRect(nvg, clickbox.position.x, clickbox.position.y,
						clickbox.dimensions.x, clickbox.dimensions.y,
						context->theme.CORNER_RADIUS);
				nvgFill(nvg);
				bool hover = context->isMouseContainedIn(this);
				if (hover) {
					nvgBeginPath(nvg);
					nvgStrokeColor(nvg, context->theme.LIGHTEST);
					nvgStrokeWidth(nvg, 2.0f);
					nvgRoundedRect(nvg, clickbox.position.x, clickbox.position.y,
							clickbox.dimensions.x, clickbox.dimensions.y,
							context->theme.CORNER_RADIUS);
					nvgStroke(nvg);
				}
			};

	checkBoundsDraw->setIgnoreCursorEvents(true);
	if (showText) {
		valueContainer->add(checkLabel);
	}
	valueContainer->add(checkBoundsDraw);
	valueContainer->add(valueLabel);

	add(valueContainer);
	this->valueLabel->textColor =
			(this->checked) ?
					MakeColor(AlloyApplicationContext()->theme.LIGHTER) :
					MakeColor(AlloyApplicationContext()->theme.DARK);
	valueLabel->onMouseDown =
			[this](AlloyContext* context, const InputEvent& event) {
				return handleMouseDown(context, event);
			};
	valueContainer->onMouseDown =
			[this](AlloyContext* context, const InputEvent& event) {
				return handleMouseDown(context, event);
			};
	/*
	if (showText) {
		checkLabel->onMouseDown =
				[this](AlloyContext* context, const InputEvent& event) {
					return handleMouseDown(context, event);
				};
	}
	*/
	Application::addListener(this);
}
void CheckBox::setValue(bool value) {
	this->checked = value;
	this->valueLabel->textColor =
			(this->checked) ?
					MakeColor(AlloyApplicationContext()->theme.LIGHTER) :
					MakeColor(AlloyApplicationContext()->theme.DARK);
}
void CheckBox::draw(AlloyContext* context) {
	bool hover = context->isMouseContainedIn(this);
	if (hover) {
	} else {
		if (checkLabel.get() != nullptr)
			checkLabel->textColor = MakeColor(context->theme.LIGHTER);
	}
	Composite::draw(context);
	const int PAD = 1.0f;
	box2px bounds = getBounds();
	NVGcontext* nvg = context->nvgContext;
	if (isObjectFocused()) {
		nvgLineJoin(nvg, NVG_MITER);
		nvgBeginPath(nvg);
		if (roundCorners) {
			nvgRoundedRect(nvg, bounds.position.x + PAD, bounds.position.y + PAD,
					bounds.dimensions.x - 2 * PAD, bounds.dimensions.y - 2 * PAD,context->theme.CORNER_RADIUS);
		} else {
			nvgRect(nvg, bounds.position.x + PAD, bounds.position.y + PAD,
					bounds.dimensions.x - 2 * PAD, bounds.dimensions.y - 2 * PAD);
		}
		nvgStrokeWidth(nvg, 2.0f);
		nvgStrokeColor(nvg, context->theme.FOCUS);
		nvgStroke(nvg);
	}
}
bool ToggleBox::handleMouseDown(AlloyContext* context,
		const InputEvent& event) {
	if (event.button == GLFW_MOUSE_BUTTON_LEFT) {
		setFocus(true);
		this->toggledOn = !(this->toggledOn);
		onLabel->setVisible(this->toggledOn);
		offLabel->setVisible(!(this->toggledOn));
		if (onChange)
			onChange(this->toggledOn);
		return true;
	} else if (event.button == GLFW_MOUSE_BUTTON_RIGHT) {
		setFocus(false);
		return true;
	}
	return false;
}
ToggleBox::ToggleBox(const std::string& label, const AUnit2D& position,
		const AUnit2D& dimensions, bool checked, bool showText) :
		Composite(label, position, dimensions), toggledOn(checked) {
	this->aspectRatio = 4.0f;
	backgroundColor = MakeColor(AlloyApplicationContext()->theme.DARK);
	borderColor = MakeColor(AlloyApplicationContext()->theme.LIGHT);
	borderWidth = UnitPX(1.0f);
	setRoundCorners(true);

	CompositePtr valueContainer = MakeComposite("Check Bounds",
			CoordPerPX(0.0f, 0.0f, 5.0f, 5.0f),
			CoordPerPX(1.0f, 1.0f, -10.0f, -10.0f));
	if (showText) {
		toggleLabel = MakeTextLabel(label, CoordPercent(0.0f, 0.0f),
				CoordPercent(1.0f, 1.0f), FontType::Bold, UnitPercent(1.0f),
				AlloyApplicationContext()->theme.LIGHTER.toRGBA(),
				HorizontalAlignment::Left, VerticalAlignment::Middle);
	}
	onLabel = MakeTextLabel("ON", CoordPercent(0.2f, 0.0f),
			CoordPercent(0.3f, 1.0f), FontType::Bold, UnitPerPX(1.0f, -4.0f),
			AlloyApplicationContext()->theme.LIGHTER,
			HorizontalAlignment::Center, VerticalAlignment::Middle);
	offLabel = MakeTextLabel("OFF", CoordPercent(0.5f, 0.0f),
			CoordPercent(0.3f, 1.0f), FontType::Bold, UnitPerPX(1.0f, -4.0f),
			AlloyApplicationContext()->theme.DARK, HorizontalAlignment::Center,
			VerticalAlignment::Middle);
	onLabel->setTruncate(false);
	offLabel->setTruncate(false);

	DrawPtr toggleRegion = DrawPtr(
			new Draw("Toggle Region", CoordPX(0.0f, 0.0f),
					CoordPercent(1.0f, 1.0f)));
	toggleRegion->setIgnoreCursorEvents(true);
	toggleRegion->onDraw =
			[this](AlloyContext* context, const box2px& clickbounds) {
				NVGcontext* nvg = context->nvgContext;
				nvgBeginPath(nvg);
				nvgFillColor(nvg, context->theme.NEUTRAL);
				box2px clickbox = clickRegion->getBounds();
				float radius = clickbox.dimensions.y / 2;
				nvgRoundedRect(nvg, clickbox.position.x, clickbox.position.y,
						clickbox.dimensions.x, clickbox.dimensions.y, radius);
				nvgFill(nvg);
				bool hover = context->isMouseContainedIn(clickRegion.get());
				float pos;
				if (toggledOn) {
					pos = clickbox.position.x + clickbox.dimensions.x - radius;
				}
				else {
					pos = clickbox.position.x + radius;
				}
				if (hover) {
					nvgBeginPath(nvg);
					nvgStrokeColor(nvg, context->theme.LIGHTEST);
					nvgStrokeWidth(nvg, 2.0f);
					nvgRoundedRect(nvg, clickbox.position.x, clickbox.position.y,
							clickbox.dimensions.x, clickbox.dimensions.y, radius);
					nvgStroke(nvg);
				}
				nvgBeginPath(nvg);
				nvgFillColor(nvg, context->theme.LIGHT);
				nvgCircle(nvg, pos, clickbox.position.y + radius, radius - 4);
				nvgFill(nvg);
			};

	if (showText) {
		clickRegion = MakeComposite("tog select", CoordPercent(1.0f, 0.0f),
				CoordPercent(0.42f, 1.0f));
		clickRegion->setOrigin(Origin::TopRight);
		clickRegion->setAspectRatio(2.5f);
		clickRegion->setAspectRule(AspectRule::FixedHeight);
		toggleRegion->setOrigin(Origin::TopRight);
		toggleRegion->setAspectRatio(2.5f);
		toggleRegion->setAspectRule(AspectRule::FixedHeight);
	} else {
		clickRegion = MakeComposite("tog select", CoordPercent(0.5f, 0.0f),
				CoordPercent(1.0f, 1.0f));
		clickRegion->setOrigin(Origin::TopCenter);
		clickRegion->setAspectRatio(2.5f);
		clickRegion->setAspectRule(AspectRule::FixedHeight);
	}
	clickRegion->add(onLabel);
	clickRegion->add(offLabel);
	if (showText) {
		valueContainer->add(toggleLabel);
	}
	valueContainer->add(toggleRegion);
	valueContainer->add(clickRegion);
	add(valueContainer);
	onLabel->setVisible(this->toggledOn);
	offLabel->setVisible(!this->toggledOn);

	onLabel->onMouseDown =
			[this](AlloyContext* context, const InputEvent& event) {
				return handleMouseDown(context, event);
			};
	offLabel->onMouseDown =
			[this](AlloyContext* context, const InputEvent& event) {
				return handleMouseDown(context, event);
			};
	clickRegion->onMouseDown =
			[this](AlloyContext* context, const InputEvent& event) {
				return handleMouseDown(context, event);
			};
	/*
	if (showText) {
		toggleLabel->onMouseDown =
				[this](AlloyContext* context, const InputEvent& event) {
					return handleMouseDown(context, event);
				};
	}
	*/
	Application::addListener(this);
}

bool ToggleBox::onEventHandler(AlloyContext* context, const InputEvent& event) {
	if (event.type == InputType::MouseButton && event.isDown()&&context->isMouseOver(this,true)) {
		if(event.button==GLFW_MOUSE_BUTTON_LEFT){
			setFocus(true);
		} else if(event.button==GLFW_MOUSE_BUTTON_RIGHT){
			setFocus(false);
		}
	} else if (event.type == InputType::Key && event.isDown()
			&& event.key == GLFW_KEY_SPACE
			&& isObjectFocused()) {
		this->toggledOn = !this->toggledOn;
		onLabel->setVisible(this->toggledOn);
		offLabel->setVisible(!this->toggledOn);
		if (onChange)
			onChange(this->toggledOn);

	}
	return Composite::onEventHandler(context, event);
}
bool CheckBox::onEventHandler(AlloyContext* context, const InputEvent& event) {
	if (event.type == InputType::MouseButton && event.isDown()&&context->isMouseOver(this,true)) {
		if(event.button==GLFW_MOUSE_BUTTON_LEFT){
			setFocus(true);
		} else if(event.button==GLFW_MOUSE_BUTTON_RIGHT){
			setFocus(false);
		}
	} else if (event.type == InputType::Key && event.isDown()
			&& event.key == GLFW_KEY_SPACE
			&& isObjectFocused()) {
		this->checked = !this->checked;
		this->valueLabel->textColor =
				(this->checked) ?
						MakeColor(AlloyApplicationContext()->theme.LIGHTER) :
						MakeColor(AlloyApplicationContext()->theme.DARK);
		if (onChange)
			onChange(this->checked);
	}
	return Composite::onEventHandler(context, event);
}
void ToggleBox::setValue(bool value) {
	this->toggledOn = value;
	onLabel->setVisible(this->toggledOn);
	offLabel->setVisible(!this->toggledOn);
}
void ToggleBox::draw(AlloyContext* context) {
	bool hover = context->isMouseContainedIn(this);
	if (toggleLabel.get() != nullptr) {
		if (hover) {
			toggleLabel->textColor = MakeColor(context->theme.LIGHTEST);
		} else {
			toggleLabel->textColor = MakeColor(context->theme.LIGHTER);
		}
	}
	Composite::draw(context);
	const int PAD = 1.0f;
	box2px bounds = getBounds();
	NVGcontext* nvg = context->nvgContext;
	if (isObjectFocused()) {
		nvgLineJoin(nvg, NVG_MITER);
		nvgBeginPath(nvg);
		if (roundCorners) {
			nvgRoundedRect(nvg, bounds.position.x + PAD, bounds.position.y + PAD,
					bounds.dimensions.x - 2 * PAD, bounds.dimensions.y - 2 * PAD,context->theme.CORNER_RADIUS);
		} else {
			nvgRect(nvg, bounds.position.x + PAD, bounds.position.y + PAD,
					bounds.dimensions.x - 2 * PAD, bounds.dimensions.y - 2 * PAD);
		}
		nvgStrokeWidth(nvg, 2.0f);
		nvgStrokeColor(nvg, context->theme.FOCUS);
		nvgStroke(nvg);
	}
}

void MessageDialog::setMessage(const std::string& message) {
	textLabel->setLabel(message);
}
std::string MessageDialog::getMessage() const {
	return textLabel->getLabel();
}

}

