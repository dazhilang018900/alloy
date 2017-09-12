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
#ifndef ALLOYTENSOR3_H_
#define ALLOYTENSOR3_H_
#include "AlloyCommon.h"
#include "AlloyMath.h"
#include "AlloyOptimizationMath.h"
#include "sha2.h"
#include "AlloyFileUtil.h"
#include "AlloyImage.h"
#include "AlloyTensor2.h"
#include "cereal/types/vector.hpp"
#include <vector>
#include <functional>
#include <fstream>
#include <random>
namespace aly {
template<class T, ImageType I> struct Tensor3;
template<class T, ImageType I> void WriteImageToRawFile(const std::string& fileName, const Tensor3<T, I>& img);
template<class T, ImageType I> bool ReadImageFromRawFile(const std::string& fileName, Tensor3<T, I>& img);
template<class T, ImageType I> struct Tensor3 {
	public:
		int rows;
		int cols;
		int slices;
		int channels;
		uint64_t id;
		std::vector<T, aligned_allocator<T,64> > data;
		typedef Vec<T> ValueType;
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
		const ImageType type = I;
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
			archive(CEREAL_NVP(id),CEREAL_NVP(rows), CEREAL_NVP(cols), CEREAL_NVP(slices),CEREAL_NVP(channels),CEREAL_NVP(data));
		}

		Tensor3(int r, int c, int s,int ch, uint64_t id = 0) :rows(r), cols(c), slices(s),channels(ch), id(id){
		data.resize((size_t)r *(size_t) c * (size_t)s);
		}
		Tensor3(const std::vector<T>& ref, int r, int c, int s, int ch, int x = 0, int y =
			0, int z = 0, uint64_t id = 0) : data(ref) , rows(r), cols(c), slices(s),channels(ch), id(id){
		}
		Tensor3() : rows(0), cols(0), slices(0),channels(0), id(0) {
		}
		Tensor3<T, I>& operator=(const Tensor3<T, I>& rhs) {
			if (this == &rhs)return *this;
			this->resize(rhs.rows, rhs.cols, rhs.slices,rhs.channels);
			this->id = rhs.id;
			this->data=rhs.data;
			return *this;
		}
		int4 dimensions() const {
			return int4(rows, cols, slices, channels);
		}
		size_t size() const {
			return data.size();
		}
		size_t typeSize() const {
			return sizeof(T);
		}
		void resize(int r, int c, int s,int ch) {
			data.resize((size_t)r * (size_t)c * (size_t)s*(size_t)ch);
			rows = r;
			cols = c;
			slices = s;
			channels=ch;
		}
		void resize(int3 dims,int ch) {
			data.resize((size_t)dims.x * (size_t)dims.y * (size_t)dims.z*(size_t)ch);
			rows = dims.x;
			cols = dims.y;
			slices=dims.z;
			channels=ch;
		}

