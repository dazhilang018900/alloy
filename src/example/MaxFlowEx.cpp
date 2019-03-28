/*
 * Copyright(C) 2018, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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
#include "graphics/AlloyDelaunay.h"
#include "example/MaxFlowEx.h"
using namespace aly;
MaxFlowEx::MaxFlowEx() :
		Application(1000, 1000, "Max-Flow Example"), srcId(-1), sinkId(-1) {
}
void MaxFlowEx::updateRender() {
	std::vector<MaxFlow::Node>& nodes = flow.getNodes();
	for (int i = 0; i < nodes.size(); i++) {
		if (nodes[i].parent == MaxFlow::ORPHAN) {
			nodeLabels[i] = static_cast<int>(MaxFlow::NodeType::Orphan);
		} else {
			nodeLabels[i] = static_cast<int>(nodes[i].type);
		}
	}
	nodeLabels[srcId] = static_cast<int>(MaxFlow::NodeType::Source);
	nodeLabels[sinkId] = static_cast<int>(MaxFlow::NodeType::Sink);
	std::set<aly::int2> saturatedSet;
	for (std::shared_ptr<MaxFlow::Edge> edge : flow.getEdges()) {
		if (edge->forwardCapacity <= MaxFlow::ZERO_TOLERANCE || edge->reverseCapacity <= MaxFlow::ZERO_TOLERANCE) {
			saturatedSet.insert(int2(edge->source->id, edge->target->id));
		}
	}
	for (int i = 0; i < edgeColors.size(); i++) {
		int2 e = edges[i];
		if (nodes[e.y].getParent() == &nodes[e.x]) {
			edgeLabels[i] = 1;
		}
		if (nodes[e.x].getParent() == &nodes[e.y]) {
			edgeLabels[i] = -1;
		} else {
			if (e.x == srcId || e.x == sinkId) {
				edgeLabels[i] = 1;
			} else if (e.y == srcId || e.y == sinkId) {
				edgeLabels[i] = -1;
			}
		}
		float w = weights[i];
		if (saturatedSet.find(e) != saturatedSet.end()) {
			saturated[i] = true;
		} else {
			saturated[i] = false;
		}
		edgeColors[i] = HSVtoColor(HSV(0, 0, 0.1f + w * 0.8f));
	}
}
bool MaxFlowEx::init(Composite& rootNode) {
	int M = 24;
	const int K = 5;
	const float SEARCH_RADIUS = 1.75f / M;
	drawRegion = DrawPtr(
			new Draw("Draw Region", CoordPercent(0.0f, 0.0f),
					CoordPercent(1.0f, 1.0f)));
	rootNode.add(drawRegion);
	float sourceDist = 1E30f, sinkDist = 1E30f;
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < M; j++) {
			float2 pt = float2((i + RandomUniform(0.2, 0.8)) / M,
					(j + RandomUniform(0.2, 0.8)) / M);
			int n = samples.size();
			samples.push_back(pt);
			float d = distanceSqr(pt, float2(0.5f, 0.0f));
			if (d < sourceDist) {
				sourceDist = d;
				srcId = n;
			}
			d = distanceSqr(pt, float2(0.5f, 1.0f));
			if (d < sinkDist) {
				sinkDist = d;
				sinkId = n;
			}
		}
	}
	int N = samples.size();
	flow.resize(N);
	{
		Locator2f locator(samples);
		const float ANGLE = 30.0f;
		float2 norm = float2(-sin(ANGLE * ALY_PI / 180.0f),
				cos(ANGLE * ALY_PI / 180.0f));
		for (int n = 0; n < samples.size(); n++) {
			std::vector<float2i> nbrs;
			locator.closest(samples[n], SEARCH_RADIUS, nbrs);
			int v1 = n;
			int counter = 0;
			for (float2i nbr : nbrs) {
				int v2 = nbr.index;
				if (v1 >= v2)
					continue;
				float w1 = clamp(RandomUniform(0.9f,1.1f)*aly::sqr(2.0f*dot(samples[v1] - float2(0.5f, 0.5f), norm)),0.0001f,1.0f);
				float w2 = clamp(RandomUniform(0.9f,1.1f)*aly::sqr(2.0f*dot(samples[v2] - float2(0.5f, 0.5f), norm)),0.0001f,1.0f);
				edges.push_back(int2(v1, v2));
				weights.push_back(0.5f * (w1 + w2));
				if (v1 == srcId) {
					flow.addSourceCapacity(v2, w2);
				} else if (v2 == srcId) {
					flow.addSourceCapacity(v1, w1);
				} else if (v1 == sinkId) {
					flow.addSinkCapacity(v2, w2);
				} else if (v2 == sinkId) {
					flow.addSinkCapacity(v1, w1);
				} else {
					//Internal edge
					flow.addEdge(v1, v2, w1, w2);
				}
			}
		}
	}
	flow.initialize();
	edgeColors.resize(edges.size());
	edgeLabels.resize(edges.size());
	nodeLabels.resize(samples.size());
	saturated.resize(edgeLabels.size(), false);

	updateRender();
	drawRegion->onDraw =
			[this, SEARCH_RADIUS](const AlloyContext* context, const box2px& bounds) {
				NVGcontext* nvg = context->getNVG();
				nvgLineJoin(nvg,NVG_ROUND);
				nvgStrokeColor(nvg, Color(64, 64, 64));
				nvgStrokeWidth(nvg, 2.0f);
				if(flow.step()){
					updateRender();
				}
				std::vector<std::shared_ptr<MaxFlow::Edge>>& edgePtrs=flow.getEdges();
				for (int n = 0;n < (int)edges.size();n++) {
					int2 edge = edges[n];
					int dir=edgeLabels[n];
					float2 pt0 = samples[edge.x];
					float2 pt1 = samples[edge.y];
					pt0 = pt0*bounds.dimensions + bounds.position;
					pt1 = pt1*bounds.dimensions + bounds.position;
					if(saturated[n]) {
						nvgStrokeColor(nvg,edgeColors[n]);
						nvgStrokeWidth(nvg, (dir!=0)?5.0f:3.0f);
						nvgBeginPath(nvg);
						nvgMoveTo(nvg, pt0.x, pt0.y);
						nvgLineTo(nvg, pt1.x, pt1.y);
						nvgStroke(nvg);

						nvgStrokeColor(nvg,Color(192,64,64));
						nvgStrokeWidth(nvg, 2.0f);
						nvgBeginPath(nvg);
						nvgMoveTo(nvg, pt0.x, pt0.y);
						nvgLineTo(nvg, pt1.x, pt1.y);
						nvgStroke(nvg);

					} else {
						nvgStrokeColor(nvg,edgeColors[n]);
						nvgStrokeWidth(nvg, (dir!=0)?5.0f:2.0f);
						nvgBeginPath(nvg);
						nvgMoveTo(nvg, pt0.x, pt0.y);
						nvgLineTo(nvg, pt1.x, pt1.y);
						nvgStroke(nvg);
					}
				}
				for (int n = 0;n < (int)edges.size();n++) {
					int2 edge = edges[n];
					int dir=edgeLabels[n];
					float2 pt0 = samples[edge.x];
					float2 pt1 = samples[edge.y];
					pt0 = pt0*bounds.dimensions + bounds.position;
					pt1 = pt1*bounds.dimensions + bounds.position;
					if(dir!=0) {
						if(nodeLabels[edge.x]==static_cast<int>(MaxFlow::NodeType::Source)){
							nvgFillColor(nvg,Color(64,64,192));
						} else if(nodeLabels[edge.x]==static_cast<int>(MaxFlow::NodeType::Sink)){
							nvgFillColor(nvg,Color(64,192,64));
						} else {
							nvgFillColor(nvg,Color(255,255,255,180));
						}
						float2 mid=0.5f*(pt0+pt1);
						float2 vec=((float)dir)*normalize(pt1-pt0);
						float2 orth(-vec.y,vec.x);
						const float arrowSize=4;
						nvgBeginPath(nvg);
						nvgMoveTo(nvg,mid.x+vec.x*arrowSize,mid.y+vec.y*arrowSize);
						nvgLineTo(nvg,mid.x+(orth.x-vec.x)*arrowSize,mid.y+(orth.y-vec.y)*arrowSize);
						nvgLineTo(nvg,mid.x+(-orth.x-vec.x)*arrowSize,mid.y+(-orth.y-vec.y)*arrowSize);
						nvgClosePath(nvg);
						nvgFill(nvg);
					}
				}
				nvgFillColor(nvg, Color(128, 128, 128));
				nvgStrokeColor(nvg,Color(255,255,255));
				nvgStrokeWidth(nvg,3.0f);
				int id=0;
				for (float2 pt : samples.data) {
					int l=nodeLabels[id];
					nvgBeginPath(nvg);
					pt = pt*bounds.dimensions + bounds.position;
					if(l==static_cast<int>(MaxFlow::NodeType::Source)) {
						nvgFillColor(nvg,Color(64,64,192));
						if(id==srcId) {
							nvgCircle(nvg, pt.x, pt.y,6.0f);
							nvgStroke(nvg);
						} else {
							nvgCircle(nvg, pt.x, pt.y,4.0f);
						}
					} else if(l==static_cast<int>(MaxFlow::NodeType::Sink)) {
						nvgFillColor(nvg,Color(64,192,64));
						if(id==sinkId) {
							nvgCircle(nvg, pt.x, pt.y,6.0f);
							nvgStroke(nvg);
						} else {
							nvgCircle(nvg, pt.x, pt.y,4.0f);
						}
					} else if(l==static_cast<int>(MaxFlow::NodeType::Orphan)) {
						nvgFillColor(nvg,Color(192,0,0));
						nvgCircle(nvg, pt.x, pt.y, 6.0f);
					} else if(l==static_cast<int>(MaxFlow::NodeType::Unknown)) {
						nvgFillColor(nvg,Color(192,192,192));
						nvgCircle(nvg, pt.x, pt.y, 4.0f);
					}
					nvgFill(nvg);
					id++;
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

