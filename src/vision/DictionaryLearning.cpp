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
 */

/*

 Contains implementation of:

 Aharon, Michal, Michael Elad, and Alfred Bruckstein. "K-SVD: An algorithm for designing overcomplete
 dictionaries for sparse representation." IEEE Transactions on signal processing 54.11 (2006): 4311.

 */

#include "math/AlloyOptimization.h"
#include "vision/DictionaryLearning.h"
#include "image/AlloyImageProcessing.h"
#include "image/AlloyAnisotropicFilter.h"
#include "image/AlloyVolume.h"
#include "common/cereal/archives/xml.hpp"
#include "common/cereal/archives/json.hpp"
#include "common/cereal/archives/portable_binary.hpp"
#include <set>
#include "common/AlloyUnits.h"

namespace aly {
void SamplePatch::sample(const aly::Image1f& gray) {
	data.resize(width * height);
	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			float2 pix(width * (i / (float) (width - 1) - 0.5f),
					height * (j / (float) (height - 1) - 0.5f));
			data[i + j * width] = gray(position.x + pix.x, position.y + pix.y);
		}
	}
}
void SamplePatch::sample(const DenseMat<float>& gray) {
	data.resize(width * height);
	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			float2 pix(width * (i / (float) (width - 1) - 0.5f),
					height * (j / (float) (height - 1) - 0.5f));
			data[i + j * width] = gray(clamp((int)(position.x + pix.x),0,gray.rows-1),clamp((int)(position.y + pix.y),0,gray.cols-1));
		}
	}
}
void SamplePatch::sample(const aly::Image3f& colors, int c) {
	data.resize(width * height);
	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			float2 pix(width * (i / (float) (width - 1) - 0.5f),
					height * (j / (float) (height - 1) - 0.5f));
			data[i + j * width] =
					colors(position.x + pix.x, position.y + pix.y)[c];
		}
	}
}
void SamplePatch::sample(const aly::ImageRGB& colors, int c) {
	data.resize(width * height);
	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			float2 pix(width * (i / (float) (width - 1) - 0.5f),
					height * (j / (float) (height - 1) - 0.5f));
			data[i + j * width] =
					colors(position.x + pix.x, position.y + pix.y)[c] / 255.0f;
		}
	}
}
float SamplePatch::residual(const std::vector<FilterBank>& banks,
		bool correlation) {
	int N = (int) data.size();
	int M = (int) weights.size();
	if (weights.size() != banks.size()) {
		throw std::runtime_error(
				MakeString() << "Weight vector doesn't match filter banks "
						<< weights.size() << " " << banks.size());
	}
	double err = 0;
	if (correlation) {
		double length1 = 0;
		double length2 = 0;
		for (int i = 0; i < N; i++) {
			float val = 0;
			for (int j = 0; j < M; j++) {
				val += banks[j].data[i] * weights[j];
			}
			length1 += aly::sqr(data[i]);
			length2 += aly::sqr(val);
			err += data[i] * val;
		}
		err = -err / std::sqrt(length1 * length2);
	} else {
		for (int i = 0; i < N; i++) {
			float val = 0;
			for (int j = 0; j < M; j++) {
				val += banks[j].data[i] * weights[j];
			}

			err += std::abs(data[i] - val);
		}
		err /= N;
	}
	return (float) err;
}
std::vector<float> SamplePatch::getLabelWeights(
		const std::vector<FilterBank>& banks) {
	std::vector<float> histogram(5, 0.0f);
	for (int k = 0; k < weights.size(); k++) {
		int idx = static_cast<int>(banks[k].type);
		assert(idx >= 0 && idx < 5);
		histogram[idx] += std::abs(weights[k]);
	}
	/*
	 FilterType label=FilterType::Unknown;
	 float bestWeight=0;
	 for(int n=1;n<histogram.size();n++){
	 if(histogram[n]>bestWeight){
	 label=static_cast<FilterType>(n);
	 bestWeight=histogram[n];
	 }
	 }
	 */
	return histogram;
}
float SamplePatch::synthesize(const std::vector<FilterBank>& banks,
		std::vector<float>& sample) {
	int N = (int) data.size();
	int M = (int) weights.size();
	double err = 0;
	sample.resize(width * height);
	for (int i = 0; i < sample.size(); i++) {
		float val = 0;
		for (int j = 0; j < M; j++) {
			val += banks[j].data[i] * weights[j];
		}
		err += std::abs(data[i] - val);
		sample[i] = clamp(val, 0.0f, 1.0f);
	}
	err /= N;
	return (float) err;
}
void SamplePatch::resize(int w, int h) {
	data.resize(w * h, 0.0f);
	width = w;
	height = h;
}
double FilterBank::score(const std::vector<float>& sample, bool correlation) {
	int N = (int) std::min(data.size(), sample.size());
	double err = 0;
	//use positive correlation to avoid cancellations
	if (correlation) {
		for (int i = 0; i < N; i++) {
			err += data[i] * sample[i];
		}
		err = std::abs(err) / N;
		return -err;
	} else {
		for (int i = 0; i < N; i++) {
			err += std::abs(data[i] - sample[i]);
		}
		err /= N;
		return err;
	}
}

