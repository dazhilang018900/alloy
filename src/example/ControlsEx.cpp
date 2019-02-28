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
#include "example/ControlsEx.h"
using namespace aly;
ControlsEx::ControlsEx() :
		Application(800, 600, "Controls Example") {
}
bool ControlsEx::init(Composite& rootNode) {

	HSliderPtr hsliderNoLabel = HSliderPtr(
			new HorizontalSlider("Integer Slider", CoordPerPX(0.1f, 0.3f, 0, 0),
					CoordPX(200.0f, 40.0f), false,Integer(1), Integer(12),
					Integer(7)));

	HSliderPtr hsliderLabel = HSliderPtr(
			new HorizontalSlider("Horizontal Slider", CoordPercent(0.5f, 0.7f),
					CoordPX(350.0f, 50.0f),Double(1), Double(12), Double(7)));
	hsliderLabel->setLabelFormatter([](const Number& num){
		std::string str=MakeString()<<std::setprecision(4)<<num.toFloat();
		return str;
	});
	VSliderPtr vslider1 = VSliderPtr(
			new VerticalSlider("Vert Slider", CoordPerPX(0.85f, 0.1f, 0, 0),
					CoordPX(100.0f, 200.0f), Integer(0), Integer(100),
					Integer(20)));
	vslider1->setLabelFormatter([](const Number& num){
		std::string str=MakeString()<<num.toInteger()<<"%";
		return str;
	});

	CheckBoxPtr checkbox = CheckBoxPtr(
			new CheckBox("Check", CoordPX(10.0f, 100.0f),
					CoordPercent(0.4f, 0.07f), false,true));




	RangeSliderPtr rangeSlider = RangeSliderPtr(
		new RangeSlider("Range", CoordPerPX(0.5f,0.0f,10.0f, 130.0f),
			CoordPX(250.0f,50.0f),Integer(0), Integer(100), Integer(25), Integer(75)));
	SelectionPtr selection1 = SelectionPtr(
			new Selection("SF District", CoordPX(5, 350), CoordPX(200, 30),
					std::vector<std::string> { "Civic Center", "Tenderloin",
							"Nob Hill", "Mission", "Potrero", "Hayes Valley",
							"Noe Valey", "Bernal Heights", "Presidio",
							"Financial District", "SoMa", "Haight", "Richmond",
							"Sunset", "Chinatown", "Japantown", "Nob Hill" }));




	SelectionPtr selection2 = SelectionPtr(
			new Selection("LA District", CoordPX(25, 550), CoordPX(200, 30),
					std::vector<std::string> { "Hermosa Beach","Torrance","Long Beach","Venice","Santa Monica","Manhattan Beach","Hollywood","Paramount","Echo Park","Pasadena" }));



	ToggleBoxPtr togglebox = ToggleBoxPtr(
			new ToggleBox("Toggle", CoordPX(200.0f, 40.0f),
					CoordPercent(0.4f, 0.07f), false,true));
	TextButtonPtr textButton = TextButtonPtr(
			new TextButton("Text Button", CoordPX(10.0f, 300.0f),
					CoordPX(100, 30)));

	TextLabelPtr textLabel = TextLabelPtr(
			new TextLabel("Label", CoordPX(220.0f, 300.0f), CoordPX(100, 30)));
	textLabel->backgroundColor = MakeColor(128, 0, 0);
	textLabel->borderColor = MakeColor(200, 200, 200);
	textLabel->borderWidth = UnitPX(1.0f);
	textLabel->setRoundCorners(true);
	textLabel->horizontalAlignment = HorizontalAlignment::Center;
	textLabel->verticalAlignment = VerticalAlignment::Middle;

	ModifiableLabelPtr modifyLabel = ModifiableLabelPtr(
		new ModifiableLabel("Mod Label", CoordPerPX(0.5f,1.0f,20.0f,-50.0f), CoordPX(160, 40)));
	modifyLabel->backgroundColor = MakeColor(32,32,32);
	modifyLabel->borderColor = MakeColor(64,64,64);
	modifyLabel->textColor = MakeColor(Color(51, 153, 255));
	modifyLabel->borderWidth = UnitPX(1.0f);
	modifyLabel->fontSize = UnitPX(30);
	modifyLabel->setRoundCorners(false);
	modifyLabel->setValue("Click to edit");
	modifyLabel->horizontalAlignment = HorizontalAlignment::Center;
	modifyLabel->verticalAlignment = VerticalAlignment::Middle;
	ModifiableNumberPtr modifyNumber = ModifiableNumberPtr(
		new ModifiableNumber("Mod Number", CoordPerPX(0.5f, 1.0f,100.0f, -100.0f), CoordPX(80, 30),NumberType::Float));
	modifyNumber->backgroundColor = MakeColor(32, 32, 32);
	modifyNumber->borderColor = MakeColor(64, 64, 64);
	modifyNumber->textColor = MakeColor(Color(51, 153, 255));
	modifyNumber->borderWidth = UnitPX(1.0f);
	modifyNumber->fontSize = UnitPX(24);
	modifyNumber->setRoundCorners(false);
	modifyNumber->setValue("3.14");
	modifyNumber->horizontalAlignment = HorizontalAlignment::Center;
	modifyNumber->verticalAlignment = VerticalAlignment::Middle;

	ColorSelectorPtr colorselect = ColorSelectorPtr(
			new ColorSelector("Color", CoordPercent(0.5f, 0.4f),
					CoordPX(200, 50)));
	colorselect->setOrigin(Origin::MiddleCenter);
	colorselect->setColor(Color(200, 128, 32));

	ProgressBarPtr pbar = ProgressBarPtr(
			new ProgressBar("Progress", CoordPercent(0.05f, 0.7f),
					CoordPercent(0.4f, 0.05f)));


	TextIconButtonPtr textIcon = std::shared_ptr<TextIconButton>(
			new TextIconButton("Text Icon", 0xf115,
					CoordPerPX(1.0f, 1.0f, -110.0f, -40.0f), CoordPX(100, 30)));

	IconButtonPtr iconButton = std::shared_ptr<IconButton>(
			new IconButton(0xf062, CoordPerPX(1.0f, 0.0f, -35.0f, 5.0f),
					CoordPX(30, 30)));

	ProgressCirclePtr progressCircle=ProgressCirclePtr(new ProgressCircle("Progress Circle",CoordPerPX(0.0f,0.8f,240.0f,-5.0f),CoordPX(120.0f,120.0f)));

	TextFieldPtr tfield = MakeTextField("Text Field", CoordPerPX(0.0f, 0.8f,2.0f,0.0f),
			CoordPX(200.0f, 50.0f), Theme::Default.LIGHT,
			Theme::Default.DARKER);
	NumberFieldPtr ifield = MakeNumberField("Integer Field",
			CoordPerPX(0.5f, 0.5f, 0.0f, 0.0f), CoordPX(200.0f, 30.0f),
			NumberType::Integer, Theme::Default.LIGHT,
			Theme::Default.DARKER);
	NumberFieldPtr bfield = MakeNumberField("Binary Field",
			CoordPerPX(0.5f, 0.5f, 0.0f, 35.0f), CoordPX(200.0f, 30.0f),
			NumberType::Boolean, Theme::Default.LIGHT,
			Theme::Default.DARKER);
	bfield->setShowDefaultLabel(true);
	ifield->setShowDefaultLabel(true);
	NumberFieldPtr ffield = MakeNumberField("Float Field",
			CoordPercent(0.1f, 0.4f), CoordPX(200.0f, 30.0f), NumberType::Float,
			Theme::Default.LIGHT, Theme::Default.DARKER);
	SearchBoxPtr serachBox = SearchBoxPtr(new SearchBox("Search", CoordPerPX(0.5f, 0.5f, 0.0f,70.0f), CoordPX(200.0f, 30.0f)));
	TextLinkPtr lfield=TextLinkPtr(new TextLink("Hyperlink",CoordPerPX(0.5f, 0.5f, 220.0f, 35.0f), CoordPX(120.0f,30.0f)));
	lfield->setAlignment(HorizontalAlignment::Center,VerticalAlignment::Middle);
	lfield->onClick=[=](){
		std::cout<<"Clicked link!"<<std::endl;
		return true;
	};
	rootNode.add(togglebox,true);
	rootNode.add(iconButton,true);
	rootNode.add(checkbox,true);
	rootNode.add(rangeSlider,true);
	rootNode.add(vslider1,true);
	rootNode.add(hsliderNoLabel,true);
	rootNode.add(colorselect,true);
	rootNode.add(ffield,true);
	rootNode.add(textButton,true);
	rootNode.add(textLabel,true);
	rootNode.add(selection1,true);
	rootNode.add(ifield,true);
	rootNode.add(bfield,true);
	rootNode.add(lfield,true);
	rootNode.add(serachBox,true);
	rootNode.add(pbar,true);
	rootNode.add(tfield,true);
	rootNode.add(progressCircle,true);
	rootNode.add(selection2,true);
	rootNode.add(hsliderLabel,true);
	rootNode.add(modifyNumber,true);
	rootNode.add(modifyLabel,true);
	rootNode.add(textIcon,true);
	togglebox->setFocus(true);
	progressTask = std::unique_ptr<aly::RecurrentTask>(
			new RecurrentTask([pbar,progressCircle](uint64_t iter) {
				//std::cout << "Iteration " << iter << std::endl;
					pbar->setValue("Task Executing ...",(iter%21) / 20.0f);
					progressCircle->setValue(MakeString()<<(iter%21)<<" of 20", (iter%21) / 20.0f);
					return true;
				}, [pbar,progressCircle]() {
					pbar->setValue("Task Complete.", 1.0f);
					progressCircle->setValue("20 of 20", 1.0f);
				}, 300));
	progressTask->execute();


	return true;
}

