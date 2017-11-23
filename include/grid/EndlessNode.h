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
public:
	const float BACKGROUND_VALUE=0.0f;
	int id;
	EndlessNode<T>* parent;
	int dim;
	std::vector<int> indexes;
	std::vector<T> data;
	std::vector<std::shared_ptr<EndlessNode<T>>> children;
	static const std::shared_ptr<EndlessNode<T>> NULL_NODE;

	EndlessNode(int dim, bool isLeaf) :
			id(-1), parent(nullptr), dim(dim) {
		if (isLeaf) {
			data.resize(dim * dim * dim, BACKGROUND_VALUE);
		} else {
			indexes.resize(dim * dim * dim, -1);
		}
	}
	EndlessNode(int D, bool isLeaf, EndlessNode<T>* parent, int id) :
			id(id), parent(parent), dim(D) {
		if (isLeaf) {
			data.resize(dim * dim * dim, BACKGROUND_VALUE);
		} else {
			indexes.resize(dim * dim * dim, -1);
		}
	}
	T& operator()(int i, int j, int k) {
		assert(data.size() > 0);
		return data[i + (j + k * dim) * dim];
	}
	const int& getIndex(int i, int j, int k) const {
		assert(i >= 0 && i < dim);
		assert(j >= 0 && j < dim);
		assert(k >= 0 && k < dim);
		return indexes[i + (j + k * dim) * dim];
	}
	int& getIndex(int i, int j, int k) {
		assert(i >= 0 && i < dim);
		assert(j >= 0 && j < dim);
		assert(k >= 0 && k < dim);
		return indexes[i + (j + k * dim) * dim];
	}

	bool hasChild(int i, int j, int k) const {
		int idx = getIndex(i, j, k);
		return (idx < 0 || idx >= children.size());
	}
	std::shared_ptr<EndlessNode<T>> addChild(int i, int j, int k, int d,
			bool isLeaf) {
		int& idx = indexes[i + (j + k * dim) * dim];
		idx = (int) children.size();
		std::shared_ptr<EndlessNode<T>> node = std::shared_ptr<EndlessNode<T>>(new EndlessNode<T>(d, isLeaf, this, idx));
		children.push_back(node);
		return node;
	}
	std::shared_ptr<EndlessNode<T>> getChild(int i, int j, int k, int d, bool isLeaf) {
		int& idx = indexes[i + (j + k * dim) * dim];
		if (idx < 0){
			idx = (int) children.size();
			std::shared_ptr<EndlessNode<T>> node = std::shared_ptr<EndlessNode<T>>(new EndlessNode<T>(d, isLeaf, this, idx));
			children.push_back(node);
			return node;
		}
		return children[idx];
	}
};
}

#endif /* INCLUDE_GRID_ENDLESSNODE_H_ */
