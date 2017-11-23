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
/*
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
 */
template<typename T> class EndlessGrid {
	std::vector<int> levels; //in local units
	std::vector<int> gridSizes; //in world grid units
	std::vector<int> cellSizes; //in world grid units
	std::vector<std::shared_ptr<EndlessNode<T>>> nodes;
	std::map<int3, int> indexes;
	T backgroundValue;
	//std::vector<NodeNeighbor<T>> neighbors;
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
		//neighbors.clear();
	}
	void setBackgroundValue(T val){
		backgroundValue=val;
	}
	T getBackgroundValue() const {
		return backgroundValue;
	}
	EndlessGrid(const std::initializer_list<int>& l, T bgValue) {
		backgroundValue=bgValue;
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
	std::vector<std::pair<int3, std::shared_ptr<EndlessNode<T>>>> getNodes() const {
		std::vector<std::pair<int3, std::shared_ptr<EndlessNode<T>>>> result;
		for (auto pr : indexes) {
			result.push_back( { pr.first, nodes[pr.second] });
		}
		return result;
	}
	const std::vector<int>& getCellSizes() const {
		return cellSizes;
	}
	const std::vector<int>& getGridSizes() const {
		return gridSizes;
	}
	const std::vector<int>& getLevelSizes() const {
		return levels;
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
		std::shared_ptr<EndlessNode<T>> node = getNode(ti, tj, tk);
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
		/*
		std::cout << "(" << ti << "," << tj << "," << tk << ") (" << i << ","
				<< j << "," << k << ") " << "(" << iii << "," << jjj << ","
				<< kkk << ") " << std::endl;*/
		std::shared_ptr<EndlessNode<T>> node = getNode(ti, tj, tk);
		for (int c = 0; c < levels.size() - 1; c++) {
			cdim = cellSizes[c];
			int3 pos = int3(iii / cdim, jjj / cdim, kkk / cdim);
			iii = iii % cdim;
			jjj = jjj % cdim;
			kkk = kkk % cdim;
			node = node->getChild(pos.x, pos.y, pos.z, levels[c + 1], backgroundValue, (c == levels.size() - 2));
		}
		return (*node)(iii, jjj, kkk);
	}
	std::shared_ptr<EndlessNode<T>> getNode(int ti, int tj, int tk) {
		auto idx = indexes.find(int3(ti, tj, tk));
		if (idx != indexes.end()) {
			return nodes[idx->second];
		} else {
			std::shared_ptr<EndlessNode<T>> node = std::shared_ptr<
					EndlessNode<T>>(
					new EndlessNode<T>(levels[0],backgroundValue,  levels.size() <= 1));
			indexes[int3(ti, tj, tk)] = (int) nodes.size();
			node->location = int3(ti, tj, tk);
			nodes.push_back(node);
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

typedef std::shared_ptr<EndlessNode<float>> EndlessNodeFloatPtr;
typedef std::shared_ptr<EndlessNode<float2>> EndlessNodeFloat2Ptr;
typedef std::shared_ptr<EndlessNode<float3>> EndlessNodeFloat3Ptr;
typedef std::shared_ptr<EndlessNode<float4>> EndlessNodeFloat4Ptr;
typedef std::shared_ptr<EndlessNode<int>> EndlessNodeIntegerPtr;
typedef std::shared_ptr<EndlessNode<std::pair<float, int>>> EndlessNodeFloatIndexPtr;
typedef std::shared_ptr<EndlessNode<RGB>> EndlessNodeRGBfPtr;
typedef std::shared_ptr<EndlessNode<RGBAf>> EndlessNodeRGBAfPtr;

void WriteGridToFile(const std::string& dir, EndlessGrid<float>& grid);

}

#endif /* INCLUDE_GRID_ENDLESSGRID_H_ */
