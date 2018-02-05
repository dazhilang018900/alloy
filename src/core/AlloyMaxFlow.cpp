/*
 * AlloyMinMaxFlow.cpp
 *
 *  Created on: Feb 3, 2018
 *      Author: blake
 */

#include <AlloyMaxFlow.h>
namespace aly {
MaxFlow::Edge* MaxFlow::ROOT = (MaxFlow::Edge*) -1;
MaxFlow::Edge* MaxFlow::ORPHAN = (MaxFlow::Edge*) -2;
const float MaxFlow::ZERO_TOLERANCE = 1E-8f;
MaxFlow::Node* MaxFlow::Node::getParent() {
	if (parent != nullptr && parent != MaxFlow::ROOT
			&& parent != MaxFlow::ORPHAN) {
		return parent->other(this);
	}
	return nullptr;
}
MaxFlow::Edge::Edge(Node* src, Node* tar, float fwd_cap, float rev_cap) :
		source(src), target(tar), forwardCapacity(fwd_cap), reverseCapacity(
				rev_cap) {
	if (src != nullptr) {
		src->edges.push_back(this);
	} else if (tar != nullptr) {
		tar->edges.push_back(this);
	}
}
const MaxFlow::Node* MaxFlow::Node::getParent() const {
	if (parent != nullptr && parent != MaxFlow::ROOT
			&& parent != MaxFlow::ORPHAN) {
		return parent->other(this);
	}
	return nullptr;
}
MaxFlow::MaxFlow(size_t sz) :
		iterationCount(0), totalFlow(0) {
	resize(sz);
}
void MaxFlow::resize(size_t sz) {
	nodes.resize(sz);
	totalFlow = 0;
	for (size_t id = 0; id < nodes.size(); id++) {
		nodes[id].id = id;
	}
}
void MaxFlow::setSource(size_t nodeId) {
	nodes[nodeId].type = NodeType::Source;
}
void MaxFlow::setSink(size_t nodeId) {
	nodes[nodeId].type = NodeType::Sink;
}
void MaxFlow::setCapacity(size_t nodeId, float cap) {
	nodes[nodeId].treeCapacity = cap;
}
void MaxFlow::initialize() {
	activeList.clear();
	orphanList.clear();
	iterationCount = 0;
	bool foundSource = false;
	bool foundSink = false;
	for (Node& node : nodes) {
		node.children.clear();
		node.marked = false;
		node.pathLength = 0;
		node.timestamp = 0;
		if (node.treeCapacity > 0) {
			node.type = NodeType::Source;
			node.parent = ROOT;
			node.active = true;
			activeList.push_back(&node);
			node.pathLength = 1;
			foundSource = true;
		} else if (node.treeCapacity < 0) {
			node.type = NodeType::Sink;
			node.parent = ROOT;
			node.active = true;
			activeList.push_back(&node);
			node.pathLength = 1;
			foundSink = true;
		} else {
			node.parent = nullptr;
		}
	}
	if (!foundSink || !foundSource) {
		throw std::runtime_error("Could not find source and/or sink.");
	}
}
float MaxFlow::augment(Edge *joinEdge) {
	Node* srcNode = nullptr;
	Node* sinkNode = nullptr;
	if (joinEdge->source->type == NodeType::Source) {
		srcNode = joinEdge->source;
	} else if (joinEdge->target->type == NodeType::Source) {
		srcNode = joinEdge->target;
	}
	if (joinEdge->source->type == NodeType::Sink) {
		sinkNode = joinEdge->source;
	} else if (joinEdge->target->type == NodeType::Sink) {
		sinkNode = joinEdge->target;
	}
	Node *pivot;
	Edge *a;
	float bottleneck, cap;
	/* 1. Finding bottleneck capacity */
	/* 1a - the source tree */
	bottleneck = joinEdge->getForwardCapacity(srcNode);
	//std::cout << "Initial bottleneck " << bottleneck << std::endl;
	pivot = srcNode;
	//Backtrack towards source
	while (1) {
		a = pivot->parent;
		if (a == ROOT)
			break;
		cap = a->getReverseCapacity(pivot);
		if (bottleneck > cap)
			bottleneck = cap;
		pivot = a->other(pivot);
	}
	cap = pivot->treeCapacity;
	if (bottleneck > cap) {
		bottleneck = cap;
	}
	pivot = sinkNode;
	//Follow forward paths to sink
	while (1) {
		a = pivot->parent;
		if (a == ROOT)
			break;
		cap = a->getForwardCapacity(pivot);
		if (bottleneck > cap)
			bottleneck = cap;
		pivot = a->other(pivot);
	}
	cap = -pivot->treeCapacity;
	if (bottleneck > cap)
		bottleneck = cap;

	//std::cout << "Bottleneck " << bottleneck << std::endl;
	/* 2. Augmenting */
	/* 2a - the source tree */
	joinEdge->getReverseCapacity(srcNode) += bottleneck;
	joinEdge->getForwardCapacity(srcNode) -= bottleneck;
	pivot = srcNode;
	//Backtrack towards source

	while (1) {
		a = pivot->parent;
		if (a == ROOT)
			break;
		a->getForwardCapacity(pivot) += bottleneck;
		a->getReverseCapacity(pivot) -= bottleneck;
		if (a->getReverseCapacity(pivot) <= ZERO_TOLERANCE) {
			pivot->parent = ORPHAN;
			orphanList.push_front(pivot);
		}
		pivot = a->other(pivot);
	}
	pivot->treeCapacity -= bottleneck;
	if (pivot->treeCapacity <= ZERO_TOLERANCE) {
		pivot->parent = ORPHAN;
		orphanList.push_front(pivot);
	}

	pivot = sinkNode;
	while (1) {
		a = pivot->parent;
		if (a == ROOT)
			break;
		a->getReverseCapacity(pivot) += bottleneck;
		a->getForwardCapacity(pivot) -= bottleneck;
		if (a->getForwardCapacity(pivot) <= ZERO_TOLERANCE) {
			pivot->parent = ORPHAN;
			orphanList.push_back(pivot);
		}
		pivot = a->other(pivot);
	}

	pivot->treeCapacity += bottleneck;

	if (pivot->treeCapacity >= -ZERO_TOLERANCE) {
		pivot->parent = ORPHAN;
		orphanList.push_front(pivot);
	}
	return bottleneck;
}
void MaxFlow::setSourceCapacity(int i, float cap) {
	setNodeCapacity(i, cap, 0);
}
void MaxFlow::setSinkCapacity(int i, float cap) {
	setNodeCapacity(i, 0, cap);
}


const MaxFlow::Node* MaxFlow::Edge::other(const Node* n) const {
	if (n == source)
		return target;
	if (n == target)
		return source;
	throw std::runtime_error(MakeString()<<"other()::Node does not attach to edge. "<<((n==nullptr)?std::string("null"):(MakeString()<<*n))<<" | "<<*this);
}
MaxFlow::Node* MaxFlow::Edge::other(const Node* n) {
	if (n == source)
		return target;
	if (n == target)
		return source;
	throw std::runtime_error(MakeString()<<"other()::Node does not attach to edge. "<<((n==nullptr)?std::string("null"):(MakeString()<<*n))<<" | "<<*this);
}

const float& MaxFlow::Edge::getForwardCapacity(Node* n) const {
	if (n == source)
		return forwardCapacity;
	if (n == target)
		return reverseCapacity;
	throw std::runtime_error("getCapacity()::Node does not attach to edge");
	return forwardCapacity;
}
float& MaxFlow::Edge::getForwardCapacity(Node* n) {
	if (n == source)
		return forwardCapacity;
	if (n == target)
		return reverseCapacity;
	throw std::runtime_error("getCapacity()::Node does not attach to edge");
	return forwardCapacity;
}
const float& MaxFlow::Edge::getReverseCapacity(Node* n) const {
	if (n == source)
		return reverseCapacity;
	if (n == target)
		return forwardCapacity;
	throw std::runtime_error("getReverseCapacity()::Node does not attach to edge");
	return reverseCapacity;
}
float& MaxFlow::Edge::getReverseCapacity(Node* n) {
	if (n == source)
		return reverseCapacity;
	if (n == target)
		return forwardCapacity;
	throw std::runtime_error("getReverseCapacity()::Node does not attach to edge");
	return reverseCapacity;
}

MaxFlow::Node* MaxFlow::Edge::next(Node* n) const {
	if (n == source)
		return target;
	if (n == target)
		return source;
	throw std::runtime_error("next()::Node does not attach to edge");
	return nullptr;
}


void MaxFlow::setNodeCapacity(int i, float sourceCapacity,
		float sinkCapacity) {
	float delta = nodes[i].treeCapacity;
	if (delta > 0) {
		sourceCapacity += delta;
	} else {
		sinkCapacity -= delta;
	}
	totalFlow +=
			(sourceCapacity < sinkCapacity) ? sourceCapacity : sinkCapacity;
	nodes[i].treeCapacity = sourceCapacity - sinkCapacity; //negative for sink capacity
}
void MaxFlow::processSourceOrphan(Node *pivot) {
	static const int MAX_PATH_LENGTH = std::numeric_limits<int>::max();
	Node *next;
	Edge *minEdge = nullptr;
	Edge *a;
	int d;
	int minLength = MAX_PATH_LENGTH;
	for (Edge* edge : pivot->edges){
		if (edge->getReverseCapacity(pivot) > ZERO_TOLERANCE) {
			next = edge->other(pivot);
			if (next->parent != nullptr && next->type == NodeType::Source) {
				d = 0;
				while (1) {
					if (next->timestamp == iterationCount) {
						d += next->pathLength;
						break;
					}
					a = next->parent;
					d++;
					if (a == ROOT) {
						next->timestamp = iterationCount;
						next->pathLength = 1;
						break;
					}
					if (a == ORPHAN) {
						d = MAX_PATH_LENGTH;
						break;
					}
					next = a->other(next);
				}
				if (d < MAX_PATH_LENGTH) {
					if (d < minLength) {
						minEdge = edge;
						minLength = d;
					}
					next = edge->other(pivot);
					while (next!=nullptr&&next->timestamp != iterationCount) {
						next->timestamp = iterationCount;
						next->pathLength = d--;
						next = next->getParent();
					}
				}
			}
		}
	}
	pivot->parent = minEdge;
	if (pivot->parent!=nullptr) {
		pivot->timestamp = iterationCount;
		pivot->pathLength = minLength + 1;
	} else {
		for (Edge* a0 : pivot->edges) {
			next = a0->other(pivot);
			a = next->parent;
			if (a != nullptr && next->type == NodeType::Source) {
				if (a0->getReverseCapacity(pivot) > ZERO_TOLERANCE) {
					activeList.push_back(next);
					next->active = true;
				}
				if (a != ROOT && a != ORPHAN && a->other(next) == pivot) {
					orphanList.push_back(next);
					next->parent = ORPHAN;
				}
			}
		}
	}
}

void MaxFlow::processSinkOrphan(Node *pivot) {
	static const int MAX_PATH_LENGTH = std::numeric_limits<int>::max();
	Node *next;
	Edge* minEdge = nullptr;
	Edge* a;
	int d;
	int minLength = MAX_PATH_LENGTH;
	for (Edge* edge : pivot->edges) {
		if (edge->getForwardCapacity(pivot) > ZERO_TOLERANCE) {
			next = edge->other(pivot);
			if (next->parent != nullptr && next->type == NodeType::Sink) {
				d = 0;
				while (1) {
					if (next->timestamp == iterationCount) {
						d += next->pathLength;
						break;
					}
					a = next->parent;
					d++;
					if (a == ROOT) {
						next->timestamp = iterationCount;
						next->pathLength = 1;
						break;
					}
					if (a == ORPHAN) {
						d = MAX_PATH_LENGTH;
						break;
					}
					next = a->other(next);
				}
				if (d < MAX_PATH_LENGTH) {
					if (d < minLength) {
						minEdge = edge;
						minLength = d;
					}
					next = edge->other(pivot);
					while (next!=nullptr&&next->timestamp != iterationCount) {
						next->timestamp = iterationCount;
						next->pathLength = d--;
						next = next->getParent();
					}
				}
			}
		}
	}
	pivot->parent = minEdge;
	if (pivot->parent!=nullptr) {
		pivot->timestamp = iterationCount;
		pivot->pathLength = minLength + 1;
	} else {
		for (Edge* a0 : pivot->edges) {
			next = a0->other(pivot);
			a = next->parent;
			if (a != nullptr && next->type == NodeType::Sink) {
				if (a0->getForwardCapacity(pivot) > ZERO_TOLERANCE) {
					activeList.push_back(next);
					next->active = true;
				}
				if (a != ROOT && a != ORPHAN && a->other(next) == pivot) {
					orphanList.push_back(next);
					next->parent = ORPHAN;
				}
			}
		}
	}
}
void MaxFlow::solve(){
	Node* pivot = nullptr;
	Node* nbr;
	initialize();
	while (1) {
		if (pivot == nullptr) {
			for (Node* node : activeList) {
				if (node->active) {
					if (node->parent != nullptr) { //node has parent, it belongs to a tree
						pivot = node;
						break;
					}
				}
			}
			if (pivot == nullptr)
				break;
		}
		std::cout<<"Pivot "<<*pivot<<" "<<totalFlow<<std::endl;
		pivot->active = false;
		Edge* joinEdge = nullptr;
		if (pivot->type == NodeType::Source) {
			for (Edge* a : pivot->edges) {
				if (a->getForwardCapacity(pivot) > ZERO_TOLERANCE) {
					nbr = a->next(pivot);
					if (nbr->parent == nullptr) {
						nbr->type = NodeType::Source;
						nbr->parent = a;
						nbr->timestamp = pivot->timestamp;
						nbr->pathLength = pivot->pathLength + 1;
						nbr->active = true;
						activeList.push_back(nbr);
					} else if (nbr->type == NodeType::Sink) {
						joinEdge = a;
						break;
					} else if (nbr->timestamp <= pivot->timestamp
							&& nbr->pathLength > pivot->pathLength) {
						nbr->parent = a;
						nbr->timestamp = pivot->timestamp;
						nbr->pathLength = pivot->pathLength + 1;
					}
				}
			}
		} else if (pivot->type == NodeType::Sink) {
			for (Edge* a : pivot->edges) {
				if (a->getReverseCapacity(pivot) > ZERO_TOLERANCE) {
					nbr = a->next(pivot);
					if (!nbr->parent) {
						nbr->type = NodeType::Sink;
						nbr->parent = a;
						nbr->timestamp = pivot->timestamp;
						nbr->pathLength = pivot->pathLength + 1;
						nbr->active = true;
						activeList.push_back(nbr);
					} else if (nbr->type == NodeType::Source) {
						joinEdge = a;
						break;
					} else if (nbr->timestamp <= pivot->timestamp
							&& nbr->pathLength > pivot->pathLength) {
						nbr->parent = a;
						nbr->timestamp = pivot->timestamp;
						nbr->pathLength = pivot->pathLength + 1;
					}
				}
			}
		}
		iterationCount++;
		if (joinEdge != nullptr) {
			totalFlow += augment(joinEdge);
			while (!orphanList.empty()) {
				Node* node = orphanList.front();
				orphanList.pop_front();
				if (node->type == NodeType::Sink) {
					processSinkOrphan(node);
				} else if (node->type == NodeType::Source) {
					processSourceOrphan(node);
				}
			}
			if(pivot->parent==nullptr){
				pivot=nullptr;
			}
		} else {
			pivot = nullptr;
		}
		for (auto iter = activeList.begin(); iter != activeList.end(); iter++) {
			Node* node = *iter;
			if (!node->active) {
				activeList.erase(iter);
				if (!activeList.empty()){
					iter--;
				} else {
					break;
				}
			}
		}

	}
}

std::shared_ptr<MaxFlow::Edge> MaxFlow::addEdge(int startId, int endId,
		float fwd_cap, float rev_cap) {
	assert(startId>=0&&startId<nodes.size());
	assert(endId>=0&&endId<nodes.size());
	std::shared_ptr<Edge> edge(
			new Edge(&nodes[startId], &nodes[endId], fwd_cap, rev_cap));
	edges.push_back(edge);
	return edge;
}
}

