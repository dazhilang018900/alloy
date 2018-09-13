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
#include "math/AlloyMathBase.h"
namespace aly {
int4 min(const std::vector<int4>& a, int init) {
	size_t sz = a.size();
	int4 tmp(init);
	for (size_t i = 0; i < sz; i++) {
		tmp = aly::min(tmp, a[i]);
	}
	return tmp;
}
int4 max(const std::vector<int4>& a, int init) {
	size_t sz = a.size();
	int4 tmp(init);
	for (size_t i = 0; i < sz; i++) {
		tmp = aly::max(a[i], tmp);
	}
	return tmp;
}

float4 min(const std::vector<float4>& a, float init) {
	size_t sz = a.size();
	float4 tmp(init);
	for (size_t i = 0; i < sz; i++) {
		tmp = aly::min(tmp, a[i]);
	}
	return tmp;
}
float4 max(const std::vector<float4>& a, float init) {
	size_t sz = a.size();
	float4 tmp(init);
	for (size_t i = 0; i < sz; i++) {
		tmp = aly::max(a[i], tmp);
	}
	return tmp;
}
double4 min(const std::vector<double4>& a, double init) {
	size_t sz = a.size();
	double4 tmp(init);
	for (size_t i = 0; i < sz; i++) {
		tmp = aly::min(tmp, a[i]);
	}
	return tmp;
}
double4 max(const std::vector<double4>& a, double init) {
	size_t sz = a.size();
	double4 tmp(init);
	for (size_t i = 0; i < sz; i++) {
		tmp = aly::max(a[i], tmp);
	}
	return tmp;
}

int3 min(const std::vector<int3>& a, int init) {
	size_t sz = a.size();
	int3 tmp(init);
	for (size_t i = 0; i < sz; i++) {
		tmp = aly::min(tmp, a[i]);
	}
	return tmp;
}
int3 max(const std::vector<int3>& a, int init) {
	size_t sz = a.size();
	int3 tmp(init);
	for (size_t i = 0; i < sz; i++) {
		tmp = aly::max(a[i], tmp);
	}
	return tmp;
}

float3 min(const std::vector<float3>& a, float init) {
	size_t sz = a.size();
	float3 tmp(init);
	for (size_t i = 0; i < sz; i++) {
		tmp = aly::min(tmp, a[i]);
	}
	return tmp;
}
float3 max(const std::vector<float3>& a, float init) {
	size_t sz = a.size();
	float3 tmp(init);
	for (size_t i = 0; i < sz; i++) {
		tmp = aly::max(a[i], tmp);
	}
	return tmp;
}
double3 min(const std::vector<double3>& a, double init) {
	size_t sz = a.size();
	double3 tmp(init);
	for (size_t i = 0; i < sz; i++) {
		tmp = aly::min(tmp, a[i]);
	}
	return tmp;
}
double3 max(const std::vector<double3>& a, double init) {
	size_t sz = a.size();
	double3 tmp(init);
	for (size_t i = 0; i < sz; i++) {
		tmp = aly::max(a[i], tmp);
	}
	return tmp;
}
int2 min(const std::vector<int2>& a, int init) {
	size_t sz = a.size();
	int2 tmp(init);
	for (size_t i = 0; i < sz; i++) {
		tmp = aly::min(tmp, a[i]);
	}
	return tmp;
}
int2 max(const std::vector<int2>& a, int init) {
	size_t sz = a.size();
	int2 tmp(init);
	for (size_t i = 0; i < sz; i++) {
		tmp = aly::max(a[i], tmp);
	}
	return tmp;
}

float2 min(const std::vector<float2>& a, float init) {
	size_t sz = a.size();
	float2 tmp(init);
	for (size_t i = 0; i < sz; i++) {
		tmp = aly::min(tmp, a[i]);
	}
	return tmp;
}
float2 max(const std::vector<float2>& a, float init) {
	size_t sz = a.size();
	float2 tmp(init);
	for (size_t i = 0; i < sz; i++) {
		tmp = aly::max(a[i], tmp);
	}
	return tmp;
}
double2 min(const std::vector<double2>& a, double init) {
	size_t sz = a.size();
	double2 tmp(init);
	for (size_t i = 0; i < sz; i++) {
		tmp = aly::min(tmp, a[i]);
	}
	return tmp;
}
double2 max(const std::vector<double2>& a, double init) {
	size_t sz = a.size();
	double2 tmp(init);
	for (size_t i = 0; i < sz; i++) {
		tmp = aly::max(a[i], tmp);
	}
	return tmp;
}
int min(const std::vector<int>& a, int init) {
	size_t sz = a.size();
	int tmp = init;
	for (size_t i = 0; i < sz; i++) {
		tmp = std::min(tmp, a[i]);
	}
	return tmp;
}
int max(const std::vector<int>& a, int init) {
	size_t sz = a.size();
	int tmp = init;
	for (size_t i = 0; i < sz; i++) {
		tmp = std::max(a[i], tmp);
	}
	return tmp;
}
float min(const std::vector<float>& a, float init) {
	size_t sz = a.size();
	float tmp = init;
	for (size_t i = 0; i < sz; i++) {
		tmp = std::min(tmp, a[i]);
	}
	return tmp;
}
float max(const std::vector<float>& a, float init) {
	size_t sz = a.size();
	float tmp = init;
	for (size_t i = 0; i < sz; i++) {
		tmp = std::max(a[i], tmp);
	}
	return tmp;
}
double min(const std::vector<double>& a, double init) {
	size_t sz = a.size();
	double tmp = init;
	for (size_t i = 0; i < sz; i++) {
		tmp = std::min(tmp, a[i]);
	}
	return tmp;
}
double max(const std::vector<double>& a, double init) {
	size_t sz = a.size();
	double tmp = init;
	for (size_t i = 0; i < sz; i++) {
		tmp = std::max(a[i], tmp);
	}
	return tmp;
}
float InvSqrt(float x) {
	float xhalf = 0.5f * x;
	int i = *(int*) &x;
	i = 0x5f3759df - (i >> 1);
	x = *(float*) &i;
	x = x * (1.5f - xhalf * x * x);
	return x;
}

float RandomUniform(float min, float max) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<float> noise(min, max);
	return noise(gen);
}
int RandomUniform(int min, int max) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_int_distribution<int> noise(min, max);
	return noise(gen);
}
double RandomUniform(double min, double max) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<double> noise(min, max);
	return noise(gen);
}
double RandomGaussian(double mean, double stddev) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::normal_distribution<double> noise(mean, stddev);
	return noise(gen);
}
float RandomGaussian(float mean, float stddev) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::normal_distribution<float> noise(mean, stddev);
	return noise(gen);
}

