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
#include "common/AlloyCommon.h"
#include "system/sha2.h"
#include "system/AlloyFileUtil.h"
#include "common/cereal/types/vector.hpp"
#include "image/AlloyImage.h"
#include "math/AlloyOptimizationMath.h"
#include <vector>
#include <functional>
#include <fstream>
#include <random>

#include "AlloyVecMath.h"
namespace aly {
template<class T, ImageType I> struct Tensor3;
template<class T, ImageType I> void WriteImageToRawFile(
		const std::string& fileName, const Tensor3<T, I>& img);
template<class T, ImageType I> bool ReadImageFromRawFile(
		const std::string& fileName, Tensor3<T, I>& img);
template<class T, ImageType I> struct Tensor3 {
public:
	int rows;
	int cols;
	int channels;
	uint64_t id;
	std::vector<DenseMat<T>> tensors;
	typedef DenseMat<T> ValueType;
	typedef typename std::vector<ValueType>::iterator iterator;
	typedef typename std::vector<ValueType>::const_iterator const_iterator;
	typedef typename std::vector<ValueType>::reverse_iterator reverse_iterator;
	bool empty() const {
		return tensors.empty();
	}
	iterator begin() {
		return tensors.begin();
	}
	iterator end() {
		return tensors.end();
	}
	const_iterator cbegin() const {
		return tensors.cbegin();
	}
	const_iterator cend() const {
		return tensors.cend();
	}
	reverse_iterator rbegin() {
		return tensors.rbegin();
	}
	reverse_iterator rend() {
		return tensors.rend();
	}
	reverse_iterator rbegin() const {
		return tensors.rbegin();
	}
	reverse_iterator rend() const {
		return tensors.rend();
	}
	const ImageType type = I;
	inline bool contains(const float3& pt) {
		return (pt.x >= 0 && pt.y >= 0 && pt.z >= 0 && pt.x < rows
				&& pt.y < cols);
	}
	inline bool contains(const int3& pt) {
		return (pt.x >= 0 && pt.y >= 0 && pt.z >= 0 && pt.x < rows
				&& pt.y < cols);
	}
	inline bool contains(int x, int y, int z) {
		return (x >= 0 && y >= 0 && z >= 0 && x < rows && y < cols);
	}
	inline bool contains(float x, float y, float z) {
		return (x >= 0 && y >= 0 && z >= 0 && x < rows && y < cols);
	}
	template<class Archive> void serialize(Archive & archive) {
		archive(CEREAL_NVP(id), CEREAL_NVP(rows), CEREAL_NVP(cols),
				CEREAL_NVP(channels), CEREAL_NVP(tensors));
	}

	Tensor3(int r, int c, int ch, uint64_t id = 0) :
			rows(r), cols(c), channels(ch), id(id) {
		tensors.resize(ch, DenseMat<T>(r, c));
	}
	Tensor3(const int3& dims, uint64_t id = 0) :
			rows(dims.x), cols(dims.y), channels(dims.z), id(id) {
		tensors.resize(dims.z, DenseMat<T>(dims.x, dims.y));
	}
	Tensor3() :
			rows(0), cols(0), channels(0), id(0) {
	}
	Tensor3<T, I>& operator=(const Tensor3<T, I>& rhs) {
		if (this == &rhs)
			return *this;
		this->resize(rhs.rows, rhs.cols, rhs.channels);
		this->id = rhs.id;
		this->tensors = rhs.tensors;
		return *this;
	}
	int3 dimensions() const {
		return int3(rows, cols, channels);
	}
	size_t size() const {
		return tensors.size();
	}
	size_t typeSize() const {
		return sizeof(T);
	}
	void resize(int r, int c, int ch) {
		tensors.resize(ch, DenseMat<T>(r, c));
		rows = r;
		cols = c;
		channels = ch;
	}
	void resize(int2 dims, int ch) {
		tensors.resize(ch, DenseMat<T>(dims.x, dims.y));
		rows = dims.x;
		cols = dims.y;
		channels = ch;
	}
	void resize(int3 dims) {
		tensors.resize(dims.z, DenseMat<T>(dims.x, dims.y));
		rows = dims.x;
		cols = dims.y;
		channels = dims.z;
	}
	inline void clear() {
		tensors.clear();
		tensors.shrink_to_fit();
		rows = 0;
		cols = 0;
		channels = 0;
	}
	void setZero() {
		for (DenseMat<T>& M : tensors) {
			M.setZero();
		}
	}
	void set(const T& val) {
		for (DenseMat<T>& M : tensors) {
			M.set(val);
		}
	}
	const DenseMat<T>& operator[](const size_t i) const {
		return tensors[i];
	}
	DenseMat<T>& operator[](const size_t i) {
		return tensors[i];
	}

	T& operator()(const size_t i, const size_t j, const size_t k) {
		return *(&tensors[k][i][j]);
	}
	const T& operator()(const size_t i, const size_t j, const size_t k) const {
		return tensors[k][i][j];
	}
};
template<class T, ImageType I> void Transform(Tensor3<T, I>& im1,
		Tensor3<T, I>& im2, const std::function<void(T&, T&)>& func) {
	if (im1.dimensions() != im2.dimensions())
		throw std::runtime_error(
				MakeString() << "Tensor2 dimensions do not match. "
						<< im1.dimensions() << "!=" << im2.dimensions());
	size_t sz = im1.size();
#pragma omp parallel for
	for (size_t offset = 0; offset < sz; offset++) {
		func(im1.data[offset], im2.data[offset]);
	}
}
template<class T, ImageType I> void Transform(Tensor3<T, I>& im1,
		const Tensor3<T, I>& im2, const Tensor3<T, I>& im3,
		const Tensor3<T, I>& im4,
		const std::function<void(T&, const T&, const T&, const T&)>& func) {
	if (im1.dimensions() != im2.dimensions())
		throw std::runtime_error(
				MakeString() << "Tensor2 dimensions do not match. "
						<< im1.dimensions() << "!=" << im2.dimensions());
	size_t sz = im1.size();
#pragma omp parallel for
	for (size_t offset = 0; offset < sz; offset++) {
		func(im1.data[offset], im2.data[offset], im3.data[offset],
				im4.data[offset]);
	}
}
template<class T, ImageType I> void Transform(Tensor3<T, I>& im1,
		const std::function<void(T&)>& func) {
	size_t sz = im1.size();
#pragma omp parallel for
	for (size_t offset = 0; offset < sz; offset++) {
		func(im1.data[offset]);
	}
}
template<class T, ImageType I> void Transform(Tensor3<T, I>& im1,
		const Tensor3<T, I>& im2,
		const std::function<void(T&, const T&)>& func) {
	if (im1.dimensions() != im2.dimensions())
		throw std::runtime_error(
				MakeString() << "Tensor2 dimensions do not match. "
						<< im1.dimensions() << "!=" << im2.dimensions());
	size_t sz = im1.size();
#pragma omp parallel for
	for (size_t offset = 0; offset < sz; offset++) {
		func(im1.data[offset], im2.data[offset]);
	}
}
template<class T, ImageType I> void Transform(Tensor3<T, I>& im1,
		const Tensor3<T, I>& im2, const Tensor3<T, I>& im3,
		const std::function<void(T&, const T&, const T&)>& func) {
	if (im1.dimensions() != im2.dimensions())
		throw std::runtime_error(
				MakeString() << "Tensor2 dimensions do not match. "
						<< im1.dimensions() << "!=" << im2.dimensions());
	size_t sz = im1.size();
#pragma omp parallel for
	for (size_t offset = 0; offset < sz; offset++) {
		func(im1.data[offset], im2.data[offset], im3.data[offset]);
	}
}
template<class T, ImageType I> void Transform(Tensor3<T, I>& im1,
		Tensor3<T, I>& im2,
		const std::function<void(int i, int j, int k, T& val1, T& val2)>& func) {
	if (im1.dimensions() != im2.dimensions())
		throw std::runtime_error(
				MakeString() << "Tensor2 dimensions do not match. "
						<< im1.dimensions() << "!=" << im2.dimensions());
#pragma omp parallel for
	for (int k = 0; k < im1.channels; k++) {
		DenseMat<T>& im1d = im1[k];
		DenseMat<T>& im2d = im2[k];
		for (int j = 0; j < im1.cols; j++) {
			for (int i = 0; i < im1.rows; i++) {
				func(i, j, k, im1d(i, j), im2d(i, j));
			}
		}
	}
}
template<class T, ImageType I> void Transform(Tensor3<T, I>& im1,
		Tensor3<T, I>& im2,
		const std::function<void(size_t offset, T& val1, T& val2)>& func) {
	if (im1.dimensions() != im2.dimensions())
		throw std::runtime_error(
				MakeString() << "Tensor2 dimensions do not match. "
						<< im1.dimensions() << "!=" << im2.dimensions());
	size_t sz = im1.size();
#pragma omp parallel for
	for (int k = 0; k < im1.channels; k++) {
		DenseMat<T>& im1d = im1[k];
		DenseMat<T>& im2d = im2[k];
		for (int j = 0; j < im1.cols; j++) {
			for (int i = 0; i < im1.rows; i++) {
				func(i, j, k, im1d(i, j), im2d(i, j));
			}
		}
	}
}
template<class VecT, class T, class L, class R, int C, ImageType I> std::basic_ostream<
		L, R> & operator <<(std::basic_ostream<L, R> & ss,
		const Tensor3<T, I> & A) {
	ss << "Tensor2 (" << A.getTypeName() << "): " << A.id << " Position: ("
			<< A.x << "," << A.y << ") Dimensions: [" << A.rows << "," << A.cols
			<< "]\n";
	return ss;
}
template<class T, ImageType I> Tensor3<T, I> operator+(const T& scalar,
		const Tensor3<T, I>& img) {
	Tensor3<T, I> out(img.rows, img.cols, img.channels, img.position());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = scalar + val2;};
	Transform(out, img, f);
	return out;
}

