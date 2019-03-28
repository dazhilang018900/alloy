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
#include "example/NonNegativeLeastSquaresEx.h"
#include "ui/AlloyDrawUtil.h"
#include "math/AlloyOptimization.h"
using namespace aly;
NonNegativeLeastSquaresEx::NonNegativeLeastSquaresEx() :
	Application(600, 600, "Non-Negative Least Squares") {
}
void NonNegativeLeastSquaresEx::update(float2 qt) {
	int N=controlPoints.size();
	DenseMatrixFloat A(3+N,N);
	VecFloat b(3+N);
	float len=0.0f;
	for(int n=0;n<N;n++){
		len+=length(controlPoints[n]);
	}
	len=len/N;
	for(int n=0;n<N;n++){
		float2 pt=controlPoints[n];
		A(n,n)=1;
		A(n,(n+1)%N)=-0.5f;
		A(n,(n+N-1)%N)=-0.5f;
		A(N,n)=pt.x/len;
		A(N+1,n)=pt.y/len;
		A(N+2,n)=1;
		b[n]=0;
		weights[n]=1/N;
	}
	b[N]=qt.x/len;
	b[N+1]=qt.y/len;
	b[N+2]=1;
	if(constraintBox->getValue()){
		weights=SolveNonNegative(A,b,MatrixFactorization::SVD);
	} else {
		weights=Solve(A,b,MatrixFactorization::SVD);
	}
	error=length(A*weights-b)/weights.size();
	//std::cout<<"x="<<weights<<" Error="<<<<std::endl;
}
bool NonNegativeLeastSquaresEx::init(Composite& rootNode) {
	const int N=16;
	for(int n;n<N;n++){
		float r=(n%2==0)?RandomUniform(0.5f,0.9f):RandomUniform(0.3f,0.5f);
		float th=2*(n+RandomUniform(-0.25f,0.25f))*ALY_PI/N;
		controlPoints.push_back(float2(r*std::cos(th),r*std::sin(th)));
	}
	weights.resize(controlPoints.size(),0.0f);
	range = std::pair<float2, float2>(float2(-1, -1), float2(1, 1));
	draw = DrawPtr(new Draw("Control Points", CoordPX(0.0f,0.0f), CoordPercent(1.0f, 1.0f)));
	draw->setAspectRule(AspectRule::FixedHeight);
	draw->setAspectRatio(1.0f);
	error=0;
	draw->onDraw = [this](const AlloyContext* context, const box2px& bounds) {
		NVGcontext* nvg = context->getNVG();
		nvgTextAlign(nvg,NVG_ALIGN_TOP|NVG_ALIGN_LEFT);
		nvgFontFaceId(nvg,context->getFontHandle(FontType::Normal));
		nvgFontSize(nvg,20);
		drawText(nvg,bounds.position+float2(bounds.dimensions.x-115,2),MakeString()<<"Error="<<std::setw(6)<<std::setprecision(6)<<error,FontStyle::Normal,Color(128,128,128),Color(10,10,10));

		float r;
		nvgStrokeColor(nvg,Color(192,192,192));
		nvgStrokeWidth(nvg, 2.0f);
		nvgBeginPath(nvg);
		for (int n = 0;n <(int)controlPoints.size();n++) {
			float2 pt = controlPoints[n];
			pt = (pt - range.first) / (range.second - range.first);
			pt.y = 1.0f - pt.y;
			pt = pt*bounds.dimensions + bounds.position;
			if(n==0){
				nvgMoveTo(nvg,pt.x,pt.y);
			} else {
				nvgLineTo(nvg,pt.x,pt.y);
			}
		}
		nvgClosePath(nvg);
		nvgStroke(nvg);
		for (int n = 0;n <(int)controlPoints.size();n++) {
			if (n == selectedControlPoint) {
				nvgFillColor(nvg, Color(255,255,255));
				r = 8;
			} else {
				nvgFillColor(nvg, Color(192,192,192));
				r = 6;
			}
			nvgBeginPath(nvg);
			float2 pt = controlPoints[n];
			pt = (pt - range.first) / (range.second - range.first);
			pt.y = 1.0f - pt.y;
			pt = pt*bounds.dimensions + bounds.position;
			nvgCircle(nvg, pt.x, pt.y,r);
			nvgFill(nvg);
		}
		nvgTextAlign(nvg,NVG_ALIGN_BOTTOM|NVG_ALIGN_CENTER);
		nvgFontFaceId(nvg,context->getFontHandle(FontType::Code));
		nvgFontSize(nvg,20);
		for (int n = 0;n <(int)controlPoints.size();n++) {
			float2 pt = controlPoints[n];
			pt = (pt - range.first) / (range.second - range.first);
			pt.y = 1.0f - pt.y;
			pt = pt*bounds.dimensions + bounds.position;
			pt.y-=8;
			if(std::abs(weights[n])<1E-10f){
				drawText(nvg,pt,MakeString()<<std::setw(4)<<std::setprecision(3)<<weights[n],FontStyle::Outline,Color(96,96,96),Color(10,10,10));
			} else {
				if(weights[n]<0){
					drawText(nvg,pt,MakeString()<<std::setw(4)<<std::setprecision(3)<<weights[n],FontStyle::Outline,Color(200,60,60),Color(10,10,10));
				} else {
					drawText(nvg,pt,MakeString()<<std::setw(4)<<std::setprecision(3)<<weights[n],FontStyle::Outline,Color(200,200,200),Color(10,10,10));
				}
			}
		}

	};
	draw->onMouseDown = [this](const AlloyContext* context, const InputEvent& e) {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
			mouseDownControlPoint = selectedControlPoint;

		}

		return false;
	};
	draw->onMouseUp = [this](const AlloyContext* context, const InputEvent& e) {
		mouseDownControlPoint = -1;
		return false;
	};
	draw->onMouseOver=[this](const AlloyContext* context, const InputEvent& e) {
		box2px bounds = draw->getBounds();
		selectedControlPoint = -1;
		for (int n = 0;n <(int) controlPoints.size();n++) {
			float2 pt = (controlPoints[n] - range.first) / (range.second - range.first);
			pt.y = 1.0f - pt.y;
			pt = pt*bounds.dimensions + bounds.position;
			if (distanceSqr(pt, e.cursor) <36) {
				selectedControlPoint = n;
				break;
			}
		}
		if (context->isLeftMouseButtonDown()) {
			if (mouseDownControlPoint >= 0) {
				float2 pt = (e.cursor - bounds.position) / bounds.dimensions;
				pt.y = 1.0f - pt.y;
				pt = pt*(range.second - range.first) + range.first;
				controlPoints[mouseDownControlPoint] = pt;
			}
		}
		if(selectedControlPoint<=0){
			box2px bounds = draw->getBounds();
			float2 pt = (e.cursor - bounds.position) / bounds.dimensions;
			pt.y = 1.0f - pt.y;
			pt = pt*(range.second - range.first) + range.first;
			this->update(pt);
		}
		return false;
	};
	rootNode.add(draw);
	rootNode.add(constraintBox=CheckBoxPtr(
			new CheckBox("Non-Negative", CoordPX(5,5),
					CoordPX(145.0f, 30.0f), true, true)));

	return true;
}

