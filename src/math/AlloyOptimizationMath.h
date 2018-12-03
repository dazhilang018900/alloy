/*
 * Copyright(C) 2016, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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

#ifndef INCLUDE_CORE_ALLOYOPTIMIZATIONMATH_H_
#define INCLUDE_CORE_ALLOYOPTIMIZATIONMATH_H_
#include "math/AlloyVector.h"
#include "image/AlloyImage.h"
#include "system/AlignedAllocator.h"
#include "common/cereal/types/list.hpp"
#include "common/cereal/types/vector.hpp"
#include "common/cereal/types/tuple.hpp"
#include "common/cereal/types/map.hpp"
#include "common/cereal/types/memory.hpp"
#include "common/cereal/archives/portable_binary.hpp"
#include <vector>
#include <list>
#include <map>
namespace aly {
template<class T> struct VecType {
	virtual size_t size() const=0;
	virtual T& operator[](const size_t i)=0;
	virtual const T& operator[](const size_t i) const =0;
	virtual ~VecType() {
	}
};
template<class T> struct Vec;
template<class T> struct VecMap: public VecType<T> {
private:
	T* ptr;
	const size_t sz;
	const size_t stride;
public:
	VecMap(T* ptr, size_t sz, size_t stride = 1) :
			ptr(ptr), sz(sz), stride(stride) {
	}
	VecMap(Vec<T>& vec);
	VecMap() :
			ptr(nullptr), sz(0), stride(0) {
	}
	T& operator[](const size_t i) override {
		if (i >= sz) {
			throw std::runtime_error("Index exceeds vector map bounds.");
		}
		if (ptr == nullptr) {
			throw std::runtime_error("Pointer is null.");
		}
		return ptr[i * stride];
	}
	const T& operator[](const size_t i) const override {
		if (i >= sz) {
			throw std::runtime_error("Index exceeds vector map bounds.");
		}
		if (ptr == nullptr) {
			throw std::runtime_error("Pointer is null.");
		}
		return ptr[i * stride];
	}
	size_t getStride() const {
		return stride;
	}
	size_t size() const override {
		return sz;
	}
	inline void setZero() {
		for (size_t i = 0; i < sz; i++) {
			ptr[i * stride] = T(0);
		}
	}
	VecMap<T>& operator=(const Vec<T>& rhs);

};
template<class T> struct Vec: VecType<T> {
public:

	std::vector<T> data; //, aligned_allocator<T, 64>
	typedef T ValueType;
	typedef typename std::vector<ValueType>::iterator iterator;
	typedef typename std::vector<ValueType>::const_iterator const_iterator;
	typedef typename std::vector<ValueType>::reverse_iterator reverse_iterator;
	inline void setZero() {
		data.assign(data.size(), T(0));
	}
	inline iterator begin() {
		return data.begin();
	}
	inline iterator end() {
		return data.end();
	}
	inline const_iterator begin() const {
		return data.begin();
	}
	inline const_iterator end() const {
		return data.end();
	}
	inline reverse_iterator rbegin() {
		return data.rbegin();
	}
	inline reverse_iterator rend() {
		return data.rend();
	}
	Vec(size_t sz = 0, T value = T(0)) :
			data(sz, value) {
	}
	void set(const T& val) {
		data.assign(data.size(), val);
	}
	inline void set(const Vec<T>& val) {
		this->data = val.data;
	}
	inline void set(const std::vector<T>& val) {
		this->data = data;
	}
	Vec<T>& operator=(const VecMap<T>& rhs) {
		this->resize(rhs.size());
		for (size_t index = 0; index < rhs.size(); index++) {
			data[index] = rhs[index];
		}
		return *this;
	}
	template<class Archive>
	void save(Archive & archive) const {
		archive(CEREAL_NVP(data));
	}

	template<class Archive>
	void load(Archive & archive) {
		archive(CEREAL_NVP(data));
	}
	void resize(size_t sz) {
		data.resize(sz);
		data.shrink_to_fit();
	}
	void resize(size_t sz, const T& val) {
		data.resize(sz, val);
		data.shrink_to_fit();
	}
	bool empty() const {
		return data.empty();
	}
	void append(const T& val) {
		data.push_back(val);
	}
	void push_back(const T& val) {
		data.push_back(val);
	}
	T* ptr() {
		return data.data();
	}
	const T* ptr() const {
		return data.data();
	}
	const T& operator[](const size_t i) const override {
		if (i >= data.size())
			throw std::runtime_error(
					MakeString() << "Vector index out of bounds " << i << "/"
							<< data.size());
		return data[i];
	}
	T& operator[](const size_t i) override {
		if (i >= data.size())
			throw std::runtime_error(
					MakeString() << "Vector index out of bounds " << i << "/"
							<< data.size());
		return data[i];
	}
	T& at(const size_t i) {
		if (i >= data.size())
			throw std::runtime_error(
					MakeString() << "Vector index out of bounds " << i << "/"
							<< data.size());
		return data[i];
	}
	const T& at(const size_t i) const {
		if (i >= data.size())
			throw std::runtime_error(
					MakeString() << "Vector index out of bounds " << i << "/"
							<< data.size());
		return data[i];
	}
	inline static Vec<float> zero(int d) {
		Vec<float> v(d);
		v.data.assign(v.size(), 0);
		return v;
	}
	inline void clear() {
		data.clear();
		data.shrink_to_fit();
	}
	size_t size() const override {
		return data.size();
	}
};
template<class T> VecMap<T>::VecMap(Vec<T>& vec) :
		ptr(vec.data.data()), sz(vec.size()), stride(1) {
}
template<class T> VecMap<T>& VecMap<T>::operator=(const Vec<T>& rhs) {
	if (rhs.size() != sz || ptr == nullptr) {
		throw std::runtime_error("Could not assign vecmap.");
	}
	for (size_t index = 0; index < sz; index++) {
		(*this)[index] = rhs[index];
	}
	return *this;
}
template<class T> void Transform(VecType<T>& im1, VecType<T>& im2,
		const std::function<void(T&, T&)>& func) {
	if (im1.size() != im2.size())
		throw std::runtime_error(
				MakeString() << "Vector dimensions do not match. " << im1.size()
						<< "!=" << im2.size());
	size_t sz = im1.size();
#pragma omp parallel for
	for (size_t offset = 0; offset < sz; offset++) {
		func(im1[offset], im2[offset]);
	}
}
template<class T> void Transform(VecType<T>& im1,
		const std::function<void(T&)>& func) {
	size_t sz = im1.size();
#pragma omp parallel for
	for (size_t offset = 0; offset < sz; offset++) {
		func(im1[offset]);
	}
}
template<class T> void Transform(VecType<T>& im1, const VecType<T>& im2,
		const std::function<void(T&, const T&)>& func) {
	if (im1.size() != im2.size())
		throw std::runtime_error(
				MakeString() << "Vector dimensions do not match. " << im1.size()
						<< "!=" << im2.size());
	size_t sz = im1.size();
#pragma omp parallel for
	for (size_t offset = 0; offset < sz; offset++) {
		func(im1[offset], im2[offset]);
	}
}
template<class T> void Transform(VecType<T>& im1, const VecType<T>& im2,
		const VecType<T>& im3, const VecType<T>& im4,
		const std::function<void(T&, const T&, const T&, const T&)>& func) {
	if (im1.size() != im2.size())
		throw std::runtime_error(
				MakeString() << "Vector dimensions do not match. " << im1.size()
						<< "!=" << im2.size());
	size_t sz = im1.size();
#pragma omp parallel for
	for (size_t offset = 0; offset < sz; offset++) {
		func(im1[offset], im2[offset], im3[offset], im4[offset]);
	}
}
template<class T> void Transform(VecType<T>& im1, const VecType<T>& im2,
		const VecType<T>& im3,
		const std::function<void(T&, const T&, const T&)>& func) {
	if (im1.size() != im2.size())
		throw std::runtime_error(
				MakeString() << "Vector dimensions do not match. " << im1.size()
						<< "!=" << im2.size());
	size_t sz = im1.size();
#pragma omp parallel for
	for (size_t offset = 0; offset < sz; offset++) {
		func(im1[offset], im2[offset], im3[offset]);
	}
}
template<class T> void Transform(VecType<T>& im1, VecType<T>& im2,
		const std::function<void(size_t offset, T& val1, T& val2)>& func) {
	if (im1.size() != im2.size())
		throw std::runtime_error(
				MakeString() << "Vector dimensions do not match. " << im1.size()
						<< "!=" << im2.size());
	size_t sz = im1.size();
#pragma omp parallel for
	for (size_t offset = 0; offset < sz; offset++) {
		func(offset, im1[offset], im2[offset]);
	}
}
template<class T, class L, class R> std::basic_ostream<L, R> & operator <<(
		std::basic_ostream<L, R> & ss, const VecType<T> & A) {
	ss << "[";
	for (size_t index = 0; index < A.size(); index++) {
		ss << A[index] << ((index < A.size() - 1) ? "," : "");
	}
	ss << "]";
	return ss;
}
template<class T> Vec<T> operator+(const T& scalar, const VecType<T>& img) {
	Vec<T> out(img.size());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = scalar + val2;};
	Transform(out, img, f);
	return out;
}
template<class T> void ScaleAdd(Vec<T>& out, const T& scalar,
		const VecType<T>& in) {
	out.resize(in.size());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 += scalar * val2;};
	Transform(out, in, f);
}
template<class T> void ScaleAdd(Vec<T>& out, const VecType<T>& in1,
		const T& scalar, const VecType<T>& in2) {
	out.resize(in1.size());
	std::function<void(T&, const T&, const T&)> f =
			[=](T& val1, const T& val2, const T& val3) {val1 = val2+scalar * val3;};
	Transform(out, in1, in2, f);
}
template<class T> void ScaleAdd(Vec<T>& out, const VecType<T>& in1,
		const T& scalar2, const VecType<T>& in2, const T& scalar3,
		const VecType<T>& in3) {
	out.resize(in1.size());
	std::function<void(T&, const T&, const T&, const T&)> f = [=](T& out,
			const T& val1,
			const T& val2,
			const T& val3) {
		out = val1+scalar2*val2+scalar3 * val3;};
	Transform(out, in1, in2, in3, f);
}
template<class T> void ScaleSubtract(Vec<T>& out, const T& scalar,
		const VecType<T>& in) {
	out.resize(in.size());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 -= scalar * val2;};
	Transform(out, in, f);
}
template<class T> void ScaleSubtract(Vec<T>& out, const VecType<T>& in1,
		const T& scalar, const VecType<T>& in2) {
	out.resize(in1.size());
	std::function<void(T&, const T&, const T&)> f =
			[=](T& val1, const T& val2, const T& val3) {val1 = val2 - scalar * val3;};
	Transform(out, in1, in2, f);
}
template<class T> void Subtract(Vec<T>& out, const VecType<T>& v1,
		const VecType<T>& v2) {
	out.resize(v1.size());
	std::function<void(T&, const T&, const T&)> f =
			[=](T& val1, const T& val2, const T& val3) {val1 = val2-val3;};
	Transform(out, v1, v2, f);
}
template<class T> void Add(Vec<T>& out, const VecType<T>& v1,
		const VecType<T>& v2) {
	out.resize(v1.size());
	std::function<void(T&, const T&, const T&)> f =
			[=](T& val1, const T& val2, const T& val3) {val1 = val2 + val3;};
	Transform(out, v1, v2, f);
}
template<class T> Vec<T> operator-(const T& scalar, const VecType<T>& img) {
	Vec<T> out(img.size());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = scalar - val2;};
	Transform(out, img, f);
	return out;
}
template<class T> Vec<T> operator*(const T& scalar, const VecType<T>& img) {
	Vec<T> out(img.size());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = scalar*val2;};
	Transform(out, img, f);
	return out;
}
template<class T> Vec<T> operator/(const T& scalar, const VecType<T>& img) {
	Vec<T> out(img.size());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = scalar / val2;};
	Transform(out, img, f);
	return out;
}
template<class T> Vec<T> operator+(const VecType<T>& img, const T& scalar) {
	Vec<T> out(img.size());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = val2 + scalar;};
	Transform(out, img, f);
	return out;
}
template<class T> Vec<T> operator-(const VecType<T>& img, const T& scalar) {
	Vec<T> out(img.size());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = val2 - scalar;};
	Transform(out, img, f);
	return out;
}
template<class T> Vec<T> operator*(const VecType<T>& img, const T& scalar) {
	Vec<T> out(img.size());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = val2*scalar;};
	Transform(out, img, f);
	return out;
}
template<class T> Vec<T> operator/(const VecType<T>& img, const T& scalar) {
	Vec<T> out(img.size());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = val2 / scalar;};
	Transform(out, img, f);
	return out;
}
template<class T> Vec<T> operator-(const VecType<T>& img) {
	Vec<T> out(img.size());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = -val2;};
	Transform(out, img, f);
	return out;
}
template<class T> VecMap<T>& operator+=(VecMap<T>& out, const VecType<T>& img) {
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 += val2;};
	Transform(out, img, f);
	return out;
}
template<class T> VecMap<T>& operator-=(VecMap<T>& out, const VecType<T>& img) {
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 -= val2;};
	Transform(out, img, f);
	return out;
}
template<class T> VecMap<T>& operator*=(VecMap<T>& out, const VecType<T>& img) {
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 *= val2;};
	Transform(out, img, f);
	return out;
}
template<class T> VecMap<T>& operator/=(VecMap<T>& out, const VecType<T>& img) {
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 /= val2;};
	Transform(out, img, f);
	return out;
}
template<class T> VecMap<T>& operator+=(VecMap<T>& out, const T& scalar) {
	std::function<void(T&)> f = [=](T& val1) {val1 += scalar;};
	Transform(out, f);
	return out;
}
template<class T> VecMap<T>& operator-=(VecMap<T>& out, const T& scalar) {
	std::function<void(T&)> f = [=](T& val1) {val1 -= scalar;};
	Transform(out, f);
	return out;
}
template<class T> VecMap<T>& operator*=(VecMap<T>& out, const T& scalar) {
	std::function<void(T&)> f = [=](T& val1) {val1 *= scalar;};
	Transform(out, f);
	return out;
}
template<class T> VecMap<T>& operator/=(VecMap<T>& out, const T& scalar) {
	std::function<void(T&)> f = [=](T& val1) {val1 /= scalar;};
	Transform(out, f);
	return out;
}

template<class T> Vec<T>& operator+=(Vec<T>& out, const VecType<T>& img) {
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 += val2;};
	Transform(out, img, f);
	return out;
}
template<class T> Vec<T>& operator-=(Vec<T>& out, const VecType<T>& img) {
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 -= val2;};
	Transform(out, img, f);
	return out;
}
template<class T> Vec<T>& operator*=(Vec<T>& out, const VecType<T>& img) {
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 *= val2;};
	Transform(out, img, f);
	return out;
}
template<class T> Vec<T>& operator/=(Vec<T>& out, const VecType<T>& img) {
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 /= val2;};
	Transform(out, img, f);
	return out;
}
template<class T> Vec<T>& operator+=(Vec<T>& out, const T& scalar) {
	std::function<void(T&)> f = [=](T& val1) {val1 += scalar;};
	Transform(out, f);
	return out;
}
template<class T> Vec<T>& operator-=(Vec<T>& out, const T& scalar) {
	std::function<void(T&)> f = [=](T& val1) {val1 -= scalar;};
	Transform(out, f);
	return out;
}
template<class T> Vec<T>& operator*=(Vec<T>& out, const T& scalar) {
	std::function<void(T&)> f = [=](T& val1) {val1 *= scalar;};
	Transform(out, f);
	return out;
}
template<class T> Vec<T>& operator/=(Vec<T>& out, const T& scalar) {
	std::function<void(T&)> f = [=](T& val1) {val1 /= scalar;};
	Transform(out, f);
	return out;
}

template<class T> Vec<T> operator+(const VecType<T>& img1,
		const VecType<T>& img2) {
	Vec<T> out(img1.size());
	std::function<void(T&, const T&, const T&)> f =
			[=](T& val1, const T& val2, const T& val3) {val1 = val2 + val3;};
	Transform(out, img1, img2, f);
	return out;
}
template<class T> Vec<T> operator-(const VecType<T>& img1,
		const VecType<T>& img2) {
	Vec<T> out(img1.size());
	std::function<void(T&, const T&, const T&)> f =
			[=](T& val1, const T& val2, const T& val3) {val1 = val2 - val3;};
	Transform(out, img1, img2, f);
	return out;
}
template<class T> Vec<T> operator*(const VecType<T>& img1,
		const VecType<T>& img2) {
	Vec<T> out(img1.size());
	std::function<void(T&, const T&, const T&)> f =
			[=](T& val1, const T& val2, const T& val3) {val1 = val2*val3;};
	Transform(out, img1, img2, f);
	return out;
}
template<class T> Vec<T> operator/(const VecType<T>& img1,
		const VecType<T>& img2) {
	Vec<T> out(img1.size());
	std::function<void(T&, const T&, const T&)> f =
			[=](T& val1, const T& val2, const T& val3) {val1 = val2 / val3;};
	Transform(out, img1, img2, f);
	return out;
}
template<class T> struct DenseMat {
public:
	std::vector<T> data; //, aligned_allocator<T, 64>
	int rows, cols;
	typedef T ValueType;
	typedef typename std::vector<ValueType>::iterator iterator;
	typedef typename std::vector<ValueType>::const_iterator const_iterator;
	typedef typename std::vector<ValueType>::reverse_iterator reverse_iterator;
	size_t size() const {
		return rows * cols;
	}
	inline void setZero() {
		data.assign(data.size(), T(0));
	}
	iterator begin(int i) const {
		return data[i].begin();
	}
	iterator end(int i) const {
		return data[i].end();
	}
	iterator begin(int i) {
		return data[i].begin();
	}
	iterator end(int i) {
		return data[i].end();
	}
	const_iterator cbegin(int i) const {
		return data[i].cbegin();
	}
	const_iterator cend(int i) const {
		return data[i].cend();
	}
	reverse_iterator rbegin(int i) {
		return data[i].rbegin();
	}
	reverse_iterator rend(int i) {
		return data[i].rend();
	}
	reverse_iterator rbegin(int i) const {
		return data[i].rbegin();
	}
	reverse_iterator rend(int i) const {
		return data[i].rend();
	}
	template<class Archive> void serialize(Archive & archive) {
		archive(CEREAL_NVP(rows), CEREAL_NVP(cols),
				cereal::make_nvp(MakeString() << "matrix", data));
	}
	T* operator[](size_t i) {
		return &data[cols * i];
	}
	const T* operator[](size_t i) const {
		return &data[cols * i];
	}
	aly::dim2 dimensions() const {
		return aly::dim2(rows, cols);
	}
	DenseMat() :
			rows(0), cols(0) {
	}
	DenseMat(int rows, int cols) :
			rows(rows), cols(cols) {
		data.resize(rows * (size_t) cols);
	}
	void resize(int rows, int cols) {
		if (this->rows != rows || this->cols != cols) {
			data.resize(rows * (size_t) cols);
			this->rows = rows;
			this->cols = cols;
		}
	}
	void set(size_t i, size_t j, const T& value) {
		if (i >= (size_t) rows || j >= (size_t) cols || i < 0 || j < 0)
			throw std::runtime_error(
					MakeString() << "Index (" << i << "," << j
							<< ") exceeds matrix bounds [" << rows << ","
							<< cols << "]");
		data[cols * i + j] = value;
	}
	T get(size_t i, size_t j) const {
		if (i >= (size_t) rows || j >= (size_t) cols || i < 0 || j < 0)
			throw std::runtime_error(
					MakeString() << "Index (" << i << "," << j
							<< ") exceeds matrix bounds [" << rows << ","
							<< cols << "]");
		return data[cols * i + j];
	}
	inline VecMap<T> getRow(size_t i) const {
		if (i >= rows)
			throw std::runtime_error(
					MakeString() << "Row index " << i << "/" << rows
							<< " out of bounds.");
		return VecMap<T>((T*) (&data[cols * i]), cols, 1); //forces const ptr to regular ptr
	}
	inline VecMap<T> getColumn(size_t j) const {
		if (j >= cols)
			throw std::runtime_error(
					MakeString() << "Column index " << j << "/" << cols
							<< " out of bounds.");
		return VecMap<T>((T*) (&data[j]), rows, cols); //forces const ptr to regular ptr
	}

	inline void set(const DenseMat<T>& mat) {
		rows = mat.rows;
		cols = mat.cols;
		data = mat.data;
	}
	inline void setRow(const VecType<T>& vec, size_t i) {
		if (vec.size() != cols) {
			throw std::runtime_error(
					"Could not set row because vector length doesn't match columns.");
		}
		for (int j = 0; j < cols; j++) {
			data[cols * i + j] = vec[j];
		}
	}
	inline void setColumn(const VecType<T>& vec, size_t j) {
		if (vec.size() != rows) {
			throw std::runtime_error(
					"Could not set column because vector length doesn't match rows.");
		}
		for (int i = 0; i < rows; i++) {
			data[cols * i + j] = vec[i];
		}
	}
	T& operator()(int i, int j) {
		assert(i<rows);
		assert(j<cols);
		assert(i>=0);
		assert(j>=0);
		return data[cols * (size_t)i + j];
	}
	const T& operator()(int i, int j) const {
		assert(i<rows);
		assert(j<cols);
		assert(i>=0);
		assert(j>=0);
		return data[cols * (size_t)i + j];
	}
	inline bool contains(int i,int j) const {
		return (i>=0&& j>=0 && i<rows && j<cols);
	}
	inline DenseMat<T> transpose() const {
		DenseMat<T> M(cols, rows);
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				M(j, i) = operator()(i, j);
			}
		}
		return M;
	}
	inline DenseMat<T> square() const {
		//Computes AtA
		DenseMat<T> A(cols, cols);
		A.setZero();
		for (int i = 0; i < cols; i++) {
			for (int j = 0; j <=i; j++) {
				T sum = 0;
				for (int k = 0; k < rows; k++) {
					sum += operator()(k, i) * operator()(k, j);

				}
				A(i, j) = sum;
				if(j!=i)A(j, i) = sum;
			}
		}
		return A;
	}
	inline static DenseMat<T> identity(size_t M, size_t N) {
		DenseMat<T> A(M, N);
		for (int i = 0; i < M; i++) {
			for (int j = 0; j < N; j++) {
				A(i, j) = T(T((i == j) ? 1 : 0));
			}
		}
		return A;
	}
	inline static DenseMat<T> zero(size_t M, size_t N) {
		DenseMat<T> A(M, N);
		for (int i = 0; i < M; i++) {
			for (int j = 0; j < N; j++) {
				A(i, j) = T(T(0));
			}
		}
		return A;
	}
	inline static DenseMat<T> diagonal(const Vec<T>& v) {
		DenseMat<T> A((int) v.size(), (int) v.size());
		for (int i = 0; i < A.rows; i++) {
			for (int j = 0; j < A.cols; j++) {
				A(i, j) = T(T((i == j) ? v[i] : 0));
			}
		}
		return A;
	}
	template<ImageType I> void set(const Image<T,1,I>& img){
		resize(img.height,img.width);
		for(size_t idx=0;idx<img.size();idx++){
			data[idx]=img[idx].x;
		}
	}
	template<ImageType I> void get(Image<T,1,I>& img){
		img.resize(cols,rows);
		for(size_t idx=0;idx<img.size();idx++){
			img[idx].x=data[idx];
		}
	}
	void set(const std::vector<T>& in){
		data=in;
	}
	template<int C,ImageType I> void set(const Image<T,C,I>& img, int c){
		resize(img.height,img.width);
		for(size_t idx=0;idx<img.size();idx++){
			data[idx]=img[idx][c];
		}
	}
	DenseMat<T>& operator=(const DenseMat<T>& rhs) {
		if (this == &rhs) {
			return *this;
		}
		resize(rhs.rows, rhs.cols);
		data = rhs.data;
		return *this;
	}
};
template<class A, class B, class T> std::basic_ostream<A, B> & operator <<(
		std::basic_ostream<A, B> & ss, const DenseMat<T>& M) {
	ss << "\n";
	for (int i = 0; i < M.rows; i++) {
		ss << "[";
		for (int j = 0; j < M.cols; j++) {
			ss << std::setprecision(10) << std::setw(16) << M(i, j)
					<< ((j < M.cols - 1) ? "," : "]\n");
		}
	}
	return ss;
}

template<class T> Vec<T> operator*(const DenseMat<T>& A, const VecType<T>& v) {
	Vec<T> out(A.rows);
	for (int i = 0; i < A.rows; i++) {
		T sum(0.0);
		for (int j = 0; j < A.cols; j++) {
			sum += A(i, j) * v[j];
		}
		out[i] = sum;
	}
	return out;
}
template<class T> DenseMat<T> operator*(const DenseMat<T>& A,
		const DenseMat<T>& B) {
	if (A.cols != B.rows)
		throw std::runtime_error(
				MakeString()
						<< "Cannot multiply matrices. Inner dimensions do not match. "
						<< "[" << A.rows << "," << A.cols << "] * [" << B.rows
						<< "," << B.cols << "]");
	DenseMat<T> out(A.rows, B.cols);
	for (int i = 0; i < out.rows; i++) {
		for (int j = 0; j < out.cols; j++) {
			T sum(0.0);
			for (int k = 0; k < A.cols; k++) {
				sum += A(i, k) * B(k, j);
			}
			out(i, j) = sum;
		}
	}
	return out;
}
//Slight abuse of mathematics here. Vectors are always interpreted as column vectors as a convention,
//so this multiplcation is equivalent to multiplying "A" with a diagonal matrix constructed from "W".
//To multiply a matrix with a column vector to get a row vector, convert "W" to a dense matrix.
template<class T> DenseMat<T> operator*(const VecType<T>& W,
		const DenseMat<T>& A) {
	if (A.rows != (int) W.size())
		throw std::runtime_error(
				MakeString()
						<< "Cannot scale matrix by vector. Rows must match. "
						<< "[" << W.size() << "] * [" << A.rows << "," << A.cols
						<< "]");
	DenseMat<T> out(A.rows, A.cols);
	for (int i = 0; i < out.rows; i++) {
		for (int j = 0; j < out.cols; j++) {
			out(i, j) = W[i] * A(i, j);
		}
	}
	return out;
}
template<class T> DenseMat<T>& operator*=(DenseMat<T>& A, const VecType<T>& W) {
	if (A.rows != W.size())
		throw std::runtime_error(
				MakeString()
						<< "Cannot scale matrix by vector. Rows must match. "
						<< "[" << W.size() << "] * [" << A.rows << "," << A.cols
						<< "]");
	for (int i = 0; i < A.rows; i++) {
		for (int j = 0; j < A.cols; j++) {
			A(i, j) *= W[i];
		}
	}
	return A;
}
template<class T> DenseMat<T> operator-(const DenseMat<T>& A) {
	DenseMat<T> out(A.rows, A.cols);
	for (int i = 0; i < (int) out.rows; i++) {
		for (int j = 0; j < out.cols; j++) {
			out(i, j) = -A(i, j);
		}
	}
	return out;
}
template<class T> DenseMat<T> operator-(const DenseMat<T>& A,
		const DenseMat<T>& B) {
	if (A.rows != B.rows || A.cols != B.cols) {
		throw std::runtime_error(
				"Cannot subtract matricies. Matrix dimensions must match.");
	}
	DenseMat<T> out(A.rows, A.cols);
	for (int i = 0; i < (int) out.rows; i++) {
		for (int j = 0; j < out.cols; j++) {
			out(i, j) = A(i, j) - B(i, j);
		}
	}
	return out;
}
template<class T> DenseMat<T> operator+(const DenseMat<T>& A,
		const DenseMat<T>& B) {
	if (A.rows != B.rows || A.cols != B.cols) {
		throw std::runtime_error(
				"Cannot add matricies. Matrix dimensions must match.");
	}
	DenseMat<T> out(A.rows, A.cols);
	for (int i = 0; i < (int) out.rows; i++) {
		for (int j = 0; j < out.cols; j++) {
			out(i, j) = A(i, j) + B(i, j);
		}
	}
	return out;
}
template<class T> DenseMat<T> operator*(const DenseMat<T>& A, const T& v) {
	DenseMat<T> out(A.rows, A.cols);
	for (int i = 0; i < A.rows; i++) {
		for (int j = 0; j < A.cols; j++) {
			out(i, j) = A(i, j) * v;
		}
	}
	return out;
}
template<class T> DenseMat<T> operator/(const DenseMat<T>& A, const T& v) {
	DenseMat<T> out(A.rows, A.cols);
	for (int i = 0; i < A.rows; i++) {
		for (int j = 0; j < A.cols; j++) {
			out(i, j) = A(i, j) / v;
		}
	}
	return out;
}
template<class T> DenseMat<T> operator+(const DenseMat<T>& A, const T& v) {
	DenseMat<T> out(A.rows, A.cols);
	for (int i = 0; i < A.rows; i++) {
		for (int j = 0; j < A.cols; j++) {
			out(i, j) = A(i, j) + v;
		}
	}
	return out;
}
template<class T> DenseMat<T> operator-(const DenseMat<T>& A, const T& v) {
	DenseMat<T> out(A.rows, A.cols);
	for (int i = 0; i < A.rows; i++) {
		for (int j = 0; j < A.cols; j++) {
			out(i, j) = A(i, j) - v;
		}
	}
	return out;
}
template<class T> DenseMat<T> operator*(const T& v, const DenseMat<T>& A) {
	DenseMat<T> out(A.rows, A.cols);
	for (int i = 0; i < A.rows; i++) {
		for (int j = 0; j < A.cols; j++) {
			out(i, j) = v * A(i, j);
		}
	}
	return out;
}
template<class T> DenseMat<T> operator/(const T& v, const DenseMat<T>& A) {
	DenseMat<T> out(A.rows, A.cols);
	for (int i = 0; i < A.rows; i++) {
		for (int j = 0; j < A.cols; j++) {
			out(i, j) = v / A(i, j);
		}
	}
	return out;
}
template<class T> DenseMat<T> operator+(const T& v, const DenseMat<T>& A) {
	DenseMat<T> out(A.rows, A.cols);
	for (int i = 0; i < A.rows; i++) {
		for (int j = 0; j < A.cols; j++) {
			out(i, j) = v + A(i, j);
		}
	}
	return out;
}
template<class T> DenseMat<T> operator-(const T& v, const DenseMat<T>& A) {
	DenseMat<T> out(A.rows, A.cols);
	for (int i = 0; i < A.rows; i++) {
		for (int j = 0; j < A.cols; j++) {
			out(i, j) = v - A(i, j);
		}
	}
	return out;
}
template<class T> DenseMat<T>& operator*=(DenseMat<T>& A, const T& v) {
	for (int i = 0; i < A.rows; i++) {
		for (int j = 0; j < A.cols; j++) {
			A(i, j) = A(i, j) * v;
		}
	}
	return A;
}
template<class T> DenseMat<T>& operator/=(DenseMat<T>& A, const T& v) {
	for (int i = 0; i < A.rows; i++) {
		for (int j = 0; j < A.cols; j++) {
			A(i, j) = A(i, j) / v;
		}
	}
	return A;
}
template<class T> DenseMat<T>& operator+=(DenseMat<T>& A, const T& v) {
	for (int i = 0; i < A.rows; i++) {
		for (int j = 0; j < A.cols; j++) {
			A(i, j) = A(i, j) + v;
		}
	}
	return A;
}
template<class T> DenseMat<T>& operator-=(DenseMat<T>& A, const T& v) {
	for (int i = 0; i < A.rows; i++) {
		for (int j = 0; j < A.cols; j++) {
			A(i, j) = A(i, j) - v;
		}
	}
	return A;
}

template<class T> void WriteDenseMatToFile(const std::string& file,
		const DenseMat<T>& matrix) {
	std::ofstream os(file);
	cereal::PortableBinaryOutputArchive ar(os);
	ar(cereal::make_nvp("dense_matrix", matrix));
}
template<class T> void ReadDenseMatFromFile(const std::string& file,
		DenseMat<T>& matrix) {
	std::ifstream os(file);
	cereal::PortableBinaryInputArchive ar(os);
	ar(cereal::make_nvp("dense_matrix", matrix));
}

template<class T> struct SparseMat {
private:
	std::vector<std::map<size_t, T>> storage;
public:
	size_t rows, cols;
	SparseMat() :
			rows(0), cols(0) {

	}
	template<class Archive> void serialize(Archive & archive) {
		archive(CEREAL_NVP(rows), CEREAL_NVP(cols),
				cereal::make_nvp(MakeString() << "matrix", storage));
	}
	std::map<size_t, T>& operator[](size_t i) {
		if (i >= rows || i < 0)
			throw std::runtime_error(
					MakeString() << "Index (" << i
							<< ",*) exceeds matrix bounds [" << rows << ","
							<< cols << "]");
		return storage[i];
	}
	const std::map<size_t, T>& operator[](size_t i) const {
		if (i >= rows || i < 0)
			throw std::runtime_error(
					MakeString() << "Index (" << i
							<< ",*) exceeds matrix bounds [" << rows << ","
							<< cols << "]");
		return storage[i];
	}
	SparseMat(size_t rows, size_t cols) :
			storage(rows), rows(rows), cols(cols) {
	}
	size_t size() const {
		size_t count = 0;
		for (const std::map<size_t, T>& vec : storage) {
			count += vec.size();
		}
		return count;
	}
	void resize(size_t rows, size_t cols) {
		this->rows = rows;
		this->cols = cols;
		storage.resize(rows);
	}
	void set(size_t i, size_t j, const T& value) {
		if (i >= rows || j >= cols || i < 0 || j < 0)
			throw std::runtime_error(
					MakeString() << "Index (" << i << "," << j
							<< ") exceeds matrix bounds [" << rows << ","
							<< cols << "]");
		storage[i][j] = value;
	}
	T& operator()(size_t i, size_t j) {
		if (i >= rows || j >= cols || i < 0 || j < 0)
			throw std::runtime_error(
					MakeString() << "Index (" << i << "," << j
							<< ") exceeds matrix bounds [" << rows << ","
							<< cols << "]");
		return storage[i][j];
	}

	T get(size_t i, size_t j) const {
		if (i >= rows || j >= cols || i < 0 || j < 0)
			throw std::runtime_error(
					MakeString() << "Index (" << i << "," << j
							<< ") exceeds matrix bounds [" << rows << ","
							<< cols << "]");
		if (storage[i].find(j) == storage[i].end()) {
			return T(T(0));
		} else {
			return storage[i].at(j);
		}
	}
	T operator()(size_t i, size_t j) const {
		return get(i, j);
	}
	SparseMat<T> transpose() const {
		SparseMat<T> M(cols, rows);
		for (int i = 0; i < (int) storage.size(); i++) {
			for (const std::pair<size_t, T>& iv : storage[i]) {
				M.set(iv.first, i, iv.second);
			}
		}
		return M;
	}
	static SparseMat<T> identity(size_t M, size_t N) {
		SparseMat<T> A(M, N);
		int K = (int) aly::min(M, N);
#pragma omp parallel for
		for (int k = 0; k < K; k++) {
			A[k][k] = T(T(1));
		}
		return A;
	}
	static SparseMat<T> diagonal(const VecType<T>& v) {
		SparseMat<T> A(v.size(), v.size());
#pragma omp parallel for
		for (int k = 0; k < (int) v.size(); k++) {
			A[k][k] = v[k];
		}
		return A;
	}
};
template<class A, class B, class T> std::basic_ostream<A, B> & operator <<(
		std::basic_ostream<A, B> & ss, const SparseMat<T>& M) {
	for (int i = 0; i < (int) M.rows; i++) {
		ss << "M[" << i << ",*]=";
		for (const std::pair<size_t, T>& pr : M[i]) {
			ss << "<" << pr.first << ":" << pr.second << "> ";
		}
		ss << std::endl;
	}
	return ss;
}

template<class T> SparseMat<T>& operator*=(SparseMat<T>& A, const T& v) {
#pragma omp parallel for
	for (int i = 0; i < (int) A.rows; i++) {
		for (std::pair<size_t, T>& pr : A[i]) {
			A[i][pr.first] = pr.second * v;
		}
	}
	return A;
}
template<class T> SparseMat<T>& operator/=(const SparseMat<T>& A, const T& v) {
#pragma omp parallel for
	for (int i = 0; i < (int) A.rows; i++) {
		for (std::pair<size_t, T>& pr : A[i]) {
			A[i][pr.first] = pr.second / v;
		}
	}
	return A;
}
template<class T> SparseMat<T>& operator+=(SparseMat<T>& A, const T& v) {
#pragma omp parallel for
	for (int i = 0; i < (int) A.rows; i++) {
		for (std::pair<size_t, T>& pr : A[i]) {
			A[i][pr.first] = pr.second + v;
		}
	}
	return A;
}
template<class T> SparseMat<T>& operator-=(SparseMat<T>& A, const T& v) {
#pragma omp parallel for
	for (int i = 0; i < (int) A.rows; i++) {
		for (std::pair<size_t, T>& pr : A[i]) {
			A[i][pr.first] = pr.second - v;
		}
	}
	return A;
}

template<class T> SparseMat<T>& operator/=(SparseMat<T>& A, const T& v) {
#pragma omp parallel for
	for (int i = 0; i < (int) A.rows; i++) {
		for (std::pair<size_t, T>& pr : A[i]) {
			A[i][pr.first] = pr.second / v;
		}
	}
	return A;
}

template<class T> SparseMat<T> operator*(const SparseMat<T>& A,
		const SparseMat<T>& B) {
	if (A.cols != B.rows)
		throw std::runtime_error(
				MakeString()
						<< "Cannot multiply matrices. Inner dimensions do not match. "
						<< "[" << A.rows << "," << A.cols << "] * [" << B.rows
						<< "," << B.cols << "]");
	SparseMat<T> out(A.rows, B.cols);
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) { //a[i,*]
		for (std::pair<size_t, T> pr1 : A[i]) { //a[i,k]
			int k = (int) pr1.first;
			for (std::pair<size_t, T> pr2 : B[k]) { //b[k,j]
				int j = (int) pr2.first;
				out(i, j) += pr1.second * pr2.second;
			}
		}
	}
	return out;
}

template<class T> SparseMat<T> operator*(const T& v, const SparseMat<T>& A) {
	SparseMat<T> out = A;
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) {
		for (std::pair<size_t, T> pr : A[i]) {
			out[i][pr.first] = v * pr.second;
		}
	}
	return out;
}
template<class T> SparseMat<T> operator/(const T& v, const SparseMat<T>& A) {
	SparseMat<T> out = A;
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) {
		for (std::pair<size_t, T> pr : A[i]) {
			out[i][pr.first] = v / pr.second;
		}
	}
	return out;
}
template<class T> SparseMat<T> operator+(const T& v, const SparseMat<T>& A) {
	SparseMat<T> out = A;
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) {
		for (std::pair<size_t, T> pr : A[i]) {
			out[i][pr.first] = T(v) + pr.second;
		}
	}
	return out;
}
template<class T> SparseMat<T> operator-(const T& v, const SparseMat<T>& A) {
	SparseMat<T> out = A;
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) {
		for (std::pair<size_t, T> pr : A[i]) {
			out[i][pr.first] = T(v) - pr.second;
		}
	}
	return out;
}

template<class T> SparseMat<T> operator-(const SparseMat<T>& A, const T& v) {
	SparseMat<T> out = A;
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) {
		for (std::pair<size_t, T> pr : A[i]) {
			out[i][pr.first] = pr.second - v;
		}
	}
	return out;
}
template<class T> SparseMat<T> operator+(const SparseMat<T>& A, const T& v) {
	SparseMat<T> out = A;
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) {
		for (std::pair<size_t, T> pr : A[i]) {
			out[i][pr.first] = pr.second + v;
		}
	}
	return out;
}
template<class T> SparseMat<T> operator*(const SparseMat<T>& A, const T& v) {
	SparseMat<T> out = A;
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) {
		for (std::pair<size_t, T> pr : A[i]) {
			out[i][pr.first] = pr.second * v;
		}
	}
	return out;
}

template<class T> SparseMat<T> operator/(const SparseMat<T>& A, const T& v) {
	SparseMat<T> out = A;
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) {
		for (std::pair<size_t, T> pr : A[i]) {
			out[i][pr.first] = pr.second / v;
		}
	}
	return out;
}

template<class T> SparseMat<T> operator-(const SparseMat<T>& A) {
	SparseMat<T> out = A;
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) {
		for (std::pair<size_t, T> pr : A[i]) {
			out[i][pr.first] = -pr.second;
		}
	}
	return out;
}
template<class T> SparseMat<T> operator+(const SparseMat<T>& A,
		const SparseMat<T>& B) {
	if (A.rows != B.rows || A.cols != B.cols)
		throw std::runtime_error(
				MakeString() << "Cannot add matrices. Dimensions do not match. "
						<< "[" << A.rows << "," << A.cols << "] * [" << B.rows
						<< "," << B.cols << "]");
	SparseMat<T> out = A;
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) {
		for (std::pair<size_t, T> pr : B[i]) {
			out[i][pr.first] += pr.second;
		}
	}
	return out;
}
template<class T> SparseMat<T> operator-(const SparseMat<T>& A,
		const SparseMat<T>& B) {
	if (A.rows != B.rows || A.cols != B.cols)
		throw std::runtime_error(
				MakeString()
						<< "Cannot subtract matrices. Dimensions do not match. "
						<< "[" << A.rows << "," << A.cols << "] * [" << B.rows
						<< "," << B.cols << "]");
	SparseMat<T> out = A;
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) {
		for (std::pair<size_t, T> pr : B[i]) {
			out[i][pr.first] -= pr.second;
		}
	}
	return out;
}
template<class T> SparseMat<T>& operator+=(SparseMat<T>& A,
		const SparseMat<T>& B) {
	if (A.rows != B.rows || A.cols != B.cols)
		throw std::runtime_error(
				MakeString() << "Cannot add matrices. Dimensions do not match. "
						<< "[" << A.rows << "," << A.cols << "] * [" << B.rows
						<< "," << B.cols << "]");
#pragma omp parallel for
	for (int i = 0; i < (int) A.rows; i++) {
		for (std::pair<size_t, T> pr : B[i]) {
			A[i][pr.first] += pr.second;
		}
	}
	return A;
}
template<class T> SparseMat<T>& operator-=(SparseMat<T>& A,
		const SparseMat<T>& B) {
	if (A.rows != B.rows || A.cols != B.cols)
		throw std::runtime_error(
				MakeString()
						<< "Cannot subtract matrices. Dimensions do not match. "
						<< "[" << A.rows << "," << A.cols << "] * [" << B.rows
						<< "," << B.cols << "]");
#pragma omp parallel for
	for (int i = 0; i < (int) A.rows; i++) {
		for (std::pair<size_t, T> pr : B[i]) {
			A[i][pr.first] -= pr.second;
		}
	}
	return A;
}
template<class T> void Multiply(Vec<T>& out, const SparseMat<T>& A,
		const Vec<T>& v) {
	out.resize(A.rows);
#pragma omp parallel for
	for (int i = 0; i < (int) A.rows; i++) {
		double sum = (0.0);
		for (const std::pair<size_t, T>& pr : A[i]) {
			sum += double(v[pr.first]) * (double) pr.second;
		}
		out[i] = T(sum);
	}
}
template<class T> void AddMultiply(Vec<T>& out, const VecType<T>& b,
		const SparseMat<T>& A, const VecType<T>& v) {
	out.resize(A.rows);
#pragma omp parallel for
	for (int i = 0; i < (int) A.rows; i++) {
		double sum = 0.0;
		for (const std::pair<size_t, T>& pr : A[i]) {
			sum += double(v[pr.first]) * (double) pr.second;
		}
		out[i] = b[i] + T(sum);
	}
}
template<class T> void SubtractMultiply(Vec<T>& out, const VecType<T>& b,
		const SparseMat<T>& A, const VecType<T>& v) {
	out.resize(A.rows);
#pragma omp parallel for
	for (int i = 0; i < (int) A.rows; i++) {
		double sum = 0.0;
		for (const std::pair<size_t, T>& pr : A[i]) {
			sum += double(v[pr.first]) * (double) pr.second;
		}
		out[i] = b[i] - T(sum);
	}
}
template<class T> Vec<T> operator*(const SparseMat<T>& A, const VecType<T>& v) {
	Vec<T> out(A.rows);
#pragma omp parallel for
	for (int i = 0; i < (int) A.rows; i++) {
		double sum = 0.0;
		for (const std::pair<size_t, T>& pr : A[i]) {
			sum += double(v[pr.first]) * double(pr.second);
		}
		out[i] = T(sum);
	}
	return out;
}
template<class T> void MultiplyVec(Vec<T>& out, const SparseMat<T>& A,
		const VecType<T>& v) {
	out.resize(A.rows);
#pragma omp parallel for
	for (int i = 0; i < (int) A.rows; i++) {
		double sum = 0.0;
		for (const std::pair<size_t, T>& pr : A[i]) {
			sum += double(v[pr.first]) * double(pr.second);
		}
		out[i] = T(sum);
	}
}

template<class T> void AddMultiplyVec(Vec<T>& out, const VecType<T>& b,
		const SparseMat<T>& A, const VecType<T>& v) {
	out.resize(A.rows);
#pragma omp parallel for
	for (int i = 0; i < A.rows; i++) {
		double sum = 0.0;
		for (const std::pair<size_t, T>& pr : A[i]) {
			sum += double(v[pr.first]) * double(pr.second);
		}
		out[i] = b[i] + T(sum);
	}
}
template<class T> void SubtractMultiplyVec(Vec<T>& out, const VecType<T>& b,
		const SparseMat<T>& A, const VecType<T>& v) {
	out.resize(A.rows);
#pragma omp parallel for
	for (int i = 0; i < (int) A.rows; i++) {
		double sum = 0.0;
		for (const std::pair<size_t, T>& pr : A[i]) {
			sum += double(v[pr.first]) * double(pr.second);
		}
		out[i] = b[i] - T(sum);
	}
}
template<class T> void WriteSparseMatToFile(const std::string& file,
		const SparseMat<T>& matrix) {
	std::ofstream os(file);
	cereal::PortableBinaryOutputArchive ar(os);
	ar(cereal::make_nvp("sparse_matrix", matrix));
}
template<class T> void ReadSparseMatFromFile(const std::string& file,
		SparseMat<T>& matrix) {
	std::ifstream os(file);
	cereal::PortableBinaryInputArchive ar(os);
	ar(cereal::make_nvp("sparse_matrix", matrix));
}
template<class T> double lengthSqr(const VecType<T>& a) {
	size_t sz = a.size();
	double cans = 0;
#pragma omp parallel for reduction(+:cans)
	for (int i = 0; i < (int) sz; i++) {
		double val = a[i];
		cans += val * val;
	}
	return cans;
}
template<class T> double distanceSqr(const VecType<T>& a, const VecType<T>& b) {
	if (a.size() != b.size())
		throw std::runtime_error(
				MakeString() << "Vector dimensions do not match. " << a.size()
						<< "!=" << b.size());
	size_t sz = a.size();
	double cans = 0;
#pragma omp parallel for reduction(+:cans)
	for (int i = 0; i < (int) sz; i++) {
		double val = a[i] - b[i];
		cans += val * val;
	}
	return cans;
}
template<class T> double distance(const VecType<T>& a, const VecType<T>& b) {
	return std::sqrt(distanceSqr(a, b));
}
template<class T> double length(const VecType<T>& a) {
	return std::sqrt(lengthSqr(a));
}
template<class T> double dot(const VecType<T>& a, const VecType<T>& b) {
	double ans = 0.0;
	if (a.size() != b.size())
		throw std::runtime_error(
				MakeString() << "Vector dimensions do not match. " << a.size()
						<< "!=" << b.size());
	size_t sz = a.size();
#pragma omp parallel for reduction(+:ans)
	for (int i = 0; i < (int) sz; i++) {
		ans += double(a[i]) * double(b[i]);
	}
	return ans;
}
template<class T> T lengthL1(const VecType<T>& a) {
	T ans(0);
	size_t sz = a.size();
#pragma omp parallel for reduction(+:ans)
	for (int i = 0; i < (int) sz; i++) {
		ans += std::abs(a[i]);
	}
	return ans;
}
template<class T> T reduce(const VecType<T>& a) {
	T ans(0);
	size_t sz = a.size();
#pragma omp parallel for reduction(+:ans)
	for (int i = 0; i < (int) sz; i++) {
		ans += a[i];
	}
	return ans;
}
template<class T> T lengthInf(const VecType<T>& a) {
	T ans(0);
	size_t sz = a.size();
	for (int i = 0; i < (int) sz; i++) {
		ans = std::max(ans, std::abs(a[i]));
	}
	return ans;
}
template<class T> struct DenseVol {
public:
	std::vector<T> data;
	typedef T ValueType;
	typedef typename std::vector<ValueType>::iterator iterator;
	typedef typename std::vector<ValueType>::const_iterator const_iterator;
	typedef typename std::vector<ValueType>::reverse_iterator reverse_iterator;
	iterator begin() {
		return data.begin();
	}
	iterator end() {
		return data.end();
	}
	const_iterator cbegin() const {
		return data.cbegin();
	}
	const_iterator cend() const {
		return data.cend();
	}
	reverse_iterator rbegin() {
		return data.rbegin();
	}
	reverse_iterator rend() {
		return data.rend();
	}
	reverse_iterator rbegin() const {
		return data.rbegin();
	}
	reverse_iterator rend() const {
		return data.rend();
	}
	int rows;
	int cols;
	int slices;
	inline bool contains(const float3& pt) {
		return (pt.x >= 0 && pt.y >= 0 && pt.z >= 0 && pt.x < rows
				&& pt.y < cols && pt.z < slices);
	}
	inline bool contains(const int3& pt) {
		return (pt.x >= 0 && pt.y >= 0 && pt.z >= 0 && pt.x < rows
				&& pt.y < cols && pt.z < slices);
	}
	inline bool contains(int x, int y, int z) {
		return (x >= 0 && y >= 0 && z >= 0 && x < rows && y < cols && z < slices);
	}
	inline bool contains(float x, float y, float z) {
		return (x >= 0 && y >= 0 && z >= 0 && x < rows && y < cols && z < slices);
	}
	void set(const T& val) {
		data.assign(data.size(), T(val));
	}
	void set(const std::vector<T>& val) {
		data = val;
	}
	inline void set(const DenseVol<T>& vol) {
		rows = vol.rows;
		cols = vol.cols;
		slices = vol.slices;
		data = vol.data;
	}
	void set(T* val) {
		if (val == nullptr)
			return;
		size_t offset = 0;
		for (T& x : data) {
			x = val[offset++];
		}
	}
	void set(const T* val) {
		if (val == nullptr)
			return;
		size_t offset = 0;
		for (T& x : data) {
			x = val[offset++];
		}
	}
	DenseVol<T>& operator=(const DenseVol<T>& rhs) {
		if (this == &rhs) {
			return *this;
		}
		rows = rhs.rows;
		cols = rhs.cols;
		slices = rhs.slices;
		data = rhs.data;
		return *this;
	}
	DenseVol(int r, int c, int s) :
			rows(r), cols(c), slices(s) {
		data.resize(r * c * s);
	}
	DenseVol(T* ptr, int r, int c, int s) :
			rows(r), cols(c), slices(s) {
		set(ptr);
	}
	DenseVol(const T* ptr, int r, int c, int s) :
			rows(r), cols(c), slices(s) {
		set(ptr);
	}

	DenseVol(const std::vector<T>& ref, int r, int c, int s) :
			rows(r), cols(c), slices(s) {
		data = ref;
	}
	DenseVol() :
			rows(0), cols(0), slices(0) {
	}
	DenseVol(const DenseVol<T>& img) :
			DenseVol(img.rows, img.cols, img.slices) {
		set(img.data);
	}

	dim3 dimensions() const {
		return dim3(rows, cols, slices);
	}
	size_t size() const {
		return data.size();
	}
	void resize(int r, int c, int s) {
		data.resize(r * c * s);
		data.shrink_to_fit();
		rows = r;
		cols = c;
		slices = s;
	}
	void resize(int3 dims) {
		resize(dims.x, dims.y, dims.z);
	}
	inline void clear() {
		data.clear();
		data.shrink_to_fit();
		rows = 0;
		cols = 0;
		slices = 0;
	}
	T* ptr() {
		if (data.size() == 0)
			return nullptr;
		return &(data[0]);
	}
	const T* ptr() const {
		if (data.size() == 0)
			return nullptr;
		return &(data[0]);
	}
	void setZero() {
		data.assign(data.size(), T(0));
	}
	const T& operator[](const size_t i) const {
		return data[i];
	}
	T& operator[](const size_t i) {
		return data[i];
	}
	T& operator()(const int i, const int j, const int k) {
		return data[clamp(i, 0, rows - 1)
				+ clamp(j, 0, cols - 1) * (size_t) rows
				+ clamp(k, 0, slices - 1) * (size_t) rows * (size_t) cols];
	}
	T& operator()(const size_t i, const size_t j, const size_t k) {
		return data[clamp((int) i, 0, rows - 1)
				+ clamp((int) j, 0, cols - 1) * (size_t) rows
				+ clamp((int) k, 0, slices - 1) * (size_t) rows * (size_t) cols];
	}
	T& operator()(const int3 ijk) {
		return data[clamp(ijk.x, 0, rows - 1)
				+ clamp(ijk.y, 0, cols - 1) * (size_t) rows
				+ clamp(ijk.z, 0, slices - 1) * (size_t) rows * (size_t) cols];
	}
	const T& operator()(const int i, const int j, const int k) const {
		return data[clamp(i, 0, rows - 1)
				+ clamp(j, 0, cols - 1) * (size_t) rows
				+ clamp(k, 0, slices - 1) * (size_t) rows * (size_t) cols];
	}
	const T& operator()(const size_t i, const size_t j, const size_t k) const {
		return data[clamp((int) i, 0, rows - 1)
				+ clamp((int) j, 0, cols - 1) * (size_t) rows
				+ clamp((int) k, 0, slices - 1) * (size_t) rows * (size_t) cols];
	}
	const T& operator()(const int3 ijk) const {
		return data[clamp(ijk.x, 0, rows - 1)
				+ clamp(ijk.y, 0, cols - 1) * (size_t) rows
				+ clamp(ijk.z, 0, slices - 1) * (size_t) rows * (size_t) cols];
	}
	inline float operator()(float x, float y, float z) {
		int i = static_cast<int>(std::floor(x));
		int j = static_cast<int>(std::floor(y));
		int k = static_cast<int>(std::floor(z));
		float rgb000 = float(operator()(i, j, k));
		float rgb100 = float(operator()(i + 1, j, k));
		float rgb110 = float(operator()(i + 1, j + 1, k));
		float rgb010 = float(operator()(i, j + 1, k));
		float rgb001 = float(operator()(i, j, k + 1));
		float rgb101 = float(operator()(i + 1, j, k + 1));
		float rgb111 = float(operator()(i + 1, j + 1, k + 1));
		float rgb011 = float(operator()(i, j + 1, k + 1));
		float dx = x - i;
		float dy = y - j;
		float dz = z - k;
		float lower = ((rgb000 * (1.0f - dx) + rgb100 * dx) * (1.0f - dy)
				+ (rgb010 * (1.0f - dx) + rgb110 * dx) * dy);
		float upper = ((rgb001 * (1.0f - dx) + rgb101 * dx) * (1.0f - dy)
				+ (rgb011 * (1.0f - dx) + rgb111 * dx) * dy);
		return (1.0f - dz) * lower + dz * upper;
	}
	inline float operator()(float x, float y, float z) const {
		int i = static_cast<int>(std::floor(x));
		int j = static_cast<int>(std::floor(y));
		int k = static_cast<int>(std::floor(z));
		float rgb000 = float(operator()(i, j, k));
		float rgb100 = float(operator()(i + 1, j, k));
		float rgb110 = float(operator()(i + 1, j + 1, k));
		float rgb010 = float(operator()(i, j + 1, k));
		float rgb001 = float(operator()(i, j, k + 1));
		float rgb101 = float(operator()(i + 1, j, k + 1));
		float rgb111 = float(operator()(i + 1, j + 1, k + 1));
		float rgb011 = float(operator()(i, j + 1, k + 1));
		float dx = x - i;
		float dy = y - j;
		float dz = z - k;
		float lower = ((rgb000 * (1.0f - dx) + rgb100 * dx) * (1.0f - dy)
				+ (rgb010 * (1.0f - dx) + rgb110 * dx) * dy);
		float upper = ((rgb001 * (1.0f - dx) + rgb101 * dx) * (1.0f - dy)
				+ (rgb011 * (1.0f - dx) + rgb111 * dx) * dy);
		return (1.0f - dz) * lower + dz * upper;

	}
	T& operator()(int i, int j, int k, int c) {
		return data[clamp((int) i, 0, rows - 1)
				+ clamp((int) j, 0, cols - 1) * rows
				+ clamp((int) k, 0, slices - 1) * rows * cols][c];
	}
	const T& operator()(int i, int j, int k, int c) const {
		return data[clamp((int) i, 0, rows - 1)
				+ clamp((int) j, 0, cols - 1) * rows
				+ clamp((int) k, 0, slices - 1) * rows * cols][c];
	}
	template<class F> void apply(F f) {
		size_t sz = size();
#pragma omp parallel for
		for (size_t offset = 0; offset < sz; offset++) {
			f(offset, data[offset]);
		}
	}
	T min(T valt = std::numeric_limits<T>::max()) const {
		T minVal(valt);
		for (T& val : data) {
			minVal = aly::minVec(val, minVal);
		}
		return minVal;
	}
	T max(T valt = std::numeric_limits<T>::min()) const {
		T maxVal(valt);
		for (T& val : data) {
			maxVal = aly::maxVec(val, maxVal);
		}
		return maxVal;
	}
};
template<class T> void Transform(DenseVol<T>& im1, DenseVol<T>& im2,
		const std::function<void(T&, T&)>& func) {
	if (im1.dimensions() != im2.dimensions())
		throw std::runtime_error(
				MakeString() << "DenseVol dimensions do not match. "
						<< im1.dimensions() << "!=" << im2.dimensions());
	size_t sz = im1.size();
#pragma omp parallel for
	for (size_t offset = 0; offset < sz; offset++) {
		func(im1.data[offset], im2.data[offset]);
	}
}
template<class T> void Transform(DenseVol<T>& im1, const DenseVol<T>& im2,
		const DenseVol<T>& im3, const DenseVol<T>& im4,
		const std::function<void(T&, const T&, const T&, const T&)>& func) {
	if (im1.dimensions() != im2.dimensions())
		throw std::runtime_error(
				MakeString() << "DenseVol dimensions do not match. "
						<< im1.dimensions() << "!=" << im2.dimensions());
	size_t sz = im1.size();
#pragma omp parallel for
	for (size_t offset = 0; offset < sz; offset++) {
		func(im1.data[offset], im2.data[offset], im3.data[offset],
				im4.data[offset]);
	}
}
template<class T> void Transform(DenseVol<T>& im1,
		const std::function<void(T&)>& func) {
	size_t sz = im1.size();
#pragma omp parallel for
	for (size_t offset = 0; offset < sz; offset++) {
		func(im1.data[offset]);
	}
}
template<class T> void Transform(DenseVol<T>& im1, const DenseVol<T>& im2,
		const std::function<void(T&, const T&)>& func) {
	if (im1.dimensions() != im2.dimensions())
		throw std::runtime_error(
				MakeString() << "DenseVol dimensions do not match. "
						<< im1.dimensions() << "!=" << im2.dimensions());
	size_t sz = im1.size();
#pragma omp parallel for
	for (size_t offset = 0; offset < sz; offset++) {
		func(im1.data[offset], im2.data[offset]);
	}
}
template<class T> void Transform(DenseVol<T>& im1, const DenseVol<T>& im2,
		const DenseVol<T>& im3,
		const std::function<void(T&, const T&, const T&)>& func) {
	if (im1.dimensions() != im2.dimensions())
		throw std::runtime_error(
				MakeString() << "DenseVol dimensions do not match. "
						<< im1.dimensions() << "!=" << im2.dimensions());
	size_t sz = im1.size();
#pragma omp parallel for
	for (size_t offset = 0; offset < sz; offset++) {
		func(im1.data[offset], im2.data[offset], im3.data[offset]);
	}
}
template<class T> void Transform(DenseVol<T>& im1, DenseVol<T>& im2,
		const std::function<void(int i, int j, int k, T& val1, T& val2)>& func) {
	if (im1.dimensions() != im2.dimensions())
		throw std::runtime_error(
				MakeString() << "DenseVol dimensions do not match. "
						<< im1.dimensions() << "!=" << im2.dimensions());
#pragma omp parallel for
	for (int k = 0; k < im1.slices; k++) {
		for (int j = 0; j < im1.cols; j++) {
			for (int i = 0; i < im1.rows; i++) {
				size_t offset = i + j * im1.rows + k * im1.rows * im1.cols;
				func(i, j, k, im1.data[offset], im2.data[offset]);
			}
		}
	}
}
template<class T> void Transform(DenseVol<T>& im1, DenseVol<T>& im2,
		const std::function<void(size_t offset, T& val1, T& val2)>& func) {
	if (im1.dimensions() != im2.dimensions())
		throw std::runtime_error(
				MakeString() << "DenseVol dimensions do not match. "
						<< im1.dimensions() << "!=" << im2.dimensions());
	size_t sz = im1.size();
#pragma omp parallel for
	for (size_t offset = 0; offset < sz; offset++) {
		func(offset, im1.data[offset], im2.data[offset]);
	}
}
template<class T> DenseVol<T> operator+(const T& scalar,
		const DenseVol<T>& img) {
	DenseVol<T> out(img.rows, img.cols, img.slices, img.position());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = scalar + val2;};
	Transform(out, img, f);
	return out;
}

template<class T> DenseVol<T> operator-(const T& scalar,
		const DenseVol<T>& img) {
	DenseVol<T> out(img.rows, img.cols, img.slices, img.position());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = scalar - val2;};
	Transform(out, img, f);
	return out;
}
template<class T> DenseVol<T> operator*(const T& scalar,
		const DenseVol<T>& img) {
	DenseVol<T> out(img.rows, img.cols, img.slices, img.position());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = scalar*val2;};
	Transform(out, img, f);
	return out;
}
template<class T> DenseVol<T> operator/(const T& scalar,
		const DenseVol<T>& img) {
	DenseVol<T> out(img.rows, img.cols, img.slices, img.position());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = scalar / val2;};
	Transform(out, img, f);
	return out;
}
template<class T> DenseVol<T> operator+(const DenseVol<T>& img,
		const T& scalar) {
	DenseVol<T> out(img.rows, img.cols, img.slices, img.position());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = val2 + scalar;};
	Transform(out, img, f);
	return out;
}
template<class T> DenseVol<T> operator-(const DenseVol<T>& img,
		const T& scalar) {
	DenseVol<T> out(img.rows, img.cols, img.slices, img.position());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = val2 - scalar;};
	Transform(out, img, f);
	return out;
}
template<class T> DenseVol<T> operator*(const DenseVol<T>& img,
		const T& scalar) {
	DenseVol<T> out(img.rows, img.cols, img.slices, img.position());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = val2*scalar;};
	Transform(out, img, f);
	return out;
}
template<class T> DenseVol<T> operator/(const DenseVol<T>& img,
		const T& scalar) {
	DenseVol<T> out(img.rows, img.cols, img.slices, img.position());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = val2 / scalar;};
	Transform(out, img, f);
	return out;
}
template<class T> DenseVol<T> operator-(const DenseVol<T>& img) {
	DenseVol<T> out(img.rows, img.cols, img.slices, img.position());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = -val2;};
	Transform(out, img, f);
	return out;
}
template<class T> DenseVol<T> operator+=(DenseVol<T>& out,
		const DenseVol<T>& img) {
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 += val2;};
	Transform(out, img, f);
	return out;
}
template<class T> DenseVol<T> operator-=(DenseVol<T>& out,
		const DenseVol<T>& img) {
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 -= val2;};
	Transform(out, img, f);
	return out;
}
template<class T> DenseVol<T> operator*=(DenseVol<T>& out,
		const DenseVol<T>& img) {
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 *= val2;};
	Transform(out, img, f);
	return out;
}
template<class T> DenseVol<T> operator/=(DenseVol<T>& out,
		const DenseVol<T>& img) {
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 /= val2;};
	Transform(out, img, f);
	return out;
}

template<class T> DenseVol<T> operator+=(DenseVol<T>& out, const T& scalar) {
	std::function<void(T&)> f = [=](T& val1) {val1 += scalar;};
	Transform(out, f);
	return out;
}
template<class T> DenseVol<T> operator-=(DenseVol<T>& out, const T& scalar) {
	std::function<void(T&)> f = [=](T& val1) {val1 -= scalar;};
	Transform(out, f);
	return out;
}
template<class T> DenseVol<T> operator*=(DenseVol<T>& out, const T& scalar) {
	std::function<void(T&)> f = [=](T& val1) {val1 *= scalar;};
	Transform(out, f);
	return out;
}
template<class T> DenseVol<T> operator/=(DenseVol<T>& out, const T& scalar) {
	std::function<void(T&)> f = [=](T& val1) {val1 /= scalar;};
	Transform(out, f);
	return out;
}

template<class T> DenseVol<T> operator+(const DenseVol<T>& img1,
		const DenseVol<T>& img2) {
	DenseVol<T> out(img1.rows, img1.cols, img1.slices);
	std::function<void(T&, const T&, const T&)> f =
			[=](T& val1, const T& val2, const T& val3) {val1 = val2 + val3;};
	Transform(out, img1, img2, f);
	return out;
}
template<class T> DenseVol<T> operator-(const DenseVol<T>& img1,
		const DenseVol<T>& img2) {
	DenseVol<T> out(img1.rows, img1.cols, img1.slices);
	std::function<void(T&, const T&, const T&)> f =
			[=](T& val1, const T& val2, const T& val3) {val1 = val2 - val3;};
	Transform(out, img1, img2, f);
	return out;
}
template<class T> DenseVol<T> operator*(const DenseVol<T>& img1,
		const DenseVol<T>& img2) {
	DenseVol<T> out(img1.rows, img1.cols, img1.slices);
	std::function<void(T&, const T&, const T&)> f =
			[=](T& val1, const T& val2, const T& val3) {val1 = val2*val3;};
	Transform(out, img1, img2, f);
	return out;
}
template<class T> DenseVol<T> operator/(const DenseVol<T>& img1,
		const DenseVol<T>& img2) {
	DenseVol<T> out(img1.rows, img1.cols, img1.slices);
	std::function<void(T&, const T&, const T&)> f =
			[=](T& val1, const T& val2, const T& val3) {val1 = val2 / val3;};
	Transform(out, img1, img2, f);
	return out;
}
typedef DenseMat<double> DenseMatrixDouble;
typedef SparseMat<double> SparseMatrixDouble;
typedef DenseVol<double> DenseVolDouble;

typedef Vec<double> VecDouble;
typedef Vec<double> Vec1d;
typedef Vec<double2> Vec2d;
typedef Vec<double3> Vec3d;
typedef Vec<double3> Vec4d;

typedef VecMap<double> VecMapDouble;
typedef VecMap<double> VecMap1d;
typedef VecMap<double2> VecMap2d;
typedef VecMap<double3> VecMap3d;
typedef VecMap<double3> VecMap4d;

typedef DenseMat<float> DenseMatrixFloat;
typedef SparseMat<float> SparseMatrixFloat;
typedef DenseVol<float> DenseVolFloat;

typedef Vec<float> VecFloat;
typedef Vec<float> Vec1f;
typedef Vec<float2> Vec2f;
typedef Vec<float3> Vec3f;
typedef Vec<float3> Vec4f;

typedef VecMap<float> VecMapFloat;
typedef VecMap<float> VecMap1f;
typedef VecMap<float2> VecMap2f;
typedef VecMap<float3> VecMap3f;
typedef VecMap<float3> VecMap4f;

}

#endif /* INCLUDE_CORE_ALLOYOPTIMIZATIONMATH_H_ */
