/*
 * AlloyScrollWidget.cpp
 *
 *  Created on: Sep 13, 2018
 *      Author: blake
 */

#include "ui/AlloySlider.h"
#include "ui/AlloyApplication.h"
#include "ui/AlloyDrawUtil.h"
namespace aly {

void Slider::setSliderColor(const Color& startColor, const Color& endColor) {
	sliderTrack->startColor = startColor;
	sliderTrack->endColor = endColor;
}
void Slider::setMinValue(const Number& v) {
	minValue = v;
}
void Slider::setMaxValue(const Number& v) {
	maxValue = v;
}
Slider::Slider(const std::string& name, const Number& min, const Number& max,
		const Number& val) :
		Composite(name), minValue(min), maxValue(max), value(val), sliderPosition(
				0.0) {
}
Slider::Slider(const std::string& name, const AUnit2D& pos, const AUnit2D& dims,
		const Number& min, const Number& max, const Number& val) :
		Composite(name, pos, dims), minValue(min), maxValue(max), value(val), sliderPosition(
				0.0) {
}
void Slider::setValue(int value) {
	setValue((double) value);
}
void Slider::setValue(float value) {
	setValue((double) value);
}
const Number& Slider::getValue() {
	return value;
}
const Number& Slider::getMinValue() {
	return minValue;
}
const Number& Slider::getMaxValue() {
	return maxValue;
}
void Slider::setOnChangeEvent(
		const std::function<void(const Number& value)>& func) {
	onChangeEvent = func;
}
void Slider::setLabelFormatter(
		const std::function<std::string(const Number& value)>& func) {
	if (valueLabel.get() != nullptr) {
		valueLabel->labelFormatter = func;
	}
}

SliderTrack::SliderTrack(const std::string& name, Orientation orient,
		const Color& st, const Color& ed) :
		Composite(name), orientation(orient), activeRegion(0.0f, 0.0f), currentPosition(
				-1.0f), startColor(st), endColor(ed) {
}

void SliderTrack::draw(AlloyContext* context) {
	NVGcontext* nvg = context->nvgContext;
	box2px bounds = getBounds();
	float ax, ay, bx, by;
	if (orientation == Orientation::Horizontal) {
		nvgStrokeWidth(nvg, 10.0f);
		nvgBeginPath(nvg);
		nvgMoveTo(nvg, ax =
				(bounds.position.x + bounds.dimensions.y * 0.5f - 1),
				ay = (bounds.position.y + bounds.dimensions.y * 0.5f));
		nvgLineTo(nvg,
				bx = (bounds.position.x - bounds.dimensions.y * 0.5f + 1
						+ bounds.dimensions.x),
				by = (bounds.position.y + bounds.dimensions.y * 0.5f));
		NVGpaint paint = nvgLinearGradient(nvg, ax, ay, bx, by, endColor,
				startColor);
		nvgStrokePaint(nvg, paint);
		nvgLineCap(nvg, NVG_ROUND);
		nvgStroke(nvg);
		nvgStrokeWidth(nvg, 11.0f);
		if (activeRegion.y - activeRegion.x > 0.0f) {
			pushScissor(nvg, getCursorBounds());
			pushScissor(nvg,
					bounds.position.x + bounds.dimensions.x * activeRegion.x
							+ 2.0f, bounds.position.y,
					bounds.dimensions.x * (activeRegion.y - activeRegion.x)
							- 4.0f, bounds.dimensions.y);
			nvgBeginPath(nvg);
			nvgMoveTo(nvg, ax, ay);
			nvgLineTo(nvg, bx, by);
			nvgStrokeColor(nvg, context->theme.DARK.toSemiTransparent(0.5f));
			nvgStroke(nvg);
			popScissor(nvg);
			popScissor(nvg);
		}

		if (currentPosition >= 0) {
			nvgBeginPath(nvg);
			nvgStrokeWidth(nvg, 2.0f);
			nvgStrokeColor(nvg, context->theme.LIGHTEST);
			nvgMoveTo(nvg,
					bounds.position.x + bounds.dimensions.x * currentPosition,
					bounds.position.y);
			nvgLineTo(nvg,
					bounds.position.x + bounds.dimensions.x * currentPosition,
					bounds.position.y + 0.75f * bounds.dimensions.y);
			nvgStroke(nvg);
		}
	} else if (orientation == Orientation::Vertical) {
		nvgBeginPath(nvg);
		nvgMoveTo(nvg, ax = (bounds.position.x + bounds.dimensions.x * 0.5f),
				ay = (bounds.position.y + bounds.dimensions.x * 0.5f - 1));
		nvgLineTo(nvg, bx = (bounds.position.x + bounds.dimensions.x * 0.5f),
				by = (bounds.position.y - bounds.dimensions.x * 0.5f + 1
						+ bounds.dimensions.y));
		NVGpaint paint = nvgLinearGradient(nvg, ax, ay, bx, by, endColor,
				startColor);
		nvgStrokePaint(nvg, paint);
		nvgStrokeWidth(nvg, 10.0f);
		nvgLineCap(nvg, NVG_ROUND);
		nvgStroke(nvg);

	}
	for (std::shared_ptr<Region> ptr : children) {
		ptr->draw(context);
	}
}
void SliderHandle::draw(AlloyContext* context) {
	NVGcontext* nvg = context->nvgContext;
	box2px bounds = getBounds();

	const float w = bounds.dimensions.x;
	const float h = bounds.dimensions.y;

	const float cosx = cos(ALY_PI * 60.0f / 180.0f);
	const float sinx = sin(ALY_PI * 60.0f / 180.0f);

	float r = h * 0.4f / sinx;
	float x;
	float y = bounds.position.y;
	if (context->isMouseOver(this) || context->isMouseDown(this)) {
		nvgFillColor(nvg, context->theme.LIGHT.toSemiTransparent(0.5f));
		if (handleShape == SliderHandleShape::Whole) {
			nvgBeginPath(nvg);
			nvgCircle(nvg, bounds.position.x + bounds.dimensions.x * 0.5f,
					bounds.position.y + bounds.dimensions.y * 0.5f,
					bounds.dimensions.y * 0.4f);
			nvgFill(nvg);
		} else if (handleShape == SliderHandleShape::HalfLeft) {
			nvgBeginPath(nvg);
			x = bounds.position.x + 0.15f * w;
			nvgMoveTo(nvg, x + w - r, y + 0.5f * h);
			nvgLineTo(nvg, x + w - r * cosx, y + 0.5f * h - r * sinx);
			nvgLineTo(nvg, x + w, y + 0.5f * h - r * sinx);
			nvgLineTo(nvg, x + w, y + 0.5f * h + r * sinx);
			nvgLineTo(nvg, x + w - r * cosx, y + 0.5f * h + r * sinx);
			nvgClosePath(nvg);
			nvgFill(nvg);
		} else if (handleShape == SliderHandleShape::HalfRight) {
			nvgBeginPath(nvg);
			x = bounds.position.x - 0.15f * w;
			nvgMoveTo(nvg, x + r, y + 0.5f * h);
			nvgLineTo(nvg, x + r * cosx, y + 0.5f * h - r * sinx);
			nvgLineTo(nvg, x, y + 0.5f * h - r * sinx);
			nvgLineTo(nvg, x, y + 0.5f * h + r * sinx);
			nvgLineTo(nvg, x + r * cosx, y + 0.5f * h + r * sinx);
			nvgClosePath(nvg);
			nvgFill(nvg);
		} else if (handleShape == SliderHandleShape::Hat) {
			nvgBeginPath(nvg);
			x = bounds.position.x;
			nvgMoveTo(nvg, x + 0.5f * w - r * cosx, y + 0.5f * h + r * sinx);
			nvgLineTo(nvg, x + 0.5f * w + r * cosx, y + 0.5f * h + r * sinx);
			nvgLineTo(nvg, x + 0.5f * w + r, y + 0.5f * h - r * sinx);
			nvgLineTo(nvg, x + 0.5f * w - r, y + 0.5f * h - r * sinx);
			nvgClosePath(nvg);
			nvgFill(nvg);

		}
	}
	nvgStrokeWidth(nvg, 2.0f);
	nvgStrokeColor(nvg, context->theme.NEUTRAL.toSemiTransparent(0.5f));
	nvgFillColor(nvg, context->theme.LIGHTEST);

	x = bounds.position.x;
	r = h * 0.25f / sinx;
	if (handleShape == SliderHandleShape::Whole) {
		nvgBeginPath(nvg);
		nvgCircle(nvg, bounds.position.x + bounds.dimensions.x * 0.5f,
				bounds.position.y + bounds.dimensions.y * 0.5f,
				bounds.dimensions.y * 0.25f);
		nvgFill(nvg);
		nvgStroke(nvg);
	} else if (handleShape == SliderHandleShape::HalfLeft) {
		nvgBeginPath(nvg);
		nvgMoveTo(nvg, x + w - r, y + 0.5f * h);
		nvgLineTo(nvg, x + w - r * cosx, y + 0.5f * h - r * sinx);
		nvgLineTo(nvg, x + w, y + 0.5f * h - r * sinx);
		nvgLineTo(nvg, x + w, y + 0.5f * h + r * sinx);
		nvgLineTo(nvg, x + w - r * cosx, y + 0.5f * h + r * sinx);
		nvgClosePath(nvg);
		nvgFill(nvg);
		nvgStroke(nvg);
	} else if (handleShape == SliderHandleShape::HalfRight) {
		nvgBeginPath(nvg);
		nvgMoveTo(nvg, x + r, y + 0.5f * h);
		nvgLineTo(nvg, x + r * cosx, y + 0.5f * h - r * sinx);
		nvgLineTo(nvg, x, y + 0.5f * h - r * sinx);
		nvgLineTo(nvg, x, y + 0.5f * h + r * sinx);
		nvgLineTo(nvg, x + r * cosx, y + 0.5f * h + r * sinx);
		nvgClosePath(nvg);
		nvgFill(nvg);
		nvgStroke(nvg);
	} else if (handleShape == SliderHandleShape::Hat) {
		nvgBeginPath(nvg);
		nvgMoveTo(nvg, x + 0.5f * w - r * cosx, y + 0.5f * h + r * sinx);
		nvgLineTo(nvg, x + 0.5f * w + r * cosx, y + 0.5f * h + r * sinx);
		nvgLineTo(nvg, x + 0.5f * w + r, y + 0.5f * h - r * sinx);
		nvgLineTo(nvg, x + 0.5f * w - r, y + 0.5f * h - r * sinx);
		nvgClosePath(nvg);
		nvgFill(nvg);
		nvgStroke(nvg);
		nvgStrokeColor(nvg, context->theme.LIGHTEST);
		nvgLineCap(nvg, NVG_ROUND);

		nvgBeginPath(nvg);
		nvgMoveTo(nvg, x + 0.5f * w, y + 0.5f * h + r * sinx);
		nvgLineTo(nvg, x + 0.5f * w, y + h);
		nvgStroke(nvg);

	}

}
void ScrollHandle::draw(AlloyContext* context) {
	if (!visible)
		return;
	box2px bounds = getBounds();
	float x = bounds.position.x;
	float y = bounds.position.y;
	float w = bounds.dimensions.x;
	float h = bounds.dimensions.y;
	pixel lineWidth = borderWidth.toPixels(bounds.dimensions.y, context->dpmm.y,
			context->pixelRatio);
	NVGcontext* nvg = context->nvgContext;
	if (orientation == Orientation::Vertical) {
		NVGpaint shadowPaint = nvgBoxGradient(nvg, x + lineWidth - 1,
				y + lineWidth - 1, w - 2 * lineWidth, h - 2 * lineWidth,
				(w - 2 - 2 * lineWidth) / 2, 4, context->theme.LIGHTEST,
				context->theme.NEUTRAL);
		nvgBeginPath(nvg);
		nvgRoundedRect(nvg, x + 1 + lineWidth, y + 1 + lineWidth,
				w - 2 - 2 * lineWidth, h - 2 - 2 * lineWidth,
				(w - 2 - 2 * lineWidth) / 2);
		nvgFillPaint(nvg, shadowPaint);
		nvgFill(nvg);
	} else if (orientation == Orientation::Horizontal) {
		NVGpaint shadowPaint = nvgBoxGradient(nvg, x + lineWidth - 1,
				y + lineWidth - 1, w - 2 * lineWidth, h - 2 * lineWidth,
				(h - 2 - 2 * lineWidth) / 2, 4, context->theme.LIGHTEST,
				context->theme.NEUTRAL);
		nvgBeginPath(nvg);
		nvgRoundedRect(nvg, x + 1 + lineWidth, y + 1 + lineWidth,
				w - 2 - 2 * lineWidth, h - 2 - 2 * lineWidth,
				(h - 2 - 2 * lineWidth) / 2);
		nvgFillPaint(nvg, shadowPaint);
		nvgFill(nvg);
	}
}

void ScrollTrack::draw(AlloyContext* context) {
	if (!visible)
		return;
	box2px bounds = getBounds();
	float x = bounds.position.x;
	float y = bounds.position.y;
	float w = bounds.dimensions.x;
	float h = bounds.dimensions.y;
	pixel lineWidth = borderWidth.toPixels(bounds.dimensions.y, context->dpmm.y,
			context->pixelRatio);
	NVGcontext* nvg = context->nvgContext;

	if (orientation == Orientation::Vertical) {
		NVGpaint shadowPaint = nvgBoxGradient(nvg, x + lineWidth + 1, //-1
		y + lineWidth + 1, //+1
		w - 2 * lineWidth, h - 2 * lineWidth, (w - 2 * lineWidth) / 2, 4,
				context->theme.DARKEST.toSemiTransparent(32),
				context->theme.DARKEST.toSemiTransparent(92));
		nvgBeginPath(nvg);
		nvgRoundedRect(nvg, x + lineWidth, y + lineWidth, w - 2 * lineWidth,
				h - 2 * lineWidth, (w - 2 * lineWidth) / 2);
		nvgFillPaint(nvg, shadowPaint);
		nvgFill(nvg);
	} else if (orientation == Orientation::Horizontal) {
		NVGpaint shadowPaint = nvgBoxGradient(nvg, x + lineWidth + 1, //-1
		y + lineWidth + 1, //+1
		w - 2 * lineWidth, h - 2 * lineWidth, (h - 2 * lineWidth) / 2, 4,
				context->theme.DARKEST.toSemiTransparent(32),
				context->theme.DARKEST.toSemiTransparent(92));
		nvgBeginPath(nvg);
		nvgRoundedRect(nvg, x + lineWidth, y + lineWidth, w - 2 * lineWidth,
				h - 2 * lineWidth, (h - 2 * lineWidth) / 2);
		nvgFillPaint(nvg, shadowPaint);
		nvgFill(nvg);
	}
}
HorizontalSlider::HorizontalSlider(const std::string& label,
		const AUnit2D& position, const AUnit2D& dimensions, bool showLabel,
		const Number& min, const Number& max, const Number& value) :
		Slider(label, min, max, value) {
	this->position = position;
	this->dimensions = dimensions;
	float handleSize = 30.0f;
	float trackPadding = 10.0f;
	this->aspectRatio = 4.0f;

	sliderPosition = value.toDouble();
	textColor = MakeColor(AlloyApplicationContext()->theme.LIGHTER);
	backgroundColor = MakeColor(AlloyApplicationContext()->theme.DARK);
	borderColor = MakeColor(AlloyApplicationContext()->theme.LIGHT);
	borderWidth = UnitPX(1.0f);
	setRoundCorners(true);

	sliderHandle = std::shared_ptr<SliderHandle>(
			new SliderHandle("Scroll Handle"));

	sliderHandle->position = CoordPercent(0.0, 0.0);
	sliderHandle->dimensions = CoordPX(handleSize, handleSize);
	sliderHandle->backgroundColor = MakeColor(
			AlloyApplicationContext()->theme.LIGHT);
	sliderHandle->setDragEnabled(true);

	sliderTrack = std::shared_ptr<SliderTrack>(
			new SliderTrack("Scroll Track", Orientation::Horizontal,
					AlloyApplicationContext()->theme.LIGHTEST,
					AlloyApplicationContext()->theme.LIGHTEST));

	sliderTrack->backgroundColor = MakeColor(
			AlloyApplicationContext()->theme.DARK);
	sliderTrack->add(sliderHandle);
	sliderTrack->onMouseDown =
			[this](AlloyContext* context, const InputEvent& e) {return this->onMouseDown(context, sliderTrack.get(), e);};
	sliderHandle->onMouseDown =
			[this](AlloyContext* context, const InputEvent& e) {return this->onMouseDown(context, sliderHandle.get(), e);};
	sliderHandle->onMouseDrag =
			[this](AlloyContext* context, const InputEvent& e) {
				return this->onMouseDrag(context, sliderHandle.get(), e);};
	if (showLabel) {
		sliderTrack->position = CoordPerPX(0.0f, 1.0f, 0.0f, -handleSize);
		sliderTrack->dimensions = CoordPerPX(1.0f, 0.0f, 0.0f, handleSize);
		add(
				sliderLabel = MakeTextLabel(label,
						CoordPerPX(0.0f, 0.0f, trackPadding, 2.0f),
						CoordPerPX(0.5f, 1.0f, 0.0f,
								-(handleSize - trackPadding * 0.75f)),
						FontType::Bold, UnitPerPX(1.0f, 0.0f),
						AlloyApplicationContext()->theme.LIGHTER.toRGBA(),
						HorizontalAlignment::Left, VerticalAlignment::Bottom));
		sliderLabel->setTruncate(false);

		add(
				valueLabel = std::shared_ptr<ModifiableNumber>(
						new ModifiableNumber("Value",
								CoordPerPX(0.5f, 0.0f, 0.0f, 0.0f),
								CoordPerPX(0.5f, 1.0f, -trackPadding,
										-(handleSize - trackPadding * 0.75f)),
								value.type(), true)));

		valueLabel->fontType = FontType::Normal;
		valueLabel->fontSize = UnitPerPX(1.0f, -2);
		valueLabel->backgroundColor = MakeColor(COLOR_NONE);
		valueLabel->borderColor = MakeColor(COLOR_NONE);
		valueLabel->borderWidth = UnitPX(0.0f);
		valueLabel->textColor = MakeColor(
				AlloyApplicationContext()->theme.LIGHTER);
		valueLabel->setAlignment(HorizontalAlignment::Right,
				VerticalAlignment::Bottom);
		valueLabel->onTextEntered = [this](NumberField* field) {
			this->setValue(valueLabel->getValue().toDouble());
			if (onChangeEvent)
				onChangeEvent(this->value);

		};
	} else {
		sliderTrack->position = CoordPerPX(0.0f, 0.5f, 0.0f,
				-0.5f * handleSize);
		sliderTrack->dimensions = CoordPerPX(1.0f, 0.0f, 0.0f, handleSize);
	}

	add(sliderTrack);
	this->onPack = [this]() {
		this->setValue(sliderPosition);
	};
	Application::addListener(this);
}
bool HorizontalSlider::onEventHandler(AlloyContext* context,
		const InputEvent& event) {
	if (event.type == InputType::Scroll && isVisible()
			&& context->isMouseContainedIn(this)) {
		double oldV = getBlendValue();
		double newV = clamp(event.scroll.y * 0.1f + oldV, 0.0, 1.0);
		if (newV != oldV) {
			this->setBlendValue(newV);
			if (onChangeEvent)
				onChangeEvent(this->value);
			return true;
		}
	}
	return Composite::onEventHandler(context, event);
}
void HorizontalSlider::setValue(double value) {
	double interp = clamp(
			(value - minValue.toDouble())
					/ (maxValue.toDouble() - minValue.toDouble()), 0.0, 1.0);
	float xoff = (float) (sliderTrack->getBoundsPositionX()
			+ interp
					* (sliderTrack->getBoundsDimensionsX()
							- sliderHandle->getBoundsDimensionsX()));
	sliderHandle->setDragOffset(
			pixel2(xoff, sliderHandle->getBoundsDimensionsY()),
			pixel2(0.0f, 0.0f));
	sliderPosition = value;
	this->value.setValue(
			clamp(value, minValue.toDouble(), maxValue.toDouble()));
	if (valueLabel.get() != nullptr)
		valueLabel->setNumberValue(this->value);
}
void HorizontalSlider::update() {
	double interp = (sliderHandle->getBoundsPositionX()
			- sliderTrack->getBoundsPositionX())
			/ (double) (sliderTrack->getBoundsDimensionsX()
					- sliderHandle->getBoundsDimensionsX());
	double val = (double) ((1.0 - interp) * minValue.toDouble()
			+ interp * maxValue.toDouble());
	sliderPosition = val;
	value.setValue(clamp(val, minValue.toDouble(), maxValue.toDouble()));
	if (valueLabel.get() != nullptr)
		valueLabel->setNumberValue(this->value);
}
bool HorizontalSlider::onMouseDown(AlloyContext* context, Region* region,
		const InputEvent& event) {
	if (event.button == GLFW_MOUSE_BUTTON_LEFT) {
		if (region == sliderTrack.get()) {
			sliderHandle->setDragOffset(event.cursor,
					sliderHandle->getBoundsDimensions() * 0.5f);
			context->setDragObject(sliderHandle.get());
			update();
			if (onChangeEvent)
				onChangeEvent(value);
			return true;
		} else if (region == sliderHandle.get()) {
			update();
			if (onChangeEvent)
				onChangeEvent(value);
			return true;
		}
	}
	return false;
}
void Slider::setBlendValue(double value) {
	value = clamp(value, 0.0, 1.0);
	setValue(
			value * (maxValue.toDouble() - minValue.toDouble())
					+ minValue.toDouble());
}

double Slider::getBlendValue() const {
	return (sliderPosition - minValue.toDouble())
			/ (maxValue.toDouble() - minValue.toDouble());
}
bool HorizontalSlider::onMouseDrag(AlloyContext* context, Region* region,
		const InputEvent& event) {
	if (region == sliderHandle.get()) {
		region->setDragOffset(event.cursor,
				context->getRelativeCursorDownPosition());
		update();
		if (onChangeEvent)
			onChangeEvent(value);
		return true;
	}
	return false;
}

VerticalSlider::VerticalSlider(const std::string& label,
		const AUnit2D& position, const AUnit2D& dimensions, const Number& min,
		const Number& max, const Number& value) :
		Slider(label, min, max, value) {
	this->position = position;
	this->dimensions = dimensions;
	float handleSize = 30.0f;
	this->aspectRatio = 4.0f;
	sliderPosition = value.toDouble();
	textColor = MakeColor(AlloyApplicationContext()->theme.LIGHTER);
	backgroundColor = MakeColor(AlloyApplicationContext()->theme.DARK);
	borderColor = MakeColor(AlloyApplicationContext()->theme.LIGHT);
	borderWidth = UnitPX(1.0f);
	setRoundCorners(true);
	sliderHandle = std::shared_ptr<SliderHandle>(
			new SliderHandle("Scroll Handle"));

	sliderHandle->position = CoordPercent(0.0, 0.0);
	sliderHandle->dimensions = CoordPX(handleSize, handleSize);
	sliderHandle->backgroundColor = MakeColor(
			AlloyApplicationContext()->theme.LIGHT);
	sliderHandle->setDragEnabled(true);

	sliderTrack = std::shared_ptr<SliderTrack>(
			new SliderTrack("Scroll Track", Orientation::Vertical,
					AlloyApplicationContext()->theme.LIGHTEST,
					AlloyApplicationContext()->theme.LIGHTEST));

	sliderTrack->position = CoordPerPX(0.5f, 0.1f, -handleSize * 0.5f, 2.0f);
	sliderTrack->dimensions = CoordPerPX(0.0f, 0.8f, handleSize, -4.0f);

	sliderTrack->backgroundColor = MakeColor(
			AlloyApplicationContext()->theme.DARK);
	sliderTrack->add(sliderHandle);
	sliderTrack->onMouseDown =
			[this](AlloyContext* context, const InputEvent& e) {return this->onMouseDown(context, sliderTrack.get(), e);};
	sliderHandle->onMouseDown =
			[this](AlloyContext* context, const InputEvent& e) {
				return this->onMouseDown(context, sliderHandle.get(), e);};
	sliderHandle->onMouseDrag =
			[this](AlloyContext* context, const InputEvent& e) {
				return this->onMouseDrag(context, sliderHandle.get(), e);};

	add(
			sliderLabel = MakeTextLabel(label, CoordPercent(0.0f, 0.0f),
					CoordPercent(1.0f, 0.1f), FontType::Bold,
					UnitPerPX(1.0f, 0),
					AlloyApplicationContext()->theme.LIGHTER.toRGBA(),
					HorizontalAlignment::Center, VerticalAlignment::Top));
	add(
			valueLabel = std::shared_ptr<ModifiableNumber>(
					new ModifiableNumber("Value",
							CoordPerPX(0.0f, 1.0f, 0.0f, -4.0f),
							CoordPerPX(1.0f, 0.1f, 0.0f, 4.0f), value.type(),
							true)));

	valueLabel->fontType = FontType::Normal;
	valueLabel->fontSize = UnitPerPX(1.0f, -2);
	valueLabel->backgroundColor = MakeColor(COLOR_NONE);
	valueLabel->borderColor = MakeColor(COLOR_NONE);
	valueLabel->borderWidth = UnitPX(0.0f);
	valueLabel->textColor = MakeColor(AlloyApplicationContext()->theme.LIGHTER);
	valueLabel->setAlignment(HorizontalAlignment::Center,
			VerticalAlignment::Bottom);
	valueLabel->setOrigin(Origin::BottomLeft);
	valueLabel->onTextEntered = [this](NumberField* field) {
		this->setValue(valueLabel->getValue().toDouble());
		if (onChangeEvent)
			onChangeEvent(this->value);
	};
	add(sliderTrack);
	this->onPack = [this]() {
		this->setValue(sliderPosition);
	};
	Application::addListener(this);
}
bool VerticalSlider::onEventHandler(AlloyContext* context,
		const InputEvent& event) {
	if (event.type == InputType::Scroll && isVisible()
			&& context->isMouseContainedIn(this)) {
		double oldV = getBlendValue();
		double newV = clamp(event.scroll.y * 0.1f + oldV, 0.0, 1.0);
		if (newV != oldV) {
			this->setBlendValue(newV);
			if (onChangeEvent)
				onChangeEvent(this->value);
			return true;
		}
	}
	return Composite::onEventHandler(context, event);
}
void VerticalSlider::setValue(double value) {
	double interp = 1.0f
			- clamp(
					(value - minValue.toDouble())
							/ (maxValue.toDouble() - minValue.toDouble()), 0.0,
					1.0);
	float yoff = (float) (sliderTrack->getBoundsPositionY()
			+ interp
					* (sliderTrack->getBoundsDimensionsY()
							- sliderHandle->getBoundsDimensionsY()));
	sliderHandle->setDragOffset(
			pixel2(sliderHandle->getBoundsDimensionsX(), yoff),
			pixel2(0.0f, 0.0f));
	sliderPosition = value;
	this->value.setValue(
			clamp(value, minValue.toDouble(), maxValue.toDouble()));
	if (valueLabel.get() != nullptr)
		valueLabel->setNumberValue(this->value);

}
void VerticalSlider::update() {
	double interp = (sliderHandle->getBoundsPositionY()
			- sliderTrack->getBoundsPositionY())
			/ (double) (sliderTrack->getBoundsDimensionsY()
					- sliderHandle->getBoundsDimensionsY());
	double val = (double) (interp * minValue.toDouble()
			+ (1.0 - interp) * maxValue.toDouble());
	sliderPosition = val;
	value.setValue(clamp(val, minValue.toDouble(), maxValue.toDouble()));
	if (valueLabel.get() != nullptr)
		valueLabel->setNumberValue(this->value);
}
bool VerticalSlider::onMouseDown(AlloyContext* context, Region* region,
		const InputEvent& event) {
	if (event.button == GLFW_MOUSE_BUTTON_LEFT) {
		if (region == sliderTrack.get()) {
			sliderHandle->setDragOffset(event.cursor,
					sliderHandle->getBoundsDimensions() * 0.5f);
			context->setDragObject(sliderHandle.get());
			update();
			if (onChangeEvent)
				onChangeEvent(value);
			return true;
		} else if (region == sliderHandle.get()) {
			update();
			if (onChangeEvent)
				onChangeEvent(value);
			return true;
		}
	}
	return false;
}
bool VerticalSlider::onMouseDrag(AlloyContext* context, Region* region,
		const InputEvent& event) {
	if (region == sliderHandle.get()) {
		region->setDragOffset(event.cursor,
				context->getRelativeCursorDownPosition());
		update();
		if (onChangeEvent)
			onChangeEvent(value);
		return true;
	}
	return false;
}

void RangeSlider::setSliderColor(const Color& startColor,
		const Color& endColor) {
	sliderTrack->startColor = startColor;
	sliderTrack->endColor = endColor;
}
RangeSlider::RangeSlider(const std::string& name, const AUnit2D& pos,
		const AUnit2D& dims, const Number& min, const Number& max,
		const Number& lowerValue, const Number& upperValue, bool showLabel) :
		Composite(name, pos, dims), minValue(min), maxValue(max), lowerValue(
				lowerValue), upperValue(upperValue), sliderPosition(0.0) {
	this->position = position;
	this->dimensions = dimensions;
	float handleSize = 30.0f;
	float trackPadding = 10.0f;
	this->aspectRatio = 4.0f;

	textColor = MakeColor(AlloyApplicationContext()->theme.LIGHTER);
	backgroundColor = MakeColor(AlloyApplicationContext()->theme.DARK);
	borderColor = MakeColor(AlloyApplicationContext()->theme.LIGHT);
	borderWidth = UnitPX(1.0f);
	setRoundCorners(true);

	lowerSliderHandle = std::shared_ptr<SliderHandle>(
			new SliderHandle("Lower Handle", SliderHandleShape::HalfLeft));
	lowerSliderHandle->position = CoordPercent(0.0, 0.0);
	lowerSliderHandle->dimensions = CoordPX(handleSize * 0.5f, handleSize);
	lowerSliderHandle->backgroundColor = MakeColor(
			AlloyApplicationContext()->theme.LIGHT);
	lowerSliderHandle->setDragEnabled(true);

	upperSliderHandle = std::shared_ptr<SliderHandle>(
			new SliderHandle("Upper Handle", SliderHandleShape::HalfRight));
	upperSliderHandle->position = CoordPercent(0.0, 0.0);
	upperSliderHandle->dimensions = CoordPX(handleSize * 0.5f, handleSize);
	upperSliderHandle->backgroundColor = MakeColor(
			AlloyApplicationContext()->theme.LIGHT);
	upperSliderHandle->setDragEnabled(true);

	sliderTrack = std::shared_ptr<SliderTrack>(
			new SliderTrack("Scroll Track", Orientation::Horizontal,
					AlloyApplicationContext()->theme.LIGHTEST,
					AlloyApplicationContext()->theme.LIGHTEST));
	sliderTrack->backgroundColor = MakeColor(
			AlloyApplicationContext()->theme.DARK);
	sliderTrack->add(lowerSliderHandle);
	sliderTrack->add(upperSliderHandle);
	sliderTrack->onMouseDown =
			[this](AlloyContext* context, const InputEvent& e) {return this->onMouseDown(context, sliderTrack.get(), e);};
	lowerSliderHandle->onMouseDown =
			[this](AlloyContext* context, const InputEvent& e) {return this->onMouseDown(context, lowerSliderHandle.get(), e);};
	lowerSliderHandle->onMouseDrag =
			[this](AlloyContext* context, const InputEvent& e) {return this->onMouseDrag(context, lowerSliderHandle.get(), e);};
	upperSliderHandle->onMouseDown =
			[this](AlloyContext* context, const InputEvent& e) {return this->onMouseDown(context, upperSliderHandle.get(), e);};
	upperSliderHandle->onMouseDrag =
			[this](AlloyContext* context, const InputEvent& e) {return this->onMouseDrag(context, upperSliderHandle.get(), e);};
	if (showLabel) {
		sliderTrack->position = CoordPerPX(0.0f, 1.0f, 0.0f, -handleSize);
		sliderTrack->dimensions = CoordPerPX(1.0f, 0.0f, 0.0f, handleSize);
		add(
				sliderLabel = MakeTextLabel(name,
						CoordPerPX(0.5f, 0.0f, 0.0f, 2.0f),
						CoordPerPX(1.0f, 1.0f, 0.0f,
								-(handleSize - trackPadding * 0.75f)),
						FontType::Bold, UnitPerPX(1.0f, 0.0f),
						AlloyApplicationContext()->theme.LIGHTER.toRGBA(),
						HorizontalAlignment::Center,
						VerticalAlignment::Bottom));
		sliderLabel->setOrigin(Origin::TopCenter);
		add(
				lowerValueLabel = std::shared_ptr<ModifiableNumber>(new ModifiableNumber("Lower Value",
						CoordPerPX(0.0f, 0.0f, trackPadding, 2.0f),
						CoordPerPX(0.35f, 1.0f, 0.0f,
								-(handleSize - trackPadding * 0.75f)),
						lowerValue.type(),true)));
		add(
				upperValueLabel = std::shared_ptr<ModifiableNumber>(new ModifiableNumber("Upper Value",
						CoordPerPX(0.65f, 0.0f, 0.0f, 2.0f),
						CoordPerPX(0.35f, 1.0f, -trackPadding,
								-(handleSize - trackPadding * 0.75f)),
						upperValue.type(),true)));

		lowerValueLabel->fontType = FontType::Normal;
		lowerValueLabel->fontSize = UnitPerPX(1.0f, -2);
		lowerValueLabel->backgroundColor = MakeColor(COLOR_NONE);
		lowerValueLabel->borderColor = MakeColor(COLOR_NONE);
		lowerValueLabel->borderWidth = UnitPX(0.0f);
		lowerValueLabel->textColor = MakeColor(
				AlloyApplicationContext()->theme.LIGHTER);
		lowerValueLabel->setAlignment(HorizontalAlignment::Left,
				VerticalAlignment::Bottom);
		lowerValueLabel->onTextEntered = [this](NumberField* field) {
			this->setValue(double2(lowerValueLabel->getValue().toDouble(),upperValueLabel->getValue().toDouble()));
			if (onChangeEvent)onChangeEvent(this->lowerValue,this->upperValue);

		};
		upperValueLabel->fontType = FontType::Normal;
		upperValueLabel->fontSize = UnitPerPX(1.0f, -2);
		upperValueLabel->backgroundColor = MakeColor(COLOR_NONE);
		upperValueLabel->borderColor = MakeColor(COLOR_NONE);
		upperValueLabel->borderWidth = UnitPX(0.0f);
		upperValueLabel->textColor = MakeColor(
				AlloyApplicationContext()->theme.LIGHTER);
		upperValueLabel->setAlignment(HorizontalAlignment::Right,
				VerticalAlignment::Bottom);
		upperValueLabel->onTextEntered = [this](NumberField* field) {
			this->setValue(double2(lowerValueLabel->getValue().toDouble(),upperValueLabel->getValue().toDouble()));
			if (onChangeEvent)onChangeEvent(this->lowerValue,this->upperValue);

		};
	} else {
		sliderTrack->position = CoordPerPX(0.0f, 0.5f, 0.0f,
				-0.5f * handleSize);
		sliderTrack->dimensions = CoordPerPX(1.0f, 0.0f, 0.0f, handleSize);
	}

	add(sliderTrack);
	this->onPack = [this]() {
		this->setValue(sliderPosition);
	};
	this->onEvent =
			[this](AlloyContext* context, const InputEvent& event) {

				if (event.type == InputType::Scroll&&isVisible() && context->isMouseContainedIn(this)) {
					double2 oldV = getBlendValue();
					double2 newV = clamp(event.scroll.y*0.1 + oldV, double2(0.0), double2(1.0));
					if (newV != oldV) {
						this->setBlendValue(newV);
						if (onChangeEvent)onChangeEvent(this->lowerValue,this->upperValue);
						return true;
					}
				}
				return false;
			};
	setLowerValue(lowerValue.toDouble());
	setUpperValue(upperValue.toDouble());
	Application::addListener(this);
}
inline void RangeSlider::setLabelFormatter(
		const std::function<std::string(const Number& value)>& func) {
	if (lowerValueLabel.get() != nullptr) {
		lowerValueLabel->labelFormatter = func;
	}
	if (upperValueLabel.get() != nullptr) {
		upperValueLabel->labelFormatter = func;
	}
}
void RangeSlider::setBlendValue(double2 value) {
	value = clamp(value, 0.0, 1.0);
	setValue(
			value * (maxValue.toDouble() - minValue.toDouble())
					+ minValue.toDouble());
}

double2 RangeSlider::getBlendValue() const {
	return (sliderPosition - minValue.toDouble())
			/ (maxValue.toDouble() - minValue.toDouble());
}
bool RangeSlider::onMouseDown(AlloyContext* context, Region* region,
		const InputEvent& event) {
	if (event.button == GLFW_MOUSE_BUTTON_LEFT) {
		if (region == sliderTrack.get()) {
			if (distanceSqr(event.cursor,
					lowerSliderHandle->getBounds().center())
					< distanceSqr(event.cursor,
							upperSliderHandle->getBounds().center())) {
				lowerSliderHandle->setDragOffset(event.cursor,
						lowerSliderHandle->getBoundsDimensions() * 0.5f);
				context->setDragObject(lowerSliderHandle.get());
			} else {
				upperSliderHandle->setDragOffset(event.cursor,
						upperSliderHandle->getBoundsDimensions() * 0.5f);
				context->setDragObject(upperSliderHandle.get());
			}
			update();
			if (onChangeEvent)
				onChangeEvent(lowerValue, upperValue);
			return true;

		} else if (region == lowerSliderHandle.get()) {
			update();
			if (onChangeEvent)
				onChangeEvent(lowerValue, upperValue);
			return true;
		} else if (region == upperSliderHandle.get()) {
			update();
			if (onChangeEvent)
				onChangeEvent(lowerValue, upperValue);
			return true;
		}
	}
	return false;
}
bool RangeSlider::onMouseDrag(AlloyContext* context, Region* region,
		const InputEvent& event) {
	if (region == lowerSliderHandle.get()) {
		region->setDragOffset(event.cursor,
				context->getRelativeCursorDownPosition());
		update();
		if (sliderPosition.x > sliderPosition.y) {
			setUpperValue(sliderPosition.x);
		}
		if (onChangeEvent)
			onChangeEvent(lowerValue, upperValue);
		return true;
	} else if (region == upperSliderHandle.get()) {
		region->setDragOffset(event.cursor,
				context->getRelativeCursorDownPosition());
		update();
		if (sliderPosition.x > sliderPosition.y) {
			setLowerValue(sliderPosition.y);
		}
		if (onChangeEvent)
			onChangeEvent(lowerValue, upperValue);
		return true;
	}
	return false;
}
void RangeSlider::update() {

	double interpLo = (lowerSliderHandle->getBoundsPositionX()
			- sliderTrack->getBoundsPositionX())
			/ (double) (sliderTrack->getBoundsDimensionsX()
					- 2 * lowerSliderHandle->getBoundsDimensionsX());
	double val = (double) ((1.0 - interpLo) * minValue.toDouble()
			+ interpLo * maxValue.toDouble());
	sliderPosition.x = val;
	lowerValue.setValue(clamp(val, minValue.toDouble(), maxValue.toDouble()));

	double interpHi = (upperSliderHandle->getBoundsPositionX()
			- sliderTrack->getBoundsPositionX()
			- upperSliderHandle->getBoundsDimensionsX())
			/ (double) (sliderTrack->getBoundsDimensionsX()
					- 2 * upperSliderHandle->getBoundsDimensionsX());
	val = (double) ((1.0 - interpHi) * minValue.toDouble()
			+ interpHi * maxValue.toDouble());
	sliderPosition.y = val;
	upperValue.setValue(clamp(val, minValue.toDouble(), maxValue.toDouble()));
	sliderTrack->setLower(
			(lowerSliderHandle->getBoundsPositionX()
					- sliderTrack->getBoundsPositionX()
					+ 0.5f * lowerSliderHandle->getBoundsDimensionsX())
					/ sliderTrack->getBoundsDimensionsX());
	sliderTrack->setUpper(
			(upperSliderHandle->getBoundsPositionX()
					- sliderTrack->getBoundsPositionX()
					+ 0.5f * upperSliderHandle->getBoundsDimensionsX())
					/ sliderTrack->getBoundsDimensionsX());

	if (lowerValueLabel.get() != nullptr) {
		lowerValueLabel->setNumberValue(lowerValue);
	}
	if (upperValueLabel.get() != nullptr) {
		upperValueLabel->setNumberValue(upperValue);
	}
}
void RangeSlider::setLowerValue(double value) {
	double interp = clamp(
			(value - minValue.toDouble())
					/ (maxValue.toDouble() - minValue.toDouble()), 0.0, 1.0);
	float xoff = (float) (sliderTrack->getBoundsPositionX()
			+ interp
					* (sliderTrack->getBoundsDimensionsX()
							- 2 * lowerSliderHandle->getBoundsDimensionsX()));
	lowerSliderHandle->setDragOffset(
			pixel2(xoff, lowerSliderHandle->getBoundsDimensionsY()),
			pixel2(0.0f, 0.0f));
	sliderPosition.x = value;
	lowerValue.setValue(clamp(value, minValue.toDouble(), maxValue.toDouble()));
	sliderTrack->setLower(
			(lowerSliderHandle->getBoundsPositionX()
					- sliderTrack->getBoundsPositionX()
					+ 0.5f * lowerSliderHandle->getBoundsDimensionsX())
					/ sliderTrack->getBoundsDimensionsX());
	if (lowerValueLabel.get() != nullptr) {
		lowerValueLabel->setNumberValue(lowerValue);
	}
}
void RangeSlider::setUpperValue(double value) {
	double interp = clamp(
			(value - minValue.toDouble())
					/ (maxValue.toDouble() - minValue.toDouble()), 0.0, 1.0);
	float xoff = (float) (sliderTrack->getBoundsPositionX()
			+ upperSliderHandle->getBoundsDimensionsX()
			+ interp
					* (sliderTrack->getBoundsDimensionsX()
							- 2 * upperSliderHandle->getBoundsDimensionsX()));
	upperSliderHandle->setDragOffset(
			pixel2(xoff, upperSliderHandle->getBoundsDimensionsY()),
			pixel2(0.0f, 0.0f));
	sliderPosition.y = value;
	upperValue.setValue(clamp(value, minValue.toDouble(), maxValue.toDouble()));
	sliderTrack->setUpper(
			(upperSliderHandle->getBoundsPositionX()
					- sliderTrack->getBoundsPositionX()
					+ 0.5f * upperSliderHandle->getBoundsDimensionsX())
					/ sliderTrack->getBoundsDimensionsX());
	if (upperValueLabel.get() != nullptr) {
		upperValueLabel->setNumberValue(upperValue);
	}
}
void RangeSlider::setValue(double2 value) {
	double interp = clamp(
			(value.x - minValue.toDouble())
					/ (maxValue.toDouble() - minValue.toDouble()), 0.0, 1.0);
	float xoff = (float) (sliderTrack->getBoundsPositionX()
			+ interp
					* (sliderTrack->getBoundsDimensionsX()
							- 2 * lowerSliderHandle->getBoundsDimensionsX()));
	lowerSliderHandle->setDragOffset(
			pixel2(xoff, lowerSliderHandle->getBoundsDimensionsY()),
			pixel2(0.0f, 0.0f));
	interp = clamp(
			(value.y - minValue.toDouble())
					/ (maxValue.toDouble() - minValue.toDouble()), 0.0, 1.0);
	xoff = (float) (sliderTrack->getBoundsPositionX()
			+ upperSliderHandle->getBoundsDimensionsX()
			+ interp
					* (sliderTrack->getBoundsDimensionsX()
							- 2 * upperSliderHandle->getBoundsDimensionsX()));
	upperSliderHandle->setDragOffset(
			pixel2(xoff, upperSliderHandle->getBoundsDimensionsY()),
			pixel2(0.0f, 0.0f));
	sliderPosition = value;
	lowerValue.setValue(
			clamp(value.x, minValue.toDouble(), maxValue.toDouble()));
	upperValue.setValue(
			clamp(value.y, minValue.toDouble(), maxValue.toDouble()));
	sliderTrack->setLower(
			(lowerSliderHandle->getBoundsPositionX()
					- sliderTrack->getBoundsPositionX()
					+ 0.5f * lowerSliderHandle->getBoundsDimensionsX())
					/ sliderTrack->getBoundsDimensionsX());
	sliderTrack->setUpper(
			(upperSliderHandle->getBoundsPositionX()
					- sliderTrack->getBoundsPositionX()
					+ 0.5f * upperSliderHandle->getBoundsDimensionsX())
					/ sliderTrack->getBoundsDimensionsX());
	if (lowerValueLabel.get() != nullptr) {
		lowerValueLabel->setNumberValue(lowerValue);
	}
	if (upperValueLabel.get() != nullptr) {
		upperValueLabel->setNumberValue(upperValue);
	}
}

}
