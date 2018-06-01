/*
 * FilterBankLearning.cpp
 *
 *  Created on: Aug 27, 2017
 *      Author: blake
 */

#include "AlloyImageProcessing.h"
#include "AlloyAnisotropicFilter.h"
#include <AlloyOptimization.h>
#include <AlloyUnits.h>
#include <AlloyVolume.h>
#include <set>
#include <cereal/archives/xml.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>
#include "ml/DictionaryLearning.h"
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
float SamplePatch::error(const std::vector<FilterBank>& banks,
		bool correlation) {
	int N = (int) data.size();
	int M = (int) weights.size();
	if(weights.size()!=banks.size()){
		throw std::runtime_error(MakeString()<<"Weight vector doesn't match filter banks "<<weights.size()<<" "<<banks.size());
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
}

std::vector<int> DictionaryLearning::solveOrthoMatchingPursuit( int sparsity, const Image1f& gray,SamplePatch& patch) {
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
void DictionaryLearning::optimizeWeights(int t) {
#pragma omp parallel for
	for (int idx = 0; idx < (int) patches.size(); idx++) {
		SamplePatch& p = patches[idx];
		solveOrthoMatchingPursuit(t, p);
	}
}
double DictionaryLearning::error() {
	double err = 0;
	for (SamplePatch& p : patches) {
		err += p.error(filterBanks, false);
	}
	err /= patches.size();
	return err;
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
void DictionaryLearning::writeEstimatedPatches(const std::string& outImage,
		int M, int N) {
	//int M=std::ceil(std::sqrt(patches.size()));
	//int N=(patches.size()%M==0)?(patches.size()/M):(1+patches.size()/M);
	std::vector<ImageRGB> tiles(patches.size());
	std::vector<double> scores(patches.size());
	double maxError = 0;
	for (int n = 0; n < patches.size(); n++) {
		scores[n] = patches[n].error(filterBanks, false);
		if (scores[n] > maxError) {
			maxError = scores[n];
		}
	}

	for (size_t idx = 0; idx < patches.size(); idx++) {
		SamplePatch& p = patches[idx];
		std::vector<float> tmp;
		p.synthesize(filterBanks, tmp);
		ImageRGB& tile = tiles[idx];
		tile.resize(p.width, p.height);
		int count = 0;
		float cw = scores[idx] / maxError;
		for (int j = 0; j < tile.height; j++) {
			for (int i = 0; i < tile.width; i++) {
				float val = tmp[count];
				tile[count] = ToRGB(float3(val));
				/*
				 if (val < 0) {
				 tile[count] = aly::RGB(0, 0, 0);
				 } else {

				 float err = std::abs(tmp[count] - p.data[count]);
				 tile[count] = ToRGB(
				 HSVtoRGBf(HSV(1.0f - 0.9f * cw, cw, val)));
				 }
				 */
				count++;
			}
		}
	}
	std::cout << "Tiles " << tiles.size() << std::endl;
	ImageRGB out;
	Tile(tiles, out, N, M);
	WriteImageToFile(outImage, out);
}
double DictionaryLearning::score(std::vector<std::pair<int, double>>& scores) {
	scores.resize(patches.size());
	for (int n = 0; n < patches.size(); n++) {
		scores[n]= {n,patches[n].error(filterBanks,false)};
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
void DictionaryLearning::stash(const ImageRGB& img,int subsample){
	aly::Image1f gray;
	ConvertImage(img, gray);
	int N = img.height / subsample;
	int M = img.width / subsample;
	ImageRGBA edges(M,N);
	Image2f orientation(M,N);
	int patchWidth=filterBanks.front().width;
	int patchHeight=filterBanks.front().height;
#pragma omp parallel for
	for (int n = 0; n < N; n++) {
		for (int m = 0; m < M; m++) {
			float2 center = float2(m * subsample + subsample * 0.5f,n * subsample + subsample * 0.5f);
			SamplePatch patch(center,patchWidth,patchHeight);
			std::vector<int> order=solveOrthoMatchingPursuit(4,gray,patch);
			if(order.size()>3){
				edges(m,n)=RGBA(order[0],order[1],order[2],order[3]);
				for(int k=0;k<order.size();k++){
					FilterBank& bank=filterBanks[order[k]];
					if(bank.type==FilterType::Edge||bank.type==FilterType::Line){
						float w=aly::sign(patch.weights[order[k]]);
						orientation(m,n)=float2(bank.angle*w,bank.shift*w);
					}
				}
			}
		}
	}
	WriteImageToRawFile(MakeDesktopFile("edges.xml"),edges);
	WriteImageToRawFile(MakeDesktopFile("orients.xml"),orientation);

}
void DictionaryLearning::train(const std::vector<ImageRGB>& images,
		int targetFilterBankSize, int subsample, int patchWidth,
		int patchHeight, int sparsity) {
	aly::Image1f gray;
	patches.clear();
	std::cout << "Start Learning " << patches.size() << "..." << std::endl;
	for (int nn = 0; nn < images.size(); nn++) {
		const ImageRGB& img = images[nn];
		ConvertImage(img, gray);
		int N = img.height / subsample;
		int M = img.width / subsample;
		for (int n = 0; n < N; n++) {
			for (int m = 0; m < M; m++) {
				float2 center = float2(m * subsample + subsample * 0.5f,n * subsample + subsample * 0.5f);
				SamplePatch p(center, patchWidth, patchHeight);
				p.sample(gray);
				patches.push_back(p);
			}
		}
	}
	std::cout << "Patches " << patches.size() << " " << patchWidth << " "<< patchHeight << std::endl;
	WritePatchesToFile(MakeDesktopFile("tiles.png"), patches);
	filterBanks.clear();
	size_t K = patchWidth * patchHeight;
	float fuzz = 0.5f;
	float var = fuzz * fuzz;
	//float var2 = 0.4f * 0.4f;
	//float var3 = 0.5f * 0.5f;
	std::vector<float> shifts = { -fuzz, -fuzz * 0.5f, 0.0f, fuzz * 0.5,fuzz };
	std::vector<float> angles;
	for (int a = -180; a < 180; a += 20) {
		angles.push_back(a);
	}

	int initFilterBanks = 3 +  angles.size() * shifts.size();
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
				filterBanks[idx++].data[k] = 0.5f- 1.0f / (1 + std::exp(r / var));
				//filterBanks[idx++].data[k] = std::exp(-0.5f * r * r / var);
			}
		}
	}
	filterBanks[0].type=FilterType::Shading;
	filterBanks[0].angle=-1;

	filterBanks[1].type=FilterType::Shading;
	filterBanks[1].angle=0;;

	filterBanks[2].type=FilterType::Shading;
	filterBanks[2].angle=90;
	int idx=3;
	for (int a = 0; a < angles.size(); a++) {
		for (int sh = 0; sh < shifts.size(); sh++) {

			filterBanks[idx].angle=angles[a];
			filterBanks[idx].shift=shifts[sh];
			filterBanks[idx].type=FilterType::Edge;
			idx++;

			/*
			filterBanks[idx].angle=angles[a];
			filterBanks[idx].shift=shifts[sh];
			filterBanks[idx].type=FilterType::Line;
			idx++;
			*/
		}
	}
	int KK = filterBanks.size() * 4;
//Orthogonal basis pursuit
	std::set<int> nonZeroSet;
	const int optimizationIterations = 4;
	int startFilter = filterBanks.size();
	for (int n = 0; n < filterBanks.size(); n++) {
		filterBanks[n].normalize();
	}
	optimizeWeights(sparsity);
	error();
	for (int l = 0; filterBanks.size() < targetFilterBankSize; l++) {
		float err1 = error();
		std::cout << filterBanks.size() << " Before Optimize Error: " << err1
				<< " for sparsity " << sparsity << std::endl;
		for (int h = 0; h < optimizationIterations; h++) {
			optimizeWeights(sparsity);
			nonZeroSet = optimizeFilters(startFilter);
		}
		float err2 = error();
		std::cout << filterBanks.size() << " After Optimize Error: " << err2
				<< " for sparsity " << sparsity << std::endl;
		/*
		 std::cout<<"non-zero =[";
		 for(int n:nonZeroSet){
		 std::cout<<n<<" ";
		 }

		 std::cout<<"]"<<std::endl;
		 */
		if (std::abs(err2 - err1) < 1E-5f) {
			sparsity++;
			startFilter--;
		}
		if (filterBanks.size() == targetFilterBankSize) {
			break;
		}
		std::vector<std::pair<int, double>> scores;
		score(scores);
		//Add to filter bank patch with most error
		int bestId=scores[0].first;
		float err=scores[0].second;
		FilterBank fb(patches[bestId].data, patchWidth, patchHeight);
		startFilter = std::max((int) filterBanks.size() - 1, initFilterBanks);
		add(fb);
		filterBanks.back().normalize();
		std::cout << "Add Filter " << bestId << " " << err<<" "<<filterBanks.back().score(patches[bestId].data)<<std::endl;
	}
}
}

