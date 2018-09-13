/*
 * Copyright(C) 2018, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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

/*

 Implementation influenced by:

 2005-2015 Alexander Shashkevich <stunpix_gmail.com>
 https://github.com/stunpix/yuvit
 Source code is licensed under LGPLv3.

 Options to convert to popular YUV formats. All examples show YUV layout of resulting 4x4 image.

 -w Swap UV components order
 -i Interleave UV rows for planar formats
 -f Output YUV format
 -s Chroma Scaling
 ### Planar ###
 **I420, IYUV** : -f yuv -s h2v2

 YYYY
 YYYY
 YYYY
 YYYY
 UU
 UU
 VV
 VV

 **YV12 (MPEG codecs)** : -f yuv -s h2v2 -w

 YYYY
 YYYY
 YYYY
 YYYY
 VV
 VV
 UU
 UU

 **YV16** : -f yuv -s h2v1 -w

 YY
 YY
 V
 V
 U
 U

 **NV12** : -f yyuv -s h2v2

 YYYY
 YYYY
 YYYY
 YYYY
 UVUV
 UVUV

 ### Packed ###

 **YUY2, V422, YUYV** : -f yuyv

 YUYVYUYV
 YUYVYUYV
 YUYVYUYV
 YUYVYUYV

 **YVYU** : -f yuyv -w

 YVYUYVYU
 YVYUYVYU
 YVYUYVYU
 YVYUYVYU

 **UYVY** : -f uyvy

 UYVYUYVY
 UYVYUYVY
 UYVYUYVY
 UYVYUYVY


 */
