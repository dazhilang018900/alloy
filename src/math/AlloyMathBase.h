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

#ifndef INCLUDE_CORE_ALLOYMATHBASE_H_
#define INCLUDE_CORE_ALLOYMATHBASE_H_
#undef _USE_MATH_DEFINES
#undef USE_MATH_DEFINES
#include <stdint.h>
#include <tuple>
#include <iomanip>
#include <iostream>
#include <limits>
#include <cmath>
#include <random>
#include "common/AlloyCommon.h"
#include "common/cereal/cereal.hpp"
#define ALY_PI float(3.1415926535897932384626433832795)
#define ALY_PI_2 float(0.5f*ALY_PI)
#define ALY_1_PI float(1.0f/ALY_PI)
#define ALY_2_PI float(2.0f/ALY_PI)
#define ALY_PI_4 float(0.25f*ALY_PI)
namespace aly {
template<typename T> T min(const T& x, const T& y) {
	return ((x) < (y) ? (x) : (y));
}
template<typename T> T max(const T& x, const T& y) {
	return ((x) > (y) ? (x) : (y));
}
template<typename T> T max(const T& x, const T& y, const T& z) {
	return ((x) > (y) ? ((x > z) ? x : z) : ((y > z) ? y : z));
}
template<typename T> T max(const T& x, const T& y, const T& z, const T& w) {
	return max(max(x, y), max(z, w));
}
template<typename T> T min(const T& x, const T& y, const T& z) {
	return ((x) < (y) ? ((x < z) ? x : z) : ((y < z) ? y : z));
}
template<typename T> T min(const T& x, const T& y, const T& z, const T& w) {
	return min(min(x, y), min(z, w));
}
template<typename T> T clamp(const T& val, const T& min, const T& max) {
	return aly::min(aly::max(val, min), max);
}
template<typename T> T sign(const T& a, const T& b) {
	return ((b) >= 0.0 ? std::abs(a) : -std::abs(a));
}
template<typename T> T sign(const T& a) {
	return (a == 0) ? T(0) : ((a > 0.0) ? T(1) : T(-1));
}
template<typename T> T round(const T & v) {
	return T(std::floor(T(v + 0.5)));
}
template<typename T> T round(const T & v, int sigs) {
	return T(std::floor(T(v * std::pow(10, sigs) + 0.5)) * std::pow(10, -sigs));
}
float InvSqrt(float x);

// The intent of this library is to provide the bulk of the functionality
// you need to write programs that frequently use small, fixed-size vectors
// and matrices, in domains such as computational geometry or computer
// graphics. It strives for terse, readable source code.
//
// The original author of this software is Sterling Orsten, and its permanent
// home is <http://github.com/sgorsten/linalg/>. If you find this software
// useful, an acknowledgement in your source text and/or product documentation
// is appreciated, but not required.
//
// The author acknowledges significant insights and contributions by:
//     Stan Melax <http://github.com/melax/>
//     Dimitri Diakopoulos <http://github.com/ddiakopoulos/>

// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// For more information, please refer to <http://unlicense.org/>

template<class T, int M> struct vec;
template<class T> struct vec<T, 1> {
	T x = T(0);
	vec(T* ptr) :
			x(*ptr) {
	}
	explicit vec(T s = (T) 0) :
			x(s) {
	}
	inline size_t size() const {
		return 1;
	}
	template<class U> explicit vec(const vec<U, 1> & r) :
			x(T(r.x)) {
	}
	const T & operator [](size_t i) const {
		return x;
	}
	T & operator [](size_t i) {
		return x;
	}
	operator T() const {
		return x;
	}
	bool operator ==(const vec & r) const {
		return (x == r.x);
	}
	bool operator !=(const vec & r) const {
		return (x != r.x);
	}
	bool operator <(const vec & r) const {
		return (x < r.x);
	}
	bool operator >(const vec & r) const {
		return (x > r.x);
	}
	vec<T, 1>& operator =(const T& r) {
		x = r;
		return *this;
	}
	template<class Archive> void serialize(Archive & archive) {
		archive(CEREAL_NVP(x));
	}
};

// A vector with exactly M elements, each of which is an instance of type T. Can also be thought of as an M x 1 matrix.
template<class T, int M> struct vec;
template<class T> struct vec<T, 2> {
	T x, y;

	vec(T x, T y) :
			x(x), y(y) {
	}
	vec(T* ptr) :
			x(ptr[0]), y(ptr[1]) {
	}
	explicit vec(T s = (T) 0) :
			x(s), y(s) {
	}
	inline size_t size() const {
		return 2;
	}
	template<class U> explicit vec(const vec<U, 2> & r) :
			x(T(r.x)), y(T(r.y)) {
	}

	const T & operator [](size_t i) const {
		return (&x)[i];
	}
	T & operator [](size_t i) {
		return (&x)[i];
	}
	vec<T, 3> xyz() const;

	bool operator ==(const vec & r) const {
		return (x == r.x && y == r.y);
	}
	bool operator !=(const vec & r) const {
		return (x != r.x || y != r.y);
	}
	bool operator <(const vec & r) const {
		return (std::make_tuple(x, y) < std::make_tuple(r.x, r.y));
	}
	bool operator >(const vec & r) const {
		return (std::make_tuple(x, y) > std::make_tuple(r.x, r.y));
	}
	template<class Archive> void serialize(Archive & archive) {
		archive(CEREAL_NVP(x), CEREAL_NVP(y));
	}

};
template<class T> struct vec<T, 3> {
	T x, y, z;

	vec(T x, T y, T z) :
			x(x), y(y), z(z) {
	}
	vec(T* ptr) :
			x(ptr[0]), y(ptr[1]), z(ptr[2]) {
	}
	vec(vec<T, 2> xy, T z) :
			x(xy.x), y(xy.y), z(z) {
	}
	explicit vec(T s = (T) 0) :
			x(s), y(s), z(s) {
	}
	inline size_t size() const {
		return 3;
	}
	template<class U> explicit vec(const vec<U, 3> & r) :
			x(T(r.x)), y(T(r.y)), z(T(r.z)) {
	}
	const T & operator [](size_t i) const {
		return (&x)[i];
	}
	T & operator [](size_t i) {
		return (&x)[i];
	}
	vec<T, 2> xy() const {
		return vec<T, 2>(x, y);
	}
	vec<T, 4> xyzw() const;

	bool operator ==(const vec & r) const {
		return (x == r.x && y == r.y && z == r.z);
	}
	bool operator !=(const vec & r) const {
		return (x != r.x || y != r.y || z != r.z);
	}
	bool operator <(const vec & r) const {
		return (std::make_tuple(x, y, z) < std::make_tuple(r.x, r.y, r.z));
	}
	bool operator >(const vec & r) const {
		return (std::make_tuple(x, y, z) > std::make_tuple(r.x, r.y, r.z));
	}
	template<class Archive> void serialize(Archive & archive) {
		archive(CEREAL_NVP(x), CEREAL_NVP(y), CEREAL_NVP(z));
	}

};
template<class T> struct vec<T, 4> {
	T x, y, z, w;

	vec(T x, T y, T z, T w) :
			x(x), y(y), z(z), w(w) {
	}
	vec(T* ptr) :
			x(ptr[0]), y(ptr[1]), z(ptr[2]), w(ptr[3]) {
	}
	vec(vec<T, 3> xyz, T w) :
			x(xyz.x), y(xyz.y), z(xyz.z), w(w) {
	}
	explicit vec(T s = (T) 0) :
			x(s), y(s), z(s), w(s) {
	}
	inline size_t size() const {
		return 4;
	}
	template<class Archive> void serialize(Archive & archive) {
		archive(CEREAL_NVP(x), CEREAL_NVP(y), CEREAL_NVP(z), CEREAL_NVP(w));
	}
	template<class U> explicit vec(const vec<U, 4> & r) :
			x(T(r.x)), y(T(r.y)), z(T(r.z)), w(T(r.w)) {
	}

	const T & operator [](size_t i) const {
		return (&x)[i];
	}
	T & operator [](size_t i) {
		return (&x)[i];
	}
	vec<T, 3> xyz() const {
		return vec<T, 3>(x, y, z);
	}
	vec<T, 3> yzw() const {
		return vec<T, 3>(y, z, w);
	}
	vec<T, 2> xy() const {
		return vec<T, 2>(x, y);
	}

	bool operator ==(const vec & r) const {
		return (x == r.x && y == r.y && z == r.z && w == r.w);
	}
	bool operator !=(const vec & r) const {
		return (x != r.x || y != r.y || z != r.z || w != r.w);
	}
	bool operator <(const vec & r) const {
		return (std::make_tuple(x, y, z, w)
				< std::make_tuple(r.x, r.y, r.z, r.w));
	}
	bool operator >(const vec & r) const {
		return (std::make_tuple(x, y, z, w)
				> std::make_tuple(r.x, r.y, r.z, r.w));
	}

};
template<class T> vec<T, 3> vec<T, 2>::xyz() const {
	return vec<T, 3>(x, y, 1);
}
template<class T> vec<T, 4> vec<T, 3>::xyzw() const {
	return vec<T, 4>(x, y, z, 1);
}

// A matrix with exactly M rows and N columns, where each element is an instance of type T
template<class T, int M, int N> struct matrix;

template<class T, int M, int N> const matrix<T, M, N> Identity() {
	static matrix<T, M, N> Id;
	static bool once = true;
	if (once) {
		for (int m = 0; m < M; m++) {
			for (int n = 0; n < N; n++) {
				Id(m, n) = (T) ((m == n) ? 1 : 0);
			}
		}
		once = false;
	}
	return Id;
}
;
template<class T, int M, int N> const matrix<T, M, N> Zero() {
	static matrix<T, M, N> Zero;
	static bool once = true;
	if (once) {
		for (int m = 0; m < M; m++) {
			for (int n = 0; n < N; n++) {
				Zero(m, n) = 0;
			}
		}
		once = false;
	}
	return Zero;
}
;
template<class T, int M, int N> const matrix<T, M - 1, N - 1> SubMatrix(
		matrix<T, M, N> A) {
	static matrix<T, M - 1, N - 1> B;
	for (int m = 0; m < M - 1; m++) {
		for (int n = 0; n < N - 1; n++) {
			B(m, n) = A(m, n);
		}
	}
	return B;
}
;
template<class T, int M, int N> const matrix<T, M - 1, N> SubRowMatrix(
		matrix<T, M, N> A) {
	static matrix<T, M - 1, N> B;
	for (int m = 0; m < M - 1; m++) {
		for (int n = 0; n < N; n++) {
			B(m, n) = A(m, n);
		}
	}
	return B;
}
;
template<class T, int M, int N> const matrix<T, M, N - 1> SubColMatrix(
		matrix<T, M, N> A) {
	static matrix<T, M, N - 1> B;
	for (int m = 0; m < M; m++) {
		for (int n = 0; n < N - 1; n++) {
			B(m, n) = A(m, n);
		}
	}
	return B;
}
;
template<class T, int M> struct matrix<T, M, 2> {
	typedef vec<T, M> C;
	C x, y;
	template<class Archive> void serialize(Archive & archive) {
		archive(CEREAL_NVP(x), CEREAL_NVP(y));
	}
	matrix(const T (&A)[M * 2]) {
		for (int m = 0; m < M; m++) {
			x[m] = A[m * 2];
			y[m] = A[m * 2 + 1];
		}
	}
	matrix() {
	}
	matrix(C x, C y) :
			x(x), y(y) {
	}
	explicit matrix(T s) :
			x(s), y(s) {
	}
	matrix(T a00, T a01, T a10, T a11) {

		x.x = a00;
		x.y = a10;
		y.x = a01;
		y.y = a11;
	}
	template<class U> explicit matrix(const matrix<U, M, 2> & r) :
			x(C(r.x)), y(C(r.y)) {
	}
	T* ptr() {
		return &(x.x);
	}
	const T* ptr() const {
		return &(x.x);
	}
	vec<T, M>* vecPtr() {
		return &x[0];
	}
	vec<T, 2> row(int i) const {
		return {x[i], y[i]};
	}
	const C & operator [](size_t j) const {
		return (&x)[j];
	}
	const T & operator ()(int i, int j) const {
		return (*this)[j][i];
	}
	C & operator [](size_t j) {
		return (&x)[j];
	}
	T & operator ()(int i, int j) {
		return (*this)[j][i];
	}
	static matrix<T, M, 2> zero() {
		return Zero<T, M, 2>();
	}
	static matrix<T, M, 2> identity() {
		return Identity<T, M, 2>();
	}
};

// Specialization for an M x 3 matrix, with exactly three columns, each of which is an M-element vector
template<class T, int M> struct matrix<T, M, 3> {
	typedef vec<T, M> C;
	C x, y, z;
	template<class Archive> void serialize(Archive & archive) {
		archive(CEREAL_NVP(x), CEREAL_NVP(y), CEREAL_NVP(z));
	}
	matrix() {
	}
	matrix(const T (&A)[M * 3]) {
		for (int m = 0; m < M; m++) {
			x[m] = A[m * 3];
			y[m] = A[m * 3 + 1];
			z[m] = A[m * 3 + 2];
		}
	}
	matrix(T a00, T a01, T a02, T a10, T a11, T a12, T a20, T a21, T a22) {

		x.x = a00;
		x.y = a10;
		x.z = a20;
		y.x = a01;
		y.y = a11;
		y.z = a21;
		z.x = a02;
		z.y = a12;
		z.z = a22;

	}
	matrix(T a00, T a01, T a02, T a10, T a11, T a12) {

		x.x = a00;
		x.y = a10;
		y.x = a01;
		y.y = a11;
		z.x = a02;
		z.y = a12;
	}
	matrix(C x, C y, C z) :
			x(x), y(y), z(z) {
	}
	explicit matrix(T s) :
			x(s), y(s), z(s) {
	}
	template<class U> explicit matrix(const matrix<U, M, 3> & r) :
			x(C(r.x)), y(C(r.y)), z(C(r.z)) {
	}
	T* ptr() {
		return &(x.x);
	}
	const T* ptr() const {
		return &(x.x);
	}
	vec<T, M>* vecPtr() {
		return &x[0];
	}
	vec<T, 3> row(int i) const {
		return {x[i], y[i], z[i]};
	}
	const C & operator [](size_t j) const {
		return (&x)[j];
	}
	const T & operator ()(int i, int j) const {
		return (*this)[j][i];
	}
	C & operator [](size_t j) {
		return (&x)[j];
	}
	T & operator ()(int i, int j) {
		return (*this)[j][i];
	}
	static matrix<T, M, 3> zero() {
		return Zero<T, M, 3>();
	}
	static matrix<T, M, 3> identity() {
		return Identity<T, M, 3>();
	}
};

// Specialization for an M x 4 matrix, with exactly four columns, each of which is an M-element vector
template<class T, int M> struct matrix<T, M, 4> {
	typedef vec<T, M> C;
	C x, y, z, w;
	template<class Archive> void serialize(Archive & archive) {
		archive(CEREAL_NVP(x), CEREAL_NVP(y), CEREAL_NVP(z), CEREAL_NVP(w));
	}
	matrix() {
	}
	matrix(const T (&A)[M * 4]) {
		for (int m = 0; m < M; m++) {
			x[m] = A[m * 4];
			y[m] = A[m * 4 + 1];
			z[m] = A[m * 4 + 2];
			w[m] = A[m * 4 + 3];
		}
	}
	matrix(C x, C y, C z, C w) :
			x(x), y(y), z(z), w(w) {
	}
	matrix(T a00, T a01, T a02, T a03, T a10, T a11, T a12, T a13, T a20, T a21,
			T a22, T a23, T a30, T a31, T a32, T a33) {

		x.x = a00;
		x.y = a10;
		x.z = a20;
		x.w = a30;
		y.x = a01;
		y.y = a11;
		y.z = a21;
		y.w = a31;
		z.x = a02;
		z.y = a12;
		z.z = a22;
		z.w = a32;
		w.x = a03;
		w.y = a13;
		w.z = a23;
		w.w = a33;
	}
	matrix(T a00, T a01, T a02, T a03, T a10, T a11, T a12, T a13, T a20, T a21,
			T a22, T a23) {

		x.x = a00;
		x.y = a10;
		x.z = a20;
		y.x = a01;
		y.y = a11;
		y.z = a21;
		z.x = a02;
		z.y = a12;
		z.z = a22;
		w.x = a03;
		w.y = a13;
		w.z = a23;
	}
	matrix(T a00, T a01, T a02, T a03, T a10, T a11, T a12, T a13) {

		x.x = a00;
		x.y = a10;
		y.x = a01;
		y.y = a11;
		z.x = a02;
		z.y = a12;
		w.x = a03;
		w.y = a13;
	}
	explicit matrix(T s) :
			x(s), y(s), z(s), w(s) {
	}
	template<class U> explicit matrix(const matrix<U, M, 4> & r) :
			x(C(r.x)), y(C(r.y)), z(C(r.z)), w(C(r.w)) {
	}
	T* ptr() {
		return &(x.x);
	}
	const T* ptr() const {
		return &(x.x);
	}
	vec<T, M>* vecPtr() {
		return &x[0];
	}
	vec<T, 4> row(int i) const {
		return {x[i], y[i], z[i], w[i]};
	}
	const C & operator [](size_t j) const {
		return (&x)[j];
	}
	const T & operator ()(int i, int j) const {
		return (*this)[j][i];
	}
	C & operator [](size_t j) {
		return (&x)[j];
	}
	T & operator ()(int i, int j) {
		return (*this)[j][i];
	}
	static matrix<T, M, 4> zero() {
		return Zero<T, M, 4>();
	}
	static matrix<T, M, 4> identity() {
		return Identity<T, M, 4>();
	}
};

/////////////
// Functional helpers //
////////////////////////

// elem_t<T> is a type trait for the type of the underlying elements of a tensor (scalar, vector, or matrix)
template<class T> struct elem_type {
	typedef T type;
};
template<class T, int M> struct elem_type<vec<T, M>> {
	typedef T type;
};
template<class T, int M, int N> struct elem_type<matrix<T, M, N>> {
	typedef T type;
};
template<class T> using elem_t = typename elem_type<T>::type;

// Form a scalar by applying function f to adjacent components of vector or matrix v
template<class T, class F> T reduce(const vec<T, 2> & v, F f) {
	return f(v.x, v.y);
}
template<class T, class F> T reduce(const vec<T, 3> & v, F f) {
	return f(f(v.x, v.y), v.z);
}
template<class T, class F> T reduce(const vec<T, 4> & v, F f) {
	return f(f(f(v.x, v.y), v.z), v.w);
}
template<class T, int M, class F> T reduce(const matrix<T, M, 2> & m, F f) {
	return f(reduce(m.x, f), reduce(m.y, f));
}
template<class T, int M, class F> T reduce(const matrix<T, M, 3> & m, F f) {
	return f(f(reduce(m.x, f), reduce(m.y, f)), reduce(m.z, f));
}
template<class T, int M, class F> T reduce(const matrix<T, M, 4> & m, F f) {
	return f(f(f(reduce(m.x, f), reduce(m.y, f)), reduce(m.z, f)),
			reduce(m.w, f));
}
/////////////////////////////////
// General aggregate functions //
/////////////////////////////////

// Return true if any/all elements of vector or matrix v would evaluate to true in a boolean context
template<class T> bool any(const T & v) {
	return reduce(v, [](bool a, bool b) {return a || b;});
}
template<class T> bool all(const T & v) {
	return reduce(v, [](bool a, bool b) {return a && b;});
}

// Compute the sum of all elements in vector or matrix v
template<class T> elem_t<T> sum(const T & v) {
	return reduce(v, [](elem_t<T> a, elem_t<T> b) {return a + b;});
}
template<class T> elem_t<T> product(const T & v) {
	return reduce(v, [](elem_t<T> a, elem_t<T> b) {return a * b;});
}

// Form a vector or matrix by applying std::abs/ceil/floor/round to each component of vector or matrix v
template<class T, int M> vec<T, M> abs(const vec<T, M> & v) {
	vec<T, M> result;
	for (int m = 0; m < M; m++)
		result[m] = std::abs(v[m]);
	return result;
}
template<class T, int M> vec<T, M> ceil(const vec<T, M> & v) {
	vec<T, M> result;
	for (int m = 0; m < M; m++)
		result[m] = std::ceil(v[m]);
	return result;
}
template<class T, int M> vec<T, M> floor(const vec<T, M> & v) {
	vec<T, M> result;
	for (int m = 0; m < M; m++)
		result[m] = std::floor(v[m]);
	return result;
}
template<class T, int M> vec<T, M> round(const vec<T, M> & v) {
	vec<T, M> result;
	for (int m = 0; m < M; m++)
		result[m] = std::floor(v[m] + T(0.5));
	return result;
}
template<class T, int M> vec<T, M> exp(const vec<T, M> & v) {
	vec<T, M> result;
	for (int m = 0; m < M; m++)
		result[m] = std::exp(v[m]);
	return result;
}
template<class T, int M> vec<T, M> log(const vec<T, M> & v) {
	vec<T, M> result;
	for (int m = 0; m < M; m++)
		result[m] = std::log(v[m]);
	return result;
}
template<class T, int M> vec<T, M> sqrt(const vec<T, M> & v) {
	vec<T, M> result;
	for (int m = 0; m < M; m++)
		result[m] = std::sqrt(v[m]);
	return result;
}
template<class T> T sqrt(const T& v) {
	return T(std::sqrt(double(v)));
}
template<class T, int M> vec<T, M> pow(const vec<T, M> & v,
		const vec<T, M> & p) {
	vec<T, M> result;
	for (int m = 0; m < M; m++)
		result[m] = std::pow(v[m], p[m]);
	return result;
}
template<class T, int M> vec<T, M> pow(const vec<T, M> & v, const T& p) {
	vec<T, M> result;
	for (int m = 0; m < M; m++)
		result[m] = std::pow(v[m], p);
	return result;
}
template<class T> T pow(const T& v, const T& p) {
	return T(std::pow(double(v), double(p)));
}
inline float sqr(const float& v) {
	return v * v;
}
inline double sqr(const double& v) {
	return v * v;
}
inline int sqr(const int& v) {
	return v * v;
}
inline int64_t sqr(const int64_t& v) {
	return v * v;
}
inline uint64_t sqr(const uint64_t& v) {
	return v * v;
}
inline uint32_t sqr(const uint32_t& v) {
	return v * v;
}
inline float ToDegrees(float angle) {
	return (float) (angle * 180.0f / ALY_PI);
}
inline double ToDegrees(double angle) {
	return (double) (angle * 180.0 / ALY_PI);
}
inline float ToRadians(float angle) {
	return (float) (angle * ALY_PI / 180.0f);
}
inline double ToRadians(double angle) {
	return (double) (angle * ALY_PI / 180.0);
}
template<int M> vec<float, M> sqr(const vec<float, M>& v) {
	return v * v;
}
template<int M> vec<double, M> sqr(const vec<double, M>& v) {
	return v * v;
}
template<class T, int M> vec<T, M> clamp(const vec<T, M> & v, const T& mn,
		const T& mx) {
	vec<T, M> result;
	for (int m = 0; m < M; m++)
		result[m] = clamp(v[m], mn, mx);
	return result;
}
template<class T, int M> vec<T, M> clamp(const vec<T, M> & v,
		const vec<T, M>& mn, const vec<T, M>& mx) {
	vec<T, M> result;
	for (int m = 0; m < M; m++)
		result[m] = clamp(v[m], mn[m], mx[m]);
	return result;
}
// Form a vector or matrix by taking the componentwise max/min of two vectors or matrices
template<class T, int M> vec<T, M> max(const vec<T, M> & l,
		const vec<T, M> & r) {
	vec<T, M> result;
	for (int m = 0; m < M; m++)
		result[m] = std::max(l[m], r[m]);
	return result;
}
template<class T, int M> vec<T, M> min(const vec<T, M> & l,
		const vec<T, M> & r) {
	vec<T, M> result;
	for (int m = 0; m < M; m++)
		result[m] = std::min(l[m], r[m]);
	return result;
}
template<class T, int M> vec<T, M> maxVec(const vec<T, M> & l,
		const vec<T, M> & r) {
	vec<T, M> result;
	for (int m = 0; m < M; m++)
		result[m] = std::max(l[m], r[m]);
	return result;
}
template<class T, int M> vec<T, M> minVec(const vec<T, M> & l,
		const vec<T, M> & r) {
	vec<T, M> result;
	for (int m = 0; m < M; m++)
		result[m] = std::min(l[m], r[m]);
	return result;
}
template<class T, int M> T max(const vec<T, M> & l) {
	T result = std::numeric_limits<T>::min();
	for (int m = 0; m < M; m++)
		result = std::max(l[m], result);
	return result;
}
template<class T, int M> T min(const vec<T, M> & l) {
	T result = std::numeric_limits<T>::max();
	for (int m = 0; m < M; m++)
		result = std::min(l[m], result);
	return result;
}

template<class T, int M> vec<T, M> mix(const vec<T, M> & a, const vec<T, M> & b,
		const T& t) {
	return vec<T, M>(
			vec<float, M>(a) * (1.0f - (float) t) + vec<float, M>(b) * t);
}
template<class T, int M> vec<T, M> mix(const vec<T, M> & a, const vec<T, M> & b,
		const double t) {
	return vec<T, M>(vec<double, M>(a) * (1 - t) + vec<double, M>(b) * (t));
}
template<class T> T mix(const T & a, const T & b, double t) {
	return (T) ((double) a * (1.0 - t) + (double) b * (t));
}
template<class T, int M> vec<T, M> mix(const vec<T, M> & a, const vec<T, M> & b,
		const vec<T, M> & t) {
	return vec<T, M>(
			vec<float, M>(a) * (vec<float, M>(1) - t) + vec<float, M>(b) * t);
}
//////////////////////////////
// Vector algebra functions //
//////////////////////////////

// Compute the dot/cross product of two vectors
template<class T, int N> T dot(const vec<T, N> & l, const vec<T, N> & r) {
	return sum(l * r);
}
template<class T> vec<T, 3> cross(const vec<T, 3> & l, const vec<T, 3> & r) {
	return vec<T, 3>(l.y * r.z - l.z * r.y, l.z * r.x - l.x * r.z,
			l.x * r.y - l.y * r.x);
}
template<class T> vec<T, 3> cross(const vec<T, 2> & l, const vec<T, 2> & r) {
	return vec<T, 3>(0, 0, l.x * r.y - l.y * r.x);
}
template<class T> T crossMag(const vec<T, 2> & l, const vec<T, 2> & r) {
	return l.x * r.y - l.y * r.x;
}
template<class T> T crossMag(const vec<T, 3> & l, const vec<T, 3> & r) {
	return length(
			vec<T, 3>(l.y * r.z - l.z * r.y, l.z * r.x - l.x * r.z,
					l.x * r.y - l.y * r.x));
}

// Compute the length/square length of a vector
template<class T, int N> T lengthSqr(const vec<T, N> & v) {
	return dot(v, v);
}
template<class T, int N> T lengthL1(const vec<T, N> & v) {
	T sum = 0;
	for (int n = 0; n < N; n++) {
		sum += std::abs(v[n]);
	}
	return sum;
}
template<class T, int N> T lengthInf(const vec<T, N>& a) {
	T ans(0);
	for (int i = 0; i < N; i++) {
		ans = std::max(ans, std::abs(a[i]));
	}
	return ans;
}
template<class T, int N> T length(const vec<T, N> & v) {
	return std::sqrt(lengthSqr(v));
}
template<class T> vec<T, 3> reflect(const vec<T, 3> & I, const vec<T, 3> & N) {
	return (I - T(2) * dot(N, I) * N);
}
template<class T> vec<T, 2> reflect(const vec<T, 2> & I, const vec<T, 2> & N) {
	return (I - T(2) * dot(N, I) * N);
}
// Compute the distance/square distance between two points, expressed as vectors
template<class T, int N> T distance(const vec<T, N> & l, const vec<T, N> & r) {
	return length(r - l);
}
template<class T, int N> T distanceSqr(const vec<T, N> & l,
		const vec<T, N> & r) {
	return lengthSqr(r - l);
}

// Compute a normalized vector with the same direction as the original vector
template<class T, int N> vec<T, N> normalize(const vec<T, N> & v,
		const double eps = 1E-6) {
	return v / std::max(length(v), static_cast<T>(eps));
}
template<class T, int N> void unitize(vec<T, N> & v) {
	T len = length(v);
	if (len != 0 && len != 1) {
		v /= len;
	}
}
template<class T> inline vec<T, 2> proj(const vec<T, 3>& v) {
	vec<T, 2> u(v[0], v[1]);
	if (v[2] != 1.0 && v[2] != 0.0)
		u /= v[2];
	return u;
}
;
template<class T> inline vec<T, 3> proj(const vec<T, 4>& v) {
	vec<T, 3> u(v[0], v[1], v[2]);
	if (v[3] != 1.0 && v[3] != 0.0)
		u /= v[3];
	return u;
}
;

template<class T, int M> vec<T, M> operator -(const vec<T, M> & a) {
	vec<T, M> result;
	for (int m = 0; m < M; m++) {
		result[m] = -a[m];
	};
	return result;
}
template<class T, int M> vec<T, M> operator +(const vec<T, M> & a,
		const vec<T, M> & b) {
	vec<T, M> result;
	for (int m = 0; m < M; m++) {
		result[m] = a[m] + b[m];
	};
	return result;
}
template<class T, int M> vec<T, M> operator -(const vec<T, M> & a,
		const vec<T, M> & b) {
	vec<T, M> result;
	for (int m = 0; m < M; m++) {
		result[m] = a[m] - b[m];
	};
	return result;
}
template<class T, int M> vec<T, M> operator *(const vec<T, M> & a,
		const vec<T, M> & b) {
	vec<T, M> result;
	for (int m = 0; m < M; m++) {
		result[m] = a[m] * b[m];
	};
	return result;
}
template<class T, int M> vec<T, M> operator /(const vec<T, M> & a,
		const vec<T, M> & b) {
	vec<T, M> result;
	for (int m = 0; m < M; m++) {
		result[m] = a[m] / b[m];
	};
	return result;
}

template<class T, int M> vec<T, M> operator +(const vec<T, M> & a,
		const T & b) {
	vec<T, M> result;
	for (int m = 0; m < M; m++) {
		result[m] = a[m] + b;
	};
	return result;
}
template<class T, int M> vec<T, M> operator -(const vec<T, M> & a,
		const T & b) {
	vec<T, M> result;
	for (int m = 0; m < M; m++) {
		result[m] = a[m] - b;
	};
	return result;
}
template<class T, int M> vec<T, M> operator *(const vec<T, M> & a,
		const T & b) {
	vec<T, M> result;
	for (int m = 0; m < M; m++) {
		result[m] = a[m] * b;
	};
	return result;
}
template<class T, int M> vec<T, M> operator /(const vec<T, M> & a,
		const T & b) {
	vec<T, M> result;
	for (int m = 0; m < M; m++) {
		result[m] = a[m] / b;
	};
	return result;
}

template<class T, int M> vec<T, M> operator +(const T & a,
		const vec<T, M> & b) {
	vec<T, M> result;
	for (int m = 0; m < M; m++) {
		result[m] = a + b[m];
	};
	return result;
}
template<class T, int M> vec<T, M> operator -(const T & a,
		const vec<T, M> & b) {
	vec<T, M> result;
	for (int m = 0; m < M; m++) {
		result[m] = a - b[m];
	};
	return result;
}
template<class T, int M> vec<T, M> operator *(const T & a,
		const vec<T, M> & b) {
	vec<T, M> result;
	for (int m = 0; m < M; m++) {
		result[m] = a * b[m];
	};
	return result;
}
template<class T, int M> vec<T, M> operator /(const T & a,
		const vec<T, M> & b) {
	vec<T, M> result;
	for (int m = 0; m < M; m++) {
		result[m] = a / b[m];
	};
	return result;
}

template<class T, int M> vec<T, M> & operator +=(vec<T, M> & a,
		const vec<T, M> & b) {
	return a = a + b;
}
template<class T, int M> vec<T, M> & operator -=(vec<T, M> & a,
		const vec<T, M> & b) {
	return a = a - b;
}
template<class T, int M> vec<T, M> & operator *=(vec<T, M> & a,
		const vec<T, M> & b) {
	return a = a * b;
}
template<class T, int M> vec<T, M> & operator /=(vec<T, M> & a,
		const vec<T, M> & b) {
	return a = a / b;
}
template<class T, int M> vec<T, M> & operator +=(vec<T, M> & a, const T & b) {
	return a = a + b;
}
template<class T, int M> vec<T, M> & operator -=(vec<T, M> & a, const T & b) {
	return a = a - b;
}
template<class T, int M> vec<T, M> & operator *=(vec<T, M> & a, const T & b) {
	return a = a * b;
}
template<class T, int M> vec<T, M> & operator /=(vec<T, M> & a, const T & b) {
	return a = a / b;
}

template<class T, int M, int N> matrix<T, M, N> operator -(
		const matrix<T, M, N> & a) {
	matrix<T, M, N> result;
	for (int i = 0; i < N; i++) {
		result[i] = -a[i];
	};
	return result;
}
template<class T, int M, int N> matrix<T, M, N> operator +(
		const matrix<T, M, N> & a, const matrix<T, M, N> & b) {
	matrix<T, M, N> result;
	for (int i = 0; i < N; i++) {
		result[i] = a[i] + b[i];
	};
	return result;
}
template<class T, int M, int N> matrix<T, M, N> operator -(
		const matrix<T, M, N> & a, const matrix<T, M, N> & b) {
	matrix<T, M, N> result;
	for (int i = 0; i < N; i++) {
		result[i] = a[i] - b[i];
	};
	return result;
}
template<class T, int M, int N> matrix<T, M, N> operator +(
		const matrix<T, M, N> & a, T b) {
	matrix<T, M, N> result;
	for (int i = 0; i < N; i++) {
		result[i] = a[i] + b;
	};
	return result;
}
template<class T, int M, int N> matrix<T, M, N> operator -(
		const matrix<T, M, N> & a, T b) {
	matrix<T, M, N> result;
	for (int i = 0; i < N; i++) {
		result[i] = a[i] - b;
	};
	return result;
}
template<class T, int M, int N> matrix<T, M, N> operator *(
		const matrix<T, M, N> & a, T b) {
	matrix<T, M, N> result;
	for (int i = 0; i < N; i++) {
		result[i] = a[i] * b;
	};
	return result;
}
template<class T, int M, int N> matrix<T, M, N> operator /(
		const matrix<T, M, N> & a, T b) {
	matrix<T, M, N> result;
	for (int i = 0; i < N; i++) {
		result[i] = a[i] / b;
	};
	return result;
}
template<class T, int M, int N> matrix<T, M, N> operator *(T a,
		const matrix<T, M, N> & b) {
	matrix<T, M, N> result;
	for (int i = 0; i < N; i++) {
		result[i] = a * b[i];
	};
	return result;
}
template<class T, int M, int N> matrix<T, M, N> operator /(T a,
		const matrix<T, M, N> & b) {
	matrix<T, M, N> result;
	for (int i = 0; i < N; i++) {
		result[i] = a / b[i];
	};
	return result;
}
template<class T, int M, int N> matrix<T, M, N> operator +(T a,
		const matrix<T, M, N> & b) {
	matrix<T, M, N> result;
	for (int i = 0; i < N; i++) {
		result[i] = a + b[i];
	};
	return result;
}
template<class T, int M, int N> matrix<T, M, N> operator -(T a,
		const matrix<T, M, N> & b) {
	matrix<T, M, N> result;
	for (int i = 0; i < N; i++) {
		result[i] = a - b[i];
	};
	return result;
}

template<class T, int M, int N> matrix<T, M, N> & operator +=(
		matrix<T, M, N> & a, const matrix<T, M, N> & b) {
	return a = a + b;
}
template<class T, int M, int N> matrix<T, M, N> & operator -=(
		matrix<T, M, N> & a, const matrix<T, M, N> & b) {
	return a = a - b;
}
template<class T, int M, int N> matrix<T, M, N> & operator *=(
		matrix<T, M, N> & a, T b) {
	return a = a * b;
}
template<class T, int M, int N> matrix<T, M, N> & operator /=(
		matrix<T, M, N> & a, T b) {
	return a = a / b;
}

//////////////////////////////////////
// Matrix multiplication operations //
//////////////////////////////////////

template<class T, int M, int N> vec<T, M> operator *(const matrix<T, M, N> & a,
		const vec<T, N> & b) {
	return mul(a, b);
} // Interpret b as column vector (N x 1 matrix)
template<class T, int M, int N> vec<T, N> operator *(const vec<T, M> & a,
		const matrix<T, M, N> & b) {
	return mul(transpose(b), a);
} // Interpret a as row vector (1 x M matrix)
template<class T, int M, int N, int P> matrix<T, M, P> operator *(
		const matrix<T, M, N> & a, const matrix<T, N, P> & b) {
	return mul(a, b);
}
template<class T, int M> matrix<T, M, M> & operator *=(matrix<T, M, M> & a,
		const matrix<T, M, M> & b) {
	return a = a * b;
}

template<class C, class R, class T> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const vec<T, 1> & v) {
	return ss << v.x;
}
template<class C, class R, class T> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const vec<T, 2> & v) {
	return ss << "(" << v.x << ", " << v.y << ")";
}
template<class C, class R, class T> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const vec<T, 3> & v) {
	return ss << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}
