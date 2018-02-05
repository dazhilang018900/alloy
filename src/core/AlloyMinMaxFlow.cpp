/*
 * AlloyMinMaxFlow.cpp
 *
 *  Created on: Feb 3, 2018
 *      Author: blake
 */

#include <AlloyMinMaxFlow.h>
namespace aly {
MinMaxFlow::Edge* MinMaxFlow::ROOT = (MinMaxFlow::Edge*) -1;
MinMaxFlow::Edge* MinMaxFlow::ORPHAN = (MinMaxFlow::Edge*) -2;
const float MinMaxFlow::ZERO_TOLERANCE = 1E-6f;
MinMaxFlow::Node* MinMaxFlow::Node::getParent() {
	if (parent != nullptr && parent != MinMaxFlow::ROOT
			&& parent != MinMaxFlow::ORPHAN) {
		return parent->other(this);
	}
	return nullptr;
}
MinMaxFlow::Edge::Edge(Node* src, Node* tar, float fwd_cap, float rev_cap) :
		source(src), target(tar), forwardCapacity(fwd_cap), reverseCapacity(
				rev_cap) {
	if (src != nullptr) {
		src->edges.push_back(this);
	} else if (tar != nullptr) {
		tar->edges.push_back(this);
	}
}
const MinMaxFlow::Node* MinMaxFlow::Node::getParent() const {
	if (parent != nullptr && parent != MinMaxFlow::ROOT
			&& parent != MinMaxFlow::ORPHAN) {
		return parent->other(this);
	}
	return nullptr;
}
MinMaxFlow::MinMaxFlow(size_t sz) :
		iterationCount(0), totalFlow(0) {
	resize(sz);
}
void MinMaxFlow::resize(size_t sz) {
	nodes.resize(sz);
	totalFlow = 0;
	for (size_t id = 0; id < nodes.size(); id++) {
		nodes[id].id = id;
	}
}
void MinMaxFlow::setSource(size_t nodeId) {
	nodes[nodeId].type = NodeType::Source;
}
void MinMaxFlow::setSink(size_t nodeId) {
	nodes[nodeId].type = NodeType::Sink;
}
void MinMaxFlow::setCapacity(size_t nodeId, float cap) {
	nodes[nodeId].treeCapacity = cap;
}
void MinMaxFlow::initialize() {
	activeList.clear();
	orphanList.clear();
	changeList.clear();
	iterationCount = 0;
	bool foundSource = false;
	bool foundSink = false;
	for (Node& node : nodes) {
		node.children.clear();
		node.marked = false;
		node.changed = false;
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
float MinMaxFlow::augment(Edge *joinEdge) {
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
	bottleneck = joinEdge->getCapacity(srcNode);
	//std::cout << "Initial bottleneck " << bottleneck << std::endl;
	pivot = srcNode;
	//Backtrack towards source
	while (1) {
		a = pivot->parent;
		if (a == ROOT)
			break;
		std::cout << "Backtrack " << *pivot << " " << *a << std::endl;
		cap = a->getReverseCapacity(pivot);
		if (bottleneck > cap)
			bottleneck = cap;
		pivot = a->other(pivot);
	}
	cap = pivot->treeCapacity;
	std::cout << "Source Tree Capacity " << cap << std::endl;
	if (bottleneck > cap) {
		bottleneck = cap;
	}
	pivot = sinkNode;
	//Follow forward paths to sink
	while (1) {
		a = pivot->parent;
		if (a == ROOT)
			break;

		std::cout << "Track " << *pivot << " " << *a << std::endl;
		cap = a->getCapacity(pivot);
		if (bottleneck > cap)
			bottleneck = cap;
		pivot = a->other(pivot);
	}
	cap = -pivot->treeCapacity;
	std::cout << "Sink Tree Capacity " << cap << std::endl;
	if (bottleneck > cap)
		bottleneck = cap;

	//std::cout << "Bottleneck " << bottleneck << std::endl;
	/* 2. Augmenting */
	/* 2a - the source tree */
	joinEdge->getReverseCapacity(srcNode) += bottleneck;
	joinEdge->getCapacity(srcNode) -= bottleneck;
	pivot = srcNode;
	//Backtrack towards source

	while (1) {
		a = pivot->parent;
		if (a == ROOT)
			break;
		a->getCapacity(pivot) += bottleneck;
		a->getReverseCapacity(pivot) -= bottleneck;
		if (a->getReverseCapacity(pivot) <= ZERO_TOLERANCE) {
			pivot->parent = ORPHAN;
			orphanList.push_front(pivot);
		}
		std::cout<<"Backtrack again "<<*pivot<<std::endl;
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
		a->getCapacity(pivot) -= bottleneck;
		if (a->getCapacity(pivot) <= ZERO_TOLERANCE) {
			pivot->parent = ORPHAN;
			orphanList.push_back(pivot);
		}
		std::cout<<"Track again"<<*pivot<<std::endl;
		pivot = a->other(pivot);
	}
	pivot->treeCapacity += bottleneck;
	if (pivot->treeCapacity <= ZERO_TOLERANCE) {
		pivot->parent = ORPHAN;
		orphanList.push_back(pivot);
	}
	return bottleneck;
}
void MinMaxFlow::setSourceCapacity(int i, float cap) {
	setNodeCapacity(i, cap, 0);
}
void MinMaxFlow::setSinkCapacity(int i, float cap) {
	setNodeCapacity(i, 0, cap);
}


const MinMaxFlow::Node* MinMaxFlow::Edge::other(const Node* n) const {
	if (n == source)
		return target;
	if (n == target)
		return source;
	throw std::runtime_error(MakeString()<<"other()::Node does not attach to edge. "<<((n==nullptr)?std::string("null"):(MakeString()<<*n))<<" | "<<*this);
}
MinMaxFlow::Node* MinMaxFlow::Edge::other(const Node* n) {
	if (n == source)
		return target;
	if (n == target)
		return source;
	throw std::runtime_error(MakeString()<<"other()::Node does not attach to edge. "<<((n==nullptr)?std::string("null"):(MakeString()<<*n))<<" | "<<*this);
}

const float& MinMaxFlow::Edge::getCapacity(Node* n) const {
	if (n == source)
		return forwardCapacity;
	if (n == target)
		return reverseCapacity;
	throw std::runtime_error("getCapacity()::Node does not attach to edge");
	return forwardCapacity;
}
float& MinMaxFlow::Edge::getCapacity(Node* n) {
	if (n == source)
		return forwardCapacity;
	if (n == target)
		return reverseCapacity;
	throw std::runtime_error("getCapacity()::Node does not attach to edge");
	return forwardCapacity;
}
const float& MinMaxFlow::Edge::getReverseCapacity(Node* n) const {
	if (n == source)
		return reverseCapacity;
	if (n == target)
		return forwardCapacity;
	throw std::runtime_error("getReverseCapacity()::Node does not attach to edge");
	return reverseCapacity;
}
float& MinMaxFlow::Edge::getReverseCapacity(Node* n) {
	if (n == source)
		return reverseCapacity;
	if (n == target)
		return forwardCapacity;
	throw std::runtime_error("getReverseCapacity()::Node does not attach to edge");
	return reverseCapacity;
}

MinMaxFlow::Node* MinMaxFlow::Edge::next(Node* n) const {
	if (n == source)
		return target;
	if (n == target)
		return source;
	throw std::runtime_error("next()::Node does not attach to edge");
	return nullptr;
}


void MinMaxFlow::setNodeCapacity(int i, float sourceCapacity,
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
void MinMaxFlow::processSourceOrphan(Node *pivot) {
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
	if (pivot->parent == minEdge) {
		pivot->timestamp = iterationCount;
		pivot->pathLength = minLength + 1;
	} else {
		//changeList.push_back(pivot);
		pivot->changed = true;
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

void MinMaxFlow::processSinkOrphan(Node *pivot) {
	static const int MAX_PATH_LENGTH = std::numeric_limits<int>::max();
	Node *next;
	Edge* minEdge = nullptr;
	Edge* a;
	int d;
	int minLength = MAX_PATH_LENGTH;
	for (Edge* edge : pivot->edges) {
		if (edge->getCapacity(pivot) > ZERO_TOLERANCE) {
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
	if (pivot->parent == minEdge) {
		pivot->timestamp = iterationCount;
		pivot->pathLength = minLength + 1;
	} else {
		//changeList.push_back(pivot);
		pivot->changed = true;
		for (Edge* a0 : pivot->edges) {
			next = a0->other(pivot);
			a = next->parent;
			if (a != nullptr && next->type == NodeType::Sink) {
				if (a0->getCapacity(pivot) > ZERO_TOLERANCE) {
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

void MinMaxFlow::step() {
	Node* pivot = nullptr;
	Node* nbr;
	while (1) {
		if (pivot == nullptr) {
			for (Node* node : activeList) {
				if (node->active) {
					if (node->parent != nullptr) { //node has parent, it's not part of any tree
						pivot = node;
						break;
					}
				}
			}
			if (pivot == nullptr)
				break;
		}
		std::cout<<"New Pivot "<<*pivot<<std::endl;
		Edge* joinEdge = nullptr;
		if (pivot->type == NodeType::Source) {
			for (Edge* a : pivot->edges) {
				if (a->getCapacity(pivot) > ZERO_TOLERANCE) {
					nbr = a->next(pivot);
					//std::cout << "Source Neighbor " << *nbr << " " << *a<< std::endl;
					if (nbr->parent == nullptr) {
						nbr->type = NodeType::Source;
						nbr->parent = a;
						nbr->timestamp = pivot->timestamp;
						nbr->pathLength = pivot->pathLength + 1;

						nbr->active = true;
						activeList.push_back(nbr);

						nbr->changed = true;
						//changeList.push_back(nbr);
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
					//std::cout << "Sink Neighbor " << *nbr << " " << *a<< std::endl;
					if (!nbr->parent) {
						nbr->type = NodeType::Sink;
						nbr->parent = a;
						nbr->timestamp = pivot->timestamp;
						nbr->pathLength = pivot->pathLength + 1;

						nbr->active = true;
						activeList.push_back(nbr);

						nbr->changed = true;
						//changeList.push_back(nbr);
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

		pivot->active = false;
		iterationCount++;
		if (joinEdge != nullptr) {
			std::cout<<*joinEdge<<" "<<totalFlow<<std::endl;
			totalFlow += augment(joinEdge);
			std::cout<<"Done Augment"<<std::endl;
			/*
			while (!orphanList.empty()) {
				Node* node = orphanList.front();
				orphanList.pop_front();
				std::cout<<"Process Orphan "<<*node<<std::endl;
				if (node->type == NodeType::Sink) {
					processSinkOrphan(node);
				} else if (node->type == NodeType::Source) {
					processSourceOrphan(node);
				}
				/*
				 for (auto iter = changeList.begin(); iter != changeList.end();
				 iter++) {
				 if (*iter == node) {
				 node->changed = false;
				 changeList.erase(iter);
				 break;
				 }
				 }
				 */
			//}
			break;
		} else {
			pivot = nullptr;
		}
		std::cout<<"Active List Size "<<activeList.size()<<std::endl;
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

std::shared_ptr<MinMaxFlow::Edge> MinMaxFlow::addEdge(int startId, int endId,
		float fwd_cap, float rev_cap) {
	assert(startId>=0&&startId<nodes.size());
	assert(endId>=0&&endId<nodes.size());
	std::shared_ptr<Edge> edge(
			new Edge(&nodes[startId], &nodes[endId], fwd_cap, rev_cap));
	edges.push_back(edge);
	return edge;
}
}