template<class T> matrix<T, 4, 4> MakeRotationInternal(const vec<T, 3>& axis,
		T angle) {
	matrix<T, 4, 4> M = Identity<T, 4, 4>();
	T mag = length(axis);
	if (mag >= 1E-6f) {
		mag = ((T) 1.0) / mag;
		T ax = axis[0] * mag;
		T ay = axis[1] * mag;
		T az = axis[2] * mag;
		T sinTheta = (T) sin(angle);
		T cosTheta = (T) cos(angle);
		T t = (T) 1.0 - cosTheta;

		T xz = ax * az;
		T xy = ax * ay;
		T yz = ay * az;

		M(0, 0) = t * ax * ax + cosTheta;
		M(0, 1) = t * xy - sinTheta * az;
		M(0, 2) = t * xz + sinTheta * ay;

		M(1, 0) = t * xy + sinTheta * az;
		M(1, 1) = t * ay * ay + cosTheta;
		M(1, 2) = t * yz - sinTheta * ax;

		M(2, 0) = t * xz - sinTheta * ay;
		M(2, 1) = t * yz + sinTheta * ax;
		M(2, 2) = t * az * az + cosTheta;
	}
	return M;
}
template<class T> matrix<T, 4, 4> MakeRotationInternal(const vec<T, 3>& axis) {
	matrix<T, 4, 4> M = Identity<T, 4, 4>();
	T mag = length(axis);
	if (mag >= 1E-6f) {
		T sinTheta = (T) sin(mag);
		T cosTheta = (T) cos(mag);
		mag = ((T) 1.0) / mag;
		T ax = axis[0] * mag;
		T ay = axis[1] * mag;
		T az = axis[2] * mag;
		T t = (T) 1.0 - cosTheta;

		T xz = ax * az;
		T xy = ax * ay;
		T yz = ay * az;

		M(0, 0) = t * ax * ax + cosTheta;
		M(0, 1) = t * xy - sinTheta * az;
		M(0, 2) = t * xz + sinTheta * ay;

		M(1, 0) = t * xy + sinTheta * az;
		M(1, 1) = t * ay * ay + cosTheta;
		M(1, 2) = t * yz - sinTheta * ax;

		M(2, 0) = t * xz - sinTheta * ay;
		M(2, 1) = t * yz + sinTheta * ax;
		M(2, 2) = t * az * az + cosTheta;
	}
	return M;
}
matrix<float, 4, 4> MakeRotation(const vec<float, 3>& axis, float angle) {
	return MakeRotationInternal(axis, angle);
}
matrix<double, 4, 4> MakeRotation(const vec<double, 3>& axis, double angle) {
	return MakeRotationInternal(axis, angle);
}

matrix<float, 4, 4> MakeRotation(const vec<float, 3>& axis) {
	return MakeRotationInternal(axis);
}
matrix<double, 4, 4> MakeRotation(const vec<double, 3>& axis) {
	return MakeRotationInternal(axis);
}
}