#include "image/AlloyImageEncoder.h"
namespace aly {
void SANITY_CHECK_VIDEOENCODER() {
	ImageRGB img;
	ReadImageFromFile("/home/blake/workspace/studio/alloy/assets/images/sfmarket.png",img);
	{
		YUVConverter converter(YUVMatrix::JPEG, YUVFormat::YUV, YUVScale::H2V1,true);
		std::vector<uint8_t> buffer;
		converter.evaluate(img,buffer);
		std::cout<<"Image Size= "<<img.size()*img.typeSize()<<" Buffer Size= "<<buffer.size()<<std::endl;
	}
	{
		YUVConverter converter(YUVMatrix::JPEG, YUVFormat::YUV_INTERLEAVE, YUVScale::H2V1,false);
		std::vector<uint8_t> buffer;
		converter.evaluate(img,buffer);
		std::cout<<"Image Size= "<<img.size()*img.typeSize()<<" Buffer Size= "<<buffer.size()<<std::endl;
	}
	{
		YUVConverter converter(YUVType::I420);
		std::vector<uint8_t> buffer;
		converter.evaluate(img,buffer);
		std::cout<<"I420 Image Size= "<<img.size()*img.typeSize()<<" Buffer Size= "<<buffer.size()<<std::endl;
	}
	{
		YUVConverter converter(YUVType::YV12);
		std::vector<uint8_t> buffer;
		converter.evaluate(img,buffer);
		std::cout<<"YV12 Image Size= "<<img.size()*img.typeSize()<<" Buffer Size= "<<buffer.size()<<std::endl;
	}
	{
		YUVConverter converter(YUVType::NV12);
		std::vector<uint8_t> buffer;
		converter.evaluate(img,buffer);
		std::cout<<"NV12 Image Size= "<<img.size()*img.typeSize()<<" Buffer Size= "<<buffer.size()<<std::endl;
	}
	{
		YUVConverter converter(YUVType::UYVY);
		std::vector<uint8_t> buffer;
		converter.evaluate(img,buffer);
		std::cout<<"UYVY Image Size= "<<img.size()*img.typeSize()<<" Buffer Size= "<<buffer.size()<<std::endl;
	}
	{
		YUVConverter converter(YUVType::YUY2);
		std::vector<uint8_t> buffer;
		converter.evaluate(img,buffer);
		std::cout<<"YUY2 Image Size= "<<img.size()*img.typeSize()<<" Buffer Size= "<<buffer.size()<<std::endl;
	}
}

void YUVConverter::setMatrix() {
	switch (uvMatrix) {
	case YUVMatrix::JPEG:
		setJPEGMatrix();
		break;
	case YUVMatrix::HDTV:
		setHDTVMatrix();
		break;
	case YUVMatrix::SDTV:
		setSDTVMatrix();
		break;
	default:
		throw std::runtime_error("No YUV matrix specified.");
	}
}
void YUVConverter::evaluate(const ImageRGB& image, std::vector<uint8_t>& out) {
	uint32_t lumaWidth, lumaHeight;
	uint32_t chromaWidth, chromaHeight;
	uint32_t xMask, yMask;
	int x, y;
	uint8_t Rc, Gc, Bc;
	uint8_t *yPtr, *uPtr, *vPtr;
	bool warned = false;
	// For performance reasons get matrix values here to put them on stack
	// instead of accessing them in deep loops from vector
	float yr = yuvMatrix[0], yg = yuvMatrix[1], yb = yuvMatrix[2];
	float ur = yuvMatrix[3], ug = yuvMatrix[4], ub = yuvMatrix[5];
	float vr = yuvMatrix[6], vg = yuvMatrix[7], vb = yuvMatrix[8];
	lumaWidth = image.width;
	lumaHeight = image.height;
	switch (uvScale) {
	default: /* Default scale h1v1 */
	case YUVScale::H1V1:
		chromaWidth = lumaWidth;
		chromaHeight = lumaHeight;
		xMask = 0;
		yMask = 0;
		break;
	case YUVScale::H2V2:
		chromaWidth = lumaWidth / 2;
		chromaHeight = lumaHeight / 2;
		yMask = xMask = 1;
		break;
	case YUVScale::H1V2:
		chromaWidth = lumaWidth;
		chromaHeight = lumaHeight / 2;
		xMask = 0;
		yMask = 1;
		break;
	case YUVScale::H2V1:
		chromaWidth = lumaWidth / 2;
		chromaHeight = lumaHeight;
		xMask = 1;
		yMask = 0;
		break;
	}
	// Pointers that are always pointing on buffers
	std::vector<uint8_t> yPixels(lumaHeight * lumaWidth);
	std::vector<uint8_t> uPixels(chromaHeight * chromaWidth);
	std::vector<uint8_t> vPixels(chromaHeight * chromaWidth);
	// Pointers we are working with
	yPtr = yPixels.data();
	uPtr = uPixels.data();
	vPtr = vPixels.data();
	out.clear();
	for (y = 0; y < lumaHeight; y++) {
		for (x = 0; x < lumaWidth; x++) {
			aly::RGB c = image(x, y);
			Rc = c.x;
			Gc = c.y;
			Bc = c.z;
			*yPtr++ = uint8_t(Rc * yr + Gc * yg + Bc * yb);
			if ((y & yMask) == 0 && (x & xMask) == 0 && (y / 2) < chromaHeight
					&& (x / 2) < chromaWidth) {
				*uPtr++ = uint8_t(Rc * ur + Gc * ug + Bc * ub + 128);
				*vPtr++ = uint8_t(Rc * vr + Gc * vg + Bc * vb + 128);
			}
		}
	}
	if (uvOrderSwap) {	// UV components should be swapped
		std::swap(uPixels, vPixels);
	}
	yPtr = yPixels.data();
	uPtr = uPixels.data();
	vPtr = vPixels.data();
	if (yuvFormat == YUVFormat::YUV || yuvFormat == YUVFormat::YUV_INTERLEAVE) {// Writing planar image
		out.reserve(lumaHeight * lumaWidth + 2 * chromaWidth * chromaHeight);
		out.insert(out.end(), yPixels.begin(), yPixels.end());
		if (yuvFormat == YUVFormat::YUV_INTERLEAVE) {// U and V rows should be interleaved after each other
			while (chromaHeight--) {
				out.insert(out.end(), uPtr, uPtr + chromaWidth);// Write U line
				out.insert(out.end(), vPtr, vPtr + chromaWidth);// Write V line
				uPtr += chromaWidth;
				vPtr += chromaWidth;
			}
		} else {
			// Simply write U and V planes
			out.insert(out.end(), uPixels.begin(), uPixels.end());
			out.insert(out.end(), vPixels.begin(), vPixels.end());
		}
	} else if (yuvFormat == YUVFormat::YYUV) {	// Writing planar image
		out.reserve(lumaHeight * lumaWidth + 2 * chromaWidth * chromaHeight);
		out.insert(out.end(), yPixels.begin(), yPixels.end());
		// U and V columns should be interleaved after each other
		for (uint32_t row = 0; row < chromaHeight; row++) {
			for (uint32_t col = 0; col < chromaWidth; col++) {// Write in following order Y, U, Y, V
				out.push_back(*uPtr++);
				out.push_back(*vPtr++);
			}
		}
	} else {
		out.reserve(lumaHeight * (lumaWidth/2));
		// Writing packed image
		if (yuvFormat == YUVFormat::YUYV) {
			for (uint32_t row = 0; row < lumaHeight; row++) {
				for (uint32_t col = 0; col < lumaWidth; col += 2) {	// Write in following order Y, U, Y, V
					out.push_back(*yPtr++);
					out.push_back(*uPtr++);
					out.push_back(*yPtr++);
					out.push_back(*vPtr++);
				}
			}
		} else {
			for (uint32_t row = 0; row < lumaHeight; row++) {
				for (uint32_t col = 0; col < lumaWidth; col += 2) {	// Write in following order U, Y, V, Y
					out.push_back(*uPtr++);
					out.push_back(*yPtr++);
					out.push_back(*vPtr++);
					out.push_back(*yPtr++);
				}
			}
		}
	}
}
YUVConverter::YUVConverter() {
	uvOrderSwap = false; /* no UV order swap */
	yuvFormat = YUVFormat::YUV; /* YUV output mode. Default: h2v2 */
	uvScale = YUVScale::H1V1; /* UV scaling for planar mode. Default: h1v1 */
	uvMatrix = YUVMatrix::JPEG;
	setMatrix();
}
YUVConverter::YUVConverter(YUVMatrix mat, YUVFormat format, YUVScale scale,
		bool orderSwap) {
	uvOrderSwap = orderSwap;
	yuvFormat = format;
	uvScale = scale;
	uvMatrix = mat;
	setMatrix();
}
YUVConverter::YUVConverter(YUVType type, YUVMatrix mat) {
	switch (type) {
	case YUVType::I420:
		uvOrderSwap = false;
		yuvFormat = YUVFormat::YUV;
		uvScale = YUVScale::H2V2;
		uvMatrix = YUVMatrix::SDTV;
		break;
	case YUVType::YV12:
		uvOrderSwap = true;
		yuvFormat = YUVFormat::YUV;
		uvScale = YUVScale::H2V2;
		uvMatrix = YUVMatrix::SDTV;
		break;
	case YUVType::YV16:
		uvOrderSwap = true;
		yuvFormat = YUVFormat::YUV;
		uvScale = YUVScale::H2V1;
		uvMatrix = YUVMatrix::SDTV;
		break;
	case YUVType::NV12:
		uvOrderSwap = false;
		yuvFormat = YUVFormat::YYUV;
		uvScale = YUVScale::H2V2;
		uvMatrix = YUVMatrix::HDTV;
		break;
	case YUVType::YUY2:
		uvOrderSwap = false;
		yuvFormat = YUVFormat::YUYV;
		uvScale = YUVScale::H1V1;
		uvMatrix = YUVMatrix::JPEG;
		break;
	case YUVType::YVYU:
		uvOrderSwap = true;
		yuvFormat = YUVFormat::YUYV;
		uvScale = YUVScale::H1V1;
		uvMatrix = YUVMatrix::JPEG;
		break;
	case YUVType::UYVY:
		uvOrderSwap = false;
		yuvFormat = YUVFormat::UYVY;
		uvScale = YUVScale::H1V1;
		uvMatrix = YUVMatrix::JPEG;
		break;
	}
	if (mat != YUVMatrix::UNSPECIFIED) {
		uvMatrix = mat;
	}
	setMatrix();
}
void YUVConverter::setJPEGMatrix() {
	yuvMatrix = {0.299f, 0.587f, 0.114f,
		-0.168736f, -0.331264f, 0.5f,
		0.5f, -0.418688f, -0.081312f};
}

void YUVConverter::setSDTVMatrix() {
	yuvMatrix = {0.299f, 0.587f, 0.114f,
		-0.14713f, -0.28886f, 0.436f,
		0.615f, -0.51499f, -0.10001f};
}

void YUVConverter::setHDTVMatrix() {
	yuvMatrix = {0.2126f, 0.7152f, 0.0722f,
		-0.09991f, -0.33609f, 0.436f,
		0.615f, -0.55861f, -0.05639f};
}
}