template<class T, ImageType I> Tensor3<T, I> operator-(const T& scalar,
		const Tensor3<T, I>& img) {
	Tensor3<T, I> out(img.rows, img.cols, img.channels, img.position());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = scalar - val2;};
	Transform(out, img, f);
	return out;
}
template<class T, ImageType I> Tensor3<T, I> operator*(const T& scalar,
		const Tensor3<T, I>& img) {
	Tensor3<T, I> out(img.rows, img.cols, img.channels, img.position());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = scalar*val2;};
	Transform(out, img, f);
	return out;
}
template<class T, ImageType I> Tensor3<T, I> operator/(const T& scalar,
		const Tensor3<T, I>& img) {
	Tensor3<T, I> out(img.rows, img.cols, img.channels, img.position());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = scalar / val2;};
	Transform(out, img, f);
	return out;
}
template<class T, ImageType I> Tensor3<T, I> operator+(const Tensor3<T, I>& img,
		const T& scalar) {
	Tensor3<T, I> out(img.rows, img.cols, img.channels, img.position());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = val2 + scalar;};
	Transform(out, img, f);
	return out;
}
template<class T, ImageType I> Tensor3<T, I> operator-(const Tensor3<T, I>& img,
		const T& scalar) {
	Tensor3<T, I> out(img.rows, img.cols, img.channels, img.position());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = val2 - scalar;};
	Transform(out, img, f);
	return out;
}
template<class T, ImageType I> Tensor3<T, I> operator*(const Tensor3<T, I>& img,
		const T& scalar) {
	Tensor3<T, I> out(img.rows, img.cols, img.channels, img.position());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = val2*scalar;};
	Transform(out, img, f);
	return out;
}
template<class T, ImageType I> Tensor3<T, I> operator/(const Tensor3<T, I>& img,
		const T& scalar) {
	Tensor3<T, I> out(img.rows, img.cols, img.channels, img.position());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = val2 / scalar;};
	Transform(out, img, f);
	return out;
}
template<class T, ImageType I> Tensor3<T, I> operator-(
		const Tensor3<T, I>& img) {
	Tensor3<T, I> out(img.rows, img.cols, img.channels, img.position());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = -val2;};
	Transform(out, img, f);
	return out;
}
template<class T, ImageType I> Tensor3<T, I>& operator+=(Tensor3<T, I>& out,
		const Tensor3<T, I>& img) {
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 += val2;};
	Transform(out, img, f);
	return out;
}
template<class T, ImageType I> Tensor3<T, I>& operator-=(Tensor3<T, I>& out,
		const Tensor3<T, I>& img) {
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 -= val2;};
	Transform(out, img, f);
	return out;
}
template<class T, ImageType I> Tensor3<T, I>& operator*=(Tensor3<T, I>& out,
		const Tensor3<T, I>& img) {
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 *= val2;};
	Transform(out, img, f);
	return out;
}
template<class T, ImageType I> Tensor3<T, I>& operator/=(Tensor3<T, I>& out,
		const Tensor3<T, I>& img) {
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 /= val2;};
	Transform(out, img, f);
	return out;
}

