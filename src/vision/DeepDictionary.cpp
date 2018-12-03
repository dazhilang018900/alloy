/*
 * DeepDictionary.cpp
 *
 *  Created on: Dec 1, 2018
 *      Author: blake
 */

#include "DeepDictionary.h"

namespace aly {
void FilterBankTensor::evaluate(const DenseMat<float>& in, DenseMat<float>& out,
		int index) {
	filters[index].score(in, out);
	auto fw = weights[index];
	for (float& val : out.data) {
		val = fw.evaluate(val);
	}
}
void FilterBankTensor::evaluate(const Image1ub& in, DenseMat<float>& out,
		int index) {
	filters[index].score(in, out);
	auto fw = weights[index];
	for (float& val : out.data) {
		val = fw.evaluate(val);
	}
}
void FilterBankTensor::evaluate(const ImageRGB& in, DenseMat<float>& out,
		int index) {
	filters[index].score(in, out);
	auto fw = weights[index];
	for (float& val : out.data) {
		val = fw.evaluate(val);
	}
}
void FilterBankTensor::evaluate(const Image1ub& in, Image1f& out, int index) {
	filters[index].score(in, out);
	auto fw = weights[index];
	for (float1& val : out.data) {
		val.x = fw.evaluate(val.x);
	}
}
void FilterBankTensor::evaluate(const ImageRGB& in, Image3f& out, int index) {
	filters[index].score(in, out);
	auto fw = weights[index];
	for (float3& val : out.data) {
		val.x = fw.evaluate(val.x);
		val.y = fw.evaluate(val.y);
		val.z = fw.evaluate(val.z);
	}
}

void FilterLayer::learnInput(const std::vector<Image1ub>& images,
		int inputIndex, int subsample) {
	DictionaryLearning dictionary;
	dictionary.initializeFilters(patchSize, patchSize, angleSamples);
	dictionary.setTrainingData(images, subsample);
	learnInput(dictionary, inputIndex);
}
void FilterLayer::learnInput(const std::vector<LayerData>& images,
		int inputIndex, int subsample) {
	DictionaryLearning dictionary;
	dictionary.initializeFilters(patchSize, patchSize, angleSamples);
	for (int n = 0; n < images.size(); n++) {
		dictionary.addTrainingData(images[n][inputIndex], subsample);
	}
	learnInput(dictionary, inputIndex);
}
void FilterLayer::evaluate(const ImageRGB& in, LayerData& out) {
	Image1f gray;
	ConvertImage(in, gray, true);
	std::vector<DenseMat<float>> mat(1);
	mat[0].set(gray);
	evaluate(mat, out);
}
void FilterLayer::evaluate(const LayerData& in, LayerData& out) {
	DenseMat<float> accum, tmp;
	assert(in.size() == tensors.size());
	assert(in.size() == inputSize);
	out.resize(outputSize);
	for (int outIdx = 0; outIdx < outputSize; outIdx++) {
		for (int inIdx = 0; inIdx < inputSize; inIdx++) {
			const DenseMat<float>& input = in[inIdx];
			FilterBankTensor& tensor = tensors[inIdx];
			assert(tensor.size() == outputSize);
			if (inIdx == 0) {
				accum.resize(input.rows, input.cols);
				accum.setZero();
			}
			tensor.filters[outIdx].score(input, tmp);
			auto fw = tensor.weights[outIdx];
#pragma omp parallel for
			for (size_t idx = 0; idx < accum.size(); idx++) {
				accum.data[idx] += fw.evaluate(tmp.data[idx]);
			}
		}
#pragma omp parallel for
		for (size_t idx = 0; idx < accum.size(); idx++) {
			accum.data[idx] = clamp(accum.data[idx], 0.0f, 1.0f);
		}
		out[outIdx] = accum;
	}

//include max pool operation here
}
void FilterLayer::stash() {

}
void FilterLayer::learnInput(const std::vector<ImageRGB>& images,
		int inputIndex, int subsample) {
	DictionaryLearning dictionary;
	dictionary.initializeFilters(patchSize, patchSize, angleSamples);
	dictionary.setTrainingData(images, subsample);
	learnInput(dictionary, inputIndex);
}
void FilterLayer::learnInput(DictionaryLearning& dictionary, int inputIndex) {
	dictionary.optimizeWeights(sparsity);
	while (dictionary.getFilterCount() < outputSize) {
		dictionary.addFilters(1);
		dictionary.optimizeWeights(sparsity);
	}
	dictionary.optimizeDictionary(1E-5f, 128, sparsity);
	FilterBankTensor& t = tensors[inputIndex];
	t.filters = dictionary.getFilterBanks();
	for (int idx = 0; idx < outputSize; idx++) {
		float2 scaleOffset = dictionary.normalize(idx, 0.001f);
		FilterBankWeights& w = t.weights[idx];
		w.normalizeScale = scaleOffset.x * 0.75;
		w.normalizeOffset = scaleOffset.y * 0.75;
		w.weight = 1.0f / inputSize;
	}
	dictionary.writeFilterBanks(
			MakeDesktopFile(MakeString() << label << "_filter.png"));
}
FilterLayer::FilterLayer(const std::string& label, int featuresIn,
		int featuresOut, int patchSize) :
		label(label), inputSize(featuresIn), outputSize(featuresOut), patchSize(
				patchSize), sparsity(3), angleSamples(8) {
	tensors.resize(featuresIn, FilterBankTensor(featuresOut));
}
void FilterLayer::setSparsity(int s) {
	sparsity = s;
}
void FilterLayer::setAngleSamples(int s) {
	angleSamples = s;
}
DeepDictionary::DeepDictionary(const std::initializer_list<int>& outputSizes,
		const std::initializer_list<int>& filterSizes, int sparsity,
		int angles) {
	int lastSize = 1;
	auto sizeIter = filterSizes.begin();
	int count = 0;
	for (int f : outputSizes) {
		FilterLayer layer(MakeString() << "layer" << count, lastSize, f,
				*sizeIter);
		layer.setSparsity(sparsity);
		layer.setAngleSamples(angles);
		layers.push_back(layer);
		lastSize = f;
		sizeIter++;
		count++;
	}
}
void DeepDictionary::train(const std::vector<ImageRGB>& images, int subsample) {
	if (layers.size() == 0) {
		throw std::runtime_error("No layers in dictionary.");
	}
	std::vector<LayerData> trainingSet(images.size());
	std::vector<LayerData> newTrainingSet;
	layers[0].learnInput(images, 0, subsample);
	layers[0].stash();
	for (int n = 0; n < images.size(); n++) {
		layers[0].evaluate(images[n], trainingSet[n]);
		for (int k = 0; k < trainingSet[n].size(); k++) {
			Image1f tmp;
			trainingSet[n][k].get(tmp);
			WriteImageToRawFile(
					MakeDesktopFile(
							MakeString() << layers[0].getName() << "_"
									<< ZeroPad(n, 2) << "_" << ZeroPad(k, 3)
									<< ".xml"), tmp);
		}
	}
	std::exit(0);
	for (int l = 1; l < (int) layers.size(); l++) {
		FilterLayer& layer = layers[l];
		for (int n = 0; n < layer.getInputSize(); n++) {
			layer.learnInput(trainingSet, n, subsample);
		}
		layer.stash();
		newTrainingSet.resize(images.size());
		for (int n = 0; n < layer.getInputSize(); n++) {
			//Compute output of each layer and store.
			layer.evaluate(trainingSet[n], newTrainingSet[n]);
		}
		trainingSet = newTrainingSet;
		newTrainingSet.clear();
	}
}

} /* namespace intel */
