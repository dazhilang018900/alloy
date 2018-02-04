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
#include "AlloyDelaunay.h"
#include "../../include/example/MaxFlowEx.h"
using namespace aly;
MaxFlowEx::MaxFlowEx() :
	Application(800, 800, "Max-Flow Example") {
}
bool MaxFlowEx::init(Composite& rootNode) {
	const int N = 300;
	const int K = 5;
	const float SEARCH_RADIUS = 0.2f;
	drawRegion = DrawPtr(new Draw("Draw Region", CoordPerPX(0.5f, 0.5f, -400, -400), CoordPX(800, 800)));
	rootNode.add(drawRegion);
	samples.resize(N);
	float minY=1E30f,maxY=-1E30f;
	for (int n = 0; n < N; n++) {
		float2 pt;
		samples[n] =pt= float2(RandomUniform(0.02f, 0.98f),
			RandomUniform(0.02f, 0.98f));
		if(pt.y<minY){
			minY=pt.y;
			srcId=n;
		}
		if(pt.y>maxY){
			maxY=pt.y;
			sinkId=n;
		}
	}

	std::vector<aly::uint3> delaunayTriangles;
	MakeDelaunay(samples, delaunayTriangles);
	for(uint3 tri:delaunayTriangles){
		for(int i=0;i<3;i++){
			int v1=tri[i];
			int v2=tri[(i+1)%3];
			edges.push_back(int2(v1,v2));
			float2 midPt=0.5f*(samples[v1]+samples[v2]);
			weights.push_back(2.0f*RandomUniform(0.5f, 1.0f)*std::abs(midPt.y-0.5f));
		}
	}
	edgeColors.resize(edges.size());
	labels.resize(samples.size());
	labels[srcId]=1;
	labels[sinkId]=-1;
	for(int i=0;i<edgeColors.size();i++){
		edgeColors[i]=Color(ColorMapToRGB(weights[i],ColorMap::RedToGreen));
	}
	drawRegion->onDraw = [this, SEARCH_RADIUS](const AlloyContext* context, const box2px& bounds) {
		NVGcontext* nvg = context->nvgContext;
		nvgLineJoin(nvg,NVG_ROUND);
		nvgStrokeColor(nvg, Color(64, 64, 64));
		nvgStrokeWidth(nvg, 2.0f);
		for (int n = 0;n < (int)edges.size();n++) {
			int2 edge = edges[n];
			float2 pt0 = samples[edge.x];
			float2 pt1 = samples[edge.y];
			pt0 = pt0*bounds.dimensions + bounds.position;
			pt1 = pt1*bounds.dimensions + bounds.position;
			nvgStrokeColor(nvg,edgeColors[n]);
			nvgBeginPath(nvg);
			nvgMoveTo(nvg, pt0.x, pt0.y);
			nvgLineTo(nvg, pt1.x, pt1.y);
			nvgStroke(nvg);
		}
		nvgFillColor(nvg, Color(128, 128, 128));
		int id=0;
		for (float2 pt : samples.data) {
			int l=labels[id++];
			nvgBeginPath(nvg);
			pt = pt*bounds.dimensions + bounds.position;
			if(l<0){
				nvgFillColor(nvg,Color(255,255,255));
				nvgCircle(nvg, pt.x, pt.y, 6.0f);
			} else if(l>0){
				nvgFillColor(nvg,Color(192,192,192));
				nvgCircle(nvg, pt.x, pt.y, 6.0f);
			} else {
				nvgFillColor(nvg,Color(64,64,64));
				nvgCircle(nvg, pt.x, pt.y, 4.0f);
			}
			nvgFill(nvg);
		}
	};
	drawRegion->onMouseOver = [this, K, SEARCH_RADIUS](const AlloyContext* context, const InputEvent& event) {
		box2px bounds = drawRegion->getBounds();
		cursor = event.cursor;
		float2 pt = (cursor - bounds.position) / bounds.dimensions;
		return true;
	};
	return true;
}

