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
	std::vector<double> scaleFactors;
	void initializeMeans(const DenseMat<float>& X);
	void initializeParameters(const DenseMat<float>&X, float var_floor);
	bool iterateKMeans(const DenseMat<float>& X, int max_iter);
public:
	double distanceMahalanobis(const Vec<float>& pt, int g) const;
	double distanceEuclidean(const Vec<float>& pt, int g) const;
	double distanceMahalanobis(const Vec<float>& pt) const;
	double distanceEuclidean(const Vec<float>& pt) const;
	double likelihood(const Vec<float>& pt) const;
	GaussianMixture();
	bool solve(const DenseMat<float>& data, int N_gaus, int km_iter,
			int em_iter, float var_floor = 1E-16f);
	//
	virtual ~GaussianMixture() {
	}
};
class GaussianMixtureRGB {
protected:
	std::vector<float3> means;
	std::vector<float3x3> sigmas;
	std::vector<float3x3> invSigmas;
	std::vector<float> priors;
	std::vector<double> scaleFactors;
	void initializeMeans(const std::vector<float3>& X);
	void initializeParameters(const std::vector<float3>& X, float var_floor);
	bool iterateKMeans(const std::vector<float3>& X, int max_iter);
public:
	GaussianMixtureRGB();
	double distanceMahalanobis(float3 pt, int g) const;
	double distanceEuclidean(float3 pt, int g) const;

	float3 deltaMahalanobis(float3 pt, int g) const;
	float3 deltaEuclidean(float3 pt, int g) const;

	float3 deltaMahalanobis(float3 pt) const;
	float3 deltaEuclidean(float3 pt) const;

	float3 getMean(int g) const;
	float3x3 getCovariance(int g) const;
	float3x3 getInverseCovariance(int g) const;
	float getPrior(int g) const;
	int maxPrior() const;
	double distanceMahalanobis(float3 pt) const;
	double distanceEuclidean(float3 pt) const;



	int closestMahalanobis(float3 pt) const;
	int closestEuclidean(float3 pt) const;
	double likelihood(float3 pt) const;
	bool solve(const std::vector<float3>& data, int N_gaus, int km_iter,
			int em_iter, float var_floor = 1E-16f);
	virtual ~GaussianMixtureRGB() {
	}
};
}

#endif /* INCLUDE_CORE_ALLOYGAUSSIANMIXTURE_H_ */