void DictionaryLearning::maxPool(const aly::Image3f& image, aly::Image3f& out,
		int size) {
	out.resize(image.width / size, image.height / size);
	for (int j = 0; j < out.height; j++) {
		for (int i = 0; i < out.width; i++) {
			float3 maxVal(-1E30f);
			for (int jj = 0; jj < size; jj++) {
				for (int ii = 0; ii < size; ii++) {
					maxVal = aly::max(maxVal,
							image(i * size + ii, j * size + jj));
				}
			}
			out(i, j) = maxVal;
		}
	}
}
void DictionaryLearning::normalize(aly::Image3f& image, float percent,
		int stride) {
	std::vector<float> values;
	values.reserve(3 * image.size() / stride);
	for (int i = 0; i < image.width; i += stride) {
		for (int j = 0; j < image.height; j += stride) {
			float3 samp = image(RandomUniform(i, i + stride),
					RandomUniform(j, j + stride));
			values.push_back(samp.x);
			values.push_back(samp.y);
			values.push_back(samp.z);
		}
	}
	std::sort(values.begin(), values.end());
	float lower = values[clamp((int) (percent * values.size()), 0,
			(int) values.size() - 1)];
	float upper = values[clamp((int) ((1.0f - percent) * values.size()), 0,
			(int) values.size() - 1)];
	std::cout << "Soft Limits [" << lower << "," << upper << "]" << std::endl;
	if (upper - lower < 1E-6f) {
		for (float3& v : image.data) {
			v = aly::clamp((v - lower), 0.0f, 1.0f);
		}
	} else {
		for (float3& v : image.data) {
			v = aly::clamp(
					(1.0f - 2 * percent) * (v - lower) / (upper - lower)
							+ percent, 0.0f, 1.0f);
		}
	}
}
float2 DictionaryLearning::normalize(int index, float percent) {
	std::vector<float> values;
	values.reserve(patches.size());
	for (SamplePatch& p : patches) {
		values.push_back(filterBanks[index].score(p));
	}
	std::sort(values.begin(), values.end());
	float lower = values[clamp((int) (percent * values.size()), 0,
			(int) values.size() - 1)];
	float upper = values[clamp((int) ((1.0f - percent) * values.size()), 0,
			(int) values.size() - 1)];

	std::cout << "Soft Limits [" << lower << "," << upper << "]" << std::endl;
	if(upper-lower<1E-6){
		return float2(1.0f,-lower);
	}
	return float2((1.0f - 2 * percent) / (upper - lower),-(1.0f - 2 * percent) * lower + percent);
}
void FilterBank::score(const aly::Image1ub& image, aly::Image1f& out) {
	out.resize(image.width, image.height);
#pragma omp parallel for
	for (int j = 0; j < image.height; j++) {
		for (int i = 0; i < image.width; i++) {
			double avg = 0;
			for (int jj = 0; jj < height; jj++) {
				for (int ii = 0; ii < width; ii++) {
					int x = i + ii - width / 2;
					int y = j + jj - height / 2;
					avg += image(x, y).x / 255.0f;
				}
			}
			avg /= (width * height);
			double err = 0;
			for (int jj = 0; jj < height; jj++) {
				for (int ii = 0; ii < width; ii++) {
					int x = i + ii - width / 2;
					int y = j + jj - height / 2;
					err += data[ii + jj * width]
							* (image(x, y).x / 255.0f - avg);
				}
			}
			out(i, j) = std::abs(err);
		}
	}
}
float FilterBank::score(const SamplePatch& patch) const {
	double err = 0;
	double avg;
	for (int ii = 0; ii < patch.size(); ii++) {
		avg += (double) (patch.data[ii]);
	}
	avg /= patch.size();
	for (int ii = 0; ii < patch.size(); ii++) {
		err += (double) (data[ii]) * (patch.data[ii] - avg);
	}
	err/=patch.size();
	return std::abs(err);
}
void FilterBank::score(const aly::Image1ub& image, aly::DenseMat<float>& out) {
	out.resize(image.width, image.height);
#pragma omp parallel for
	for (int j = 0; j < image.height; j++) {
		for (int i = 0; i < image.width; i++) {
			double avg = 0;
			for (int jj = 0; jj < height; jj++) {
				for (int ii = 0; ii < width; ii++) {
					int x = i + ii - width / 2;
					int y = j + jj - height / 2;
					avg += image(x, y).x / 255.0f;
				}
			}
			avg /= (width * height);
			double err = 0;
			for (int jj = 0; jj < height; jj++) {
				for (int ii = 0; ii < width; ii++) {
					int x = i + ii - width / 2;
					int y = j + jj - height / 2;
					err += data[ii + jj * width]
							* (image(x, y).x / 255.0f - avg);
				}
			}
			err/=(width*height);
			out(i, j) = std::abs(err);
		}
	}
}

