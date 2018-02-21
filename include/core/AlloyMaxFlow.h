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

 -------------------------------------------------------------------------------
 This class implements the maxflow algorithm described in
 "An Experimental Comparison of Min-Cut/Max-Flow Algorithms for Energy Minimization in Vision."
 Yuri Boykov and Vladimir Kolmogorov. In IEEE Transactions on Pattern Analysis and Machine Intelligence (PAMI),
 September 2004

 */

#ifndef INCLUDE_CORE_ALLOYMAXFLOW_H_
#define INCLUDE_CORE_ALLOYMAXFLOW_H_

#include <AlloyMath.h>
#include <list>
namespace aly {
class MaxFlow {
public:
	struct Node;
	struct Edge;
private:
	std::vector<Node> nodes;
	std::vector<std::shared_ptr<Edge>> edges;
	std::list<Node*> activeList;
	std::list<Node*> orphanList;
	uint64_t iterationCount;
	float totalFlow;
	float augment(Edge* joinEdge);
	void processSourceOrphan(Node *i);
	void processSinkOrphan(Node *i);
public:
	enum class NodeType {
		Source = 0, Sink = 1, Orphan = 2, Unknown = 3
	};
	static const float ZERO_TOLERANCE;
	static Edge* ROOT;
	static Edge* ORPHAN;
	struct Node {
		size_t id;
		std::vector<Edge*> edges;
		Edge *parent;
		uint64_t timestamp;
		int pathLength;
		NodeType type;
		bool active;
		float treeCapacity; // tree capacity to source >0 or sink <0
		Node(size_t id = -1) :
				id(id), parent(nullptr), timestamp(0), pathLength(0), type(
						NodeType::Unknown), active(false), treeCapacity(0.0f) {
		}
		const Node* getParent() const;
		Node* getParent();

	};
	struct Edge {
		float forwardCapacity;
		float reverseCapacity;
		Node* source = nullptr;
		Node* target = nullptr;
		Edge(Node* src = nullptr, Node* tar = nullptr, float fwd_cap = 0.0f,
				float rev_cap = 0.0f);
		const Node* other(const Node* n) const;
		Node* other(const Node* n);
		const float& getForwardCapacity(Node* n) const;
		float& getForwardCapacity(Node* n);
		const float& getReverseCapacity(Node* n) const;
		float& getReverseCapacity(Node* n);
		Node* next(Node* n) const;
	};
	MaxFlow(size_t sz = 0);
	inline std::vector<Node>& getNodes() {
		return nodes;
	}
	inline std::vector<std::shared_ptr<Edge>>& getEdges() {
		return edges;
	}
	size_t getActiveListSize() const {
		return activeList.size();
	}
	float getTotalFlow() const {
		return totalFlow;
	}
	void solve(
			const std::function<bool(const std::string& message, float progress)>& monitor);
	bool step();
	void reset();
	void resize(size_t sz);
	void addNodeCapacity(int i, float sourceCapacity, float sinkCapacity);
	void addSourceCapacity(int i, float cap);
	void addSinkCapacity(int i, float cap);
	void initialize();
	void setCapacity(size_t nodeId, float cap);
	void setSource(size_t nodeId);
	void setSink(size_t nodeId);
	std::shared_ptr<Edge> addEdge(int startId, int endId, float fwd_cap,
			float rev_cap);
	std::shared_ptr<Edge> addEdge(int2 edge, float fwd_cap, float rev_cap) {
		return addEdge(edge.x, edge.y, fwd_cap, rev_cap);
	}
};
class FastMaxFlow {
protected:
	static const size_t OUT_OF_BOUNDS;
	static const int INF_DISTANCE;
	static const float INF_FLOW;
	static const float ZERO_TOLERANCE;
	int width;
	int height;
	size_t iterationCount;
	const int nbrX[4];
	const int nbrY[4];
	const int reverse[4];
	std::vector<float> excessFlow;
	std::vector<int> distField;
	std::vector<uint8_t> labels;
	std::vector<float> edgeCapacity[4];
	size_t index(int i, int j) const;
	size_t index(int i, int j, int dir) const;
	void push(size_t x,size_t y,int k);
	bool relabel(int i, int j);
public:
	FastMaxFlow(int width = 0, int height = 0);
	void initialize(bool bfsInit=true);
	bool step();
	void resize(int width, int height);
	void reset();
	void solve(int iterations);
	void stash(int iter);
	inline int getDistance(int i, int j) const {
		return distField[index(i, j)];
	}
	inline int getDistance(size_t idx) const {
		return distField[idx];
	}
	inline float getFlow(size_t idx) const {
		return excessFlow[idx];
	}
	inline float getFlow(int i, int j) const {
		return excessFlow[index(i, j)];
	}
	inline int getLabel(int i, int j) const {
		return labels[index(i,j)];
	}
	inline int getLabel(size_t idx) const {
		return labels[idx];
	}
	void setSourceCapacity(int i, int j, float w);
	void setSinkCapacity(int i, int j, float w);
	void setTerminalCapacity(int i, int j, float srcW, float sinkW);
	void setEdgeCapacity(int i, int j, int dir, float w1, float w2);
};
template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const MaxFlow::NodeType& n) {
	switch (n) {
	case MaxFlow::NodeType::Source:
		ss << "Source";
		break;
	case MaxFlow::NodeType::Sink:
		ss << "Sink";
		break;
	case MaxFlow::NodeType::Orphan:
		ss << "Orphan";
		break;
	case MaxFlow::NodeType::Unknown:
	default:
		ss << "Unknown";
		break;
	}
	return ss;
}
template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const MaxFlow::Node& n) {
	ss << "<" << n.id << " [" << n.type << "] "
			<< ((n.parent == MaxFlow::ROOT) ?
					"Root" :
					((n.parent == MaxFlow::ORPHAN) ?
							"Orphan" :
							((n.parent != nullptr) ?
									std::string(
											MakeString() << n.getParent()->id) :
									"NULL"))) << " cap=" << n.treeCapacity
			<< " len=" << n.pathLength << " time=" << n.timestamp << ">";
	return ss;
}
template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const MaxFlow::Edge& e) {
	ss << "[" << e.source->id << "::" << e.forwardCapacity << ","
			<< e.target->id << "::" << e.reverseCapacity << "]";
	return ss;
}
}
#endif /* INCLUDE_CORE_ALLOYMAXFLOW_H_ */