template<class C, class R, class T> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const vec<T, 4> & v) {
	return ss << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
}
template<class C, class R, class T> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss,
		const std::pair<vec<T, 1>, vec<T, 1>> & v) {
	return ss << "<" << v.first << ", " << v.second << ">";
}
template<class C, class R, class T> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss,
		const std::pair<vec<T, 2>, vec<T, 2>> & v) {
	return ss << "<" << v.first << ", " << v.second << ">";
}
template<class C, class R, class T> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss,
		const std::pair<vec<T, 3>, vec<T, 3>> & v) {
	return ss << "<" << v.first << ", " << v.second << ">";
}
template<class C, class R, class T> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss,
		const std::pair<vec<T, 4>, vec<T, 4>> & v) {
	return ss << "<" << v.first << ", " << v.second << ">";
}
template<class T> inline std::istream &operator>>(std::istream &in,
		vec<T, 4>& v) {
	return in >> v[0] >> v[1] >> v[2] >> v[3];
}
;

template<class T> inline std::istream &operator>>(std::istream &in,
		vec<T, 3>& v) {
	return in >> v[0] >> v[1] >> v[2];
}
;

template<class T> inline std::istream &operator>>(std::istream &in,
		vec<T, 2>& v) {
	return in >> v[0] >> v[1];
}
;