void FilterBank::score(const DenseMat<float>& image, aly::DenseMat<float>& out) {
	out.resize(image.rows,image.cols);
#pragma omp parallel for
	for (int j = 0; j < image.cols; j++) {
		for (int i = 0; i < image.rows; i++) {
			double avg = 0;
			for (int jj = 0; jj < height; jj++) {
				for (int ii = 0; ii < width; ii++) {
					int x = i + ii - width / 2;
					int y = j + jj - height / 2;
					avg += image(clamp(x,0,image.rows-1), clamp(y,0,image.cols-1));
				}
			}
			avg /= (width * height);
			double err = 0;
			for (int jj = 0; jj < height; jj++) {
				for (int ii = 0; ii < width; ii++) {
					int x = i + ii - width / 2;
					int y = j + jj - height / 2;
					err += data[ii + jj * width]
							* (image(clamp(x,0,image.rows-1), clamp(y,0,image.cols-1)) - avg);
				}
			}
			err/=(width*height);
			out(i, j) = std::abs(err);
		}
	}
}
void FilterBank::score(const aly::ImageRGB& image, aly::DenseMat<float>& out) {
	out.resize(image.width, image.height);
#pragma omp parallel for
	for (int j = 0; j < image.height; j++) {
		for (int i = 0; i < image.width; i++) {
			double avg(0.0);
			for (int jj = 0; jj < height; jj++) {
				for (int ii = 0; ii < width; ii++) {
					int x = i + ii - width / 2;
					int y = j + jj - height / 2;
					float3 c = ToRGBf(image(x, y));
					avg += RGBtoGray(c, true);
				}
			}
			avg /= double(width * height);
			double3 err(0.0);
			for (int jj = 0; jj < height; jj++) {
				for (int ii = 0; ii < width; ii++) {
					int x = i + ii - width / 2;
					int y = j + jj - height / 2;
					float3 c = ToRGBf(image(x, y));
					err += (double) (data[ii + jj * width])
							* (RGBtoGray(image(x, y), true) - avg);
				}
			}
			err/=(double)(width*height);
			out(i, j) = aly::sum(aly::abs(float3(err.x, err.y, err.z)));
		}
	}
}
void FilterBank::score(const aly::ImageRGB& image, aly::Image3f& out) {
	out.resize(image.width, image.height);
#pragma omp parallel for
	for (int j = 0; j < image.height; j++) {
		for (int i = 0; i < image.width; i++) {
			double3 avg(0.0);
			for (int jj = 0; jj < height; jj++) {
				for (int ii = 0; ii < width; ii++) {
					int x = i + ii - width / 2;
					int y = j + jj - height / 2;
					float3 c = ToRGBf(image(x, y));
					avg += double3(c.x, c.y, c.z);
				}
			}
			avg /= double(width * height);
			double3 err(0.0);
			for (int jj = 0; jj < height; jj++) {
				for (int ii = 0; ii < width; ii++) {
					int x = i + ii - width / 2;
					int y = j + jj - height / 2;
					float3 c = ToRGBf(image(x, y));
					err += (double) (data[ii + jj * width])
							* (double3(c.x, c.y, c.z) - avg);
				}
			}
			out(i, j) = aly::abs(float3(err.x, err.y, err.z));
		}
	}
}
void FilterBank::normalize() {
	double sum = 0.0;
	const int N = (int) data.size();
	for (int i = 0; i < N; i++) {
		sum += data[i] * data[i];
	}
	sum = std::sqrt(std::abs(sum)) / N;
	if (sum > 1E-8) {
		for (int i = 0; i < N; i++) {
			data[i] /= sum;
		}
	}
}
DictionaryLearning::DictionaryLearning() {

}
std::set<int> DictionaryLearning::optimizeFilters(int start) {
	int S = patches.size();
	int K = filterBanks.size();
	int N = filterBanks.front().data.size();
	std::set<int> nonZeroSet;
	for (int s = 0; s < S; s++) {
		SamplePatch& patch = patches[s];
		for (int k = start; k < K; k++) {
			float w = patch.weights[k];
			if (w != 0) {
				nonZeroSet.insert(k);
			}
		}
	}
	std::vector<int> nonZeroIndexes(nonZeroSet.begin(), nonZeroSet.end());
	std::sort(nonZeroIndexes.begin(), nonZeroIndexes.end());
	int KK = nonZeroIndexes.size();
	DenseMat<float> A(S, KK);
	Vec<float> AtB(KK);
	Vec<float> x(KK);
	for (int s = 0; s < S; s++) {
		SamplePatch& patch = patches[s];
		for (int kk = 0; kk < KK; kk++) {
			int k = nonZeroIndexes[kk];
			float w = patch.weights[k];
			A[s][kk] = w;
		}
	}
	DenseMat<float> At = A.transpose();
	DenseMat<float> AtA = At * A;
	double totalRes = 0.0f;
	for (int n = 0; n < N; n++) {
		AtB.setZero();
		for (int kk = 0; kk < KK; kk++) {
			int k = nonZeroIndexes[kk];
			FilterBank& bank = filterBanks[k];
			x[kk] = bank.data[n];
			for (int s = 0; s < S; s++) {
				SamplePatch& patch = patches[s];
				float val = patch.data[n];
				for (int ll = 0; ll < start; ll++) {
					val -= filterBanks[ll].data[n] * patch.weights[ll];
				}
				AtB[kk] += val * At[kk][s];
			}
		}
		x = Solve(AtA, AtB, MatrixFactorization::SVD);
		for (int kk = 0; kk < KK; kk++) {
			int k = nonZeroIndexes[kk];
			FilterBank& bank = filterBanks[k];
			bank.data[n] = x[kk];
		}
	}

	for (int kk = 0; kk < KK; kk++) {
		int k = nonZeroIndexes[kk];
		filterBanks[k].normalize();
	}

	for (int i = 0; i < start; i++) {
		nonZeroSet.insert(i);
	}
	return nonZeroSet;
}
void DictionaryLearning::solveOrthoMatchingPursuit(int T, SamplePatch& patch) {
	std::vector<float>& sample = patch.data;
	std::vector<float>& weights = patch.weights;
	weights.resize(filterBanks.size(), 0.0f);
	size_t B = filterBanks.size();
	size_t N = sample.size();
	DenseMat<float> A;
	DenseMat<float> At;
	Vec<float> b(N);
	Vec<float> x(N);
	Vec<float> r(N);
	double score;
	double bestScore = 1E30;
	int bestBank = -1;
	for (int n = 0; n < N; n++) {
		b.data[n] = sample[n];
	}
	std::vector<bool> mask(B, false);
	for (int bk = 0; bk < B; bk++) {
		FilterBank& bank = filterBanks[bk];
		score = bank.score(sample, true);
		if (score < bestScore) {
			bestScore = score;
			bestBank = bk;
		}
	}
	if (bestBank < 0) {
		return;
	}
	mask[bestBank] = true;
	int nonzero = 1;
	int count = 0;
	for (int t = 0; t < T; t++) {
		A.resize(N, nonzero);
		count = 0;
		for (int bk = 0; bk < B; bk++) {
			if (mask[bk]) {
				FilterBank& bank = filterBanks[bk];
				for (int j = 0; j < N; j++) {
					A[j][count] = bank.data[j];
				}
				count++;
			}
		}
		x = Solve(A, b, MatrixFactorization::SVD);
		r = A * x - b;
		bestScore = 1E30;
		count = 0;
		for (int bk = 0; bk < B; bk++) {
			if (mask[bk]) {
				weights[bk] = x[count++];
			} else {
				weights[bk] = 0.0f;
			}
		}
		bestBank = -1;
		if (t < T - 1) {
			for (int bk = 0; bk < B; bk++) {
				if (mask[bk]) {
					continue;
				}
				FilterBank& bank = filterBanks[bk];
				score = bank.score(r.data, true);
				if (score < bestScore) {
					bestScore = score;
					bestBank = bk;
				}
			}
			if (bestBank < 0) { // || bestScore < 1E-5f
				break;
			}
			mask[bestBank] = true;
			nonzero++;
		}
		//std::cout<<t<<") Weights "<<weights<<" residual="<<length(r)<<std::endl;
	}
	//std::cout << "Weights " << weights << std::endl;
}

