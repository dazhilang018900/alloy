/*
 * Copyright(C) 2014, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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
#include "image/AlloyImage.h"
#include "common/AlloyCommon.h"
#include "system/AlloyFileUtil.h"
#include "math/AlloyVecMath.h"
#include "image/TiffReader.h"
#include "image/TiffWriter.h"
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "image/stb_image.h"
#include "image/stb_image_write.h"
#include "image/tinyexr.h"
#include <fstream>
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS // suppress warnings about fopen()
#endif

namespace aly {
void ConvertImage(const Image1f& in, ImageRGBAf& out) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t N = out.size();
#pragma omp parallel for
	for (size_t i = 0; i < N; i++) {
		float lum = in[i].x;
		out[i] = float4(lum, lum, lum, 1.0f);
	}
}
void ConvertImage(const Image2f& in, ImageRGBAf& out) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t N = out.size();
#pragma omp parallel for
	for (size_t i = 0; i < N; i++) {
		float2 val = in[i];
		float lum = val.x;
		out[i] = float4(lum, lum, lum, val.y);
	}
}
void ConvertImage(const Image1f& in, ImageRGBf& out) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t N = out.size();
#pragma omp parallel for
	for (size_t i = 0; i < N; i++) {
		float lum = in[i].x;
		out[i] = float3(lum, lum, lum);
	}
}
void ConvertImage(const Image2f& in, ImageRGBf& out) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t N = out.size();
#pragma omp parallel for
	for (size_t i = 0; i < N; i++) {
		float lum = in[i].x;
		out[i] = float3(lum, lum, lum);
	}
}
void ConvertImage(const Image1ub& in, ImageRGBAf& out) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t N = out.size();
#pragma omp parallel for
	for (size_t i = 0; i < N; i++) {
		float lum = in[i].x / 255.0f;
		out[i] = float4(lum, lum, lum, 1.0f);
	}
}
void ConvertImage(const Image1ub& in, ImageRGBf& out) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t N = out.size();
#pragma omp parallel for
	for (size_t i = 0; i < N; i++) {
		float lum = in[i].x / 255.0f;
		out[i] = float3(lum, lum, lum);
	}
}
void ConvertImage(const Image1ub& in, ImageRGBA& out) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t N = out.size();
#pragma omp parallel for
	for (size_t i = 0; i < N; i++) {
		ubyte lum = in[i].x;
		out[i] = aly::RGBA(lum, lum, lum, 255);
	}
}
void ConvertImage(const Image1ub& in, ImageRGB& out) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t N = out.size();
#pragma omp parallel for
	for (size_t i = 0; i < N; i++) {
		ubyte lum = in[i].x;
		out[i] = aly::ubyte3(lum, lum, lum);
	}
}




void ConvertImage(const Image2ub& in, ImageRGBAf& out) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t N = out.size();
#pragma omp parallel for
	for (size_t i = 0; i < N; i++) {
		float lum = in[i].x / 255.0f;
		out[i] = float4(lum, lum, lum, in[i].y/255.0f);
	}
}
void ConvertImage(const Image2ub& in, ImageRGBf& out) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t N = out.size();
#pragma omp parallel for
	for (size_t i = 0; i < N; i++) {
		float lum = in[i].x / 255.0f;
		out[i] = float3(lum, lum, lum);
	}
}
void ConvertImage(const Image2ub& in, ImageRGBA& out) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t N = out.size();
#pragma omp parallel for
	for (size_t i = 0; i < N; i++) {
		ubyte lum = in[i].x;
		out[i] = aly::RGBA(lum, lum, lum, in[i].y);
	}
}
void ConvertImage(const Image2ub& in, ImageRGB& out) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t N = out.size();
#pragma omp parallel for
	for (size_t i = 0; i < N; i++) {
		ubyte lum = in[i].x;
		out[i] = aly::ubyte3(lum, lum, lum);
	}
}


void ConvertImage(const Image2f& in, ImageRGBA& out) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t N = out.size();
#pragma omp parallel for
	for (size_t i = 0; i < N; i++) {
		ubyte lum = (ubyte) clamp(255.0 * in[i].x, 0.0, 255.0);
		out[i] = RGBA(lum, lum, lum, (ubyte) clamp(255.0 * in[i].y, 0.0, 255.0));
	}
}
void ConvertImage(const Image1f& in, ImageRGBA& out) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t N = out.size();
#pragma omp parallel for
	for (size_t i = 0; i < N; i++) {
		ubyte lum = (ubyte) clamp(255.0 * in[i].x, 0.0, 255.0);
		out[i] = RGBA(lum, lum, lum, 255);
	}
}
void ConvertImage(const Image1f& in, ImageRGB& out) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t N = out.size();
#pragma omp parallel for
	for (size_t i = 0; i < N; i++) {
		ubyte lum = (ubyte) clamp(255.0 * in[i].x, 0.0, 255.0);
		out[i] = aly::ubyte3(lum, lum, lum);
	}
}
void ConvertImage(const Image2f& in, ImageRGB& out) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t N = out.size();
#pragma omp parallel for
	for (size_t i = 0; i < N; i++) {
		ubyte lum = (ubyte) clamp(255.0 * in[i].x, 0.0, 255.0);
		out[i] = aly::ubyte3(lum, lum, lum);
	}
}
void ConvertImage(const ImageRGBA& in, Image1f& out, bool sRGB) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t N = out.size();

	if (sRGB) {
#pragma omp parallel for
		for (size_t i = 0; i < N; i++) {
			ubyte4 c = in[i];
			out[i] = float1(
					(float) (0.21 * c.x + 0.72 * c.y + 0.07 * c.z) / 255.0f);
		}
	} else {
#pragma omp parallel for
		for (size_t i = 0; i < N; i++) {
			ubyte4 c = in[i];
			out[i] = float1(
					(float) (0.30 * c.x + 0.59 * c.y + 0.11 * c.z) / 255.0f);
		}
	}
}

void ConvertImage(const ImageRGB& in, Image1f& out, bool sRGB) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t N = out.size();
	if (sRGB) {
#pragma omp parallel for
		for (size_t i = 0; i < N; i++) {
			ubyte3 c = in[i];
			out[i] = float1(
					(float) (0.21 * c.x + 0.72 * c.y + 0.07 * c.z) / 255.0f);
		}
	} else {
#pragma omp parallel for
		for (size_t i = 0; i < N; i++) {
			ubyte3 c = in[i];
			out[i] = float1(
					(float) (0.30 * c.x + 0.59 * c.y + 0.11 * c.z) / 255.0f);
		}
	}
}
void ConvertImage(const ImageRGBAf& in, Image1ub& out, bool sRGB) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t N = out.size();

	if (sRGB) {
#pragma omp parallel for
		for (size_t i = 0; i < N; i++) {
			float4 c = in[i];
			out[i] = ubyte1(
					(uint8_t) clamp(
							255 * (0.21 * c.x + 0.72 * c.y + 0.07 * c.z), 0.0,
							255.0));
		}
	} else {
#pragma omp parallel for
		for (size_t i = 0; i < N; i++) {
			float4 c = in[i];
			out[i] = ubyte1(
					(uint8_t) clamp(
							255 * (0.30 * c.x + 0.59 * c.y + 0.11 * c.z), 0.0,
							255.0));
		}
	}
}
void ConvertImage(const ImageRGBf& in, Image1ub& out, bool sRGB) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t N = out.size();

	if (sRGB) {
#pragma omp parallel for
		for (size_t i = 0; i < N; i++) {
			float3 c = in[i];
			out[i] = ubyte1(
					(uint8_t) clamp(
							255 * (0.21 * c.x + 0.72 * c.y + 0.07 * c.z), 0.0,
							255.0));
		}
	} else {
#pragma omp parallel for
		for (size_t i = 0; i < N; i++) {
			float3 c = in[i];
			out[i] = ubyte1(
					(uint8_t) clamp(
							255 * (0.30 * c.x + 0.59 * c.y + 0.11 * c.z), 0.0,
							255.0));
		}
	}
}

void ConvertImage(const ImageRGBA& in, Image2f& out, bool sRGB) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t N = out.size();
	if (sRGB) {
#pragma omp parallel for
		for (size_t i = 0; i < N; i++) {
			ubyte4 c = in[i];
			out[i] = float2(
					(float) (0.21 * c.x + 0.72 * c.y + 0.07 * c.z) / 255.0f,c.w/255.0f);
		}
	} else {
#pragma omp parallel for
		for (size_t i = 0; i < N; i++) {
			ubyte4 c = in[i];
			out[i] = float2(
					(float) (0.30 * c.x + 0.59 * c.y + 0.11 * c.z) / 255.0f,c.w/255.0f);
		}
	}
}

void ConvertImage(const ImageRGB& in, Image2f& out, bool sRGB) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t N = out.size();

	if (sRGB) {
#pragma omp parallel for
		for (size_t i = 0; i < N; i++) {
			ubyte3 c = in[i];
			out[i] = float2(
					(float) (0.21 * c.x + 0.72 * c.y + 0.07 * c.z) / 255.0f,1.0f);
		}
	} else {
#pragma omp parallel for
		for (size_t i = 0; i < N; i++) {
			ubyte3 c = in[i];
			out[i] = float2(
					(float) (0.30 * c.x + 0.59 * c.y + 0.11 * c.z) / 255.0f,1.0f);
		}
	}
}
void ConvertImage(const ImageRGBAf& in, Image2ub& out, bool sRGB) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t N = out.size();

	if (sRGB) {
#pragma omp parallel for
		for (size_t i = 0; i < N; i++) {
			float4 c = in[i];
			out[i] = ubyte2(
					(uint8_t) clamp(
							255 * (0.21 * c.x + 0.72 * c.y + 0.07 * c.z), 0.0,
							255.0),(uint8_t)clamp(255.0f*c.w,0.0f,255.0f));
		}
	} else {
#pragma omp parallel for
		for (size_t i = 0; i < N; i++) {
			float4 c = in[i];
			out[i] = ubyte2(
					(uint8_t) clamp(
							255 * (0.30 * c.x + 0.59 * c.y + 0.11 * c.z), 0.0,
							255.0),(uint8_t)clamp(255.0f*c.w,0.0f,255.0f));
		}
	}
}
void ConvertImage(const ImageRGBf& in, Image2ub& out, bool sRGB) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t N = out.size();

	if (sRGB) {
#pragma omp parallel for
		for (size_t i = 0; i < N; i++) {
			float3 c = in[i];
			out[i] = ubyte2(
					(uint8_t) clamp(
							255 * (0.21 * c.x + 0.72 * c.y + 0.07 * c.z), 0.0,
							255.0),255);
		}
	} else {
#pragma omp parallel for
		for (size_t i = 0; i < N; i++) {
			float3 c = in[i];
			out[i] = ubyte2(
					(uint8_t) clamp(
							255 * (0.30 * c.x + 0.59 * c.y + 0.11 * c.z), 0.0,
							255.0),255);
		}
	}
}

template<class T, int C, ImageType I> void ReadTiffImageFromFileInternal(const std::string& file,Image<T,C,I>& image){
	ReadTiffImage(file,image);
}
template<class T, int C, ImageType I> void WriteTiffImageToFileInternal(const std::string& file,const Image<T,C,I>& image){
	WriteTiffImage(file,image,-1);//use LZW
}
void WriteImageToFile(const std::string& file, const ImageRGB& image) {
	std::string ext = GetFileExtension(file);
	if (ext == "xml") {
		WriteImageToRawFile(file, image);
	} else if (ext == "png") {
		if (!stbi_write_png(file.c_str(), image.width, image.height, 3,
				image.data.data(), 3 * image.width)) {
			throw std::runtime_error(
					MakeString() << "Could not write " << file);
		}
	} else if (ext == "jpg" || ext == "jpeg") {
		stbi_write_jpg(file.c_str(), image.width, image.height, 3,image.data.data(),90);
	} else if (ext == "tif" || ext == "tiff") {
		WriteTiffImageToFileInternal(file,image);
	} else {
		throw std::runtime_error(MakeString() << "Could not write " << file);
	}
}
void WriteImageToFile(const std::string& file, const ImageRGB& image,int quality) {
	std::string ext = GetFileExtension(file);
	if (ext == "jpg" || ext == "jpeg") {
		stbi_write_jpg(file.c_str(), image.width, image.height, 3,image.data.data(),quality);
	} else {
		throw std::runtime_error(MakeString() << "Could not write " << file);
	}
}
void WriteImageToFile(const std::string& file, const Image1ub& image) {
	std::string ext = GetFileExtension(file);
	if (ext == "xml") {
		WriteImageToRawFile(file, image);
	} else if (ext == "png") {
		if (!stbi_write_png(file.c_str(), image.width, image.height, 1,
				image.ptr(), image.width)) {
			throw std::runtime_error(
					MakeString() << "Could not write " << file);
		}
	} else if (ext == "jpg" || ext == "jpeg") {
		stbi_write_jpg(file.c_str(), image.width, image.height, 1, image.data.data(), 90);
	} else if (ext == "tif" || ext == "tiff") {
		WriteTiffImageToFileInternal(file,image);
	} else {
		throw std::runtime_error(MakeString() << "Could not write " << file);
	}
}
void WriteImageToFile(const std::string& file, const ImageRGBA& image) {
	std::string ext = GetFileExtension(file);
	if (ext == "xml") {
		WriteImageToRawFile(file, image);
	} else if (ext == "png") {
		if (!stbi_write_png(file.c_str(), image.width, image.height, 4,
				image.ptr(), 4 * image.width)) {
			throw std::runtime_error(
					MakeString() << "Could not write " << file);
		}
	} else if(ext == "jpg" || ext == "jpeg"){
		ImageRGB tmp;
		ConvertImage(image,tmp);
		stbi_write_jpg(file.c_str(), tmp.width, tmp.height, 3,tmp.data.data(),90);
	} else if (ext == "tif" || ext == "tiff") {
		WriteTiffImageToFileInternal(file,image);
	} else {
		throw std::runtime_error(MakeString() << "Could not write " << file);
	}
}
void ReadImageFromFile(const std::string& file, ImageRGBA& image) {
	std::string ext = GetFileExtension(file);
	if (ext != "png" && ext != "tga" && ext != "bmp" && ext != "psd"
			&& ext != "gif" && ext != "jpg"&&ext!="tiff"&&ext!="tif") {
		throw std::runtime_error(
				MakeString() << "Could not read file " << file);
	}
	if(ext=="tiff"||ext=="tif"){
		ReadTiffImageFromFileInternal(file,image);
	} else {
		unsigned char* img;
		stbi_set_unpremultiply_on_load(1);
		stbi_convert_iphone_png_to_rgb(1);
		int w, h, n;
		img = stbi_load(file.c_str(), &w, &h, &n, 4);
		if (img == NULL) {
			throw std::runtime_error(
					MakeString() << "Could not read file " << file);
		}
		image.resize(w, h);
		image.set(img);
		stbi_image_free(img);
	}
}
void ReadImageFromFile(const std::string& file, Image1ub& image) {
	std::string ext = GetFileExtension(file);
	if (ext != "png" && ext != "tga" && ext != "bmp" && ext != "psd"
			&& ext != "gif" && ext != "jpg"&&ext!="tiff"&&ext!="tif") {
		throw std::runtime_error(
				MakeString() << "Could not read file " << file);
	}
	if(ext=="tiff"||ext=="tif"){
		ReadTiffImageFromFileInternal(file,image);
	} else {
		unsigned char* img;
		stbi_set_unpremultiply_on_load(1);
		stbi_convert_iphone_png_to_rgb(1);
		int w, h, n;
		img = stbi_load(file.c_str(), &w, &h, &n, 1);
		if (img == NULL) {
			throw std::runtime_error(
					MakeString() << "Could not read file " << file);
		}
		image.resize(w, h);
		image.set(img);
		stbi_image_free(img);
	}
}
void ReadImageFromFile(const std::string& file, ImageRGB& image) {
	std::string ext = GetFileExtension(file);
	if (ext != "png" && ext != "tga" && ext != "bmp" && ext != "psd"
			&& ext != "gif" && ext != "jpg"&&ext!="tiff"&&ext!="tif") {
		throw std::runtime_error(
				MakeString() << "Could not read file " << file);
	}
	if(ext=="tiff"||ext=="tif"){
		ReadTiffImageFromFileInternal(file,image);
	} else {
		unsigned char* img;
		stbi_set_unpremultiply_on_load(1);
		stbi_convert_iphone_png_to_rgb(1);
		int w, h, n;
		img = stbi_load(file.c_str(), &w, &h, &n, 3);
		if (img == NULL) {
			throw std::runtime_error(
					MakeString() << "Could not read file " << file);
		}
		image.resize(w, h);
		image.set(img);
		stbi_image_free(img);
	}

}
void ReadImageFromFile(const std::string& file, ImageRGBAf& img) {
	std::string ext = GetFileExtension(file);
	if (ext == "exr") {
		const char *message = nullptr;
		EXRImage exrImage;
		InitEXRImage(&exrImage);
		int ret = ParseMultiChannelEXRHeaderFromFile(&exrImage, file.c_str(),
				&message);
		if (ret != 0) {
			throw std::runtime_error(message);
		}
		for (int i = 0; i < exrImage.num_channels; i++) {
			if (exrImage.pixel_types[i] == TINYEXR_PIXELTYPE_HALF) {
				exrImage.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
			}
		}
		ret = LoadMultiChannelEXRFromFile(&exrImage, file.c_str(), &message);
		if (ret != 0) {
			throw std::runtime_error(message);
		}
		std::vector<int> channelIndex(4, 0);
		for (int i = 0; i < exrImage.num_channels; i++) {
			std::string cname(exrImage.channel_names[i]);
			if (cname == "R" || cname == "r"||cname=="red"||cname=="RED") {
				channelIndex[0] = i;
			} else if (cname == "G" || cname == "g"||cname=="green"||cname=="GREEN") {
				channelIndex[1] = i;
			} else if (cname == "B" || cname == "b"||cname=="blue"||cname=="BLUE") {
				channelIndex[2] = i;
			} else if (cname == "A" || cname == "a"||cname=="alpha"||cname=="ALPHA") {
				channelIndex[3] = i;
			}
		}
		img.resize(exrImage.width, exrImage.height);
		std::vector<float> imageData(img.size() * img.channels);
		float** ptr = reinterpret_cast<float **>(exrImage.images);
		for (int c = 0; c < img.channels; c++) {
			size_t index = 0;
			for (float4& val : img.data) {
				val[c] = ptr[channelIndex[c]][index++];
			}
		}
		FreeEXRImage(&exrImage);
	} else if (ext == "hdr") {
		int w, h, n;
		float *data = stbi_loadf(file.c_str(), &w, &h, &n, 4);
		if (data == NULL) {
			throw std::runtime_error(
					MakeString() << "Could not read file " << file);
		}
		img.resize(w, h);
		img.set(data);
		stbi_image_free(data);
	} else 	if(ext=="tiff"||ext=="tif"){
		ReadTiffImageFromFileInternal(file,img);
	} else {
		std::string ext = GetFileExtension(file);
		ImageRGBA rgb;
		ReadImageFromFile(file, rgb);
		img.resize(rgb.width, rgb.height);
		img.id = rgb.id;
		img.setPosition(rgb.position());

		size_t index = 0;
		for (RGBAf& ct : img.data) {
			RGBA cs = rgb[index++];
			ct = RGBAf(cs.x / 255.0f, cs.y / 255.0f, cs.z / 255.0f,
					cs.w / 255.0f);
		}
	}
}
void ReadImageFromFile(const std::string& file, ImageRGBf& img) {
	std::string ext = GetFileExtension(file);
	if (ext == "exr") {
		const char *message = nullptr;
		EXRImage exrImage;
		InitEXRImage(&exrImage);
		int ret = ParseMultiChannelEXRHeaderFromFile(&exrImage, file.c_str(),
				&message);
		if (ret != 0) {
			throw std::runtime_error(message);
		}
		for (int i = 0; i < exrImage.num_channels; i++) {
			if (exrImage.pixel_types[i] == TINYEXR_PIXELTYPE_HALF) {
				exrImage.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
			}
		}
		ret = LoadMultiChannelEXRFromFile(&exrImage, file.c_str(), &message);
		if (ret != 0) {
			throw std::runtime_error(message);
		}
		std::vector<int> channelIndex(3, 0);
		for (int i = 0; i < exrImage.num_channels; i++) {
			std::string cname(exrImage.channel_names[i]);
			if (cname == "R" || cname == "r" || cname == "red" || cname == "RED") {
				channelIndex[0] = i;
			}
			else if (cname == "G" || cname == "g" || cname == "green" || cname == "GREEN") {
				channelIndex[1] = i;
			}
			else if (cname == "B" || cname == "b" || cname == "blue" || cname == "BLUE") {
				channelIndex[2] = i;
			}
		}
		img.resize(exrImage.width, exrImage.height);
		std::vector<float> imageData(img.size() * img.channels);
		float** ptr = reinterpret_cast<float **>(exrImage.images);
		for (int c = 0; c < img.channels; c++) {
			size_t index = 0;
			for (float3& val : img.data) {
				val[c] = ptr[channelIndex[c]][index++];
			}
		}
		FreeEXRImage(&exrImage);
	} else if (ext == "hdr") {
		int w, h, n;
		float *data = stbi_loadf(file.c_str(), &w, &h, &n, 3);
		if (data == NULL) {
			throw std::runtime_error(
					MakeString() << "Could not read file " << file);
		}
		img.resize(w, h);
		img.set(data);
		stbi_image_free(data);
	} else if(ext=="tiff"||ext=="tif"){
		ReadTiffImageFromFileInternal(file,img);
	} else {
		ImageRGB rgb;
		ReadImageFromFile(file, rgb);
		img.resize(rgb.width, rgb.height);
		img.id = rgb.id;
		img.setPosition(rgb.position());
		size_t index = 0;
		for (RGBf& ct : img.data) {
			RGB cs = rgb[index++];
			ct = RGBf(cs.x / 255.0f, cs.y / 255.0f, cs.z / 255.0f);
		}
	}
}
void ReadImageFromFile(const std::string& file, Image1f& img) {
	std::string ext = GetFileExtension(file);
	if (ext == "exr") {
		const char *message = nullptr;
		EXRImage exrImage;
		InitEXRImage(&exrImage);
		int ret = ParseMultiChannelEXRHeaderFromFile(&exrImage, file.c_str(),
				&message);
		if (ret != 0) {
			throw std::runtime_error(message);
		}
		for (int i = 0; i < exrImage.num_channels; i++) {
			if (exrImage.pixel_types[i] == TINYEXR_PIXELTYPE_HALF) {
				exrImage.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
			}
		}
		ret = LoadMultiChannelEXRFromFile(&exrImage, file.c_str(), &message);
		if (ret != 0) {
			throw std::runtime_error(message);
		}
		img.resize(exrImage.width, exrImage.height);
		std::vector<float> imageData(img.size() * img.channels);
		float** ptr = reinterpret_cast<float **>(exrImage.images);
		for (int c = 0; c < img.channels; c++) {
			size_t index = 0;
			for (float1& val : img.data) {
				val[c] = ptr[c][index++];
			}
		}
		FreeEXRImage(&exrImage);
	} else if (ext == "hdr") {
		int w, h, n;
		float *data = stbi_loadf(file.c_str(), &w, &h, &n, 1);
		if (data == NULL) {
			throw std::runtime_error(
					MakeString() << "Could not read file " << file);
		}
		img.resize(w, h);
		img.set(data);
		stbi_image_free(data);
	} else if(ext=="tiff"||ext=="tif"){
		ReadTiffImageFromFileInternal(file,img);
	} else {
		Image1ub rgb;
		ReadImageFromFile(file, rgb);
		img.resize(rgb.width, rgb.height);
		img.id = rgb.id;
		img.setPosition(rgb.position());
		size_t index = 0;
		for (float1& ct : img.data) {
			ubyte1 cs = rgb[index++];
			ct.x = cs.x / 255.0f;
		}
	}
}
void ReadImageFromFile(const std::string& file, Image2f& img) {
	throw std::runtime_error("Reading two channel float images unsupported");
}
void WriteImageToFile(const std::string& file,const Image2f& img) {
	WriteImageToRawFile(GetFileWithoutExtension(file)+".xml",img);
}
void ReadImageFromFile(const std::string& file, Image2ub& img) {
	throw std::runtime_error("Reading two channel ubyte images unsupported");
}
void WriteImageToFile(const std::string& file,const Image2ub& img) {
	WriteImageToRawFile(GetFileWithoutExtension(file)+".xml",img);
}
void WriteImageToFile(const std::string& file, const ImageRGBAf& img) {
	std::string ext = GetFileExtension(file);
	if (ext == "xml") {
		WriteImageToRawFile(file, img);
	} else if (ext == "exr") {
		const char* err;
		EXRImage exrImage;
		InitEXRImage(&exrImage);
		exrImage.num_channels = img.channels;
		exrImage.width = img.width;
		exrImage.height = img.height;
		exrImage.pixel_types = (int *) malloc(sizeof(int *) * img.channels);
		exrImage.requested_pixel_types = (int *) malloc(
				sizeof(int *) * img.channels);
		exrImage.channel_names = (const char **) malloc(
				sizeof(const char *) * img.channels);
		float** array = (float**) malloc(sizeof(float*) * img.channels);
		for (int c = 0; c < img.channels; c++) {
			array[c] = (float*) malloc(sizeof(float) * img.size());
		}
		exrImage.images = (unsigned char**) array;
		for (int c = 0; c < img.channels; c++) {
			size_t index = 0;
			for (const float4& val : img.data) {
				array[img.channels - 1 - c][index++] = val[c];
			}
		}
		for (int c = 0; c < img.channels; c++) {
			exrImage.pixel_types[c] = TINYEXR_PIXELTYPE_FLOAT;
			exrImage.requested_pixel_types[c] = TINYEXR_PIXELTYPE_FLOAT;
		}
#ifdef _WIN32
		exrImage.channel_names[0] = _strdup("A");
		exrImage.channel_names[1] = _strdup("B");
		exrImage.channel_names[2] = _strdup("G");
		exrImage.channel_names[3] = _strdup("R");
#else
		exrImage.channel_names[0] = strdup("A");
		exrImage.channel_names[1] = strdup("B");
		exrImage.channel_names[2] = strdup("G");
		exrImage.channel_names[3] = strdup("R");
#endif
		int ret = SaveMultiChannelEXRToFile(&exrImage, file.c_str(), &err);
		FreeEXRImage(&exrImage);
		if (ret != 0)
			throw std::runtime_error(err);
	} else if (ext == "hdr") {
		if (!stbi_write_hdr(file.c_str(), img.width, img.height, 4,
				img.ptr())) {
			throw std::runtime_error(
					MakeString() << "Could not write " << file);
		}
	} else {
		ImageRGBA rgb;
		rgb.resize(img.width, img.height);
		rgb.id = img.id;
		rgb.setPosition(img.position());
		size_t index = 0;
		for (const RGBAf& ct : img.data) {
			rgb[index++] = RGBA(clamp((int) (ct.x * 255.0f), 0, 255),
					clamp((int) (ct.y * 255.0f), 0, 255),
					clamp((int) (ct.z * 255.0f), 0, 255),
					clamp((int) (ct.w * 255.0f), 0, 255));
		}
		WriteImageToFile(file, rgb);
	}
}
void WriteImageToFile(const std::string& file, const Image1f& img) {
	std::string ext = GetFileExtension(file);
	if (ext == "xml") {
		WriteImageToRawFile(file, img);
	} else if (ext == "exr") {
		const char* err;
		EXRImage exrImage;
		InitEXRImage(&exrImage);
		exrImage.num_channels = img.channels;
		exrImage.width = img.width;
		exrImage.height = img.height;
		exrImage.pixel_types = (int *) malloc(sizeof(int *) * img.channels);
		exrImage.requested_pixel_types = (int *) malloc(
				sizeof(int *) * img.channels);
		exrImage.channel_names = (const char **) malloc(
				sizeof(const char *) * img.channels);
		float** array = (float**) malloc(sizeof(float*) * img.channels);
		for (int c = 0; c < img.channels; c++) {
			array[c] = (float*) malloc(sizeof(float) * img.size());
		}
		exrImage.images = (unsigned char**) array;
		for (int c = 0; c < img.channels; c++) {
			size_t index = 0;
			for (const float1& val : img.data) {
				array[c][index++] = val[c];
			}
		}
		for (int c = 0; c < img.channels; c++) {
			exrImage.pixel_types[c] = TINYEXR_PIXELTYPE_FLOAT;
			exrImage.requested_pixel_types[c] = TINYEXR_PIXELTYPE_FLOAT;
		}
#ifdef _WIN32
		exrImage.channel_names[0] = _strdup("R");
#else
		exrImage.channel_names[0] = strdup("R");
#endif
		int ret = SaveMultiChannelEXRToFile(&exrImage, file.c_str(), &err);
		FreeEXRImage(&exrImage);
		if (ret != 0)
			throw std::runtime_error(err);
	} else if (ext == "hdr") {
		if (!stbi_write_hdr(file.c_str(), img.width, img.height, 1,
				img.ptr())) {
			throw std::runtime_error(
					MakeString() << "Could not write " << file);
		}
	} else if (ext == "tiff"||ext=="tif") {
		Image1ub tmp;
		ConvertImage(img,tmp);
		WriteTiffImageToFileInternal(file,tmp);
	}else {
		Image1ub rgb;
		rgb.resize(img.width, img.height);
		rgb.id = img.id;
		rgb.setPosition(img.position());
		size_t index = 0;
		for (const float1& ct : img.data) {
			rgb[index++].x = clamp((int) (ct.x * 255.0f), 0, 255);
		}
		WriteImageToFile(file, rgb);
	}
}

void WriteImageToFile(const std::string& file, const Image1us& img){
	std::string ext = GetFileExtension(file);
	if (ext == "tif" || ext == "tiff") {
		WriteTiffImageToFileInternal(file,img);
	} else {
		throw std::runtime_error(MakeString() << "Could not write " << file);
	}
}
void WriteImageToFile(const std::string& file, const Image2us& img){
	std::string ext = GetFileExtension(file);
	if (ext == "tif" || ext == "tiff") {
		WriteTiffImageToFileInternal(file,img);
	} else {
		throw std::runtime_error(MakeString() << "Could not write " << file);
	}
}
void WriteImageToFile(const std::string& file, const Image3us& img){
	std::string ext = GetFileExtension(file);
	if (ext == "tif" || ext == "tiff") {
		WriteTiffImageToFileInternal(file,img);
	} else {
		throw std::runtime_error(MakeString() << "Could not write " << file);
	}
}
void WriteImageToFile(const std::string& file, const Image4us& img){
	std::string ext = GetFileExtension(file);
	if (ext == "tif" || ext == "tiff") {
		WriteTiffImageToFileInternal(file,img);
	} else {
		throw std::runtime_error(MakeString() << "Could not write " << file);
	}
}

void ReadImageFromFile(const std::string& file, Image1us& img){
	std::string ext = GetFileExtension(file);
	if (ext == "tif" || ext == "tiff") {
		ReadTiffImageFromFileInternal(file,img);
	} else {
		throw std::runtime_error(MakeString() << "Could not read " << file);
	}
}
void ReadImageFromFile(const std::string& file, Image2us& img){
	std::string ext = GetFileExtension(file);
	if (ext == "tif" || ext == "tiff") {
		ReadTiffImageFromFileInternal(file,img);
	} else {
		throw std::runtime_error(MakeString() << "Could not read " << file);
	}
}
void ReadImageFromFile(const std::string& file, Image3us& img){
	std::string ext = GetFileExtension(file);
	if (ext == "tif" || ext == "tiff") {
		ReadTiffImageFromFileInternal(file,img);
	} else {
		throw std::runtime_error(MakeString() << "Could not read " << file);
	}
}
void ReadImageFromFile(const std::string& file, Image4us& img){
	std::string ext = GetFileExtension(file);
	if (ext == "tif" || ext == "tiff") {
		ReadTiffImageFromFileInternal(file,img);
	} else {
		throw std::runtime_error(MakeString() << "Could not read " << file);
	}
}

void WriteImageToFile(const std::string& file, const ImageRGBf& img) {
	std::string ext = GetFileExtension(file);
	if (ext == "xml") {
		WriteImageToRawFile(file, img);
	} else if (ext == "exr") {
		const char* err;
		EXRImage exrImage;
		InitEXRImage(&exrImage);
		exrImage.num_channels = img.channels;
		exrImage.width = img.width;
		exrImage.height = img.height;
		exrImage.pixel_types = (int *) malloc(sizeof(int *) * img.channels);
		exrImage.requested_pixel_types = (int *) malloc(
				sizeof(int *) * img.channels);
		exrImage.channel_names = (const char **) malloc(
				sizeof(const char *) * img.channels);
		float** array = (float**) malloc(sizeof(float*) * img.channels);
		for (int c = 0; c < img.channels; c++) {
			array[c] = (float*) malloc(sizeof(float) * img.size());
		}
		exrImage.images = (unsigned char**) array;
		for (int c = 0; c < img.channels; c++) {
			size_t index = 0;
			for (const float3& val : img.data) {
				array[img.channels - 1 - c][index++] = val[c];
			}
		}
		for (int c = 0; c < img.channels; c++) {
			exrImage.pixel_types[c] = TINYEXR_PIXELTYPE_FLOAT;
			exrImage.requested_pixel_types[c] = TINYEXR_PIXELTYPE_FLOAT;
		}
#ifdef _WIN32
		exrImage.channel_names[0] = _strdup("B");
		exrImage.channel_names[1] = _strdup("G");
		exrImage.channel_names[2] = _strdup("R");
#else
		exrImage.channel_names[0] = strdup("B");
		exrImage.channel_names[1] = strdup("G");
		exrImage.channel_names[2] = strdup("R");
#endif
		int ret = SaveMultiChannelEXRToFile(&exrImage, file.c_str(), &err);
		FreeEXRImage(&exrImage);
		if (ret != 0)
			throw std::runtime_error(err);
	} else if (ext == "hdr") {
		if (!stbi_write_hdr(file.c_str(), img.width, img.height, 3,
				img.ptr())) {
			throw std::runtime_error(
					MakeString() << "Could not write " << file);
		}
	} else {
		ImageRGB rgb;
		rgb.resize(img.width, img.height);
		rgb.id = img.id;
		rgb.setPosition(img.position());
		size_t index = 0;
		for (const RGBf& ct : img.data) {
			rgb[index++] = aly::ubyte3(clamp((int) (ct.x * 255.0f), 0, 255),
					clamp((int) (ct.y * 255.0f), 0, 255),
					clamp((int) (ct.z * 255.0f), 0, 255));
		}
		WriteImageToFile(file, rgb);
	}
}

void ConvertImage(const ImageRGBAf& in, ImageRGB& out) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t index = 0;
	for (const RGBAf& ct : in.data) {
		out[index++] = aly::ubyte3(clamp((int) (ct.x * 255.0f), 0, 255),
				clamp((int) (ct.y * 255.0f), 0, 255),
				clamp((int) (ct.z * 255.0f), 0, 255));
	}
}

void ConvertImage(const ImageRGBA& in, ImageRGBf& out) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t index = 0;
	for (const RGBA& ct : in.data) {
		out[index++] = RGBf(ct.x / 255.0f, ct.y / 255.0f, ct.z / 255.0f);
	}
}
void ConvertImage(const Image1ub& in, Image1us& out){
	out.resize(in.width,in.height);
	size_t index=0;
	for(ubyte1& b:in.data){
		out[index++].x=((unsigned int)b.x)<<8;
	}
}
void ConvertImage(const Image1us& in, Image1f& out){
	out.resize(in.width,in.height);
	size_t index=0;
	for(ushort1& b:in.data){
		out[index++].x=b.x;
	}
}
void ConvertImage(const Image1ub& in, Image1f& out){
	out.resize(in.width,in.height);
	size_t index=0;
	for(ubyte1& b:in.data){
		out[index++].x=b.x/255.0f;
	}
}
void ConvertImage(const Image1f& in, Image1ub& out){
	out.resize(in.width,in.height);
	size_t index=0;
	for(float1& b:in.data){
		out[index++].x=clamp((int)(b.x*255.0f),0,255);
	}
}
void ConvertImage(const Image1f& in, Image1us& out){
	out.resize(in.width,in.height);
	size_t index=0;
	for(float1& b:in.data){
		out[index++].x=clamp((int)b.x,(int)0,(int)65535);
	}
}
void ConvertImage(const Image2ub& in, Image2us& out){
	out.resize(in.width,in.height);
	size_t index=0;
	for(ubyte2& b:in.data){
		out[index++]=ushort2(((unsigned int)b.x)<<8,((unsigned int)b.y)<<8);

	}
}
void ConvertImage(const Image3ub& in, Image3us& out){
	out.resize(in.width,in.height);
	size_t index=0;
	for(ubyte3& b:in.data){
		out[index++]=ushort3(((unsigned int)b.x)<<8,((unsigned int)b.y)<<8,((unsigned int)b.z)<<8);

	}
}
void ConvertImage(const Image4ub& in, Image4us& out){
	out.resize(in.width,in.height);
	size_t index=0;
	for(ubyte4& b:in.data){
		out[index++]=ushort4(((unsigned int)b.x)<<8,((unsigned int)b.y)<<8,((unsigned int)b.z)<<8,((unsigned int)b.w)<<8);

	}
}
void ConvertImage(const ImageRGBf& in, ImageRGBA& out) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t index = 0;
	for (const RGBf& ct : in.data) {
		out[index++] = RGBA(clamp((int) (ct.x * 255.0f), 0, 255),
				clamp((int) (ct.y * 255.0f), 0, 255),
				clamp((int) (ct.z * 255.0f), 0, 255),255);
	}
}

void ConvertImage(const ImageRGB& in, ImageRGBAf& out) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t index = 0;
	for (const RGB& ct : in.data) {
		out[index++] = RGBAf(ct.x / 255.0f, ct.y / 255.0f, ct.z / 255.0f,1.0f);
	}
}

void ConvertImage(const ImageRGBf& in, ImageRGB& out) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t index = 0;
	for (const RGBf& ct : in.data) {
		out[index++] = aly::ubyte3(clamp((int) (ct.x * 255.0f), 0, 255),
				clamp((int) (ct.y * 255.0f), 0, 255),
				clamp((int) (ct.z * 255.0f), 0, 255));
	}
}
void ConvertImage(const ImageRGBAf& in, ImageRGBA& out) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t index = 0;
	for (const RGBAf& ct : in.data) {
		out[index++] = RGBA(clamp((int) (ct.x * 255.0f), 0, 255),
				clamp((int) (ct.y * 255.0f), 0, 255),
				clamp((int) (ct.z * 255.0f), 0, 255),
				clamp((int) (ct.w * 255.0f), 0, 255));
	}
}
void ConvertImage(const ImageRGBA& in, ImageRGBAf& out) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t index = 0;
	for (RGBAf& ct : out.data) {
		RGBA cs = in[index++];
		ct = RGBAf(cs.x / 255.0f, cs.y / 255.0f, cs.z / 255.0f, cs.w / 255.0f);
	}
}
void ConvertImage(const ImageRGB& in, ImageRGBf& out) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t index = 0;
	for (RGBf& ct : out.data) {
		RGB cs = in[index++];
		ct = RGBf(cs.x / 255.0f, cs.y / 255.0f, cs.z / 255.0f);
	}
}
void ConvertImage(const ImageRGB& in, ImageRGBA& out) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t index = 0;
	for (RGBA& ct : out.data) {
		RGB cs = in[index++];
		ct = RGBA(cs, 255);
	}
}
void ConvertImage(const ImageRGBf& in, ImageRGBAf& out) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t index = 0;
	for (RGBAf& ct : out.data) {
		RGBf cs = in[index++];
		ct = RGBAf(cs, 1.0f);
	}
}
void ConvertImage(const ImageRGBAf& in, ImageRGBf& out) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t index = 0;
	for (RGBf& ct : out.data) {
		ct = in[index++].xyz();
	}
}
void ConvertImage(const ImageRGBA& in, ImageRGB& out) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	size_t index = 0;
	for (RGB& ct : out.data) {
		ct = in[index++].xyz();
	}
}

}
