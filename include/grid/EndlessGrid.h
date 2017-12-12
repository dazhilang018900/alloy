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

#ifndef INCLUDE_GRID_ENDLESSGRID_H_
#define INCLUDE_GRID_ENDLESSGRID_H_
#include "EndlessNode.h"
#include "AlloyMath.h"
#include "AlloyMesh.h"
#include <unordered_map>
#include <map>
#include <iostream>
namespace aly {
template<typename T> class EndlessGrid {
	std::vector<int> levels; //in local units
	std::vector<int> gridSizes; //in world grid units
	std::vector<int> cellSizes; //in world grid units
	std::vector<std::shared_ptr<EndlessNode<T>>> nodes;
	std::unordered_map<int3,int> indexes;
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
	void setBackgroundValue(T val) {
		backgroundValue = val;
	}
	T getBackgroundValue() const {
		return backgroundValue;
	}
	EndlessGrid(const std::initializer_list<int>& l, T bgValue) {
		backgroundValue = bgValue;
		levels.assign(l.begin(), l.end());
		gridSizes.resize(levels.size(), 0);
		cellSizes.resize(levels.size(), 0);
		gridSizes[levels.size() - 1] = levels.back();
		cellSizes[levels.size() - 1] = 1;
		for (int c = levels.size() - 2; c >= 0; c--) {
			gridSizes[c] = gridSizes[c + 1] * levels[c];
			cellSizes[c] = gridSizes[c + 1];
		}
	}
	EndlessGrid(const std::vector<int>& l, T bgValue) {
		backgroundValue = bgValue;
		levels = l;
		gridSizes.resize(levels.size(), 0);
		cellSizes.resize(levels.size(), 0);
		gridSizes[levels.size() - 1] = levels.back();
		cellSizes[levels.size() - 1] = 1;
		for (int c = levels.size() - 2; c >= 0; c--) {
			gridSizes[c] = gridSizes[c + 1] * levels[c];
			cellSizes[c] = gridSizes[c + 1];
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
	int getCellSize(size_t i) const {
		return cellSizes[i];
	}
	int getGridSize(size_t i) const {
		return gridSizes[i];
	}
	int getLevelSize(size_t i) const {
		return levels[i];
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
	inline void allocateInternalNodes() {
		for (auto node : nodes) {
			if (!node->isLeaf())
				node->allocate(backgroundValue);
		}
	}
	inline void getLeafNodes(std::vector<int3>& positions,
			std::vector<std::shared_ptr<EndlessNode<T>>>& result) {
		positions.clear();
		result.clear();
		for (auto node : nodes) {
			if(node->isLeaf()){
				result.push_back(node);
			} else {
				node->getLeafNodes(positions, result, cellSizes);
			}
		}
	}
	inline std::vector<std::shared_ptr<EndlessNode<T>>> getLeafNodes() const {
		std::vector<std::shared_ptr<EndlessNode<T>>> result;
		for (auto node : nodes) {
			if(node->isLeaf()){
				result.push_back(node);
			} else {
				node->getLeafNodes(result);
			}
		}
		return result;
	}
	inline std::vector<std::shared_ptr<EndlessNode<T>>> getNodesAtDepth(int d) const {
		std::vector<std::shared_ptr<EndlessNode<T>>> result;
		for (auto node : nodes) {
			if (d == 0) {
				result.push_back(node);
			} else {
				node->getNodesAtDepth(result, d);
			}
		}
		return result;
	}
	T& getLeafValue(int i, int j, int k) {
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
			node = node->getChild(pos.x, pos.y, pos.z, cdim, levels[c + 1],
					backgroundValue, (c == levels.size() - 2));
		}
		return (*node)(iii, jjj, kkk);
	}
	T& getMultiResolutionValue(int i, int j, int k,
			std::shared_ptr<EndlessNode<T>>& result) {
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
		std::shared_ptr<EndlessNode<T>> node = getNode(ti, tj, tk);
		for (int c = 0; c < levels.size() - 1; c++) {
			cdim = cellSizes[c];
			int3 pos = int3(iii / cdim, jjj / cdim, kkk / cdim);
			iii = iii % cdim;
			jjj = jjj % cdim;
			kkk = kkk % cdim;
			auto child = node->getChild(pos.x, pos.y, pos.z);
			if (child.get() != nullptr) {
				node = child;
			} else {
				if (!node->hasData())
					node->allocate(backgroundValue);
				result = node;
				return (*node)(pos.x, pos.y, pos.z);
			}
		}
		result = node;
		return (*node)(iii, jjj, kkk);
	}
	void getMultiResolutionValue(int i, int j, int k,std::shared_ptr<EndlessNode<T>>& result, T*& value) const {
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
		value = nullptr;
		std::shared_ptr<EndlessNode<T>> node = getNodeIfExists(ti, tj, tk);
		if (node.get() == nullptr)
			return;
		for (int c = 0; c < levels.size() - 1; c++) {
			cdim = cellSizes[c];
			int3 pos = int3(iii / cdim, jjj / cdim, kkk / cdim);
			iii = iii % cdim;
			jjj = jjj % cdim;
			kkk = kkk % cdim;
			auto child = node->getChild(pos.x, pos.y, pos.z);
			if (child.get() != nullptr) {
				node = child;
			} else {
				if (!node->hasData()) {
					return;
				}
				result = node;
				value = &(*node)(pos.x, pos.y, pos.z);
				return;
			}
		}
		result = node;
		value = &(*node)(iii, jjj, kkk);
	}

	bool getLeafValue(int i, int j, int k,std::shared_ptr<EndlessNode<T>>& result, T& value) const {
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
		std::shared_ptr<EndlessNode<T>> node = getNodeIfExists(ti, tj, tk);
		if (node.get() == nullptr)
			return false;
		for (int c = 0; c < levels.size() - 1; c++) {
			cdim = cellSizes[c];
			int3 pos = int3(iii / cdim, jjj / cdim, kkk / cdim);
			iii = iii % cdim;
			jjj = jjj % cdim;
			kkk = kkk % cdim;
			auto child = node->getChild(pos.x, pos.y, pos.z);
			if (child.get() != nullptr) {
				node = child;
			} else {
				return false;
			}
		}
		result = node;
		value = (*node)(iii, jjj, kkk);
		return true;
	}
	T getLeafValue(int i, int j, int k) const {
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
		std::shared_ptr<EndlessNode<T>> node = getNodeIfExists(ti, tj, tk);
		if (node.get() == nullptr)
			return backgroundValue;
		for (int c = 0; c < levels.size() - 1; c++) {
			cdim = cellSizes[c];
			int3 pos = int3(iii / cdim, jjj / cdim, kkk / cdim);
			iii = iii % cdim;
			jjj = jjj % cdim;
			kkk = kkk % cdim;
			auto child = node->getChild(pos.x, pos.y, pos.z);
			if (child.get() != nullptr) {
				node = child;
			} else {
				return backgroundValue;
			}
		}
		return (*node)(iii, jjj, kkk);
	}
	T getMultiResolutionValue(int i, int j, int k) const {
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
		std::shared_ptr<EndlessNode<T>> node = getNodeIfExists(ti, tj, tk);
		if (node.get() == nullptr)
			return backgroundValue;
		for (int c = 0; c < levels.size() - 1; c++) {
			cdim = cellSizes[c];
			int3 pos = int3(iii / cdim, jjj / cdim, kkk / cdim);
			iii = iii % cdim;
			jjj = jjj % cdim;
			kkk = kkk % cdim;
			auto child = node->getChild(pos.x, pos.y, pos.z);
			if (child.get() != nullptr) {
				node = child;
			} else {
				if (node->hasData()) {
					return (*node)(pos.x, pos.y, pos.z);
				}  else {
					return backgroundValue;
				}
			}
		}
		return (*node)(iii, jjj, kkk);
	}
	T getLeafValue(int i, int j, int k,std::shared_ptr<EndlessNode<T>> node) const {
		int3 delta = int3(i, j, k)-node->location;
		int dim = node->dim;
		int xoff = roundDown(delta.x, dim);
		int yoff = roundDown(delta.y, dim);
		int zoff = roundDown(delta.z, dim);
		if (xoff == 0 && yoff == 0 && zoff == 0) {
			return (*node)(delta.x, delta.y, delta.z);
		} else {
			//Cheack parent of leaf
			EndlessNode<T>* parent = node->parent;
			if(parent!=nullptr){
				int3 pos =node->getId();
				//if(parent->getChild(pos.x, pos.y, pos.z).get()!=node.get()){
				//	std::cout<<"Could not find location "<<pos<<" "<<node->id<<std::endl;
				//}
				delta = pos + int3(xoff, yoff, zoff);
				node = parent->getChild(pos.x+xoff, pos.y+yoff, pos.z+zoff);
				if(node.get()!=nullptr){
					delta = int3(i, j, k)-node->location;
					return (*node)(delta.x, delta.y, delta.z);
				}
			}
		}
		//Deep search
		int sz = gridSizes[0];
		int cdim;
		int ti = roundDown(i, sz);
		int tj = roundDown(j, sz);
		int tk = roundDown(k, sz);
		int stride = std::max(std::max(std::abs(ti), std::abs(tj)),
				std::abs(tk)) + 1;
		int iii = ((i + stride * sz) % sz);
		int jjj = ((j + stride * sz) % sz);
		int kkk = ((k + stride * sz) % sz);
		node = getNodeIfExists(ti, tj, tk);
		if (node.get() == nullptr)
			return backgroundValue;
		for (int c = 0; c < levels.size() - 1; c++) {
			cdim = cellSizes[c];
			int3 pos = int3(iii / cdim, jjj / cdim, kkk / cdim);
			iii = iii % cdim;
			jjj = jjj % cdim;
			kkk = kkk % cdim;
			auto child = node->getChild(pos.x, pos.y, pos.z);
			if (child.get() != nullptr) {
				node = child;
			} else {
				return backgroundValue;
			}
		}
		return (*node)(iii, jjj, kkk);

	}
	T getMultiResolutionValue(std::shared_ptr<EndlessNode<T>> node, int iii,
			int jjj, int kkk) const {
		int cdim;
		for (int c = 0; c < levels.size() - 1; c++) {
			cdim = cellSizes[c];
			int3 pos = int3(iii / cdim, jjj / cdim, kkk / cdim);
			iii = iii % cdim;
			jjj = jjj % cdim;
			kkk = kkk % cdim;
			auto child = node->getChild(pos.x, pos.y, pos.z);
			if (child.get() != nullptr) {
				node = child;
			} else {
				if (node->hasData()) {
					return (*node)(pos.x, pos.y, pos.z);
				} else {
					return backgroundValue;
				}
			}
		}
		return (*node)(iii, jjj, kkk);
	}
	int getTreeDepth(std::shared_ptr<EndlessNode<T>> node, int iii, int jjj,
			int kkk) const {
		int cdim;
		int d = 0;
		for (int c = 0; c < levels.size() - 1; c++) {
			cdim = cellSizes[c];
			int3 pos = int3(iii / cdim, jjj / cdim, kkk / cdim);
			iii = iii % cdim;
			jjj = jjj % cdim;
			kkk = kkk % cdim;
			auto child = node->getChild(pos.x, pos.y, pos.z);
			if (child.get() != nullptr) {
				node = child;
				d++;
			} else {
				return d;
			}
		}
		return d;
	}
	float getDepthValue(std::shared_ptr<EndlessNode<T>> node, int iii, int jjj,
			int kkk, int D) const {
		int cdim;
		int d = 0;
		for (int c = 0; c < levels.size() - 1; c++) {
			cdim = cellSizes[c];
			int3 pos = int3(iii / cdim, jjj / cdim, kkk / cdim);
			iii = iii % cdim;
			jjj = jjj % cdim;
			kkk = kkk % cdim;
			if (d == D) {
				return (*node)(pos.x, pos.y, pos.z);
			}
			auto child = node->getChild(pos.x, pos.y, pos.z);
			if (child.get() != nullptr) {
				node = child;
				d++;
			} else {
				return backgroundValue;
			}
		}
		if (d == D) {
			return (*node)(iii, jjj, kkk);
		} else {
			return backgroundValue;
		}
	}
	std::shared_ptr<EndlessNode<T>> getNodeIfExists(int ti, int tj,
			int tk) const {
		auto idx = indexes.find(int3(ti, tj, tk));
		if (idx != indexes.end()) {
			return nodes[idx->second];
		} else {
			return std::shared_ptr<EndlessNode<T>>();
		}
	}
	std::shared_ptr<EndlessNode<T>> getNode(int ti, int tj, int tk) {
		auto idx = indexes.find(int3(ti, tj, tk));
		if (idx != indexes.end()) {
			return nodes[idx->second];
		} else {
			std::shared_ptr<EndlessNode<T>> node = std::shared_ptr<
					EndlessNode<T>>(
					new EndlessNode<T>(levels[0], backgroundValue,
							levels.size() <= 1));
			indexes[int3(ti, tj, tk)] = (int) nodes.size();
			node->location = int3(ti * gridSizes[0], tj * gridSizes[0],
					tk * gridSizes[0]);
			nodes.push_back(node);
			return node;
		}
	}
};
void FloodFill(EndlessGrid<float>& grid, float narrowBand);
float3 GetNormal(const EndlessGrid<float>& grid, int i, int j, int k);
float GetInterpolatedValue(const EndlessGrid<float>& grid, float x, float y,
		float z);
float3 GetInterpolatedNormal(const EndlessGrid<float>& grid, float x, float y,
		float z);
float4x4 MeshToLevelSet(const Mesh& mesh, EndlessGrid<float>& grid,
		float narrowBand, bool flipSign = true, float voxelScale = 0.75f,const std::function<bool(const std::string& message, float progress)>& monitor=nullptr);
float4x4 PointsToLevelSet(const Mesh& mesh, EndlessGrid<float>& grid, float voxelSize, float surfelSize,const std::function<bool(const std::string& message, float progress)>& monitor=nullptr);
typedef EndlessGrid<float> EndlessGridFloat;
typedef EndlessGrid<int> EndlessGridInt;
typedef EndlessGrid<float2> EndlessGridFloat2;
typedef EndlessGrid<float3> EndlessGridFloat3;
typedef EndlessGrid<float4> EndlessGridFloat4;
typedef EndlessGrid<std::pair<float, int>> EndlessGridFloatIndex;
typedef EndlessGrid<RGBf> EndlessGridRGBf;
typedef EndlessGrid<RGBAf> EndlessGridRGBAf;

typedef EndlessNode<float> EndlessNodeFloat;
typedef EndlessNode<int> EndlessNodeInt;
typedef EndlessNode<float2> EndlessNodeFloat2;
typedef EndlessNode<float3> EndlessNodeFloat3;
typedef EndlessNode<float4> EndlessNodeFloat4;
typedef EndlessNode<std::pair<float, int>> EndlessNodeFloatIndex;
typedef EndlessNode<RGBf> EndlessNodeRGBf;
typedef EndlessNode<RGBAf> EndlessNodeRGBAf;

typedef std::shared_ptr<EndlessNode<float>> EndlessNodeFloatPtr;
typedef std::shared_ptr<EndlessNode<int>> EndlessNodeIntPtr;
typedef std::shared_ptr<EndlessNode<float2>> EndlessNodeFloat2Ptr;
typedef std::shared_ptr<EndlessNode<float3>> EndlessNodeFloat3Ptr;
typedef std::shared_ptr<EndlessNode<float4>> EndlessNodeFloat4Ptr;
typedef std::shared_ptr<EndlessNode<std::pair<float, int>>> EndlessNodeFloatIndexPtr;
typedef std::shared_ptr<EndlessNode<RGBf>> EndlessNodeRGBfPtr;
typedef std::shared_ptr<EndlessNode<RGBAf>> EndlessNodeRGBAfPtr;

void WriteGridToFile(const std::string& dir, const EndlessGrid<float>& grid);
void WriteGridToFile(const std::string& dir, const EndlessGrid<float2>& grid);
void WriteGridToFile(const std::string& dir, const EndlessGrid<int>& grid);

}

#endif /* INCLUDE_GRID_ENDLESSGRID_H_ */
