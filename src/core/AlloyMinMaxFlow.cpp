/*
 * AlloyMinMaxFlow.cpp
 *
 *  Created on: Feb 3, 2018
 *      Author: blake
 */

#include <AlloyMinMaxFlow.h>
namespace aly {
MinMaxFlow::MinMaxFlow(size_t sz) :
		nodes(sz), iterationCount(0) {
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
		if (node.type == NodeType::Source) {
			node.parent = ROOT;
			node.active = true;
			activeList.push_back(&node);
			node.pathLength = 1;
			foundSource = true;
		} else if (node.type == NodeType::Sink) {
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
	//Follow forward paths
	while (1) {
		a = pivot->parent;
		if (a == ROOT)
			break;
		cap = a->getCapacity(pivot);
		if (bottleneck > cap)
			bottleneck = cap;
		pivot = a->other(pivot);
	}
	cap = -pivot->treeCapacity;
	if (bottleneck > cap)
		bottleneck = cap;

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
		if (a->getReverseCapacity(pivot) <= 0) {
			pivot->parent = ORPHAN;
			orphanList.push_front(pivot);
		}
	}
	pivot->treeCapacity -= bottleneck;
	if (pivot->treeCapacity <= 0) {
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
		if (a->getReverseCapacity(pivot) <= 0) {
			pivot->parent = ORPHAN;
			orphanList.push_front(pivot);
		}
	}
	pivot->treeCapacity += bottleneck;
	if (!pivot->treeCapacity) {
		pivot->parent = ORPHAN;
		orphanList.push_front(pivot);
	}
	return bottleneck;
}
void MinMaxFlow::step() {
	Node* pivot = nullptr;
	Node* nbr;
	while (1) {
		if (pivot == nullptr) {
			for (auto iter = activeList.begin(); iter != activeList.end();
					iter++) {
				Node* node = *iter;
				if (!node->active) { //remove inactive
					activeList.erase(iter);
					if (activeList.size() > 0)
						iter--;
				}
				if (node->parent != nullptr) { //node has parent, it's active
					pivot = node;
					break;
				}
			}
			if (pivot == nullptr)
				break;
		}
		Edge* joinEdge = nullptr;
		if (pivot->type == NodeType::Source) {
			for (Edge* a : pivot->edges) {
				if (a->getCapacity(pivot) > 0) {
					nbr = a->next(pivot);
					if (nbr->parent == nullptr) {
						nbr->type = NodeType::Source;
						nbr->parent = a;
						nbr->timestamp = pivot->timestamp;
						nbr->pathLength = pivot->pathLength + 1;
						nbr->active = true;
						activeList.push_back(nbr);
						nbr->changed = true;
						changeList.push_back(nbr);
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
				if (a->getCapacity(pivot) > 0) {
					nbr = a->next(pivot);
					if (!nbr->parent) {
						nbr->type = NodeType::Sink;
						nbr->parent = a;
						nbr->timestamp = pivot->timestamp;
						nbr->pathLength = pivot->pathLength + 1;
						nbr->active = true;
						activeList.push_back(nbr);
						nbr->changed = true;
						changeList.push_back(nbr);
					} else if (nbr->type != NodeType::Source) {
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
			pivot->active = true;
			totalFlow += augment(joinEdge);
			/*
			while (!orphanList.empty()) {
				Node* node = orphanList.front();
				orphanList.pop_front();
				auto ptr = changeList.find(node);
				if (ptr != changeList.end())
					changedList.erase(ptr);
				if (node->isSink)
					processSinkOrphan(node);
				else
					processSourceOrphan(node);
			}
			*/
			/* adoption end */
		} else {
			//pivot invalid, find new active node
			pivot = nullptr;
		}
	}
}
std::shared_ptr<MinMaxFlow::Edge> MinMaxFlow::addEdge(int startId, int endId,
		float fwd_cap, float rev_cap) {
	std::shared_ptr<Edge> edge(
			new Edge(&nodes[startId], &nodes[endId], fwd_cap, rev_cap));
	edges.push_back(edge);
	return edge;
}
}

