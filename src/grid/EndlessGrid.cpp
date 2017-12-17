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

#include "grid/EndlessGrid.h"
#include "AlloyVolume.h"

#include "AlloyIsoSurface.h"
#include "AlloyDistanceField.h"
#include <queue>
namespace aly {
void WriteGridToFile(const std::string& root, const EndlessGrid<float>& grid) {
	std::vector<std::pair<int3, EndlessNodeFloat*>> nodes = grid.getNodes();
	int dim = grid.getNodeSize();
	Volume1f data(dim, dim, dim);
	//Volume1ub depth(dim, dim, dim);
	std::string name = GetFileNameWithoutExtension(root);
	std::string dir = GetFileDirectoryPath(root);
	for (auto pr : nodes) {
		int3 pos = pr.first;
		//if (pos != int3(0, 0, 0))continue;
		EndlessNodeFloat* node = pr.second;
		std::string volFile = MakeString() << dir << ALY_PATH_SEPARATOR<<name<<"_"<<pos.x<<"_"<<pos.y<<"_"<<pos.z<<".xml";
		data.set(grid.getBackgroundValue());
		std::vector<int3> narrowBandList;
		for (int z = 0; z < dim; z++) {
			for (int y = 0; y < dim; y++) {
				for (int x = 0; x < dim; x++) {
					float val = grid.getMultiResolutionValue(node, x, y, z);
					data(x, y, z).x = val;
				}
			}
		}
		WriteVolumeToFile(volFile, data);
		/*

		 file = MakeString() << dir << ALY_PATH_SEPARATOR<<"tree_"<<pos.x<<"_"<<pos.y<<"_"<<pos.z<<".xml";
		 depth.set(grid.getBackgroundValue());
		 for (int z = 0; z < dim; z++) {
		 for (int y = 0; y < dim; y++) {
		 for (int x = 0; x < dim; x++) {
		 depth(x, y, z).x = grid.getTreeDepth(node, x, y, z);
		 }
		 }
		 }
		 std::cout << "Write " << file << std::endl;
		 WriteVolumeToFile(file, depth);
		 for (int d = 0; d < grid.getTreeDepth(); d++) {
		 file = MakeString() << dir << ALY_PATH_SEPARATOR<<"tree_"<<pos.x<<"_"<<pos.y<<"_"<<pos.z<<"-"<<d<<".xml";
		 data.set(grid.getBackgroundValue());
		 for (int z = 0; z < dim; z++) {
		 for (int y = 0; y < dim; y++) {
		 for (int x = 0; x < dim; x++) {
		 data(x, y, z).x = grid.getDepthValue(node, x, y, z, d);
		 }
		 }
		 }
		 std::cout<<"Write "<<file<<std::endl;
		 WriteVolumeToFile(file, data);
		 }
		 */
	}
}
void WriteGridToFile(const std::string& root, const EndlessGrid<float2>& grid) {
	std::vector<std::pair<int3, EndlessNodeFloat2*>> nodes = grid.getNodes();
	int dim = grid.getNodeSize();
	Volume2f data(dim, dim, dim);
	std::string name = GetFileNameWithoutExtension(root);
	std::string dir = GetFileDirectoryPath(root);
	for (auto pr : nodes) {
		int3 pos = pr.first;
		EndlessNodeFloat2* node = pr.second;
		std::string volFile = MakeString() << dir << ALY_PATH_SEPARATOR<<name<<"_"<<pos.x<<"_"<<pos.y<<"_"<<pos.z<<".xml";
		data.set(grid.getBackgroundValue());
		std::vector<int3> narrowBandList;
		for (int z = 0; z < dim; z++) {
			for (int y = 0; y < dim; y++) {
				for (int x = 0; x < dim; x++) {
					float2 val = grid.getMultiResolutionValue(node, x, y, z);
					data(x, y, z) = val;
				}
			}
		}
		WriteVolumeToFile(volFile, data);
	}
}
void WriteGridToFile(const std::string& root, const EndlessGrid<int>& grid) {
	std::vector<std::pair<int3, EndlessNodeInt*>> nodes = grid.getNodes();
	int dim = grid.getNodeSize();
	Volume1i data(dim, dim, dim);
	std::string name = GetFileNameWithoutExtension(root);
	std::string dir = GetFileDirectoryPath(root);
	for (auto pr : nodes) {
		int3 pos = pr.first;
		EndlessNodeInt* node = pr.second;
		std::string file = MakeString() << dir << ALY_PATH_SEPARATOR<<name<<"_"<<pos.x<<"_"<<pos.y<<"_"<<pos.z<<".xml";
		data.set(grid.getBackgroundValue());
		for (int z = 0; z < dim; z++) {
			for (int y = 0; y < dim; y++) {
				for (int x = 0; x < dim; x++) {
					data(x, y, z).x = grid.getMultiResolutionValue(node, x, y,
							z);
				}
			}
		}
		WriteVolumeToFile(file, data);

	}
}
void FloodFill(EndlessGrid<float>& grid, float narrowBand) {
	const int nbr6X[6] = { 1, -1, 0, 0, 0, 0 };
	const int nbr6Y[6] = { 0, 0, 1, -1, 0, 0 };
	const int nbr6Z[6] = { 0, 0, 0, 0, 1, -1 };
	const int nbr26X[26] = { 1, 0, -1, 1, 0, -1, 1, 0, -1, 1, 0, -1, 1, -1, 1,
			0, -1, 1, 0, -1, 1, 0, -1, 1, 0, -1 };
	const int nbr26Y[26] = { 1, 1, 1, 0, 0, 0, -1, -1, -1, 1, 1, 1, 0, 0, -1,
			-1, -1, 1, 1, 1, 0, 0, 0, -1, -1, -1 };
	const int nbr26Z[26] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
			-1, -1, -1, -1, -1, -1, -1, -1, -1 };
	float backgroundValue = grid.getBackgroundValue();
	std::set<EndlessNodeFloat*> parents;
	std::list<EndlessNodeFloat*> leafs = grid.getLeafNodes();
	for (EndlessNodeFloat* leaf : leafs) {
		if (leaf->parent != nullptr)
			leaf->parent->data[leaf->parent->getIndex(leaf)] = 0;
		int3 location = leaf->location;
		std::vector<float>& data = leaf->data;
		int dim = leaf->dim;
		std::queue<int3> posQ, negQ;
		std::set<int3> posO, negO;
		for (int k = 0; k < dim; k++) {
			for (int j = 0; j < dim; j++) {
				for (int i = 0; i < dim; i++) {
					float& val = data[i + (j + k * dim) * dim];
					if (val != backgroundValue) {
						for (int n = 0; n < 26; n++) {
							int3 nbr = int3(i + nbr26X[n], j + nbr26Y[n],
									k + nbr26Z[n]);
							if (nbr.x >= 0 && nbr.x < dim && nbr.y >= 0
									&& nbr.y < dim && nbr.z >= 0
									&& nbr.z < dim) {
								float& nval = data[nbr.x
										+ (nbr.y + nbr.z * dim) * dim];
								if (nval == backgroundValue) {
									nval = sign(val) * narrowBand;
									if (nval < 0.0f) {
										negQ.push(nbr);
									} else if (nval > 0.0f) {
										posQ.push(nbr);
									}
								}
							} else {
								nbr = location + nbr;
								if (val > 0) {
									posO.insert(nbr);
								} else if (val < 0) {
									negO.insert(nbr);
								}
							}
						}
					}
				}
			}
		}

		while (!negQ.empty()) {
			int3 pos = negQ.front();
			negQ.pop();
			for (int n = 0; n < 6; n++) {
				int3 nbr = pos + int3(nbr6X[n], nbr6Y[n], nbr6Z[n]);
				if (nbr.x >= 0 && nbr.x < dim && nbr.y >= 0 && nbr.y < dim
						&& nbr.z >= 0 && nbr.z < dim) {
					float& val = data[nbr.x + (nbr.y + nbr.z * dim) * dim];
					if (val == backgroundValue) {
						val = -narrowBand;
						negQ.push(nbr);
					}
				}
			}
		}
		while (!posQ.empty()) {
			int3 pos = posQ.front();
			posQ.pop();
			float maxVal = narrowBand * 2;
			for (int n = 0; n < 6; n++) {
				int3 nbr = pos + int3(nbr6X[n], nbr6Y[n], nbr6Z[n]);
				if (nbr.x >= 0 && nbr.x < dim && nbr.y >= 0 && nbr.y < dim
						&& nbr.z >= 0 && nbr.z < dim) {
					float& val = data[nbr.x + (nbr.y + nbr.z * dim) * dim];
					if (val == backgroundValue) {
						val = narrowBand;
						posQ.push(nbr);
					}
				}
			}

		}

		for (int3 nbr : posO) {
			EndlessNodeFloat* result;
			float* nval = nullptr;
			grid.getMultiResolutionValue(nbr.x, nbr.y, nbr.z, result, nval);
			if (nval != nullptr && *nval == backgroundValue) {
				*nval = narrowBand;
			}
		}
		for (int3 nbr : negO) {
			EndlessNodeFloat* result;
			float* nval = nullptr;
			grid.getMultiResolutionValue(nbr.x, nbr.y, nbr.z, result, nval);
			if (nval != nullptr && *nval == backgroundValue) {
				*nval = -narrowBand;
			}
		}

		if (leaf->parent != nullptr)
			parents.insert(leaf->parent);
	}