std::vector<int> DictionaryLearning::solveOrthoMatchingPursuit(int sparsity,
		const Image1f& gray, SamplePatch& patch) {
	patch.sample(gray);
	std::vector<float>& sample = patch.data;
	std::vector<float>& weights = patch.weights;
	std::vector<int> order;
	weights.resize(filterBanks.size(), 0.0f);
	size_t B = filterBanks.size();
	size_t N = sample.size();
	DenseMat<float> A;
	Vec<float> b(N);
	Vec<float> x(N);
	Vec<float> r(N);
	double score;
	double bestScore = 1E30;
	int bestBank = -1;
	for (int n = 0; n < N; n++) {
		b.data[n] = sample[n];
	}
	std::vector<bool> mask(B, false);
	for (int bk = 0; bk < B; bk++) {
		FilterBank& bank = filterBanks[bk];
		score = bank.score(sample, true);
		if (score < bestScore) {
			bestScore = score;
			bestBank = bk;
		}
	}
	if (bestBank < 0) {
		return order;
	}
	order.push_back(bestBank);
	mask[bestBank] = true;
	int nonzero = 1;
	int count = 0;
	for (int t = 0; t < sparsity; t++) {
		A.resize(N, nonzero);
		count = 0;
		for (int bk = 0; bk < B; bk++) {
			if (mask[bk]) {
				FilterBank& bank = filterBanks[bk];
				for (int j = 0; j < N; j++) {
					A[j][count] = bank.data[j];
				}
				count++;
			}
		}
		x = Solve(A, b, MatrixFactorization::SVD);
		r = A * x - b;
		bestScore = 1E30;
		count = 0;

		for (int bk = 0; bk < B; bk++) {
			if (mask[bk]) {
				weights[bk] = x[count++];
			} else {
				weights[bk] = 0.0f;
			}
		}
		bestBank = -1;
		if (t < sparsity - 1) {
			for (int bk = 0; bk < B; bk++) {
				if (mask[bk]) {
					continue;
				}
				FilterBank& bank = filterBanks[bk];
				score = bank.score(r.data, true);
				if (score < bestScore) {
					bestScore = score;
					bestBank = bk;
				}
			}
			if (bestBank < 0) { // || bestScore < 1E-5f
				break;
			}
			order.push_back(bestBank);
			mask[bestBank] = true;
			nonzero++;
		}
	}
	return order;
}
void DictionaryLearning::optimizeDictionary(float errorThreshold,
		int maxIterations, int startIndex) {
	std::vector<size_t> activeSet;
	DenseMat<float> E, U, D, Vt;
	int M = patches[0].data.size();
	int W = filterBanks.size();
	double lastError = residual();
	std::vector<int> order(W - startIndex);
	for (int k = startIndex; k < W; k++) {
		order[k - startIndex] = k;
	}

	std::cout << "Initial Error " << lastError << " order " << order.size()
			<< std::endl;
	for (int iter = 0; iter < maxIterations; iter++) {
		Shuffle(order);
		for (int kidx = 0; kidx < order.size(); kidx++) {
			int k = order[kidx];
			activeSet.clear();
			//Find set of atoms with non-zero entries
			for (int idx = 0; idx < (int) patches.size(); idx++) {
				SamplePatch& p = patches[idx];
				if (p.weights[k] != 0) {
					activeSet.push_back(idx);
				}
			}
			if (activeSet.size() == 0)
				break;
			size_t A = activeSet.size();
			E.resize(A, M);
#pragma omp parallel for
			for (size_t idx = 0; idx < activeSet.size(); idx++) {
				size_t i = activeSet[idx];
				SamplePatch& p = patches[i];
				std::vector<float>& data = p.data;
				std::vector<float>& weights = p.weights;
				for (int m = 0; m < M; m++) {
					double sum = data[m];
					for (int j = 0; j < W; j++) {
						if (j != k) { //Ignore pivot filter bank
							sum -= weights[j] * filterBanks[j][m];
						}
					}
					E(idx, m) = (float) sum;
				}
			}
			//SVD on EtE
			SVD(E.square(), U, D, Vt);
			VecMap<float> u = U.getColumn(0);
#pragma omp parallel for
			for (size_t m = 0; m < M; m++) {
				filterBanks[k][m] = u[m];
			}
			double largestLambda = std::sqrt((double) D(0, 0));
			//Compute D inverse
#pragma omp parallel for
			for (int i = 0; i < D.rows; i++) {
				double lam = std::sqrt((double) D(i, i));
				D(i, i) = (float) (largestLambda / std::max(1E-10, lam));
			}
			E = E * U * D; //compute real U
#pragma omp parallel for
			for (size_t idx = 0; idx < A; idx++) {
				size_t i = activeSet[idx];
				SamplePatch& p = patches[i];
				p.weights[k] = E(idx, 0);
			}
		}
		if (errorThreshold > 0) {
			double er = residual();
			if (std::abs(er - lastError) < errorThreshold)
				break;
			lastError = er;

			std::cout << iter << ") Error " << er << std::endl;
		}
	}
}
void DictionaryLearning::optimizeWeights(int t) {
#pragma omp parallel for
	for (int idx = 0; idx < (int) patches.size(); idx++) {
		SamplePatch& p = patches[idx];
		solveOrthoMatchingPursuit(t, p);
	}
}
double DictionaryLearning::residual() {
	double err = 0;
	for (int n = 0; n < (int) patches.size(); n++) {
		double r = patches[n].residual(filterBanks, false);
		err += r;
	}
	err /= patches.size();
	return err;
}
void DictionaryLearning::addFilters(int filters) {
	std::vector<std::pair<float, size_t>> scores(patches.size());
#pragma omp parallel for
	for (int n = 0; n < patches.size(); n++) {
		scores[n]= {patches[n].residual(filterBanks, false), n};
	}
	std::sort(scores.begin(), scores.end());
	for (int n = scores.size() - 1;
			n >= std::max(0, (int) scores.size() - filters); n--) {
		FilterBank b;
		SamplePatch& p = patches[scores[n].second];
		b.data = p.data;
		b.angle = 0;
		b.width = p.width;
		b.height = p.height;
		b.type = FilterType::Texture;
		b.normalize();
		filterBanks.push_back(b);
		for (int nn = 0; nn < patches.size(); nn++) {
			patches[nn].weights.push_back(0.0f);
		}
	}
}
void DictionaryLearning::add(const std::vector<FilterBank>& banks) {
	for (int b = 0; b < banks.size(); b++) {
		filterBanks.push_back(banks[b]);
		for (int n = 0; n < patches.size(); n++) {
			patches[n].weights.push_back(0.0f);
		}
	}
}
void DictionaryLearning::add(const FilterBank& banks) {
	filterBanks.push_back(banks);
	for (int n = 0; n < patches.size(); n++) {
		patches[n].weights.push_back(0.0f);
	}
}
void DictionaryLearning::removeFilterBanks(const std::set<int>& nonZeroSet) {
	int K = filterBanks.size();
	if (K == nonZeroSet.size())
		return;
	int count = 0;
	std::vector<FilterBank> newFilterBanks;
	for (int k = 0; k < K; k++) {
		if (nonZeroSet.find(k) != nonZeroSet.end()) {
			newFilterBanks.push_back(filterBanks[k]);
		} else {
			std::cout << "Removed Filter Bank " << k
					<< " because it's not used. " << std::endl;
		}
	}
	filterBanks = newFilterBanks;
	for (int n = 0; n < patches.size(); n++) {
		std::vector<float> newWeights;
		SamplePatch& patch = patches[n];
		for (int k = 0; k < K; k++) {
			if (nonZeroSet.find(k) != nonZeroSet.end()) {
				newWeights.push_back(patch.weights[k]);
			}
		}
		patch.weights = newWeights;
	}
}
void WriteFilterBanksToFile(const std::string& file,
		std::vector<FilterBank>& params) {
	std::string ext = GetFileExtension(file);
	if (ext == "json") {
		std::ofstream os(file);
		cereal::JSONOutputArchive archive(os);
		archive(cereal::make_nvp("filter_banks", params));
	} else if (ext == "xml") {
		std::ofstream os(file);
		cereal::XMLOutputArchive archive(os);
		archive(cereal::make_nvp("filter_banks", params));
	} else {
		std::ofstream os(file, std::ios::binary);
		cereal::PortableBinaryOutputArchive archive(os);
		archive(cereal::make_nvp("filter_banks", params));
	}
}
void ReadFilterBanksFromFile(const std::string& file,
		std::vector<FilterBank>& params) {
	std::string ext = GetFileExtension(file);
	if (ext == "json") {
		std::ifstream os(file);
		cereal::JSONInputArchive archive(os);
		archive(cereal::make_nvp("filter_banks", params));
	} else if (ext == "xml") {
		std::ifstream os(file);
		cereal::XMLInputArchive archive(os);
		archive(cereal::make_nvp("filter_banks", params));
	} else {
		std::ifstream os(file, std::ios::binary);
		cereal::PortableBinaryInputArchive archive(os);
		archive(cereal::make_nvp("filter_banks", params));
	}
}

