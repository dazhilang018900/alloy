/*
 * Copyright(C) 2016, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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
#include "image/AlloyGradientVectorFlow.h"
#include "math/AlloySparseSolve.h"
namespace aly {
void SolveEdgeFilter(const ImageRGB& in, Image1f& out, int K) {
	out.resize(in.width, in.height);
	out.set(float1(1.0f));
	std::vector<int2> nbrs;
	for (int jj = -K; jj <= K; jj++) {
		for (int ii = -K; ii <= K; ii++) {
			nbrs.push_back(int2(ii, jj));
		}
	}
	int KK = nbrs.size();
#pragma omp parallel for
	for (int j = K; j < in.height - K; j++) {
		for (int i = K; i < in.width - K; i++) {
			float3 sum(0.0f);
			for (int2 nbr : nbrs) {
				sum += ToRGBf(in(i + nbr.x, j + nbr.y));
			}
			sum /= (float) KK;
			float3 res(0.0f);
			for (int2 nbr : nbrs) {
				res += aly::abs(sum - ToRGBf(in(i + nbr.x, j + nbr.y)));
			}
			out(i, j).x = lengthL1(res) / (3.0f * KK);
		}
	}
}
void SolveEdgeFilter(const Image1f& in, Image1f& out, int K) {
	out.resize(in.width, in.height);
	out.set(float1(1.0f));
	std::vector<int2> nbrs;
	for (int jj = -K; jj <= K; jj++) {
		for (int ii = -K; ii <= K; ii++) {
			nbrs.push_back(int2(ii, jj));
		}
	}
	int KK = nbrs.size();
#pragma omp parallel for
	for (int j = K; j < in.height - K; j++) {
		for (int i = K; i < in.width - K; i++) {
			float sum(0.0f);
			for (int2 nbr : nbrs) {
				sum += in(i + nbr.x, j + nbr.y).x;
			}
			sum /= (float) KK;
			float res(0.0f);
			for (int2 nbr : nbrs) {
				res += std::abs(sum - in(i + nbr.x, j + nbr.y).x);
			}
			out(i, j).x = res / (KK);
		}
	}
}

void SolveEdgeFilter(const ImageRGBf& in, Image1f& out, int K) {
	out.resize(in.width, in.height);
	out.set(float1(1.0f));
	std::vector<int2> nbrs;
	for (int jj = -K; jj <= K; jj++) {
		for (int ii = -K; ii <= K; ii++) {
			nbrs.push_back(int2(ii, jj));
		}
	}
	int KK = nbrs.size();
#pragma omp parallel for
	for (int j = K; j < in.height - K; j++) {
		for (int i = K; i < in.width - K; i++) {
			float3 sum(0.0f);
			for (int2 nbr : nbrs) {
				sum += in(i + nbr.x, j + nbr.y);
			}
			sum /= (float) KK;
			float3 res(0.0f);
			for (int2 nbr : nbrs) {
				res += aly::abs(sum - in(i + nbr.x, j + nbr.y));
			}
			out(i, j).x = lengthL1(res) / (3.0f * KK);
		}
	}
}

void SolveEdgeFilter(const Volume1f& in, Volume1f& out, int K) {
	out.resize(in.rows, in.cols, in.slices);
	out.set(float1(1.0f));
	std::vector<int3> nbrs;
	for (int kk = -K; kk <= K; kk++) {
		for (int jj = -K; jj <= K; jj++) {
			for (int ii = -K; ii <= K; ii++) {
				nbrs.push_back(int3(ii, jj, kk));
			}
		}
	}
	int KK = nbrs.size();
#pragma omp parallel for
	for (int k = K; k < in.slices - K; k++) {
		for (int j = K; j < in.cols - K; j++) {
			for (int i = K; i < in.rows - K; i++) {
				float sum(0.0f);
				for (int3 nbr : nbrs) {
					sum += in(i + nbr.x, j + nbr.y, k + nbr.z).x;
				}
				sum /= (float) KK;
				float res(0.0f);
				for (int3 nbr : nbrs) {
					res += std::abs(
							sum - in(i + nbr.x, j + nbr.y, k + nbr.z).x);
				}
				out(i, j, k).x = res / (KK);
			}
		}
	}
}
void SolveGradientVectorFlow(const Image1f& src, Image2f& vectorField, float mu,
		int iterations, bool normalize) {
	const int nbrX[] = { 0, 0, -1, 1 };
	const int nbrY[] = { 1, -1, 0, 0 };
	vectorField.resize(src.width, src.height);
	SparseMatrix2f A;
	Vector2f b;
	Vector2f x;
	//size_t N = 0;
	size_t M = src.width * src.height;
	A.resize(M, M);
	b.resize(M, float2(0.0f));
	x.resize(M, float2(0.0f));
	for (int i = 0; i < src.width; i++) {
		for (int j = 0; j < src.height; j++) {
			int idx = i + j * src.width;
			float v21 = src(i + 1, j).x;
			float v12 = src(i, j + 1).x;
			float v10 = src(i, j - 1).x;
			float v01 = src(i - 1, j).x;
			float v11 = src(i, j).x;
			float2 grad;
			if (v11 < 0.0f) {
				grad.x = std::max(v11 - v01, 0.0f) + std::min(v21 - v11, 0.0f);
				grad.y = std::max(v11 - v10, 0.0f) + std::min(v12 - v11, 0.0f);
			} else {
				grad.x = std::min(v11 - v01, 0.0f) + std::max(v21 - v11, 0.0f);
				grad.y = std::min(v11 - v10, 0.0f) + std::max(v12 - v11, 0.0f);
			}
			float len = max(1E-6f, length(grad));
			grad = -sign(v11) * (grad / std::max(1E-6f, len));
			x[idx] = grad;
			b[idx] = grad * len;
			A(idx, idx) = float2(-len - mu);
			for (int nn = 0; nn < 4; nn++) {
				int ii = i + nbrX[nn];
				int jj = j + nbrY[nn];
				if (ii >= 0 && ii < src.width && jj >= 0 && jj < src.height) {
					A(idx, ii + src.width * jj) += float2(mu * 0.25f);

				}
			}
		}
	}
	SolveVecCG(b, A, x, iterations, 1E-20f);
	vectorField.set(x.data);
	const float minSpeed = 0.1f;
	const float captureDist = 1.5f;
	if (normalize) {
#pragma omp parallel for
		for (int i = 0; i < src.width; i++) {
			for (int j = 0; j < src.height; j++) {
				float d = std::abs(src(i, j).x);
				vectorField(i, j) = aly::normalize(vectorField(i, j))
						* (minSpeed
								+ (1.0f - minSpeed)
										* clamp(d, 0.0f, captureDist)
										/ captureDist);
			}
		}
	}
}

void SolveGradientVectorFlow(const Volume1f& src, Volume3f& vectorField,
		float mu, int iterations, bool normalize) {
	const int nbrX[] = { -1, 1, 0, 0, 0, 0 };
	const int nbrY[] = { 0, 0, -1, 1, 0, 0 };
	const int nbrZ[] = { 0, 0, 0, 0, -1, 1 };
	vectorField.resize(src.rows, src.cols, src.slices);
	SparseMatrix3f A;
	Vector3f b;
	Vector3f x;
	//size_t N = 0;
	size_t M = src.rows * src.cols * src.slices;
	A.resize(M, M);
	b.resize(M, float3(0.0f));
	x.resize(M, float3(0.0f));
	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {
			for (int k = 0; k < src.slices; k++) {
				int idx = i + j * src.rows + k * src.rows * src.cols;
				float v211 = src(i + 1, j, k).x;
				float v121 = src(i, j + 1, k).x;
				float v101 = src(i, j - 1, k).x;
				float v011 = src(i - 1, j, k).x;
				float v110 = src(i, j, k - 1).x;
				float v112 = src(i, j, k + 1).x;
				float v111 = src(i, j, k).x;
				float3 grad;
				if (v111 < 0.0f) {
					grad.x = std::max(v111 - v011, 0.0f)
							+ std::min(v211 - v111, 0.0f);
					grad.y = std::max(v111 - v101, 0.0f)
							+ std::min(v121 - v111, 0.0f);
					grad.z = std::max(v111 - v110, 0.0f)
							+ std::min(v112 - v111, 0.0f);
				} else {
					grad.x = std::min(v111 - v011, 0.0f)
							+ std::max(v211 - v111, 0.0f);
					grad.y = std::min(v111 - v101, 0.0f)
							+ std::max(v121 - v111, 0.0f);
					grad.z = std::min(v111 - v110, 0.0f)
							+ std::max(v112 - v111, 0.0f);
				}
				float len = max(1E-6f, length(grad));
				grad = -sign(v111) * (grad / std::max(1E-6f, len));
				x[idx] = grad;
				b[idx] = grad * len;
				A(idx, idx) = float3(-len - mu);
				for (int nn = 0; nn < 6; nn++) {
					int ii = i + nbrX[nn];
					int jj = j + nbrY[nn];
					int kk = k + nbrZ[nn];
					if (ii >= 0 && ii < src.rows && jj >= 0 && jj < src.cols
							&& kk >= 0 && kk < src.slices) {
						A(idx, ii + jj * src.rows + kk * src.rows * src.cols) +=
								float3(mu * 0.166666f);

					}
				}
			}
		}
	}
	SolveVecCG(b, A, x, iterations, 1E-20f);
	vectorField.set(x.data);
	const float minSpeed = 0.1f;
	const float captureDist = 1.5f;
	if (normalize) {
#pragma omp parallel for
		for (int i = 0; i < src.rows; i++) {
			for (int j = 0; j < src.cols; j++) {
				for (int k = 0; k < src.slices; k++) {
					float d = std::abs(src(i, j, k).x);
					vectorField(i, j, k) = aly::normalize(vectorField(i, j, k))
							* (minSpeed
									+ (1.0f - minSpeed)
											* clamp(d, 0.0f, captureDist)
											/ captureDist);
				}
			}
		}
	}
}
void SolveGradientVectorFlow(const Image1f& src, Image2f& vectorField,
		const Image1f& weights, float mu, int iterations, bool normalize) {
	const int nbrX[] = { 0, 0, -1, 1 };
	const int nbrY[] = { 1, -1, 0, 0 };
	vectorField.resize(src.width, src.height);
	SparseMatrix2f A;
	Vector2f b;
	Vector2f x;
	//size_t N = 0;
	size_t M = src.width * src.height;
	A.resize(M, M);
	b.resize(M, float2(0.0f));
	x.resize(M, float2(0.0f));
	for (int i = 0; i < src.width; i++) {
		for (int j = 0; j < src.height; j++) {
			int idx = i + j * src.width;
			float v21 = src(i + 1, j).x;
			float v12 = src(i, j + 1).x;
			float v10 = src(i, j - 1).x;
			float v01 = src(i - 1, j).x;
			float v11 = src(i, j).x;
			float2 grad;
			if (v11 < 0.0f) {
				grad.x = std::max(v11 - v01, 0.0f) + std::min(v21 - v11, 0.0f);
				grad.y = std::max(v11 - v10, 0.0f) + std::min(v12 - v11, 0.0f);
			} else {
				grad.x = std::min(v11 - v01, 0.0f) + std::max(v21 - v11, 0.0f);
				grad.y = std::min(v11 - v10, 0.0f) + std::max(v12 - v11, 0.0f);
			}
			float len = max(1E-6f, length(grad));
			grad = -sign(v11) * (grad / std::max(1E-6f, len));
			float w = weights(i, j).x;
			x[idx] = grad;
			b[idx] = w * grad;
			A(idx, idx) = float2(-w - mu);
			for (int nn = 0; nn < 4; nn++) {
				int ii = i + nbrX[nn];
				int jj = j + nbrY[nn];
				if (ii >= 0 && ii < src.width && jj >= 0 && jj < src.height) {
					A(idx, ii + src.width * jj) += float2(mu * 0.25f);

				}
			}
		}
	}
	SolveVecCG(b, A, x, iterations, 1E-20f);
	vectorField.set(x.data);
	const float minSpeed = 0.1f;
	const float captureDist = 1.5f;
	if (normalize) {
#pragma omp parallel for
		for (int i = 0; i < src.width; i++) {
			for (int j = 0; j < src.height; j++) {
				float d = std::abs(src(i, j).x);
				vectorField(i, j) = aly::normalize(vectorField(i, j))
						* (minSpeed
								+ (1.0f - minSpeed)
										* clamp(d, 0.0f, captureDist)
										/ captureDist);
			}
		}
	}
}

void SolveGradientVectorFlow(const Volume1f& src, Volume3f& vectorField,
		const Volume1f& weights, float mu, int iterations, bool normalize) {
	const int nbrX[] = { -1, 1, 0, 0, 0, 0 };
	const int nbrY[] = { 0, 0, -1, 1, 0, 0 };
	const int nbrZ[] = { 0, 0, 0, 0, -1, 1 };
	vectorField.resize(src.rows, src.cols, src.slices);
	SparseMatrix3f A;
	Vector3f b;
	Vector3f x;
	//size_t N = 0;
	size_t M = src.rows * src.cols * src.slices;
	A.resize(M, M);
	b.resize(M, float3(0.0f));
	x.resize(M, float3(0.0f));
	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {
			for (int k = 0; k < src.slices; k++) {
				int idx = i + j * src.rows + k * src.rows * src.cols;
				float v211 = src(i + 1, j, k).x;
				float v121 = src(i, j + 1, k).x;
				float v101 = src(i, j - 1, k).x;
				float v011 = src(i - 1, j, k).x;
				float v110 = src(i, j, k - 1).x;
				float v112 = src(i, j, k + 1).x;
				float v111 = src(i, j, k).x;
				float3 grad;
				if (v111 < 0.0f) {
					grad.x = std::max(v111 - v011, 0.0f)
							+ std::min(v211 - v111, 0.0f);
					grad.y = std::max(v111 - v101, 0.0f)
							+ std::min(v121 - v111, 0.0f);
					grad.z = std::max(v111 - v110, 0.0f)
							+ std::min(v112 - v111, 0.0f);
				} else {
					grad.x = std::min(v111 - v011, 0.0f)
							+ std::max(v211 - v111, 0.0f);
					grad.y = std::min(v111 - v101, 0.0f)
							+ std::max(v121 - v111, 0.0f);
					grad.z = std::min(v111 - v110, 0.0f)
							+ std::max(v112 - v111, 0.0f);
				}
				float len = max(1E-6f, length(grad));
				grad = -sign(v111) * (grad / std::max(1E-6f, len));

				float w = weights(i, j, k).x;
				x[idx] = grad;
				b[idx] = w * grad;
				A(idx, idx) = float3(-w - mu);
				for (int nn = 0; nn < 6; nn++) {
					int ii = i + nbrX[nn];
					int jj = j + nbrY[nn];
					int kk = k + nbrZ[nn];
					if (ii >= 0 && ii < src.rows && jj >= 0 && jj < src.cols
							&& kk >= 0 && kk < src.slices) {
						A(idx, ii + jj * src.rows + kk * src.rows * src.cols) +=
								float3(mu * 0.166666f);

					}
				}
			}
		}
	}
	SolveVecCG(b, A, x, iterations, 1E-20f);
	vectorField.set(x.data);
	const float minSpeed = 0.1f;
	const float captureDist = 1.5f;
	if (normalize) {
		for (int i = 0; i < src.rows; i++) {
			for (int j = 0; j < src.cols; j++) {
				for (int k = 0; k < src.slices; k++) {
					float d = std::abs(src(i, j, k).x);
					vectorField(i, j, k) = aly::normalize(vectorField(i, j, k))
							* (minSpeed
									+ (1.0f - minSpeed)
											* clamp(d, 0.0f, captureDist)
											/ captureDist);
				}
			}
		}
	}
}

void SolveGradientVectorFlow(const Image1f& src, Image2f& vectorField,
		int iterations, bool normalize) {
	const int nbrX[] = { 0, 0, -1, 1 };
	const int nbrY[] = { 1, -1, 0, 0 };
	vectorField.resize(src.width, src.height);
	SparseMatrix2f A;
	Vector2f b;
	Vector2f x;
	size_t N = 0;
	Image1b mask(src.width, src.height);
	mask.set(bool1(false));
	for (int i = 0; i < src.width; i++) {
		for (int j = 0; j < src.height; j++) {
			float sVal = src(i, j).x;
			bool masked = false;
			for (int nn = 0; nn < 4; nn++) {
				int ii = i + nbrX[nn];
				int jj = j + nbrY[nn];
				if (src(ii, jj).x * sVal < 0.0f) {
					mask(i, j) = bool1(true);
					masked = true;
					break;
				}
			}
			if (!masked) {
				N++;
			}
		}
	}
	size_t M = src.width * src.height;
	A.resize(M, M);
	b.resize(M, float2(0.0f));
	x.resize(M, float2(0.0f));
	for (int i = 0; i < src.width; i++) {
		for (int j = 0; j < src.height; j++) {
			int idx = i + j * src.width;
			if (mask(i, j).x) {
				float v21 = src(i + 1, j).x;
				float v12 = src(i, j + 1).x;
				float v10 = src(i, j - 1).x;
				float v01 = src(i - 1, j).x;
				float v11 = src(i, j).x;
				float2 grad;
				if (v11 < 0.0f) {
					grad.x = std::max(v11 - v01, 0.0f)
							+ std::min(v21 - v11, 0.0f);
					grad.y = std::max(v11 - v10, 0.0f)
							+ std::min(v12 - v11, 0.0f);
				} else {
					grad.x = std::min(v11 - v01, 0.0f)
							+ std::max(v21 - v11, 0.0f);
					grad.y = std::min(v11 - v10, 0.0f)
							+ std::max(v12 - v11, 0.0f);
				}
				float len = max(1E-6f, length(grad));
				grad = -sign(v11) * (grad / std::max(1E-6f, len));
				x[idx] = grad;
				b[idx] = grad;
				A(idx, idx) = float2(1.0f);
			} else {
				for (int nn = 0; nn < 4; nn++) {
					int ii = i + nbrX[nn];
					int jj = j + nbrY[nn];
					if (ii >= 0 && ii < src.width && jj >= 0
							&& jj < src.height) {
						A(idx, ii + src.width * jj) += float2(1.0f);
						A(idx, idx) += float2(-1.0f);
					}
				}
			}
		}
	}
	SolveVecBICGStab(b, A, x, iterations, 1E-10f);
	vectorField.set(x.data);
	const float minSpeed = 0.1f;
	const float captureDist = 1.5f;
	if (normalize) {
		for (int i = 0; i < src.width; i++) {
			for (int j = 0; j < src.height; j++) {
				float d = std::abs(src(i, j).x);
				vectorField(i, j) = aly::normalize(vectorField(i, j))
						* (minSpeed
								+ (1.0f - minSpeed)
										* clamp(d, 0.0f, captureDist)
										/ captureDist);
			}
		}
	}
}

void SolveGradientVectorFlow(const Volume1f& src, Volume3f& vectorField,
		int iterations, bool normalize) {
	const int nbrX[] = { -1, 1, 0, 0, 0, 0 };
	const int nbrY[] = { 0, 0, -1, 1, 0, 0 };
	const int nbrZ[] = { 0, 0, 0, 0, -1, 1 };
	vectorField.resize(src.rows, src.cols, src.slices);
	SparseMatrix3f A;
	Vector3f b;
	Vector3f x;
	//size_t N = 0;
	size_t M = src.rows * src.cols * src.slices;
	A.resize(M, M);
	b.resize(M, float3(0.0f));
	x.resize(M, float3(0.0f));
	size_t N = 0;
	Volume1b mask(src.rows, src.cols, src.slices);
	mask.set(bool1(false));
	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {
			for (int k = 0; k < src.slices; k++) {
				float sVal = src(i, j, k).x;
				bool masked = false;
				for (int nn = 0; nn < 6; nn++) {
					int ii = i + nbrX[nn];
					int jj = j + nbrY[nn];
					int kk = k + nbrZ[nn];
					if (src(ii, jj, kk).x * sVal < 0.0f) {
						mask(i, j, k) = bool1(true);
						masked = true;
						break;
					}
				}
				if (!masked) {
					N++;
				}
			}
		}
	}
	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {
			for (int k = 0; k < src.slices; k++) {
				int idx = i + j * src.rows + k * src.rows * src.cols;
				if (mask(i, j, k).x) {

					float v211 = src(i + 1, j, k).x;
					float v121 = src(i, j + 1, k).x;
					float v101 = src(i, j - 1, k).x;
					float v011 = src(i - 1, j, k).x;
					float v110 = src(i, j, k - 1).x;
					float v112 = src(i, j, k + 1).x;
					float v111 = src(i, j, k).x;
					float3 grad;
					if (v111 < 0.0f) {
						grad.x = std::max(v111 - v011, 0.0f)
								+ std::min(v211 - v111, 0.0f);
						grad.y = std::max(v111 - v101, 0.0f)
								+ std::min(v121 - v111, 0.0f);
						grad.z = std::max(v111 - v110, 0.0f)
								+ std::min(v112 - v111, 0.0f);
					} else {
						grad.x = std::min(v111 - v011, 0.0f)
								+ std::max(v211 - v111, 0.0f);
						grad.y = std::min(v111 - v101, 0.0f)
								+ std::max(v121 - v111, 0.0f);
						grad.z = std::min(v111 - v110, 0.0f)
								+ std::max(v112 - v111, 0.0f);
					}

					float len = max(1E-6f, length(grad));
					grad = -sign(v111) * (grad / std::max(1E-6f, len));
					x[idx] = grad;
					b[idx] = grad;
					A(idx, idx) = float3(1.0f);
				} else {
					for (int nn = 0; nn < 6; nn++) {
						int ii = i + nbrX[nn];
						int jj = j + nbrY[nn];
						int kk = k + nbrZ[nn];
						if (ii >= 0 && ii < src.rows && jj >= 0 && jj < src.cols
								&& kk >= 0 && kk < src.slices) {
							A(idx,
									ii + jj * src.rows
											+ kk * src.rows * src.cols) +=
									float3(1.0f);
							A(idx, idx) += float3(-1.0f);
						}
					}
				}
			}
		}
	}
	SolveVecCG(b, A, x, iterations, 1E-20f);
	vectorField.set(x.data);
	const float minSpeed = 0.1f;
	const float captureDist = 1.5f;
	if (normalize) {
		for (int i = 0; i < src.rows; i++) {
			for (int j = 0; j < src.cols; j++) {
				for (int k = 0; k < src.slices; k++) {
					float d = std::abs(src(i, j, k).x);
					vectorField(i, j, k) = aly::normalize(vectorField(i, j, k))
							* (minSpeed
									+ (1.0f - minSpeed)
											* clamp(d, 0.0f, captureDist)
											/ captureDist);
				}
			}
		}
	}
}

}
