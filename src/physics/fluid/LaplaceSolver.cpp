/*
 * Copyright(C) 2017, Blake C. Lucas, Ph.D. (img.science@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *  This implementation of a PIC/FLIP static_cast<char>(ObjectType::FLUID) simulator is derived from:
 *
 *  Ando, R., Thurey, N., & Tsuruno, R. (2012). Preserving static_cast<char>(ObjectType::FLUID) sheets with adaptively sampled anisotropic particles.
 *  Visualization and Computer Graphics, IEEE Transactions on, 18(8), 1202-1214.
 */
#include "physics/fluid/LaplaceSolver.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "physics/fluid/SimulationObjects.h"
using namespace std;
namespace aly {
// Clamped Fetch
static float x_ref(const Image1ub& A, const Image1f& L, const Image1f& x,
		int fi, int fj, int i, int j) {
	if (A(i, j).x == static_cast<char>(ObjectType::FLUID))
		return x(i, j);
	else if (A(i, j).x == static_cast<char>(ObjectType::WALL))
		return x(fi, fj).x;
	return L(i, j).x / std::min(1.0e-6f, L(fi, fj).x) * x(fi, fj).x;
}

// Ans = Ax
static void compute_Ax(const Image1ub& A, const Image1f& L, const Image1f& x,
		Image1f& ans, float voxelSize) {
	float h2 = voxelSize * voxelSize;
#pragma omp parallel for
	for (int j = 0; j < A.height; j++) {
		for (int i = 0; i < A.width; i++) {
			if (A(i, j) == static_cast<char>(ObjectType::FLUID)) {
				ans(i, j) = (4.0 * x(i, j)
						- x_ref(A, L, x, i, j, i + 1, j)
						- x_ref(A, L, x, i, j, i - 1, j)
						- x_ref(A, L, x, i, j, i, j + 1)
						- x_ref(A, L, x, i, j, i, j - 1)) / h2;
			} else {
				ans(i, j).x = 0.0;
			}
		}
	}
}
// ans = x^T * x
static double product(const Image1ub& A, const Image1f& x, const Image1f& y) {
	static double ans;
	ans = 0.0;
#pragma omp parallel for reduction(+:ans)
	for (int j = 0; j < A.height; j++) {
		for (int i = 0; i < A.width; i++) {
			if (A(i, j).x == static_cast<char>(ObjectType::FLUID))
				ans += x(i, j).x * y(i, j).x;
		}
	}
	return ans;
}

static void flipDivergence(Image1f& x) {
#pragma omp parallel for
	for (int n = 0; n < (int) x.size(); n++) {
		x[n].x = -x[n].x;
	}
}

// x <= y
static void copy(Image1f& x, Image1f& y) {
	x.set(y);
}
// Ans = x + a*y
static void op(const Image1ub& A, const Image1f& x, const Image1f& y,
		Image1f& ans, float a) {
	Image1f tmp(x.width, x.height);
#pragma omp parallel for
	for (int j = 0; j < x.height; j++) {
		for (int i = 0; i < x.width; i++) {
			if (A(i, j).x == static_cast<char>(ObjectType::FLUID))
				tmp(i, j).x = x(i, j).x + a * y(i, j).x;
			else
				tmp(i, j).x = 0.0;
		}
	}
	copy(ans, tmp);
}

// r = b - Ax
static void residual(const Image1ub& A, const Image1f& L, const Image1f& x,const Image1f& b, Image1f& r, float voxelSize) {
	r.resize(x.width, x.height);
	compute_Ax(A, L, x, r, voxelSize);
	op(A, b, r, r, -1.0);
}
static inline float square(float a) {
	return a * a;
}
static float A_ref(const Image1ub& A, int i, int j, int qi, int qj) {
	if (i < 0 || i > A.width - 1 || j < 0 || j > A.height - 1
			|| A(i, j).x != static_cast<char>(ObjectType::FLUID))
		return 0.0;
	if (qi < 0 || qi > A.width - 1 || qj < 0 || qj > A.height - 1
			|| A(qi, qj).x != static_cast<char>(ObjectType::FLUID))
		return 0.0;
	return -1.0;
}
static float A_diag(const Image1ub& A, const Image1f& L, int i, int j) {
	float diag = 6.0;
	if (A(i, j).x != static_cast<char>(ObjectType::FLUID))
		return diag;
	int q[][2] = { { i - 1, j }, { i + 1, j }, { i, j - 1 }, { i, j + 1 } };
	for (int m = 0; m < 4; m++) {
		int qi = q[m][0];
		int qj = q[m][1];
		if (qi < 0 || qi > A.width - 1 || qj < 0 || qj > A.height - 1
				|| A(qi, qj).x == static_cast<char>(ObjectType::WALL))
			diag -= 1.0;
		else if (A(qi, qj).x == static_cast<char>(ObjectType::AIR)) {
			diag -= L(qi, qj).x / std::min(1.0e-6f, L(i, j).x);
		}
	}
	return diag;
}

static double P_ref(const Image1d& P, const Image1ub& A, int i, int j) {
	if (i < 0 || i > P.width - 1 || j < 0 || j > P.height - 1
			|| A(i, j).x != static_cast<char>(ObjectType::FLUID))
		return 0.0;
	return P(i, j).x;
}
static float P_ref(const Image1f& P, const Image1ub& A, int i, int j) {
	if (i < 0 || i > P.width - 1 || j < 0 || j > P.height - 1
			|| A(i, j).x != static_cast<char>(ObjectType::FLUID))
		return 0.0;
	return P(i, j).x;
}
static void buildPreconditioner(Image1d& P, const Image1f& L,
		const Image1ub& A) {
	double a = 0.25;
	P.resize(A.width, A.height);
#pragma omp parallel for
	for (int j = 0; j < A.height; j++) {
		for (int i = 0; i < A.width; i++) {
			if (A(i, j).x == static_cast<char>(ObjectType::FLUID)) {
				double left = A_ref(A, i - 1, j, i, j) * P_ref(P, A, i - 1, j);
				double bottom = A_ref(A, i, j - 1, i, j)
						* P_ref(P, A, i, j - 1);
				double diag = A_diag(A, L, i, j);
				double e = diag - square(left) - square(bottom);
				if (e < a * diag)
					e = diag;
				P(i, j).x = 1.0 / std::sqrt(e);
			}
		}
	}
}

static void applyPreconditioner(Image1f& z, const Image1f& r, const Image1d& P,
		const Image1f& L, const Image1ub& A) {
	Image1d q(P.width, P.height);
	q.set(double1(0.0));
#pragma omp parallel for
	for (int j = 0; j < q.height; j++) {
		for (int i = 0; i < q.width; i++) {
			if (A(i, j).x == static_cast<char>(ObjectType::FLUID)) {
				double left = A_ref(A, i - 1, j, i, j) * P_ref(P, A, i - 1, j)
						* P_ref(q, A, i - 1, j);
				double bottom = A_ref(A, i, j - 1, i, j) * P_ref(P, A, i, j - 1)
						* P_ref(q, A, i, j - 1);
				double t = r(i, j).x - left - bottom;
				q(i, j).x = t * P(i, j);
			}
		}
	}
// L^T z = q
	for (int i = q.width - 1; i >= 0; i--) {
		for (int j = q.height - 1; j >= 0; j--) {
			if (A(i, j).x == static_cast<char>(ObjectType::FLUID)) {
				double right = A_ref(A, i, j, i + 1, j) * P_ref(P, A, i, j)
						* P_ref(z, A, i + 1, j);
				double top = A_ref(A, i, j, i, j + 1) * P_ref(P, A, i, j)
						* P_ref(z, A, i, j + 1);
				double t = q(i, j).x - right - top;
				z(i, j).x = t * P(i, j).x;
			}
		}
	}
}

// Conjugate Gradient Method
static void conjGrad(const Image1ub& A, const Image1d& P, const Image1f& L,
		Image1f& x, const Image1f& b, float voxelSize) {
// Pre-allocate Memory
	Image1f r(x.width, x.height);
	Image1f z(x.width, x.height);
	Image1f s(x.width, x.height);
	compute_Ax(A, L, x, z, voxelSize);                // z = applyA(x)
	op(A, b, z, r, -1.0);                  // r = b-Ax
	double error2_0 = product(A, r, r);    // error2_0 = r . r
	applyPreconditioner(z, r, P, L, A);		// Apply Conditioner z = f(r)
	copy(s, z);								// s = z
	int V = x.width*x.height;
	double eps = 1.0e-2 * (V);
	double a = product(A, z, r);			// a = z . r
	for (int k = 0; k < V; k++) {
		if(error2_0==0)break;
		compute_Ax(A, L, s, z, voxelSize);			// z = applyA(s)
		double alpha = a / product(A, z, s);	// alpha = a/(z . s)
		op(A, x, s, x, alpha);				// x = x + alpha*s
		op(A, r, z, r, -alpha);			// r = r - alpha*z;
		float error2 = product(A, r, r);	// error2 = r . r
		error2_0 = fmax(error2_0, error2);
		// Dump Progress
		double rate = 1.0
				- max(0.0, min(1.0, (error2 - eps) / (error2_0 - eps)));

		//std::cout<<k<<") Error "<<error2<<"/"<<error2_0<<std::endl;
		if (error2 <= eps&&k>=4)
			break;
		applyPreconditioner(z, r, P, L, A);	// Apply Conditioner z = f(r)
		double a2 = product(A, z, r);		// a2 = z . r
		double beta = a2 / a;                     // beta = a2 / a
		op(A, z, s, s, beta);				// s = z + beta*s
		a = a2;
	}
}

void SolveLaplace2d(const Image1ub& A, const Image1f& L, Image1f& x,const Image1f& b, float voxelSize) {
	Image1d P;
	buildPreconditioner(P, L, A);
	conjGrad(A, P, L, x, b, voxelSize);
}

}