void DictionaryLearning::write(const std::string& outFile) {
	WriteFilterBanksToFile(outFile, filterBanks);
}
void DictionaryLearning::read(const std::string& outFile) {
	ReadFilterBanksFromFile(outFile, filterBanks);
}
void WritePatchesToFile(const std::string& file,
		const std::vector<SamplePatch>& patches) {
	std::vector<ImageRGB> tiles(patches.size());
	int M = std::ceil(std::sqrt(patches.size()));
	int N = (patches.size() % M == 0) ?
			(patches.size() / M) : (1 + patches.size() / M);
	for (size_t idx = 0; idx < patches.size(); idx++) {
		const SamplePatch& p = patches[idx];
		ImageRGB& tile = tiles[idx];
		tile.resize(p.width, p.height);
		int count = 0;
		for (int j = 0; j < p.height; j++) {
			for (int i = 0; i < p.width; i++) {
				tile[count] = ColorMapToRGB(p.data[count], ColorMap::RedToBlue);
				count++;
			}
		}
	}
	ImageRGB out;
	Tile(tiles, out, N, M);
	WriteImageToFile(file, out);
}
void DictionaryLearning::estimate(const aly::ImageRGB& img, aly::ImageRGB& out,
		aly::Volume3f& labels, int sparsity) {
	int width = filterBanks[0].width;
	int height = filterBanks[0].height;
	out.resize(width * (img.width / width), height * (img.height / height));
	out.set(aly::RGB(0, 0, 0));
	labels.resize((img.width / width), (img.height / height), 5);
	size_t index = 0;
#pragma omp parallel for
	for (int j = 0; j < out.height; j += height) {
		for (int i = 0; i < out.width; i += width) {
			std::vector<float> tmp;
			float2 center = float2(i + width * 0.5f, j + height * 0.5f);
			SamplePatch p(center, width, height);
			for (int c = 0; c < 3; c++) {
				p.sample(img, c);
				solveOrthoMatchingPursuit(sparsity, p);
				p.synthesize(filterBanks, tmp);
				std::vector<float> w = p.getLabelWeights(filterBanks);
				for (int idx = 0; idx < w.size(); idx++) {
					labels(i / width, j / height, idx)[c] = w[idx];
				}
				int count = 0;
				for (int jj = 0; jj < height; jj++) {
					for (int ii = 0; ii < width; ii++) {
						float val = tmp[count++];
						out(ii + i, jj + j)[c] = clamp((int) (val * 255.0f), 0,
								255);
					}
				}
			}
		}
	}
}
double DictionaryLearning::score(std::vector<std::pair<int, double>>& scores) {
	scores.resize(patches.size());
	for (int n = 0; n < patches.size(); n++) {
		scores[n]= {n,patches[n].residual(filterBanks,false)};
	}
	std::sort(scores.begin(), scores.end(),
			[=](const std::pair<int,double>& a,const std::pair<int,double>& b) {
				return (a.second>b.second);
			});
	return scores.front().second;
}
void DictionaryLearning::writeFilterBanks(const std::string& outImage) {
	int patchWidth = filterBanks.front().width;
	int patchHeight = filterBanks.front().height;
	std::cout << "Filter Banks " << filterBanks.size() << std::endl;
	ImageRGB filters(patchWidth * filterBanks.size(), patchHeight);
	for (int n = 0; n < filterBanks.size(); n++) {
		FilterBank& b = filterBanks[n];
		float minV = 1E30f;
		float maxV = -1E30f;
		size_t K = b.data.size();
		for (int k = 0; k < K; k++) {
			minV = std::min(minV, b.data[k]);
			maxV = std::max(maxV, b.data[k]);
		}
		float diff = (maxV - minV > 1E-4f) ? 1.0f / (maxV - minV) : 1.0f;
		for (int k = 0; k < K; k++) {
			int i = k % b.width;
			int j = k / b.width;
			filters(i + n * b.width, j) = ColorMapToRGB(
					(filterBanks[n].data[k] - minV) * diff,
					ColorMap::RedToBlue);
		}
	}
	WriteImageToFile(outImage, filters);
}
void DictionaryLearning::stash(const ImageRGB& img, int subsample) {
	aly::Image1f gray;
	ConvertImage(img, gray, true);
	int N = img.height / subsample;
	int M = img.width / subsample;
	ImageRGBA edges(M, N);
	Image2f orientation(M, N);
	int patchWidth = filterBanks.front().width;
	int patchHeight = filterBanks.front().height;
#pragma omp parallel for
	for (int n = 0; n < N; n++) {
		for (int m = 0; m < M; m++) {
			float2 center = float2(m * subsample + subsample * 0.5f,
					n * subsample + subsample * 0.5f);
			SamplePatch patch(center, patchWidth, patchHeight);
			std::vector<int> order = solveOrthoMatchingPursuit(4, gray, patch);
			if (order.size() > 3) {
				edges(m, n) = RGBA(order[0], order[1], order[2], order[3]);
				for (int k = 0; k < order.size(); k++) {
					FilterBank& bank = filterBanks[order[k]];
					if (bank.type == FilterType::Edge
							|| bank.type == FilterType::Line) {
						float w = aly::sign(patch.weights[order[k]]);
						orientation(m, n) = float2(bank.angle * w,
								bank.shift * w);
					}
				}
			}
		}
	}
	WriteImageToRawFile(MakeDesktopFile("edges.xml"), edges);
	WriteImageToRawFile(MakeDesktopFile("orients.xml"), orientation);

}
void DictionaryLearning::initializeFilters(int patchWidth, int patchHeight,
		int angleSamples) {
	filterBanks.clear();
	size_t K = patchWidth * patchHeight;
	float fuzz = 0.5f;
	float var = fuzz * fuzz;
	std::vector<float> shifts = { -fuzz, -fuzz * 0.5f, 0.0f, fuzz * 0.5f, fuzz };
	std::vector<float> angles(angleSamples);
	float angleDelta = 180.0f / (float) (angleSamples);
	for (int a = 0; a < angleSamples; a++) {
		angles[a] = angleDelta * a;
	}
	int initFilterBanks = 3 + angles.size() * shifts.size();
	filterBanks.reserve(initFilterBanks);
	for (int n = 0; n < initFilterBanks; n++) {
		filterBanks.push_back(FilterBank(patchWidth, patchHeight));
	}
	for (int k = 0; k < K; k++) {
		int i = k % patchWidth;
		int j = k / patchWidth;
		float t = 2 * (j - (patchHeight - 1) * 0.5f) / (patchHeight - 1);
		float s = 2 * (i - (patchWidth - 1) * 0.5f) / (patchWidth - 1);
		int idx = 0;
		filterBanks[idx++].data[k] = 1.0f;
		filterBanks[idx++].data[k] = s;
		filterBanks[idx++].data[k] = t;
		for (int a = 0; a < angles.size(); a++) {
			for (int sh = 0; sh < shifts.size(); sh++) {
				float r = std::cos(ToRadians(angles[a])) * s
						+ std::sin(ToRadians(angles[a])) * t + shifts[sh];
				filterBanks[idx++].data[k] = 0.5f
						- 1.0f / (1 + std::exp(r / var));
			}
		}
	}
	filterBanks[0].type = FilterType::Flat;
	filterBanks[0].angle = -1;

	filterBanks[1].type = FilterType::Shading;
	filterBanks[1].angle = 0;

	filterBanks[2].type = FilterType::Shading;
	filterBanks[2].angle = 90;
	int idx = 3;
	for (int a = 0; a < angles.size(); a++) {
		for (int sh = 0; sh < shifts.size(); sh++) {
			filterBanks[idx].angle = angles[a];
			filterBanks[idx].shift = shifts[sh];
			filterBanks[idx].type = FilterType::Edge;
			idx++;
		}
	}
	int KK = filterBanks.size() * 4;
	std::set<int> nonZeroSet;
	const int optimizationIterations = 4;
	int startFilter = filterBanks.size();
	for (int n = 0; n < filterBanks.size(); n++) {
		filterBanks[n].normalize();
	}
	std::cout << "Filters: " << filterBanks.size() <<" Filter Size: "<<patchWidth<<" x "<<patchHeight<< std::endl;
}
void DictionaryLearning::setTrainingData(const std::vector<Image1ub>& images,
		int subsample) {
	int patchWidth = filterBanks[0].width;
	int patchHeight = filterBanks[0].height;
	patches.clear();
	Image1f gray;
	for (int nn = 0; nn < images.size(); nn++) {
		const Image1ub& img = images[nn];
		ConvertImage(img, gray);
		int N = img.height / subsample;
		int M = img.width / subsample;

		for (int n = 0; n < N; n++) {
			for (int m = 0; m < M; m++) {
				float2 center = float2(m * subsample + subsample * 0.5f,
						n * subsample + subsample * 0.5f);
				SamplePatch p(center, patchWidth, patchHeight);
				p.sample(gray);
				p.weights.resize(filterBanks.size(), 0.0f);
				patches.push_back(p);

			}
		}
	}
	std::cout << "Patches " << patches.size() << std::endl;
}
void DictionaryLearning::addTrainingData(const DenseMat<float>& img,
		int subsample) {
	int patchWidth = filterBanks[0].width;
	int patchHeight = filterBanks[0].height;
	int N = img.cols / subsample;
	int M = img.rows / subsample;
	for (int n = 0; n < N; n++) {
		for (int m = 0; m < M; m++) {
			float2 center = float2(m * subsample + subsample * 0.5f,
					n * subsample + subsample * 0.5f);
			SamplePatch p(center, patchWidth, patchHeight);
			p.sample(img);
			p.weights.resize(filterBanks.size(), 0.0f);
			patches.push_back(p);

		}
	}
}
void DictionaryLearning::setTrainingData(const std::vector<DenseMat<float>>& images,
		int subsample) {
	patches.clear();
	for (const DenseMat<float>& img : images) {
		addTrainingData(img);
	}
}
void DictionaryLearning::setTrainingData(const std::vector<ImageRGB>& images,
		int subsample) {
	int patchWidth = filterBanks[0].width;
	int patchHeight = filterBanks[0].height;
	aly::Image1f gray;
	patches.clear();
	for (int nn = 0; nn < images.size(); nn++) {
		const ImageRGB& img = images[nn];
		ConvertImage(img, gray, true);
		int N = img.height / subsample;
		int M = img.width / subsample;
		for (int n = 0; n < N; n++) {
			for (int m = 0; m < M; m++) {
				float2 center = float2(m * subsample + subsample * 0.5f,
						n * subsample + subsample * 0.5f);
				SamplePatch p(center, patchWidth, patchHeight);
				p.sample(gray);
				patches.push_back(p);
			}
		}
	}
	std::cout << "Patches " << patches.size() << std::endl;
}
}

