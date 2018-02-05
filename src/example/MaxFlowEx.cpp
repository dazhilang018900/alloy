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
		Application(800, 800, "Max-Flow Example"), srcId(-1), sinkId(-1) {
}
bool MaxFlowEx::init(Composite& rootNode) {
	int M = 16;
	const int K = 5;
	const float SEARCH_RADIUS = 0.2f;
	drawRegion = DrawPtr(
			new Draw("Draw Region", CoordPerPX(0.5f, 0.5f, -400, -400),
					CoordPX(800, 800)));
	rootNode.add(drawRegion);
	float minY = 1E30f, maxY = -1E30f;
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < M; j++) {
			float2 pt = float2((i + RandomUniform(0.2,0.8)) / M, (j + RandomUniform(0.2,0.8)) / M);
			int n = samples.size();
			samples.push_back(pt);
			if (pt.y < minY) {
				minY = pt.y;
				srcId = n;
			}
			if (pt.y > maxY) {
				maxY = pt.y;
				sinkId = n;
			}
		}
	}
	int N = samples.size();
	std::cout << "Source " << srcId << " Sink " << sinkId <<" "<<N<<std::endl;
	flow.resize(N);
	{
		std::vector<aly::uint3> delaunayTriangles;
		MakeDelaunay(samples, delaunayTriangles);
		for (uint3 tri : delaunayTriangles) {
			for (int i = 0; i < 3; i++) {
				int v1 = tri[i];
				int v2 = tri[(i + 1) % 3];
				edges.push_back(int2(v1, v2));
				float2 midPt = 0.5f * (samples[v1] + samples[v2]);
				float w = 2.0f * RandomUniform(0.5f, 1.0f)
						* std::abs(midPt.y - 0.5f);
				weights.push_back(w);
				if (v1 == srcId) {
					flow.setSourceCapacity(v2, w);
				} else if (v2 == srcId) {
					flow.setSourceCapacity(v1, w);
				} else if (v1 == sinkId) {
					flow.setSinkCapacity(v2, w);
				} else if (v2 == sinkId) {
					flow.setSinkCapacity(v1, w);
				} else {
					//Internal edge
					flow.addEdge(v1, v2, w, w);
				}
			}
		}
	}
	try {
		flow.initialize();
		flow.step();
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
	edgeColors.resize(edges.size());
	edgeLabels.resize(edges.size());
	nodeLabels.resize(samples.size());
	std::vector<MinMaxFlow::Node>& nodes = flow.getNodes();
	for (int i = 0; i < nodes.size(); i++) {
		if (nodes[i].parent == MinMaxFlow::ORPHAN) {
			nodeLabels[i] = static_cast<int>(MinMaxFlow::NodeType::Orphan);
		} else {
			nodeLabels[i] = static_cast<int>(nodes[i].type);
		}
	}
	nodeLabels[srcId] = static_cast<int>(MinMaxFlow::NodeType::Source);
	nodeLabels[sinkId] = static_cast<int>(MinMaxFlow::NodeType::Sink);

	for (int i = 0; i < edgeColors.size(); i++) {
		int2 e = edges[i];
		if (nodes[e.y].getParent() == &nodes[e.x]) {
			edgeLabels[i] = 1;
		}
		if (nodes[e.x].getParent() == &nodes[e.y]) {
			edgeLabels[i] = -1;
		} else {
			if(e.x==srcId||e.x==sinkId){
				edgeLabels[i] = 1;
			} else if(e.y==srcId||e.y==sinkId){
				edgeLabels[i] = -1;
			}
		}
		float w = weights[i];
		edgeColors[i] = HSVtoColor(HSV(0, 0, 0.1f + w * 0.8f));
	}
	drawRegion->onDraw =
			[this, SEARCH_RADIUS](const AlloyContext* context, const box2px& bounds) {
				NVGcontext* nvg = context->nvgContext;
				nvgLineJoin(nvg,NVG_ROUND);
				nvgStrokeColor(nvg, Color(64, 64, 64));
				nvgStrokeWidth(nvg, 2.0f);
				for (int n = 0;n < (int)edges.size();n++) {
					int2 edge = edges[n];
					int dir=edgeLabels[n];
					float2 pt0 = samples[edge.x];
					float2 pt1 = samples[edge.y];
					pt0 = pt0*bounds.dimensions + bounds.position;
					pt1 = pt1*bounds.dimensions + bounds.position;
					nvgStrokeColor(nvg,edgeColors[n]);

					nvgStrokeWidth(nvg, (dir!=0)?4.0f:2.0f);
					nvgBeginPath(nvg);
					nvgMoveTo(nvg, pt0.x, pt0.y);
					nvgLineTo(nvg, pt1.x, pt1.y);
					nvgStroke(nvg);
				}
				for (int n = 0;n < (int)edges.size();n++) {
					int2 edge = edges[n];
					int dir=edgeLabels[n];
					float2 pt0 = samples[edge.x];
					float2 pt1 = samples[edge.y];
					pt0 = pt0*bounds.dimensions + bounds.position;
					pt1 = pt1*bounds.dimensions + bounds.position;
					if(dir!=0) {
						nvgFillColor(nvg,Color(255,255,255,180));
						float2 mid=0.5f*(pt0+pt1);
						float2 vec=((float)dir)*normalize(pt1-pt0);
						float2 orth(-vec.y,vec.x);
						float s=6;
						nvgBeginPath(nvg);
						nvgMoveTo(nvg,mid.x+vec.x*s,mid.y+vec.y*s);
						nvgLineTo(nvg,mid.x+(orth.x-vec.x)*s,mid.y+(orth.y-vec.y)*s);
						nvgLineTo(nvg,mid.x+(-orth.x-vec.x)*s,mid.y+(-orth.y-vec.y)*s);
						nvgClosePath(nvg);
						nvgFill(nvg);
					}
				}
				nvgFillColor(nvg, Color(128, 128, 128));
				int id=0;
				for (float2 pt : samples.data) {
					int l=nodeLabels[id];
					nvgBeginPath(nvg);
					pt = pt*bounds.dimensions + bounds.position;
					if(l==static_cast<int>(MinMaxFlow::NodeType::Source)) {
						nvgFillColor(nvg,Color(32,32,128));
						nvgCircle(nvg, pt.x, pt.y,(id==srcId)? 6.0f:4.0f);
					} else if(l==static_cast<int>(MinMaxFlow::NodeType::Sink)) {
						nvgFillColor(nvg,Color(32,128,32));
						nvgCircle(nvg, pt.x, pt.y, (id==sinkId)? 6.0f:4.0f);
					} else if(l==static_cast<int>(MinMaxFlow::NodeType::Orphan)) {
						nvgFillColor(nvg,Color(255,0,0));
						nvgCircle(nvg, pt.x, pt.y, 6.0f);
					} else if(l==static_cast<int>(MinMaxFlow::NodeType::Unknown)) {
						nvgFillColor(nvg,Color(192,192,192));
						nvgCircle(nvg, pt.x, pt.y, 4.0f);
					}
					id++;
					nvgFill(nvg);
				}

			};
	drawRegion->onMouseOver =
			[this, K, SEARCH_RADIUS](const AlloyContext* context, const InputEvent& event) {
				box2px bounds = drawRegion->getBounds();
				cursor = event.cursor;
				float2 pt = (cursor - bounds.position) / bounds.dimensions;
				return true;
			};
	return true;
}

