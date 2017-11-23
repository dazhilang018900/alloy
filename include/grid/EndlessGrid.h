/*
 * EndlessGrid.h
 *
 *  Created on: Nov 21, 2017
 *      Author: blake
 */

#ifndef INCLUDE_GRID_ENDLESSGRID_H_
#define INCLUDE_GRID_ENDLESSGRID_H_
#include "EndlessNode.h"
#include "AlloyMath.h"
#include "AlloyMesh.h"
#include <map>
#include <iostream>
namespace aly {
struct EndlessLocation: public std::vector<int3> {
	aly::int3 nodePosition;
	aly::int3 localPosition;
	aly::int3 worldPosition;
	EndlessLocation(size_t sz = 0) :
			std::vector<int3>(sz) {
	}
};
template<typename T> struct NodeNeighbor {
	EndlessNode<T>* left;
	EndlessNode<T>* right;
	EndlessNode<T>* up;
	EndlessNode<T>* down;
	EndlessNode<T>* top;
	EndlessNode<T>* bottom;
	EndlessNode<T>*& operator[](size_t idx) {
		return (&left)[idx];
	}
	const EndlessNode<T>*& operator[](size_t idx) const {
		return (&left)[idx];
	}
	NodeNeighbor() :
			left(nullptr), right(nullptr), up(nullptr), down(nullptr), top(
					nullptr), bottom(nullptr) {
	}
};
template<typename T> class EndlessGrid {
	std::vector<int> levels; //in local units
	std::vector<int> gridSizes; //in world grid units
	std::vector<int> cellSizes; //in world grid units
	std::vector<std::shared_ptr<EndlessNode<T>>> nodes;
	std::map<int3, int> indexes;
	std::vector<NodeNeighbor<T>> neighbors;
	int roundDown(int val, int size) const {
		int ret;
		if (val < 0) {
			ret = (val + 1) / size - 1;
		} else {
			ret = val / size;
		}
		return ret;
	}
public:
	void clear() {
		nodes.clear();
		indexes.clear();
		neighbors.clear();
	}
	EndlessGrid(const std::initializer_list<int>& l = { 16, 16, 4 }) {
		levels.assign(l.begin(), l.end());
		gridSizes.resize(levels.size(), 0);
		cellSizes.resize(levels.size(), 0);
		gridSizes[levels.size() - 1] = levels.back();
		cellSizes[levels.size() - 1] = 1;
		for (int c = levels.size() - 2; c >= 0; c--) {
			gridSizes[c] = gridSizes[c + 1] * levels[c];
			cellSizes[c] = gridSizes[c + 1];
		}
		for (int c = 0; c < levels.size(); c++) {
			std::cout << levels[c] << ") " << gridSizes[c] << " "
					<< cellSizes[c] << std::endl;
		}
	}
	int getNodeSize() const {
		return gridSizes[0];
	}
	int getNodeCount() const {
		return nodes.size();
	}
	int getTreeDepth() const {
		return levels.size();
	}
	std::shared_ptr<EndlessNode<T>> getLocation(int i, int j, int k,
			EndlessLocation& location) {
		location.resize(levels.size());
		int sz = gridSizes[0];
		int dim, cdim;
		int ti = roundDown(i, sz);
		int tj = roundDown(j, sz);
		int tk = roundDown(k, sz);
		int stride = std::max(std::max(std::abs(ti), std::abs(tj)),
				std::abs(tk)) + 1;
		int iii = ((i + stride * sz) % sz);
		int jjj = ((j + stride * sz) % sz);
		int kkk = ((k + stride * sz) % sz);
		location.nodePosition = int3(ti, tj, tk);
		location.localPosition = int3(iii, jjj, kkk);
		location.worldPosition = int3(i, j, k);
		std::shared_ptr<EndlessNode<T>> node = getNode(ti, tj, tk, true);
		for (int c = 0; c < (int) levels.size(); c++) {
			cdim = cellSizes[c];
			location[c] = int3(iii / cdim, jjj / cdim, kkk / cdim);
			iii = iii % cdim;
			jjj = jjj % cdim;
			kkk = kkk % cdim;
		}
		return node;
	}
	T& getValue(int i, int j, int k) {
		int sz = gridSizes[0];
		int cdim, dim;
		int ti = roundDown(i, sz);
		int tj = roundDown(j, sz);
		int tk = roundDown(k, sz);
		int stride = std::max(std::max(std::abs(ti), std::abs(tj)),
				std::abs(tk)) + 1;
		int iii = ((i + stride * sz) % sz);
		int jjj = ((j + stride * sz) % sz);
		int kkk = ((k + stride * sz) % sz);
		std::shared_ptr<EndlessNode<T>> node = getNode(ti, tj, tk, true);
		for (int c = 0; c < levels.size() - 1; c++) {
			cdim = cellSizes[c];
			int3 pos = int3(iii / cdim, jjj / cdim, kkk / cdim);
			iii = iii % cdim;
			jjj = jjj % cdim;
			kkk = kkk % cdim;
			node = node->getChild(pos.x, pos.y, pos.z, levels[c + 1],
					(c == levels.size() - 2));
		}
		return (*node)(iii, jjj, kkk);
	}
	std::shared_ptr<EndlessNode<T>> getNode(int i, int j, int k,
			bool AddIfNull = true) {
		const int nbrsX[6] = { 1, -1, 0, 0, 0, 0 };
		const int nbrsY[6] = { 0, 0, 1, -1, 0, 0 };
		const int nbrsZ[6] = { 0, 0, 0, 0, 1, -1 };
		const int comps[6] = { 1, 0, 3, 2, 5, 4 };
		int sz = gridSizes[0];
		int ti = roundDown(i, sz);
		int tj = roundDown(j, sz);
		int tk = roundDown(k, sz);
		auto idx = indexes.find(int3(ti, tj, tk));
		if (idx != indexes.end()) {
			return nodes[idx->second];
		} else {
			if (!AddIfNull) {
				return EndlessNode<T>::NULL_NODE;
			}
			std::shared_ptr<EndlessNode<T>> node = std::shared_ptr<EndlessNode<T>>(new EndlessNode<T>(levels[0], levels.size() <= 1));
			indexes[int3(ti,tj,tk)]=(int)nodes.size();
			nodes.push_back(node);
			/*
			 NodeNeighbor<T> nbrs;
			 for (int n = 0; n < 6; n++) {
			 int3 nbr(i + nbrsX[n], j + nbrsY[n], k + nbrsZ[n]);
			 auto nbri = indexes.find(nbr);
			 if (nbri != indexes.end()) {
			 nbrs[n] = nodes[nbri->second].get();
			 neighbors[nbri->second][comps[n]] = node.get();
			 }
			 }
			 neighbors.push_back(nbrs);
			 */
			return node;
		}
	}
};
void MeshToLevelSet(const Mesh& mesh, EndlessGrid<float>& grid);
typedef EndlessGrid<float> EndlessGridFloat;
typedef EndlessGrid<float2> EndlessGridFloat2;
typedef EndlessGrid<float3> EndlessGridFloat3;
typedef EndlessGrid<float4> EndlessGridFloat4;
typedef EndlessGrid<int> EndlessGridInteger;
typedef EndlessGrid<std::pair<float, int>> EndlessGridFloatIndex;
typedef EndlessGrid<RGB> EndlessGridRGBf;
typedef EndlessGrid<RGBAf> EndlessGridRGBAf;

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
}

#endif /* INCLUDE_GRID_ENDLESSGRID_H_ */
