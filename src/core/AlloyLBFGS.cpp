/*
 * LBFSLineSearch.cpp
 *
 *  Created on: Sep 8, 2017
 *      Author: Blake Lucas
 */
#include <AlloyLBFGS.h>
#include <AlloyOptimizationMath.h>
#include <functional>
namespace aly {
void SANITY_CHECK_LBFGS(){
	const int N=10;
	const std::function<float(Vec<float>& x, Vec<float>& grad)> rosenbrock=[=](Vec<float>& x, Vec<float>& grad){
        float fx = 0.0;
        for(int i = 0; i < N; i += 2)
        {
            float t1 = 1.0 - x[i];
            float t2 = 10 * (x[i + 1] - x[i] * x[i]);
            grad[i + 1] = 20 * t2;
            grad[i]     = -2.0 * (x[i] * grad[i + 1] + t1);
            fx += t1 * t1 + t2 * t2;
        }
        return fx;
	};

    LBFGSParam<float> param;
    Vec<float> x(N);
    x.set(0.0f);
    float fx;
    SolveLBFGS(rosenbrock,x,fx,param,[=](int k, double err){
    	std::cout<<"LBFGS "<<k<<") "<<err<<std::endl;
    	return true;
    });
}
///
/// LBFGS solver for unconstrained numerical optimization
///
template<typename Scalar>
class LBFGSSolver {
private:
	typedef Vec<Scalar> Vector;
	typedef VecMap<Scalar> VectorM;
	typedef DenseMat<Scalar> Matrix;
	const LBFGSParam<Scalar>& m_param; // Parameters to control the LBFGS algorithm
	Matrix m_s;      // History of the s vectors
	Matrix m_y;      // History of the y vectors
	Vector m_ys;     // History of the s'y values
	Vector m_alpha;  // History of the step lengths
	Vector m_fx;     // History of the objective function values
	Vector m_xp;     // Old x
	Vector m_grad;   // New gradient
	Vector m_gradp;  // Old gradient
	Vector m_drt;    // Moving direction
	void Backtracking(
				const std::function<Scalar(Vector& x, Vector& grad)>& f, Scalar& fx,
				Vector& x, Vector& grad, Scalar& step, const Vector& drt,
				const Vector& xp, const LBFGSParam<Scalar>& param) {
			// Decreasing and increasing factors
			const Scalar dec = Scalar(0.5);
			const Scalar inc = Scalar(2.1);

			// Check the value of step
			if (step <= Scalar(0))
				std::invalid_argument("'step' must be positive");

			// Save the function value at the current x
			const Scalar fx_init = fx;
			// Projection of gradient on the search direction
			const Scalar dg_init = Scalar(dot(grad,drt));
			// Make sure d points to a descent direction
			if (dg_init > 0)
				std::logic_error(
						"the moving direction increases the objective function value");

			const Scalar dg_test = param.ftol * dg_init;
			Scalar width;

			int iter;
			for (iter = 0; iter < param.max_linesearch; iter++) {
				// x_{k+1} = x_k + step * d_k
				x = xp + step * drt;
				// Evaluate this candidate
				fx = f(x, grad);

				if (fx > fx_init + step * dg_test) {
					width = dec;
				} else {
					// Armijo condition is met
					if (param.linesearch ==LineSearchType::BACKTRACKING_ARMIJO)
						break;

					const Scalar dg = Scalar(dot(grad,drt));
					if (dg < param.wolfe * dg_init) {
						width = inc;
					} else {
						// Regular Wolfe condition is met
						if (param.linesearch== LineSearchType::BACKTRACKING_WOLFE)
							break;

						if (dg > -param.wolfe * dg_init) {
							width = dec;
						} else {
							// Strong Wolfe condition is met
							break;
						}
					}
				}

				if (iter >= param.max_linesearch)
					throw std::runtime_error(
							"the line search routine reached the maximum number of iterations");

				if (step < param.min_step)
					throw std::runtime_error(
							"the line search step became smaller than the minimum value allowed");

				if (step > param.max_step)
					throw std::runtime_error(
							"the line search step became larger than the maximum value allowed");

				step *= width;
			}
		}
	void reset(int n) {
		const int m = m_param.m;
		m_s.resize(n, m);
		m_y.resize(n, m);
		m_ys.resize(m);
		m_alpha.resize(m);
		m_xp.resize(n);
		m_grad.resize(n);
		m_gradp.resize(n);
		m_drt.resize(n);
		if (m_param.past > 0)
			m_fx.resize(m_param.past);
	}

public:
	///
	/// Constructor for LBFGS solver.
	///
	/// \param param An object of \ref LBFGSParam to store parameters for the
	///        algorithm
	///
	LBFGSSolver(const LBFGSParam<Scalar>& param) :
			m_param(param) {
		m_param.check_param();
	}

