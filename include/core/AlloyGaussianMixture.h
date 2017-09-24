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
	std::vector<DenseMat<float>> fcovs;
	std::vector<DenseMat<float>> inv_fcovs;
	Vec<float> memberships;
	std::vector<float> log_det_etc;
	void generate_initial_means(const DenseMat<float>& X);
	void generate_initial_params(const DenseMat<float>&X, float var_floor);
	bool km_iterate(const DenseMat<float>& X, int max_iter);
	//void em_fix_params(float var_floor);
	void init_constants();
	double internal_scalar_log_p(const double* x, const int g) const;
	double em_generate_acc(const DenseMat<float>& X, int start_index,
			int end_index, DenseMat<float>& acc_means,
			DenseVol<float>& acc_fcovs, Vec<float>& acc_norm_lhoods,
			Vec<float>& gaus_log_lhoods) const;
	bool gmm_em_iterate(const DenseMat<float>& X, int max_iter,
			float var_floor);
	bool em_update_params(const DenseMat<float>& X,
			DenseMat<float>& t_acc_means, DenseVol<float>& t_acc_fcovs,
			DenseVol<float>& t_acc_norm_lhoods,
			DenseVol<float>& t_gaus_log_lhoods, float var_floor);
public:

	GaussianMixture();
	bool learn(const DenseMat<float>& data, int N_gaus, int km_iter,int em_iter, float var_floor=1E-16f);
	//
	virtual ~GaussianMixture(){}
};
}

#endif /* INCLUDE_CORE_ALLOYGAUSSIANMIXTURE_H_ */

