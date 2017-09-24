/*
 * AlloyGMM.cpp
 *
 *  Created on: Sep 22, 2017
 *      Author: Blake Lucas
 */

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
//! \addtogroup gmm_full
//! @{
#include <AlloyGaussianMixture.h>
#include <AlloyOptimization.h>
namespace aly {
void SANITY_CHECK_GMM() {
	int G = 4;
	int D = 2;
	int S = 1000;
	std::vector<float> priors(G);
	std::vector<Vec<float>> centers(G, Vec<float>(D));
	std::cout << "Start Allocation" << std::endl;
	centers[0].data= {0.1f,0.3f};
	centers[1].data= {0.5f,0.3f};
	centers[2].data= {0.4f,1.0f};
	centers[3].data= {0.9f,0.3f};

	priors[0] = 0.1f;
	priors[1] = 0.5f;
	priors[2] = 0.3f;
	priors[3] = 0.1f;

	DenseMat<float> samples(D, S);
	std::cout << "Generate samples" << std::endl;
	std::vector<int> order(S);
	for (int i = 0; i < order.size(); i++) {
		order[i] = i;
	}
	Shuffle(order);
	for (int i = 0; i < S; i++) {
		int s = order[i];
		float thresh = RandomUniform(0.0f, 1.0f);
		float last = 0.0f;
		float curr = 0.0f;
		for (int k = 0; k < G; k++) {
			curr += priors[k];
			if (thresh >= last && thresh < curr) {
				Vec<float> sample(D);
				for (int d = 0; d < D; d++) {
					sample[d] = RandomGaussian(0.0f, 1.0f);
				}
				sample = sample * 0.1f + centers[k];
				samples.setColumn(sample, i);
				break;
			}
			last = curr;
		}
	}
	GaussianMixture gmm;
	std::cout << "Start Learning " << samples.dimensions() << std::endl;
	gmm.learn(samples, G, 100, 20, 0.0001f);
}
GaussianMixture::GaussianMixture() {

}

void GaussianMixture::init_constants() {
	const int N_dims = means.rows;
	const int N_gaus = means.cols;
	const double tmp = (double(N_dims) / double(2))
			* std::log(double(2) * ALY_PI);
	int2 dims = fcovs.front().dimensions();
	inv_fcovs.resize(fcovs.size(), DenseMat<float>(dims.x, dims.y));
	DenseMat<float> U, D, Vt;
	for (int g = 0; g < N_gaus; ++g) {
		const DenseMat<float>& fcov = fcovs[g];
		DenseMat<float>& inv_fcov = inv_fcovs[g];
		inv_fcov = inverse(fcov, 1E-16f);
	}
	log_det_etc.resize(N_gaus, 0.0f);
}
double GaussianMixture::internal_scalar_log_p(const double* x,
		const int g) const {
	/*
	 const int N_dims = means.rows;
	 VecMap<float> mean_mem = means.getColumn(g);
	 double outer_acc = double(0);
	 DenseMat<float> inv_fcov_coldata = inv_fcovs[g];
	 for (int i = 0; i < N_dims; ++i) {
	 double inner_acc = double(0);
	 for (int j = 0; j < N_dims; ++j) {
	 inner_acc += (x[j] - mean_mem[j]) * inv_fcov_coldata[j];
	 }

	 inv_fcov_coldata += N_dims;
	 outer_acc += inner_acc * (x[i] - mean_mem[i]);
	 }

	 return double(-0.5) * outer_acc + log_det_etc[g];
	 */
	return 0;

}
double GaussianMixture::em_generate_acc(const DenseMat<float>& X,
		int start_index, int end_index, DenseMat<float>& acc_means,
		DenseVol<float>& acc_fcovs, Vec<float>& acc_norm_lhoods,
		Vec<float>& gaus_log_lhoods) const {
	/*
	 acc_means.setZero();
	 acc_fcovs.setZero();
	 acc_norm_lhoods.setZero();
	 gaus_log_lhoods.setZero();
	 const int N_dims = means.rows;
	 const int N_gaus = means.cols;
	 for (int i = start_index; i <= end_index; i++) {
	 const double* x = X.getColumn(i);
	 for (int g = 0; g < N_gaus; ++g) {
	 gaus_log_lhoods_mem[g] = internal_scalar_log_p(x, g)
	 + log_hefts_mem[g];
	 }

	 double log_lhood_sum = gaus_log_lhoods_mem[0];

	 for (int g = 1; g < N_gaus; ++g) {
	 log_lhood_sum = log_add_exp(log_lhood_sum, gaus_log_lhoods_mem[g]);
	 }

	 progress_log_lhood += log_lhood_sum;

	 for (int g = 0; g < N_gaus; ++g) {
	 const double norm_lhood = std::exp(
	 gaus_log_lhoods_mem[g] - log_lhood_sum);

	 acc_norm_lhoods[g] += norm_lhood;

	 double* acc_mean_mem = acc_means.colptr(g);

	 for (int d = 0; d < N_dims; ++d) {
	 acc_mean_mem[d] += x[d] * norm_lhood;
	 }

	 Mat<double>& acc_fcov = access::rw(acc_fcovs).slice(g);

	 // specialised version of acc_fcov += norm_lhood * (xx * xx.t());

	 for (int d = 0; d < N_dims; ++d) {
	 const int dp1 = d + 1;

	 const double xd = x[d];

	 double* acc_fcov_col_d = acc_fcov.colptr(d) + d;
	 double* acc_fcov_row_d = &(acc_fcov.at(d, dp1));

	 (*acc_fcov_col_d) += norm_lhood * (xd * xd);
	 acc_fcov_col_d++;

	 for (int e = dp1; e < N_dims; ++e) {
	 const double val = norm_lhood * (xd * x[e]);

	 (*acc_fcov_col_d) += val;
	 acc_fcov_col_d++;
	 (*acc_fcov_row_d) += val;
	 acc_fcov_row_d += N_dims;
	 }
	 }
	 }
	 }

	 progress_log_lhood /= double((end_index - start_index) + 1);
	 */
}

bool GaussianMixture::em_update_params(const DenseMat<float>& X,
		DenseMat<float>& t_acc_means, DenseVol<float>& t_acc_fcovs,
		DenseVol<float>& t_acc_norm_lhoods, DenseVol<float>& t_gaus_log_lhoods,
		float var_floor) {
	/*
	 // em_generate_acc() is the "map" operation, which produces partial accumulators for means, diagonal covariances and hefts
	 em_generate_acc(X, boundaries.at(0, 0), boundaries.at(1, 0), t_acc_means[0],
	 t_acc_fcovs[0], t_acc_norm_lhoods[0], t_gaus_log_lhoods[0],
	 t_progress_log_lhood[0]);

	 const int N_dims = means.rows;
	 const int N_gaus = means.cols;
	 DenseMat<float> mean_outer(N_dims, N_dims);
	 for (int g = 0; g < N_gaus; ++g) {
	 float acc_norm_lhood = t_acc_norm_lhoods[g];
	 for (int d = 0; d < N_dims; ++d) {
	 t_acc_means[d] /= acc_norm_lhood;
	 }
	 Vec<float> new_mean = t_acc_means;
	 mean_outer = new_mean * new_mean;
	 DenseMat<float>& acc_fcov = t_acc_fcovs[g];
	 acc_fcov /= acc_norm_lhood;
	 acc_fcov -= mean_outer;
	 for (int d = 0; d < N_dims; ++d) {
	 double& val = acc_fcov.at(d, d);
	 if (val < var_floor) {
	 val = var_floor;
	 }
	 }

	 if (acc_fcov.is_finite() == false) {
	 continue;
	 }

	 double log_det_val = double(0);
	 double log_det_sign = double(0);
	 hefts_mem[g] = acc_norm_lhood / double(X.cols);
	 double* mean_mem = access::rw(means).colptr(g);
	 for (int d = 0; d < N_dims; ++d) {
	 mean_mem[d] = acc_mean_mem[d];
	 }
	 Mat<double>& fcov = access::rw(fcovs).slice(g);

	 fcov = acc_fcov;
	 }
	 }*/
	return true;
}
/*
void GaussianMixture::em_fix_params(float var_floor) {
	const int N_dims = means.rows;
	const int N_gaus = means.cols;
	const double ZERO_TOLERANCE = 1E-16;
	const double var_ceiling = 1E16f;
	for (int g = 0; g < N_gaus; ++g) {
		DenseMat<float> &fcov = fcovs[g];
		for (int d = 0; d < N_dims; ++d) {
			float& var_val = fcov[d][d];
			if (var_val < var_floor) {
				var_val = var_floor;
			} else if (var_val > var_ceiling) {
				var_val = var_ceiling;
			} else if (std::isnan(var_val)) {
				var_val = double(1);
			}
		}
	}
	for (int g1 = 0; g1 < N_gaus; ++g1) {
		if (memberships[g1] > double(0)) {
			VecMap<float> means_colptr_g1 = means.getColumn(g1);
			for (int g2 = (g1 + 1); g2 < N_gaus; ++g2) {
				if ((memberships[g2] > double(0))
						&& (std::abs(memberships[g1] - memberships[g2])
								<= std::numeric_limits<double>::epsilon())) {
					double dist = distance(means.getColumn(g2),
							means_colptr_g1);
					if (dist == double(0)) {
						memberships[g2] = double(0);
					}
				}
			}
		}
	}
	const double heft_floor = std::numeric_limits<double>::min();
	const double heft_initial = double(1) / double(N_gaus);
	for (int i = 0; i < N_gaus; ++i) {
		float& heft_val = memberships[i];
		if (heft_val < heft_floor) {
			heft_val = heft_floor;
		} else if (heft_val > double(1)) {
			heft_val = double(1);
		} else if (std::isnan(heft_val)) {
			heft_val = heft_initial;
		}
	}
	float heft_sum = reduce(memberships);
	memberships /= heft_sum;
}
*/
bool GaussianMixture::gmm_em_iterate(const DenseMat<float>& X, int max_iter,
		float var_floor) {
	const int N_dims = means.rows;
	const int N_gaus = means.cols;
	const double ZERO_TOLERANCE = 1E30;
	DenseMat<double> t_acc_means(N_dims, N_gaus);
	Vec<double> t_acc_norm_lhoods(N_gaus);
	Vec<double> t_gaus_log_lhoods(N_gaus);
	Vec<double> t_progress_log_lhood(N_gaus);
	DenseVol<double> t_acc_fcovs(N_dims, N_dims, N_gaus);
	/*
	 double old_avg_log_p = -1E30;
	 for (int iter = 1; iter <= max_iter; ++iter) {
	 init_constants();
	 em_update_params(X, boundaries, t_acc_means, t_acc_fcovs,
	 t_acc_norm_lhoods, t_gaus_log_lhoods, t_progress_log_lhood,
	 var_floor);
	 em_fix_params(var_floor);
	 const double new_avg_log_p = reduce(t_progress_log_lhood)
	 / double(t_progress_log_lhood.size());
	 if (std::isfinite(new_avg_log_p) == false) {
	 return false;
	 }
	 if (std::abs(old_avg_log_p - new_avg_log_p) <= ZERO_TOLERANCE) {
	 break;
	 }
	 old_avg_log_p = new_avg_log_p;
	 }
	 for (int g = 0; g < N_gaus; ++g) {
	 const DenseMat<double>& fcov = fcovs[g];
	 for (int n = 0; n < N_gaus; n++) {
	 if (fcov[n][n] <= 0) {
	 return false;
	 }
	 }
	 }
	 */
	return true;
}

void GaussianMixture::generate_initial_params(const DenseMat<float>& X,
		float var_floor) {
	const int N_dims = means.rows;
	const int N_gaus = means.cols;
	const int X_cols = X.cols;
	if (X_cols == 0) {
		return;
	}
// as the covariances are calculated via accumulators,
// the means also need to be calculated via accumulators to ensure numerical consistency
	DenseMat<float> acc_means(N_dims, N_gaus);
	DenseMat<float> acc_dcovs(N_dims, N_gaus);
	acc_means.setZero();
	acc_dcovs.setZero();
	std::vector<int> sumMembers(N_gaus, 0);
	for (int i = 0; i < X_cols; ++i) {
		VecMap<float> sample = X.getColumn(i);
		double min_dist = 1E30;
		int best_g = 0;
		for (int g = 0; g < N_gaus; ++g) {
			VecMap<float> mean = means.getColumn(g);
			double dist = distanceSqr(sample, mean);

			if (dist < min_dist) {
				min_dist = dist;
				best_g = g;
			}
		}
		sumMembers[best_g]++;
		VecMap<float> acc_mean = acc_means.getColumn(best_g);
		VecMap<float> acc_dcov = acc_dcovs.getColumn(best_g);
		for (int d = 0; d < N_dims; ++d) {
			float x_d = sample[d];

			acc_mean[d] += x_d;
			acc_dcov[d] += x_d * x_d;
		}
	}
	for (int g = 0; g < N_gaus; ++g) {
		VecMap<float> acc_mean = acc_means.getColumn(g);
		VecMap<float> acc_dcov = acc_dcovs.getColumn(g);
		int sumMember = sumMembers[g];
		VecMap<float> mean = means.getColumn(g);
		DenseMat<float>& fcov = fcovs[g];
		fcov.setZero();
		for (int d = 0; d < N_dims; ++d) {
			double tmp = acc_mean[d] / double(sumMember);
			mean[d] = (sumMember >= 1) ? tmp : (0);
			fcov[d][d] =(sumMember >= 2) ?float((acc_dcov[d] / float(sumMember))- (tmp * tmp)) :float(var_floor);
		}
		memberships[g] = sumMember / (float) X_cols;
	}
	//em_fix_params(var_floor);
}

void GaussianMixture::generate_initial_means(const DenseMat<float>& X) {
	const int N_gaus = means.cols;
	const int N_dims = means.rows;
	// going through all of the samples can be extremely time consuming;
	// instead, if there are enough samples, randomly choose samples with probability 0.1
	const int stride = 10;
	const bool use_sampling = ((X.cols / int(stride * stride)) > N_gaus);
	const int step = (use_sampling) ? int(stride) : int(1);
	int start_index = RandomUniform(0, X.cols - 1);
	means.setColumn(X.getColumn(start_index), 0);
	double max_dist = 0.0;
	for (int g = 1; g < N_gaus; ++g) {
		int best_i = int(0);
		int start_i = int(0);
		for (int i = 0; i < X.cols; i += step) {
			bool ignore_i = false;
			// find the average distance between sample i and the means so far
			VecMap<float> sample = X.getColumn(i + RandomUniform(0, step - 1));
			double sum = 0.0;
			for (int h = 0; h < g; ++h) {
				double dist = distanceSqr(means.getColumn(h), sample);
				if (dist == 0.0) {
					ignore_i = true;
					break;
				} else {
					sum += dist;
				}
			}
			if (!ignore_i && sum >= max_dist) {
				max_dist = sum;
				best_i = i;
			}
		}
		// set the mean to the sample that is the furthest away from the means so far
		means.setColumn(X.getColumn(best_i), g);
	}
}
bool GaussianMixture::km_iterate(const DenseMat<float>& X, int max_iter) {
	const double ZERO_TOLERANCE = 1E-16;
	const int X_n_cols = X.cols;
	const int N_dims = means.rows;
	const int N_gaus = means.cols;
	DenseMat<float> acc_means(N_dims, N_gaus);
	std::vector<int> acc_hefts(N_gaus, 0);
	std::vector<int> last_indx(N_gaus, 0);
	acc_means.setZero();
	DenseMat<float> new_means = means;
	DenseMat<float> old_means = means;
	const int n_threads = 1;
	std::cout << " " << new_means.dimensions() << " " << means.dimensions()
			<< std::endl;
	for (int iter = 1; iter <= max_iter; ++iter) {
		//Find closest cluster
		for (int i = 0; i < X_n_cols; ++i) {
			VecMap<float> sample = X.getColumn(i);
			double min_dist = 1E30;
			int best_g = 0;
			for (int g = 0; g < N_gaus; ++g) {
				double dist = distanceSqr(old_means.getColumn(g), sample);
				if (dist < min_dist) {
					min_dist = dist;
					best_g = g;
				}
			}
			VecMap<float> acc_mean = acc_means.getColumn(best_g);
			acc_mean += sample;

			acc_hefts[best_g]++;
			last_indx[best_g] = i;
		}
		// generate new means
		for (int g = 0; g < N_gaus; ++g) {
			VecMap<float> acc_mean = acc_means.getColumn(g);
			int acc_heft = acc_hefts[g];
			VecMap<float> new_mean = new_means.getColumn(g);
			for (int d = 0; d < N_dims; ++d) {
				new_mean[d] =
						(acc_heft >= 1) ?
								(acc_mean[d] / float(acc_heft)) : float(0);
			}
		}
		// heuristics to resurrect dead means in the even cluster centers collapse
		std::vector<int> dead_gs;
		for (int i = 0; i < acc_hefts.size(); i++) {
			if (acc_hefts[i] == 0) {
				dead_gs.push_back(i);
			}
			if (dead_gs.size() > 0) {
				std::vector<int> live_gs;
				for (int i = 0; i < acc_hefts.size(); i++) {
					if (acc_hefts[i] >= 2) {
						live_gs.push_back(i);
					}
				}
				std::sort(live_gs.begin(), live_gs.end(),
						[=](const int& a,const int& b) {return a>b;});

				if (live_gs.size() == 0) {
					return false;
				}
				int live_gs_count = 0;
				for (int dead_gs_count = 0;
						dead_gs_count < (int) dead_gs.size(); ++dead_gs_count) {
					const int dead_g_id = dead_gs[dead_gs_count];
					int proposed_i = 0;
					if (live_gs_count < live_gs.size()) {
						const int live_g_id = live_gs[live_gs_count];
						++live_gs_count;
						if (live_g_id == dead_g_id) {
							return false;
						}
						// recover by using a sample from a known good mean
						proposed_i = last_indx[live_g_id];
					} else {
						// recover by using a randomly selected sample (last resort)
						proposed_i = RandomUniform(0, X_n_cols - 1);
					}
					if (proposed_i >= X_n_cols) {
						return false;
					}
					new_means.setColumn(X.getColumn(proposed_i), dead_g_id);
				}
			}
			double rs_delta = 0;
			for (int g = 0; g < N_gaus; ++g) {
				rs_delta += distance(old_means.getColumn(g),
						new_means.getColumn(g));
			}
			rs_delta /= N_gaus;
			std::swap(old_means, new_means);
			if (rs_delta <= ZERO_TOLERANCE) {
				break;
			}
		}
		means = old_means;
	}

	std::cout << "Estimated Means:" << std::endl;
	for (int g = 0; g < means.cols; g++) {
		std::cout << "means[" << g << "]=" << means.getColumn(g) << std::endl;
	}
	return true;
}

bool GaussianMixture::learn(const DenseMat<float>& data, int N_gaus,
		int km_iter, int em_iter, float var_floor) {
	means.resize(data.rows, N_gaus);
	memberships.resize(N_gaus);
	fcovs.resize(N_gaus, DenseMat<float>(data.rows, data.rows));
	inv_fcovs.resize(N_gaus, DenseMat<float>(data.rows, data.rows));

	// initial means

	generate_initial_means(data);
// k-means
	if (km_iter > 0) {
		if (!km_iterate(data, km_iter)) {
			return false;
		}
	}
	// initial fcovs
	//generate_initial_params(data, var_floor);
	/*
	 // EM algorithm

	 if (em_iter > 0) {
	 const arma_ostream_state stream_state(get_stream_err2());

	 const bool status = em_iterate(X, em_iter, var_floor_actual,
	 print_mode);

	 stream_state.restore(get_stream_err2());

	 if (status == false) {
	 arma_debug_warn("gmm_full::learn(): EM algorithm failed");
	 init (orig);
	 return false;
	 }
	 }

	 mah_aux.reset();

	 init_constants();
	 */
	return true;
}
}
/*

 namespace gmm_priv
 {


 template<typename double>
 inline
 gmm_full<double>::~gmm_full()
 {
 arma_extra_debug_sigprint_this(this);

 arma_type_check(( (is_same_type<double,float>::value == false) && (is_same_type<double,double>::value == false) ));
 }



 template<typename double>
 inline
 gmm_full<double>::gmm_full()
 {
 arma_extra_debug_sigprint_this(this);
 }



 template<typename double>
 inline
 gmm_full<double>::gmm_full(const gmm_full<double>& x)
 {
 arma_extra_debug_sigprint_this(this);

 init(x);
 }



 template<typename double>
 inline
 gmm_full<double>&
 gmm_full<double>::operator=(const gmm_full<double>& x)
 {
 arma_extra_debug_sigprint();

 init(x);

 return *this;
 }



 template<typename double>
 inline
 gmm_full<double>::gmm_full(const gmm_diag<double>& x)
 {
 arma_extra_debug_sigprint_this(this);

 init(x);
 }



 template<typename double>
 inline
 gmm_full<double>&
 gmm_full<double>::operator=(const gmm_diag<double>& x)
 {
 arma_extra_debug_sigprint();

 init(x);

 return *this;
 }



 template<typename double>
 inline
 gmm_full<double>::gmm_full(const int in_n_dims, const int in_n_gaus)
 {
 arma_extra_debug_sigprint_this(this);

 init(in_n_dims, in_n_gaus);
 }



 template<typename double>
 inline
 void
 gmm_full<double>::reset()
 {
 arma_extra_debug_sigprint();

 init(0, 0);
 }



 template<typename double>
 inline
 void
 gmm_full<double>::reset(const int in_n_dims, const int in_n_gaus)
 {
 arma_extra_debug_sigprint();

 init(in_n_dims, in_n_gaus);
 }



 template<typename double>
 template<typename T1, typename T2, typename T3>
 inline
 void
 gmm_full<double>::set_params(const Base<double,T1>& in_means_expr, const BaseCube<double,T2>& in_fcovs_expr, const Base<double,T3>& in_hefts_expr)
 {
 arma_extra_debug_sigprint();

 const unwrap     <T1> tmp1(in_means_expr.get_ref());
 const unwrap_cube<T2> tmp2(in_fcovs_expr.get_ref());
 const unwrap     <T3> tmp3(in_hefts_expr.get_ref());

 const Mat <double>& in_means = tmp1.M;
 const Cube<double>& in_fcovs = tmp2.M;
 const Mat <double>& in_hefts = tmp3.M;

 arma_debug_check
 (
 (in_means.cols != in_fcovs.n_slices) || (in_means.rows != in_fcovs.rows) || (in_fcovs.rows != in_fcovs.cols) || (in_hefts.cols != in_means.cols) || (in_hefts.rows != 1),
 "gmm_full::set_params(): given parameters have inconsistent and/or wrong sizes"
 );

 arma_debug_check( (in_means.is_finite() == false), "gmm_full::set_params(): given means have non-finite values" );
 arma_debug_check( (in_fcovs.is_finite() == false), "gmm_full::set_params(): given fcovs have non-finite values" );
 arma_debug_check( (in_hefts.is_finite() == false), "gmm_full::set_params(): given hefts have non-finite values" );

 for(int g=0; g < in_fcovs.n_slices; ++g)
 {
 arma_debug_check( (any(diagvec(in_fcovs.slice(g)) <= double(0))), "gmm_full::set_params(): given fcovs have negative or zero values on diagonals" );
 }

 arma_debug_check( (any(vectorise(in_hefts) <  double(0))), "gmm_full::set_params(): given hefts have negative values" );

 const double s = accu(in_hefts);

 arma_debug_check( ((s < (double(1) - Datum<double>::eps)) || (s > (double(1) + Datum<double>::eps))), "gmm_full::set_params(): sum of given hefts is not 1" );

 access::rw(means) = in_means;
 access::rw(fcovs) = in_fcovs;
 access::rw(hefts) = in_hefts;

 init_constants();
 }



 template<typename double>
 template<typename T1>
 inline
 void
 gmm_full<double>::set_means(const Base<double,T1>& in_means_expr)
 {
 arma_extra_debug_sigprint();

 const unwrap<T1> tmp(in_means_expr.get_ref());

 const Mat<double>& in_means = tmp.M;

 arma_debug_check( (size(in_means) != size(means)), "gmm_full::set_means(): given means have incompatible size" );
 arma_debug_check( (in_means.is_finite() == false), "gmm_full::set_means(): given means have non-finite values" );

 access::rw(means) = in_means;
 }



 template<typename double>
 template<typename T1>
 inline
 void
 gmm_full<double>::set_fcovs(const BaseCube<double,T1>& in_fcovs_expr)
 {
 arma_extra_debug_sigprint();

 const unwrap_cube<T1> tmp(in_fcovs_expr.get_ref());

 const Cube<double>& in_fcovs = tmp.M;

 arma_debug_check( (size(in_fcovs) != size(fcovs)), "gmm_full::set_fcovs(): given fcovs have incompatible size" );
 arma_debug_check( (in_fcovs.is_finite() == false), "gmm_full::set_fcovs(): given fcovs have non-finite values" );

 for(int i=0; i < in_fcovs.n_slices; ++i)
 {
 arma_debug_check( (any(diagvec(in_fcovs.slice(i)) <= double(0))), "gmm_full::set_fcovs(): given fcovs have negative or zero values on diagonals" );
 }

 access::rw(fcovs) = in_fcovs;

 init_constants();
 }



 template<typename double>
 template<typename T1>
 inline
 void
 gmm_full<double>::set_hefts(const Base<double,T1>& in_hefts_expr)
 {
 arma_extra_debug_sigprint();

 const unwrap<T1> tmp(in_hefts_expr.get_ref());

 const Mat<double>& in_hefts = tmp.M;

 arma_debug_check( (size(in_hefts) != size(hefts)),     "gmm_full::set_hefts(): given hefts have incompatible size" );
 arma_debug_check( (in_hefts.is_finite() == false),     "gmm_full::set_hefts(): given hefts have non-finite values" );
 arma_debug_check( (any(vectorise(in_hefts) <  double(0))), "gmm_full::set_hefts(): given hefts have negative values"   );

 const double s = accu(in_hefts);

 arma_debug_check( ((s < (double(1) - Datum<double>::eps)) || (s > (double(1) + Datum<double>::eps))), "gmm_full::set_hefts(): sum of given hefts is not 1" );

 // make sure all hefts are positive and non-zero

 const double* in_hefts_mem = in_hefts.memptr();
 double*    hefts_mem = access::rw(hefts).memptr();

 for(int i=0; i < hefts.n_elem; ++i)
 {
 hefts_mem[i] = (std::max)( in_hefts_mem[i], std::numeric_limits<double>::min() );
 }

 access::rw(hefts) /= accu(hefts);

 log_hefts = log(hefts);
 }



 template<typename double>
 inline
 int
 gmm_full<double>::n_dims() const
 {
 return means.rows;
 }



 template<typename double>
 inline
 int
 gmm_full<double>::n_gaus() const
 {
 return means.cols;
 }



 template<typename double>
 inline
 bool
 gmm_full<double>::load(const std::string name)
 {
 arma_extra_debug_sigprint();

 field< Mat<double> > storage;

 bool status = storage.load(name, arma_binary);

 if( (status == false) || (storage.n_elem < 2) )
 {
 reset();
 arma_debug_warn("gmm_full::load(): problem with loading or incompatible format");
 return false;
 }

 int count = 0;

 const Mat<double>& storage_means = storage(count);  ++count;
 const Mat<double>& storage_hefts = storage(count);  ++count;

 const int N_dims = storage_means.rows;
 const int N_gaus = storage_means.cols;

 if( (storage.n_elem != (N_gaus + 2)) || (storage_hefts.rows != 1) || (storage_hefts.cols != N_gaus) )
 {
 reset();
 arma_debug_warn("gmm_full::load(): incompatible format");
 return false;
 }

 reset(N_dims, N_gaus);

 access::rw(means) = storage_means;
 access::rw(hefts) = storage_hefts;

 for(int g=0; g < N_gaus; ++g)
 {
 const Mat<double>& storage_fcov = storage(count);  ++count;

 if( (storage_fcov.rows != N_dims) || (storage_fcov.cols != N_dims) )
 {
 reset();
 arma_debug_warn("gmm_full::load(): incompatible format");
 return false;
 }

 access::rw(fcovs).slice(g) = storage_fcov;
 }

 init_constants();

 return true;
 }



 template<typename double>
 inline
 bool
 gmm_full<double>::save(const std::string name) const
 {
 arma_extra_debug_sigprint();

 const int N_gaus = means.cols;

 field< Mat<double> > storage(2 + N_gaus);

 int count = 0;

 storage(count) = means;  ++count;
 storage(count) = hefts;  ++count;

 for(int g=0; g < N_gaus; ++g)
 {
 storage(count) = fcovs.slice(g);  ++count;
 }

 const bool status = storage.save(name, arma_binary);

 return status;
 }



 template<typename double>
 inline
 Col<double>
 gmm_full<double>::generate() const
 {
 arma_extra_debug_sigprint();

 const int N_dims = means.rows;
 const int N_gaus = means.cols;

 Col<double> out( (N_gaus > 0) ? N_dims : int(0)              );
 Col<double> tmp( (N_gaus > 0) ? N_dims : int(0), fill::randn );

 if(N_gaus > 0)
 {
 const double val = randu<double>();

 double csum    = double(0);
 int  gaus_id = 0;

 for(int j=0; j < N_gaus; ++j)
 {
 csum += hefts[j];

 if(val <= csum)  { gaus_id = j; break; }
 }

 out  = chol_fcovs.slice(gaus_id) * tmp;
 out += means.col(gaus_id);
 }

 return out;
 }



 template<typename double>
 inline
 Mat<double>
 gmm_full<double>::generate(const int N_vec) const
 {
 arma_extra_debug_sigprint();

 const int N_dims = means.rows;
 const int N_gaus = means.cols;

 Mat<double> out( ( (N_gaus > 0) ? N_dims : int(0) ), N_vec              );
 Mat<double> tmp( ( (N_gaus > 0) ? N_dims : int(0) ), N_vec, fill::randn );

 if(N_gaus > 0)
 {
 const double* hefts_mem = hefts.memptr();

 for(int i=0; i < N_vec; ++i)
 {
 const double val = randu<double>();

 double csum    = double(0);
 int  gaus_id = 0;

 for(int j=0; j < N_gaus; ++j)
 {
 csum += hefts_mem[j];

 if(val <= csum)  { gaus_id = j; break; }
 }

 Col<double> out_vec(out.colptr(i), N_dims, false, true);
 Col<double> tmp_vec(tmp.colptr(i), N_dims, false, true);

 out_vec  = chol_fcovs.slice(gaus_id) * tmp_vec;
 out_vec += means.col(gaus_id);
 }
 }

 return out;
 }



 template<typename double>
 template<typename T1>
 inline
 double
 gmm_full<double>::log_p(const T1& expr, const gmm_empty_arg& junk1, typename enable_if<((is_arma_type<T1>::value) && (resolves_to_colvector<T1>::value == true))>::result* junk2) const
 {
 arma_extra_debug_sigprint();
 arma_ignore(junk1);
 arma_ignore(junk2);

 const int N_dims = means.rows;

 const quasi_unwrap<T1> U(expr);

 arma_debug_check( (U.M.rows != N_dims), "gmm_full::log_p(): incompatible dimensions" );

 return internal_scalar_log_p( U.M.memptr() );
 }



 template<typename double>
 template<typename T1>
 inline
 double
 gmm_full<double>::log_p(const T1& expr, const int gaus_id, typename enable_if<((is_arma_type<T1>::value) && (resolves_to_colvector<T1>::value == true))>::result* junk2) const
 {
 arma_extra_debug_sigprint();
 arma_ignore(junk2);

 const int N_dims = means.rows;

 const quasi_unwrap<T1> U(expr);

 arma_debug_check( (U.M.rows != N_dims),    "gmm_full::log_p(): incompatible dimensions"            );
 arma_debug_check( (gaus_id >= means.cols), "gmm_full::log_p(): specified gaussian is out of range" );

 return internal_scalar_log_p( U.M.memptr(), gaus_id );
 }



 template<typename double>
 template<typename T1>
 inline
 Row<double>
 gmm_full<double>::log_p(const T1& expr, const gmm_empty_arg& junk1, typename enable_if<((is_arma_type<T1>::value) && (resolves_to_colvector<T1>::value == false))>::result* junk2) const
 {
 arma_extra_debug_sigprint();
 arma_ignore(junk1);
 arma_ignore(junk2);

 if(is_subview<T1>::value)
 {
 const subview<double>& X = reinterpret_cast< const subview<double>& >(expr);

 return internal_vec_log_p(X);
 }
 else
 {
 const unwrap<T1>   tmp(expr);
 const Mat<double>& X = tmp.M;

 return internal_vec_log_p(X);
 }
 }



 template<typename double>
 template<typename T1>
 inline
 Row<double>
 gmm_full<double>::log_p(const T1& expr, const int gaus_id, typename enable_if<((is_arma_type<T1>::value) && (resolves_to_colvector<T1>::value == false))>::result* junk2) const
 {
 arma_extra_debug_sigprint();
 arma_ignore(junk2);

 if(is_subview<T1>::value)
 {
 const subview<double>& X = reinterpret_cast< const subview<double>& >(expr);

 return internal_vec_log_p(X, gaus_id);
 }
 else
 {
 const unwrap<T1>   tmp(expr);
 const Mat<double>& X = tmp.M;

 return internal_vec_log_p(X, gaus_id);
 }
 }



 template<typename double>
 template<typename T1>
 inline
 double
 gmm_full<double>::sum_log_p(const Base<double,T1>& expr) const
 {
 arma_extra_debug_sigprint();

 if(is_subview<T1>::value)
 {
 const subview<double>& X = reinterpret_cast< const subview<double>& >( expr.get_ref() );

 return internal_sum_log_p(X);
 }
 else
 {
 const unwrap<T1>   tmp(expr.get_ref());
 const Mat<double>& X = tmp.M;

 return internal_sum_log_p(X);
 }
 }



 template<typename double>
 template<typename T1>
 inline
 double
 gmm_full<double>::sum_log_p(const Base<double,T1>& expr, const int gaus_id) const
 {
 arma_extra_debug_sigprint();

 if(is_subview<T1>::value)
 {
 const subview<double>& X = reinterpret_cast< const subview<double>& >( expr.get_ref() );

 return internal_sum_log_p(X, gaus_id);
 }
 else
 {
 const unwrap<T1>   tmp(expr.get_ref());
 const Mat<double>& X = tmp.M;

 return internal_sum_log_p(X, gaus_id);
 }
 }



 template<typename double>
 template<typename T1>
 inline
 double
 gmm_full<double>::avg_log_p(const Base<double,T1>& expr) const
 {
 arma_extra_debug_sigprint();

 if(is_subview<T1>::value)
 {
 const subview<double>& X = reinterpret_cast< const subview<double>& >( expr.get_ref() );

 return internal_avg_log_p(X);
 }
 else
 {
 const unwrap<T1>   tmp(expr.get_ref());
 const Mat<double>& X = tmp.M;

 return internal_avg_log_p(X);
 }
 }



 template<typename double>
 template<typename T1>
 inline
 double
 gmm_full<double>::avg_log_p(const Base<double,T1>& expr, const int gaus_id) const
 {
 arma_extra_debug_sigprint();

 if(is_subview<T1>::value)
 {
 const subview<double>& X = reinterpret_cast< const subview<double>& >( expr.get_ref() );

 return internal_avg_log_p(X, gaus_id);
 }
 else
 {
 const unwrap<T1>   tmp(expr.get_ref());
 const Mat<double>& X = tmp.M;

 return internal_avg_log_p(X, gaus_id);
 }
 }



 template<typename double>
 template<typename T1>
 inline
 int
 gmm_full<double>::assign(const T1& expr, const gmm_dist_mode& dist, typename enable_if<((is_arma_type<T1>::value) && (resolves_to_colvector<T1>::value == true))>::result* junk) const
 {
 arma_extra_debug_sigprint();
 arma_ignore(junk);

 if(is_subview_col<T1>::value)
 {
 const subview_col<double>& X = reinterpret_cast< const subview_col<double>& >(expr);

 return internal_scalar_assign(X, dist);
 }
 else
 {
 const unwrap<T1>   tmp(expr);
 const Mat<double>& X = tmp.M;

 return internal_scalar_assign(X, dist);
 }
 }



 template<typename double>
 template<typename T1>
 inline
 urowvec
 gmm_full<double>::assign(const T1& expr, const gmm_dist_mode& dist, typename enable_if<((is_arma_type<T1>::value) && (resolves_to_colvector<T1>::value == false))>::result* junk) const
 {
 arma_extra_debug_sigprint();
 arma_ignore(junk);

 urowvec out;

 if(is_subview<T1>::value)
 {
 const subview<double>& X = reinterpret_cast< const subview<double>& >(expr);

 internal_vec_assign(out, X, dist);
 }
 else
 {
 const unwrap<T1>   tmp(expr);
 const Mat<double>& X = tmp.M;

 internal_vec_assign(out, X, dist);
 }

 return out;
 }



 template<typename double>
 template<typename T1>
 inline
 urowvec
 gmm_full<double>::raw_hist(const Base<double,T1>& expr, const gmm_dist_mode& dist_mode) const
 {
 arma_extra_debug_sigprint();

 const unwrap<T1>   tmp(expr.get_ref());
 const Mat<double>& X = tmp.M;

 arma_debug_check( (X.rows != means.rows), "gmm_full::raw_hist(): incompatible dimensions" );

 arma_debug_check( ((dist_mode != eucl_dist) && (dist_mode != prob_dist)), "gmm_full::raw_hist(): unsupported distance mode" );

 urowvec hist;

 internal_raw_hist(hist, X, dist_mode);

 return hist;
 }



 template<typename double>
 template<typename T1>
 inline
 Row<double>
 gmm_full<double>::norm_hist(const Base<double,T1>& expr, const gmm_dist_mode& dist_mode) const
 {
 arma_extra_debug_sigprint();

 const unwrap<T1>   tmp(expr.get_ref());
 const Mat<double>& X = tmp.M;

 arma_debug_check( (X.rows != means.rows), "gmm_full::norm_hist(): incompatible dimensions" );

 arma_debug_check( ((dist_mode != eucl_dist) && (dist_mode != prob_dist)), "gmm_full::norm_hist(): unsupported distance mode" );

 urowvec hist;

 internal_raw_hist(hist, X, dist_mode);

 const int  hist_n_elem = hist.n_elem;
 const int* hist_mem    = hist.memptr();

 double acc = double(0);
 for(int i=0; i<hist_n_elem; ++i)  { acc += double(hist_mem[i]); }

 if(acc == double(0))  { acc = double(1); }

 Row<double> out(hist_n_elem);

 double* out_mem = out.memptr();

 for(int i=0; i<hist_n_elem; ++i)  { out_mem[i] = double(hist_mem[i]) / acc; }

 return out;
 }



 template<typename double>
 template<typename T1>
 inline
 bool
 gmm_full<double>::learn
 (
 const Base<double,T1>&   data,
 const int          N_gaus,
 const gmm_dist_mode& dist_mode,
 const gmm_seed_mode& seed_mode,
 const int          km_iter,
 const int          em_iter,
 const double             var_floor,
 const bool           print_mode
 )
 {
 arma_extra_debug_sigprint();

 const bool dist_mode_ok = (dist_mode == eucl_dist) || (dist_mode == maha_dist);

 const bool seed_mode_ok = \
       (seed_mode == keep_existing)
 || (seed_mode == static_subset)
 || (seed_mode == static_spread)
 || (seed_mode == random_subset)
 || (seed_mode == random_spread);

 arma_debug_check( (dist_mode_ok == false), "gmm_full::learn(): dist_mode must be eucl_dist or maha_dist" );
 arma_debug_check( (seed_mode_ok == false), "gmm_full::learn(): unknown seed_mode"                        );
 arma_debug_check( (var_floor < double(0)    ), "gmm_full::learn(): variance floor is negative"               );

 const unwrap<T1>   tmp_X(data.get_ref());
 const Mat<double>& X = tmp_X.M;

 if(X.is_empty()          )  { arma_debug_warn("gmm_full::learn(): given matrix is empty"             ); return false; }
 if(X.is_finite() == false)  { arma_debug_warn("gmm_full::learn(): given matrix has non-finite values"); return false; }

 if(N_gaus == 0)  { reset(); return true; }

 if(dist_mode == maha_dist)
 {
 mah_aux = var(X,1,1);

 const int mah_aux_n_elem = mah_aux.n_elem;
 double*   mah_aux_mem    = mah_aux.memptr();

 for(int i=0; i < mah_aux_n_elem; ++i)
 {
 const double val = mah_aux_mem[i];

 mah_aux_mem[i] = ((val != double(0)) && arma_isfinite(val)) ? double(1) / val : double(1);
 }
 }


 // copy current model, in case of failure by k-means and/or EM

 const gmm_full<double> orig = (*this);


 // initial means

 if(seed_mode == keep_existing)
 {
 if(means.is_empty()        )  { arma_debug_warn("gmm_full::learn(): no existing means"      ); return false; }
 if(X.rows != means.rows)  { arma_debug_warn("gmm_full::learn(): dimensionality mismatch"); return false; }

 // TODO: also check for number of vectors?
 }
 else
 {
 if(X.cols < N_gaus)  { arma_debug_warn("gmm_full::learn(): number of vectors is less than number of gaussians"); return false; }

 reset(X.rows, N_gaus);

 if(print_mode)  { get_stream_err2() << "gmm_full::learn(): generating initial means\n"; get_stream_err2().flush(); }

 if(dist_mode == eucl_dist)  { generate_initial_means<1>(X, seed_mode); }
 else if(dist_mode == maha_dist)  { generate_initial_means<2>(X, seed_mode); }
 }


 // k-means

 if(km_iter > 0)
 {
 const arma_ostream_state stream_state(get_stream_err2());

 bool status = false;

 if(dist_mode == eucl_dist)  { status = km_iterate<1>(X, km_iter, print_mode); }
 else if(dist_mode == maha_dist)  { status = km_iterate<2>(X, km_iter, print_mode); }

 stream_state.restore(get_stream_err2());

 if(status == false)  { arma_debug_warn("gmm_full::learn(): k-means algorithm failed; not enough data, or too many gaussians requested"); init(orig); return false; }
 }


 // initial fcovs

 const double var_floor_actual = (double(var_floor) > double(0)) ? double(var_floor) : std::numeric_limits<double>::min();

 if(seed_mode != keep_existing)
 {
 if(print_mode)  { get_stream_err2() << "gmm_full::learn(): generating initial covariances\n"; get_stream_err2().flush(); }

 if(dist_mode == eucl_dist)  { generate_initial_params<1>(X, var_floor_actual); }
 else if(dist_mode == maha_dist)  { generate_initial_params<2>(X, var_floor_actual); }
 }


 // EM algorithm

 if(em_iter > 0)
 {
 const arma_ostream_state stream_state(get_stream_err2());

 const bool status = em_iterate(X, em_iter, var_floor_actual, print_mode);

 stream_state.restore(get_stream_err2());

 if(status == false)  { arma_debug_warn("gmm_full::learn(): EM algorithm failed"); init(orig); return false; }
 }

 mah_aux.reset();

 init_constants();

 return true;
 }



 //
 //
 //



 template<typename double>
 inline
 void
 gmm_full<double>::init(const gmm_full<double>& x)
 {
 arma_extra_debug_sigprint();

 gmm_full<double>& t = *this;

 if(&t != &x)
 {
 access::rw(t.means) = x.means;
 access::rw(t.fcovs) = x.fcovs;
 access::rw(t.hefts) = x.hefts;

 init_constants();
 }
 }



 template<typename double>
 inline
 void
 gmm_full<double>::init(const gmm_diag<double>& x)
 {
 arma_extra_debug_sigprint();

 access::rw(hefts) = x.hefts;
 access::rw(means) = x.means;

 const int N_dims = x.means.rows;
 const int N_gaus = x.means.cols;

 access::rw(fcovs).zeros(N_dims,N_dims,N_gaus);

 for(int g=0; g < N_gaus; ++g)
 {
 Mat<double>& fcov = access::rw(fcovs).slice(g);

 const double* dcov_mem = x.dcovs.colptr(g);

 for(int d=0; d < N_dims; ++d)
 {
 fcov.at(d,d) = dcov_mem[d];
 }
 }

 init_constants();
 }



 template<typename double>
 inline
 void
 gmm_full<double>::init(const int in_n_dims, const int in_n_gaus)
 {
 arma_extra_debug_sigprint();

 access::rw(means).zeros(in_n_dims, in_n_gaus);

 access::rw(fcovs).zeros(in_n_dims, in_n_dims, in_n_gaus);

 for(int g=0; g < in_n_gaus; ++g)
 {
 access::rw(fcovs).slice(g).diag().ones();
 }

 access::rw(hefts).set_size(in_n_gaus);
 access::rw(hefts).fill(double(1) / double(in_n_gaus));

 init_constants();
 }



 template<typename double>
 inline
 void
 gmm_full<double>::init_constants(const bool calc_chol)
 {
 arma_extra_debug_sigprint();

 const int N_dims = means.rows;
 const int N_gaus = means.cols;

 const double tmp = (double(N_dims)/double(2)) * std::log(double(2) * Datum<double>::pi);

 //

 inv_fcovs.copy_size(fcovs);
 log_det_etc.set_size(N_gaus);

 Mat<double> tmp_inv;

 for(int g=0; g < N_gaus; ++g)
 {
 const Mat<double>&     fcov =      fcovs.slice(g);
 Mat<double>& inv_fcov =  inv_fcovs.slice(g);

 //const bool inv_ok = auxlib::inv(tmp_inv, fcov);
 const bool inv_ok = auxlib::inv_sympd(tmp_inv, fcov);

 double log_det_val  = double(0);
 double log_det_sign = double(0);

 log_det(log_det_val, log_det_sign, fcov);

 const bool log_det_ok = ( (arma_isfinite(log_det_val)) && (log_det_sign > double(0)) );

 if(inv_ok && log_det_ok)
 {
 inv_fcov = tmp_inv;
 }
 else
 {
 inv_fcov.zeros();

 log_det_val = double(0);

 for(int d=0; d < N_dims; ++d)
 {
 const double sanitised_val = (std::max)( double(fcov.at(d,d)), double(std::numeric_limits<double>::min()) );

 inv_fcov.at(d,d) = double(1) / sanitised_val;

 log_det_val += std::log(sanitised_val);
 }
 }

 log_det_etc[g] = double(-1) * ( tmp + double(0.5) * log_det_val );
 }

 //

 double* hefts_mem = access::rw(hefts).memptr();

 for(int g=0; g < N_gaus; ++g)
 {
 hefts_mem[g] = (std::max)( hefts_mem[g], std::numeric_limits<double>::min() );
 }

 log_hefts = log(hefts);


 if(calc_chol)
 {
 chol_fcovs.copy_size(fcovs);

 Mat<double> tmp_chol;

 for(int g=0; g < N_gaus; ++g)
 {
 const Mat<double>& fcov      =      fcovs.slice(g);
 Mat<double>& chol_fcov = chol_fcovs.slice(g);

 const int chol_layout = 1;  // indicates "lower"

 const bool chol_ok = auxlib::chol(tmp_chol, fcov, chol_layout);

 if(chol_ok)
 {
 chol_fcov = tmp_chol;
 }
 else
 {
 chol_fcov.zeros();

 for(int d=0; d < N_dims; ++d)
 {
 const double sanitised_val = (std::max)( double(fcov.at(d,d)), double(std::numeric_limits<double>::min()) );

 chol_fcov.at(d,d) = std::sqrt(sanitised_val);
 }
 }
 }
 }
 }



 template<typename double>
 inline
 umat
 gmm_full<double>::internal_gen_boundaries(const int N) const
 {
 arma_extra_debug_sigprint();

 #if defined(ARMA_USE_OPENMP)
 const int n_threads_avail = int(omp_get_max_threads());
 const int n_threads       = (n_threads_avail > 0) ? ( (n_threads_avail <= N) ? n_threads_avail : 1 ) : 1;
 #else
 static const int n_threads = 1;
 #endif

 // get_stream_err2() << "gmm_full::internal_gen_boundaries(): n_threads: " << n_threads << '\n';

 umat boundaries(2, n_threads);

 if(N > 0)
 {
 const int chunk_size = N / n_threads;

 int count = 0;

 for(int t=0; t<n_threads; t++)
 {
 boundaries.at(0,t) = count;

 count += chunk_size;

 boundaries.at(1,t) = count-1;
 }

 boundaries.at(1,n_threads-1) = N - 1;
 }
 else
 {
 boundaries.zeros();
 }

 // get_stream_err2() << "gmm_full::internal_gen_boundaries(): boundaries: " << '\n' << boundaries << '\n';

 return boundaries;
 }



 template<typename double>
 inline
 double
 gmm_full<double>::internal_scalar_log_p(const double* x) const
 {
 arma_extra_debug_sigprint();

 const double* log_hefts_mem = log_hefts.mem;

 const int N_gaus = means.cols;

 if(N_gaus > 0)
 {
 double log_sum = internal_scalar_log_p(x, 0) + log_hefts_mem[0];

 for(int g=1; g < N_gaus; ++g)
 {
 const double log_val = internal_scalar_log_p(x, g) + log_hefts_mem[g];

 log_sum = log_add_exp(log_sum, log_val);
 }

 return log_sum;
 }
 else
 {
 return -Datum<double>::inf;
 }
 }



 template<typename double>
 inline
 double
 gmm_full<double>::internal_scalar_log_p(const double* x, const int g) const
 {
 arma_extra_debug_sigprint();

 const int N_dims   = means.rows;
 const double*   mean_mem = means.colptr(g);

 double outer_acc = double(0);

 const double* inv_fcov_coldata = inv_fcovs.slice(g).memptr();

 for(int i=0; i < N_dims; ++i)
 {
 double inner_acc = double(0);

 for(int j=0; j < N_dims; ++j)
 {
 inner_acc += (x[j] - mean_mem[j]) * inv_fcov_coldata[j];
 }

 inv_fcov_coldata += N_dims;

 outer_acc += inner_acc * (x[i] - mean_mem[i]);
 }

 return double(-0.5)*outer_acc + log_det_etc.mem[g];
 }



 template<typename double>
 template<typename T1>
 inline
 Row<double>
 gmm_full<double>::internal_vec_log_p(const T1& X) const
 {
 arma_extra_debug_sigprint();

 const int N_dims    = means.rows;
 const int N_samples = X.cols;

 arma_debug_check( (X.rows != N_dims), "gmm_full::log_p(): incompatible dimensions" );

 Row<double> out(N_samples);

 if(N_samples > 0)
 {
 #if defined(ARMA_USE_OPENMP)
 {
 const umat boundaries = internal_gen_boundaries(N_samples);

 const int n_threads = boundaries.cols;

 #pragma omp parallel for schedule(static)
 for(int t=0; t < n_threads; ++t)
 {
 const int start_index = boundaries.at(0,t);
 const int   end_index = boundaries.at(1,t);

 double* out_mem = out.memptr();

 for(int i=start_index; i <= end_index; ++i)
 {
 out_mem[i] = internal_scalar_log_p( X.colptr(i) );
 }
 }
 }
 #else
 {
 double* out_mem = out.memptr();

 for(int i=0; i < N_samples; ++i)
 {
 out_mem[i] = internal_scalar_log_p( X.colptr(i) );
 }
 }
 #endif
 }

 return out;
 }



 template<typename double>
 template<typename T1>
 inline
 Row<double>
 gmm_full<double>::internal_vec_log_p(const T1& X, const int gaus_id) const
 {
 arma_extra_debug_sigprint();

 const int N_dims    = means.rows;
 const int N_samples = X.cols;

 arma_debug_check( (X.rows != N_dims),       "gmm_full::log_p(): incompatible dimensions"            );
 arma_debug_check( (gaus_id  >= means.cols), "gmm_full::log_p(): specified gaussian is out of range" );

 Row<double> out(N_samples);

 if(N_samples > 0)
 {
 #if defined(ARMA_USE_OPENMP)
 {
 const umat boundaries = internal_gen_boundaries(N_samples);

 const int n_threads = boundaries.cols;

 #pragma omp parallel for schedule(static)
 for(int t=0; t < n_threads; ++t)
 {
 const int start_index = boundaries.at(0,t);
 const int   end_index = boundaries.at(1,t);

 double* out_mem = out.memptr();

 for(int i=start_index; i <= end_index; ++i)
 {
 out_mem[i] = internal_scalar_log_p( X.colptr(i), gaus_id );
 }
 }
 }
 #else
 {
 double* out_mem = out.memptr();

 for(int i=0; i < N_samples; ++i)
 {
 out_mem[i] = internal_scalar_log_p( X.colptr(i), gaus_id );
 }
 }
 #endif
 }

 return out;
 }



 template<typename double>
 template<typename T1>
 inline
 double
 gmm_full<double>::internal_sum_log_p(const T1& X) const
 {
 arma_extra_debug_sigprint();

 arma_debug_check( (X.rows != means.rows), "gmm_full::sum_log_p(): incompatible dimensions" );

 const int N = X.cols;

 if(N == 0)  { return (-Datum<double>::inf); }


 #if defined(ARMA_USE_OPENMP)
 {
 const umat boundaries = internal_gen_boundaries(N);

 const int n_threads = boundaries.cols;

 Col<double> t_accs(n_threads, fill::zeros);

 #pragma omp parallel for schedule(static)
 for(int t=0; t < n_threads; ++t)
 {
 const int start_index = boundaries.at(0,t);
 const int   end_index = boundaries.at(1,t);

 double t_acc = double(0);

 for(int i=start_index; i <= end_index; ++i)
 {
 t_acc += internal_scalar_log_p( X.colptr(i) );
 }

 t_accs[t] = t_acc;
 }

 return double(accu(t_accs));
 }
 #else
 {
 double acc = double(0);

 for(int i=0; i<N; ++i)
 {
 acc += internal_scalar_log_p( X.colptr(i) );
 }

 return acc;
 }
 #endif
 }



 template<typename double>
 template<typename T1>
 inline
 double
 gmm_full<double>::internal_sum_log_p(const T1& X, const int gaus_id) const
 {
 arma_extra_debug_sigprint();

 arma_debug_check( (X.rows != means.rows), "gmm_full::sum_log_p(): incompatible dimensions"            );
 arma_debug_check( (gaus_id  >= means.cols), "gmm_full::sum_log_p(): specified gaussian is out of range" );

 const int N = X.cols;

 if(N == 0)  { return (-Datum<double>::inf); }


 #if defined(ARMA_USE_OPENMP)
 {
 const umat boundaries = internal_gen_boundaries(N);

 const int n_threads = boundaries.cols;

 Col<double> t_accs(n_threads, fill::zeros);

 #pragma omp parallel for schedule(static)
 for(int t=0; t < n_threads; ++t)
 {
 const int start_index = boundaries.at(0,t);
 const int   end_index = boundaries.at(1,t);

 double t_acc = double(0);

 for(int i=start_index; i <= end_index; ++i)
 {
 t_acc += internal_scalar_log_p( X.colptr(i), gaus_id );
 }

 t_accs[t] = t_acc;
 }

 return double(accu(t_accs));
 }
 #else
 {
 double acc = double(0);

 for(int i=0; i<N; ++i)
 {
 acc += internal_scalar_log_p( X.colptr(i), gaus_id );
 }

 return acc;
 }
 #endif
 }



 template<typename double>
 template<typename T1>
 inline
 double
 gmm_full<double>::internal_avg_log_p(const T1& X) const
 {
 arma_extra_debug_sigprint();

 const int N_dims    = means.rows;
 const int N_samples = X.cols;

 arma_debug_check( (X.rows != N_dims), "gmm_full::avg_log_p(): incompatible dimensions" );

 if(N_samples == 0)  { return (-Datum<double>::inf); }


 #if defined(ARMA_USE_OPENMP)
 {
 const umat boundaries = internal_gen_boundaries(N_samples);

 const int n_threads = boundaries.cols;

 field< running_mean_scalar<double> > t_running_means(n_threads);


 #pragma omp parallel for schedule(static)
 for(int t=0; t < n_threads; ++t)
 {
 const int start_index = boundaries.at(0,t);
 const int   end_index = boundaries.at(1,t);

 running_mean_scalar<double>& current_running_mean = t_running_means[t];

 for(int i=start_index; i <= end_index; ++i)
 {
 current_running_mean( internal_scalar_log_p( X.colptr(i) ) );
 }
 }


 double avg = double(0);

 for(int t=0; t < n_threads; ++t)
 {
 running_mean_scalar<double>& current_running_mean = t_running_means[t];

 const double w = double(current_running_mean.count()) / double(N_samples);

 avg += w * current_running_mean.mean();
 }

 return avg;
 }
 #else
 {
 running_mean_scalar<double> running_mean;

 for(int i=0; i < N_samples; ++i)
 {
 running_mean( internal_scalar_log_p( X.colptr(i) ) );
 }

 return running_mean.mean();
 }
 #endif
 }



 template<typename double>
 template<typename T1>
 inline
 double
 gmm_full<double>::internal_avg_log_p(const T1& X, const int gaus_id) const
 {
 arma_extra_debug_sigprint();

 const int N_dims    = means.rows;
 const int N_samples = X.cols;

 arma_debug_check( (X.rows != N_dims),       "gmm_full::avg_log_p(): incompatible dimensions"            );
 arma_debug_check( (gaus_id  >= means.cols), "gmm_full::avg_log_p(): specified gaussian is out of range" );

 if(N_samples == 0)  { return (-Datum<double>::inf); }


 #if defined(ARMA_USE_OPENMP)
 {
 const umat boundaries = internal_gen_boundaries(N_samples);

 const int n_threads = boundaries.cols;

 field< running_mean_scalar<double> > t_running_means(n_threads);


 #pragma omp parallel for schedule(static)
 for(int t=0; t < n_threads; ++t)
 {
 const int start_index = boundaries.at(0,t);
 const int   end_index = boundaries.at(1,t);

 running_mean_scalar<double>& current_running_mean = t_running_means[t];

 for(int i=start_index; i <= end_index; ++i)
 {
 current_running_mean( internal_scalar_log_p( X.colptr(i), gaus_id) );
 }
 }


 double avg = double(0);

 for(int t=0; t < n_threads; ++t)
 {
 running_mean_scalar<double>& current_running_mean = t_running_means[t];

 const double w = double(current_running_mean.count()) / double(N_samples);

 avg += w * current_running_mean.mean();
 }

 return avg;
 }
 #else
 {
 running_mean_scalar<double> running_mean;

 for(int i=0; i<N_samples; ++i)
 {
 running_mean( internal_scalar_log_p( X.colptr(i), gaus_id ) );
 }

 return running_mean.mean();
 }
 #endif
 }



 template<typename double>
 template<typename T1>
 inline
 int
 gmm_full<double>::internal_scalar_assign(const T1& X, const gmm_dist_mode& dist_mode) const
 {
 arma_extra_debug_sigprint();

 const int N_dims = means.rows;
 const int N_gaus = means.cols;

 arma_debug_check( (X.rows != N_dims), "gmm_full::assign(): incompatible dimensions" );
 arma_debug_check( (N_gaus == 0),        "gmm_full::assign(): model has no means"      );

 const double* X_mem = X.colptr(0);

 if(dist_mode == eucl_dist)
 {
 double    best_dist = Datum<double>::inf;
 int best_g    = 0;

 for(int g=0; g < N_gaus; ++g)
 {
 const double tmp_dist = distance<double,1>::eval(N_dims, X_mem, means.colptr(g), X_mem);

 if(tmp_dist <= best_dist)
 {
 best_dist = tmp_dist;
 best_g    = g;
 }
 }

 return best_g;
 }
 else
 if(dist_mode == prob_dist)
 {
 const double* log_hefts_mem = log_hefts.memptr();

 double    best_p = -Datum<double>::inf;
 int best_g = 0;

 for(int g=0; g < N_gaus; ++g)
 {
 const double tmp_p = internal_scalar_log_p(X_mem, g) + log_hefts_mem[g];

 if(tmp_p >= best_p)
 {
 best_p = tmp_p;
 best_g = g;
 }
 }

 return best_g;
 }
 else
 {
 arma_debug_check(true, "gmm_full::assign(): unsupported distance mode");
 }

 return int(0);
 }



 template<typename double>
 template<typename T1>
 inline
 void
 gmm_full<double>::internal_vec_assign(urowvec& out, const T1& X, const gmm_dist_mode& dist_mode) const
 {
 arma_extra_debug_sigprint();

 const int N_dims = means.rows;
 const int N_gaus = means.cols;

 arma_debug_check( (X.rows != N_dims), "gmm_full::assign(): incompatible dimensions" );

 const int X_cols = (N_gaus > 0) ? X.cols : 0;

 out.set_size(1,X_cols);

 int* out_mem = out.memptr();

 if(dist_mode == eucl_dist)
 {
 #if defined(ARMA_USE_OPENMP)
 {
 #pragma omp parallel for schedule(static)
 for(int i=0; i<X_cols; ++i)
 {
 const double* X_colptr = X.colptr(i);

 double    best_dist = Datum<double>::inf;
 int best_g    = 0;

 for(int g=0; g<N_gaus; ++g)
 {
 const double tmp_dist = distance<double,1>::eval(N_dims, X_colptr, means.colptr(g), X_colptr);

 if(tmp_dist <= best_dist)  { best_dist = tmp_dist; best_g = g; }
 }

 out_mem[i] = best_g;
 }
 }
 #else
 {
 for(int i=0; i<X_cols; ++i)
 {
 const double* X_colptr = X.colptr(i);

 double    best_dist = Datum<double>::inf;
 int best_g    = 0;

 for(int g=0; g<N_gaus; ++g)
 {
 const double tmp_dist = distance<double,1>::eval(N_dims, X_colptr, means.colptr(g), X_colptr);

 if(tmp_dist <= best_dist)  { best_dist = tmp_dist; best_g = g; }
 }

 out_mem[i] = best_g;
 }
 }
 #endif
 }
 else
 if(dist_mode == prob_dist)
 {
 #if defined(ARMA_USE_OPENMP)
 {
 const umat boundaries = internal_gen_boundaries(X_cols);

 const int n_threads = boundaries.cols;

 const double* log_hefts_mem = log_hefts.memptr();

 #pragma omp parallel for schedule(static)
 for(int t=0; t < n_threads; ++t)
 {
 const int start_index = boundaries.at(0,t);
 const int   end_index = boundaries.at(1,t);

 for(int i=start_index; i <= end_index; ++i)
 {
 const double* X_colptr = X.colptr(i);

 double    best_p = -Datum<double>::inf;
 int best_g = 0;

 for(int g=0; g<N_gaus; ++g)
 {
 const double tmp_p = internal_scalar_log_p(X_colptr, g) + log_hefts_mem[g];

 if(tmp_p >= best_p)  { best_p = tmp_p; best_g = g; }
 }

 out_mem[i] = best_g;
 }
 }
 }
 #else
 {
 const double* log_hefts_mem = log_hefts.memptr();

 for(int i=0; i<X_cols; ++i)
 {
 const double* X_colptr = X.colptr(i);

 double    best_p = -Datum<double>::inf;
 int best_g = 0;

 for(int g=0; g<N_gaus; ++g)
 {
 const double tmp_p = internal_scalar_log_p(X_colptr, g) + log_hefts_mem[g];

 if(tmp_p >= best_p)  { best_p = tmp_p; best_g = g; }
 }

 out_mem[i] = best_g;
 }
 }
 #endif
 }
 else
 {
 arma_debug_check(true, "gmm_full::assign(): unsupported distance mode");
 }
 }




 template<typename double>
 inline
 void
 gmm_full<double>::internal_raw_hist(urowvec& hist, const Mat<double>& X, const gmm_dist_mode& dist_mode) const
 {
 arma_extra_debug_sigprint();

 const int N_dims = means.rows;
 const int N_gaus = means.cols;

 const int X_cols = X.cols;

 hist.zeros(N_gaus);

 if(N_gaus == 0)  { return; }

 #if defined(ARMA_USE_OPENMP)
 {
 const umat boundaries = internal_gen_boundaries(X_cols);

 const int n_threads = boundaries.cols;

 field<urowvec> thread_hist(n_threads);

 for(int t=0; t < n_threads; ++t)  { thread_hist(t).zeros(N_gaus); }


 if(dist_mode == eucl_dist)
 {
 #pragma omp parallel for schedule(static)
 for(int t=0; t < n_threads; ++t)
 {
 int* thread_hist_mem = thread_hist(t).memptr();

 const int start_index = boundaries.at(0,t);
 const int   end_index = boundaries.at(1,t);

 for(int i=start_index; i <= end_index; ++i)
 {
 const double* X_colptr = X.colptr(i);

 double    best_dist = Datum<double>::inf;
 int best_g    = 0;

 for(int g=0; g < N_gaus; ++g)
 {
 const double tmp_dist = distance<double,1>::eval(N_dims, X_colptr, means.colptr(g), X_colptr);

 if(tmp_dist <= best_dist)  { best_dist = tmp_dist; best_g = g; }
 }

 thread_hist_mem[best_g]++;
 }
 }
 }
 else
 if(dist_mode == prob_dist)
 {
 const double* log_hefts_mem = log_hefts.memptr();

 #pragma omp parallel for schedule(static)
 for(int t=0; t < n_threads; ++t)
 {
 int* thread_hist_mem = thread_hist(t).memptr();

 const int start_index = boundaries.at(0,t);
 const int   end_index = boundaries.at(1,t);

 for(int i=start_index; i <= end_index; ++i)
 {
 const double* X_colptr = X.colptr(i);

 double    best_p = -Datum<double>::inf;
 int best_g = 0;

 for(int g=0; g < N_gaus; ++g)
 {
 const double tmp_p = internal_scalar_log_p(X_colptr, g) + log_hefts_mem[g];

 if(tmp_p >= best_p)  { best_p = tmp_p; best_g = g; }
 }

 thread_hist_mem[best_g]++;
 }
 }
 }

 // reduction
 for(int t=0; t < n_threads; ++t)
 {
 hist += thread_hist(t);
 }
 }
 #else
 {
 int* hist_mem = hist.memptr();

 if(dist_mode == eucl_dist)
 {
 for(int i=0; i<X_cols; ++i)
 {
 const double* X_colptr = X.colptr(i);

 double    best_dist = Datum<double>::inf;
 int best_g    = 0;

 for(int g=0; g < N_gaus; ++g)
 {
 const double tmp_dist = distance<double,1>::eval(N_dims, X_colptr, means.colptr(g), X_colptr);

 if(tmp_dist <= best_dist)  { best_dist = tmp_dist; best_g = g; }
 }

 hist_mem[best_g]++;
 }
 }
 else
 if(dist_mode == prob_dist)
 {
 const double* log_hefts_mem = log_hefts.memptr();

 for(int i=0; i<X_cols; ++i)
 {
 const double* X_colptr = X.colptr(i);

 double    best_p = -Datum<double>::inf;
 int best_g = 0;

 for(int g=0; g < N_gaus; ++g)
 {
 const double tmp_p = internal_scalar_log_p(X_colptr, g) + log_hefts_mem[g];

 if(tmp_p >= best_p)  { best_p = tmp_p; best_g = g; }
 }

 hist_mem[best_g]++;
 }
 }
 }
 #endif
 }



 template<typename double>
 template<int dist_id>
 inline
 void
 gmm_full<double>::generate_initial_means(const Mat<double>& X, const gmm_seed_mode& seed_mode)
 {
 arma_extra_debug_sigprint();

 const int N_dims = means.rows;
 const int N_gaus = means.cols;

 if( (seed_mode == static_subset) || (seed_mode == random_subset) )
 {
 uvec initial_indices;

 if(seed_mode == static_subset)  { initial_indices = linspace<uvec>(0, X.cols-1, N_gaus);                   }
 else if(seed_mode == random_subset)  { initial_indices = uvec(sort_index(randu<vec>(X.cols))).rows(0,N_gaus-1); }

 // not using randi() here as on some primitive systems it produces vectors with non-unique values

 // initial_indices.print("initial_indices:");

 access::rw(means) = X.cols(initial_indices);
 }
 else
 if( (seed_mode == static_spread) || (seed_mode == random_spread) )
 {
 // going through all of the samples can be extremely time consuming;
 // instead, if there are enough samples, randomly choose samples with probability 0.1

 const bool  use_sampling = ((X.cols/int(100)) > N_gaus);
 const int step         = (use_sampling) ? int(10) : int(1);

 int start_index = 0;

 if(seed_mode == static_spread)  { start_index = X.cols / 2;                                         }
 else if(seed_mode == random_spread)  { start_index = as_scalar(randi<uvec>(1, distr_param(0,X.cols-1))); }

 access::rw(means).col(0) = X.unsafe_col(start_index);

 const double* mah_aux_mem = mah_aux.memptr();

 running_stat<double> rs;

 for(int g=1; g < N_gaus; ++g)
 {
 double    max_dist = double(0);
 int best_i   = int(0);
 int start_i  = int(0);

 if(use_sampling)
 {
 int start_i_proposed = int(0);

 if(seed_mode == static_spread)  { start_i_proposed = g % int(10);                               }
 if(seed_mode == random_spread)  { start_i_proposed = as_scalar(randi<uvec>(1, distr_param(0,9))); }

 if(start_i_proposed < X.cols)  { start_i = start_i_proposed; }
 }


 for(int i=start_i; i < X.cols; i += step)
 {
 rs.reset();

 const double* X_colptr = X.colptr(i);

 bool ignore_i = false;

 // find the average distance between sample i and the means so far
 for(int h = 0; h < g; ++h)
 {
 const double dist = distance<double,dist_id>::eval(N_dims, X_colptr, means.colptr(h), mah_aux_mem);

 // ignore sample already selected as a mean
 if(dist == double(0))  { ignore_i = true; break; }
 else               { rs(dist);               }
 }

 if( (rs.mean() >= max_dist) && (ignore_i == false))
 {
 max_dist = double(rs.mean()); best_i = i;
 }
 }

 // set the mean to the sample that is the furthest away from the means so far
 access::rw(means).col(g) = X.unsafe_col(best_i);
 }
 }

 // get_stream_err2() << "generate_initial_means():" << '\n';
 // means.print();
 }



 template<typename double>
 template<int dist_id>
 inline
 void
 gmm_full<double>::generate_initial_params(const Mat<double>& X, const double var_floor)
 {
 arma_extra_debug_sigprint();

 const int N_dims = means.rows;
 const int N_gaus = means.cols;

 const double* mah_aux_mem = mah_aux.memptr();

 const int X_cols = X.cols;

 if(X_cols == 0)  { return; }

 // as the covariances are calculated via accumulators,
 // the means also need to be calculated via accumulators to ensure numerical consistency

 Mat<double> acc_means(N_dims, N_gaus, fill::zeros);
 Mat<double> acc_dcovs(N_dims, N_gaus, fill::zeros);

 Row<int> acc_hefts(N_gaus, fill::zeros);

 int* acc_hefts_mem = acc_hefts.memptr();

 #if defined(ARMA_USE_OPENMP)
 {
 const umat boundaries = internal_gen_boundaries(X_cols);

 const int n_threads = boundaries.cols;

 field< Mat<double>    > t_acc_means(n_threads);
 field< Mat<double>    > t_acc_dcovs(n_threads);
 field< Row<int> > t_acc_hefts(n_threads);

 for(int t=0; t < n_threads; ++t)
 {
 t_acc_means(t).zeros(N_dims, N_gaus);
 t_acc_dcovs(t).zeros(N_dims, N_gaus);
 t_acc_hefts(t).zeros(N_gaus);
 }

 #pragma omp parallel for schedule(static)
 for(int t=0; t < n_threads; ++t)
 {
 int* t_acc_hefts_mem = t_acc_hefts(t).memptr();

 const int start_index = boundaries.at(0,t);
 const int   end_index = boundaries.at(1,t);

 for(int i=start_index; i <= end_index; ++i)
 {
 const double* X_colptr = X.colptr(i);

 double     min_dist = Datum<double>::inf;
 int  best_g   = 0;

 for(int g=0; g<N_gaus; ++g)
 {
 const double dist = distance<double,dist_id>::eval(N_dims, X_colptr, means.colptr(g), mah_aux_mem);

 if(dist < min_dist)  { min_dist = dist;  best_g = g; }
 }

 double* t_acc_mean = t_acc_means(t).colptr(best_g);
 double* t_acc_dcov = t_acc_dcovs(t).colptr(best_g);

 for(int d=0; d<N_dims; ++d)
 {
 const double x_d = X_colptr[d];

 t_acc_mean[d] += x_d;
 t_acc_dcov[d] += x_d*x_d;
 }

 t_acc_hefts_mem[best_g]++;
 }
 }

 // reduction
 acc_means = t_acc_means(0);
 acc_dcovs = t_acc_dcovs(0);
 acc_hefts = t_acc_hefts(0);

 for(int t=1; t < n_threads; ++t)
 {
 acc_means += t_acc_means(t);
 acc_dcovs += t_acc_dcovs(t);
 acc_hefts += t_acc_hefts(t);
 }
 }
 #else
 {
 for(int i=0; i<X_cols; ++i)
 {
 const double* X_colptr = X.colptr(i);

 double     min_dist = Datum<double>::inf;
 int  best_g   = 0;

 for(int g=0; g<N_gaus; ++g)
 {
 const double dist = distance<double,dist_id>::eval(N_dims, X_colptr, means.colptr(g), mah_aux_mem);

 if(dist < min_dist)  { min_dist = dist;  best_g = g; }
 }

 double* acc_mean = acc_means.colptr(best_g);
 double* acc_dcov = acc_dcovs.colptr(best_g);

 for(int d=0; d<N_dims; ++d)
 {
 const double x_d = X_colptr[d];

 acc_mean[d] += x_d;
 acc_dcov[d] += x_d*x_d;
 }

 acc_hefts_mem[best_g]++;
 }
 }
 #endif

 double* hefts_mem = access::rw(hefts).memptr();

 for(int g=0; g<N_gaus; ++g)
 {
 const double*   acc_mean = acc_means.colptr(g);
 const double*   acc_dcov = acc_dcovs.colptr(g);
 const int acc_heft = acc_hefts_mem[g];

 double* mean = access::rw(means).colptr(g);

 Mat<double>& fcov = access::rw(fcovs).slice(g);
 fcov.zeros();

 for(int d=0; d<N_dims; ++d)
 {
 const double tmp = acc_mean[d] / double(acc_heft);

 mean[d]      = (acc_heft >= 1) ? tmp : double(0);
 fcov.at(d,d) = (acc_heft >= 2) ? double((acc_dcov[d] / double(acc_heft)) - (tmp*tmp)) : double(var_floor);
 }

 hefts_mem[g] = double(acc_heft) / double(X_cols);
 }

 em_fix_params(var_floor);
 }



 //! multi-threaded implementation of k-means, inspired by MapReduce
 template<typename double>
 template<int dist_id>
 inline
 bool
 gmm_full<double>::km_iterate(const Mat<double>& X, const int max_iter, const bool verbose)
 {
 arma_extra_debug_sigprint();

 if(verbose)
 {
 get_stream_err2().unsetf(ios::showbase);
 get_stream_err2().unsetf(ios::uppercase);
 get_stream_err2().unsetf(ios::showpos);
 get_stream_err2().unsetf(ios::scientific);

 get_stream_err2().setf(ios::right);
 get_stream_err2().setf(ios::fixed);
 }

 const int X_cols = X.cols;

 if(X_cols == 0)  { return true; }

 const int N_dims = means.rows;
 const int N_gaus = means.cols;

 const double* mah_aux_mem = mah_aux.memptr();

 Mat<double>    acc_means(N_dims, N_gaus, fill::zeros);
 Row<int> acc_hefts(N_gaus, fill::zeros);
 Row<int> last_indx(N_gaus, fill::zeros);

 Mat<double> new_means = means;
 Mat<double> old_means = means;

 running_mean_scalar<double> rs_delta;

 #if defined(ARMA_USE_OPENMP)
 const umat boundaries = internal_gen_boundaries(X_cols);
 const int n_threads = boundaries.cols;

 field< Mat<double>    > t_acc_means(n_threads);
 field< Row<int> > t_acc_hefts(n_threads);
 field< Row<int> > t_last_indx(n_threads);
 #else
 const int n_threads = 1;
 #endif

 if(verbose)  { get_stream_err2() << "gmm_full::learn(): k-means: n_threads: " << n_threads << '\n';  get_stream_err2().flush(); }

 for(int iter=1; iter <= max_iter; ++iter)
 {
 #if defined(ARMA_USE_OPENMP)
 {
 for(int t=0; t < n_threads; ++t)
 {
 t_acc_means(t).zeros(N_dims, N_gaus);
 t_acc_hefts(t).zeros(N_gaus);
 t_last_indx(t).zeros(N_gaus);
 }

 #pragma omp parallel for schedule(static)
 for(int t=0; t < n_threads; ++t)
 {
 Mat<double>& t_acc_means_t   = t_acc_means(t);
 int*   t_acc_hefts_mem = t_acc_hefts(t).memptr();
 int*   t_last_indx_mem = t_last_indx(t).memptr();

 const int start_index = boundaries.at(0,t);
 const int   end_index = boundaries.at(1,t);

 for(int i=start_index; i <= end_index; ++i)
 {
 const double* X_colptr = X.colptr(i);

 double     min_dist = Datum<double>::inf;
 int  best_g   = 0;

 for(int g=0; g<N_gaus; ++g)
 {
 const double dist = distance<double,dist_id>::eval(N_dims, X_colptr, old_means.colptr(g), mah_aux_mem);

 if(dist < min_dist)  { min_dist = dist;  best_g = g; }
 }

 double* t_acc_mean = t_acc_means_t.colptr(best_g);

 for(int d=0; d<N_dims; ++d)  { t_acc_mean[d] += X_colptr[d]; }

 t_acc_hefts_mem[best_g]++;
 t_last_indx_mem[best_g] = i;
 }
 }

 // reduction

 acc_means = t_acc_means(0);
 acc_hefts = t_acc_hefts(0);

 for(int t=1; t < n_threads; ++t)
 {
 acc_means += t_acc_means(t);
 acc_hefts += t_acc_hefts(t);
 }

 for(int g=0; g < N_gaus;    ++g)
 for(int t=0; t < n_threads; ++t)
 {
 if( t_acc_hefts(t)(g) >= 1 )  { last_indx(g) = t_last_indx(t)(g); }
 }
 }
 #else
 {
 int* acc_hefts_mem = acc_hefts.memptr();
 int* last_indx_mem = last_indx.memptr();

 for(int i=0; i < X_cols; ++i)
 {
 const double* X_colptr = X.colptr(i);

 double     min_dist = Datum<double>::inf;
 int  best_g   = 0;

 for(int g=0; g<N_gaus; ++g)
 {
 const double dist = distance<double,dist_id>::eval(N_dims, X_colptr, old_means.colptr(g), mah_aux_mem);

 if(dist < min_dist)  { min_dist = dist;  best_g = g; }
 }

 double* acc_mean = acc_means.colptr(best_g);

 for(int d=0; d<N_dims; ++d)  { acc_mean[d] += X_colptr[d]; }

 acc_hefts_mem[best_g]++;
 last_indx_mem[best_g] = i;
 }
 }

 #endif

 // generate new means

 int* acc_hefts_mem = acc_hefts.memptr();

 for(int g=0; g < N_gaus; ++g)
 {
 const double*   acc_mean = acc_means.colptr(g);
 const int acc_heft = acc_hefts_mem[g];

 double* new_mean = access::rw(new_means).colptr(g);

 for(int d=0; d<N_dims; ++d)
 {
 new_mean[d] = (acc_heft >= 1) ? (acc_mean[d] / double(acc_heft)) : double(0);
 }
 }


 // heuristics to resurrect dead means

 const uvec dead_gs = find(acc_hefts == int(0));

 if(dead_gs.n_elem > 0)
 {
 if(verbose)  { get_stream_err2() << "gmm_full::learn(): k-means: recovering from dead means\n"; get_stream_err2().flush(); }

 int* last_indx_mem = last_indx.memptr();

 const uvec live_gs = sort( find(acc_hefts >= int(2)), "descend" );

 if(live_gs.n_elem == 0)  { return false; }

 int live_gs_count  = 0;

 for(int dead_gs_count = 0; dead_gs_count < dead_gs.n_elem; ++dead_gs_count)
 {
 const int dead_g_id = dead_gs(dead_gs_count);

 int proposed_i = 0;

 if(live_gs_count < live_gs.n_elem)
 {
 const int live_g_id = live_gs(live_gs_count);  ++live_gs_count;

 if(live_g_id == dead_g_id)  { return false; }

 // recover by using a sample from a known good mean
 proposed_i = last_indx_mem[live_g_id];
 }
 else
 {
 // recover by using a randomly seleced sample (last resort)
 proposed_i = as_scalar(randi<uvec>(1, distr_param(0,X_cols-1)));
 }

 if(proposed_i >= X_cols)  { return false; }

 new_means.col(dead_g_id) = X.col(proposed_i);
 }
 }

 rs_delta.reset();

 for(int g=0; g < N_gaus; ++g)
 {
 rs_delta( distance<double,dist_id>::eval(N_dims, old_means.colptr(g), new_means.colptr(g), mah_aux_mem) );
 }

 if(verbose)
 {
 get_stream_err2() << "gmm_full::learn(): k-means: iteration: ";
 get_stream_err2().unsetf(ios::scientific);
 get_stream_err2().setf(ios::fixed);
 get_stream_err2().width(std::streamsize(4));
 get_stream_err2() << iter;
 get_stream_err2() << "   delta: ";
 get_stream_err2().unsetf(ios::fixed);
 //get_stream_err2().setf(ios::scientific);
 get_stream_err2() << rs_delta.mean() << '\n';
 get_stream_err2().flush();
 }

 arma::swap(old_means, new_means);

 if(rs_delta.mean() <= Datum<double>::eps)  { break; }
 }

 access::rw(means) = old_means;

 if(means.is_finite() == false)  { return false; }

 return true;
 }



 //! multi-threaded implementation of Expectation-Maximisation, inspired by MapReduce
 template<typename double>
 inline
 bool
 gmm_full<double>::em_iterate(const Mat<double>& X, const int max_iter, const double var_floor, const bool verbose)
 {
 arma_extra_debug_sigprint();

 const int N_dims = means.rows;
 const int N_gaus = means.cols;

 if(verbose)
 {
 get_stream_err2().unsetf(ios::showbase);
 get_stream_err2().unsetf(ios::uppercase);
 get_stream_err2().unsetf(ios::showpos);
 get_stream_err2().unsetf(ios::scientific);

 get_stream_err2().setf(ios::right);
 get_stream_err2().setf(ios::fixed);
 }

 const umat boundaries = internal_gen_boundaries(X.cols);

 const int n_threads = boundaries.cols;

 field<  Mat<double> > t_acc_means(n_threads);
 field< Cube<double> > t_acc_fcovs(n_threads);

 field< Col<double> > t_acc_norm_lhoods(n_threads);
 field< Col<double> > t_gaus_log_lhoods(n_threads);

 Col<double>          t_progress_log_lhood(n_threads);

 for(int t=0; t<n_threads; t++)
 {
 t_acc_means[t].set_size(N_dims, N_gaus);
 t_acc_fcovs[t].set_size(N_dims, N_dims, N_gaus);

 t_acc_norm_lhoods[t].set_size(N_gaus);
 t_gaus_log_lhoods[t].set_size(N_gaus);
 }


 if(verbose)
 {
 get_stream_err2() << "gmm_full::learn(): EM: n_threads: " << n_threads  << '\n';
 }

 double old_avg_log_p = -Datum<double>::inf;

 const bool calc_chol = false;

 for(int iter=1; iter <= max_iter; ++iter)
 {
 init_constants(calc_chol);

 em_update_params(X, boundaries, t_acc_means, t_acc_fcovs, t_acc_norm_lhoods, t_gaus_log_lhoods, t_progress_log_lhood, var_floor);

 em_fix_params(var_floor);

 const double new_avg_log_p = accu(t_progress_log_lhood) / double(t_progress_log_lhood.n_elem);

 if(verbose)
 {
 get_stream_err2() << "gmm_full::learn(): EM: iteration: ";
 get_stream_err2().unsetf(ios::scientific);
 get_stream_err2().setf(ios::fixed);
 get_stream_err2().width(std::streamsize(4));
 get_stream_err2() << iter;
 get_stream_err2() << "   avg_log_p: ";
 get_stream_err2().unsetf(ios::fixed);
 //get_stream_err2().setf(ios::scientific);
 get_stream_err2() << new_avg_log_p << '\n';
 get_stream_err2().flush();
 }

 if(arma_isfinite(new_avg_log_p) == false)  { return false; }

 if(std::abs(old_avg_log_p - new_avg_log_p) <= Datum<double>::eps)  { break; }


 old_avg_log_p = new_avg_log_p;
 }


 for(int g=0; g < N_gaus; ++g)
 {
 const Mat<double>& fcov = fcovs.slice(g);

 if(any(vectorise(fcov.diag()) <= double(0)))  { return false; }
 }

 if(means.is_finite() == false)  { return false; }
 if(fcovs.is_finite() == false)  { return false; }
 if(hefts.is_finite() == false)  { return false; }

 return true;
 }




 template<typename double>
 inline
 void
 gmm_full<double>::em_update_params
 (
 const Mat<double>&           X,
 const umat&              boundaries,
 field<  Mat<double> >& t_acc_means,
 field< Cube<double> >& t_acc_fcovs,
 field<  Col<double> >& t_acc_norm_lhoods,
 field<  Col<double> >& t_gaus_log_lhoods,
 Col<double>&           t_progress_log_lhood,
 const double                 var_floor
 )
 {
 arma_extra_debug_sigprint();

 const int n_threads = boundaries.cols;


 // em_generate_acc() is the "map" operation, which produces partial accumulators for means, diagonal covariances and hefts

 #if defined(ARMA_USE_OPENMP)
 {
 #pragma omp parallel for schedule(static)
 for(int t=0; t<n_threads; t++)
 {
 Mat<double>& acc_means          = t_acc_means[t];
 Cube<double>& acc_fcovs          = t_acc_fcovs[t];
 Col<double>& acc_norm_lhoods    = t_acc_norm_lhoods[t];
 Col<double>& gaus_log_lhoods    = t_gaus_log_lhoods[t];
 double&      progress_log_lhood = t_progress_log_lhood[t];

 em_generate_acc(X, boundaries.at(0,t), boundaries.at(1,t), acc_means, acc_fcovs, acc_norm_lhoods, gaus_log_lhoods, progress_log_lhood);
 }
 }
 #else
 {
 em_generate_acc(X, boundaries.at(0,0), boundaries.at(1,0), t_acc_means[0], t_acc_fcovs[0], t_acc_norm_lhoods[0], t_gaus_log_lhoods[0], t_progress_log_lhood[0]);
 }
 #endif

 const int N_dims = means.rows;
 const int N_gaus = means.cols;

 Mat<double>& final_acc_means = t_acc_means[0];
 Cube<double>& final_acc_fcovs = t_acc_fcovs[0];

 Col<double>& final_acc_norm_lhoods = t_acc_norm_lhoods[0];


 // the "reduce" operation, which combines the partial accumulators produced by the separate threads

 for(int t=1; t<n_threads; t++)
 {
 final_acc_means += t_acc_means[t];
 final_acc_fcovs += t_acc_fcovs[t];

 final_acc_norm_lhoods += t_acc_norm_lhoods[t];
 }


 double* hefts_mem = access::rw(hefts).memptr();

 Mat<double> mean_outer(N_dims, N_dims);


 //// update each component without sanity checking
 //for(int g=0; g < N_gaus; ++g)
 //  {
 //  const double acc_norm_lhood = (std::max)( final_acc_norm_lhoods[g], std::numeric_limits<double>::min() );
 //
 //    hefts_mem[g] = acc_norm_lhood / double(X.cols);
 //
 //    double*     mean_mem = access::rw(means).colptr(g);
 //    double* acc_mean_mem = final_acc_means.colptr(g);
 //
 //    for(int d=0; d < N_dims; ++d)
 //      {
 //      mean_mem[d] = acc_mean_mem[d] / acc_norm_lhood;
 //      }
 //
 //    const Col<double> mean(mean_mem, N_dims, false, true);
 //
 //    mean_outer = mean * mean.t();
 //
 //    Mat<double>&     fcov = access::rw(fcovs).slice(g);
 //    Mat<double>& acc_fcov = final_acc_fcovs.slice(g);
 //
 //    fcov = acc_fcov / acc_norm_lhood - mean_outer;
 //    }


 // conditionally update each component; if only a subset of the hefts was updated, em_fix_params() will sanitise them
 for(int g=0; g < N_gaus; ++g)
 {
 const double acc_norm_lhood = (std::max)( final_acc_norm_lhoods[g], std::numeric_limits<double>::min() );

 if(arma_isfinite(acc_norm_lhood) == false)  { continue; }

 double* acc_mean_mem = final_acc_means.colptr(g);

 for(int d=0; d < N_dims; ++d)
 {
 acc_mean_mem[d] /= acc_norm_lhood;
 }

 const Col<double> new_mean(acc_mean_mem, N_dims, false, true);

 mean_outer = new_mean * new_mean.t();

 Mat<double>& acc_fcov = final_acc_fcovs.slice(g);

 acc_fcov /= acc_norm_lhood;
 acc_fcov -= mean_outer;

 for(int d=0; d < N_dims; ++d)
 {
 double& val = acc_fcov.at(d,d);

 if(val < var_floor)  { val = var_floor; }
 }

 if(acc_fcov.is_finite() == false)  { continue; }

 double log_det_val  = double(0);
 double log_det_sign = double(0);

 log_det(log_det_val, log_det_sign, acc_fcov);

 const bool log_det_ok = ( (arma_isfinite(log_det_val)) && (log_det_sign > double(0)) );

 const bool inv_ok = (log_det_ok) ? bool(auxlib::inv_sympd(mean_outer, acc_fcov)) : bool(false);  // mean_outer is used as a junk matrix

 if(log_det_ok && inv_ok)
 {
 hefts_mem[g] = acc_norm_lhood / double(X.cols);

 double* mean_mem = access::rw(means).colptr(g);

 for(int d=0; d < N_dims; ++d)
 {
 mean_mem[d] = acc_mean_mem[d];
 }

 Mat<double>& fcov = access::rw(fcovs).slice(g);

 fcov = acc_fcov;
 }
 }
 }



 template<typename double>
 inline
 void
 gmm_full<double>::em_generate_acc
 (
 const  Mat<double>& X,
 const  int    start_index,
 const  int      end_index,
 Mat<double>& acc_means,
 Cube<double>& acc_fcovs,
 Col<double>& acc_norm_lhoods,
 Col<double>& gaus_log_lhoods,
 double&      progress_log_lhood
 )
 const
 {
 arma_extra_debug_sigprint();

 progress_log_lhood = double(0);

 acc_means.zeros();
 acc_fcovs.zeros();

 acc_norm_lhoods.zeros();
 gaus_log_lhoods.zeros();

 const int N_dims = means.rows;
 const int N_gaus = means.cols;

 const double* log_hefts_mem       = log_hefts.memptr();
 double* gaus_log_lhoods_mem = gaus_log_lhoods.memptr();


 for(int i=start_index; i <= end_index; i++)
 {
 const double* x = X.colptr(i);

 for(int g=0; g < N_gaus; ++g)
 {
 gaus_log_lhoods_mem[g] = internal_scalar_log_p(x, g) + log_hefts_mem[g];
 }

 double log_lhood_sum = gaus_log_lhoods_mem[0];

 for(int g=1; g < N_gaus; ++g)
 {
 log_lhood_sum = log_add_exp(log_lhood_sum, gaus_log_lhoods_mem[g]);
 }

 progress_log_lhood += log_lhood_sum;

 for(int g=0; g < N_gaus; ++g)
 {
 const double norm_lhood = std::exp(gaus_log_lhoods_mem[g] - log_lhood_sum);

 acc_norm_lhoods[g] += norm_lhood;

 double* acc_mean_mem = acc_means.colptr(g);

 for(int d=0; d < N_dims; ++d)
 {
 acc_mean_mem[d] += x[d] * norm_lhood;
 }

 Mat<double>& acc_fcov = access::rw(acc_fcovs).slice(g);

 // specialised version of acc_fcov += norm_lhood * (xx * xx.t());

 for(int d=0; d < N_dims; ++d)
 {
 const int dp1 = d+1;

 const double xd = x[d];

 double* acc_fcov_col_d = acc_fcov.colptr(d) + d;
 double* acc_fcov_row_d = &(acc_fcov.at(d,dp1));

 (*acc_fcov_col_d) += norm_lhood * (xd * xd);  acc_fcov_col_d++;

 for(int e=dp1; e < N_dims; ++e)
 {
 const double val = norm_lhood * (xd * x[e]);

 (*acc_fcov_col_d) += val;  acc_fcov_col_d++;
 (*acc_fcov_row_d) += val;  acc_fcov_row_d += N_dims;
 }
 }
 }
 }

 progress_log_lhood /= double((end_index - start_index) + 1);
 }



 template<typename double>
 inline
 void
 gmm_full<double>::em_fix_params(const double var_floor)
 {
 arma_extra_debug_sigprint();

 const int N_dims = means.rows;
 const int N_gaus = means.cols;

 const double var_ceiling = std::numeric_limits<double>::max();

 for(int g=0; g < N_gaus; ++g)
 {
 Mat<double>& fcov = access::rw(fcovs).slice(g);

 for(int d=0; d < N_dims; ++d)
 {
 double& var_val = fcov.at(d,d);

 if(var_val < var_floor  )  { var_val = var_floor;   }
 else if(var_val > var_ceiling)  { var_val = var_ceiling; }
 else if(arma_isnan(var_val)  )  { var_val = double(1);       }
 }
 }


 double* hefts_mem = access::rw(hefts).memptr();

 for(int g1=0; g1 < N_gaus; ++g1)
 {
 if(hefts_mem[g1] > double(0))
 {
 const double* means_colptr_g1 = means.colptr(g1);

 for(int g2=(g1+1); g2 < N_gaus; ++g2)
 {
 if( (hefts_mem[g2] > double(0)) && (std::abs(hefts_mem[g1] - hefts_mem[g2]) <= std::numeric_limits<double>::epsilon()) )
 {
 const double dist = distance<double,1>::eval(N_dims, means_colptr_g1, means.colptr(g2), means_colptr_g1);

 if(dist == double(0)) { hefts_mem[g2] = double(0); }
 }
 }
 }
 }

 const double heft_floor   = std::numeric_limits<double>::min();
 const double heft_initial = double(1) / double(N_gaus);

 for(int i=0; i < N_gaus; ++i)
 {
 double& heft_val = hefts_mem[i];

 if(heft_val < heft_floor)  { heft_val = heft_floor;   }
 else if(heft_val > double(1)     )  { heft_val = double(1);        }
 else if(arma_isnan(heft_val) )  { heft_val = heft_initial; }
 }

 const double heft_sum = accu(hefts);

 if((heft_sum < (double(1) - Datum<double>::eps)) || (heft_sum > (double(1) + Datum<double>::eps)))  { access::rw(hefts) /= heft_sum; }
 }



 } // namespace gmm_priv


 //! @}

 */
