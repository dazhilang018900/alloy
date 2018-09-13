/*
 * Copyright(C) 2017, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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

#ifndef INCLUDE_GRID_ENDLESSNODE_H_
#define INCLUDE_GRID_ENDLESSNODE_H_

#include <list>
#include "math/AlloyVecMath.h"
namespace aly {
struct EndlessLocation: public std::vector<int3> {
	aly::int3 nodePosition;
	aly::int3 localPosition;
	aly::int3 worldPosition;
	EndlessLocation(size_t sz = 0) :
			std::vector<int3>(sz) {
	}
};
template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const EndlessLocation& v) {
	ss << "[";
	for (int i = 0; i < v.size(); i++) {
		ss << v[i];
		if (i < v.size() - 1) {
			ss << ", ";
		}
	}
	ss << "] node:" << v.nodePosition << " local:" << v.localPosition
			<< " world:" << v.worldPosition;
	return ss;
}
template<typename T> class EndlessNode {
public:
	int dim;
	uint id;
	EndlessNode<T>* parent;
	int3 location;
	std::vector<int> indexes;
	std::vector<T> data;
	std::vector<std::unique_ptr<EndlessNode<T>>> children;
	bool isLeaf() const {
		return (indexes.size() == 0);
	}
	bool hasData() const {
		return (data.size()>0);
	}
	void allocate(T backgroundValue) {
		data.resize(dim*dim*dim,backgroundValue);
		for(const std::unique_ptr<EndlessNode<T>>& child:children){
			if(!child->isLeaf())child->allocate(backgroundValue);
		}
	}
	void getNodesAtDepth(std::list<EndlessNode<T>*>& result,int target,int parent=0) const {
		parent++;
		if(parent==target){
			result.insert(result.end(),children.begin(),children.end());
		} else {
			for (EndlessNode<T>* child : children) {
				child->getNodesAtDepth(result,target,parent);
			}
		}
	}
	void getLeafNodes(std::list<EndlessLocation>& positions,
			std::vector<EndlessNode<T>*>& result,
			EndlessLocation offset) const {
		offset.push_back(location);
		for (EndlessNode<T>* child : children) {
			if (child->isLeaf()) {
				EndlessLocation pos = offset;
				pos.push_back(child->location);
				positions.push_back(pos);
				result.push_back(child);
			} else {
				child->getLeafNodes(positions, result, offset);
			}
		}
	}
	void getLeafNodes(std::list<EndlessNode<T>*>& result) const {
		for (const std::unique_ptr<EndlessNode<T>>& child : children) {
			if (child->isLeaf()) {
				result.push_back(child.get());
			} else {
				child->getLeafNodes(result);
			}
		}
	}
	virtual ~EndlessNode(){
	}
	EndlessNode(int dim,T bgValue, bool isLeaf) :
			location(0, 0, 0), parent(nullptr), dim(dim),id(-1) {
		if (isLeaf) {
			data.resize(dim * dim * dim, bgValue);
		} else {
			indexes.resize(dim * dim * dim, -1);
		}
	}
	EndlessNode(int D,T bgValue, bool isLeaf, EndlessNode<T>* parent, int3 location) :
			location(location), parent(parent), dim(D),id(-1) {
		if (isLeaf) {
			data.resize(dim * dim * dim, bgValue);
		} else {
			indexes.resize(dim * dim * dim, -1);
		}
	}
	inline int3 getId() const {
		int x=id&0x03ff;
		int y=(id>>10)&0x03ff;
		int z=(id>>20)&0x03ff;
		return int3(x,y,z);
	}
	inline void setId(int i,int j,int k) {
		id=(0x03ff&i)|((0x03ff&j)<<10)|((0x03ff&k)<<20);
	}
	inline int getIndex(EndlessNode<T>* node){
		for(int i=0;i<children.size();i++){
			if(children[i].get()==node){
				for(int n=0;n<indexes.size();n++){
					if(indexes[n]==i)return n;
				}
			}
		}
		return -1;
	}
	float getSideValue(int s) const {
		int3 nbr;
		const int nbr6X[6] = { 1, -1, 0, 0, 0, 0 };
		const int nbr6Y[6] = { 0, 0, 1, -1, 0, 0 };
		const int nbr6Z[6] = { 0, 0, 0, 0, 1, -1 };
		nbr.x = clamp(dim / 2 + dim * nbr6X[s], 0, dim - 1);
		nbr.y = clamp(dim / 2 + dim * nbr6Y[s], 0, dim - 1);
		nbr.z = clamp(dim / 2 + dim * nbr6Z[s], 0, dim - 1);
		/*
		const int nbr26X[26] = { 1, 0, -1, 1, 0, -1, 1, 0, -1, 1, 0, -1, 1, -1, 1,
				0, -1, 1, 0, -1, 1, 0, -1, 1, 0, -1 };
		const int nbr26Y[26] = { 1, 1, 1, 0, 0, 0, -1, -1, -1, 1, 1, 1, 0, 0, -1,
				-1, -1, 1, 1, 1, 0, 0, 0, -1, -1, -1 };
		const int nbr26Z[26] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
				-1, -1, -1, -1, -1, -1, -1, -1, -1 };

		nbr.x = clamp(dim / 2 + dim * nbr26X[s], 0, dim - 1);
		nbr.y = clamp(dim / 2 + dim * nbr26Y[s], 0, dim - 1);
		nbr.z = clamp(dim / 2 + dim * nbr26Z[s], 0, dim - 1);
		*/return data[nbr.x+(nbr.y+nbr.z*dim)*dim];
	}
	inline float getChildSideValue(int3 pos,int s) const {
		int idx=indexes[pos.x+(pos.y+pos.z*dim)*dim];
		if(idx>=0){
			return children[idx]->getSideValue(s);
		} else {
			return data[pos.x+(pos.y+pos.z*dim)*dim];
		}
	}
	inline T& operator()(int i, int j, int k) {
		//assert(data.size() > 0);
		//assert(i >= 0 && i < dim);
		//assert(j >= 0 && j < dim);
		//assert(k >= 0 && k < dim);
		return data[i + (j + k * dim) * dim];
	}
	inline const T& operator()(int i, int j, int k) const {
		//assert(data.size() > 0);
		//assert(i >= 0 && i < dim);
		//assert(j >= 0 && j < dim);
		//assert(k >= 0 && k < dim);
		return data[i + (j + k * dim) * dim];
	}
	inline const int& getIndex(int i, int j, int k) const {
		//assert(i >= 0 && i < dim);
		//assert(j >= 0 && j < dim);
		//assert(k >= 0 && k < dim);
		return indexes[i + (j + k * dim) * dim];
	}
	inline int& getIndex(int i, int j, int k) {
		//assert(i >= 0 && i < dim);
		//assert(j >= 0 && j < dim);
		//assert(k >= 0 && k < dim);
		return indexes[i + (j + k * dim) * dim];
	}
	inline bool hasChild(int i, int j, int k) const {
		int idx = getIndex(i, j, k);
		return (idx < 0 || idx >= children.size());
	}
	EndlessNode<T>* addChild(int i, int j, int k, int d,T bgValue,
			bool isLeaf) {
		int& idx = indexes[i + (j + k * dim) * dim];
		idx = (int) children.size();
		EndlessNode<T>* node = new EndlessNode<T>(d,bgValue, isLeaf, this,location+int3(d*i, d*j, d*k));
		node->setId(i,j,k);
		children.push_back(std::unique_ptr<EndlessNode<T>>(node));
		return node;
	}
	EndlessNode<T>* getChild(int i, int j, int k, int c, int d,T bgValue,bool isLeaf) {
		//assert(i>=0&&i<dim);
		//assert(j>=0&&j<dim);
		//assert(k>=0&&k<dim);
		int& idx = indexes[i + (j + k * dim) * dim];
		if (idx < 0) {
			idx = (int) children.size();
			EndlessNode<T>* node = new EndlessNode<T>(d,bgValue, isLeaf, this, location+int3(c*i, c*j, c*k));
			node->setId(i,j,k);
			children.push_back(std::unique_ptr<EndlessNode<T>>(node));
			return node;
		}
		return children[idx].get();
	}
	inline EndlessNode<T>* getChild(int i, int j, int k) const {
		if(		i<0||i>=dim||
				j<0||j>=dim||
				k<0||k>=dim){
			return nullptr;
		}
		int idx = indexes[i + (j + k * dim) * dim];
		if (idx < 0||idx>=children.size()) {
			return nullptr;
		}
		return children[idx].get();
	}

};
}

#endif /* INCLUDE_GRID_ENDLESSNODE_H_ */
