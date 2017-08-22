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
#include <AlloySparseMatrix.h>
#include <AlloySparseSolve.h>
namespace aly {
template<int C> void AnisotropicDiffusionT(
		const Image<float, C, ImageType::FLOAT>& imageIn,
		Image<float, C, ImageType::FLOAT>& out, int iterations,
		const AnisotropicKernel& kernel, float K, float dt) {
	//T=dt*iterations = 0.5*sigma*sigma where sigma refers to equivalent gaussian filter
	//SIFT uses sigmas separated by 2^1/S where S is the total number of scales
	const int M = 3;
	const int N = 3;
	const float sigma = 1.2f;
	float kernelGX[M][N];
	float kernelGY[M][N];
	float kernelL[M][N];

	GaussianKernelDerivative(kernelGX, kernelGY, sigma, sigma);
	GaussianKernelLaplacian(kernelL, sigma, sigma);
	aly::Image<float, C, ImageType::FLOAT> imageGx(imageIn.width,
			imageIn.height);
	aly::Image<float, C, ImageType::FLOAT> imageGy(imageIn.width,
			imageIn.height);
	aly::Image<float, C, ImageType::FLOAT> imageL(imageIn.width,
			imageIn.height);
	aly::Image<float, C, ImageType::FLOAT> imageC(imageIn.width,
			imageIn.height);
	out = imageIn;
	const float ZERO_TOLERANCE = 1E-6f;
	int R = (int) (imageIn.width * imageIn.height);
	aly::SparseMatrix<float,C> A(R, R);
	Vector<float,C> b;
	for (int iter = 0; iter < iterations; iter++) {
#pragma omp parallel for
		for (int j = 0; j < imageIn.height; j++) {
			for (int i = 0; i < imageIn.width; i++) {
				vec<float, C> gX(0.0f);
				vec<float, C> gY(0.0f);
				vec<float, C> L(0.0f);
				for (int ii = 0; ii < M; ii++) {
					for (int jj = 0; jj < N; jj++) {
						vec<float, C> val = out((int) (i + ii - M / 2),
								(int) (j + jj - N / 2));
						gX += kernelGX[ii][jj] * val;
						gY += kernelGY[ii][jj] * val;
						L += kernelL[ii][jj] * val;
					}
				}
				imageGx(i, j) = gX;
				imageGy(i, j) = gY;
				imageL(i, j) = L;
				vec<float, C> score(0.0f);
				if (kernel == AnisotropicKernel::Gaussian) {
					vec<float, C> mag = gX * gX + gY * gY;
					for (int n = 0; n < C; n++) {
						score[n] = (float) std::exp(
								-std::max(mag[n], 0.0f) / (K * K));
					}
				} else if (kernel == AnisotropicKernel::PeronaMalik) {
					vec<float, C> mag = gX * gX + gY * gY;
					for (int n = 0; n < C; n++) {
						score[n] = (float) 1.0f
								/ std::max(ZERO_TOLERANCE,
										1 + mag[n] / (K * K));
					}
				} else if (kernel == AnisotropicKernel::Weickert) {	//Used in KAZE filters
					vec<float, C> mag = gX * gX + gY * gY;
					for (int n = 0; n < C; n++) {
						double det = std::pow(std::sqrt(mag[n]) / K, 8.0);
						if (det <= ZERO_TOLERANCE) {
							score[n] = 1.0f;
						} else {
							score[n] = 1.0f - (float)std::exp(-3.315 / det);
						}
					}
				}
				imageC(i, j) = score;
			}
		}
		for (int j = 0; j < imageIn.height ; j++) {
			for (int i = 0; i < imageIn.width ; i++) {
				int n11 = i + j * imageIn.width;
				int n01 = (i - 1) + j * imageIn.width;
				int n21 = (i + 1) + j * imageIn.width;
				int n10 = i + (j - 1) * imageIn.width;
				int n12 = i + (j + 1) * imageIn.width;
				vec<float, C> score=imageC(i, j);
				//vec<float, C> cX=0.5f*(imageC(i+1,j)-imageC(i-1,j));
				//vec<float, C> cY=0.5f*(imageC(i,j+1)-imageC(i,j-1));
				out(i, j) = clamp(out(i, j), vec<float, C>(0.0f), vec<float, C>(1.0f));
				vec<float, C> cX(0.0f);
				vec<float, C> cY(0.0f);
				for (int ii = 0; ii < M; ii++) {
					for (int jj = 0; jj < N; jj++) {
						vec<float, C> val = imageC((int) (i + ii - M / 2),(int) (j + jj - N / 2));
						cX += kernelGX[ii][jj] * val;
						cY += kernelGY[ii][jj] * val;
					}
				}

				if(i>0&&j>0&&i<imageIn.width-1&&j<imageIn.height-1){
					A.set(n11, n11,   score*dt+1.0f);
					A.set(n11, n01,  -(-0.5f*cX+0.25f*score)*dt);
					A.set(n11, n21,  -( 0.5f*cX+0.25f*score)*dt);
					A.set(n11, n10,  -(-0.5f*cY+0.25f*score)*dt);
					A.set(n11, n12,  -( 0.5f*cY+0.25f*score)*dt);
				} else {
					A.set(n11, n11, 1.0f);
				}
			}
		}
		b=out.vector;
		SolveVecCG(b,A,out.vector,32, 1E-16f);
	}
	for (int j = 0; j < imageIn.height ; j++) {
		for (int i = 0; i < imageIn.width ; i++) {
			out(i, j) = clamp(out(i, j), vec<float, C>(0.0f), vec<float, C>(1.0f));
		}
	}
}

void AnisotropicDiffusion(const Image1f& imageIn, Image1f& out, int iterations,
	const AnisotropicKernel& kernel, float K, float dt) {
AnisotropicDiffusionT(imageIn, out, iterations, kernel, K, dt);
}
void AnisotropicDiffusion(const Image2f& imageIn, Image2f& out, int iterations,
	const AnisotropicKernel& kernel, float K, float dt) {
AnisotropicDiffusionT(imageIn, out, iterations, kernel, K, dt);
}
void AnisotropicDiffusion(const Image3f& imageIn, Image3f& out, int iterations,
	const AnisotropicKernel& kernel, float K, float dt) {
AnisotropicDiffusionT(imageIn, out, iterations, kernel, K, dt);
}
void AnisotropicDiffusion(const Image4f& imageIn, Image4f& out, int iterations,
	const AnisotropicKernel& kernel, float K, float dt) {
AnisotropicDiffusionT(imageIn, out, iterations, kernel, K, dt);
}

}

