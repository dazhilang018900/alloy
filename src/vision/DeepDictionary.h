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
class FilterLayer{
	std::vector<FilterBank> filters;
	std::vector<FilterWeights> weights;
	std::vector<DenseMat<float>> inputs;
};
class DeepDictionary {
public:
	DeepDictionary();
	virtual ~DeepDictionary();
};

} /* namespace intel */

#endif /* SRC_VISION_DEEPDICTIONARY_H_ */
