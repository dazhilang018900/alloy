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

namespace aly {
void AnisotropicDiffusion(const ImageRGBAf& imageIn, ImageRGBAf& out,
		int iterations, float K, float dt) {
	const int M = 3;
	const int N = 3;
	float kernelGX[M][N];
	float kernelGY[M][N];
	float kernelL[M][N];
	float sigma = 1.2f;
	GaussianKernelDerivative(kernelGX, kernelGY, sigma, sigma);
	GaussianKernelLaplacian(kernelL, sigma, sigma);
	aly::ImageRGBAf imageGx(imageIn.width, imageIn.height);
	aly::ImageRGBAf imageGy(imageIn.width, imageIn.height);
	aly::ImageRGBAf imageL(imageIn.width, imageIn.height);
	aly::ImageRGBAf imageC(imageIn.width, imageIn.height);
	out = imageIn;
	for (int iter = 0; iter < iterations; iter++) {
#pragma omp parallel for
		for (int j = 0; j < imageIn.height; j++) {
			for (int i = 0; i < imageIn.width; i++) {
				RGBAf gX(0.0f);
				RGBAf gY(0.0f);
				RGBAf L(0.0f);
				for (int ii = 0; ii < M; ii++) {
					for (int jj = 0; jj < N; jj++) {
						RGBAf val = out((int) (i + ii - M / 2),
								(int) (j + jj - N / 2));
						gX += kernelGX[ii][jj] * val;
						gY += kernelGY[ii][jj] * val;
						L += kernelL[ii][jj] * val;
					}
				}
				imageGx(i, j) = gX;
				imageGy(i, j) = gY;
				imageL(i, j) = L;
				RGBAf score(0.0f);
				RGBAf mag = gX * gX + gY * gY;
				for (int n = 0; n < 4; n++) {
					score[n] = (float)std::exp(-std::max(mag[n],0.0f) / (K * K));
				}
				imageC(i, j) = score;
			}
		}
		//WriteImageToFile(MakeString()<<GetDesktopDirectory()<<ALY_PATH_SEPARATOR<<"out"<<iter<<".png",out);
		//WriteImageToFile(MakeString()<<GetDesktopDirectory()<<ALY_PATH_SEPARATOR<<"C"<<iter<<".png",imageC);
#pragma omp parallel for
		for (int j = 0; j < imageIn.height; j++) {
			for (int i = 0; i < imageIn.width; i++) {
				RGBAf gX = imageGx(i, j);
				RGBAf gY = imageGy(i, j);
				RGBAf L = imageL(i, j);
				RGBAf C = imageC(i, j);
				RGBAf cX = imageC(i + 1, j) - imageC(i - 1, j);
				RGBAf cY = imageC(i, j + 1) - imageC(i, j - 1);
				RGBAf c = dt * (cX * gX + cY * gY + C * L);
				out(i, j) =out(i, j)+ c;
			}
		}
	}
}
}