	int treeDepth = grid.getTreeDepth() - 2;
	while (parents.size() > 0) {
		std::set<EndlessNodeFloat*> newParents;
		for (EndlessNodeFloat* node : parents) {
			if (treeDepth < 0) {
				parents.clear();
				break;
			}
			if (node->parent != nullptr)
				node->parent->data[node->parent->getIndex(node)] = 0;
			int3 location = node->location;
			std::vector<float>& data = node->data;
			int dim = node->dim;
			int c = grid.getCellSizes()[treeDepth];
			std::queue<int3> posQ, negQ;
			std::set<int3> posO, negO;
			for (int k = 0; k < dim; k++) {
				for (int j = 0; j < dim; j++) {
					for (int i = 0; i < dim; i++) {
						float& val = data[i + (j + k * dim) * dim];
						if (val != backgroundValue) {
							for (int n = 0; n < 26; n++) {
								int3 nbr = int3(i + nbr26X[n], j + nbr26Y[n],
										k + nbr26Z[n]);
								if (nbr.x >= 0 && nbr.x < dim && nbr.y >= 0
										&& nbr.y < dim && nbr.z >= 0
										&& nbr.z < dim) {
									float& nval = data[nbr.x
											+ (nbr.y + nbr.z * dim) * dim];
									if (nval == backgroundValue) {
										nval = sign(val) * narrowBand;
										if (nval < 0.0f) {
											negQ.push(nbr);
										} else if (nval > 0.0f) {
											posQ.push(nbr);
										}
									}
								} else {
									nbr = location + c * nbr;
									if (val > 0) {
										posO.insert(nbr);
									} else if (val < 0) {
										negO.insert(nbr);
									}
								}
							}
						}
					}
				}
			}
			while (!negQ.empty()) {
				int3 pos = negQ.front();
				negQ.pop();
				for (int n = 0; n < 6; n++) {
					int3 nbr = pos + int3(nbr6X[n], nbr6Y[n], nbr6Z[n]);
					if (nbr.x >= 0 && nbr.x < dim && nbr.y >= 0 && nbr.y < dim
							&& nbr.z >= 0 && nbr.z < dim) {
						float& val = data[nbr.x + (nbr.y + nbr.z * dim) * dim];
						if (val == backgroundValue) {
							val = -narrowBand;
							negQ.push(nbr);
						}
					}
				}
			}
			while (!posQ.empty()) {
				int3 pos = posQ.front();
				posQ.pop();
				for (int n = 0; n < 6; n++) {
					int3 nbr = pos + int3(nbr6X[n], nbr6Y[n], nbr6Z[n]);
					if (nbr.x >= 0 && nbr.x < dim && nbr.y >= 0 && nbr.y < dim
							&& nbr.z >= 0 && nbr.z < dim) {
						float& val = data[nbr.x + (nbr.y + nbr.z * dim) * dim];
						if (val == backgroundValue) {
							val = narrowBand;
							posQ.push(nbr);
						}
					}
				}
			}
			for (int3 nbr : posO) {
				EndlessNodeFloat* result;
				float* nval = nullptr;
				grid.getMultiResolutionValue(nbr.x, nbr.y, nbr.z, result, nval);
				if (nval != nullptr && *nval == backgroundValue) {
					*nval = narrowBand;
				}
			}
			for (int3 nbr : negO) {
				EndlessNodeFloat* result;
				float* nval = nullptr;
				grid.getMultiResolutionValue(nbr.x, nbr.y, nbr.z, result, nval);
				if (nval != nullptr && *nval == backgroundValue) {
					*nval = -narrowBand;
				}
			}
			if (node->parent != nullptr)
				newParents.insert(node->parent);
		}
		treeDepth--;
		parents = newParents;
	}

}

