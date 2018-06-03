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
	YUV_YUV,
	YUV_UYVY,
	YUV_YUYV,
	YUV_YYUV
};
enum class YUVScale
{
	SCALE_H2V2,
	SCALE_H2V1,
	SCALE_H1V2,
	SCALE_H1V1
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
	bool uvInterleave;			/* if not zero, UV rows in planar images are interleaved*/
	bool uvOrderSwap;				/*Swap UV order*/
	YUVFormat yuvFormat;			/* YUV output mode. Default: h2v2 */
	YUVScale uvScale;			/* Defines how UV components are scaled in planar mode */
	YUVMatrix uvMatrix;  		/*matrix type*/
	std::string inFileNamePattern;
	std::string outFileNamePattern;
	std::vector<float>yuvMatrix;
	YUVConverter();
	YUVConverter(YUVMatrix mat,YUVFormat format,YUVScale scale,bool orderSwap,bool interleave);
	YUVConverter(YUVType type,YUVMatrix mat=YUVMatrix::UNSPECIFIED,bool interleave=false);
	void evaluate(const ImageRGB& image,std::vector<uint8_t>& out);
protected:
	void setJPEGMatrix();
	void setSDTVMatrix();
	void setHDTVMatrix();
	void setMatrix();
};

}



#endif /* INCLUDE_CORE_ALLOYIMAGEENCODER_H_ */