template<class T> inline std::istream &operator>>(std::istream &in,
		vec<T, 1>& v) {
	return in >> v[0];
}
;

template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const vec<uint8_t, 1> & v) {
	return ss << (int) v.x;
}
template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const vec<uint8_t, 2> & v) {
	return ss << "(" << (int) v.x << ", " << (int) v.y << ")";
}
template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const vec<uint8_t, 3> & v) {
	return ss << "(" << (int) v.x << ", " << (int) v.y << ", " << (int) v.z
			<< ")";
}
template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const vec<uint8_t, 4> & v) {
	return ss << "(" << (int) v.x << ", " << (int) v.y << ", " << (int) v.z
			<< ", " << (int) v.w << ")";
}

template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const vec<int8_t, 1> & v) {
	return ss << (int) v.x;
}
template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const vec<int8_t, 2> & v) {
	return ss << "(" << (int) v.x << ", " << (int) v.y << ")";
}
template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const vec<int8_t, 3> & v) {
	return ss << "(" << (int) v.x << ", " << (int) v.y << ", " << (int) v.z
			<< ")";
}
template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const vec<int8_t, 4> & v) {
	return ss << "(" << (int) v.x << ", " << (int) v.y << ", " << (int) v.z
			<< ", " << (int) v.w << ")";
}