template<class T, ImageType I> Tensor3<T, I>& operator+=(Tensor3<T, I>& out,
		const T& scalar) {
	std::function<void(T&)> f = [=](T& val1) {val1 += scalar;};
	Transform(out, f);
	return out;
}
template<class T, ImageType I> Tensor3<T, I>& operator-=(Tensor3<T, I>& out,
		const T& scalar) {
	std::function<void(T&)> f = [=](T& val1) {val1 -= scalar;};
	Transform(out, f);
	return out;
}
template<class T, ImageType I> Tensor3<T, I>& operator*=(Tensor3<T, I>& out,
		const T& scalar) {
	std::function<void(T&)> f = [=](T& val1) {val1 *= scalar;};
	Transform(out, f);
	return out;
}
template<class T, ImageType I> Tensor3<T, I>& operator/=(Tensor3<T, I>& out,
		const T& scalar) {
	std::function<void(T&)> f = [=](T& val1) {val1 /= scalar;};
	Transform(out, f);
	return out;
}

template<class T, ImageType I> Tensor3<T, I> operator+(
		const Tensor3<T, I>& img1, const Tensor3<T, I>& img2) {
	Tensor3<T, I> out(img1.rows, img1.cols, img1.channels);
	std::function<void(T&, const T&, const T&)> f =
			[=](T& val1, const T& val2, const T& val3) {val1 = val2 + val3;};
	Transform(out, img1, img2, f);
	return out;
}
template<class T, ImageType I> Tensor3<T, I> operator-(
		const Tensor3<T, I>& img1, const Tensor3<T, I>& img2) {
	Tensor3<T, I> out(img1.rows, img1.cols, img1.channels);
	std::function<void(T&, const T&, const T&)> f =
			[=](T& val1, const T& val2, const T& val3) {val1 = val2 - val3;};
	Transform(out, img1, img2, f);
	return out;
}
template<class T, ImageType I> Tensor3<T, I> operator*(
		const Tensor3<T, I>& img1, const Tensor3<T, I>& img2) {
	Tensor3<T, I> out(img1.rows, img1.cols, img1.channels);
	std::function<void(T&, const T&, const T&)> f =
			[=](T& val1, const T& val2, const T& val3) {val1 = val2*val3;};
	Transform(out, img1, img2, f);
	return out;
}
template<class T, ImageType I> Tensor3<T, I> operator/(
		const Tensor3<T, I>& img1, const Tensor3<T, I>& img2) {
	Tensor3<T, I> out(img1.rows, img1.cols, img1.channels);
	std::function<void(T&, const T&, const T&)> f =
			[=](T& val1, const T& val2, const T& val3) {val1 = val2 / val3;};
	Transform(out, img1, img2, f);
	return out;
}
template<class T, ImageType I> bool ReadImageFromRawFile(
		const std::string& file, Tensor3<T, I>& img) {
	std::string xmlFile = GetFileWithoutExtension(file) + ".xml";
	std::string rawFile = GetFileWithoutExtension(file) + ".raw";
	MipavHeader header;
	img.clear();
	if (!ReadMipavHeaderFromFile(xmlFile, header))
		return false;
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
	if (ToLower(header.dataType) != typeName) {
		throw std::runtime_error(
				MakeString() << "Type " << header.dataType << "/" << typeName
						<< " do not match.");
	}
	img.resize(header.extents[0], header.extents[1], header.extents[2],
			header.extents[3]);
	FILE* f = fopen(rawFile.c_str(), "rb");
	if (f == NULL) {
		throw std::runtime_error(
				MakeString() << "Could not open " << rawFile
						<< " for reading.");
	}
	for (int k = 0; k < img.channels; k++) {
		for (int j = 0; j < img.cols; j++) {
			for (int i = 0; i < img.rows; i++) {
				T* ptr = &img(i, j, k);
				fread(ptr, sizeof(T), 1, f);
			}
		}
	}
	fclose(f);
}
template<class T, ImageType I> void WriteImageToRawFile(const std::string& file,
		const Tensor3<T, I>& img) {
	std::ostringstream vstr;
	std::string fileName = GetFileWithoutExtension(file);
	vstr << fileName << ".raw";
	FILE* f = fopen(vstr.str().c_str(), "wb");
	if (f == NULL) {
		throw std::runtime_error(
				MakeString() << "Could not open " << vstr.str().c_str()
						<< " for writing.");
	}
	for (int k = 0; k < img.channels; k++) {
		for (int j = 0; j < img.cols; j++) {
			for (int i = 0; i < img.rows; i++) {
				T* ptr = &img(i, j, k);
				fwrite(ptr, sizeof(T), 1, f);
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
	sstr<< "<image xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" nDimensions=\"3\">\n";
	sstr << "	<Dataset-attributes>\n";
	sstr << "		<Image-offset>0</Image-offset>\n";
	sstr << "		<Data-type>" << typeName << "</Data-type>\n";
	sstr << "		<Endianess>Little</Endianess>\n";
	sstr << "		<Extents>" << img.rows << "</Extents>\n";
	sstr << "		<Extents>" << img.cols << "</Extents>\n";
	sstr << "		<Extents>" << img.channels << "</Extents>\n";
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
template<class T, ImageType I> void WriteTensorToFile(const std::string& file,
		const Tensor3<T, I>& img) {
	WriteImageToRawFile(file, img);
}
template<class T, ImageType I> bool ReadTensorFromFile(const std::string& file,
		Tensor3<T, I>& img) {
	return ReadImageFromRawFile(file, img);
}
typedef Tensor3<float, ImageType::FLOAT> Tensor3f;
typedef Tensor3<double, ImageType::DOUBLE> Tensor3d;
typedef Tensor3<int32_t, ImageType::INT> Tensor3i;
typedef Tensor3<uint32_t, ImageType::UINT> Tensor3ui;
typedef Tensor3<uint16_t, ImageType::USHORT> Tensor3us;
typedef Tensor3<int16_t, ImageType::SHORT> Tensor3s;
typedef Tensor3<uint8_t, ImageType::UBYTE> Tensor3ub;
typedef Tensor3<int8_t, ImageType::BYTE> Tensor3b;

}
;

#endif
