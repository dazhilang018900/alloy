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
		Application(800, 600, "Procedural Sky Example") {
}
bool ProceduralSkyEx::init(Composite& rootNode) {
	box3f renderBBox = box3f(float3(-0.5f, -0.5f, -0.5f),float3(1.0f, 1.0f, 1.0f));
	//Make region on screen to render 3d view
	renderRegion=MakeComposite("Render View",CoordPX(0,0),CoordPercent(1.0f,1.0f),COLOR_NONE,COLOR_NONE,UnitPX(0.0f));
	sunPitch=Integer(65);
	sunAngle=Integer(130);
	pitchSlider = HorizontalSliderPtr(
			new HorizontalSlider("Sun Elevation", CoordPX(5.0f, 5.0f),
					CoordPX(200.0f, 45.0f), true, Integer(0), Integer(90),
					sunPitch));
	angleSlider = HorizontalSliderPtr(
			new HorizontalSlider("Azimuth", CoordPX(5.0f, 55.0f),
					CoordPX(200.0f, 45.0f), true, Integer(0), Integer(180),
					sunAngle));
	methodSelection= SelectionPtr(
				new Selection("Method", CoordPerPX(1.0f, 0.0f, -150.0f, 5.0f),
						CoordPX(145.0f, 30.0f), std::vector<std::string> {
								"Hosek","Preetham" }));
	methodSelection->setSelectedIndex(0);
	renderRegion->add(methodSelection);
	renderRegion->add(pitchSlider);
	renderRegion->add(angleSlider);
	rootNode.add(renderRegion);
	return true;

}
void ProceduralSkyEx::draw(AlloyContext* context){
	//Recompute lighting at every draw pass.
	if(methodSelection->getSelectedIndex()==0){
		hosekShader.setSunPositionDegrees(sunAngle.toFloat(),sunPitch.toFloat());
		hosekShader.update();
		hosekShader.draw(context->pixelRatio*renderRegion->getBounds());
	} else {
		preethamShader.setSunPositionDegrees(sunAngle.toFloat(),sunPitch.toFloat());
		preethamShader.update();
		preethamShader.draw(context->pixelRatio*renderRegion->getBounds());
	}
}

