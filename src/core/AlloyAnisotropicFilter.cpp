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
#include <AlloyAnisotropicFilter.h>
#include <AlloyImage.h>
namespace aly {
template <int C> void AnisotropicDiffusionT(const Image<float,C,ImageType::FLOAT>& imageIn, Image<float,C,ImageType::FLOAT>& out,int iterations, float K, float dt) {
	//T=dt*iterations = 0.5*sigma*sigma where sigma refers to equivalent gaussian filter
	const int M = 3;
	const int N = 3;
	const float sigma = 1.2f;
	float kernelGX[M][N];
	float kernelGY[M][N];
	float kernelL[M][N];

	GaussianKernelDerivative(kernelGX, kernelGY, sigma, sigma);
	GaussianKernelLaplacian(kernelL, sigma, sigma);
	aly::Image<float,C,ImageType::FLOAT> imageGx(imageIn.width, imageIn.height);
	aly::Image<float,C,ImageType::FLOAT> imageGy(imageIn.width, imageIn.height);
	aly::Image<float,C,ImageType::FLOAT> imageL(imageIn.width, imageIn.height);
	aly::Image<float,C,ImageType::FLOAT> imageC(imageIn.width, imageIn.height);
	out = imageIn;
	for (int iter = 0; iter < iterations; iter++) {
#pragma omp parallel for
		for (int j = 0; j < imageIn.height; j++) {
			for (int i = 0; i < imageIn.width; i++) {
				vec<float,C> gX(0.0f);
				vec<float,C> gY(0.0f);
				vec<float,C> L(0.0f);
				for (int ii = 0; ii < M; ii++) {
					for (int jj = 0; jj < N; jj++) {
						vec<float,C> val = out((int) (i + ii - M / 2),(int) (j + jj - N / 2));
						gX += kernelGX[ii][jj] * val;
						gY += kernelGY[ii][jj] * val;
						L += kernelL[ii][jj] * val;
					}
				}
				imageGx(i, j) = gX;
				imageGy(i, j) = gY;
				imageL(i, j) = L;
				vec<float,C> score(0.0f);
				vec<float,C> mag = gX * gX + gY * gY;
				for (int n = 0; n < C; n++) {
					score[n] = (float) std::exp(
							-std::max(mag[n], 0.0f) / (K * K));
				}
				imageC(i, j) = score;
			}
		}
#pragma omp parallel for
		for (int j = 0; j < imageIn.height; j++) {
			for (int i = 0; i < imageIn.width; i++) {
				vec<float,C> gX = imageGx(i, j);
				vec<float,C> gY = imageGy(i, j);
				vec<float,C> L = imageL(i, j);
				vec<float,C> score = imageC(i, j);
				vec<float,C> cX;
				vec<float,C> cY;
				for (int ii = 0; ii < M; ii++) {
					for (int jj = 0; jj < N; jj++) {
						vec<float,C> val = imageC((int) (i + ii - M / 2),(int) (j + jj - N / 2));
						cX += kernelGX[ii][jj] * val;
						cY += kernelGY[ii][jj] * val;
					}
				}
				out(i, j) +=  dt * (cX * gX + cY * gY + score * L);
			}
		}
	}
#pragma omp parallel for
	for (int j = 0; j < imageIn.height; j++) {
		for (int i = 0; i < imageIn.width; i++) {
			out(i, j) = clamp(out(i, j), vec<float,C>(0.0f), vec<float,C>(1.0f));
		}
	}
}

void AnisotropicDiffusion(const Image1f& imageIn,Image1f& out,int iterations,float K,float dt){
	AnisotropicDiffusionT(imageIn,out,iterations,K,dt);
}
void AnisotropicDiffusion(const Image2f& imageIn,Image2f& out,int iterations,float K,float dt){
	AnisotropicDiffusionT(imageIn,out,iterations,K,dt);
}
void AnisotropicDiffusion(const Image3f& imageIn,Image3f& out,int iterations,float K,float dt){
	AnisotropicDiffusionT(imageIn,out,iterations,K,dt);
}
void AnisotropicDiffusion(const Image4f& imageIn,Image4f& out,int iterations,float K,float dt){
	AnisotropicDiffusionT(imageIn,out,iterations,K,dt);
}

}

