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
	void processSourceOrphan(Node *i);
	void processSinkOrphan(Node *i);
public:
	enum class NodeType {
		Source = 0, Sink = 1, Orphan=2, Unknown = 3
	};
	static const float ZERO_TOLERANCE;
	static Edge* ROOT;
	static Edge* ORPHAN;
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
		const float& getCapacity(Node* n) const;
		float& getCapacity(Node* n);
		const float& getReverseCapacity(Node* n) const;
		float& getReverseCapacity(Node* n);
		Node* next(Node* n) const;
	};
	MinMaxFlow(size_t sz=0);
	inline std::vector<Node>& getNodes(){
		return nodes;
	}
	inline std::vector<std::shared_ptr<Edge>>& getEdges(){
		return edges;
	}
	void resize(size_t sz);
	void setNodeCapacity(int i, float sourceCapacity, float sinkCapacity);
	void setSourceCapacity(int i, float cap);
	void setSinkCapacity(int i, float cap);
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
template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const MinMaxFlow::NodeType& n) {
	switch (n) {
	case MinMaxFlow::NodeType::Source:
		ss << "Source";
		break;
	case MinMaxFlow::NodeType::Sink:
		ss << "Sink";
		break;
	case MinMaxFlow::NodeType::Orphan:
		ss << "Orphan";
		break;
	case MinMaxFlow::NodeType::Unknown:
	default:
		ss << "Unknown";
		break;
	}
	return ss;
}
template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const MinMaxFlow::Node& n) {
	ss << "<" << n.id << " [" << n.type << "] cap=" << n.treeCapacity << " len="
			<< n.pathLength << " time=" << n.timestamp << ">";
	return ss;
}
template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const MinMaxFlow::Edge& e) {
	ss << "[" << e.source->id << "::" << e.forwardCapacity << ","
			<< e.target->id << "::" << e.reverseCapacity << "]";
	return ss;
}
}
#endif /* INCLUDE_CORE_ALLOYMINMAXFLOW_H_ */