float3 GetNormal(const EndlessGrid<float>& grid, int i, int j, int k) {
	EndlessNodeFloat* node = nullptr;
	float gx, gy, gz, centerVal;
	if (grid.getLeafValue(i, j, k, node, centerVal)) {
		gx = grid.getLeafValue(i + 1, j, k, node)
				- grid.getLeafValue(i - 1, j, k, node);
		gy = grid.getLeafValue(i, j + 1, k, node)
				- grid.getLeafValue(i, j - 1, k, node);
		gz = grid.getLeafValue(i, j, k + 1, node)
				- grid.getLeafValue(i, j, k - 1, node);
		return float3(gx, gy, gz);
	} else {
		gx = grid.getLeafValue(i + 1, j, k) - grid.getLeafValue(i - 1, j, k);
		gy = grid.getLeafValue(i, j + 1, k) - grid.getLeafValue(i, j - 1, k);
		gz = grid.getLeafValue(i, j, k + 1) - grid.getLeafValue(i, j, k - 1);
		return float3(gx, gy, gz);
	}
}
float4 GetNormalAndValue(const EndlessGrid<float>& grid, int i, int j, int k) {
	EndlessNodeFloat* node = nullptr;
	float gx, gy, gz, centerVal;
	if (grid.getLeafValue(i, j, k, node, centerVal)) {
		gx = grid.getLeafValue(i + 1, j, k, node)
				- grid.getLeafValue(i - 1, j, k, node);
		gy = grid.getLeafValue(i, j + 1, k, node)
				- grid.getLeafValue(i, j - 1, k, node);
		gz = grid.getLeafValue(i, j, k + 1, node)
				- grid.getLeafValue(i, j, k - 1, node);
		return float4(gx, gy, gz, centerVal);
	} else {
		gx = grid.getLeafValue(i + 1, j, k) - grid.getLeafValue(i - 1, j, k);
		gy = grid.getLeafValue(i, j + 1, k) - grid.getLeafValue(i, j - 1, k);
		gz = grid.getLeafValue(i, j, k + 1) - grid.getLeafValue(i, j, k - 1);
		centerVal = grid.getLeafValue(i, j, k);
		return float4(gx, gy, gz, centerVal);
	}
}
template<class T> void GetStencilCrossInternal(const EndlessGrid<T>& grid,
		int3 pos, Stencil<T, 3>& result) {
	T centerVal;
	result.leaf = nullptr;
	if (!grid.getLeafValue(pos.x, pos.y, pos.z, result.leaf, centerVal)) {
		result.data[1][1][1] = grid.getLeafValue(pos.x, pos.y, pos.z);

		result.data[0][1][1] = grid.getLeafValue(pos.x - 1, pos.y, pos.z);
		result.data[2][1][1] = grid.getLeafValue(pos.x + 1, pos.y, pos.z);

		result.data[1][0][1] = grid.getLeafValue(pos.x, pos.y - 1, pos.z);
		result.data[1][2][1] = grid.getLeafValue(pos.x, pos.y + 1, pos.z);

		result.data[1][1][0] = grid.getLeafValue(pos.x, pos.y, pos.z - 1);
		result.data[1][1][2] = grid.getLeafValue(pos.x, pos.y, pos.z + 1);

	} else {
		result.data[1][1][1] = centerVal;

		result.data[0][1][1] = grid.getLeafValue(pos.x - 1, pos.y, pos.z,
				result.leaf);
		result.data[2][1][1] = grid.getLeafValue(pos.x + 1, pos.y, pos.z,
				result.leaf);

		result.data[1][0][1] = grid.getLeafValue(pos.x, pos.y - 1, pos.z,
				result.leaf);
		result.data[1][2][1] = grid.getLeafValue(pos.x, pos.y + 1, pos.z,
				result.leaf);

		result.data[1][1][0] = grid.getLeafValue(pos.x, pos.y, pos.z - 1,
				result.leaf);
		result.data[1][1][2] = grid.getLeafValue(pos.x, pos.y, pos.z + 1,
				result.leaf);
	}
}
void GetStencilCross(const EndlessGrid<float>& grid, int3 pos,
		Stencil<float, 3>& result) {
	GetStencilCrossInternal(grid, pos, result);
}
void GetStencilCross(const EndlessGrid<int>& grid, int3 pos,
		Stencil<int, 3>& result) {
	GetStencilCrossInternal(grid, pos, result);
}
template<class T, int N> void GetStencilBlockInternal(
		const EndlessGrid<T>& grid, int3 pos, Stencil<T, N>& result) {
	T centerVal;
	result.leaf = nullptr;
	const int M = N / 2;
	if (!grid.getLeafValue(pos.x, pos.y, pos.z, result.leaf, centerVal)) {
		for (int k = 0; k < N; k++) {
			for (int j = 0; j < N; j++) {
				for (int i = 0; i < N; i++) {
					result.data[i][j][k] = grid.getLeafValue(pos.x - M,
							pos.y - M, pos.z - M);
				}
			}
		}
	} else {
		for (int k = 0; k < N; k++) {
			for (int j = 0; j < N; j++) {
				for (int i = 0; i < N; i++) {
					if (i == M && j == M && k == M) {
						result.data[i][j][k] = centerVal;
					} else {
						result.data[i][j][k] = grid.getLeafValue(pos.x - M,
								pos.y - M, pos.z - M, result.leaf);
					}
				}
			}
		}
	}
}

