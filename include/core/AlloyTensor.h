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
 */
#ifndef ALLOYTENSOR_H_
#define ALLOYTENSOR_H_
#include "AlloyCommon.h"
#include "AlloyMath.h"
#include "AlloyOptimizationMath.h"
#include "sha2.h"
#include "AlloyFileUtil.h"
#include "AlloyImage.h"
#include "cereal/types/vector.hpp"
#include <vector>
#include <functional>
#include <fstream>
#include <random>
namespace aly {
template<class VecT, class T,int C, ImageType I> struct Tensor;
template<class VecT, class T,int C, ImageType I> void WriteImageToRawFile(const std::string& fileName, const Tensor<VecT, T, C, I>& img);
template<class VecT, class T,int C, ImageType I> bool ReadImageFromRawFile(const std::string& fileName, Tensor<VecT, T, C, I>& img);
	template<class VecT, class T,int C, ImageType I> struct Tensor {
	private:
		std::string hashCode;
		int x, y, z;
	public:
		Vec<VecT> vector;//Treat whole tensor as flat vector. Useful!
		std::vector<T,aligned_allocator<T,64>>& data;
		typedef VecT ValueType;
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
		uint64_t id;
		const int channels = C;
		const ImageType type = I;
		int3 position() const {
			return int3(x, y, z);
		}
		void setPosition(const int3& pos) {
			x = pos.x;
			y = pos.y;
			z = pos.z;
		}
		std::string updateHashCode(size_t MAX_SAMPLES = 0, HashMethod method =
			HashMethod::SHA256);
		std::string getHashCode() {
			return hashCode;
		}
		inline bool contains(const float3& pt){
			return (pt.x>=0&&pt.y>=0&&pt.z>=0&&pt.x<rows&&pt.y<cols&&pt.z<slices);
		}
		inline bool contains(const int3& pt){
			return (pt.x>=0&&pt.y>=0&&pt.z>=0&&pt.x<rows&&pt.y<cols&&pt.z<slices);
		}
		inline bool contains(int x,int y,int z){
			return (x>=0&&y>=0&&z>=0&&x<rows&&y<cols&&z<slices);
		}
		inline bool contains(float x,float y,float z){
			return (x>=0&&y>=0&&z>=0&&x<rows&&y<cols&&z<slices);
		}
		template<class Archive> void serialize(Archive & archive) {
			archive(cereal::make_nvp(MakeString() << type << channels, id),CEREAL_NVP(id),CEREAL_NVP(rows), CEREAL_NVP(cols), CEREAL_NVP(slices),
				CEREAL_NVP(x), CEREAL_NVP(y), CEREAL_NVP(z),
				CEREAL_NVP(hashCode),CEREAL_NVP(vector));
		}
		inline void writeToXML(const std::string& fileName) const {
			WriteImageToRawFile(fileName, *this);
		}
		inline bool readFromXML(const std::string& fileName) {
			return ReadImageFromRawFile(fileName,*this);
		}
		void set(const VecT& val) {
			data.assign(data.size(), val);
		}
		void set(const std::vector<VecT>& val) {
			data = val;
		}
		void set(VecT* val) {
			if (val == nullptr)return;
			std::memcpy(data.data(),val,data.size()*sizeof(VecT));
		}
		void set(const Tensor<VecT, T, C, I>& other) {
			resize(other.rows, other.cols, other.slices);
			id = other.id;
			x = other.x;
			y = other.y;
			z = other.z;
			set(other.data());
		}
		Tensor(int r, int c, int s, int x = 0, int y = 0, int z = 0,
			uint64_t id = 0) :
				 x(x), y(y), z(z),data(vector.data),rows(r), cols(c), slices(s), id(id){
		data.resize((size_t)r *(size_t) c * (size_t)s);
		}
		Tensor(int r, int c, int s, int3 pos,
			uint64_t id = 0) :
			x(pos.x), y(pos.y), z(pos.z),data(vector.data), rows(r), cols(c), slices(s), id(id){
			data.resize((size_t)r * (size_t)c * (size_t)s);

		}
		Tensor(int3 dims, int3 pos=int3(0),
			uint64_t id = 0) :
			x(pos.x), y(pos.y), z(pos.z),data(vector.data), rows(dims.x), cols(dims.y), slices(dims.z), id(id){
			data.resize((size_t)dims.x*(size_t)dims.y*(size_t)dims.z);

		}
		Tensor(VecT* ptr, int r, int c, int s, int x = 0, int y = 0, int z = 0,
			uint64_t id = 0) :
			Tensor(r, c, s, x, y, z, id) {
			set(ptr);
		}
		Tensor(const std::vector<T>& ref, int r, int c, int s, int x = 0, int y =
			0, int z = 0, uint64_t id = 0) :
			x(x), y(y), z(z), data(vector.data),data(ref) , rows(r), cols(c), slices(s), id(id){
		}
		Tensor() :
			x(0), y(0), z(0), data(vector.data),rows(0), cols(0), slices(0), id(0) {
		}
		Tensor(const Tensor<VecT, T, C, I>& img) :
			Tensor(img.rows, img.cols, img.slices, img.position(), img.id) {
			set(img.data);
		}
		Tensor<VecT, T, C, I>& operator=(const Tensor<VecT, T, C, I>& rhs) {
			if (this == &rhs)return *this;
			this->resize(rhs.rows, rhs.cols, rhs.slices);
			this->setPosition(rhs.position());
			this->id = rhs.id;
			this->set(rhs.data);
			return *this;
		}
		int3 dimensions() const {
			return int3(rows, cols, slices);
		}

		size_t size() const {
			return data.size();
		}
		size_t typeSize() const {
			return sizeof(T);
		}
		void resize(int r, int c, int s) {
			data.resize((size_t)r * (size_t)c * (size_t)s);
			data.shrink_to_fit();
			rows = r;
			cols = c;
			slices = s;
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
			return (T*)&(data.data()[0]);
		}
		const T* ptr()  const {
			if (data.size() == 0)
				return nullptr;
			return (T*)&(data.data()[0]);
		}
		VecT* vecPtr() {
			if (data.size() == 0)
				return nullptr;
			return (VecT*)data.data();
		}
		const VecT* vecPtr() const {
			if (data.size() == 0)
				return nullptr;
			return (const VecT*)data.data();
		}
		void setZero() {
			data.assign(data.size(), (T)0);
		}
		const VecT& operator[](const size_t i) const {
			return data[i];
		}
		VecT& operator[](const size_t i) {
			return data[i];
		}
		VecT& operator()(const int i, const int j, const int k) {
			return data[clamp(i, 0, rows - 1) + clamp(j, 0, cols - 1) * rows
				+ clamp(k, 0, slices - 1) * rows * cols];
		}
		VecT& operator()(const size_t i, const size_t j, const size_t k) {
			return data[clamp((int)i, 0, rows - 1) + clamp((int)j, 0, cols - 1) * rows
				+ clamp((int)k, 0, slices - 1) * rows * cols];
		}
		T& operator()(const size_t i, const size_t j, const size_t k,const int c) {
			return *(&data[clamp((int)i, 0, rows - 1) + clamp((int)j, 0, cols - 1) * rows
				+ clamp((int)k, 0, slices - 1) * rows * cols]+c);
		}
		VecT& operator()(const int3 ijk) {
			return data[clamp(ijk.x, 0, rows - 1) + clamp(ijk.y, 0, cols - 1) * rows
				+ clamp(ijk.z, 0, slices - 1) * rows * cols];
		}
		const VecT& operator()(const int i, const int j, const int k) const {
			return data[clamp(i, 0, rows - 1) + clamp(j, 0, cols - 1) * rows
				+ clamp(k, 0, slices - 1) * rows * cols];
		}
		const VecT& operator()(const size_t i, const size_t j, const size_t k) const {
			return data[clamp((int)i, 0, rows - 1) + clamp((int)j, 0, cols - 1) * rows
				+ clamp((int)k, 0, slices - 1) * rows * cols];
		}
		const T& operator()(const size_t i, const size_t j, const size_t k,const int c) const {
			return *(&data[clamp((int)i, 0, rows - 1) + clamp((int)j, 0, cols - 1) * rows + clamp((int)k, 0, slices - 1) * rows * cols]+c);
		}
		const VecT& operator()(const int3 ijk) const {
			return data[clamp(ijk.x, 0, rows - 1) + clamp(ijk.y, 0, cols - 1) * rows
				+ clamp(ijk.z, 0, slices - 1) * rows * cols];
		}
		inline VecT operator()(float x,float y,int k) const {
			int i = static_cast<int>(std::floor(x));
			int j = static_cast<int>(std::floor(y));
			VecT rgb000 = (VecT)(operator()(i, j, k));
			VecT rgb100 = (VecT)(operator()(i + 1, j, k));
			VecT rgb110 = (VecT)(operator()(i + 1, j + 1, k));
			VecT rgb010 = (VecT)(operator()(i, j + 1, k));
			VecT dx = x - i;
			VecT dy = y - j;
			VecT dz = z - k;
			VecT lower = ((rgb000 * (1.0f - dx) + rgb100 * dx) * (1.0f - dy)+ (rgb010 * (1.0f - dx) + rgb110 * dx) * dy);
			return lower;
		}

		inline Vec<VecT> operator()(int i,int j) const {
			Vec<VecT> out(slices);
			for(int k=0;k<slices;k++){
				out[k]=(VecT)(operator()(i, j, k));
			}
			return out;
		}
		inline Vec<VecT> operator()(float x,float y) const {
			Vec<VecT> out(slices);
			int i = static_cast<int>(std::floor(x));
			int j = static_cast<int>(std::floor(y));
			for(int k=0;k<slices;k++){
				VecT rgb000 = (VecT)(operator()(i, j, k));
				VecT rgb100 = (VecT)(operator()(i + 1, j, k));
				VecT rgb110 = (VecT)(operator()(i + 1, j + 1, k));
				VecT rgb010 = (VecT)(operator()(i, j + 1, k));
				VecT dx = x - i;
				VecT dy = y - j;
				VecT dz = z - k;
				out[k]= ((rgb000 * (1.0f - dx) + rgb100 * dx) * (1.0f - dy)
					+ (rgb010 * (1.0f - dx) + rgb110 * dx) * dy);
				}
			return out;
		}
		inline VecT operator()(float x,float y,float z) const {
			int i = static_cast<int>(std::floor(x));
			int j = static_cast<int>(std::floor(y));
			int k = static_cast<int>(std::floor(z));
			VecT rgb000 = (VecT)(operator()(i, j, k));
			VecT rgb100 = (VecT)(operator()(i + 1, j, k));
			VecT rgb110 = (VecT)(operator()(i + 1, j + 1, k));
			VecT rgb010 = (VecT)(operator()(i, j + 1, k));
			VecT rgb001 = (VecT)(operator()(i, j, k + 1));
			VecT rgb101 = (VecT)(operator()(i + 1, j, k + 1));
			VecT rgb111 = (VecT)(operator()(i + 1, j + 1, k + 1));
			VecT rgb011 = (VecT)(operator()(i, j + 1, k + 1));
			VecT dx = x - i;
			VecT dy = y - j;
			VecT dz = z - k;
			VecT lower = ((rgb000 * (1.0f - dx) + rgb100 * dx) * (1.0f - dy)
				+ (rgb010 * (1.0f - dx) + rgb110 * dx) * dy);
			VecT upper = ((rgb001 * (1.0f - dx) + rgb101 * dx) * (1.0f - dy)
				+ (rgb011 * (1.0f - dx) + rgb111 * dx) * dy);
			return (1.0f - dz) * lower + dz * upper;

		}
		template<class F> void apply(F f) {
			size_t sz = size();
#pragma omp parallel for
			for (int offset = 0; offset < (int)sz; offset++) {
				f(offset, data[offset]);
			}
		}
	}
	;
	template<class VecT, class T,int C, ImageType I> std::string Tensor<VecT, T, C, I>::updateHashCode(
		size_t MAX_SAMPLES, HashMethod method) {
		if (MAX_SAMPLES == 0) {
			hashCode = HashCode(data, method);
		}
		else {
			const size_t seed = 8743128921;
			std::mt19937 mt(seed);
			std::uniform_int_distribution<int> wSampler(0, rows - 1);
			std::uniform_int_distribution<int> hSampler(0, cols - 1);
			std::uniform_int_distribution<int> dSampler(0, slices - 1);
			std::vector<VecT> sample(MAX_SAMPLES);
			for (int i = 0; i < MAX_SAMPLES; i++) {
				sample[i] = this->operator()(wSampler(mt), hSampler(mt),
					dSampler(mt));
			}
			hashCode = HashCode(sample, method);
		}
		return hashCode;
	}
	template<class VecT, class T, int C, ImageType I> void Transform(Tensor<VecT, T, C, I>& im1,
		Tensor<VecT, T, C, I>& im2,
		const std::function<void(VecT&, VecT&)>& func) {
		if (im1.dimensions() != im2.dimensions())
			throw std::runtime_error(
				MakeString() << "Tensor dimensions do not match. "
				<< im1.dimensions() << "!=" << im2.dimensions());
		size_t sz = im1.size();
#pragma omp parallel for
		for (int offset = 0; offset < (int)sz; offset++) {
			func(im1.data[offset], im2.data[offset]);
		}
	}
	template<class VecT, class T, int C, ImageType I> void Transform(Tensor<VecT, T, C, I>& im1,
		const Tensor<VecT, T, C, I>& im2, const Tensor<VecT, T, C, I>& im3,
		const Tensor<VecT, T, C, I>& im4,
		const std::function<
		void(VecT&, const VecT&, const VecT&,
			const VecT&)>& func) {
		if (im1.dimensions() != im2.dimensions())
			throw std::runtime_error(
				MakeString() << "Tensor dimensions do not match. "
				<< im1.dimensions() << "!=" << im2.dimensions());
		size_t sz = im1.size();
#pragma omp parallel for
		for (int offset = 0; offset < (int)sz; offset++) {
			func(im1.data[offset], im2.data[offset], im3.data[offset],
				im4.data[offset]);
		}
	}
	template<class VecT, class T, int C, ImageType I> void Transform(Tensor<VecT, T, C, I>& im1,
		const std::function<void(VecT&)>& func) {
		size_t sz = im1.size();
#pragma omp parallel for
		for (int offset = 0; offset < (int)sz; offset++) {
			func(im1.data[offset]);
		}
	}
	template<class VecT, class T, int C, ImageType I> void Transform(Tensor<VecT, T, C, I>& im1,
		const Tensor<VecT, T, C, I>& im2,
		const std::function<void(VecT&, const VecT&)>& func) {
		if (im1.dimensions() != im2.dimensions())
			throw std::runtime_error(
				MakeString() << "Tensor dimensions do not match. "
				<< im1.dimensions() << "!=" << im2.dimensions());
		size_t sz = im1.size();
#pragma omp parallel for
		for (int offset = 0; offset < (int)sz; offset++) {
			func(im1.data[offset], im2.data[offset]);
		}
	}
	template<class VecT, class T, int C, ImageType I> void Transform(Tensor<VecT, T, C, I>& im1,
		const Tensor<VecT, T, C, I>& im2, const Tensor<VecT, T, C, I>& im3,
		const std::function<void(VecT&, const VecT&, const VecT&)>& func) {
		if (im1.dimensions() != im2.dimensions())
			throw std::runtime_error(
				MakeString() << "Tensor dimensions do not match. "
				<< im1.dimensions() << "!=" << im2.dimensions());
		size_t sz = im1.size();
#pragma omp parallel for
		for (int offset = 0; offset < (int)sz; offset++) {
			func(im1.data[offset], im2.data[offset], im3.data[offset]);
		}
	}
	template<class VecT, class T, int C, ImageType I> void Transform(Tensor<VecT, T, C, I>& im1,
		Tensor<VecT, T, C, I>& im2,
		const std::function<
		void(int i, int j, int k, VecT& val1, VecT& val2)>& func) {
		if (im1.dimensions() != im2.dimensions())
			throw std::runtime_error(
				MakeString() << "Tensor dimensions do not match. "
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
	template<class VecT, class T, int C, ImageType I> void Transform(Tensor<VecT, T, C, I>& im1,
		Tensor<VecT, T, C, I>& im2,
		const std::function<
		void(size_t offset, VecT& val1, VecT& val2)>& func) {
		if (im1.dimensions() != im2.dimensions())
			throw std::runtime_error(
				MakeString() << "Tensor dimensions do not match. "
				<< im1.dimensions() << "!=" << im2.dimensions());
		size_t sz = im1.size();
#pragma omp parallel for
		for (int offset = 0; offset < (int)sz; offset++) {
			func(offset, im1.data[offset], im2.data[offset]);
		}
	}
	template<class VecT, class T, class L, class R, int C, ImageType I> std::basic_ostream<L, R> & operator <<(
		std::basic_ostream<L, R> & ss, const Tensor<VecT, T, C, I> & A) {
		ss << "Tensor (" << A.getTypeName() << "): " << A.id << " Position: ("
			<< A.x << "," << A.y << ") Dimensions: [" << A.rows << "," << A.cols
			<< "]\n";
		return ss;
	}
	template<class VecT, class T, int C, ImageType I> Tensor<VecT, T, C, I> operator+(
		const VecT& scalar, const Tensor<VecT, T, C, I>& img) {
		Tensor<VecT, T, C, I> out(img.rows, img.cols, img.slices, img.position());
		std::function<void(VecT&, const VecT&)> f =
			[=](VecT& val1, const VecT& val2) {val1 = scalar + val2;};
		Transform(out, img, f);
		return out;
	}

	template<class VecT, class T, int C, ImageType I> Tensor<VecT, T, C, I> operator-(
		const VecT& scalar, const Tensor<VecT, T, C, I>& img) {
		Tensor<VecT, T, C, I> out(img.rows, img.cols, img.slices, img.position());
		std::function<void(VecT&, const VecT&)> f =
			[=](VecT& val1, const VecT& val2) {val1 = scalar - val2;};
		Transform(out, img, f);
		return out;
	}
	template<class VecT, class T, int C, ImageType I> Tensor<VecT, T, C, I> operator*(
		const VecT& scalar, const Tensor<VecT, T, C, I>& img) {
		Tensor<VecT, T, C, I> out(img.rows, img.cols, img.slices, img.position());
		std::function<void(VecT&, const VecT&)> f =
			[=](VecT& val1, const VecT& val2) {val1 = scalar*val2;};
		Transform(out, img, f);
		return out;
	}
	template<class VecT, class T, int C, ImageType I> Tensor<VecT, T, C, I> operator/(
		const VecT& scalar, const Tensor<VecT, T, C, I>& img) {
		Tensor<VecT, T, C, I> out(img.rows, img.cols, img.slices, img.position());
		std::function<void(VecT&, const VecT&)> f =
			[=](VecT& val1, const VecT& val2) {val1 = scalar / val2;};
		Transform(out, img, f);
		return out;
	}
	template<class VecT, class T, int C, ImageType I> Tensor<VecT, T, C, I> operator+(
		const Tensor<VecT, T, C, I>& img, const VecT& scalar) {
		Tensor<VecT, T, C, I> out(img.rows, img.cols, img.slices, img.position());
		std::function<void(VecT&, const VecT&)> f =
			[=](VecT& val1, const VecT& val2) {val1 = val2 + scalar;};
		Transform(out, img, f);
		return out;
	}
	template<class VecT, class T, int C, ImageType I> Tensor<VecT, T, C, I> operator-(
		const Tensor<VecT, T, C, I>& img, const VecT& scalar) {
		Tensor<VecT, T, C, I> out(img.rows, img.cols, img.slices, img.position());
		std::function<void(VecT&, const VecT&)> f =
			[=](VecT& val1, const VecT& val2) {val1 = val2 - scalar;};
		Transform(out, img, f);
		return out;
	}
	template<class VecT, class T, int C, ImageType I> Tensor<VecT, T, C, I> operator*(
		const Tensor<VecT, T, C, I>& img, const VecT& scalar) {
		Tensor<VecT, T, C, I> out(img.rows, img.cols, img.slices, img.position());
		std::function<void(VecT&, const VecT&)> f =
			[=](VecT& val1, const VecT& val2) {val1 = val2*scalar;};
		Transform(out, img, f);
		return out;
	}
	template<class VecT, class T, int C, ImageType I> Tensor<VecT, T, C, I> operator/(
		const Tensor<VecT, T, C, I>& img, const VecT& scalar) {
		Tensor<VecT, T, C, I> out(img.rows, img.cols, img.slices, img.position());
		std::function<void(VecT&, const VecT&)> f =
			[=](VecT& val1, const VecT& val2) {val1 = val2 / scalar;};
		Transform(out, img, f);
		return out;
	}
	template<class VecT, class T, int C, ImageType I> Tensor<VecT, T, C, I> operator-(
		const Tensor<VecT, T, C, I>& img) {
		Tensor<VecT, T, C, I> out(img.rows, img.cols, img.slices, img.position());
		std::function<void(VecT&, const VecT&)> f =
			[=](VecT& val1, const VecT& val2) {val1 = -val2;};
		Transform(out, img, f);
		return out;
	}
	template<class VecT, class T, int C, ImageType I> Tensor<VecT, T, C, I> operator+=(
		Tensor<VecT, T, C, I>& out, const Tensor<VecT, T, C, I>& img) {
		std::function<void(VecT&, const VecT&)> f =
			[=](VecT& val1, const VecT& val2) {val1 += val2;};
		Transform(out, img, f);
		return out;
	}
	template<class VecT, class T, int C, ImageType I> Tensor<VecT, T, C, I> operator-=(
		Tensor<VecT, T, C, I>& out, const Tensor<VecT, T, C, I>& img) {
		std::function<void(VecT&, const VecT&)> f =
			[=](VecT& val1, const VecT& val2) {val1 -= val2;};
		Transform(out, img, f);
		return out;
	}
	template<class VecT, class T, int C, ImageType I> Tensor<VecT, T, C, I> operator*=(
		Tensor<VecT, T, C, I>& out, const Tensor<VecT, T, C, I>& img) {
		std::function<void(VecT&, const VecT&)> f =
			[=](VecT& val1, const VecT& val2) {val1 *= val2;};
		Transform(out, img, f);
		return out;
	}
	template<class VecT, class T, int C, ImageType I> Tensor<VecT, T, C, I> operator/=(
		Tensor<VecT, T, C, I>& out, const Tensor<VecT, T, C, I>& img) {
		std::function<void(VecT&, const VecT&)> f =
			[=](VecT& val1, const VecT& val2) {val1 /= val2;};
		Transform(out, img, f);
		return out;
	}

	template<class VecT, class T, int C, ImageType I> Tensor<VecT, T, C, I> operator+=(
		Tensor<VecT, T, C, I>& out, const VecT& scalar) {
		std::function<void(VecT&)> f = [=](VecT& val1) {val1 += scalar;};
		Transform(out, f);
		return out;
	}
	template<class VecT, class T, int C, ImageType I> Tensor<VecT, T, C, I> operator-=(
		Tensor<VecT, T, C, I>& out, const VecT& scalar) {
		std::function<void(VecT&)> f = [=](VecT& val1) {val1 -= scalar;};
		Transform(out, f);
		return out;
	}
	template<class VecT, class T, int C, ImageType I> Tensor<VecT, T, C, I> operator*=(
		Tensor<VecT, T, C, I>& out, const VecT& scalar) {
		std::function<void(VecT&)> f = [=](VecT& val1) {val1 *= scalar;};
		Transform(out, f);
		return out;
	}
	template<class VecT, class T, int C, ImageType I> Tensor<VecT, T, C, I> operator/=(
		Tensor<VecT, T, C, I>& out, const VecT& scalar) {
		std::function<void(VecT&)> f = [=](VecT& val1) {val1 /= scalar;};
		Transform(out, f);
		return out;
	}

	template<class VecT, class T, int C, ImageType I> Tensor<VecT, T, C, I> operator+(
		const Tensor<VecT, T, C, I>& img1, const Tensor<VecT, T, C, I>& img2) {
		Tensor<VecT, T, C, I> out(img1.rows, img1.cols, img1.slices);
		std::function<void(VecT&, const VecT&, const VecT&)> f =
			[=](VecT& val1, const VecT& val2, const VecT& val3) {val1 = val2 + val3;};
		Transform(out, img1, img2, f);
		return out;
	}
	template<class VecT, class T, int C, ImageType I> Tensor<VecT, T, C, I> operator-(
		const Tensor<VecT, T, C, I>& img1, const Tensor<VecT, T, C, I>& img2) {
		Tensor<VecT, T, C, I> out(img1.rows, img1.cols, img1.slices);
		std::function<void(VecT&, const VecT&, const VecT&)> f =
			[=](VecT& val1, const VecT& val2, const VecT& val3) {val1 = val2 - val3;};
		Transform(out, img1, img2, f);
		return out;
	}
	template<class VecT, class T, int C, ImageType I> Tensor<VecT, T, C, I> operator*(
		const Tensor<VecT, T, C, I>& img1, const Tensor<VecT, T, C, I>& img2) {
		Tensor<VecT, T, C, I> out(img1.rows, img1.cols, img1.slices);
		std::function<void(VecT&, const VecT&, const VecT&)> f =
			[=](VecT& val1, const VecT& val2, const VecT& val3) {val1 = val2*val3;};
		Transform(out, img1, img2, f);
		return out;
	}
	template<class VecT, class T, int C, ImageType I> Tensor<VecT, T, C, I> operator/(
		const Tensor<VecT, T, C, I>& img1, const Tensor<VecT, T, C, I>& img2) {
		Tensor<VecT, T, C, I> out(img1.rows, img1.cols, img1.slices);
		std::function<void(VecT&, const VecT&, const VecT&)> f =
			[=](VecT& val1, const VecT& val2, const VecT& val3) {val1 = val2 / val3;};
		Transform(out, img1, img2, f);
		return out;
	}
	template<class VecT, class T, int C, ImageType I> bool ReadImageFromRawFile(
				const std::string& file, Tensor<VecT, T, C, I>& img) {
			std::string xmlFile = GetFileWithoutExtension(file)+".xml";
			std::string rawFile = GetFileWithoutExtension(file)+".raw";
			MipavHeader header;
			img.clear();
			if(!ReadMipavHeaderFromFile(xmlFile,header))return false;
			if(header.dimensions==4&&header.extents[3]!=C){
				throw std::runtime_error(MakeString() << "Channels " <<header.dimensions<<"/"<<C<< " do not match.");
			}
			if(header.dimensions==3&&C!=1){
				throw std::runtime_error(MakeString() << "Channels " <<header.dimensions<<"/"<<C<< " do not match.");
			}
			std::string typeName = "";
			switch (img.type) {
			case ImageType::BYTE:
				typeName = "byte";
				break;
			case ImageType::UBYTE:
				typeName = "unsigned byte";
				break;
			case ImageType::SHORT:
				typeName = "short";
				break;
			case ImageType::USHORT:
				typeName = "unsigned short";
				break;
			case ImageType::INT:
				typeName = "integer";
				break;
			case ImageType::UINT:
				typeName = "unsigned integer";
				break;
			case ImageType::FLOAT:
				typeName = "float";
				break;
			case ImageType::DOUBLE:
				typeName = "double";
				break;
			case ImageType::UNKNOWN:
				typeName = "unknown";
				break;
			default:
				break;
			}
			if(ToLower(header.dataType)!=typeName){
				throw std::runtime_error(MakeString() << "Type " <<header.dataType<<"/"<<typeName<< " do not match.");
			}
			img.resize(header.extents[0],header.extents[1],header.extents[2]);
			FILE* f = fopen(rawFile.c_str(), "rb");
			if (f == NULL) {
				throw std::runtime_error(MakeString() << "Could not open " <<rawFile<< " for reading.");
			}
			for (int c = 0; c < img.channels; c++) {
				for (int k = 0; k < img.slices; k++) {
					for (int j = 0; j < img.cols; j++) {
						for (int i = 0; i < img.rows; i++) {
							T* ptr=&img(i,j,k);
							fread(ptr+c, sizeof(T), 1, f);
						}
					}
				}
			}
			fclose(f);
		}
		template<class VecT, class T, int C, ImageType I> void WriteImageToRawFile(
			const std::string& file, const Tensor<VecT, T, C, I>& img) {
			std::ostringstream vstr;
			std::string fileName = GetFileWithoutExtension(file);
			vstr << fileName << ".raw";
			FILE* f = fopen(vstr.str().c_str(), "wb");
			if (f == NULL) {
				throw std::runtime_error(
					MakeString() << "Could not open " << vstr.str().c_str()
					<< " for writing.");
			}
			for (int c = 0; c < img.channels; c++) {
				for (int k = 0; k < img.slices; k++) {
					for (int j = 0; j < img.cols; j++) {
						for (int i = 0; i < img.rows; i++) {
							T* ptr=&img(i,j,k);
							fwrite(ptr+c, sizeof(T), 1, f);
						}
					}
				}
			}
			fclose(f);
			std::string typeName = "";
			switch (img.type) {
			case ImageType::BYTE:
				typeName = "Byte";
				break;
			case ImageType::UBYTE:
				typeName = "Unsigned Byte";
				break;
			case ImageType::SHORT:
				typeName = "Short";
				break;
			case ImageType::USHORT:
				typeName = "Unsigned Short";
				break;
			case ImageType::INT:
				typeName = "Integer";
				break;
			case ImageType::UINT:
				typeName = "Unsigned Integer";
				break;
			case ImageType::FLOAT:
				typeName = "Float";
				break;
			case ImageType::DOUBLE:
				typeName = "Double";
				break;
			case ImageType::UNKNOWN:
				typeName = "Unknown";
				break;
			}
			//std::cout << vstr.str() << std::endl;
			std::stringstream sstr;
			sstr << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
			sstr << "<!-- MIPAV header file -->\n";
			if (img.channels > 1) {
				sstr << "<image xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" nDimensions=\"4\">\n";
			}
			else {
				sstr << "<image xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" nDimensions=\"3\">\n";
			}
			sstr << "	<Dataset-attributes>\n";
			sstr << "		<Image-offset>0</Image-offset>\n";
			sstr << "		<Data-type>" << typeName << "</Data-type>\n";
			sstr << "		<Endianess>Little</Endianess>\n";
			sstr << "		<Extents>" << img.rows << "</Extents>\n";
			sstr << "		<Extents>" << img.cols << "</Extents>\n";
			sstr << "		<Extents>" << img.slices << "</Extents>\n";
			if (img.channels > 1) {
				sstr << "		<Extents>" << img.channels << "</Extents>\n";
			}
			sstr << "		<Resolutions>\n";
			sstr << "			<Resolution>1.0</Resolution>\n";
			sstr << "			<Resolution>1.0</Resolution>\n";
			sstr << "			<Resolution>1.0</Resolution>\n";
			sstr << "		</Resolutions>\n";
			sstr << "		<Slice-spacing>1.0</Slice-spacing>\n";
			sstr << "		<Slice-thickness>0.0</Slice-thickness>\n";
			sstr << "		<Units>Millimeters</Units>\n";
			sstr << "		<Units>Millimeters</Units>\n";
			sstr << "		<Units>Millimeters</Units>\n";
			sstr << "		<Compression>none</Compression>\n";
			sstr << "		<Orientation>Unknown</Orientation>\n";
			sstr << "		<Subject-axis-orientation>Unknown</Subject-axis-orientation>\n";
			sstr << "		<Subject-axis-orientation>Unknown</Subject-axis-orientation>\n";
			sstr << "		<Subject-axis-orientation>Unknown</Subject-axis-orientation>\n";
			sstr << "		<Origin>0.0</Origin>\n";
			sstr << "		<Origin>0.0</Origin>\n";
			sstr << "		<Origin>0.0</Origin>\n";
			sstr << "		<Modality>Unknown Modality</Modality>\n";
			sstr << "	</Dataset-attributes>\n";
			sstr << "</image>\n";
			std::ofstream myfile;
			std::stringstream xmlFile;
			xmlFile << fileName << ".xml";
			myfile.open(xmlFile.str().c_str(), std::ios_base::out);
			if (!myfile.is_open()) {
				throw std::runtime_error(
					MakeString() << "Could not open " << xmlFile.str()
					<< " for writing.");
			}
			myfile << sstr.str();
			myfile.close();
		}
	template<class VecT, class T,int C, ImageType I> void WriteTensorToFile(
		const std::string& file, const Tensor<VecT, T, C, I>& img) {
		WriteImageToRawFile(file,img);
	}
	template<class VecT, class T,int C, ImageType I> bool ReadTensorFromFile(
		const std::string& file, Tensor<VecT, T, C, I>& img) {
		return ReadImageFromRawFile(file,img);
	}

	typedef Tensor<float,float,1,ImageType::FLOAT> Tensor1f;
	typedef Tensor<double,double,1,ImageType::DOUBLE> Tensor1d;
	typedef Tensor<int32_t,int32_t,1,ImageType::INT> Tensor1i;
	typedef Tensor<uint32_t,uint32_t,1,ImageType::UINT> Tensor1ui;
	typedef Tensor<int16_t,int16_t,1,ImageType::USHORT> Tensor1s;
	typedef Tensor<uint16_t,uint16_t,1,ImageType::SHORT> Tensor1us;
	typedef Tensor<uint8_t,uint8_t,1,ImageType::UBYTE> Tensor1ub;
	typedef Tensor<int8_t,int8_t,1,ImageType::BYTE> Tensor1b;

	typedef Tensor<vec<float,2>,float,2,ImageType::FLOAT> Tensor2f;
	typedef Tensor<vec<double,2>,double,2,ImageType::DOUBLE> Tensor2d;
	typedef Tensor<vec<int32_t,2>,int32_t,2,ImageType::INT> Tensor2i;
	typedef Tensor<vec<uint32_t,2>,uint32_t,2,ImageType::UINT> Tensor2ui;
	typedef Tensor<vec<int16_t,2>,int16_t,2,ImageType::USHORT> Tensor2s;
	typedef Tensor<vec<uint16_t,2>,uint16_t,2,ImageType::SHORT> Tensor2us;
	typedef Tensor<vec<uint8_t,2>,uint8_t,2,ImageType::UBYTE> Tensor2ub;
	typedef Tensor<vec<int8_t,2>,int8_t,2,ImageType::BYTE> Tensor2b;

	typedef Tensor<vec<float,3>,float,3,ImageType::FLOAT> Tensor3f;
	typedef Tensor<vec<double,3>,double,3,ImageType::DOUBLE> Tensor3d;
	typedef Tensor<vec<int32_t,3>,int32_t,3,ImageType::INT> Tensor3i;
	typedef Tensor<vec<uint32_t,3>,uint32_t,3,ImageType::UINT> Tensor3ui;
	typedef Tensor<vec<int16_t,3>,int16_t,3,ImageType::USHORT> Tensor3s;
	typedef Tensor<vec<uint16_t,3>,uint16_t,3,ImageType::SHORT> Tensor3us;
	typedef Tensor<vec<uint8_t,3>,uint8_t,3,ImageType::UBYTE> Tensor3ub;
	typedef Tensor<vec<int8_t,3>,int8_t,3,ImageType::BYTE> Tensor3b;

	typedef Tensor<vec<float,4>,float,4,ImageType::FLOAT> Tensor4f;
	typedef Tensor<vec<double,4>,double,4,ImageType::DOUBLE> Tensor4d;
	typedef Tensor<vec<int32_t,4>,int32_t,4,ImageType::INT> Tensor4i;
	typedef Tensor<vec<uint32_t,4>,uint32_t,4,ImageType::UINT> Tensor4ui;
	typedef Tensor<vec<int16_t,4>,int16_t,4,ImageType::USHORT> Tensor4s;
	typedef Tensor<vec<uint16_t,4>,uint16_t,4,ImageType::SHORT> Tensor4us;
	typedef Tensor<vec<uint8_t,4>,uint8_t,4,ImageType::UBYTE> Tensor4ub;
	typedef Tensor<vec<int8_t,4>,int8_t,4,ImageType::BYTE> Tensor4b;
}
;

#endif
