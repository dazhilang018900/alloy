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
struct FilterWeights{
	float weight=1;
	float normalizeScale=1;
	float normalizeOffset=0;
	inline float evaluate(float x){
		return weight*((x-normalizeOffset)*normalizeScale);
	}
};
struct FilterTensor{
	std::vector<FilterBank> filters;
	std::vector<FilterWeights> weights;
	FilterTensor(size_t sz=0):filters(sz),weights(sz){}

	void evaluate(const Image1ub& in,DenseMat<float>& out,int index);
	void evaluate(const ImageRGB& in,DenseMat<float>& out,int index);
	void evaluate(const Image1ub& in,Image1f& out,int index);
	void evaluate(const ImageRGB& in,Image3f& out,int index);

};
class FilterLayer{
private:
	std::vector<FilterTensor> tensors;
	std::vector<DenseMat<float>> inputs;
	std::vector<DenseMat<float>> outputs;
	int inputSize;
	int outputSize;
	int patchSize;
	int sparsity;
	int angleSamples;
	void learnInput(DictionaryLearning& dictionary,int inputIndex);
public:
	FilterLayer(int featuresIn,int featuresOut,int patchSize):inputSize(featuresIn),outputSize(featuresOut),patchSize(patchSize),sparsity(3),angleSamples(8){
		tensors.resize(featuresIn,FilterTensor(featuresOut));
		inputs.resize(featuresIn);
		outputs.resize(featuresOut);
	}
	void setSparsity(int s){
		sparsity=s;
	}
	void setAngleSamples(int s){
		angleSamples=s;
	}
	void learnInput(const std::vector<Image1ub>& images,int inputIndex,int subsample);
	void learnInput(const std::vector<ImageRGB>& images,int inputIndex,int subsample);
	void learnOutput();//forward pass, optimize filter weights

};
class DeepDictionary {
public:
	DeepDictionary();
	virtual ~DeepDictionary();
};

} /* namespace intel */

#endif /* SRC_VISION_DEEPDICTIONARY_H_ */
