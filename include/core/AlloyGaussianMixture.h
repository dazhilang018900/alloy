/*
 * AlloyGMM.h
 *
 *  Created on: Sep 22, 2017
 *      Author: blake
 */

#ifndef INCLUDE_CORE_ALLOYGAUSSIANMIXTURE_H_
#define INCLUDE_CORE_ALLOYGAUSSIANMIXTURE_H_

// Copyright 2008-2016 Conrad Sanderson (http://conradsanderson.id.au)
// Copyright 2008-2016 National ICT Australia (NICTA)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ------------------------------------------------------------------------
#include <AlloyOptimizationMath.h>
#include <AlloyMath.h>
namespace aly {
void SANITY_CHECK_GMM();
class GaussianMixture {
protected:
	DenseMat<float> means;
	std::vector<DenseMat<float>> sigmas;
	std::vector<DenseMat<float>> invSigmas;
	Vec<float> priors;
	void initializeMeans(const DenseMat<float>& X);
	void initializeParameters(const DenseMat<float>&X, float var_floor);
	bool iterateKMeans(const DenseMat<float>& X, int max_iter);
public:

	GaussianMixture();
	bool solve(const DenseMat<float>& data, int N_gaus, int km_iter,
			int em_iter, float var_floor = 1E-16f);
	//
	virtual ~GaussianMixture() {
	}
};
}

#endif /* INCLUDE_CORE_ALLOYGAUSSIANMIXTURE_H_ */