template<class C, class R, class T, int M, int N> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const matrix<T, M, N> & A) {
	ss << "\n";

	for (int m = 0; m < M; m++) {
		ss << "[";
		for (int n = 0; n < N; n++) {
			ss << std::setprecision(10) << std::setw(16) << A(m, n)
					<< ((n < N - 1) ? "," : "]\n");
		}
	}
	return ss;
}
template<class T> T Angle(const vec<T, 3>& v0, const vec<T, 3>& v1,
		const vec<T, 3>& v2) {
	vec<T, 3> v = v0 - v1;
	vec<T, 3> w = v2 - v1;
	float len1 = length(v);
	float len2 = length(w);
	return std::acos(dot(v, w) / std::max(1E-8f, len1 * len2));
}
template<class T> T Angle(const vec<T, 2>& v0, const vec<T, 2>& v1,
		const vec<T, 2>& v2) {
	vec<T, 2> v = normalize(v0 - v1);
	vec<T, 2> w = normalize(v2 - v1);
	return std::asin(crossMag(v, w));
}
// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 1.0.0 (2014/08/11)
//----------------------------------------------------------------------------
template<class T> matrix<T, 2, 2> inverse(matrix<T, 2, 2> const& M) {
	matrix<T, 2, 2> result;
	T det = M(0, 0) * M(1, 1) - M(0, 1) * M(1, 0);
	if (det != (T) 0) {
		T invDet = ((T) 1) / det;
		result = matrix<T, 2, 2>(
				{ M(1, 1) * invDet, -M(0, 1) * invDet, -M(1, 0) * invDet, M(0,
						0) * invDet });
	} else {
		result = Zero<T, 2, 2>();
		throw std::runtime_error("Could not invert matrix.");
	}

	return result;
}
template<class T> matrix<T, 2, 2> outerProd(const vec<T, 2> &a,
		const vec<T, 2>& b) {
	return matrix<T, 2, 2>(b.x * a, b.y * a);
}
template<class T> matrix<T, 3, 3> outerProd(const vec<T, 3> &a,
		const vec<T, 3>& b) {
	return matrix<T, 3, 3>(b.x * a, b.y * a, b.z * a);
}
template<class T> matrix<T, 4, 4> outerProd(const vec<T, 4> &a,
		const vec<T, 4>& b) {
	return matrix<T, 4, 4>(b.x * a, b.y * a, b.z * a, b.w * a);
}
template<typename T>
matrix<T, 3, 3> inverse(matrix<T, 3, 3> const& M) {
	matrix<T, 3, 3> result;
	T c00 = M(1, 1) * M(2, 2) - M(1, 2) * M(2, 1);
	T c10 = M(1, 2) * M(2, 0) - M(1, 0) * M(2, 2);
	T c20 = M(1, 0) * M(2, 1) - M(1, 1) * M(2, 0);
	T det = determinant(M);
	if (det != (T) 0) {
		T invDet = ((T) 1) / det;
		result = matrix<T, 3, 3>(
				{ c00 * invDet, (M(0, 2) * M(2, 1) - M(0, 1) * M(2, 2))
						* invDet, (M(0, 1) * M(1, 2) - M(0, 2) * M(1, 1))
						* invDet, c10 * invDet, (M(0, 0) * M(2, 2)
						- M(0, 2) * M(2, 0)) * invDet, (M(0, 2) * M(1, 0)
						- M(0, 0) * M(1, 2)) * invDet, c20 * invDet, (M(0, 1)
						* M(2, 0) - M(0, 0) * M(2, 1)) * invDet, (M(0, 0)
						* M(1, 1) - M(0, 1) * M(1, 0)) * invDet });
	} else {
		result = Zero<T, 3, 3>();
		throw std::runtime_error("Could not invert matrix.");
	}
	return result;
}
template<typename T> matrix<T, 3, 3> MakeSkew(const vec<T, 3>& v) {
	return matrix<T, 3, 3>(0, -v.z, v.y, v.z, 0, -v.x, -v.y, v.x, 0);
}
//----------------------------------------------------------------------------
template<class T> matrix<T, 4, 4> inverse(matrix<T, 4, 4> const& M) {
	matrix<T, 4, 4> result;
	T a0 = M(0, 0) * M(1, 1) - M(0, 1) * M(1, 0);
	T a1 = M(0, 0) * M(1, 2) - M(0, 2) * M(1, 0);
	T a2 = M(0, 0) * M(1, 3) - M(0, 3) * M(1, 0);
	T a3 = M(0, 1) * M(1, 2) - M(0, 2) * M(1, 1);
	T a4 = M(0, 1) * M(1, 3) - M(0, 3) * M(1, 1);
	T a5 = M(0, 2) * M(1, 3) - M(0, 3) * M(1, 2);
	T b0 = M(2, 0) * M(3, 1) - M(2, 1) * M(3, 0);
	T b1 = M(2, 0) * M(3, 2) - M(2, 2) * M(3, 0);
	T b2 = M(2, 0) * M(3, 3) - M(2, 3) * M(3, 0);
	T b3 = M(2, 1) * M(3, 2) - M(2, 2) * M(3, 1);
	T b4 = M(2, 1) * M(3, 3) - M(2, 3) * M(3, 1);
	T b5 = M(2, 2) * M(3, 3) - M(2, 3) * M(3, 2);
	T det = a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0;
	if (det != (T) 0) {
		T invDet = ((T) 1) / det;
		result = matrix<T, 4, 4>(
				{ (+M(1, 1) * b5 - M(1, 2) * b4 + M(1, 3) * b3) * invDet, (-M(0,
						1) * b5 + M(0, 2) * b4 - M(0, 3) * b3) * invDet, (+M(3,
						1) * a5 - M(3, 2) * a4 + M(3, 3) * a3) * invDet, (-M(2,
						1) * a5 + M(2, 2) * a4 - M(2, 3) * a3) * invDet, (-M(1,
						0) * b5 + M(1, 2) * b2 - M(1, 3) * b1) * invDet, (+M(0,
						0) * b5 - M(0, 2) * b2 + M(0, 3) * b1) * invDet, (-M(3,
						0) * a5 + M(3, 2) * a2 - M(3, 3) * a1) * invDet, (+M(2,
						0) * a5 - M(2, 2) * a2 + M(2, 3) * a1) * invDet, (+M(1,
						0) * b4 - M(1, 1) * b2 + M(1, 3) * b0) * invDet, (-M(0,
						0) * b4 + M(0, 1) * b2 - M(0, 3) * b0) * invDet, (+M(3,
						0) * a4 - M(3, 1) * a2 + M(3, 3) * a0) * invDet, (-M(2,
						0) * a4 + M(2, 1) * a2 - M(2, 3) * a0) * invDet, (-M(1,
						0) * b3 + M(1, 1) * b1 - M(1, 2) * b0) * invDet, (+M(0,
						0) * b3 - M(0, 1) * b1 + M(0, 2) * b0) * invDet, (-M(3,
						0) * a3 + M(3, 1) * a1 - M(3, 2) * a0) * invDet, (+M(2,
						0) * a3 - M(2, 1) * a1 + M(2, 2) * a0) * invDet });
	} else {
		result = Zero<T, 4, 4>();
		throw std::runtime_error("Could not invert matrix.");
	}
	return result;
}
template<class T, int M> struct box {
	vec<T, M> position;
	vec<T, M> dimensions;
	box() :
			position((T) 0), dimensions((T) 0) {
	}
	template<class Archive> void serialize(Archive & archive) {
		archive(CEREAL_NVP(position), CEREAL_NVP(dimensions));
	}
	box(vec<T, M> pt, vec<T, M> dims) :
			position(pt), dimensions(dims) {
	}
	inline bool contains(const vec<T, M>& qt) const {
		for (int m = 0; m < M; m++)
			if (qt[m] < position[m] || qt[m] >= position[m] + dimensions[m])
				return false;
		return true;
	}
	inline void clamp(const box<T, M>& other) {
		dimensions = aly::min(dimensions, other.dimensions);
		position = aly::clamp(position, other.position,
				other.position + other.dimensions - dimensions);

	}
	inline vec<T, M> clamp(const vec<T, M>& qt) const {
		return aly::clamp(qt, position, position + dimensions);
	}
	inline void intersect(const box<T, M>& other) {
		vec<T, M> mn = aly::max(position, other.position);
		vec<T, M> mx = aly::min(max(), other.max());
		dimensions = mx - mn;
		for (int m = 0; m < M; m++) {
			dimensions[m] = std::max(dimensions[m], (T) 0);
		}
		position = mn;
	}

	inline void merge(const box<T, M>& other) {
		vec<T, M> mn = aly::min(position, other.position);
		vec<T, M> mx = aly::max(max(), other.max());
		dimensions = mx - mn;
		position = mn;
	}
	inline void merge(const vec<T, M>& other) {
		position = aly::min(position, other);
		dimensions = aly::max(position + dimensions, other) - position;
	}
	inline bool intersects(const box<T, M>& other) const {
		vec<T, M> mn = aly::max(position, other.position);
		vec<T, M> mx = aly::min(max(), other.max());
		vec<T, M> dims = mx - mn;
		T size = 1;
		for (int m = 0; m < M; m++) {
			dims[m] = std::max(dims[m], (T) 0);
			size *= dims[m];
		}
		return (size > 0);
	}
	inline vec<T, M> max() const {
		return position + dimensions;
	}
	inline vec<T, M> min() const {
		return position;
	}
	inline vec<T, M> center() const {
		return position + dimensions / (T) (2);
	}
	inline vec<T, M> clamp(const vec<T, M>& pt, const box<T, M>& parent) {
		return aly::clamp(pt, parent.position,
				parent.position + parent.dimensions - dimensions);
	}
	inline vec<T, M> clamp(const vec<T, M>& pt) {
		return aly::clamp(pt, position, position + dimensions);
	}
};
template<class T, int C> bool operator==(const box<T, C>& a,
		const box<T, C>& b) {
	return (a.position == b.position && a.dimensions == b.dimensions);
}
template<class T, int C> bool operator!=(const box<T, C>& a,
		const box<T, C>& b) {
	return (a.position != b.position || a.dimensions != b.dimensions);
}

