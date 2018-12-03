/*
 * DeepDictionary.cpp
 *
 *  Created on: Dec 1, 2018
 *      Author: blake
 */

#include "DeepDictionary.h"

namespace aly {

void FilterTensor::evaluate(const Image1ub& in,DenseMat<float>& out,int index){
	filters[index].score(in ,out);
	out=weights[index].normalizeScale*out+weights[index].normalizeOffset;
}
void FilterTensor::evaluate(const ImageRGB& in,DenseMat<float>& out,int index){
	filters[index].score(in ,out);
	out=weights[index].normalizeScale*out+weights[index].normalizeOffset;
}
void FilterTensor::evaluate(const Image1ub& in,Image1f& out,int index){
	filters[index].score(in ,out);
	out=weights[index].normalizeScale*out+weights[index].normalizeOffset;

}
void FilterTensor::evaluate(const ImageRGB& in,Image3f& out,int index){
	filters[index].score(in ,out);
	out=weights[index].normalizeScale*out+weights[index].normalizeOffset;

}

void FilterLayer::learnInput(const std::vector<Image1ub>& images,int inputIndex,
		int subsample) {
	DictionaryLearning dictionary;
	dictionary.initializeFilters(patchSize, patchSize, angleSamples);
	dictionary.setTrainingData(images, subsample);
	learnInput(dictionary,inputIndex);
}
void FilterLayer::learnInput(const std::vector<ImageRGB>& images,int inputIndex,
		int subsample) {
	DictionaryLearning dictionary;
	dictionary.initializeFilters(patchSize, patchSize, angleSamples);
	dictionary.setTrainingData(images, subsample);
	learnInput(dictionary,inputIndex);
}
void FilterLayer::learnInput(DictionaryLearning& dictionary,int inputIndex) {
	dictionary.optimizeWeights(sparsity);
	while (dictionary.getFilterCount() < outputSize) {
		dictionary.addFilters(1);
		dictionary.optimizeWeights(sparsity);
	}
	dictionary.optimizeDictionary(1E-5f, 128, 3);
	FilterTensor& t=tensors[inputIndex];
	t.filters=dictionary.getFilterBanks();
	t.weights.resize(outputs.size());
	for (int idx=0;idx<outputSize;idx++) {
		float2 scaleOffset=dictionary.normalize(idx,0.05f);
		std::cout<<"Normalization "<<scaleOffset<<std::endl;
		FilterWeights& w=t.weights[idx];
		w.normalizeScale=scaleOffset.x;
		w.normalizeOffset=scaleOffset.y;
		w.weight=1.0f/outputSize;
	}
}
DeepDictionary::DeepDictionary() {
	// TODO Auto-generated constructor stub

}

DeepDictionary::~DeepDictionary() {
	// TODO Auto-generated destructor stub
}

} /* namespace intel */
