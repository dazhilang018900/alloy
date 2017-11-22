/*
 * EndlessNode.h
 *
 *  Created on: Nov 21, 2017
 *      Author: blake
 */

#ifndef INCLUDE_GRID_ENDLESSNODE_H_
#define INCLUDE_GRID_ENDLESSNODE_H_

#include "AlloyMath.h"
#include <array>
namespace aly {
template<typename T> class EndlessNode {
	static const std::shared_ptr<EndlessNode<T>> NULL_NODE;
	int id;
	EndlessNode<T>* parent;
	int dim;
	std::vector<int> indexes;
	std::vector<T> data;
	std::vector<std::shared_ptr<EndlessNode<T>>> children;
	EndlessNode(int dim) :
			id(-1), parent(nullptr), dim(dim) {
		indexes.resize(dim * dim * dim, -1);
	}
	EndlessNode(int D, EndlessNode<T>* parent, int id) :
			id(id), parent(parent), dim(D) {
		indexes.resize(-1);
	}
	const T& operator()(int i, int j, int k) const {
		return data[indexes[i + (j + k * dim) * dim]];
	}
	T& operator()(int i, int j, int k) {
		return data[indexes[i + (j + k * dim) * dim]];
	}
	const int& getIndex(int i, int j, int k) const {
		return indexes[i + (j + k * dim) * dim];
	}
	int& getIndex(int i, int j, int k) {
		return indexes[i + (j + k * dim) * dim];
	}

	bool hasChild(int i, int j, int k) const {
		int idx = operator()(i, j, k);
		return (idx < 0 || idx >= children.size());
	}
	std::shared_ptr<EndlessNode<T>> addChild(int i, int j, int k, int d) {
		int& idx = getIndex(i, j, k);
		idx = (int) children.size();
		std::shared_ptr<EndlessNode<T>> node = std::shared_ptr(
				new EndlessNode<T>(d, this, idx));
		children.push_back(node);
		return node;
	}
	std::shared_ptr<EndlessNode<T>> getChild(int i, int j, int k) const {
		int idx = getIndex(i, j, k);
		if (idx < 0 || idx >= children.size())
			return NULL_NODE;
		return children[idx];
	}
};
}

#endif /* INCLUDE_GRID_ENDLESSNODE_H_ */