void GetStencilBlock(const EndlessGrid<float>& grid, int3 pos,
		Stencil<float, 3>& result) {
	GetStencilBlockInternal(grid, pos, result);
}
void GetStencilBlock(const EndlessGrid<float>& grid, int3 pos,
		Stencil<float, 5>& result) {
	GetStencilBlockInternal(grid, pos, result);
}
void GetStencilBlock(const EndlessGrid<float>& grid, int3 pos,
		Stencil<float, 7>& result) {
	GetStencilBlockInternal(grid, pos, result);
}

void GetStencilBlock(const EndlessGrid<int>& grid, int3 pos,
		Stencil<int, 3>& result) {
	GetStencilBlockInternal(grid, pos, result);
}
void GetStencilBlock(const EndlessGrid<int>& grid, int3 pos,
		Stencil<int, 5>& result) {
	GetStencilBlockInternal(grid, pos, result);
}
void GetStencilBlock(const EndlessGrid<int>& grid, int3 pos,
		Stencil<int, 7>& result) {
	GetStencilBlockInternal(grid, pos, result);
}

float GetInterpolatedValue(const EndlessGrid<float>& grid, float x, float y,
		float z) {
	int x1 = (int) std::ceil(x);
	int y1 = (int) std::ceil(y);
	int z1 = (int) std::ceil(z);
	int x0 = (int) std::floor(x);
	int y0 = (int) std::floor(y);
	int z0 = (int) std::floor(z);
	float dx = x - x0;
	float dy = y - y0;
	float dz = z - z0;
	float hx = 1.0f - dx;
	float hy = 1.0f - dy;
	float hz = 1.0f - dz;
	EndlessNodeFloat* node = nullptr;
	float centerVal;
	if (!grid.getLeafValue(x0, y0, z0, node, centerVal)) {
		return ((((grid.getLeafValue(x0, y0, z0) * hx
				+ grid.getLeafValue(x1, y0, z0) * dx) * hy
				+ (grid.getLeafValue(x0, y1, z0) * hx
						+ grid.getLeafValue(x1, y1, z0) * dx) * dy) * hz
				+ ((grid.getLeafValue(x0, y0, z1) * hx
						+ grid.getLeafValue(x1, y0, z1) * dx) * hy
						+ (grid.getLeafValue(x0, y1, z1) * hx
								+ grid.getLeafValue(x1, y1, z1) * dx) * dy) * dz));
	} else {
		return ((((centerVal * hx + grid.getLeafValue(x1, y0, z0, node) * dx)
				* hy
				+ (grid.getLeafValue(x0, y1, z0, node) * hx
						+ grid.getLeafValue(x1, y1, z0, node) * dx) * dy) * hz
				+ ((grid.getLeafValue(x0, y0, z1, node) * hx
						+ grid.getLeafValue(x1, y0, z1, node) * dx) * hy
						+ (grid.getLeafValue(x0, y1, z1, node) * hx
								+ grid.getLeafValue(x1, y1, z1, node) * dx) * dy)
						* dz));
	}

}
float3 GetInterpolatedNormal(const EndlessGrid<float>& grid, float x, float y,
		float z) {
	int x1 = (int) std::ceil(x);
	int y1 = (int) std::ceil(y);
	int z1 = (int) std::ceil(z);
	int x0 = (int) std::floor(x);
	int y0 = (int) std::floor(y);
	int z0 = (int) std::floor(z);
	float dx = x - x0;
	float dy = y - y0;
	float dz = z - z0;
	float hx = 1.0f - dx;
	float hy = 1.0f - dy;
	float hz = 1.0f - dz;
	return aly::float3(
			(((GetNormal(grid, x0, y0, z0) * hx
					+ GetNormal(grid, x1, y0, z0) * dx) * hy
					+ (GetNormal(grid, x0, y1, z0) * hx
							+ GetNormal(grid, x1, y1, z0) * dx) * dy) * hz
					+ ((GetNormal(grid, x0, y0, z1) * hx
							+ GetNormal(grid, x1, y0, z1) * dx) * hy
							+ (GetNormal(grid, x0, y1, z1) * hx
									+ GetNormal(grid, x1, y1, z1) * dx) * dy)
							* dz));
}