	///
	/// Minimizing a multivariate function using LBFGS algorithm.
	/// Exceptions will be thrown if error occurs.
	///
	/// \param f  A function object such that `f(x, grad)` returns the
	///           objective function value at `x`, and overwrites `grad` with
	///           the gradient.
	/// \param x  In: An initial guess of the optimal point. Out: The best point
	///           found.
	/// \param fx Out: The objective function value at `x`.
	///
	/// \return Number of iterations used.
	///
	int minimize(const std::function<Scalar(Vector& x, Vector& grad)>& f,
			Vector& x, Scalar& fx,const std::function<bool(int, double)>& iterationMonitor = nullptr) {
		const Scalar ZERO_TOLERANCE=1E-20f;
		const int n = (int)x.size();
		const int fpast = m_param.past;
		reset(n);
		// Evaluate function and compute gradient
		fx = f(x, m_grad);
		Scalar xnorm = Scalar(length(x));
		Scalar gnorm = Scalar(length(m_grad));
		if (fpast > 0)
			m_fx[0] = fx;

		// Early exit if the initial x is already a minimizer
		if (gnorm <= m_param.epsilon * std::max(xnorm, Scalar(1.0))) {
			return 1;
		}

		// Initial direction
		m_drt = -m_grad;
		// Initial step
		Scalar step = Scalar(1.0) / Scalar(length(m_drt));

		int k = 1;
		int end = 0;
		for (;;) {
			if(iterationMonitor){
				if(!iterationMonitor(k,fx)){
					break;
				}
			}
			// Save the curent x and gradient
			m_xp = x;
			m_gradp = m_grad;

			// Line search to update x, fx and gradient
			Backtracking(f, fx, x, m_grad, step, m_drt,m_xp, m_param);

			// New x norm and gradient norm
			xnorm = Scalar(length(x));
			gnorm = Scalar(length(m_grad));

			// Convergence test -- gradient
			if (gnorm <= m_param.epsilon * std::max(xnorm, Scalar(1.0))) {
				return k;
			}
			// Convergence test -- objective function value
			if (fpast > 0) {
				if (k >= fpast && (m_fx[k % fpast] - fx) / fx < m_param.delta)
					return k;

				m_fx[k % fpast] = fx;
			}
			// Maximum number of iterations
			if (m_param.max_iterations != 0 && k >= m_param.max_iterations) {
				return k;
			}

			// Update s and y
			// s_{k+1} = x_{k+1} - x_k
			// y_{k+1} = g_{k+1} - g_k
			VectorM svec=m_s.getColumn(end);
			VectorM yvec=m_y.getColumn(end);
			svec=x - m_xp;
			yvec=m_grad - m_gradp;
			// ys = y's = 1/rho
			// yy = y'y
			Scalar ys = Scalar(dot(yvec,svec));
			Scalar yy = Scalar(lengthSqr(yvec));
			m_ys[end] = ys;

			// Recursive formula to compute d = -H * g
			m_drt = -m_grad;
			int bound = std::min(m_param.m, k);
			end = (end + 1) % m_param.m;
			int j = end;
			for (int i = 0; i < bound; i++) {
				j = (j + m_param.m - 1) % m_param.m;
				VectorM sj=m_s.getColumn(j);
				VectorM yj=m_y.getColumn(j);
				m_alpha[j] = Scalar(dot(sj,m_drt)) / m_ys[j];
				m_drt -= m_alpha[j] * yj;
			}
			if(std::abs(yy)<ZERO_TOLERANCE)break;
			m_drt *= (ys / yy);

			for (int i = 0; i < bound; i++) {
				VectorM sj=m_s.getColumn(j);
				VectorM yj=m_y.getColumn(j);
				Scalar beta = Scalar(dot(yj,m_drt)) / m_ys[j];
				m_drt += (m_alpha[j] - beta) * sj;
				j = (j + 1) % m_param.m;
			}

			// step = 1.0 as initial guess
			step = Scalar(1.0);
			// step = Scalar(1e-1);

			k++;
		}

		return k;
	}
};
int SolveLBFGS(const std::function<float(Vec<float>& x, Vec<float>& grad)>& f,aly::Vec<float>& x, float& fx,const LBFGSParam<float>& param,const std::function<bool(int, double)>& iterationMonitor){
	LBFGSSolver<float> solver(param);
	return solver.minimize(f,x,fx,iterationMonitor);
}
int SolveLBFGS(const std::function<double(Vec<double>& x, Vec<double>& grad)>& f,aly::Vec<double>& x, double& fx,const LBFGSParam<double>& param,const std::function<bool(int, double)>& iterationMonitor){
	LBFGSSolver<double> solver(param);
	return solver.minimize(f,x,fx,iterationMonitor);
}
}
