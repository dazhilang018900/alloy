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


#ifndef INCLUDE_DICTIONARYLEARNING_H_
#define INCLUDE_DICTIONARYLEARNING_H_

#include "image/AlloyImage.h"
#include <set>
namespace aly {
enum FilterType{
	Unknown=0,
	Shading=1,
	Edge=2,
	Line=3
};
struct FilterBank {
	int width, height;
	float angle;
	float shift;
	FilterType type;
	std::vector<float> data;
	size_t size() const {
		return data.size();
	}
	inline const float& operator[](const size_t& sz) const {
		if(sz>=data.size()){
			throw std::runtime_error("Filter bank index out of bounds.");
		}
		return data[sz];
	}
	inline float& operator[](const size_t& sz) {
		if(sz>=data.size()){
			throw std::runtime_error("Filter bank index out of bounds.");
		}
		return data[sz];
	}
	FilterBank(int width=0, int height=0) :
			data(width * height), width(width), height(height), type(FilterType::Unknown),angle(0),shift(0) {
	}
	FilterBank(const std::vector<float>& data, int width, int height) :
			data(data), width(width), height(height), type(FilterType::Unknown),angle(0),shift(0) {
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
	SamplePatch(float2 center, int patchWidth, int patchHeight) :
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
	std::vector<int> solveOrthoMatchingPursuit(int sparsity, const Image1f& gray,SamplePatch& patch);
	void removeFilterBanks(const std::set<int>& indexes);
	void add(const std::vector<FilterBank>& banks);
	void add(const FilterBank& banks);
	std::set<int> optimizeFilters(int start);
	void optimizeWeights(int t);
	void optimizeDictionary(float errorThreshold=1E-6f,int maxIterations=128);
	double error();
	double score(std::vector<std::pair<int, double>>& scores);
public:
	std::vector<FilterBank> filterBanks;
	std::vector<SamplePatch> patches;
	DictionaryLearning();
	void write(const std::string& outFile);
	void read(const std::string& outFile);
	void stash(const ImageRGB& img,int subsample);
	void writeEstimatedPatches(const std::string& outImage,int M,int N);
	void writeFilterBanks(const std::string& outImage);
	virtual ~DictionaryLearning() {
	}
	void train(const std::vector<ImageRGB>& img, int targetFilterBankSize = 32,
			int subsample = 8, int patchWidth = 16, int patchHeight = 8,int sparsity=4);
};
void WritePatchesToFile(const std::string& file,const std::vector<SamplePatch>& patches);
void WriteFilterBanksToFile(const std::string& file,
		const std::vector<FilterBank>& banks);
void ReadFilterBanksFromFile(const std::string& file,
		std::vector<FilterBank>& banks);

}

#endif /* INCLUDE_DICTIONARYLEARNING_H_ */
