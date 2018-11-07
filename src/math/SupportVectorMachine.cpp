/*
 * SupportVectorMachine.cpp
 *
 *  Created on: Nov 6, 2018
 *      Author: blake
 */

#include "SupportVectorMachine.h"
#include <exception>
#include <stdexcept>
#include <iostream>
namespace aly {
void SupportVectorMachine::crossValidation() {
	int i;
	int total_correct = 0;
	double total_error = 0;
	double sumv = 0, sumy = 0, sumvv = 0, sumyy = 0, sumvy = 0;
	std::vector<double> target(problem.l);
	svm_cross_validation(&problem, &param, (int) weightLabels.size(),
			target.data());
	if (param.svm_type == EPSILON_SVR || param.svm_type == NU_SVR) {
		for (i = 0; i < problem.l; i++) {
			double y = problem.y[i];
			double v = target[i];
			total_error += (v - y) * (v - y);
			sumv += v;
			sumy += y;
			sumvv += v * v;
			sumyy += y * y;
			sumvy += v * y;
		}
		std::cout << "Cross Validation Mean squared error = "
				<< total_error / problem.l << std::endl;
		std::cout << "Cross Validation Squared correlation coefficient = "
				<< ((problem.l * sumvy - sumv * sumy)
						* (problem.l * sumvy - sumv * sumy))
						/ ((problem.l * sumvv - sumv * sumv)
								* (problem.l * sumyy - sumy * sumy))
				<< std::endl;
	} else {
		for (i = 0; i < problem.l; i++)
			if (target[i] == problem.y[i])
				++total_correct;
		std::cout << "Cross Validation Accuracy = "
				<< 100.0 * total_correct / problem.l << std::endl;
	}
}
SupportVectorMachine::SupportVectorMachine() :model(nullptr) {
	param.svm_type = C_SVC;
	param.kernel_type = RBF;
	param.degree = 3;
	param.gamma = 0;	// 1/num_features
	param.coef0 = 0;
	param.nu = 0.5;
	param.cache_size = 100;
	param.C = 1;
	param.eps = 1e-3;
	param.p = 0.1;
	param.shrinking = 1;
	param.probability = 0;
	param.nr_weight = 0;
	param.weight_label = nullptr;
	param.weight = nullptr;

}
double SupportVectorMachine::evaluate(int index, double value,
		std::vector<double>& probabilities) {
	int nr_class = svm_get_nr_class(model);
	struct svm_node x = { index, value };
	if ((param.svm_type == C_SVC || param.svm_type == NU_SVC)) {
		probabilities.resize(nr_class);
		return svm_predict_probability(model, &x, probabilities.data());
	} else {
		return svm_predict(model, &x);
	}
}
double SupportVectorMachine::evaluate(int index, double value) {
	struct svm_node x = { index, value };
	return svm_predict(model, &x);

}

//add case for sparse vector
void SupportVectorMachine::train(const std::vector<std::vector<int>>& index,const std::vector<std::vector<double>>& x, const std::vector<double>& y,int cross_validation) {
	if(index.size()!=x.size()||x.size()!=y.size()||index.size()!=y.size()){
		throw std::runtime_error("SVM input size must match output size.");
	}
	size_t sz = x.size();
	problem.l = sz;
	outputs = y;
	inputs.resize(sz);
	int max_index = 0;
	if(problem.x)delete[] problem.x;
	problem.x = new struct svm_node*[sz];
	problem.y = outputs.data();
#pragma omp parallel for
	for (size_t n = 0; n < sz; n++) {
		auto indexArray=index[n];
		auto xArray=x[n];
		struct svm_node node;
		for(int k=0;k<indexArray.size();k++){
			node={indexArray[k],xArray[k]};
			inputs[n].push_back(node);
			max_index=std::max(indexArray[k],max_index);
		}
		node={-1,0};//indicate end of feature
		inputs[n].push_back(node);
		problem.x[n]=inputs[n].data();
	}
	if (param.gamma == 0 && max_index > 0) {
		param.gamma = 1.0 / max_index;
	}
	const char* error_msg = svm_check_parameter(&problem, &param);
	if (error_msg) {
		throw std::runtime_error(error_msg);
	}
	if (cross_validation > 0) {
		crossValidation();
	} else {
		model = svm_train(&problem, &param);
	}
}
SupportVectorMachine::~SupportVectorMachine() {
	if (problem.x)
		delete[] problem.x;
	if (model)
		svm_free_and_destroy_model(&model);
}

} /* namespace intel */