		inline void clear() {
			data.clear();
			data.shrink_to_fit();
			rows = 0;
			cols = 0;
			slices = 0;
			channels = 0;
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
		void setZero() {
			data.assign(data.size(), (T)0);
		}
		void set(const T& val) {
			data.assign(data.size(), val);
		}
		const T& operator[](const size_t i) const {
			return data[i];
		}
		T& operator[](const size_t i) {
			return data[i];
		}
		size_t getIndex(int i,int j,int k,int ch=0) const {
			return	  clamp((int)ch, 0, channels)
					+ clamp((int)i , 0, rows   - 1) * channels
					+ clamp((int)j , 0, cols   - 1) * rows * channels
					+ clamp((int)k , 0, slices - 1) * rows * cols * channels;
		}

		T& operator()(const size_t i, const size_t j, const size_t k,const int c) {
			return *(&data[getIndex(i,j,k,c)]);
		}
		const T& operator()(const size_t i, const size_t j, const size_t k,const int c) const {
			return data[getIndex(i,j,k,c)];
		}
		inline VecMap<T> operator()(int i,int j,int k) const {
			return VecMap<T>(&data[getIndex(i,j,k)], channels, 1);
		}
		template<int C> void set(const aly::Image<T,C,I>& img,bool channelMajor=true) {
			size_t a=img.size();
			if(channelMajor){
				resize(img.width,img.height,C,1);
				for(size_t idx=0;idx<a;idx++){
					for(int c=0;c<C;c++){
						data[idx+a*c]=img[idx][c];
					}
				}
			} else {
				resize(img.width,img.height,1,C);
				for(size_t idx=0;idx<a;idx+=C){
					for(int c=0;c<C;c++){
						data[idx+c]=img[idx][c];
					}
				}
			}
		}
		void set(const Tensor2<T,I>& in,int slice) {
			if(in.rows!=rows||in.cols!=cols||in.channels!=channels||slice>=slices||slice<0){
				throw std::runtime_error(MakeString()<<"Tensor dimensions not compatible "<<dimensions()<<" :: "<<in.dimensions());
			}
			size_t a=in.size();
			size_t offset=channels*rows*cols*slice;
			for(size_t idx=0;idx<a;idx++){
				data[idx+offset]=in.data[idx];
			}
		}
		template<int C> void set(const aly::Vector<T,C>& in) {
			resize(in.size(),1,1,1);
			size_t a=in.size();
			for(size_t idx=0;idx<a;idx++){
				for(int c=0;c<C;c++){
					data[idx*C+c]=in.data[idx][c];
				}
			}
		}
		template<int C> void set(const std::vector<vec<T,C>>& in) {
			resize(in.size(),1,1,1);
			size_t a=in.size();
			for(size_t idx=0;idx<a;idx++){
				for(int c=0;c<C;c++){
					data[idx*C+c]=in[idx][c];
				}
			}
		}
		template<int C> void get(aly::Vector<T,C>& out) {
			if(channels!=C){
				throw std::runtime_error("Channel sizes do not match.");
			}
			out.resize(rows*cols*slices);
			size_t sz=out.size();
			for(size_t idx=0;idx<sz;idx++){
				for(int c=0;c<C;c++){
					out[idx][c]=data[idx*C+c];
				}
			}
		}
		template<int C> void get(std::vector<vec<T,C>>& out) {
			if(channels!=C){
				throw std::runtime_error("Channel sizes do not match.");
			}
			out.resize(rows*cols*slices);
			size_t sz=out.size();
			for(size_t idx=0;idx<sz;idx++){
				for(int c=0;c<C;c++){
					out[idx][c]=data[idx*C+c];
				}
			}
		}
		void get(std::vector<T>& out) {
			out.resize(rows*cols*slices*channels);
			size_t sz=out.size();
			for(size_t idx=0;idx<sz;idx++){
				out[idx]=data[idx];
			}
		}
		template<int C> void get(aly::Image<T,C,I>& img) {
			img.resize(rows,cols);
			size_t a=img.size();
			if(slices==C){
				for(size_t idx=0;idx<a;idx++){
					for(int c=0;c<C;c++){
						img[idx][c]=data[idx+a*c];
					}
				}
			} else {
				for(size_t idx=0;idx<a;idx++){
					for(int c=0;c<C;c++){
						img[idx][c]=data[idx*C+c];
					}
				}
			}
		}

		void set(const std::vector<T>& data) {
			resize(data.size(),1,1,1);
			this->data.assign(data.begin(),data.end());
		}
		inline void apply(const std::function<void(size_t index,T& value)>& func){
			size_t sz=data.size();
#pragma omp parallel for
			for (size_t offset = 0; offset < sz; offset++) {
				func(offset, data[offset]);
			}
		}
		inline void apply(const std::function<void(size_t index,size_t slice,T& value)>& func){
			size_t sz=rows*cols*channels;
#pragma omp parallel for
			for (size_t i = 0; i < sz; i++) {
				for (size_t s = 0; s < slices; s++) {
					func(i,s, data[i+sz*s]);
				}
			}
		}
		inline void apply(const std::function<void(size_t index,VecMap<T>& value)>& func){
			size_t sz=rows*cols*slices;
		#pragma omp parallel for
			for (size_t i = 0; i < sz; i++) {
				f(i,VecMap<T>(&data[i*channels], channels, 1));
			}
		}
	};
	template<class T, ImageType I> void Transform(Tensor3<T, I>& im1,
		Tensor3<T, I>& im2,
		const std::function<void(T&, T&)>& func) {
		if (im1.dimensions() != im2.dimensions())
			throw std::runtime_error(
				MakeString() << "Tensor3 dimensions do not match. "
				<< im1.dimensions() << "!=" << im2.dimensions());
		size_t sz = im1.size();
#pragma omp parallel for
		for (int offset = 0; offset < (int)sz; offset++) {
			func(im1.data[offset], im2.data[offset]);
		}
	}
	template<class T, ImageType I> void Transform(Tensor3<T, I>& im1,
		const Tensor3<T, I>& im2, const Tensor3<T, I>& im3,
		const Tensor3<T, I>& im4,
		const std::function<
		void(T&, const T&, const T&,
			const T&)>& func) {
		if (im1.dimensions() != im2.dimensions())
			throw std::runtime_error(
				MakeString() << "Tensor3 dimensions do not match. "
				<< im1.dimensions() << "!=" << im2.dimensions());
		size_t sz = im1.size();
#pragma omp parallel for
		for (int offset = 0; offset < (int)sz; offset++) {
			func(im1.data[offset], im2.data[offset], im3.data[offset],
				im4.data[offset]);
		}
	}
	template<class T, ImageType I> void Transform(Tensor3<T, I>& im1,
		const std::function<void(T&)>& func) {
		size_t sz = im1.size();
#pragma omp parallel for
		for (int offset = 0; offset < (int)sz; offset++) {
			func(im1.data[offset]);
		}
	}
	template<class T, ImageType I> void Transform(Tensor3<T, I>& im1,
		const Tensor3<T, I>& im2,
		const std::function<void(T&, const T&)>& func) {
		if (im1.dimensions() != im2.dimensions())
			throw std::runtime_error(
				MakeString() << "Tensor3 dimensions do not match. "
				<< im1.dimensions() << "!=" << im2.dimensions());
		size_t sz = im1.size();
#pragma omp parallel for
		for (int offset = 0; offset < (int)sz; offset++) {
			func(im1.data[offset], im2.data[offset]);
		}
	}
	template<class T, ImageType I> void Transform(Tensor3<T, I>& im1,
		const Tensor3<T, I>& im2, const Tensor3<T, I>& im3,
		const std::function<void(T&, const T&, const T&)>& func) {
		if (im1.dimensions() != im2.dimensions())
			throw std::runtime_error(
				MakeString() << "Tensor3 dimensions do not match. "
				<< im1.dimensions() << "!=" << im2.dimensions());
		size_t sz = im1.size();
#pragma omp parallel for
		for (int offset = 0; offset < (int)sz; offset++) {
			func(im1.data[offset], im2.data[offset], im3.data[offset]);
		}
	}
	template<class T, ImageType I> void Transform(Tensor3<T, I>& im1,
		Tensor3<T, I>& im2,
		const std::function<
		void(int i, int j, int k, T& val1, T& val2)>& func) {
		if (im1.dimensions() != im2.dimensions())
			throw std::runtime_error(
				MakeString() << "Tensor3 dimensions do not match. "
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
	template<class T, ImageType I> void Transform(Tensor3<T, I>& im1,
		Tensor3<T, I>& im2,
		const std::function<
		void(size_t offset, T& val1, T& val2)>& func) {
		if (im1.dimensions() != im2.dimensions())
			throw std::runtime_error(
				MakeString() << "Tensor3 dimensions do not match. "
				<< im1.dimensions() << "!=" << im2.dimensions());
		size_t sz = im1.size();
#pragma omp parallel for
		for (int offset = 0; offset < (int)sz; offset++) {
			func(offset, im1.data[offset], im2.data[offset]);
		}
	}
	template<class VecT, class T, class L, class R, int C, ImageType I> std::basic_ostream<L, R> & operator <<(
		std::basic_ostream<L, R> & ss, const Tensor3<T, I> & A) {
		ss << "Tensor3 (" << A.getTypeName() << "): " << A.id << " Position: ("
			<< A.x << "," << A.y << ") Dimensions: [" << A.rows << "," << A.cols
			<< "]\n";
		return ss;
	}
	template<class T, ImageType I> Tensor3<T, I> operator+(
		const T& scalar, const Tensor3<T, I>& img) {
		Tensor3<T, I> out(img.rows, img.cols, img.slices, img.position());
		std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = scalar + val2;};
		Transform(out, img, f);
		return out;
	}

	template<class T, ImageType I> Tensor3<T, I> operator-(
		const T& scalar, const Tensor3<T, I>& img) {
		Tensor3<T, I> out(img.rows, img.cols, img.slices, img.position());
		std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = scalar - val2;};
		Transform(out, img, f);
		return out;
	}
	template<class T, ImageType I> Tensor3<T, I> operator*(
		const T& scalar, const Tensor3<T, I>& img) {
		Tensor3<T, I> out(img.rows, img.cols, img.slices, img.position());
		std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = scalar*val2;};
		Transform(out, img, f);
		return out;
	}
	template<class T, ImageType I> Tensor3<T, I> operator/(
		const T& scalar, const Tensor3<T, I>& img) {
		Tensor3<T, I> out(img.rows, img.cols, img.slices, img.position());
		std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = scalar / val2;};
		Transform(out, img, f);
		return out;
	}
	template<class T, ImageType I> Tensor3<T, I> operator+(
		const Tensor3<T, I>& img, const T& scalar) {
		Tensor3<T, I> out(img.rows, img.cols, img.slices, img.position());
		std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = val2 + scalar;};
		Transform(out, img, f);
		return out;
	}
	template<class T, ImageType I> Tensor3<T, I> operator-(
		const Tensor3<T, I>& img, const T& scalar) {
		Tensor3<T, I> out(img.rows, img.cols, img.slices, img.position());
		std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = val2 - scalar;};
		Transform(out, img, f);
		return out;
	}
	template<class T, ImageType I> Tensor3<T, I> operator*(
		const Tensor3<T, I>& img, const T& scalar) {
		Tensor3<T, I> out(img.rows, img.cols, img.slices, img.position());
		std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = val2*scalar;};
		Transform(out, img, f);
		return out;
	}
	template<class T, ImageType I> Tensor3<T, I> operator/(
		const Tensor3<T, I>& img, const T& scalar) {
		Tensor3<T, I> out(img.rows, img.cols, img.slices, img.position());
		std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = val2 / scalar;};
		Transform(out, img, f);
		return out;
	}
	template<class T, ImageType I> Tensor3<T, I> operator-(
		const Tensor3<T, I>& img) {
		Tensor3<T, I> out(img.rows, img.cols, img.slices, img.position());
		std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = -val2;};
		Transform(out, img, f);
		return out;
	}
	template<class T, ImageType I> Tensor3<T, I> operator+=(
		Tensor3<T, I>& out, const Tensor3<T, I>& img) {
		std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 += val2;};
		Transform(out, img, f);
		return out;
	}
	template<class T, ImageType I> Tensor3<T, I> operator-=(
		Tensor3<T, I>& out, const Tensor3<T, I>& img) {
		std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 -= val2;};
		Transform(out, img, f);
		return out;
	}
	template<class T, ImageType I> Tensor3<T, I> operator*=(
		Tensor3<T, I>& out, const Tensor3<T, I>& img) {
		std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 *= val2;};
		Transform(out, img, f);
		return out;
	}
	template<class T, ImageType I> Tensor3<T, I> operator/=(
		Tensor3<T, I>& out, const Tensor3<T, I>& img) {
		std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 /= val2;};
		Transform(out, img, f);
		return out;
	}

	template<class T, ImageType I> Tensor3<T, I> operator+=(
		Tensor3<T, I>& out, const T& scalar) {
		std::function<void(T&)> f = [=](T& val1) {val1 += scalar;};
		Transform(out, f);
		return out;
	}
	template<class T, ImageType I> Tensor3<T, I> operator-=(
		Tensor3<T, I>& out, const T& scalar) {
		std::function<void(T&)> f = [=](T& val1) {val1 -= scalar;};
		Transform(out, f);
		return out;
	}
	template<class T, ImageType I> Tensor3<T, I> operator*=(
		Tensor3<T, I>& out, const T& scalar) {
		std::function<void(T&)> f = [=](T& val1) {val1 *= scalar;};
		Transform(out, f);
		return out;
	}
	template<class T, ImageType I> Tensor3<T, I> operator/=(
		Tensor3<T, I>& out, const T& scalar) {
		std::function<void(T&)> f = [=](T& val1) {val1 /= scalar;};
		Transform(out, f);
		return out;
	}

	template<class T, ImageType I> Tensor3<T, I> operator+(
		const Tensor3<T, I>& img1, const Tensor3<T, I>& img2) {
		Tensor3<T, I> out(img1.rows, img1.cols, img1.slices);
		std::function<void(T&, const T&, const T&)> f =
			[=](T& val1, const T& val2, const T& val3) {val1 = val2 + val3;};
		Transform(out, img1, img2, f);
		return out;
	}
	template<class T, ImageType I> Tensor3<T, I> operator-(
		const Tensor3<T, I>& img1, const Tensor3<T, I>& img2) {
		Tensor3<T, I> out(img1.rows, img1.cols, img1.slices);
		std::function<void(T&, const T&, const T&)> f =
			[=](T& val1, const T& val2, const T& val3) {val1 = val2 - val3;};
		Transform(out, img1, img2, f);
		return out;
	}
	template<class T, ImageType I> Tensor3<T, I> operator*(
		const Tensor3<T, I>& img1, const Tensor3<T, I>& img2) {
		Tensor3<T, I> out(img1.rows, img1.cols, img1.slices);
		std::function<void(T&, const T&, const T&)> f =
			[=](T& val1, const T& val2, const T& val3) {val1 = val2*val3;};
		Transform(out, img1, img2, f);
		return out;
	}
	template<class T, ImageType I> Tensor3<T, I> operator/(
		const Tensor3<T, I>& img1, const Tensor3<T, I>& img2) {
		Tensor3<T, I> out(img1.rows, img1.cols, img1.slices);
		std::function<void(T&, const T&, const T&)> f =
			[=](T& val1, const T& val2, const T& val3) {val1 = val2 / val3;};
		Transform(out, img1, img2, f);
		return out;
	}
	template<class T, ImageType I> bool ReadImageFromRawFile(
				const std::string& file, Tensor3<T, I>& img) {
			std::string xmlFile = GetFileWithoutExtension(file)+".xml";
			std::string rawFile = GetFileWithoutExtension(file)+".raw";
			MipavHeader header;
			img.clear();
			if(!ReadMipavHeaderFromFile(xmlFile,header))return false;
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
			img.resize(header.extents[0],header.extents[1],header.extents[2],header.extents[3]);
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
		template<class T, ImageType I> void WriteImageToRawFile(
			const std::string& file, const Tensor3<T, I>& img) {
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
	template<class T, ImageType I> void WriteTensorToFile(
		const std::string& file, const Tensor3<T, I>& img) {
		WriteImageToRawFile(file,img);
	}
	template<class T, ImageType I> bool ReadTensorFromFile(
		const std::string& file, Tensor3<T, I>& img) {
		return ReadImageFromRawFile(file,img);
	}
	typedef Tensor3<float,ImageType::FLOAT> Tensor3f;
	typedef Tensor3<double,ImageType::DOUBLE> Tensor3d;
	typedef Tensor3<int32_t,ImageType::INT> Tensor3i;
	typedef Tensor3<uint32_t,ImageType::UINT> Tensor3ui;
	typedef Tensor3<uint16_t,ImageType::USHORT> Tensor3us;
	typedef Tensor3<int16_t,ImageType::SHORT> Tensor3s;
	typedef Tensor3<uint8_t,ImageType::UBYTE> Tensor3ub;
	typedef Tensor3<int8_t,ImageType::BYTE> Tensor3b;
}
;

#endif
