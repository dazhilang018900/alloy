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

#include "Alloy.h"
#include "../../include/example/ProceduralSkyEx.h"
using namespace aly;
ProceduralSkyEx::ProceduralSkyEx() :
		Application(960, 540, "Procedural Sky Example"), hosekShader(1.0f), preethamShader(
				1.0f) {
}
bool ProceduralSkyEx::init(Composite& rootNode) {
	box3f renderBBox = box3f(float3(-0.5f, -0.5f, -0.5f),
			float3(1.0f, 1.0f, 1.0f));
	//Make region on screen to render 3d view
	renderRegion = MakeComposite("Render View", CoordPX(0, 0),
			CoordPercent(1.0f, 1.0f), COLOR_NONE, COLOR_NONE, UnitPX(0.0f));
	sunPitch = Integer(72);
	sunAngle = Integer(110);
	albedo = Float(0.1f);
	strength = Float(1.15f);
	turbidity = Float(4.0f);
	pitchSlider = HorizontalSliderPtr(
			new HorizontalSlider("Zenith Angle", CoordPX(5.0f, 5.0f),
					CoordPX(200.0f, 45.0f), true, Integer(0), Integer(90),
					sunPitch));
	angleSlider = HorizontalSliderPtr(
			new HorizontalSlider("Azimuth Angle", CoordPX(5.0f, 55.0f),
					CoordPX(200.0f, 45.0f), true, Integer(0), Integer(180),
					sunAngle));
	albedoSlider = HorizontalSliderPtr(
			new HorizontalSlider("Albedo", CoordPX(5.0f, 105.0f),
					CoordPX(200.0f, 45.0f), true, Float(0), Float(0.5f),
					albedo));
	strengthSlider = HorizontalSliderPtr(
			new HorizontalSlider("Strength", CoordPX(5.0f, 160.0f),
					CoordPX(200.0f, 45.0f), true, Float(0.5), Float(2.0f),
					strength));
	turbiditySlider = HorizontalSliderPtr(
			new HorizontalSlider("Turbidity", CoordPX(5.0f, 210.0f),
					CoordPX(200.0f, 45.0f), true, Float(1.0f), Float(10.0f),
					turbidity));
	methodSelection = SelectionPtr(
			new Selection("Method", CoordPerPX(1.0f, 0.0f, -150.0f, 5.0f),
					CoordPX(145.0f, 30.0f), std::vector<std::string> { "Hosek",
							"Preetham" }));
	pitchSlider->backgroundColor = MakeColor(
			getContext()->theme.DARK.toSemiTransparent(0.5f));
	angleSlider->backgroundColor = MakeColor(
			getContext()->theme.DARK.toSemiTransparent(0.5f));
	albedoSlider->backgroundColor = MakeColor(
			getContext()->theme.DARK.toSemiTransparent(0.5f));
	strengthSlider->backgroundColor = MakeColor(
			getContext()->theme.DARK.toSemiTransparent(0.5f));
	turbiditySlider->backgroundColor = MakeColor(
			getContext()->theme.DARK.toSemiTransparent(0.5f));

	pitchSlider->setOnChangeEvent(
			[this](const Number& val) {
				preethamShader.setSunPositionDegrees(sunAngle.toFloat(),sunPitch.toFloat());
				hosekShader.setSunPositionDegrees(sunAngle.toFloat(),sunPitch.toFloat());
			});
	angleSlider->setOnChangeEvent(
			[this](const Number& val) {
				preethamShader.setSunPositionDegrees(sunAngle.toFloat(),sunPitch.toFloat());
				hosekShader.setSunPositionDegrees(sunAngle.toFloat(),sunPitch.toFloat());
			});
	albedoSlider->setOnChangeEvent([this](const Number& val) {
		hosekShader.setAlbedo(albedo.toFloat());
		preethamShader.setAlbedo(albedo.toFloat());
	});
	strengthSlider->setOnChangeEvent([this](const Number& val) {
		hosekShader.setStrength(strength.toFloat());
		preethamShader.setStrength(strength.toFloat());
	});
	turbiditySlider->setOnChangeEvent([this](const Number& val) {
		hosekShader.setTurbidity(turbidity.toFloat());
		preethamShader.setTurbidity(turbidity.toFloat());
	});
	camera.setNearFarPlanes(0.1f, 6.0f);
	camera.setZoom(1.25f);
	camera.setZoomRange(1.25f, 1.25f);
	camera.setTranslationRange(float3(0.0f), float3(0.0f));
	camera.setCameraType(CameraType::Perspective);
	camera.setDirty(true);
	addListener(&camera);
	methodSelection->setSelectedIndex(0);
	renderRegion->add(methodSelection);
	renderRegion->add(pitchSlider);
	renderRegion->add(angleSlider);
	renderRegion->add(albedoSlider);
	renderRegion->add(strengthSlider);
	renderRegion->add(turbiditySlider);
	rootNode.add(renderRegion);
	return true;
}
void ProceduralSkyEx::draw(AlloyContext* context) {
	if (methodSelection->getSelectedIndex() == 0) {
		hosekShader.draw(camera,
				context->pixelRatio * renderRegion->getBounds());
	} else {
		preethamShader.draw(camera,
				context->pixelRatio * renderRegion->getBounds());
	}
}

