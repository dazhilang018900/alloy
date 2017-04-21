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

#ifndef INCLUDE_ImageArray2dCL
#define INCLUDE_ImageArray2dCL
#include "MemoryCL.h"
#include <AlloyImage.h>
#include <array>
namespace aly {
class ImageArray2dCL: public MemoryCL {
protected:
	virtual void read(size_t index, void* data, bool block = true) const;
	virtual void write(size_t index, const void* data, bool block = true);
	int height;
	int width;
	int channels;
	int typeSize;
	int arraySize;
public:

	void create(cl_mem_flags f, const cl_image_format& formats, int w, int h,
			int c, int arraySize, int typeSize);
	void create(const cl_image_format& formats, int w, int h, int c,
			int arraySize, int typeSize) {
		create(formats, w, h, c, arraySize, typeSize);
	}
	template<class T, int C, ImageType I> void create(cl_mem_flags f,
			std::vector<Image<T, C, I>>& img) {
		cl_image_format format;
		switch (C) {
		case 1:
			format.image_channel_order = CL_R;
			break;
		case 2:
			format.image_channel_order = CL_RA;
			break;
		case 3:
			format.image_channel_order = CL_RGB;
			break;
		case 4:
			format.image_channel_order = CL_RGBA;
			break;
		default:
			break;
		}
		switch (I) {
		case ImageType::UBYTE:
			format.image_channel_data_type = CL_UNSIGNED_INT8;
			break;
		case ImageType::BYTE:
			format.image_channel_data_type = CL_SIGNED_INT8;
			break;
		case ImageType::FLOAT:
			format.image_channel_data_type = CL_FLOAT;
			break;
		case ImageType::USHORT:
			format.image_channel_data_type = CL_UNSIGNED_INT16;
			break;
		case ImageType::SHORT:
			format.image_channel_data_type = CL_SIGNED_INT16;
			break;
		case ImageType::UINT:
			format.image_channel_data_type = CL_UNSIGNED_INT32;
			break;
		case ImageType::INT:
			format.image_channel_data_type = CL_SIGNED_INT32;
			break;
		default:
			break;
		}
		create(f, format, img[0].width, img[0].height, img[0].channels,
				(int) img.size(), sizeof(T));
		for (int n = 0; n < (int) img.size(); n++) {
			write(n, img[n]);
		}
	}
	template<typename T> void read(size_t index, std::vector<T>& vec,
			bool block = true) const {
		read(index, &(vec[0]), block);
	}
	template<typename T> void write(size_t index, const std::vector<T>& vec,
			bool block = true) {
		write(index, &(vec[0]), block);
	}
	void write(size_t index,cl_mem mem);
	inline size_t getTypeSize() const {
		return typeSize;
	}
	inline int getWidth() const {
		return width;
	}
	inline int getHeight() const {
		return height;
	}
	inline int getChannels() const {
		return channels;
	}
	inline int getArraySize() const {
		return arraySize;
	}
	ImageArray2dCL();
	~ImageArray2dCL();
	template<class T, int M, ImageType I> void read(size_t index,
			Image<T, M, I>& vec, bool block = true) const {
		read(index, vec.data, block);
	}
	template<class T, int M, ImageType I> void write(size_t index,
			const Image<T, M, I>& vec, bool block = true) {
		write(index, vec.data, block);
	}
};
}
#endif