template<class T, int C> box<T, C> operator *(const box<T, C>& a, const T& b) {
	box<T, C> out = a;
	out.position *= b;
	out.dimensions *= b;
	return out;
}
template<class T, int C> box<T, C> operator *(const T& b, const box<T, C>& a) {
	box<T, C> out = a;
	out.position *= b;
	out.dimensions *= b;
	return out;
}
template<class T, int C> box<T, C> operator /(const box<T, C>& a, const T& b) {
	box<T, C> out = a;
	out.position /= b;
	out.dimensions /= b;
	return out;
}
template<class T, int C> box<T, C>& operator *=(box<T, C>& a,
		const vec<T, C>& b) {
	a.position *= b;
	a.dimensions *= b;
	return a;
}
template<class T, int C> box<T, C>& operator *=(box<T, C>& a, const T& b) {
	a.position *= b;
	a.dimensions *= b;
	return a;
}
template<class T, int C> box<T, C>& operator /=(box<T, C>& a,
		const vec<T, C>& b) {
	a.position /= b;
	a.dimensions /= b;
	return a;
}

template<class T, int C> box<T, C>& operator /=(box<T, C>& a, const T& b) {
	a.position /= b;
	a.dimensions /= b;
	return a;
}
template<class T, int C> box<T, C>& operator +=(box<T, C>& a,
		const vec<T, C>& b) {
	a.position += b;
	return a;
}
template<class T, int C> box<T, C>& operator -=(box<T, C>& a,
		const vec<T, C>& b) {
	a.position -= b;
	return a;
}
template<class T, int M> struct lineseg {
public:
	vec<T, M> start;
	vec<T, M> end;
private:
	bool clip(T denom, T numer, T& t0, T& t1, T tolerance = T(1E-15)) const {
		if (denom > tolerance) {
			if (numer > denom * t1) {
				return false;
			}
			if (numer > denom * t0) {
				t0 = numer / denom;
			}
			return true;
		} else if (denom < -tolerance) {
			if (numer > denom * t0) {
				return false;
			}
			if (numer > denom * t1) {
				t1 = numer / denom;
			}
			return true;
		} else {
			return (numer <= T(0));
		}
	}
	inline int min(const std::vector<int>& a, int init =
			std::numeric_limits<int>::max()) {
		size_t sz = a.size();
		int tmp = init;
		for (size_t i = 0; i < sz; i++) {
			tmp = std::min(tmp, a[i]);
		}
		return tmp;
	}
	inline int max(const std::vector<int>& a, int init =
			std::numeric_limits<int>::min()) {
		size_t sz = a.size();
		int tmp = init;
		for (size_t i = 0; i < sz; i++) {
			tmp = std::max(a[i], tmp);
		}
		return tmp;
	}
	inline float min(const std::vector<float>& a, float init =
			std::numeric_limits<float>::max()) {
		size_t sz = a.size();
		float tmp = init;
		for (size_t i = 0; i < sz; i++) {
			tmp = std::min(tmp, a[i]);
		}
		return tmp;
	}
	inline float max(const std::vector<float>& a, float init =
			std::numeric_limits<float>::min()) {
		size_t sz = a.size();
		float tmp = init;
		for (size_t i = 0; i < sz; i++) {
			tmp = std::max(a[i], tmp);
		}
		return tmp;
	}
	inline double min(const std::vector<double>& a, double init =
			std::numeric_limits<double>::max()) {
		size_t sz = a.size();
		double tmp = init;
		for (size_t i = 0; i < sz; i++) {
			tmp = std::min(tmp, a[i]);
		}
		return tmp;
	}
	inline double max(const std::vector<double>& a, double init =
			std::numeric_limits<double>::min()) {
		size_t sz = a.size();
		double tmp = init;
		for (size_t i = 0; i < sz; i++) {
			tmp = std::max(a[i], tmp);
		}
		return tmp;
	}
public:
	lineseg(const vec<T, M>& start = vec<T, M>(T(0)), const vec<T, M>& end =
			vec<T, M>(T(0))) :
			start(start), end(end) {
	}
	inline float length() const {
		return aly::distance(start, end);
	}
	T distance(const vec<T, M>& pt) const {
		T l2 = aly::distanceSqr(start, end);
		if (l2 < 1E-15f)
			return std::min(aly::distance(pt, start), aly::distance(pt, end));
		T t = dot(pt - start, end - start) / l2;
		if (t < T(0))
			return aly::distance(pt, start);
		else if (t > T(1))
			return aly::distance(pt, end);
		return aly::distance(pt, start + t * (end - start));
	}
	vec<T, M> closest(const vec<T, M>& pt) const {
		T l2 = aly::distanceSqr(start, end);
		if (l2 < 1E-15f) {
			if (aly::distanceSqr(pt, start) < aly::distanceSqr(pt, end)) {
				return start;
			} else {
				return end;
			}
		}
		T t = dot(pt - start, end - start) / l2;
		if (t < T(0))
			return start;
		else if (t > T(1))
			return end;
		return start + t * (end - start);
	}
	bool intersects(const lineseg<T, M>& line, T& t, T& s,
			T tolerance = T(1E-15)) const {
		vec<T, M> a = (end - start);
		vec<T, M> b = (line.end - line.start);
		vec<T, M> c = (line.start - start);
		vec<T, 3> cb = cross(c, b);
		vec<T, 3> ab = cross(a, b);
		T len = lengthSqr(ab);
		if (len >= tolerance) {
			t = dot(cb, ab) / len;
			s = dot(b, start + a * t - line.start) / lengthSqr(b);
			if (t >= T(0) && t <= T(1) && s >= T(0) && s <= T(1)) {
				return true;
			} else {
				return false;
			}
		} else {
			t = std::numeric_limits<T>::max();
			s = std::numeric_limits<T>::max();
			return false;
		}
	}
	bool intersects(const vec<T, M>& origin, const vec<T, M>& ray, T& t,
			T tolerance = T(1E-15)) const {
		vec<T, M> a = (end - start);
		vec<T, M> b = ray;
		vec<T, M> c = (origin - start);
		vec<T, 3> cb = cross(c, b);
		vec<T, 3> ab = cross(a, b);
		T len = lengthSqr(ab);
		if (len >= tolerance) {
			t = dot(cb, ab) / len;
			if (t >= T(0) && t <= T(1)) {
				return true;
			} else {
				return false;
			}
		} else {
			t = std::numeric_limits<T>::max();
			return false;
		}
	}
	vec<T, M> intersects(const vec<T, M>& origin, const vec<T, M>& ray) const {
		vec<T, M> a = (end - start);
		vec<T, M> b = ray;
		vec<T, M> c = (origin - start);
		vec<T, 3> cb = cross(c, b);
		vec<T, 3> ab = cross(a, b);
		T len = lengthSqr(ab);
		T t = dot(cb, ab) / len;
		return (start + t * a);
	}
	// Geometric Tools LLC, Redmond WA 98052
	// Copyright (c) 1998-2015
	// Distributed under the Boost Software License, Version 1.0.
	// http://www.boost.org/LICENSE_1_0.txt
	// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
	bool intersects(const box<T, M>& box, T tolerance = T(1E-15)) const {
		T t0 = -std::numeric_limits<T>::max();
		T t1 = std::numeric_limits<T>::max();
		T len = aly::distance(start, end);
		if (len < tolerance)
			return box.contains(T(0.5) * (start + end));
		vec<T, M> lineDirection = (end - start) / len;
		vec<T, M> boxExtent = box.dimensions * T(0.5);
		vec<T, M> lineOrigin = start - box.position - boxExtent;

		if (M == 2) {
			if (clip(+lineDirection[0], -lineOrigin[0] - boxExtent[0], t0, t1,
					tolerance)
					&& clip(-lineDirection[0], +lineOrigin[0] - boxExtent[0],
							t0, t1, tolerance)
					&& clip(+lineDirection[1], -lineOrigin[1] - boxExtent[1],
							t0, t1, tolerance)
					&& clip(-lineDirection[1], +lineOrigin[1] - boxExtent[1],
							t0, t1, tolerance)) {
				if (t1 > t0) {
					if ((t1 <= len && t1 >= 0) || (t0 <= len && t0 >= 0))
						return true;
				} else {
					if (t0 <= len && t0 >= 0)
						return true;
				}
				return false;
			}
		} else if (M == 3) {
			if (clip(+lineDirection[0], -lineOrigin[0] - boxExtent[0], t0, t1,
					tolerance)
					&& clip(-lineDirection[0], +lineOrigin[0] - boxExtent[0],
							t0, t1, tolerance)
					&& clip(+lineDirection[1], -lineOrigin[1] - boxExtent[1],
							t0, t1, tolerance)
					&& clip(-lineDirection[1], +lineOrigin[1] - boxExtent[1],
							t0, t1, tolerance)
					&& clip(+lineDirection[2], -lineOrigin[2] - boxExtent[2],
							t0, t1, tolerance)
					&& clip(-lineDirection[2], +lineOrigin[2] - boxExtent[2],
							t0, t1, tolerance)) {
				if (t1 > t0) {
					if ((t1 <= len && t1 >= 0) || (t0 <= len && t0 >= 0))
						return true;
				} else {
					if (t0 <= len && t0 >= 0)
						return true;
				}
				return false;
			}
		}
		return false;
	}
	static const lineseg<T, M> NONE;
};
template<class T, int C> const lineseg<T, C> lineseg<T, C>::NONE =
		lineseg<T, C>(vec<T, C>(std::numeric_limits<T>::min()),
				vec<T, C>(std::numeric_limits<T>::min()));

template<class T, int C> bool operator==(const lineseg<T, C>& a,
		const lineseg<T, C>& b) {
	return (a.start == b.start && a.end == b.end);
}
template<class T, int C> bool operator!=(const lineseg<T, C>& a,
		const lineseg<T, C>& b) {
	return (a.start != b.start || a.end != b.end);
}
template<class T, int K, class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const lineseg<T, K>& line) {
	return ss << "[" << line.start << "->" << line.end << "]";
}
typedef lineseg<float, 2> lineseg2f;
typedef lineseg<float, 3> lineseg3f;
typedef lineseg<double, 2> lineseg2d;
typedef lineseg<double, 3> lineseg3d;

template<class C, class R, class T, int M> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const box<T, M> & v) {
	return ss << "{min: " << v.position << ", max: "
			<< v.position + v.dimensions << ", dimensions: " << v.dimensions
			<< "}";
}

