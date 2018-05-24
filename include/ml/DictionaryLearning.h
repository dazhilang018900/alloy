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
	double score(const std::vector<float>& sample,bool correlation=true);
	void normalize();
};
struct SamplePatch {
	int width, height;
	float2 position;
	std::vector<float> data;
	std::vector<float> weights;
	template<class Archive>
	void save(Archive & archive) const {
		archive(CEREAL_NVP(width), CEREAL_NVP(height), CEREAL_NVP(position),CEREAL_NVP(data), CEREAL_NVP(weights));
	}
	template<class Archive>
	void load(Archive & archive) {
		archive(CEREAL_NVP(width), CEREAL_NVP(height), CEREAL_NVP(position),CEREAL_NVP(data), CEREAL_NVP(weights));
	}
	SamplePatch(float2 center, float2 normal, int patchWidth, int patchHeight) :
			position(center),width(patchWidth), height(
					patchHeight) {
		data.resize(patchWidth * patchHeight);
	}
	void sample(const aly::Image1f& gray);
	float error(const std::vector<FilterBank>& banks,bool correlation);
	float synthesize(const std::vector<FilterBank>& banks,std::vector<float>& sample);
	void resize(int w, int h);
};
class DictionaryLearning {
protected:
	void solveOrthoMatchingPursuit(int m, SamplePatch& patch);
	void removeFilterBanks(const std::set<int>& indexes);
	void add(const std::vector<FilterBank>& banks);
	void add(const FilterBank& banks);
	std::set<int> optimizeFilters(int start);
	void optimizeWeights(int t);
	double error();
	double score(std::vector<std::pair<int, double>>& scores);
public:
	std::vector<FilterBank> filterBanks;
	std::vector<SamplePatch> patches;
	std::set<int> excludeList;
	DictionaryLearning();
	void write(const std::string& outFile);
	void read(const std::string& outFile);
	void writeEstimatedPatches(const std::string& outImage,int M,int N);
	void writeFilterBanks(const std::string& outImage);
	virtual ~DictionaryLearning() {
	}
	void train(const std::vector<ImageRGBA>& img, int targetFilterBankSize = 32,
			int subsample = 8, int patchWidth = 16, int patchHeight = 8,int sparsity=4);
};
void WritePatchesToFile(const std::string& file,const std::vector<SamplePatch>& patches);
void WriteFilterBanksToFile(const std::string& file,
		const std::vector<FilterBank>& banks);
void ReadFilterBanksFromFile(const std::string& file,
		std::vector<FilterBank>& banks);

}

#endif /* INCLUDE_DICTIONARYLEARNING_H_ */
