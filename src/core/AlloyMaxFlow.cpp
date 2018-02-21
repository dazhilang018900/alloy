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

#include <AlloyMaxFlow.h>
#include <AlloyImage.h>
namespace aly {
MaxFlow::Edge* MaxFlow::ROOT = (MaxFlow::Edge*) -1;
MaxFlow::Edge* MaxFlow::ORPHAN = (MaxFlow::Edge*) -2;
const float MaxFlow::ZERO_TOLERANCE = 1E-8f;
const float FastMaxFlow::ZERO_TOLERANCE = 1E-8f;
const float FastMaxFlow::INF_FLOW = std::numeric_limits<float>::max();
const size_t FastMaxFlow::OUT_OF_BOUNDS = std::numeric_limits<size_t>::max();
const int FastMaxFlow::INF_DISTANCE = std::numeric_limits<int>::max();
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
	}
	if (tar != nullptr) {
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
		nodes[id] = Node(id);
	}
}
void MaxFlow::reset() {
	nodes.clear();
	edges.clear();
	activeList.clear();
	orphanList.clear();
	iterationCount = 0;
	totalFlow = 0;
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
	std::vector<size_t> order(nodes.size());
	for (size_t i = 0; i < nodes.size(); i++) {
		order[i] = i;
	}
	aly::Shuffle(order);
	for (size_t i : order) {
		Node& node = nodes[i];
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
			node.active = false;
			node.type = NodeType::Unknown;
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
	bottleneck = joinEdge->getForwardCapacity(srcNode);
	//std::cout << "Initial bottleneck " << bottleneck << std::endl;
	pivot = srcNode;
	//Backtrack towards source
	while (1) {
		a = pivot->parent;
		if (a == ROOT || a == nullptr)
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
		if (a == ROOT || a == nullptr)
			break;
		cap = a->getForwardCapacity(pivot);
		if (bottleneck > cap)
			bottleneck = cap;
		pivot = a->other(pivot);
	}
	cap = -pivot->treeCapacity;
	if (bottleneck > cap)
		bottleneck = cap;
	joinEdge->getReverseCapacity(srcNode) += bottleneck;
	joinEdge->getForwardCapacity(srcNode) -= bottleneck;
	pivot = srcNode;
	//Backtrack towards source
	while (1) {
		a = pivot->parent;
		if (a == ROOT || a == nullptr)
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
		if (a == ROOT || a == nullptr)
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
void MaxFlow::addSourceCapacity(int i, float cap) {
	addNodeCapacity(i, cap, 0);
}
void MaxFlow::addSinkCapacity(int i, float cap) {
	addNodeCapacity(i, 0, cap);
}

const MaxFlow::Node* MaxFlow::Edge::other(const Node* n) const {
	if (n == source)
		return target;
	if (n == target)
		return source;
	throw std::runtime_error(
			MakeString() << "other()::Node does not attach to edge. "
					<< ((n == nullptr) ?
							std::string("null") : (MakeString() << *n)) << " | "
					<< *this);
}
MaxFlow::Node* MaxFlow::Edge::other(const Node* n) {
	if (n == source)
		return target;
	if (n == target)
		return source;
	throw std::runtime_error(
			MakeString() << "other()::Node does not attach to edge. "
					<< ((n == nullptr) ?
							std::string("null") : (MakeString() << *n)) << " | "
					<< *this);
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
	throw std::runtime_error(
			"getReverseCapacity()::Node does not attach to edge");
	return reverseCapacity;
}
float& MaxFlow::Edge::getReverseCapacity(Node* n) {
	if (n == source)
		return reverseCapacity;
	if (n == target)
		return forwardCapacity;
	throw std::runtime_error(
			"getReverseCapacity()::Node does not attach to edge");
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

void MaxFlow::addNodeCapacity(int i, float sourceCapacity, float sinkCapacity) {
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
	for (Edge* edge : pivot->edges) {
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
					while (next != nullptr && next->timestamp != iterationCount) {
						next->timestamp = iterationCount;
						next->pathLength = d--;
						next = next->getParent();
					}
				}
			}
		}
	}
	pivot->parent = minEdge;
	if (pivot->parent != nullptr) {
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
					while (next != nullptr && next->timestamp != iterationCount) {
						next->timestamp = iterationCount;
						next->pathLength = d--;
						next = next->getParent();
					}
				}
			}
		}
	}
	pivot->parent = minEdge;
	if (pivot->parent != nullptr) {
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
void MaxFlow::solve(
		const std::function<bool(const std::string& message, float progress)>& monitor) {
	initialize();
	if (monitor) {
		if (!monitor("Solving Max-Flow ...", 0.0f))
			return;
	}
	while (step()) {
		if (monitor) {
			size_t solved = 0;
			for (Node& n : nodes) {
				if (n.type != NodeType::Unknown) {
					solved++;
				}
			}
			if (!monitor(
					MakeString() << "Solving Max-Flow [" << totalFlow << "]",
					solved / (float) nodes.size()))
				break;
		}
	}
}
std::shared_ptr<MaxFlow::Edge> MaxFlow::addEdge(int startId, int endId,
		float fwd_cap, float rev_cap) {
	assert(startId >= 0 && startId < nodes.size());
	assert(endId >= 0 && endId < nodes.size());
	std::shared_ptr<Edge> edge(
			new Edge(&nodes[startId], &nodes[endId], fwd_cap, rev_cap));
	edges.push_back(edge);
	return edge;
}

bool MaxFlow::step() {
	Node* pivot = nullptr;
	Node* nbr;
	for (Node* node : activeList) {
		if (node->active) {
			if (node->parent != nullptr) { //node has parent, it belongs to a tree
				pivot = node;
				pivot->active = false;
				break;
			}
		}
	}
	if (pivot == nullptr) {
		return false;
	}
	//std::cout << "Pivot " << *pivot << " Total Flow=" << totalFlow<<" Active="<<activeList.size()<<std::endl;
	while (1) {
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
			pivot->active = true;
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
			if (pivot->parent == nullptr) {
				break;
			}
		} else {
			break;
		}
	}
	const int UPDATE_INTERVAL = 256;
	if (iterationCount % UPDATE_INTERVAL == 0) {
		for (auto iter = activeList.begin(); iter != activeList.end(); iter++) {
			Node* node = *iter;
			if (!node->active) {
				activeList.erase(iter);
				if (!activeList.empty()) {
					iter--;
				} else {
					break;
				}
			}
		}
	}
	return true;
}
FastMaxFlow::FastMaxFlow(int w, int h) :
		width(w), height(h), nbrX( { 0, 0, 1, -1 }), nbrY( { 1, -1, 0, 0 }), reverse(
				{ 1, 0, 3, 2 }) {
	resize(w, h);
}
void FastMaxFlow::resize(int w, int h) {
	width = w;
	height = h;
	for (int k = 0; k < 4; k++) {
		edgeCapacity[k].resize(width * height, 0.0f);
	}
	excessFlow.resize(width * height, 0.0f);
	distField.resize(width * height, INF_DISTANCE);
	labels.resize(width * height, 2);
}
void FastMaxFlow::reset() {
	size_t N = width * (size_t) height;
	for (int k = 0; k < 4; k++) {
		edgeCapacity[k].assign(N, 0.0f);
	}
	excessFlow.assign(N, 0.0f);
	distField.assign(N, INF_DISTANCE);
	labels.resize(N, 2);
}
size_t FastMaxFlow::index(int i, int j) const {
	assert(i >= 0);
	assert(j >= 0);
	assert(i < width);
	assert(j < height);

	return i + j * (size_t) width;
}
size_t FastMaxFlow::index(int i, int j, int dir) const {

	int ii = i + nbrX[dir];
	int jj = j + nbrY[dir];
	if (ii >= 0 && jj >= 0 && ii < width && jj < height) {
		return ii + jj * (size_t) width;
	} else {
		return OUT_OF_BOUNDS;
	}
}
void FastMaxFlow::push(size_t x, size_t y, int k) {
	float flow = std::min(edgeCapacity[k][x], excessFlow[x]);
	float delta = excessFlow[x] - flow;
	if (delta < ZERO_TOLERANCE) {
		excessFlow[x] = 0.0f;
		flow+=delta;
	} else {
		excessFlow[x] = delta;
	}
	excessFlow[y]+=flow;
	edgeCapacity[k][x] -= flow;
	edgeCapacity[reverse[k]][y] += flow;
}

