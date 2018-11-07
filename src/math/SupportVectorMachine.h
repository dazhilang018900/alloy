/*
 * SupportVectorMachine.h
 *
 *  Created on: Nov 6, 2018
 *      Author: blake
 */

#ifndef SRC_MATH_SUPPORTVECTORMACHINE_H_
#define SRC_MATH_SUPPORTVECTORMACHINE_H_
#include "math/svm.h"
#include <vector>
namespace aly {
	enum class SVMType { C_SVC=::C_SVC, NU_SVC=::NU_SVC, ONE_CLASS=::ONE_CLASS, EPSILON_SVR=::EPSILON_SVR, NU_SVR=::NU_SVR };
	enum class SVMKernelType { LINEAR=::LINEAR, POLY=::POLY, RBF=::RBF, SIGMOID=::SIGMOID, PRECOMPUTED=::PRECOMPUTED };
class SupportVectorMachine {
protected:
	struct svm_parameter param;		// set by parse_command_line
	struct svm_problem problem;		// set by read_problem
	struct svm_model *model;
	std::vector<int> weightLabels;
	std::vector<double> weights;
	std::vector<std::vector<svm_node>> inputs;
	std::vector<double> outputs;
	void crossValidation();
public:
	void setType(SVMType type){
		param.svm_type=static_cast<int>(type);
	}
	void setKernelType(SVMKernelType type){
		param.kernel_type=static_cast<int>(type);
	}
	void setDegree(int k){
		param.degree=3;
	}
	void setGamma(double gam){
		param.gamma=gam;
	}
	void setCoefficient(double val){
		param.coef0=val;
	}
	void setNu(double val){
		param.nu=val;
	}
	void setCacheSize(size_t sz){
		param.cache_size=sz;
	}
	void setCost(double c){
		param.C=c;
	}
	void setConvergenceTolerance(double v){
		param.eps=v;
	}
	void setEpsilonLoss(double v){
		param.p=v;
	}
	void setProbability(bool v){
		param.probability=v?1:0;
	}
	void setShrinking(bool v){
		param.shrinking=v?1:0;
	}

	void setNumClasses(int v){
		param.nr_weight=v;
		weightLabels.resize(param.nr_weight);
		weights.resize(param.nr_weight);
		param.weight_label = weightLabels.data();
		param.weight = weights.data();
	}
	int getNumClasses() const {
		return param.nr_weight;
	}
	void setWeight(int idx,int cls, double val){
		param.weight_label[idx]=cls;
		param.weight[idx]=val;
	}

	void addWeight(int cls, double val){
		weights.push_back(val);
		weightLabels.push_back(cls);
		param.weight_label=weightLabels.data();
		param.weight=weights.data();
	}

	SupportVectorMachine();
	double evaluate(int index,double value);
	double evaluate(int index,double value,std::vector<double>& probabilities);
	void train(const std::vector<std::vector<int>>& index,const std::vector<std::vector<double>>& x,const std::vector<double>& y,int cross_validation=1);
	virtual ~SupportVectorMachine();
};

} /* namespace intel */

#endif /* SRC_MATH_SUPPORTVECTORMACHINE_H_ */
