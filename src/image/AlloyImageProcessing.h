/*
 * Copyright(C) 2015, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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
#ifndef INCLUDE_ALLOYIMAGEPROCESSING_H_
#define INCLUDE_ALLOYIMAGEPROCESSING_H_
#include "image/AlloyImage.h"
#include "graphics/AlloyCamera.h"
namespace aly {
bool SANITY_CHECK_IMAGE_PROCESSING();
enum BayerFilter {
	BGGR = 0, RGGB = 1, GBRG = 2, GRBG = 3
};
void Demosaic(const Image1ub& gray, ImageRGB& color,const BayerFilter& filter=BayerFilter::RGGB);
void Demosaic(const Image1ub& gray, ImageRGBf& color,const BayerFilter& filter=BayerFilter::RGGB);
void Undistort(const ImageRGBf& in,ImageRGBf& out,double fx,double fy,double cx,double cy,double k1,double k2,double k3,double p1,double p2);
void Undistort(const ImageRGB& in,ImageRGB& out,double fx,double fy,double cx,double cy,double k1,double k2,double k3,double p1,double p2);

template<class T, size_t M, size_t N> void GaussianKernel(T (&kernel)[M][N],
		T sigmaX = T(0.607902736 * (M - 1) * 0.5),
		T sigmaY = T(0.607902736 * (N - 1) * 0.5)) {
	T sum = 0;
#pragma omp parallel for reduction(+:sum)
	for (int i = 0; i < (int) M; i++) {
		for (int j = 0; j < (int) N; j++) {
			T x = T(i - 0.5 * (M - 1));
			T y = T(j - 0.5 * (N - 1));
			double xn = x / sigmaX;
			double yn = y / sigmaY;
			T w = T(std::exp(-0.5 * (xn * xn + yn * yn)));
			sum += w;
			kernel[i][j] = w;
		}
	}
	sum = T(1) / sum;
#pragma omp parallel for
	for (int i = 0; i < (int) M; i++) {
		for (int j = 0; j < (int) N; j++) {
			kernel[i][j] *= sum;
		}
	}
}
template<class T, size_t M> void GaussianKernel(T (&kernel)[M],
		T sigmaX = T(0.607902736 * (M - 1) * 0.5)) {
	T sum = 0;
#pragma omp parallel for reduction(+:sum)
	for (int i = 0; i < (int) M; i++) {
		T x = T(i - 0.5 * (M - 1));
		double xn = x / sigmaX;
		T w = T(std::exp(-0.5 * (xn * xn)));
		sum += w;
		kernel[i] = w;
	}
	sum = T(1) / sum;
#pragma omp parallel for
	for (int i = 0; i < (int) M; i++) {
		kernel[i] *= sum;
	}
}
template<class T, size_t M> void GaussianKernelDerivative(T (&gX)[M], T sigmaX = T(0.607902736 * (M - 1) * 0.5)) {
	T sum = 0;
#pragma omp parallel for reduction(+:sum)
	for (int i = 0; i < (int) M; i++) {
		T x = T(i - 0.5 * (M - 1));
		double xn = x / sigmaX;
		T w = T(std::exp(-0.5 * (xn * xn)));
		sum += w;
		gX[i] = (T) (w * xn / sigmaX);
	}
	sum = T(1) / sum;
#pragma omp parallel for
	for (int i = 0; i < (int) M; i++) {
		gX[i] *= sum;
	}
}
template<class T, size_t M> void GaussianKernelLaplacian(T (&kernel)[M],
		T sigmaX = T(0.607902736 * (M - 1) * 0.5)) {
	T sum = 0;
	T sum2 = 0;
	for (int i = 0; i < (int) M; i++) {
		T x = T(i - 0.5 * (M - 1));
		double xn = x / sigmaX;
		T w = T(std::exp(-0.5 * (xn * xn)));
		sum += w;
		T ww = (T) (w * (xn * xn / (sigmaX * sigmaX) - 1 / (sigmaX * sigmaX)));
		sum2 += ww;
		kernel[i] = ww;
	}
	sum = T(1) / sum;
	sum2 /= T(M);
	for (int i = 0; i < (int) M; i++) {
		kernel[i] = (kernel[i] - sum2) * sum;
	}
}
template<class T> void GaussianKernel(std::vector<T>& kernel, int M, int N,
		T sigmaX, T sigmaY) {
	T sum = 0;
	kernel.resize(M * N);
#pragma omp parallel for reduction(+:sum)
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			T x = T(i - 0.5 * (M - 1));
			T y = T(j - 0.5 * (N - 1));
			double xn = x / sigmaX;
			double yn = y / sigmaY;
			T w = T(std::exp(-0.5 * (xn * xn + yn * yn)));
			sum += w;
			kernel[i + M * j] = w;
		}
	}
	sum = T(1) / sum;
#pragma omp parallel for
	for (int i = 0; i < (int) M; i++) {
		for (int j = 0; j < (int) N; j++) {
			kernel[i + M * j] *= sum;
		}
	}
}
template<class T> void GaussianKernelDerivative(std::vector<T>& gX,
		std::vector<T>& gY, int M, int N, T sigmaX, T sigmaY) {
	T sum = 0;
	gX.resize(M * N);
	gY.resize(M * N);
#pragma omp parallel for reduction(+:sum)
	for (int i = 0; i < (int) M; i++) {
		for (int j = 0; j < (int) N; j++) {
			T x = T(i - 0.5 * (M - 1));
			T y = T(j - 0.5 * (N - 1));
			double xn = x / sigmaX;
			double yn = y / sigmaY;
			T w = T(std::exp(-0.5 * (xn * xn + yn * yn)));
			sum += w;
			gX[i + j * M] = (T) (w * xn / sigmaX);
			gY[i + j * M] = (T) (w * yn / sigmaY);
		}
	}
	sum = T(1) / sum;
#pragma omp parallel for
	for (int i = 0; i < (int) M; i++) {
		for (int j = 0; j < (int) N; j++) {
			gX[i + j * M] *= sum;
			gY[i + j * M] *= sum;
		}
	}
}
template<class T> void GaussianKernel(std::vector<T>& kernel, int M, T sigma) {
	kernel.resize(M);
	T sum = 0;
#pragma omp parallel for reduction(+:sum)
	for (int i = 0; i < (int) M; i++) {
		T x = T(i - 0.5 * (M - 1));
		double xn = x / sigma;
		T w = T(std::exp(-0.5 * (xn * xn)));
		sum += w;
		kernel[i] = w;
	}
	sum = T(1) / sum;
#pragma omp parallel for
	for (int i = 0; i < (int) M; i++) {
		kernel[i] *= sum;
	}
}
template<class T> void GaussianKernelDerivative(std::vector<T>& kernel, int M,
		T sigma) {
	T sum = 0;
	kernel.resize(M);
#pragma omp parallel for reduction(+:sum)
	for (int i = 0; i < (int) M; i++) {
		T x = T(i - 0.5 * (M - 1));
		double xn = x / sigma;
		T w = T(std::exp(-0.5 * (xn * xn)));
		sum += w;
		kernel[i] = (T) (w * xn / sigma);
	}
	sum = T(1) / sum;
#pragma omp parallel for
	for (int i = 0; i < (int) M; i++) {
		kernel[i] *= sum;
	}
}
template<class T> void GaussianKernelLaplacian(std::vector<T>& kernel, int M,
		T sigma) {
	T sum = 0;
	T sum2 = 0;
	kernel.resize(M);
	for (int i = 0; i < (int) M; i++) {
		T x = T(i - 0.5 * (M - 1));
		double xn = x / sigma;
		T w = T(std::exp(-0.5 * (xn * xn)));
		sum += w;
		T ww = (T) (w * (xn * xn / (sigma * sigma) - 1 / (sigma * sigma)));
		sum2 += ww;
		kernel[i] = ww;
	}
	sum = T(1) / sum;
	sum2 /= T(M);
	for (int i = 0; i < (int) M; i++) {
		kernel[i] = (kernel[i] - sum2) * sum;
	}
}
template<class T, size_t M, size_t N> void GaussianKernelDerivative(
		T (&gX)[M][N], T (&gY)[M][N], T sigmaX = T(0.607902736 * (M - 1) * 0.5),
		T sigmaY = T(0.607902736 * (N - 1) * 0.5)) {
	T sum = 0;
#pragma omp parallel for reduction(+:sum)
	for (int i = 0; i < (int) M; i++) {
		for (int j = 0; j < (int) N; j++) {
			T x = T(i - 0.5 * (M - 1));
			T y = T(j - 0.5 * (N - 1));
			double xn = x / sigmaX;
			double yn = y / sigmaY;
			T w = T(std::exp(-0.5 * (xn * xn + yn * yn)));
			sum += w;
			gX[i][j] = (T) (w * xn / sigmaX);
			gY[i][j] = (T) (w * yn / sigmaY);

		}
	}
	sum = T(1) / sum;
#pragma omp parallel for
	for (int i = 0; i < (int) M; i++) {
		for (int j = 0; j < (int) N; j++) {
			gX[i][j] *= sum;
			gY[i][j] *= sum;
		}
	}
}

template<class T, size_t M, size_t N> void GaussianKernelLaplacian(
		T (&kernel)[M][N], T sigmaX = T(0.607902736 * (M - 1) * 0.5), T sigmaY =
				T(0.607902736 * (N - 1) * 0.5)) {
	T sum = 0;
	T sum2 = 0;
	for (int i = 0; i < (int) M; i++) {
		for (int j = 0; j < (int) N; j++) {
			T x = T(i - 0.5 * (M - 1));
			T y = T(j - 0.5 * (N - 1));
			double xn = x / sigmaX;
			double yn = y / sigmaY;
			T w = T(std::exp(-0.5 * (xn * xn + yn * yn)));
			sum += w;
			T ww = (T) (w
					* (xn * xn / (sigmaX * sigmaX) + yn * yn / (sigmaY * sigmaY)
							- 1 / (sigmaX * sigmaX) - 1 / (sigmaY * sigmaY)));
			sum2 += ww;
			kernel[i][j] = ww;
		}
	}
	sum = T(1) / sum;
	sum2 /= T(M * N);
	for (int i = 0; i < (int) M; i++) {
		for (int j = 0; j < (int) N; j++) {
			kernel[i][j] = (kernel[i][j] - sum2) * sum;
		}
	}
}
template<class T, size_t M, size_t N> struct GaussianOperators {
	T filter[M][N];
	T filterGradX[M][N];
	T filterGradY[M][N];
	T filterLaplacian[M][N];
	GaussianOperators(T sigmaX = T(0.607902736 * (M - 1) * 0.5),
			T sigmaY = T(0.607902736 * (N - 1) * 0.5)) {
		T sum = 0;
		T sum2 = 0;
		for (int i = 0; i < (int) M; i++) {
			for (int j = 0; j < (int) N; j++) {
				T x = T(i - 0.5 * (M - 1));
				T y = T(j - 0.5 * (N - 1));
				double xn = x / sigmaX;
				double yn = y / sigmaY;
				T w = T(std::exp(-0.5 * (xn * xn + yn * yn)));
				filter[i][j] = w;
				filterGradX[i][j] = w * xn / sigmaX;
				filterGradY[i][j] = w * yn / sigmaY;
				sum += w;
				T ww =
						w
								* (xn * xn / (sigmaX * sigmaX)
										+ yn * yn / (sigmaY * sigmaY)
										- 1 / (sigmaX * sigmaX)
										- 1 / (sigmaY * sigmaY));
				sum2 += ww;
				filterLaplacian[i][j] = ww;
			}
		}
		sum = T(1) / sum;
		sum2 /= T(M * N);
		for (int i = 0; i < (int) M; i++) {
			for (int j = 0; j < (int) N; j++) {
				filterLaplacian[i][j] = (filterLaplacian[i][j] - sum2) * sum;
				filter[i][j] *= sum;
				filterGradX[i][j] *= sum;
				filterGradY[i][j] *= sum;
			}
		}
	}
};
template<size_t M, size_t N, class T, int C, ImageType I> void Gradient(
		const Image<T, C, I>& image, Image<T, C, I>& gX, Image<T, C, I>& gY,
		double sigmaX = (0.607902736 * (M - 1) * 0.5),
		double sigmaY = (0.607902736 * (N - 1) * 0.5)) {

	double filterX[M][N], filterY[M][N];
	GaussianKernelDerivative(filterX, filterY, sigmaX, sigmaY);

	gX.resize(image.width, image.height);
	gY.resize(image.width, image.height);
#pragma omp parallel for
	for (int j = 0; j < image.height; j++) {
		for (int i = 0; i < image.width; i++) {
			vec<double, C> vsumX(0.0);
			vec<double, C> vsumY(0.0);
			for (int ii = 0; ii < (int) M; ii++) {
				for (int jj = 0; jj < (int) N; jj++) {
					vec<T, C> val = image(i + ii - (int) M / 2,
							j + jj - (int) N / 2);
					vsumX += filterX[ii][jj] * vec<double, C>(val);
					vsumY += filterY[ii][jj] * vec<double, C>(val);
				}
			}
			gX(i, j) = vec<T, C>(vsumX);
			gY(i, j) = vec<T, C>(vsumY);
		}
	}
}
template<size_t M, size_t N, class T, int C, ImageType I> void Laplacian(
		const Image<T, C, I>& image, Image<T, C, I>& L,
		double sigmaX = (0.607902736 * (M - 1) * 0.5),
		double sigmaY = (0.607902736 * (N - 1) * 0.5)) {
	float filter[M][N];
	GaussianKernelLaplacian(filter, (float) sigmaX, (float) sigmaY);
	L.resize(image.width, image.height);
#pragma omp parallel for
	for (int j = 0; j < image.height; j++) {
		for (int i = 0; i < image.width; i++) {
			vec<float, C> vsum(0.0);
			for (int ii = 0; ii < (int) M; ii++) {
				for (int jj = 0; jj < (int) N; jj++) {
					vec<T, C> val = image(i + ii - (int) M / 2,
							j + jj - (int) N / 2);
					vsum += filter[ii][jj] * vec<float, C>(val);
				}
			}
			L(i, j) = vec<T, C>(vsum);
		}
	}
}

template<int C> void ConvolveHorizontal(const Image<float, C, ImageType::FLOAT>& input,Image<float, C, ImageType::FLOAT>& output,const std::vector<float>& filter) {
	const int w = input.width;
	const int h = input.height;
	const int hlen = filter.size();
	output.resize(w, h);
	int c, hL, hR;
	if (hlen & 1) { // odd kernel size
		c = hlen / 2;
		hL = c;
		hR = c;
	} else { // even kernel size : center is shifted to the left
		c = hlen / 2 - 1;
		hL = c;
		hR = c + 1;
	}
#pragma omp parallel for
	for (int j = 0; j < h; j++) {
		for (int i = 0; i < w; i++) {
			int jx1 = c - i;
			int jx2 = w - 1 - i + c;
			float sum = 0.0f;
			// Convolution with boundaries extension
			for (int jx = 0; jx <= hR + hL; jx++) {
				int idx_x = i - c + jx;
				if (jx < jx1)
					idx_x = jx1 - jx - 1;
				if (jx > jx2)
					idx_x = w - (jx - jx2);
				sum += input[j * w + idx_x] * filter[jx]; //symmetric kernel? Don't flip!
			}
			output[j * w + i] = sum;
		}
	}
}
template<int C> void ConvolveVertical(const Image<float, C, ImageType::FLOAT>& input,Image<float, C, ImageType::FLOAT>& output,const std::vector<float>& filter) {
	const int w = input.width;
	const int h = input.height;
	const int hlen = filter.size();
	output.resize(w, h);
	int c, hL, hR;
	if (hlen & 1) { // odd kernel size
		c = hlen / 2;
		hL = c;
		hR = c;
	} else { // even kernel size : center is shifted to the left
		c = hlen / 2 - 1;
		hL = c;
		hR = c + 1;
	}
#pragma omp parallel for
	for (int j = 0; j < h; j++) {
		for (int i = 0; i < w; i++) {
			int jy1 = c - j;
			int jy2 = h - 1 - j + c;
			float sum = 0.0f;
			// Convolution with boundaries extension
			for (int jy = 0; jy <= hR + hL; jy++) {
				int idx_y = j - c + jy;
				if (jy < jy1)
					idx_y = jy1 - jy - 1;
				if (jy > jy2)
					idx_y = h - (jy - jy2);
				sum += input[idx_y * w + i] * filter[jy]; //symmetric kernel? Don't flip!
			}
			output[j * w + i] = sum;
		}
	}
}
template<int C> void Convolve(const Image<float, C, ImageType::FLOAT>& image,
		Image<float, C, ImageType::FLOAT>& out,
		const std::vector<float>& filter, int M, int N) {
	int w = image.width;
	int h = image.height;
	out.resize(w, h);
#pragma omp parallel for
	for (int j = 0; j < image.height; j++) {
		for (int i = 0; i < image.width; i++) {
			vec<float, C> vsum(0.0);
			for (int ii = 0; ii < (int) M; ii++) {
				for (int jj = 0; jj < (int) N; jj++) {
					vsum += filter[ii + M * jj]
							* image(i + ii - (int) M / 2, j + jj - (int) N / 2);
				}
			}
			out[i + j * w] = vsum;
		}
	}
}

template<size_t M, size_t N, class T, int C, ImageType I> void Smooth(
		const Image<T, C, I>& image, Image<T, C, I>& B,
		double sigmaX = (0.607902736 * (M - 1) * 0.5),
		double sigmaY = (0.607902736 * (N - 1) * 0.5)) {
	float filter[M][N];
	GaussianKernel(filter, (float) sigmaX, (float) sigmaY);
	B.resize(image.width, image.height);
#pragma omp parallel for
	for (int j = 0; j < image.height; j++) {
		for (int i = 0; i < image.width; i++) {
			vec<float, C> vsum(0.0);
			for (int ii = 0; ii < (int) M; ii++) {
				for (int jj = 0; jj < (int) N; jj++) {
					vec<T, C> val = image(i + ii - (int) M / 2,
							j + jj - (int) N / 2);
					vsum += filter[ii][jj] * vec<float, C>(val);
				}
			}
			B(i, j) = vec<T, C>(vsum);
		}
	}
}
template<int C> void Smooth(const Image<float, C, ImageType::FLOAT>& image,
		Image<float, C, ImageType::FLOAT>& out, float sigma) {
	int fsz = (int) (5 * sigma);
	if (fsz % 2 == 0)
		fsz++;
	if (fsz < 3)
		fsz = 3;
	std::vector<float> filter;
	GaussianKernel(filter, fsz, fsz, sigma, sigma);
	Convolve(image, out, filter, fsz, fsz);
}
template<int C> void Gradient(const Image<float, C, ImageType::FLOAT>& image,
		Image<float, C, ImageType::FLOAT>& dx,Image<float, C, ImageType::FLOAT>& dy, float sigma) {
	int fsz = (int) (5 * sigma);
	if (fsz % 2 == 0)
		fsz++;
	if (fsz < 3)
		fsz = 3;
	std::vector<float> filterX,filterY;
	GaussianKernelDerivative(filterX,filterY, fsz, fsz, sigma, sigma);
	Convolve(image, dx, filterX, fsz, fsz);
	Convolve(image, dy, filterY, fsz, fsz);
}
template<class T, int C, ImageType I> void Smooth(const Image<T, C, I>& image,
		Image<T, C, I>& B, double sigmaX, double sigmaY) {
	double sigma = std::max(sigmaX, sigmaY);
	if (sigma < 1.5f) {
		Smooth<3, 3>(image, B, sigmaX, sigmaY);
	} else if (sigma < 2.5f) {
		Smooth<5, 5>(image, B, sigmaX, sigmaY);
	} else if (sigma < 3.5f) {
		Smooth<7, 7>(image, B, sigmaX, sigmaY);
	} else if (sigma < 5.5f) {
		Smooth<11, 11>(image, B, sigmaX, sigmaY);
	} else if (sigma < 6.5f) {
		Smooth<13, 13>(image, B, sigmaX, sigmaY);
	} else if (sigma < 7.5f) {
		Smooth<15, 15>(image, B, sigmaX, sigmaY);
	} else if (sigma < 8.5f) {
		Smooth<17, 17>(image, B, sigmaX, sigmaY);
	} else if (sigma < 9.5f) {
		Smooth<19, 19>(image, B, sigmaX, sigmaY);
	} else if (sigma < 10.5f) {
		Smooth<21, 21>(image, B, sigmaX, sigmaY);
	} else if (sigma < 11.5f) {
		Smooth<23, 23>(image, B, sigmaX, sigmaY);
	} else if (sigma < 12.5f) {
		Smooth<25, 25>(image, B, sigmaX, sigmaY);
	}
}
template<class T, int C, ImageType I> void Gradient(const Image<T, C, I>& image,
		Image<T, C, I>& gX, Image<T, C, I>& gY, double sigmaX, double sigmaY) {
	double sigma = std::max(sigmaX, sigmaY);
	if (sigma < 1.5f) {
		Gradient<3, 3>(image, gX, gY, sigmaX, sigmaY);
	} else if (sigma < 2.5f) {
		Gradient<5, 5>(image, gX, gY, sigmaX, sigmaY);
	} else if (sigma < 3.5f) {
		Gradient<7, 7>(image, gX, gY, sigmaX, sigmaY);
	} else if (sigma < 5.5f) {
		Gradient<11, 11>(image, gX, gY, sigmaX, sigmaY);
	} else if (sigma < 6.5f) {
		Gradient<13, 13>(image, gX, gY, sigmaX, sigmaY);
	} else if (sigma < 7.5f) {
		Gradient<15, 15>(image, gX, gY, sigmaX, sigmaY);
	} else if (sigma < 8.5f) {
		Gradient<17, 17>(image, gX, gY, sigmaX, sigmaY);
	} else if (sigma < 9.5f) {
		Gradient<19, 19>(image, gX, gY, sigmaX, sigmaY);
	} else if (sigma < 10.5f) {
		Gradient<21, 21>(image, gX, gY, sigmaX, sigmaY);
	} else if (sigma < 11.5f) {
		Gradient<23, 23>(image, gX, gY, sigmaX, sigmaY);
	} else if (sigma < 12.5f) {
		Gradient<25, 25>(image, gX, gY, sigmaX, sigmaY);
	}
}
template<class T, int C, ImageType I> void Smooth3x3(
		const Image<T, C, I>& image, Image<T, C, I>& B) {
	Smooth<3, 3>(image, B);
}
template<class T, int C, ImageType I> void Smooth5x5(
		const Image<T, C, I>& image, Image<T, C, I>& B) {
	Smooth<5, 5>(image, B);
}
template<class T, int C, ImageType I> void Smooth7x7(
		const Image<T, C, I>& image, Image<T, C, I>& B) {
	Smooth<7, 7>(image, B);
}
template<class T, int C, ImageType I> void Smooth11x11(
		const Image<T, C, I>& image, Image<T, C, I>& B) {
	Smooth<11, 11>(image, B);
}

template<class T, int C, ImageType I> void Laplacian3x3(
		const Image<T, C, I>& image, Image<T, C, I>& L) {
	Laplacian<3, 3>(image, L);
}
template<class T, int C, ImageType I> void Laplacian5x5(
		const Image<T, C, I>& image, Image<T, C, I>& L) {
	Laplacian<5, 5>(image, L);
}
template<class T, int C, ImageType I> void Laplacian7x7(
		const Image<T, C, I>& image, Image<T, C, I>& L) {
	Laplacian<7, 7>(image, L);
}
template<class T, int C, ImageType I> void Laplacian11x11(
		const Image<T, C, I>& image, Image<T, C, I>& L) {
	Laplacian<11, 11>(image, L);
}

template<class T, int C, ImageType I> void Gradient3x3(
		const Image<T, C, I>& image, Image<T, C, I>& gX, Image<T, C, I>& gY) {
	Gradient<3, 3>(image, gX, gY);
}
template<class T, int C, ImageType I> void Gradient5x5(
		const Image<T, C, I>& image, Image<T, C, I>& gX, Image<T, C, I>& gY) {
	Gradient<5, 5>(image, gX, gY);
}
template<class T, int C, ImageType I> void Gradient7x7(
		const Image<T, C, I>& image, Image<T, C, I>& gX, Image<T, C, I>& gY) {
	Gradient<7, 7>(image, gX, gY);
}
template<class T, int C, ImageType I> void Gradient11x11(
		const Image<T, C, I>& image, Image<T, C, I>& gX, Image<T, C, I>& gY) {
	Gradient<11, 11>(image, gX, gY);
}

template<class C, class R, size_t M, size_t N> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, float (&data)[M][N]) {
	ss << std::endl;
	for (size_t ii = 0; ii < M; ii++) {
		for (size_t jj = 0; jj < N; jj++) {
			ss << std::setw(8) << std::setfill(' ') << data[ii][jj];
			if (jj < N - 1) {
				ss << ",";
			}
		}
		ss << std::endl;
	}
	return ss;
}

template<class C, class R, size_t M, size_t N> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, double (&data)[M][N]) {
	ss << std::endl;
	for (size_t ii = 0; ii < M; ii++) {
		for (size_t jj = 0; jj < N; jj++) {
			ss << std::setw(8) << std::setfill(' ') << data[ii][jj];
			if (jj < N - 1) {
				ss << ",";
			}
		}
		ss << std::endl;
	}
	return ss;
}

template<class C, class R, size_t M, size_t N> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, int (&data)[M][N]) {
	ss << std::endl;
	for (size_t ii = 0; ii < M; ii++) {
		for (size_t jj = 0; jj < N; jj++) {
			ss << std::setw(8) << std::setfill(' ') << data[ii][jj];
			if (jj < N - 1) {
				ss << ",";
			}
		}
		ss << std::endl;
	}
	return ss;
}

template<class C, class R, size_t M> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, float (&data)[M]) {
	ss << "[";
	for (int ii = 0; ii < M; ii++) {
		ss << data[ii];
		if (ii < M - 1) {
			ss << ",";
		} else {
			ss << "]";
		}
	}
	return ss;
}

template<class C, class R, size_t M> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, double (&data)[M]) {
	ss << "[";
	for (int ii = 0; ii < M; ii++) {
		ss << data[ii];
		if (ii < M - 1) {
			ss << ",";
		} else {
			ss << "]";
		}
	}
	return ss;
}

template<class C, class R, size_t M> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, int (&data)[M]) {
	ss << "[";
	for (int ii = 0; ii < M; ii++) {
		ss << data[ii];
		if (ii < M - 1) {
			ss << ",";
		} else {
			ss << "]";
		}
	}
	return ss;
}

template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const std::vector<double>& data) {
	ss << "[";
	size_t M = data.size();
	for (int ii = 0; ii < M; ii++) {
		ss << data[ii];
		if (ii < M - 1) {
			ss << ",";
		} else {
			ss << "]";
		}
	}
	return ss;
}

template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const std::vector<float>& data) {
	ss << "[";
	size_t M = data.size();
	for (int ii = 0; ii < M; ii++) {
		ss << data[ii];
		if (ii < M - 1) {
			ss << ",";
		} else {
			ss << "]";
		}
	}
	return ss;
}

template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const std::vector<int>& data) {
	ss << "[";
	size_t M = data.size();
	for (int ii = 0; ii < M; ii++) {
		ss << data[ii];
		if (ii < M - 1) {
			ss << ",";
		} else {
			ss << "]";
		}
	}
	return ss;
}

}

#endif /* INCLUDE_CORE_IMAGEPROCESSING_H_ */
