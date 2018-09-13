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

#ifndef INCLUDE_MAPPINGFIELD_H_
#define INCLUDE_MAPPINGFIELD_H_
#include "image/AlloyImage.h"
#include "image/AlloyVolume.h"
#include "math/AlloyVecMath.h"
namespace aly {
	void SolveEdgeFilter(const ImageRGB& img,Image1f& out,int K=1);
	void SolveEdgeFilter(const ImageRGBf& img,Image1f& out,int K=1);
	void SolveEdgeFilter(const Image1f& img,Image1f& out,int K=1);
	void SolveEdgeFilter(const Volume1f& img,Volume1f& out,int K=1);

	void SolveGradientVectorFlow(const Image1f& src, Image2f& vectorField, int iterations, bool normalize);
	void SolveGradientVectorFlow(const Image1f& src, Image2f& vectorField,float mu, int iterations, bool normalize);
	void SolveGradientVectorFlow(const Image1f& src, Image2f& vectorField,const Image1f& weights,float mu,int iterations,  bool normalize);

	void SolveGradientVectorFlow(const Volume1f& src, Volume3f& vectorField, int iterations, bool normalize);
	void SolveGradientVectorFlow(const Volume1f& src, Volume3f& vectorField,float mu, int iterations, bool normalize);
	void SolveGradientVectorFlow(const Volume1f& src, Volume3f& vectorField,const Volume1f& weights,float mu,int iterations,  bool normalize);

}
#endif