float4x4 PointsToLevelSet(const Mesh& mesh, EndlessGrid<float>& grid,
		float voxelResolution, float surfelSize,
		const std::function<bool(const std::string& message, float progress)>& monitor) {
	const float narrowBand = 2.5f;
	EndlessGrid<float> weights(grid.getLevelSizes(), 0.0f);
	grid.clear();
	grid.setBackgroundValue(narrowBand + 0.5f);

	box3f bbox = mesh.getBoundingBox();
	size_t N = mesh.vertexLocations.size();
	int dim = std::max(std::ceil(2.0f * surfelSize / voxelResolution + 1),
			2 * narrowBand);
	int3 minIndex = int3(bbox.position / voxelResolution - (float) dim - 1.0f);
	float4x4 T = MakeScale(voxelResolution) * MakeTranslation(float3(minIndex));
	for (size_t idx = 0; idx < N; idx++) {
		if (monitor) {
			if (idx % 1000 == 0) {
				if (!monitor("Converting points to level set", idx / (float) N))
					break;
			}
		}
		float3 vert = mesh.vertexLocations[idx];
		float3 norm = mesh.vertexNormals[idx];
		int3 pos = int3(aly::floor((vert - surfelSize) / voxelResolution));
		for (int k = 0; k <= dim; k++) {
			for (int j = 0; j <= dim; j++) {
				for (int i = 0; i <= dim; i++) {
					float3 pt = float3(voxelResolution * (i + pos.x),
							voxelResolution * (j + pos.y),
							voxelResolution * (k + pos.z));
					int3 loc = int3(i + pos.x - minIndex.x,
							j + pos.y - minIndex.y, k + pos.z - minIndex.z);
					float d = distance(pt, vert);
					if (d < surfelSize) {
						float sd = aly::clamp(
								dot(pt - vert, norm) / voxelResolution,
								-narrowBand, narrowBand);
						float& value = grid.getLeafValue(loc.x, loc.y, loc.z);
						float& weight = weights.getLeafValue(loc.x, loc.y,
								loc.z);
						value = (value * weight + sd) / (weight + 1.0f);
						weight = weight + 1.0f;
					}
				}
			}
		}
	}
	return T;
}
void RebuildDistanceField(EndlessGrid<float>& grid,float maxDistance) {
	DistanceField3f df;
	df.solve(grid, maxDistance);
}
void RebuildDistanceFieldFast(EndlessGrid<float>& grid, float maxDistance) {
	float nbrs[6];
	float dist = 0.0f;
	int i, j, k;
	int dim;
	int3 pos;
	float current, oldVal, extreme;
	grid.setBackgroundValue(maxDistance + 0.5f);
	EndlessGrid<float> storage(grid.getLevelSizes(), maxDistance + 0.5f);
	EndlessGrid<float>* in = &grid;
	EndlessGrid<float>* out = &storage;
	for (EndlessNodeFloat* leaf : in->getLeafNodes()) {
		dim = leaf->dim;
		pos = leaf->location;
		for (int kk = 0; kk < dim; kk++) {
			for (int jj = 0; jj < dim; jj++) {
				for (int ii = 0; ii < dim; ii++) {
					i = pos.x + ii;
					j = pos.y + jj;
					k = pos.z + kk;
					current = in->getLeafValue(i, j, k, leaf);
					nbrs[0] = in->getLeafValue(i - 1, j, k, leaf);
					nbrs[1] = in->getLeafValue(i, j + 1, k, leaf);
					nbrs[2] = in->getLeafValue(i, j - 1, k, leaf);
					nbrs[3] = in->getLeafValue(i + 1, j, k, leaf);
					nbrs[4] = in->getLeafValue(i, j, k - 1, leaf);
					nbrs[5] = in->getLeafValue(i, j, k + 1, leaf);
					extreme = current;
					if (current > 0) {
						for (int i = 0; i < 6; i++) {
							extreme = std::min(extreme, nbrs[i]);
						}
						if (extreme <= 0) {
							dist = current / (current - extreme);
						} else {
							dist = 1.5f;
						}
					} else {
						for (int i = 0; i < 6; i++) {
							extreme = std::max(extreme, nbrs[i]);
						}
						if (extreme > 0) {
							dist = current / (extreme - current);
						} else {
							dist = -1.5f;
						}
					}
					out->getLeafValue(i, j, k) = dist;
				}
			}
		}
	}
	in->clear();
	std::swap(in, out);
	int N = std::ceil(maxDistance);
	if (N % 2 == 0)
		N++;
	for (int b = 0; b < N; b++) {
		for (EndlessNodeFloat* leaf : in->getLeafNodes()) {
			dim = leaf->dim;
			pos = leaf->location;
			for (int kk = 0; kk < dim; kk++) {
				for (int jj = 0; jj < dim; jj++) {
					for (int ii = 0; ii < dim; ii++) {
						i = pos.x + ii;
						j = pos.y + jj;
						k = pos.z + kk;
						float v211 = in->getLeafValue(i + 1, j, k, leaf);
						float v011 = in->getLeafValue(i - 1, j, k, leaf);
						float v121 = in->getLeafValue(i, j + 1, k, leaf);
						float v101 = in->getLeafValue(i, j - 1, k, leaf);
						float v112 = in->getLeafValue(i, j, k + 1, leaf);
						float v110 = in->getLeafValue(i, j, k - 1, leaf);
						current = oldVal = in->getLeafValue(i, j, k);
						if (current < -b + 0.5f) {
							current = -(1E10);
							if (v011 <= 1)
								current = max(v011, current);
							if (v121 <= 1)
								current = max(v121, current);
							if (v101 <= 1)
								current = max(v101, current);
							if (v211 <= 1)
								current = max(v211, current);
							if (v110 <= 1)
								current = max(v110, current);
							if (v112 <= 1)
								current = max(v112, current);
							current -= 1.0f;
						} else if (current > b - 0.5f) {
							current = (1E10);
							if (v011 >= -1)
								current = min(v011, current);
							if (v121 >= -1)
								current = min(v121, current);
							if (v101 >= -1)
								current = min(v101, current);
							if (v211 >= -1)
								current = min(v211, current);
							if (v110 >= -1)
								current = min(v110, current);
							if (v112 >= -1)
								current = min(v112, current);
							current += 1.0f;
						}
						if (oldVal * current > 0) {
							out->getLeafValue(i, j, k) = aly::clamp(current,
									-maxDistance, maxDistance);
						}
					}
				}
			}
		}
		std::swap(in, out);
	}
}
float4x4 MeshToLevelSet(const Mesh& mesh, EndlessGrid<float>& grid,
		float narrowBand, bool flipSign, float voxelScale,
		const std::function<bool(const std::string& message, float progress)>& monitor) {
	const int nbr6X[6] = { 1, -1, 0, 0, 0, 0 };
	const int nbr6Y[6] = { 0, 0, 1, -1, 0, 0 };
	const int nbr6Z[6] = { 0, 0, 0, 0, 1, -1 };

	const int nbr26X[26] = { 1, 0, -1, 1, 0, -1, 1, 0, -1, 1, 0, -1, 1, -1, 1,
			0, -1, 1, 0, -1, 1, 0, -1, 1, 0, -1 };
	const int nbr26Y[26] = { 1, 1, 1, 0, 0, 0, -1, -1, -1, 1, 1, 1, 0, 0, -1,
			-1, -1, 1, 1, 1, 0, 0, 0, -1, -1, -1 };
	const int nbr26Z[26] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
			-1, -1, -1, -1, -1, -1, -1, -1, -1 };
	std::set<EndlessNodeFloat*> parents;
	const float trustDistance = 1.25f;
	narrowBand = std::max(trustDistance + 1.0f, narrowBand);
	float sgn = ((flipSign) ? -1.0f : 1.0f);
	grid.clear();
	float backgroundValue = (narrowBand + 0.5f) * sgn;
	grid.setBackgroundValue(backgroundValue);
	box3f bbox = mesh.getBoundingBox();
	std::vector<box3f> splats;
	splats.reserve(mesh.triIndexes.size());
	double averageSize = 0;

	//Calculate average size of triangle to scale mesh to grid accordingly.
	for (uint3 tri : mesh.triIndexes.data) {
		float3 v1 = mesh.vertexLocations[tri.x];
		float3 v2 = mesh.vertexLocations[tri.y];
		float3 v3 = mesh.vertexLocations[tri.z];
		averageSize += 0.5f * std::abs(crossMag(v2 - v1, v3 - v1));
		float3 minPt = aly::min(aly::min(v1, v2), v3);
		float3 maxPt = aly::max(aly::max(v1, v2), v3);
		splats.push_back(box3f(minPt, maxPt - minPt));
	}
	averageSize /= mesh.triIndexes.size();
	float res = voxelScale * std::sqrt(averageSize);
	int3 minIndex = int3(bbox.position / res - narrowBand * 3.0f);
	float4x4 T = MakeScale(res) * MakeTranslation(float3(minIndex));
	//Splat regions around triangles
	size_t N = splats.size();
	for (size_t n = 0; n < N; n++) {
		if (monitor) {
			if (n % 200 == 0) {
				if (!monitor("Converting mesh to level set", n / (float) N))
					break;
			}
		}

		box3f box = splats[n];
		int3 dims = int3(
				aly::round(box.dimensions / res) + 0.5f + 2 * narrowBand);
		int3 pos = int3(aly::floor(box.position / res) - 0.25f - narrowBand);
		uint3 tri = mesh.triIndexes[n];
		float3 v1 = mesh.vertexLocations[tri.x];
		float3 v2 = mesh.vertexLocations[tri.y];
		float3 v3 = mesh.vertexLocations[tri.z];
		float3 n1 = mesh.vertexNormals[tri.x];
		float3 n2 = mesh.vertexNormals[tri.y];
		float3 n3 = mesh.vertexNormals[tri.z];
		float3 closestPoint;
		for (int k = 0; k <= dims.z; k++) {
			for (int j = 0; j <= dims.y; j++) {
				for (int i = 0; i <= dims.x; i++) {
					float3 pt = float3(res * (i + pos.x), res * (j + pos.y),
							res * (k + pos.z));
					float d = std::sqrt(
							DistanceToTriangleSqr(pt, v1, v2, v3,
									&closestPoint)) / res;
					if (d <= narrowBand) {
						int3 loc = int3(i + pos.x - minIndex.x,
								j + pos.y - minIndex.y, k + pos.z - minIndex.z);
						float& value = grid.getLeafValue(loc.x, loc.y, loc.z);
						if (d < std::abs(value)) {
							if (d <= trustDistance) {
								//Interpolate normal to compute sign. Addresses cusp issue.
								value =
										d * sgn
												* (int) aly::sign(
														dot(pt - closestPoint,
																FromBary(
																		ToBary(
																				closestPoint,
																				v1,
																				v2,
																				v3),
																		n1, n2,
																		n3)));
							} else {
								value = d * sgn;
							}
						}
					}
				}
			}
		}
	}
	splats.clear();
	std::list<EndlessNodeFloat*> leafs = grid.getLeafNodes();
	assert(leafs.size() > 0);
	grid.allocateInternalNodes();
	//Flood fill leaf nodes with narrowband value.
	for (EndlessNodeFloat* leaf : leafs) {
		if (leaf->parent != nullptr)
			leaf->parent->data[leaf->parent->getIndex(leaf)] = 0;
		int3 location = leaf->location;
		std::vector<float>& data = leaf->data;
		int dim = leaf->dim;
		std::queue<int3> posQ;
		for (int k = 0; k < dim; k++) {
			for (int j = 0; j < dim; j++) {
				for (int i = 0; i < dim; i++) {
					float& val = data[i + (j + k * dim) * dim];
					if (val != backgroundValue) {
						for (int n = 0; n < 26; n++) {
							int3 nbr = int3(i + nbr26X[n], j + nbr26Y[n],
									k + nbr26Z[n]);
							if (nbr.x >= 0 && nbr.x < dim && nbr.y >= 0
									&& nbr.y < dim && nbr.z >= 0
									&& nbr.z < dim) {
								float& nval = data[nbr.x
										+ (nbr.y + nbr.z * dim) * dim];
								if (nval == backgroundValue) {
									nval = narrowBand * sgn;
									posQ.push(nbr);
								}
							}
						}
					}
				}
			}
		}

		while (!posQ.empty()) {
			int3 pos = posQ.front();
			posQ.pop();
			for (int n = 0; n < 6; n++) {
				int3 nbr = pos + int3(nbr6X[n], nbr6Y[n], nbr6Z[n]);
				if (nbr.x >= 0 && nbr.x < dim && nbr.y >= 0 && nbr.y < dim
						&& nbr.z >= 0 && nbr.z < dim) {
					float& val = data[nbr.x + (nbr.y + nbr.z * dim) * dim];
					if (val == backgroundValue) {
						val = narrowBand * sgn;
						posQ.push(nbr);
					}
				}
			}
		}

	}
	if (monitor) {
		if (!monitor("Flood Fill", 0.25f))
			return T;
	}
	//Find extremety leaf nodes.
	EndlessNodeFloat* minX;
	EndlessNodeFloat* minY;
	EndlessNodeFloat* minZ;
	EndlessNodeFloat* maxX;
	EndlessNodeFloat* maxY;
	EndlessNodeFloat* maxZ;

	std::queue<std::pair<int3, float>> queue;
	int3 maxPt = int3(-100000000, -100000000, -100000000);
	int3 minPt = int3(100000000, 100000000, 100000000);
	for (EndlessNodeFloat* leaf : leafs) {
		if (leaf->location.x > maxPt.x) {
			maxX = leaf;
			maxPt.x = leaf->location.x;
		}
		if (leaf->location.y > maxPt.y) {
			maxY = leaf;
			maxPt.y = leaf->location.y;

		}
		if (leaf->location.z > maxPt.z) {
			maxZ = leaf;
			maxPt.z = leaf->location.z;
		}
		if (leaf->location.x < minPt.x) {
			minX = leaf;
			minPt.x = leaf->location.x;

		}
		if (leaf->location.y < minPt.y) {
			minY = leaf;
			minPt.y = leaf->location.y;

		}
		if (leaf->location.z < minPt.z) {
			minZ = leaf;
			minPt.z = leaf->location.z;
		}
	}
	{
		int dim = maxX->dim;
		int3 loc = maxX->location;
		for (int y = 0; y < dim; y++) {
			for (int z = 0; z < dim; z++) {
				int3 pos = int3(loc.x + dim - 1, loc.y + y, loc.z + z);
				float& value = (*maxX)(dim - 1, y, z);
				queue.push( { pos, value });
				value = -value;
			}
		}
	}
	{
		int dim = maxY->dim;
		int3 loc = maxY->location;
		for (int x = 0; x < dim; x++) {
			for (int z = 0; z < dim; z++) {
				int3 pos = int3(loc.x + x, loc.y + dim - 1, loc.z + z);
				float& value = (*maxY)(x, dim - 1, z);
				queue.push( { pos, value });
				value = -value;
			}
		}
	}
	{
		int dim = maxZ->dim;
		int3 loc = maxZ->location;
		for (int x = 0; x < dim; x++) {
			for (int y = 0; y < dim; y++) {
				int3 pos = int3(loc.x, loc.y + y, loc.z + dim - 1);
				float& value = (*maxZ)(x, y, dim - 1);
				queue.push( { pos, value });
				value = -value;
			}
		}
	}

	{
		int dim = minX->dim;
		int3 loc = minX->location;
		for (int y = 0; y < dim; y++) {
			for (int z = 0; z < dim; z++) {
				int3 pos = int3(loc.x, loc.y + y, loc.z + z);
				float& value = (*minX)(0, y, z);
				queue.push( { pos, value });
				value = -value;
			}
		}
	}
	{
		int dim = minY->dim;
		int3 loc = minY->location;
		for (int x = 0; x < dim; x++) {
			for (int z = 0; z < dim; z++) {
				int3 pos = int3(loc.x + x, loc.y, loc.z + z);
				float& value = (*minY)(x, 0, z);
				queue.push( { pos, value });
				value = -value;
			}
		}
	}
	{
		int dim = minZ->dim;
		int3 loc = minZ->location;
		for (int x = 0; x < dim; x++) {
			for (int y = 0; y < dim; y++) {
				int3 pos = int3(loc.x, loc.y + y, loc.z);
				float& value = (*minZ)(x, y, 0);
				queue.push( { pos, value });
				value = -value;
			}
		}
	}
	EndlessNodeFloat* result;
	float* nval;
	//Flood extremity leaf nodes with sign value.
	while (queue.size() > 0) {
		std::pair<int3, float> pr = queue.front();
		float value = pr.second;
		queue.pop();
		for (int n = 0; n < 6; n++) {
			int3 nbr = pr.first + int3(nbr6X[n], nbr6Y[n], nbr6Z[n]);
			grid.getMultiResolutionValue(nbr.x, nbr.y, nbr.z, result, nval);
			if (nval != nullptr && std::abs(*nval) > trustDistance
					&& sgn * (*nval) > 0) {
				if (result->isLeaf()) {
					queue.push( { nbr, *nval });
					*nval = -*nval;
				}
			}
		}
	}
	if (monitor) {
		if (!monitor("Flood Fill", 0.5f))
			return T;
	}
	//Flood fill rest of space using signed level set values, propagating the proper sign in the process.
	FloodFill(grid, narrowBand);
	return T;
}

}