template<class T> void ortho(const vec<T, 3>& z, vec<T, 3>& x, vec<T, 3>& y) {
	if (std::abs(z[0]) > 0.5f) {
		x[0] = z[1];
		x[1] = -z[0];
		x[2] = 0;
	} else if (std::abs(z[1]) > 0.5f) {
		x[1] = z[2];
		x[2] = -z[1];
		x[0] = 0;
	} else {
		x[2] = z[0];
		x[0] = -z[2];
		x[1] = 0;
	}
	unitize(x);
	y = cross(z, x);
}
;
template<class T> void ortho(const vec<T, 4>& z, vec<T, 4>& x, vec<T, 4>& y) {
	if (std::abs(z[0]) > 0.5) {
		x[0] = z[1];
		x[1] = -z[0];
		x[2] = 0;
	} else if (fabs(z[1]) > 0.5) {
		x[1] = z[2];
		x[2] = -z[1];
		x[0] = 0;
	} else {
		x[2] = z[0];
		x[0] = -z[2];
		x[1] = 0;
	}
	unitize(x);
	y[0] = z[1] * x[2] - z[2] * x[1];
	y[1] = z[2] * x[0] - z[0] * x[2];
	y[2] = z[0] * x[1] - z[1] * x[0];
}
;
//////////////////////////////
// Matrix algebra functions //
//////////////////////////////

// Compute the adjugate of a square matrix (equivalent to the transpose of the cofactor matrix)
template<class T> matrix<T, 2, 2> adjugate(const matrix<T, 2, 2> & a) {
	return { {a.y.y, -a.x.y}, {-a.y.x, a.x.x}};
}
template<class T> matrix<T, 3, 3> adjugate(const matrix<T, 3, 3> & a) {
	return { {a.y.y*a.z.z - a.z.y*a.y.z, a.z.y*a.x.z - a.x.y*a.z.z, a.x.y*a.y.z - a.y.y*a.x.z},
		{	a.y.z*a.z.x - a.z.z*a.y.x, a.z.z*a.x.x - a.x.z*a.z.x, a.x.z*a.y.x - a.y.z*a.x.x},
		{	a.y.x*a.z.y - a.z.x*a.y.y, a.z.x*a.x.y - a.x.x*a.z.y, a.x.x*a.y.y - a.y.x*a.x.y}};
}
template<class T> matrix<T, 4, 4> adjugate(const matrix<T, 4, 4> & a) {
	return { {a.y.y*a.z.z*a.w.w + a.w.y*a.y.z*a.z.w + a.z.y*a.w.z*a.y.w - a.y.y*a.w.z*a.z.w - a.z.y*a.y.z*a.w.w - a.w.y*a.z.z*a.y.w,
			a.x.y*a.w.z*a.z.w + a.z.y*a.x.z*a.w.w + a.w.y*a.z.z*a.x.w - a.w.y*a.x.z*a.z.w - a.z.y*a.w.z*a.x.w - a.x.y*a.z.z*a.w.w,
			a.x.y*a.y.z*a.w.w + a.w.y*a.x.z*a.y.w + a.y.y*a.w.z*a.x.w - a.x.y*a.w.z*a.y.w - a.y.y*a.x.z*a.w.w - a.w.y*a.y.z*a.x.w,
			a.x.y*a.z.z*a.y.w + a.y.y*a.x.z*a.z.w + a.z.y*a.y.z*a.x.w - a.x.y*a.y.z*a.z.w - a.z.y*a.x.z*a.y.w - a.y.y*a.z.z*a.x.w},
		{	a.y.z*a.w.w*a.z.x + a.z.z*a.y.w*a.w.x + a.w.z*a.z.w*a.y.x - a.y.z*a.z.w*a.w.x - a.w.z*a.y.w*a.z.x - a.z.z*a.w.w*a.y.x,
			a.x.z*a.z.w*a.w.x + a.w.z*a.x.w*a.z.x + a.z.z*a.w.w*a.x.x - a.x.z*a.w.w*a.z.x - a.z.z*a.x.w*a.w.x - a.w.z*a.z.w*a.x.x,
			a.x.z*a.w.w*a.y.x + a.y.z*a.x.w*a.w.x + a.w.z*a.y.w*a.x.x - a.x.z*a.y.w*a.w.x - a.w.z*a.x.w*a.y.x - a.y.z*a.w.w*a.x.x,
			a.x.z*a.y.w*a.z.x + a.z.z*a.x.w*a.y.x + a.y.z*a.z.w*a.x.x - a.x.z*a.z.w*a.y.x - a.y.z*a.x.w*a.z.x - a.z.z*a.y.w*a.x.x},
		{	a.y.w*a.z.x*a.w.y + a.w.w*a.y.x*a.z.y + a.z.w*a.w.x*a.y.y - a.y.w*a.w.x*a.z.y - a.z.w*a.y.x*a.w.y - a.w.w*a.z.x*a.y.y,
			a.x.w*a.w.x*a.z.y + a.z.w*a.x.x*a.w.y + a.w.w*a.z.x*a.x.y - a.x.w*a.z.x*a.w.y - a.w.w*a.x.x*a.z.y - a.z.w*a.w.x*a.x.y,
			a.x.w*a.y.x*a.w.y + a.w.w*a.x.x*a.y.y + a.y.w*a.w.x*a.x.y - a.x.w*a.w.x*a.y.y - a.y.w*a.x.x*a.w.y - a.w.w*a.y.x*a.x.y,
			a.x.w*a.z.x*a.y.y + a.y.w*a.x.x*a.z.y + a.z.w*a.y.x*a.x.y - a.x.w*a.y.x*a.z.y - a.z.w*a.x.x*a.y.y - a.y.w*a.z.x*a.x.y},
		{	a.y.x*a.w.y*a.z.z + a.z.x*a.y.y*a.w.z + a.w.x*a.z.y*a.y.z - a.y.x*a.z.y*a.w.z - a.w.x*a.y.y*a.z.z - a.z.x*a.w.y*a.y.z,
			a.x.x*a.z.y*a.w.z + a.w.x*a.x.y*a.z.z + a.z.x*a.w.y*a.x.z - a.x.x*a.w.y*a.z.z - a.z.x*a.x.y*a.w.z - a.w.x*a.z.y*a.x.z,
			a.x.x*a.w.y*a.y.z + a.y.x*a.x.y*a.w.z + a.w.x*a.y.y*a.x.z - a.x.x*a.y.y*a.w.z - a.w.x*a.x.y*a.y.z - a.y.x*a.w.y*a.x.z,
			a.x.x*a.y.y*a.z.z + a.z.x*a.x.y*a.y.z + a.y.x*a.z.y*a.x.z - a.x.x*a.z.y*a.y.z - a.y.x*a.x.y*a.z.z - a.z.x*a.y.y*a.x.z}};
}

// Compute the determinant of a square matrix
template<class T> T determinant(const matrix<T, 2, 2> & a) {
	return a.x.x * a.y.y - a.x.y * a.y.x;
}
template<class T> T determinant(const matrix<T, 3, 3> & a) {
	return a.x.x * (a.y.y * a.z.z - a.z.y * a.y.z)
			+ a.x.y * (a.y.z * a.z.x - a.z.z * a.y.x)
			+ a.x.z * (a.y.x * a.z.y - a.z.x * a.y.y);
}
template<class T> T determinant(const matrix<T, 4, 4> & a) {
	return a.x.x
			* (a.y.y * a.z.z * a.w.w + a.w.y * a.y.z * a.z.w
					+ a.z.y * a.w.z * a.y.w - a.y.y * a.w.z * a.z.w
					- a.z.y * a.y.z * a.w.w - a.w.y * a.z.z * a.y.w)
			+ a.x.y
					* (a.y.z * a.w.w * a.z.x + a.z.z * a.y.w * a.w.x
							+ a.w.z * a.z.w * a.y.x - a.y.z * a.z.w * a.w.x
							- a.w.z * a.y.w * a.z.x - a.z.z * a.w.w * a.y.x)
			+ a.x.z
					* (a.y.w * a.z.x * a.w.y + a.w.w * a.y.x * a.z.y
							+ a.z.w * a.w.x * a.y.y - a.y.w * a.w.x * a.z.y
							- a.z.w * a.y.x * a.w.y - a.w.w * a.z.x * a.y.y)
			+ a.x.w
					* (a.y.x * a.w.y * a.z.z + a.z.x * a.y.y * a.w.z
							+ a.w.x * a.z.y * a.y.z - a.y.x * a.z.y * a.w.z
							- a.w.x * a.y.y * a.z.z - a.z.x * a.w.y * a.y.z);
}

// Compute the inverse of a square matrix
template<class T, int N> matrix<T, N, N> inverse(const matrix<T, N, N> & a) {
	return adjugate(a) / determinant(a);
}

