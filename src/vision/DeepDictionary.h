/*
 * DeepDictionary.h
 *
 *  Created on: Dec 1, 2018
 *      Author: blake
 */

#ifndef SRC_VISION_DEEPDICTIONARY_H_
#define SRC_VISION_DEEPDICTIONARY_H_

#include "DictionaryLearning.h"
#include "math/AlloyOptimizationMath.h"
namespace aly {
typedef DenseMat<float> FeatureData;
typedef std::vector<FeatureData> LayerData;
struct FilterBankWeights{
	float weight=1;
	float normalizeScale=1;
	float normalizeOffset=0;
	inline float evaluate(float x) const {
		return weight*((x-normalizeOffset)*normalizeScale);
	}
};
struct FilterBankTensor{
	std::vector<FilterBank> filters;
	std::vector<FilterBankWeights> weights;
	FilterBankTensor(size_t sz=0):filters(sz),weights(sz){}
	size_t size() const {
		return filters.size();
	}
	void evaluate(const DenseMat<float>& in,DenseMat<float>& out,int index);
	void evaluate(const Image1ub& in,DenseMat<float>& out,int index);
	void evaluate(const ImageRGB& in,DenseMat<float>& out,int index);
	void evaluate(const Image1ub& in,Image1f& out,int index);
	void evaluate(const ImageRGB& in,Image3f& out,int index);

};

class FilterLayer{
private:
	std::vector<FilterBankTensor> tensors;
	int inputSize;
	int outputSize;
	int patchSize;
	int sparsity;
	int angleSamples;
	void learnInput(DictionaryLearning& dictionary,int inputIndex);
public:
	FilterLayer(int featuresIn,int featuresOut,int patchSize);
	void setSparsity(int s);
	void setAngleSamples(int s);
	size_t getInputSize() const {
		return inputSize;
	}
	size_t getOutputSize() const {
		return outputSize;
	}
	void learnInput(const std::vector<Image1ub>& images,int inputIndex,int subsample);
	void learnInput(const std::vector<LayerData>& images,int inputIndex,int subsample);
	void learnInput(const std::vector<ImageRGB>& images,int inputIndex,int subsample);
	void learnOutput();//forward pass, optimize filter weights
	void evaluate(const ImageRGB& img,LayerData& out);
	void evaluate(const LayerData& in,LayerData& out);
};
class DeepDictionary {
private:
	std::vector<FilterLayer> layers;
public:
	DeepDictionary(const std::initializer_list<int>& outputSizes,int inputPatchSize,int sparsity,int angles);
	void train(const std::vector<ImageRGB>& images,int subsample);
	virtual ~DeepDictionary(){}
};

} /* namespace intel */

#endif /* SRC_VISION_DEEPDICTIONARY_H_ */