bool FastMaxFlow::relabel(int i, int j) {
	int d = INF_DISTANCE;
	size_t idx = index(i, j);
	for (int k = 0; k < 4; k++) {
		size_t y = index(i, j, k);
		if (y != OUT_OF_BOUNDS && distField[y] != INF_DISTANCE
				&& edgeCapacity[k][idx] > 0) {
			d = std::min(distField[y] + 1, d);
		}
	}
	if (d != distField[idx]) {
		distField[idx] = d;
		return true;
	}
	return false;
}
void FastMaxFlow::initialize(bool bfsInit) {
	size_t N = width * (size_t) height;
	for (int idx = 0; idx < N; idx++) {
		if (excessFlow[idx] < ZERO_TOLERANCE) {
			distField[idx] = 0;
		}
	}
	iterationCount = 0;
	const int shiftX[] = { 0, 0, 1, 1 };
	const int shiftY[] = { 0, 1, 0, 1 };
	int changeCount;
	if (bfsInit) {
		do {
			changeCount = 0;
			for (int c = 0; c < 4; c++) {
#pragma omp parallel for reduction(+:changeCount)
				for (int j = 0; j < height; j += 2) {
					for (int i = 0; i < width; i += 2) {
						int ii = i + shiftX[c];
						int jj = j + shiftY[c];
						if (ii < width && jj < height) {
							size_t idx = index(ii, jj);
							if (excessFlow[idx] >= 0) {
								int d = INF_DISTANCE;
								for (int k = 0; k < 4; k++) {
									size_t y = index(ii, jj, k);
									if (y != OUT_OF_BOUNDS
											&& distField[y] != INF_DISTANCE
											&& edgeCapacity[k][idx] > 0) {
										d = std::min(distField[y] + 1, d);
									}
								}
								if (d != distField[idx] && d != INF_DISTANCE) {
									changeCount++;
									distField[idx] = d;
								}
							}
						}
					}
				}
			}
		} while (changeCount > 0);
	}

}
bool FastMaxFlow::step() {
	size_t N = width * (size_t) height;
	int changeCount = 0;
	const int shiftX[] = { 0, 0, 1, 1 };
	const int shiftY[] = { 0, 1, 0, 1 };
	for (int k = 0; k < 4; k++) {
#pragma omp parallel for
		for (int j = 0; j < height; j += 2) {
			for (int i = 0; i < width; i += 2) {
				int ii = i + shiftX[k];
				int jj = j + shiftY[k];
				if (ii < width && jj < height) {
					size_t idx = index(ii, jj);
					if (excessFlow[idx] > 0) {// && distField[idx] < N
						relabel(ii, jj);
					}
				}
			}
		}
	}
	for (int k = 0; k < 4; k++) {
#pragma omp parallel for
		for (int j = 0; j < height; j++) {
			for (int i = 0; i < width; i++) {
				size_t x = index(i, j);
				size_t y = index(i, j, k);
				if (y != OUT_OF_BOUNDS && distField[y] == distField[x] - 1
						&& excessFlow[x] > 0 && edgeCapacity[k][x] > 0) {
					push(x, y, k);
				}
			}
		}
	}
	static const int UPDATE_INTERVAL = 32;
	if (iterationCount % UPDATE_INTERVAL == 0) {
#pragma omp parallel for reduction(+:changeCount)
		for (size_t idx = 0; idx < N; idx++) {
			int l = (excessFlow[idx] < 0) ? 1 : 0;
			if (l != labels[idx]) {
				labels[idx] = l;
				changeCount++;
			}
		}
		if (changeCount == 0)
			return false;
	}
	return true;
}
void FastMaxFlow::stash(int iter) {
	std::cout << "Stash " << width << " " << height << std::endl;
	Image4f sFlow(width, height);
	Image2f tFlow(width, height);
	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			size_t idx = index(i, j);
			int dist = distField[idx];
			sFlow(i, j) = float4(edgeCapacity[0][idx], edgeCapacity[1][idx],
					edgeCapacity[2][idx], edgeCapacity[3][idx]);
			tFlow(i, j) = float2(excessFlow[idx],
					std::min(dist, width * height));
		}
	}
	WriteImageToFile(MakeString() << GetDesktopDirectory() << ALY_PATH_SEPARATOR<<"sflow"<<std::setw(5)<<std::setfill('0')<<iter<<".xml",sFlow);
	WriteImageToFile(MakeString() << GetDesktopDirectory() << ALY_PATH_SEPARATOR<<"tflow"<<std::setw(5)<<std::setfill('0')<<iter<<".xml",tFlow);
}
void FastMaxFlow::solve(int iterations) {
	initialize(true);
	int iter = 0;
	stash(iter);
	while (iter < iterations && step()) {
		if (iter % 2000 == 0) {
			std::cout << "Iteration " << iter << std::endl;
			stash(iter);
		}
		iter++;
	}
	stash(iter);
}
void FastMaxFlow::setTerminalCapacity(int i, int j, float srcW, float sinkW) {
	size_t idx = index(i, j);
	excessFlow[idx] = srcW - sinkW;
}
void FastMaxFlow::setSourceCapacity(int i, int j, float w) {
	excessFlow[index(i, j)] += w;
}
void FastMaxFlow::setSinkCapacity(int i, int j, float w) {
	excessFlow[index(i, j)] += -w;
}
void FastMaxFlow::setEdgeCapacity(int i, int j, int dir, float w1, float w2) {
	edgeCapacity[dir][index(i, j)] = w1;
	edgeCapacity[reverse[dir]][index(i, j, dir)] = w2;
}

}

