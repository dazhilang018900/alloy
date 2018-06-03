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

*/

#ifndef INCLUDE_CORE_ALLOYIMAGEENCODER_H_
#define INCLUDE_CORE_ALLOYIMAGEENCODER_H_
#include <AlloyImage.h>
namespace aly {
enum class YUVFormat
{
	YUV,
	YUV_INTERLEAVE,
	UYVY,
	YUYV,
	YYUV
};
enum class YUVScale
{
	H2V2,
	H2V1,
	H1V2,
	H1V1
};
enum class YUVMatrix
{
	UNSPECIFIED,
	JPEG,
	SDTV,
	HDTV
};
enum class YUVType{
	I420,
	YV12,
	YV16,
	NV12,
	YUY2,
	YVYU,
	UYVY
};
class YUVConverter
{
public:
	YUVConverter();
	YUVConverter(YUVMatrix mat,YUVFormat format,YUVScale scale,bool orderSwap);
	YUVConverter(YUVType type,YUVMatrix mat=YUVMatrix::UNSPECIFIED);
	void evaluate(const ImageRGB& image,std::vector<uint8_t>& out);
protected:
	bool uvOrderSwap;				/*Swap UV order*/
	YUVFormat yuvFormat;			/* YUV output mode. Default: h2v2 */
	YUVScale uvScale;			/* Defines how UV components are scaled in planar mode */
	YUVMatrix uvMatrix;  		/*matrix type*/
	std::vector<float>yuvMatrix;
	void setJPEGMatrix();
	void setSDTVMatrix();
	void setHDTVMatrix();
	void setMatrix();
};

}



#endif /* INCLUDE_CORE_ALLOYIMAGEENCODER_H_ */
