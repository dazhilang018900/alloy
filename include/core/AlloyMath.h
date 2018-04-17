/*
 * Copyright(C) 2015, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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
 */

#ifndef ALLOYMATH_H_INCLUDE_GUARD
#define ALLOYMATH_H_INCLUDE_GUARD
#include "AlloyMathBase.h"
namespace aly {
bool SANITY_CHECK_MATH();
bool SANITY_CHECK_CEREAL();
bool SANITY_CHECK_SVD();
/*

 The MIT License (MIT)

 Copyright (c) 2014 Stan Melax

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

template<class T> vec<T, 3> GetDiagonal(const matrix<T, 3, 3> &m) {
	return {m.x.x, m.y.y, m.z.z};
}
template<class T> vec<T, 4> GetDiagonal(const matrix<T, 4, 4> &m) {
	return {m.x.x, m.y.y, m.z.z,m.w.w};
}
template<class T> vec<T, 4> Diagonalizer(const matrix<T, 3, 3> &A, bool sort =
		true) {
	// A must be a symmetric matrix.
	// returns orientation of the principle axes.
	// returns quaternion q such that its corresponding column major matrix Q
	// can be used to Diagonalize A
	// Diagonal matrix D = transpose(Q) * A * (Q);  thus  A == Q*D*QT
	// The directions of q (cols of Q) are the eigenvectors D's diagonal is the eigenvalues
	// As per 'col' convention if matrix<T,3,3> Q = qgetmatrix(q); then Q*v = q*v*conj(q)
	int maxsteps = 24;  // certainly wont need that many.
	int i;
	vec<T, 4> q(0, 0, 0, 1);
	for (i = 0; i < maxsteps; i++) {
		matrix<T, 3, 3> Q = q2matrix(q); // Q*v == q*v*conj(q)
		matrix<T, 3, 3> D = transpose(Q) * A * Q;  // A = Q*D*Q^T
		vec<T, 3> offdiag(D[1][2], D[0][2], D[0][1]); // elements not on the diagonal
		vec<T, 3> om(fabsf(offdiag.x), fabsf(offdiag.y), fabsf(offdiag.z)); // mag of each offdiag elem
		int k = (om.x > om.y && om.x > om.z) ? 0 : (om.y > om.z) ? 1 : 2; // index of largest element of offdiag
		int k1 = (k + 1) % 3;
		int k2 = (k + 2) % 3;
		if (offdiag[k] == 0.0f)
			break;  // diagonal already
		float thet = (D[k2][k2] - D[k1][k1]) / (2.0f * offdiag[k]);
		float sgn = (thet > 0.0f) ? 1.0f : -1.0f;
		thet *= sgn; // make it positive
		float t = sgn
				/ (thet + ((thet < 1.E6f) ? sqrtf(thet * thet + 1.0f) : thet)); // sign(T)/(|T|+sqrt(T^2+1))
		float c = 1.0f / sqrtf(t * t + 1.0f); //  c= 1/(t^2+1) , t=s/c
		if (c == 1.0f)
			break;  // no room for improvement - reached machine precision.
		vec<T, 4> jr(0, 0, 0, 0); // jacobi rotation for this iteration.
		jr[k] = sgn * sqrtf((1.0f - c) / 2.0f); // using 1/2 angle identity sin(a/2) = sqrt((1-cos(a))/2)
		jr[k] *= -1.0f; // note we want a final result semantic that takes D to A, not A to D
		jr.w = sqrtf(1.0f - (jr[k] * jr[k]));
		if (jr.w == 1.0f)
			break; // reached limits of floating point precision
		q = qmul(q, jr);
		q = normalize(q);
	}
	if (sort) {
		float h = 1.0f / sqrtf(2.0f);  // M_SQRT2
		auto e =
				[&q, &A]() {return GetDiagonal(transpose(q2matrix(q))* A* q2matrix(q));}; // current ordering of eigenvals of q
		q = (e().x < e().z) ? qmul(q, vec<T, 4>(0, h, 0, h)) : q;
		q = (e().y < e().z) ? qmul(q, vec<T, 4>(h, 0, 0, h)) : q;
		q = (e().x < e().y) ? qmul(q, vec<T, 4>(0, 0, h, h)) : q; // size order z,y,x so xy spans a planeish spread
		q = (qrotate_z(q).z < 0) ? qmul(q, vec<T, 4>(1, 0, 0, 0)) : q;
		q = (qrotate_y(q).y < 0) ? qmul(q, vec<T, 4>(0, 0, 1, 0)) : q;
		q = (q.w < 0) ? -q : q;
	}
	//matrix<T,3,3> M = transpose(q2matrix(q)) * A * q2matrix(q);  // to test result
	return q;
}
template<class T> void Eigen(const matrix<T, 3, 3> &A, matrix<T, 3, 3>& Q,
		matrix<T, 3, 3>& D, bool sort = true) {
	vec<T, 4> q = Diagonalizer(A, sort);
	Q = q2matrix(q);
	D = transpose(Q) * A * Q;
}
inline static double pythag(double a, double b) {
	double at = std::abs(a), bt = std::abs(b), ct, result;
	if (at > bt) {
		ct = bt / at;
		result = at * std::sqrt(1.0 + ct * ct);
	} else if (bt > 0.0) {
		ct = at / bt;
		result = bt * std::sqrt(1.0 + ct * ct);
	} else
		result = 0.0;
	return (result);
}
void SVD(const matrix<float, 2, 2> &A, matrix<float, 2, 2>& U,
		matrix<float, 2, 2>& D, matrix<float, 2, 2>& Vt);
void SVD(const matrix<float, 3, 3> &A, matrix<float, 3, 3>& U,
		matrix<float, 3, 3>& D, matrix<float, 3, 3>& Vt);
void SVD(const matrix<float, 4, 4> &A, matrix<float, 4, 4>& U,
		matrix<float, 4, 4>& D, matrix<float, 4, 4>& Vt);

void SVD(const matrix<double, 2, 2> &A, matrix<double, 2, 2>& U,
		matrix<double, 2, 2>& D, matrix<double, 2, 2>& Vt);
void SVD(const matrix<double, 3, 3> &A, matrix<double, 3, 3>& U,
		matrix<double, 3, 3>& D, matrix<double, 3, 3>& Vt);
void SVD(const matrix<double, 4, 4> &A, matrix<double, 4, 4>& U,
		matrix<double, 4, 4>& D, matrix<double, 4, 4>& Vt);

template<class T> matrix<T, 3, 3> FactorRotation(const matrix<T, 3, 3> A) {
	matrix<T, 3, 3> U, D, Vt;
	SVD(A, U, D, Vt);
	matrix<T, 3, 3> R = U * Vt;
	if (determinant(R) < 0) {
		R = U * MakeDiagonal(vec<T, 3>(1, 1, -1)) * Vt;
	}
	return R;
}
template<class T> matrix<T, 3, 3> FactorRotationInverse(
		const matrix<T, 3, 3> A) {
	matrix<T, 3, 3> U, D, Vt;
	SVD(A, U, D, Vt);
	matrix<T, 3, 3> R = transpose(Vt) * transpose(U);
	if (determinant(R) < 0) {
		R = transpose(Vt) * MakeDiagonal(vec<T, 3>(1, 1, -1)) * transpose(U);
	}
	return R;
}

template<class K, class T, int C> void ConvertType(vec<T, C> in,
		vec<K, C>& out) {
	for (int c = 0; c < C; c++) {
		out[c] = static_cast<K>(in[c]);
	}
}
template<class K, class T, int C> vec<K, C> ConvertType(vec<T, C> in) {
	vec<K, C> out;
	ConvertType(in, out);
	return out;
}

template<class T> vec<T, 3> MakeEulerAngles(const matrix<T, 3, 3>& coeff,
		int a0 = 0, int a1 = 1, int a2 = 2) {
	vec<T, 3> res;
	const int odd = ((a0 + 1) % 3 == a1) ? 0 : 1;
	const int i = a0;
	const int j = (a0 + 1 + odd) % 3;
	const int k = (a0 + 2 - odd) % 3;
	if (a0 == a2) {
		res[0] = std::atan2(coeff(j, i), coeff(k, i));
		if ((odd && res[0] < T(0)) || ((!odd) && res[0] > T(0))) {
			if (res[0] > T(0)) {
				res[0] -= T(ALY_PI);
			} else {
				res[0] += T(ALY_PI);
			}
			T s2 = Vector2(coeff(j, i), coeff(k, i)).norm();
			res[1] = -std::atan2(s2, coeff(i, i));
		} else {
			T s2 = Vector2(coeff(j, i), coeff(k, i)).norm();
			res[1] = std::atan2(s2, coeff(i, i));
		}
		// With a=(0,1,0), we have i=0; j=1; k=2, and after computing the first two angles,
		// we can compute their respective rotation, and apply its inverse to M. Since the result must
		// be a rotation around x, we have:
		//
		//  c2  s1.s2 c1.s2                   1  0   0
		//  0   c1    -s1       *    M    =   0  c3  s3
		//  -s2 s1.c2 c1.c2                   0 -s3  c3
		//
		//  Thus:  m11.c1 - m21.s1 = c3  &   m12.c1 - m22.s1 = s3
		T s1 = std::sin(res[0]);
		T c1 = std::cos(res[0]);
		res[2] = std::atan2(c1 * coeff(j, k) - s1 * coeff(k, k),
				c1 * coeff(j, j) - s1 * coeff(k, j));
	} else {
		res[0] = std::atan2(coeff(j, k), coeff(k, k));
		T c2 = Vector2(coeff(i, i), coeff(i, j)).norm();
		if ((odd && res[0] < T(0)) || ((!odd) && res[0] > T(0))) {
			if (res[0] > T(0)) {
				res[0] -= T(ALY_PI);
			} else {
				res[0] += T(ALY_PI);
			}
			res[1] = std::atan2(-coeff(i, k), -c2);
		} else {
			res[1] = std::atan2(-coeff(i, k), c2);
		}
		T s1 = std::sin(res[0]);
		T c1 = std::cos(res[0]);
		res[2] = std::atan2(s1 * coeff(k, i) - c1 * coeff(j, i),
				c1 * coeff(j, j) - s1 * coeff(k, j));
	}
	if (!odd)
		res = -res;
	return res;
}
template<class T> vec<T, 3> MakeEulerAngles(const matrix<T, 4, 4>& coeff,
		int a0 = 0, int a1 = 1, int a2 = 2) {
	vec<T, 3> res;
	const int odd = ((a0 + 1) % 3 == a1) ? 0 : 1;
	const int i = a0;
	const int j = (a0 + 1 + odd) % 3;
	const int k = (a0 + 2 - odd) % 3;
	if (a0 == a2) {
		res[0] = std::atan2(coeff(j, i), coeff(k, i));
		if ((odd && res[0] < T(0)) || ((!odd) && res[0] > T(0))) {
			if (res[0] > T(0)) {
				res[0] -= T(ALY_PI);
			} else {
				res[0] += T(ALY_PI);
			}
			T s2 = Vector2(coeff(j, i), coeff(k, i)).norm();
			res[1] = -std::atan2(s2, coeff(i, i));
		} else {
			T s2 = Vector2(coeff(j, i), coeff(k, i)).norm();
			res[1] = std::atan2(s2, coeff(i, i));
		}
		// With a=(0,1,0), we have i=0; j=1; k=2, and after computing the first two angles,
		// we can compute their respective rotation, and apply its inverse to M. Since the result must
		// be a rotation around x, we have:
		//
		//  c2  s1.s2 c1.s2                   1  0   0
		//  0   c1    -s1       *    M    =   0  c3  s3
		//  -s2 s1.c2 c1.c2                   0 -s3  c3
		//
		//  Thus:  m11.c1 - m21.s1 = c3  &   m12.c1 - m22.s1 = s3
		T s1 = std::sin(res[0]);
		T c1 = std::cos(res[0]);
		res[2] = std::atan2(c1 * coeff(j, k) - s1 * coeff(k, k),
				c1 * coeff(j, j) - s1 * coeff(k, j));
	} else {
		res[0] = std::atan2(coeff(j, k), coeff(k, k));
		T c2 = Vector2(coeff(i, i), coeff(i, j)).norm();
		if ((odd && res[0] < T(0)) || ((!odd) && res[0] > T(0))) {
			if (res[0] > T(0)) {
				res[0] -= T(ALY_PI);
			} else {
				res[0] += T(ALY_PI);
			}
			res[1] = std::atan2(-coeff(i, k), -c2);
		} else {
			res[1] = std::atan2(-coeff(i, k), c2);
		}
		T s1 = std::sin(res[0]);
		T c1 = std::cos(res[0]);
		res[2] = std::atan2(s1 * coeff(k, i) - c1 * coeff(j, i),
				c1 * coeff(j, j) - s1 * coeff(k, j));
	}
	if (!odd)
		res = -res;
	return res;
}
template<class T> matrix<T, 4, 4> MakePerspectiveMatrix(const T &fovy,
		const T &aspect, const T &zNear, const T &zFar) {
	T f = 1.0f / tan(ALY_PI * fovy / 360.0f);
	T sx = f / aspect;
	T sy = f;
	T sz = -(zFar + zNear) / (zFar - zNear);
	T pz = -(2.0f * zFar * zNear) / (zFar - zNear);
	matrix<T, 4, 4> M = matrix<T, 4, 4>::zero();
	M(0, 0) = sx;
	M(1, 1) = sy;
	M(2, 2) = sz;
	M(3, 2) = -1.0f;
	M(2, 3) = pz;
	return M;
}
template<class T> matrix<T, 4, 4> MakeOrthographicMatrix(const T& scaleX,
		const T& scaleY, const T &zNear, const T &zFar) {
	T sx = 2.0f * scaleX;
	T sy = 2.0f * scaleY;
	T pz = -(zFar + zNear) / (zFar - zNear);
	T sz = -(2.0f) / (zFar - zNear);
	matrix<T, 4, 4> M = matrix<T, 4, 4>::zero();
	M(0, 0) = sx;
	M(1, 1) = sy;
	M(2, 2) = sz;
	M(3, 3) = 1.0f;
	M(2, 3) = pz;
	return M;
}
template<class T> matrix<T, 4, 4> MakeLookAtMatrix(vec<T, 3> eyePosition3D,
		vec<T, 3> center3D, vec<T, 3> upVector3D) {
	vec<T, 3> forward, side, up;
	matrix<T, 4, 4> matrix2;
	matrix<T, 4, 4> resultMatrix;
	forward = normalize(center3D - eyePosition3D);
	side = normalize(cross(forward, upVector3D));
	up = cross(side, forward);
	matrix2[0] = vec<T, 4>(side, 0.0f);
	matrix2[1] = vec<T, 4>(up, 0.0f);
	matrix2[2] = vec<T, 4>(-forward, 0.0f);
	matrix2[3] = vec<T, 4>(0, 0, 0, 1);
	matrix<T, 4, 4> M = matrix<T, 4, 4>::identity();
	M(0, 3) = -eyePosition3D[0];
	M(1, 3) = -eyePosition3D[1];
	M(2, 3) = -eyePosition3D[2];
	return transpose(matrix2) * M;
}

template<class T, int C> double LineSearch(vec<T, C>& value,
		const vec<T, C>& minValue, const vec<T, C>& maxValue,
		const std::function<double(const vec<T, C>& value)>& scoreFunc,
		double err = 1E-5) {
	const T tolerance = T(err * lengthL1(maxValue - minValue));
	const T sqrt5 = T(2.236067977499789696);
	const T lambda = T(0.5 * (sqrt5 - 1.0));
	const T mu = T(0.5 * (3.0 - sqrt5));
	const int MAX_ITERATIONS = 256;
	vec<T, C> x1;
	vec<T, C> x2;
	double fx1;
	double fx2;
	vec<T, C> a = minValue;
	vec<T, C> b = maxValue;
	x1 = b - lambda * (b - a);
	x2 = a + lambda * (b - a);
	fx1 = scoreFunc(x1);
	fx2 = scoreFunc(x2);
	int count = 0;
	while (lengthL1(b - a) >= tolerance && count < MAX_ITERATIONS) {
		if (fx1 > fx2) {
			a = x1;
			if (lengthL1(b - a) < tolerance)
				break;
			x1 = x2;
			fx1 = fx2;
			x2 = b - mu * (b - a);
			fx2 = scoreFunc(x2);
		} else {
			b = x2;
			if (lengthL1(b - a) < tolerance)
				break;
			x2 = x1;
			fx2 = fx1;
			x1 = a + mu * (b - a);
			fx1 = scoreFunc(x1);
		}
		count++;
	}
	value = a;
	return scoreFunc(a);
}
template<class T> double LineSearch(T& value, const T& minValue,
		const T& maxValue,
		const std::function<double(const T& value)>& scoreFunc, double err =
				1E-5) {
	const T tolerance = T(err * (maxValue - minValue));
	const T sqrt5 = T(2.236067977499789696);
	const T lambda = T(0.5 * (sqrt5 - 1.0));
	const T mu = T(0.5 * (3.0 - sqrt5));
	const int MAX_ITERATIONS = 256;
	T x1;
	T x2;
	double fx1;
	double fx2;
	T a = minValue;
	T b = maxValue;
	x1 = b - lambda * (b - a);
	x2 = a + lambda * (b - a);
	fx1 = scoreFunc(x1);
	fx2 = scoreFunc(x2);
	int count = 0;
	while (b - a >= tolerance && count < MAX_ITERATIONS) {
		if (fx1 > fx2) {
			a = x1;
			if (b - a < tolerance)
				break;
			x1 = x2;
			fx1 = fx2;
			x2 = b - mu * (b - a);
			fx2 = scoreFunc(x2);
		} else {
			b = x2;
			if (b - a < tolerance)
				break;
			x2 = x1;
			fx2 = fx1;
			x1 = a + mu * (b - a);
			fx1 = scoreFunc(x1);
		}
		count++;
	}
	value = a;
	return scoreFunc(a);
}

bool ClipLine(float2& pt1, float2& pt2, const float2& minPt,
		const float2& maxPt);
inline bool ClipLine(float2& pt1, float2& pt2, const box2f& box) {
	return ClipLine(pt1, pt2, box.min(), box.max());
}
inline bool ClipLine(lineseg2f& line, const box2f& box) {
	return ClipLine(line.start, line.end, box.min(), box.max());
}
inline bool ClipLine(lineseg2f& line, const float2& minPt,
		const float2& maxPt) {
	return ClipLine(line.start, line.end, minPt, maxPt);
}
bool ClipLine(double2& pt1, double2& pt2, const double2& minPt,
		const double2& maxPt);
inline bool ClipLine(double2& pt1, double2& pt2, const box2d& box) {
	return ClipLine(pt1, pt2, box.min(), box.max());
}
inline bool ClipLine(lineseg2d& line, const box2d& box) {
	return ClipLine(line.start, line.end, box.min(), box.max());
}
inline bool ClipLine(lineseg2d& line, const double2& minPt,
		const double2& maxPt) {
	return ClipLine(line.start, line.end, minPt, maxPt);
}

float IntersectPlane(const aly::float3& pt, const aly::float3& ray,
		const aly::float4& plane);
float IntersectCylinder(aly::float3 pt, aly::float3 ray, float3 start,
		float3 end, float radius);
aly::float3 MakeOrthogonalComplement(const aly::float3& v);
aly::float2 MakeOrthogonalComplement(const aly::float2& v);
bool IntersectBox(aly::float3 rayOrig, aly::float3 rayDir,
		const aly::box3f& bbox, float& near, float& far);


template<class T> void Shuffle(std::vector<T>& order) {
	static std::random_device rd;
	static std::mt19937 g(rd());
	shuffle(order.begin(), order.end(), g);
}

bool Contains(float2 p, float2 pt1, float2 pt2, float2 pt3);
float3 ToBary(float3 p, float3 pt1, float3 pt2, float3 pt3);
float3 ToBary(float2 p, float2 pt1, float2 pt2, float2 pt3);

double3 ToBary(double3 p, float3 pt1, float3 pt2, float3 pt3);
double3 ToBary(double2 p, float2 pt1, float2 pt2, float2 pt3);

float3 FromBary(float3 b, float3 p1, float3 p2, float3 p3);
float2 FromBary(float3 b, float2 p1, float2 p2, float2 p3);
float3 FromBary(double3 b, float3 p1, float3 p2, float3 p3);
float2 FromBary(double3 b, float2 p1, float2 p2, float2 p3);
float DistanceToEdgeSqr(const float3& pt, const float3& pt1, const float3& pt2,float3* lastClosestSegmentPoint);
float DistanceToEdgeSqr(const float3& pt, const float3& pt1, const float3& pt2);
float DistanceToTriangleSqr(const float3& p, const float3& v0, const float3& v1,const float3& v2, float3* closestPoint);
float DistanceToTriangleSqr(const float2& p, const float2& v0, const float2& v1,const float2& v2, float2* closestPoint);
float DistanceToQuadSqr(const float3& p, const float3& v0, const float3& v1,
		const float3& v2, const float3& v3, const float3& normal,
		float3* closestPoint);
float DistanceToQuadSqr(const float3& p, const float3& v0, const float3& v1,
		const float3& v2, const float3& v3,
		float3* closestPoint);
float DistanceToQuadSqr(const float2& p, const float2& v0, const float2& v1,const float2& v2, const float2& v3,float2* closestPoint);
inline float DistanceToQuadSqr(const float2& p, const std::vector<float2>& corners,float2* closestPoint){
	return DistanceToQuadSqr(p,corners[0],corners[1],corners[2],corners[3],closestPoint);
}
inline float DistanceToQuadSqr(const float2& p,float2 corners[4],float2* closestPoint){
	return DistanceToQuadSqr(p,corners[0],corners[1],corners[2],corners[3],closestPoint);
}
}
namespace std {
	template<> struct hash<aly::int4> {
	typedef aly::int3 argument_type;
		typedef std::size_t result_type;
		std::size_t operator()(const aly::int4& val) const {
			return ((size_t) val.x) * 73856093L ^ ((size_t) val.y) * 19349663L
					^ ((size_t) val.z) * 83492791L ^ ((size_t) val.w) * 58291031L;
		}
	};
	template<> struct hash<aly::int3> {
	typedef aly::int3 argument_type;
		typedef std::size_t result_type;
		std::size_t operator()(const aly::int3& val) const {
			return ((size_t) val.x) * 73856093L ^ ((size_t) val.y) * 19349663L
					^ ((size_t) val.z) * 83492791L;
		}
	};
	template<> struct hash<aly::int2> {
		typedef aly::int2 argument_type;
		typedef std::size_t result_type;
		std::size_t operator()(const aly::int2& val) const {
			return ((size_t) val.x) | (((size_t) val.y) << 32);;
		}
	};
	template<> struct hash<aly::int1> {
		typedef aly::int1 argument_type;
		typedef std::size_t result_type;
		std::size_t operator()(const aly::int1& val) const {
			return val.x;
		}
	};
	template<> struct hash<aly::vec<size_t,1>> {
		typedef aly::vec<size_t,1> argument_type;
		typedef std::size_t result_type;
		std::size_t operator()(const aly::vec<size_t,1>& val) const {
			return val.x;
		}
	};
	template<> struct equal_to<aly::int4> {
		constexpr bool operator()(const aly::int4& a, const aly::int4 &b) const {
			return (a.x == b.x && a.y == b.y && a.z == b.z&& a.w == b.w);
		}
	};
	template<> struct equal_to<aly::int3> {
		constexpr bool operator()(const aly::int3& a, const aly::int3 &b) const {
			return (a.x == b.x && a.y == b.y && a.z == b.z);
		}
	};
	template<> struct equal_to<aly::int2> {
		constexpr bool operator()(const aly::int2& a, const aly::int2 &b) const {
			return (a.x == b.x && a.y == b.y);
		}
	};
	template<> struct equal_to<aly::int1> {
		constexpr bool operator()(const aly::int1& a, const aly::int1 &b) const {
			return (a.x == b.x);
		}
	};
	template<> struct equal_to<aly::vec<size_t,1>> {
		constexpr bool operator()(const aly::vec<size_t,1>& a, const aly::vec<size_t,1> &b) const {
			return (a.x == b.x);
		}
	};
}
#endif
