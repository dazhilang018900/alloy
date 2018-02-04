/*
 * AlloyMinMaxFlow.h
 *
 *  Created on: Feb 3, 2018
 *      Author: blake
 */

#ifndef INCLUDE_CORE_ALLOYMINMAXFLOW_H_
#define INCLUDE_CORE_ALLOYMINMAXFLOW_H_

#include <AlloyMath.h>
#include <list>
namespace aly {
class MinMaxFlow {
public:
	struct Node;
	struct Edge;
private:
	std::vector<Node> nodes;
	std::vector<std::shared_ptr<Edge>> edges;
	std::list<Node*> activeList;
	std::list<Node*> orphanList;
	std::vector<Node*> changeList;
	int iterationCount;
	float totalFlow;
	float augment(Edge* joinEdge);
public:
	enum class NodeType {
		Source = 0, Sink = 1, Unknown = 2
	};
	Edge* ROOT = (Edge*) -1;
	Edge* ORPHAN = (Edge*) -2;
	struct Node {
		size_t id;
		//Node* next;
		std::vector<Edge*> edges;
		std::vector<Node*> children;
		Edge *parent;	// node's parent
		uint64_t timestamp;			// timestamp showing when DIST was computed
		int pathLength;		// distance to the terminal
		NodeType type;// flag showing whether the node is in the source or in the sink tree (if parent!=NULL)
		bool active;
		bool marked;	// set by mark_node()
		bool changed; // set by maxflow if
		float treeCapacity; // if tr_cap > 0 then tr_cap is residual capacity of the edge SOURCE->node, otherwise         -tr_cap is residual capacity of the edge node->SINK
		Node(size_t id = -1) :
				id(id), parent(nullptr), timestamp(0), pathLength(0), type(
						NodeType::Unknown), active(false), marked(false), changed(
						false), treeCapacity(0.0f) {
		}
	};
	struct Edge {
	protected:
		float forwardCapacity;
		float reverseCapacity;
	public:
		Node* source = nullptr;
		Node* target = nullptr;
		Edge(Node* src = nullptr, Node* tar = nullptr, float fwd_cap = 0.0f,
				float rev_cap = 0.0f) :
				source(src), target(tar), forwardCapacity(fwd_cap), reverseCapacity(
						rev_cap) {
			if (src != nullptr) {
				src->edges.push_back(this);
			} else if (src != nullptr) {
				tar->edges.push_back(this);
			}
		}
		Node* other(Node* n) const {
			if (n == source)
				return target;
			if (n == target)
				return source;
			throw std::runtime_error("Node does not attached to edge");
		}
		const float& getCapacity(Node* n) const {
			if (n == source)
				return forwardCapacity;
			if (n == target)
				return reverseCapacity;
			throw std::runtime_error("Node does not attached to edge");
			return forwardCapacity;
		}
		float& getCapacity(Node* n) {
			if (n == source)
				return forwardCapacity;
			if (n == target)
				return reverseCapacity;
			throw std::runtime_error("Node does not attached to edge");
			return forwardCapacity;
		}
		const float& getReverseCapacity(Node* n) const {
			if (n == source)
				return reverseCapacity;
			if (n == target)
				return forwardCapacity;
			throw std::runtime_error("Node does not attached to edge");
			return reverseCapacity;
		}
		float& getReverseCapacity(Node* n) {
			if (n == source)
				return reverseCapacity;
			if (n == target)
				return forwardCapacity;
			throw std::runtime_error("Node does not attached to edge");
			return reverseCapacity;
		}

		Node* next(Node* n) const {
			if (n == source)
				return target;
			if (n == target)
				return source;
			return nullptr;
		}
	};
	MinMaxFlow(size_t nodeCount);
	void initialize();
	void setCapacity(size_t nodeId, float cap);
	void setSource(size_t nodeId);
	void setSink(size_t nodeId);
	void step();
	std::shared_ptr<Edge> addEdge(int startId, int endId, float fwd_cap,
			float rev_cap);
	std::shared_ptr<Edge> addEdge(int2 edge, float fwd_cap, float rev_cap) {
		return addEdge(edge.x, edge.y, fwd_cap, rev_cap);
	}
};

}
#endif /* INCLUDE_CORE_ALLOYMINMAXFLOW_H_ */
