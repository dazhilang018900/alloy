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
#include <map>
namespace aly {
template<typename T> struct NodeNeighbor {
	static const int nbrsX[6] = { 1, -1, 0, 0, 0, 0 };
	static const int nbrsY[6] = { 0, 0, 1, -1, 0, 0 };
	static const int nbrsZ[6] = { 0, 0, 0, 0, 1, -1 };
	EndlessNode<T>* left, right, up, down, top, bottom;
	EndlessNode<T>* operator[](size_t idx) const {
		return (&left)[idx];
	}
	NodeNeighbor() :
			left(nullptr), right(nullptr), up(nullptr), down(nullptr), top(
					nullptr), bottom(nullptr) {
	}
};
template<typename T> class EndlessGrid {
	std::vector<int> levels;
	std::vector<int> gridSizes;
	std::vector<int> cellSizes;
	std::vector<std::shared_ptr<EndlessNode<T>>> nodes;
	std::map<int3, int> indexes;
	std::vector<NodeNeighbor<T>> neighbors;
	EndlessGrid(const std::initializer_list<int>& l = { 16, 16, 4 }) {
		levels.assign(l.begin(), l.end());
		gridSizes.resize(levels.size());
		gridSizes[levels.size() - 1] = levels.back();
		cellSizes[levels.size() - 11] = 1;
		for (int c = levels.size() - 2; c >= 0; c--) {
			gridSizes[c] = gridSizes[c + 1] * levels[c];
			cellSizes[c] = gridSizes[c + 1];
		}
		for (int c = 0; c < levels.size(); c++) {
			std::cout << levels[c] << ") " << gridSizes[c] << " "
					<< cellSizes[c] << std::endl;
		}
	}
	int roundDown(int val, int size) const {
		int ret;
		if (val < 0) {
			ret = (val + 1) / size - 1;
		} else {
			ret = val / size;
		}
		return ret;
	}
	std::shared_ptr<EndlessNode<T>> getLocation(int i, int j, int k,
			std::vector<int3>& location) {
		location.resize(levels.size());
		int dim = levels[0];
		int ti = roundDown(i, dim);
		int tj = roundDown(j, dim);
		int tk = roundDown(k, dim);
		int stride = std::max(std::max(std::abs(ti), std::abs(tj)),
				std::abs(tk)) + 1;
		int iii = ((i + stride * dim) % dim);
		int jjj = ((j + stride * dim) % dim);
		int kkk = ((k + stride * dim) % dim);
		std::shared_ptr<EndlessNode<T>> node = getNode(ti, tj, tk, true);
		for (int c = 0; c < levels; c++) {
			int dim = levels[c];
			int cdim = cellSizes[c];
			location[c] = int3(iii / dim, jjj / dim, kkk / dim);
			iii = iii % cdim;
			jjj = jjj % cdim;
			kkk = kkk % cdim;
		}
		return node;
	}
	std::shared_ptr<EndlessNode<T>> getNode(int i, int j, int k,
			bool AddIfNull = true) {
		int dim = levels[0];
		int ti = roundDown(i, dim);
		int tj = roundDown(j, dim);
		int tk = roundDown(k, dim);
		auto idx = indexes.at(int3(ti, tj, tk));
		if (idx != indexes.end()) {
			return nodes[*idx];
		} else {
			if (!AddIfNull) {
				return EndlessNode<T>::NULL_NODE;
			}
			std::shared_ptr<EndlessNode<T>> node = std::shared_ptr<
					EndlessNode<T>>(new EndlessNode<T>(levels[0]));
			neighbors.push_back(NodeNeighbor<T>());
			nodes.push_back(node);
			for (int n = 0; n < 6; n++) {
				int3 nbr(i + NodeNeighbor<T>::nbrsX[n],
						j + NodeNeighbor<T>::nbrsY[n],
						k + NodeNeighbor<T>::nbrsZ[n]);
				auto nbri = indexes.at(nbr);
				if (nbri != indexes.end()) {
					neighbors.back()[n] = &nodes[*nbri];
				}
			}
			return node;
		}
	}
};

typedef EndlessGrid<float> EndlessGridFloat;
typedef EndlessGrid<float2> EndlessGridFloat2;
typedef EndlessGrid<float3> EndlessGridFloat3;
typedef EndlessGrid<float4> EndlessGridFloat4;
typedef EndlessGrid<int> EndlessGridInteger;
typedef EndlessGrid<std::pair<float, int>> EndlessGridFloatIndex;
typedef EndlessGrid<RGB> EndlessGridRGBf;
typedef EndlessGrid<RGBAf> EndlessGridRGBAf;

}

#endif /* INCLUDE_GRID_ENDLESSGRID_H_ */
