/*
 * FilterBankLearning.h
 *
 *  Created on: Aug 27, 2017
 *      Author: blake
 */

#ifndef INCLUDE_DICTIONARYLEARNING_H_
#define INCLUDE_DICTIONARYLEARNING_H_

#include <AlloyImage.h>
#include <set>
namespace aly {
struct FilterBank {
	int width, height;
	std::vector<float> data;
	FilterBank(int width=0, int height=0) :
			data(width * height), width(width), height(height) {
	}
	FilterBank(const std::vector<float>& data, int width, int height) :
			data(data), width(width), height(height) {
	}
	template<class Archive>
	void save(Archive & archive) const {
		archive(CEREAL_NVP(width), CEREAL_NVP(height), CEREAL_NVP(data));
	}
	template<class Archive>
	void load(Archive & archive) {
		archive(CEREAL_NVP(width), CEREAL_NVP(height), CEREAL_NVP(data));
	}
	float score(const std::vector<float>& sample) {
		int N = (int) std::min(data.size(), sample.size());
		double err = 0;
		for (int i = 0; i < N; i++) {
			err += std::abs(data[i] - sample[i]);
		}
		err /= N;
		return (float) err;
	}
	void normalize() {

	}
};
struct OrientedPatch {
	int width, height;
	float2 position;
	float2 normal;
	std::vector<float> data;
	std::vector<float> weights;
	template<class Archive>
	void save(Archive & archive) const {
		archive(CEREAL_NVP(width), CEREAL_NVP(height), CEREAL_NVP(position),
				CEREAL_NVP(normal), CEREAL_NVP(data), CEREAL_NVP(weights));
	}
	template<class Archive>
	void load(Archive & archive) {
		archive(CEREAL_NVP(width), CEREAL_NVP(height), CEREAL_NVP(position),
				CEREAL_NVP(normal), CEREAL_NVP(data), CEREAL_NVP(weights));
	}
	OrientedPatch(float2 center, float2 normal, int patchWidth, int patchHeight) :
			position(center), normal(normal), width(patchWidth), height(
					patchHeight) {
		data.resize(patchWidth * patchHeight);
	}
	void sample(const aly::Image1f& gray) {
		float2 tanget = MakeOrthogonalComplement(normal);
		for (int j = 0; j < height; j++) {
			for (int i = 0; i < width; i++) {
				float2 pix(width * (i / (float) (width - 1) - 0.5f),
						height * (j / (float) (height - 1) - 0.5f));
				data[i + j * width] = gray(position.x + dot(normal, pix),
						position.y + dot(tanget, pix));
			}
		}
	}
	float error(const std::vector<FilterBank>& banks) {
		int N = (int) data.size();
		int M = (int) weights.size();
		double err = 0;
		for (int i = 0; i < N; i++) {
			float val = 0;
			for (int j = 0; j < M; j++) {
				val += banks[j].data[i] * weights[j];
			}
			err += std::abs(data[i] - val);
		}
		err /= N;
		return (float) err;
	}
	float synthesize(const std::vector<FilterBank>& banks,
			std::vector<float>& sample) {
		int N = (int) data.size();
		int M = (int) weights.size();
		double err = 0;
		for (int i = 0; i < N; i++) {
			float val = 0;
			for (int j = 0; j < M; j++) {
				val += banks[j].data[i] * weights[j];
			}
			err += std::abs(data[i] - val);
		}
		err /= N;
		return (float) err;
	}
	void resize(int w, int h) {
		data.resize(w * h, 0.0f);
		width = w;
		height = h;
	}
};
class DictionaryLearning {
protected:
	int targetSparsity;
	void solveOrthoMatchingPursuit(int m, OrientedPatch& patch);
	void removeFilterBanks(const std::set<int>& indexes);
	void add(const std::vector<FilterBank>& banks);
	void add(const FilterBank& banks);
	std::set<int> optimizeFilters(int start);
	void optimizeWeights(int t);
	double error();
	double score(std::vector<std::pair<int, double>>& scores);
public:

	std::vector<FilterBank> filterBanks;
	std::vector<OrientedPatch> patches;
	DictionaryLearning();
	void write(const std::string& outFile);
	void read(const std::string& outFile);
	void writeEstimatedPatches(const std::string& outImage, int M, int N);
	void writeFilterBanks(const std::string& outImage);
	virtual ~DictionaryLearning() {
	}
	void train(const std::vector<ImageRGBA>& img, int targetFilterBankSize = 32,
			int subsample = 8, int patchWidth = 16, int patchHeight = 8);
};
void WriteFilterBanksToFile(const std::string& file,
		const std::vector<FilterBank>& banks);
void ReadFilterBanksFromFile(const std::string& file,
		std::vector<FilterBank>& banks);

}

#endif /* INCLUDE_DICTIONARYLEARNING_H_ */
