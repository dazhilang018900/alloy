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
#include "AlloyImageProcessing.h"
namespace aly {
void Demosaic(const Image1ub& gray, ImageRGB& colorImage,const BayerFilter& filter) {
	colorImage.resize(gray.width, gray.height);
	const int is_rggb = (filter == BayerFilter::RGGB)?1:0;
	const int is_grbg = (filter == BayerFilter::GRBG)?1:0;
	const int is_gbrg = (filter == BayerFilter::GBRG)?1:0;
	const int is_bggr = (filter == BayerFilter::BGGR)?1:0;
	const std::function<int(int i, int j)> F = [=](int i,int j) {
		return (int)gray(i,j).x;
	};
#pragma omp parallel for
	for (int j = 0; j < gray.height; j++) {
		for (int i = 0; i < gray.width; i++) {

			const int Fij = F(i, j);
			//symmetric 4,2,-1 response - cross
			const float R1 = (4 * F(i, j)
					+ 2
							* (F(i - 1, j) + F(i, j - 1) + F(i + 1, j)
									+ F(i, j + 1)) - F(i - 2, j) - F(i + 2, j)
					- F(i, j - 2) - F(i, j + 2)) / 8.0f;
			//left-right symmetric response - with .5,1,4,5 - theta
			const float R2 = (8 * (F(i - 1, j) + F(i + 1, j)) + 10 * F(i, j)
					+ F(i, j - 2) + F(i, j + 2)
					- 2
							* ((F(i - 1, j - 1) + F(i + 1, j - 1)
									+ F(i - 1, j + 1) + F(i + 1, j + 1))
									+ F(i - 2, j) + F(i + 2, j))) / 16.0f;
			//top-bottom symmetric response - with .5,1,4,5 - phi
			const float R3 = (8 * (F(i, j - 1) + F(i, j + 1)) + 10 * F(i, j)
					+ F(i - 2, j) + F(i + 2, j)
					- 2
							* ((F(i - 1, j - 1) + F(i + 1, j - 1)
									+ F(i - 1, j + 1) + F(i + 1, j + 1))
									+ F(i, j - 2) + F(i, j + 2))) / 16.0f;
			//symmetric 3/2s response - checker
			const float R4 = (12 * F(i, j)
					- 3
							* (F(i - 2, j) + F(i + 2, j) + F(i, j - 2)
									+ F(i, j + 2))
					+ 4
							* (F(i - 1, j - 1) + F(i + 1, j - 1)
									+ F(i - 1, j + 1) + F(i + 1, j + 1)))
					/ 16.0f;
			const float G_at_red_or_blue = R1;
			const float R_at_G_in_red = R2;
			const float B_at_G_in_blue = R2;
			const float R_at_G_in_blue = R3;
			const float B_at_G_in_red = R3;
			const float R_at_B = R4;
			const float B_at_R = R4;

			//RGGB -> RedXY = (0, 0), GreenXY1 = (1, 0), GreenXY2 = (0, 1), BlueXY = (1, 1)
			//GRBG -> RedXY = (1, 0), GreenXY1 = (0, 0), GreenXY2 = (1, 1), BlueXY = (0, 1)
			//GBRG -> RedXY = (0, 1), GreenXY1 = (0, 0), GreenXY2 = (1, 1), BlueXY = (1, 0)
			//BGGR -> RedXY = (1, 1), GreenXY1 = (1, 0), GreenXY2 = (0, 1), BlueXY = (0, 0)
			const int r_mod_2 = j % 2;
			const int c_mod_2 = i % 2;

			const int red_col = is_grbg | is_bggr;
			const int red_row = is_gbrg | is_bggr;
			const int blue_col = 1 - red_col;
			const int blue_row = 1 - red_row;

			const int in_red_row = r_mod_2 == red_row;
			const int in_blue_row = r_mod_2 == blue_row;
			const int is_red_pixel = (r_mod_2 == red_row)
					& (c_mod_2 == red_col);
			const int is_blue_pixel = (r_mod_2 == blue_row)
					& (c_mod_2 == blue_col);
			const int is_green_pixel = !(is_red_pixel | is_blue_pixel);

			//at R locations: R is original
			//at B locations it is the 3/2s symmetric response
			//at G in red rows it is the left-right symmmetric with 4s
			//at G in blue rows it is the top-bottom symmetric with 4s
			float R = (Fij * is_red_pixel + R_at_B * is_blue_pixel
					+ R_at_G_in_red * (is_green_pixel & in_red_row)
					+ R_at_G_in_blue * (is_green_pixel & in_blue_row)) / 255.0f;
			//at B locations: B is original
			//at R locations it is the 3/2s symmetric response
			//at G in red rows it is the top-bottom symmmetric with 4s
			//at G in blue rows it is the left-right symmetric with 4s
			float B = (Fij * is_blue_pixel + B_at_R * is_red_pixel
					+ B_at_G_in_red * (is_green_pixel & in_red_row)
					+ B_at_G_in_blue * (is_green_pixel & in_blue_row)) / 255.0f;
			//at G locations: G is original
			//at R locations: symmetric 4,2,-1
			//at B locations: symmetric 4,2,-1
			float G = (Fij * is_green_pixel
					+ G_at_red_or_blue * (!is_green_pixel)) / 255.0f;
			colorImage(i, j) = ToRGB(float3(R, G, B));
		}
	}
}

void Demosaic(const Image1ub& gray, ImageRGBf& colorImage,const BayerFilter& filter) {
	colorImage.resize(gray.width, gray.height);
	const int is_rggb = (filter == BayerFilter::RGGB)?1:0;
	const int is_grbg = (filter == BayerFilter::GRBG)?1:0;
	const int is_gbrg = (filter == BayerFilter::GBRG)?1:0;
	const int is_bggr = (filter == BayerFilter::BGGR)?1:0;
	const std::function<int(int i, int j)> F = [=](int i,int j) {
		return (int)gray(i,j).x;
	};
#pragma omp parallel for
	for (int j = 0; j < gray.height; j++) {
		for (int i = 0; i < gray.width; i++) {

			const int Fij = F(i, j);
			//symmetric 4,2,-1 response - cross
			const float R1 = (4 * F(i, j)
					+ 2
							* (F(i - 1, j) + F(i, j - 1) + F(i + 1, j)
									+ F(i, j + 1)) - F(i - 2, j) - F(i + 2, j)
					- F(i, j - 2) - F(i, j + 2)) / 8.0f;
			//left-right symmetric response - with .5,1,4,5 - theta
			const float R2 = (8 * (F(i - 1, j) + F(i + 1, j)) + 10 * F(i, j)
					+ F(i, j - 2) + F(i, j + 2)
					- 2
							* ((F(i - 1, j - 1) + F(i + 1, j - 1)
									+ F(i - 1, j + 1) + F(i + 1, j + 1))
									+ F(i - 2, j) + F(i + 2, j))) / 16.0f;
			//top-bottom symmetric response - with .5,1,4,5 - phi
			const float R3 = (8 * (F(i, j - 1) + F(i, j + 1)) + 10 * F(i, j)
					+ F(i - 2, j) + F(i + 2, j)
					- 2
							* ((F(i - 1, j - 1) + F(i + 1, j - 1)
									+ F(i - 1, j + 1) + F(i + 1, j + 1))
									+ F(i, j - 2) + F(i, j + 2))) / 16.0f;
			//symmetric 3/2s response - checker
			const float R4 = (12 * F(i, j)
					- 3
							* (F(i - 2, j) + F(i + 2, j) + F(i, j - 2)
									+ F(i, j + 2))
					+ 4
							* (F(i - 1, j - 1) + F(i + 1, j - 1)
									+ F(i - 1, j + 1) + F(i + 1, j + 1)))
					/ 16.0f;
			const float G_at_red_or_blue = R1;
			const float R_at_G_in_red = R2;
			const float B_at_G_in_blue = R2;
			const float R_at_G_in_blue = R3;
			const float B_at_G_in_red = R3;
			const float R_at_B = R4;
			const float B_at_R = R4;

			//RGGB -> RedXY = (0, 0), GreenXY1 = (1, 0), GreenXY2 = (0, 1), BlueXY = (1, 1)
			//GRBG -> RedXY = (1, 0), GreenXY1 = (0, 0), GreenXY2 = (1, 1), BlueXY = (0, 1)
			//GBRG -> RedXY = (0, 1), GreenXY1 = (0, 0), GreenXY2 = (1, 1), BlueXY = (1, 0)
			//BGGR -> RedXY = (1, 1), GreenXY1 = (1, 0), GreenXY2 = (0, 1), BlueXY = (0, 0)
			const int r_mod_2 = j % 2;
			const int c_mod_2 = i % 2;

			const int red_col = is_grbg | is_bggr;
			const int red_row = is_gbrg | is_bggr;
			const int blue_col = 1 - red_col;
			const int blue_row = 1 - red_row;

			const int in_red_row = r_mod_2 == red_row;
			const int in_blue_row = r_mod_2 == blue_row;
			const int is_red_pixel = (r_mod_2 == red_row)
					& (c_mod_2 == red_col);
			const int is_blue_pixel = (r_mod_2 == blue_row)
					& (c_mod_2 == blue_col);
			const int is_green_pixel = !(is_red_pixel | is_blue_pixel);

			//at R locations: R is original
			//at B locations it is the 3/2s symmetric response
			//at G in red rows it is the left-right symmmetric with 4s
			//at G in blue rows it is the top-bottom symmetric with 4s
			float R = (Fij * is_red_pixel + R_at_B * is_blue_pixel
					+ R_at_G_in_red * (is_green_pixel & in_red_row)
					+ R_at_G_in_blue * (is_green_pixel & in_blue_row)) / 255.0f;
			//at B locations: B is original
			//at R locations it is the 3/2s symmetric response
			//at G in red rows it is the top-bottom symmmetric with 4s
			//at G in blue rows it is the left-right symmetric with 4s
			float B = (Fij * is_blue_pixel + B_at_R * is_red_pixel
					+ B_at_G_in_red * (is_green_pixel & in_red_row)
					+ B_at_G_in_blue * (is_green_pixel & in_blue_row)) / 255.0f;
			//at G locations: G is original
			//at R locations: symmetric 4,2,-1
			//at B locations: symmetric 4,2,-1
			float G = (Fij * is_green_pixel
					+ G_at_red_or_blue * (!is_green_pixel)) / 255.0f;
			colorImage(i, j) = float3(R, G, B);
		}
	}
}
}