// Compute the product of a matrix post-multiplied by a column vector
template<class T, int M> vec<T, M> mul(const matrix<T, M, 2> & a,
		const vec<T, 2> & b) {
	return a.x * b.x + a.y * b.y;
}
template<class T, int M> vec<T, M> mul(const matrix<T, M, 3> & a,
		const vec<T, 3> & b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
template<class T, int M> vec<T, M> mul(const matrix<T, M, 4> & a,
		const vec<T, 4> & b) {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

// Compute the product of two matrices
template<class T, int M, int N> matrix<T, M, 2> mul(const matrix<T, M, N> & a,
		const matrix<T, N, 2> & b) {
	return {mul(a,b.x), mul(a,b.y)};
}
template<class T, int M, int N> matrix<T, M, 3> mul(const matrix<T, M, N> & a,
		const matrix<T, N, 3> & b) {
	return {mul(a,b.x), mul(a,b.y), mul(a,b.z)};
}
template<class T, int M, int N> matrix<T, M, 4> mul(const matrix<T, M, N> & a,
		const matrix<T, N, 4> & b) {
	return {mul(a,b.x), mul(a,b.y), mul(a,b.z), mul(a,b.w)};
}

// Compute the tranpose of a matrix
template<class T, int M> matrix<T, M, 2> transpose(const matrix<T, 2, M> & m) {
	return {m.row(0), m.row(1)};
}
template<class T, int M> matrix<T, M, 3> transpose(const matrix<T, 3, M> & m) {
	return {m.row(0), m.row(1), m.row(2)};
}
template<class T, int M> matrix<T, M, 4> transpose(const matrix<T, 4, M> & m) {
	return {m.row(0), m.row(1), m.row(2), m.row(3)};
}

template <class T,int M, int N> vec<T, M> diagonal(const matrix<T, M, N>& Mt){
	vec<T,M> V;
	for(int m=0;m<M;m++){
		V[m]=Mt(m,m);
	}
	return V;
}
template <class T,int M, int N> T trace(const matrix<T, M, N>& Mt){
	double t=0;
	for(int m=0;m<M;m++){
		for(int n=0;n<N;n++){
			t+=Mt(m,n);
		}
	}
	return T(t);
}

//////////////////////////////////
// Quaternion algebra functions //
//////////////////////////////////

// Compute the conjugate/inverse/produce of a quaternions represented by 4D vectors
template<class T> vec<T, 4> qconjugate(const vec<T, 4> & q) {
	return {-q.x,-q.y,-q.z,q.w};
}
template<class T> vec<T, 4> qinverse(const vec<T, 4> & q) {
	return qconjugate(q) / length_sq(q);
}
template<class T> vec<T, 4> qmul(const vec<T, 4> & a, const vec<T, 4> & b) {
	return {a.x*b.w+a.w*b.x+a.y*b.z-a.z*b.y, a.y*b.w+a.w*b.y+a.z*b.x-a.x*b.z, a.z*b.w+a.w*b.z+a.x*b.y-a.y*b.x, a.w*b.w-a.x*b.x-a.y*b.y-a.z*b.z};
}
template<class T> matrix<T, 3, 3> q2matrix(const vec<T, 4> & q) {
	return {qrotate_x(q), qrotate_y(q), qrotate_z(q)};
}

// Compute spatial rotations of 3D vectors via the quaternion product qvq*
template<class T> vec<T, 3> qrotate_x(const vec<T, 4> & q) {
	return {q.w*q.w+q.x*q.x-q.y*q.y-q.z*q.z, (q.x*q.y+q.z*q.w)*2, (q.z*q.x-q.y*q.w)*2};
} // q [1,0,0,0] q*
template<class T> vec<T, 3> qrotate_y(const vec<T, 4> & q) {
	return {(q.x*q.y-q.z*q.w)*2, q.w*q.w-q.x*q.x+q.y*q.y-q.z*q.z, (q.y*q.z+q.x*q.w)*2};
} // q [0,1,0,0] q*
template<class T> vec<T, 3> qrotate_z(const vec<T, 4> & q) {
	return {(q.z*q.x+q.y*q.w)*2, (q.y*q.z-q.x*q.w)*2, q.w*q.w-q.x*q.x-q.y*q.y+q.z*q.z};
} // q [0,0,1,0] q*
template<class T> vec<T, 3> qrotate(const vec<T, 4> & q, const vec<T, 3> & v) {
	return qrotate_x(q) * v.x + qrotate_y(q) * v.y + qrotate_z(q) * v.z;
} // q [x,y,z,0] q*

template<class T> vec<T, 3> qaxis(const vec<T, 4> & q) {
	auto a = qangle(q);
	return a < 0.0000001 ? vec<T, 3>(1, 0, 0) : q.xyz() * (1 / sin(a / 2));
}

template<class T> vec<T, 2> Rotate(const vec<T, 2>& v, T angle) {
	T cs = cos(angle);
	T sn = sin(angle);
	return vec<T, 2>(cs * v[0] + sn * v[1], -sn * v[0] + cs * v[1]);
}
template<class T> matrix<T, 4, 4> MakeRotation(const vec<T, 4> & q) {
	matrix<T, 4, 4> A;
	T s=T(1)/dot(q,q);
	A.x=vec<T,4>(
		 T(1)-2*s*(q.y*q.y+q.z*q.z),
			  2*s*(q.x*q.y+q.z*q.w),
			  2*s*(q.x*q.z-q.y*q.w),0);
	A.y=vec<T,4>(
			  2*s*(q.x*q.y-q.z*q.w),
		 T(1)-2*s*(q.x*q.x+q.z*q.z),
			  2*s*(q.y*q.z+q.x*q.w),0);
	A.z=vec<T,4>(
			  2*s*(q.x*q.z+q.y*q.w),
			  2*s*(q.y*q.z-q.x*q.w),
		 T(1)-2*s*(q.x*q.x+q.y*q.y),0);
	A.w=vec<T,4>(0,0,0,1);
	return A;
	//return { vec<T,4>(qrotate_x(q),0), vec<T,4>(qrotate_y(q),0), vec<T,4>(qrotate_z(q),0), vec<T,4>(0,0,0,1)};
}
template<class T> vec<T,4> slerp(vec<T,4> q1,vec<T,4> q2,T lambda){
	T dotproduct = q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
	T theta, st, sut, sout, coeff1, coeff2;
	// algorithm adapted from Shoemake's paper
	lambda=lambda/T(2);
	theta = std::acos(dotproduct);
	if (theta<0) theta=-theta;
	st = std::sin(theta);
	sut = std::sin(lambda*theta);
	sout = std::sin((1-lambda)*theta);
	coeff1 = sout/st;
	coeff2 = sut/st;
	vec<T, 4> qr;
	qr.x = coeff1*q1.x + coeff2*q2.x;
	qr.y = coeff1*q1.y + coeff2*q2.y;
	qr.z = coeff1*q1.z + coeff2*q2.z;
	qr.w = coeff1*q1.w + coeff2*q2.w;
	qr=normalize(qr);
	return qr;
}
template<class T> matrix<T, 4, 4> MakeRotationX(T angle) {
	return MakeRotation(vec<T, 3>(1, 0, 0), angle);
}
template<class T> matrix<T, 4, 4> MakeRotationY(T angle) {
	return MakeRotation(vec<T, 3>(0, 1, 0), angle);
}
template<class T> matrix<T, 4, 4> MakeRotationZ(T angle) {
	return MakeRotation(vec<T, 3>(0, 0, 1), angle);
}
template<class T> matrix<T, 4, 4> MakeTranslation(
		const vec<T, 3>& translation) {
	return matrix<T, 4, 4>(vec<T, 4>((T) 1, (T) 0, (T) 0, (T) 0),
			vec<T, 4>((T) 0, (T) 1, (T) 0, (T) 0),
			vec<T, 4>((T) 0, (T) 0, (T) 1, (T) 0), translation.xyzw());
}
template<class T> matrix<T, 4, 4> MakeTranslation(
		const vec<T, 4>& translation) {
	return matrix<T, 4, 4>(vec<T, 4>((T) 1, (T) 0, (T) 0, (T) 0),
			vec<T, 4>((T) 0, (T) 1, (T) 0, (T) 0),
			vec<T, 4>((T) 0, (T) 0, (T) 1, (T) 0), translation);
}
template<class T> matrix<T, 4, 4> MakeScale(const vec<T, 3>& scale) {
	return matrix<T, 4, 4>(vec<T, 4>(scale.x, 0, 0, 0),
			vec<T, 4>(0, scale.y, 0, 0), vec<T, 4>(0, 0, scale.z, 0),
			vec<T, 4>(0, 0, 0, 1));
}
template<class T> matrix<T, 3, 3> MakeDiagonal(const vec<T, 3>& scale) {
	return matrix<T, 3, 3>(vec<T, 3>(scale.x, 0, 0), vec<T, 3>(0, scale.y, 0),
			vec<T, 3>(0, 0, scale.z));
}
template<class T> matrix<T, 4, 4> MakeDiagonal(const vec<T, 4>& scale) {
	return matrix<T, 4, 4>(vec<T, 4>(scale.x, 0, 0, 0),
			vec<T, 4>(0, scale.y, 0, 0), vec<T, 4>(0, 0, scale.z, 0),
			vec<T, 4>(0, 0, 0, scale.w));
}
template<class T> matrix<T, 4, 4> MakeScale(const vec<T, 4>& scale) {
	return matrix<T, 4, 4>(vec<T, 4>(scale.x, 0, 0, 0),
			vec<T, 4>(0, scale.y, 0, 0), vec<T, 4>(0, 0, scale.z, 0),
			vec<T, 4>(0, 0, 0, scale.w));
}
template<class T> matrix<T, 4, 4> MakeScale(T scale) {
	return matrix<T, 4, 4>(vec<T, 4>(scale, 0, 0, 0), vec<T, 4>(0, scale, 0, 0),
			vec<T, 4>(0, 0, scale, 0), vec<T, 4>(0, 0, 0, 1));
}
template<class T> matrix<T, 3, 3> MakeTranslation(
		const vec<T, 2>& translation) {
	return matrix<T, 3, 3>(vec<T, 3>((T) 1, (T) 0, (T) 0),
			vec<T, 3>((T) 0, (T) 1, (T) 0),
			vec<T, 3>(translation.x, translation.y, (T) 1));
}
template<class T> matrix<T, 3, 3> MakeTranslation(const T tx, const T ty) {
	return matrix<T, 3, 3>(vec<T, 3>((T) 1, (T) 0, (T) 0),
			vec<T, 3>((T) 0, (T) 1, (T) 0), vec<T, 3>(tx, ty, (T) 1));
}
template<class T> matrix<T, 3, 3> MakeRotation(const T angle) {
	T cs = std::cos(angle);
	T sn = std::sin(angle);
	return matrix<T, 3, 3>(vec<T, 3>(cs, sn, (T) 0), vec<T, 3>(-sn, cs, (T) 0),
			vec<T, 3>((T) 0, (T) 0, (T) 1));
}
template<class T> matrix<T, 3, 3> MakeScale(const T sx, const T sy) {
	return matrix<T, 3, 3>(vec<T, 3>((T) sx, (T) 0, (T) 0),
			vec<T, 3>((T) 0, (T) sy, (T) 0), vec<T, 3>((T) 0, (T) 0, (T) 1));
}
template<class T> matrix<T, 3, 3> MakeScale(const vec<T, 2>& scale) {
	return matrix<T, 3, 3>(vec<T, 3>((T) scale.x, (T) 0, (T) 0),
			vec<T, 3>((T) 0, (T) scale.y, (T) 0),
			vec<T, 3>((T) 0, (T) 0, (T) 1));
}

template<class T> matrix<T, 4, 4> MakeTransform(const box<T, 3>& src,
		const box<T, 3>& tar) {
	float scaleT = aly::min(tar.dimensions);
	float scaleS = aly::max(src.dimensions);
	return MakeTranslation((tar.position + 0.5f * tar.dimensions))
			* MakeScale(scaleT / scaleS)
			* MakeTranslation(-(src.position + 0.5f * src.dimensions));
}
template<class T> matrix<T, 4, 4> MakeTransform(const matrix<T, 3, 3>& R,
		const vec<T, 3>& t) {
	matrix<T, 4, 4> M;
	M.x = vec<T, 4>(R.x, T(0));
	M.y = vec<T, 4>(R.y, T(0));
	M.z = vec<T, 4>(R.z, T(0));
	M.w = vec<T, 4>(t, T(1));
	return M;
}
template<class T> vec<T, 4> MakeQuaternion(const matrix<T, 3, 3>& a) {
	vec<T, 4> q;
	T trace = a(0, 0) + a(1, 1) + a(2, 2);
	if (trace > 0) { // I changed M_EPSILON to 0
		float s = T(0.5) / std::sqrt(trace + 1.0);
		q.w = T(0.25) / s;
		q.x = (a(2, 1) - a(1, 2)) * s;
		q.y = (a(0, 2) - a(2, 0)) * s;
		q.z = (a(1, 0) - a(0, 1)) * s;
	} else {
		if (a(0, 0) > a(1, 1) && a(0, 0) > a(2, 2)) {
			T s = T(2) * std::sqrt(1.0f + a(0, 0) - a(1, 1) - a(2, 2));
			q.w = (a(2, 1) - a(1, 2)) / s;
			q.x = T(0.25) * s;
			q.y = (a(0, 1) + a(1, 0)) / s;
			q.z = (a(0, 2) + a(2, 0)) / s;
		} else if (a(1, 1) > a(2, 2)) {
			T s = T(2)* std::sqrt(1.0f + a(1, 1) - a(0, 0) - a(2, 2));
			q.w = (a(0, 2) - a(2, 0)) / s;
			q.x = (a(0, 1) + a(1, 0)) / s;
			q.y = T(0.25) * s;
			q.z = (a(1, 2) + a(2, 1)) / s;
		} else {
			T s = T(2) * std::sqrt(1.0f + a(2, 2) - a(0, 0) - a(1, 1));
			q.w = (a(1, 0) - a(0, 1)) / s;
			q.x = (a(0, 2) + a(2, 0)) / s;
			q.y = (a(1, 2) + a(2, 1)) / s;
			q.z =T(0.25) * s;
		}
	}
	return q;
}
matrix<float, 4, 4> MakeRotation(const vec<float, 3>& axis, float angle);
matrix<double, 4, 4> MakeRotation(const vec<double, 3>& axis, double angle);

matrix<float, 4, 4> MakeRotation(const vec<float, 3>& axis);
matrix<double, 4, 4> MakeRotation(const vec<double, 3>& axis);
/////////////////////////
// Convenience aliases //
/////////////////////////

typedef uint8_t ubyte;
typedef uint16_t ushort;
typedef int8_t byte;
typedef uint32_t uint;
typedef vec<uint8_t, 1> ubyte1;
typedef vec<int8_t, 1> byte1;
typedef vec<uint16_t, 1> ushort1;
typedef vec<int16_t, 1> short1;
typedef vec<int32_t, 1> int1;
typedef vec<uint32_t, 1> uint1;
typedef vec<bool, 1> bool1;
typedef vec<float, 1> float1;
typedef vec<double, 1> double1;
typedef vec<uint8_t, 2> ubyte2;
typedef vec<int8_t, 2> byte2;
typedef vec<uint16_t, 2> ushort2;
typedef vec<int16_t, 2> short2;
typedef vec<int32_t, 2> int2;
typedef vec<uint32_t, 2> uint2;
typedef vec<bool, 2> bool2;
typedef vec<float, 2> float2;
typedef vec<double, 2> double2;
typedef vec<uint8_t, 3> ubyte3;
typedef vec<int8_t, 3> byte3;
typedef vec<uint16_t, 3> ushort3;
typedef vec<int16_t, 3> short3;
typedef vec<int32_t, 3> int3;
typedef vec<uint32_t, 3> uint3;
typedef vec<bool, 3> bool3;
typedef vec<float, 3> float3;
typedef vec<double, 3> double3;
typedef vec<uint8_t, 4> ubyte4;
typedef vec<int8_t, 4> byte4;
typedef vec<uint16_t, 4> ushort4;
typedef vec<int16_t, 4> short4;
typedef vec<int32_t, 4> int4;
typedef vec<uint32_t, 4> uint4;
typedef vec<bool, 4> bool4;
typedef vec<float, 4> float4;
typedef vec<double, 4> double4;
typedef matrix<float, 1, 2> float1x2;
typedef matrix<float, 1, 3> float1x3;
typedef matrix<float, 1, 4> float1x4;
typedef matrix<float, 2, 2> float2x2;
typedef matrix<float, 2, 3> float2x3;
typedef matrix<float, 2, 4> float2x4;
typedef matrix<float, 3, 2> float3x2;
typedef matrix<float, 3, 3> float3x3;
typedef matrix<float, 3, 4> float3x4;
typedef matrix<float, 4, 2> float4x2;
typedef matrix<float, 4, 3> float4x3;
typedef matrix<float, 4, 4> float4x4;
typedef matrix<double, 1, 2> double1x2;
typedef matrix<double, 1, 3> double1x3;
typedef matrix<double, 1, 4> double1x4;
typedef matrix<double, 2, 2> double2x2;
typedef matrix<double, 2, 3> double2x3;
typedef matrix<double, 2, 4> double2x4;
typedef matrix<double, 3, 2> double3x2;
typedef matrix<double, 3, 3> double3x3;
typedef matrix<double, 3, 4> double3x4;
typedef matrix<double, 4, 2> double4x2;
typedef matrix<double, 4, 3> double4x3;
typedef matrix<double, 4, 4> double4x4;
typedef float4 RGBAf;
typedef float3 RGBf;
typedef float4 HSVA;
typedef float3 HSV;
typedef int4 RGBAi;
typedef int3 RGBi;
typedef ubyte4 RGBA;
typedef ubyte3 RGB;

typedef box<float, 1> box1f;
typedef box<float, 2> box2f;
typedef box<float, 3> box3f;
typedef box<float, 4> box4f;

typedef box<double, 1> box1d;
typedef box<double, 2> box2d;
typedef box<double, 3> box3d;
typedef box<double, 4> box4d;

typedef box<int, 1> box1i;
typedef box<int, 2> box2i;
typedef box<int, 3> box3i;
typedef box<int, 4> box4i;

typedef box<uint32_t, 1> box1ui;
typedef box<uint32_t, 2> box2ui;
typedef box<uint32_t, 3> box3ui;
typedef box<uint32_t, 4> box4ui;

double min(const std::vector<double>& a,
		double init = std::numeric_limits<double>::max());
double max(const std::vector<double>& a,
		double init = std::numeric_limits<double>::min());

float min(const std::vector<float>& a, float init =
		std::numeric_limits<float>::max());
float max(const std::vector<float>& a, float init =
		std::numeric_limits<float>::min());

int min(const std::vector<int>& a, int init = std::numeric_limits<int>::max());
int max(const std::vector<int>& a, int init = std::numeric_limits<int>::min());

double2 min(const std::vector<double2>& a,
		double init = std::numeric_limits<double>::max());
double2 max(const std::vector<double2>& a,
		double init = std::numeric_limits<double>::min());

float2 min(const std::vector<float2>& a, float init =
		std::numeric_limits<float>::max());
float2 max(const std::vector<float2>& a, float init =
		std::numeric_limits<float>::min());

int2 min(const std::vector<int2>& a, int init =
		std::numeric_limits<int>::max());
int2 max(const std::vector<int2>& a, int init =
		std::numeric_limits<int>::min());

double3 min(const std::vector<double3>& a,
		double init = std::numeric_limits<double>::max());
double3 max(const std::vector<double3>& a,
		double init = std::numeric_limits<double>::min());

float3 min(const std::vector<float3>& a, float init =
		std::numeric_limits<float>::max());
float3 max(const std::vector<float3>& a, float init =
		std::numeric_limits<float>::min());

int3 min(const std::vector<int3>& a, int init =
		std::numeric_limits<int>::max());
int3 max(const std::vector<int3>& a, int init =
		std::numeric_limits<int>::min());

double4 min(const std::vector<double4>& a,
		double init = std::numeric_limits<double>::max());
double4 max(const std::vector<double4>& a,
		double init = std::numeric_limits<double>::min());

float4 min(const std::vector<float4>& a, float init =
		std::numeric_limits<float>::max());
float4 max(const std::vector<float4>& a, float init =
		std::numeric_limits<float>::min());

int4 min(const std::vector<int4>& a, int init =
		std::numeric_limits<int>::max());
int4 max(const std::vector<int4>& a, int init =
		std::numeric_limits<int>::min());
inline RGBA ToRGBA(const RGBAf& r) {
	return RGBA(clamp((int) (r.x * 255.0f), 0, 255),
			clamp((int) (r.y * 255.0f), 0, 255),
			clamp((int) (r.z * 255.0f), 0, 255),
			clamp((int) (r.w * 255.0f), 0, 255));
}
inline RGBA ToRGBA(const RGBf& r) {
	return RGBA(clamp((int) (r.x * 255.0f), 0, 255),
			clamp((int) (r.y * 255.0f), 0, 255),
			clamp((int) (r.z * 255.0f), 0, 255), 255);
}
inline ubyte3 ToRGB(const RGBf& r) {
	return ubyte3(clamp((int) (r.x * 255.0f), 0, 255),
			clamp((int) (r.y * 255.0f), 0, 255),
			clamp((int) (r.z * 255.0f), 0, 255));
}
inline ubyte3 ToRGB(const RGBAf& r) {
	return ubyte3(clamp((int) (r.x * 255.0f), 0, 255),
			clamp((int) (r.y * 255.0f), 0, 255),
			clamp((int) (r.z * 255.0f), 0, 255));
}

inline RGBAf ToRGBAf(const RGBA& r) {
	return RGBAf(r.x / 255.0f, r.y / 255.0f, r.z / 255.0f, r.w / 255.0f);
}
inline RGBAf ToRGBAf(const ubyte3& r) {
	return RGBAf(r.x / 255.0f, r.y / 255.0f, r.z / 255.0f, 1.0f);
}
inline RGBf ToRGBf(const RGBA& r) {
	return RGBf(r.x / 255.0f, r.y / 255.0f, r.z / 255.0f);
}
inline RGBf ToRGBf(const ubyte3& r) {
	return RGBf(r.x / 255.0f, r.y / 255.0f, r.z / 255.0f);
}
inline RGBf ToRGBf(const RGBAf& r) {
	return r.xyz();
}
inline RGBAf ToRGBAf(const RGBf& r) {
	return RGBAf(r, 1.0f);
}
inline aly::ubyte3 ToRGB(const RGBA& r) {
	return r.xyz();
}
inline aly::RGBA ToRGBA(const ubyte3& r) {
	return aly::RGBA(r, 255);
}

struct dim2: public int2 {
	dim2(int x = 0, int y = 0) :
			int2(x, y) {
	}
	dim2(const int2& pt) :
			dim2(pt.x, pt.y) {
	}
	dim2(int x) :
			dim2(x, x) {
	}
	inline size_t operator()(const int i, const int j) const {
		return (size_t) clamp(i, 0, x - 1) + clamp(j, 0, y - 1) * (size_t) x;
	}
	inline size_t operator()(const int2& pos) const {
		return (size_t) clamp(pos.x, 0, x - 1)
				+ clamp(pos.y, 0, y - 1) * (size_t) x;
	}
	inline size_t size() const {
		return x * (size_t) y;
	}
	inline size_t area() const {
		return x * (size_t) y;
	}
	inline int2 operator()(const size_t index) const {
		int i = (int) (index % (size_t) x);
		int j = (int) (index / (size_t) x);
		return int2(i, j);
	}
	inline dim2 operator =(const int2& dim) {
		return dim2(dim);
	}
};

struct dim3: public int3 {
	dim3(int x = 0, int y = 0, int z = 0) :
			int3(x, y, z) {
	}
	dim3(const int3& pt) :
			dim3(pt.x, pt.y, pt.z) {
	}
	dim3(int x) :
			dim3(x, x, x) {
	}
	inline size_t operator()(const int i, const int j, const int k) const {
		return (size_t) clamp(i, 0, x - 1) + (size_t) clamp(j, 0, y - 1) * x
				+ clamp(k, 0, z - 1) * (size_t) x * (size_t) y;
	}
	inline size_t operator()(const int3& pos) const {
		return (size_t) clamp(pos.x, 0, x - 1)
				+ (size_t) clamp(pos.y, 0, y - 1) * x
				+ clamp(pos.z, 0, z - 1) * (size_t) x * (size_t) y;
	}
	inline size_t size() const {
		return x * (size_t) y * (size_t) z;
	}
	inline size_t area() const {
		return x * (size_t) y;
	}
	inline int3 operator()(const size_t index) const {
		size_t a = area();
		size_t k = index / a;
		size_t ij = index % a;
		int i = (int) (ij % (size_t) x);
		int j = (int) (ij / (size_t) x);
		return int3(i, j, (int) k);
	}
	inline size_t volume() const {
		return x * (size_t) y * (size_t) z;
	}
	inline dim3 operator =(const int3& dim) {
		return dim3(dim);
	}
};

struct dim4: public int4 {
	dim4(int x = 0, int y = 0, int z = 0, int w = 0) :
			int4(x, y, z, w) {
	}
	dim4(const int4& pt) :
			dim4(pt.x, pt.y, pt.z, pt.w) {
	}
	dim4(int x) :
			dim4(x, x, x, x) {
	}
	inline size_t operator()(const int i, const int j, const int k,
			const int l) const {
		return (size_t) clamp(i, 0, x - 1) + (size_t) clamp(j, 0, y - 1) * x
				+ clamp(k, 0, z - 1) * (size_t) x * (size_t) y
				+ clamp(l, 0, w - 1) * (size_t) x * (size_t) y * (size_t) z;
	}
	inline size_t operator()(const int4& pos) const {
		return (size_t) clamp(pos.x, 0, x - 1)
				+ (size_t) clamp(pos.y, 0, y - 1) * x
				+ clamp(pos.z, 0, z - 1) * (size_t) x * (size_t) y
				+ clamp(pos.w, 0, w - 1) * (size_t) x * (size_t) y * (size_t) z;
	}
	inline size_t size() const {
		return x * (size_t) y * (size_t) z * (size_t) w;
	}
	inline size_t area() const {
		return x * (size_t) y;
	}
	inline size_t volume() const {
		return x * (size_t) y * (size_t) z;
	}
	inline dim4 operator =(const int4& dim) {
		return dim4(dim);
	}
	inline int4 operator()(const size_t index) const {
		size_t v = volume();
		size_t l = index / v;
		size_t ijk = index % v;
		size_t a = area();
		size_t k = ijk / a;
		size_t ij = ijk % a;
		int i = (int) (ij % (size_t) x);
		int j = (int) (ij / (size_t) x);
		return int4(i, j, (int) k, (int) l);
	}
};

template<class T> vec<T, 3> Transform(const matrix<T, 4, 4>& M,
		const vec<T, 3>& v) {
	vec<T, 4> out = M * vec<T, 4>(v, T(1.0));
	return out.xyz() / out.w;
}
template<class T> vec<T, 4> Transform(const matrix<T, 3, 3>& M,
		const vec<T, 4>& v) {
	vec<T, 3> out = M * v.xyz();
	return vec<T, 4>(out, v.w);
}
template<class T> vec<T, 3> Transform(const matrix<T, 3, 3>& M,
		const vec<T, 3>& v) {
	vec<T, 3> out = M * v;
	return (out / out.z);
}
template<class T> vec<T, 2> Transform(const matrix<T, 3, 3>& M,
		const vec<T, 2>& v) {
	vec<T, 3> out = M * vec<T, 3>(v, T(1.0));
	return (out.xy() / out.z);
}
template<class T> vec<T, 4> Transform(const matrix<T, 4, 4>& M,
		const vec<T, 4>& v) {
	vec<T, 4> out = M * v;
	return (out / out.w);
}
template<class T> vec<T, 3> TransformNormal(const matrix<T, 4, 4>& M,
		const vec<T, 3>& v) {
	return normalize((M * vec<T, 4>(v, T(0))).xyz());
}
float RandomUniform(float min, float max);
int RandomUniform(int min, int max);
double RandomUniform(double min, double max);
double RandomGaussian(double mean, double stddev);
float RandomGaussian(float mean, float stddev);

}
#endif /* INCLUDE_CORE_ALLOYMATHBASE_H_ */
