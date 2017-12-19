/*
 * Copyright(C) 2014, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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

#include "AlloyMath.h"
#include "svd3.h"
#include <random>
using namespace std;
namespace aly {

/******************************************************************************
 * XLISP-STAT 2.1 Copyright (c) 1990, by Luke Tierney
 * XLISP version 2.1, Copyright (c) 1989, by David Betz.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Luke Tierney and David Betz not be
 * used in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  Luke Tierney and David Betz
 * make no representations about the suitability of this software for any
 * purpose. It is provided "as is" without express or implied warranty.
 *
 * LUKE TIERNEY AND DAVID BETZ DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
 * IN NO EVENT SHALL LUKE TIERNEY NOR DAVID BETZ BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * XLISP-STAT AUTHOR:
 *               Luke Tierney
 *               School of Statistics
 *               University of Minnesota
 *               Minneapolis, MN 55455
 *               (612) 625-7843
 *
 *       Email Address:
 *               internet: luke@umnstat.stat.umn.edu
 *
 * XLISP AUTHOR:
 *              David Betz
 *              P.O. Box 144
 *              Peterborough, NH 03458
 *              (603) 924-4145
 ******************************************************************************
 * XLISP-STAT 2.1 was ported to the Amiga by
 *              J.K. Lindsey
 *              Faculty of Economic, Business and Social Sciences,
 *              University of Liege,
 *              Sart Tilman B31,
 *              4000 Liege,
 *              Belgium
 *              32-41-56.29.64
 *
 * The above permission and disclaimer also applies to all of the specifically
 * Amiga portions of this software, with the restriction that the Amiga
 * version not be used for any military-related applications.
 ******************************************************************************

 */
double3 ToBary(double2 p, float2 pt1, float2 pt2, float2 pt3) {
	double a = (double)pt1.x - (double)pt3.x;
	double b = (double)pt2.x - (double)pt3.x;
	double c = (double)pt3.x - (double)p.x;
	double d = (double)pt1.y - (double)pt3.y;
	double e = (double)pt2.y - (double)pt3.y;
	double f = (double)pt3.y - (double)p.y;
	double l1 = (b * f - c * e) / (a * e - b * d);
	double l2 = (a * f - c * d) / (b * d - a * e);
	if (std::isnan(l1) || std::isinf(l1)) {
		l1 = 0;
	}
	if (std::isnan(l2) || std::isinf(l2)) {
		l2 = 0;
	}
	if (l1 > 1 || l2 > 1 || l1 + l2 > 1 || l1 < 0 || l2 < 0) {
		l1 = std::max(std::min(l1, 1.0), 0.0);
		l2 = std::max(std::min(l2, 1.0), 0.0);
		if (l1 + l2 > 1) {
			double diff = 0.5 * (1 - l1 - l2);
			l1 += diff;
			l2 += diff;
		}
	}
	return double3(l1, l2, 1 - l1 - l2);
}
double3 ToBary(double3 p, float3 pt1, float3 pt2, float3 pt3) {
	double a = (double)pt1.x - (double)pt3.x;
	double b = (double)pt2.x - (double)pt3.x;
	double c = (double)pt3.x - (double)p.x;
	double d = (double)pt1.y - (double)pt3.y;
	double e = (double)pt2.y - (double)pt3.y;
	double f = (double)pt3.y - (double)p.y;
	double g = (double)pt1.z - (double)pt3.z;
	double h = (double)pt2.z - (double)pt3.z;
	double i = (double)pt3.z - (double)p.z;
	double l1 = (b * (f + i) - c * (e + h)) / (a * (e + h) - b * (d + g));
	double l2 = (a * (f + i) - c * (d + g)) / (b * (d + g) - a * (e + h));
	if (std::isnan(l1) || std::isinf(l1)) {
		l1 = 0;
	}
	if (std::isnan(l2) || std::isinf(l2)) {
		l2 = 0;
	}
	if (l1 > 1 || l2 > 1 || l1 + l2 > 1 || l1 < 0 || l2 < 0) {
		l1 = std::max(std::min(l1, 1.0), 0.0);
		l2 = std::max(std::min(l2, 1.0), 0.0);
		if (l1 + l2 > 1) {
			double diff = 0.5 * (1 - l1 - l2);
			l1 += diff;
			l2 += diff;
		}
	}
	return double3(l1, l2, 1 - l1 - l2);
}
float3 ToBary(float2 p, float2 pt1, float2 pt2, float2 pt3) {
	float a = pt1.x - pt3.x;
	float b = pt2.x - pt3.x;
	float c = pt3.x - p.x;
	float d = pt1.y - pt3.y;
	float e = pt2.y - pt3.y;
	float f = pt3.y - p.y;
	float l1 = (b * f - c * e) / (a * e - b * d);
	float l2 = (a * f - c * d) / (b * d - a * e);
	if (std::isnan(l1) || std::isinf(l1)) {
		l1 = 0;
	}
	if (std::isnan(l2) || std::isinf(l2)) {
		l2 = 0;
	}
	if (l1 > 1 || l2 > 1 || l1 + l2 > 1 || l1 < 0 || l2 < 0) {
		l1 = std::max(std::min(l1, 1.0f), 0.0f);
		l2 = std::max(std::min(l2, 1.0f), 0.0f);
		if (l1 + l2 > 1) {
			double diff = 0.5 * (1 - l1 - l2);
			l1 += diff;
			l2 += diff;
		}
	}
	return float3(l1, l2, 1 - l1 - l2);
}
float3 ToBary(float3 p, float3 pt1, float3 pt2, float3 pt3) {
	float a = pt1.x - pt3.x;
	float b = pt2.x - pt3.x;
	float c = pt3.x - p.x;
	float d = pt1.y - pt3.y;
	float e = pt2.y - pt3.y;
	float f = pt3.y - p.y;
	float g = pt1.z - pt3.z;
	float h = pt2.z - pt3.z;
	float i = pt3.z - p.z;
	float l1 = (b * (f + i) - c * (e + h)) / (a * (e + h) - b * (d + g));
	float l2 = (a * (f + i) - c * (d + g)) / (b * (d + g) - a * (e + h));
	if (std::isnan(l1) || std::isinf(l1)) {
		l1 = 0;
	}
	if (std::isnan(l2) || std::isinf(l2)) {
		l2 = 0;
	}
	if (l1 > 1 || l2 > 1 || l1 + l2 > 1 || l1 < 0 || l2 < 0) {
		l1 = std::max(std::min(l1, 1.0f), 0.0f);
		l2 = std::max(std::min(l2, 1.0f), 0.0f);
		if (l1 + l2 > 1) {
			double diff = 0.5 * (1 - l1 - l2);
			l1 += diff;
			l2 += diff;
		}
	}
	return float3(l1, l2, 1 - l1 - l2);
}
float3 FromBary(float3 b, float3 p1, float3 p2, float3 p3) {
	return float3(p1.x * b.x + p2.x * b.y + p3.x * b.z,
			p1.y * b.x + p2.y * b.y + p3.y * b.z,
			p1.z * b.x + p2.z * b.y + p3.z * b.z);
}
float2 FromBary(float3 b, float2 p1, float2 p2, float2 p3) {
	return float2(p1.x * b.x + p2.x * b.y + p3.x * b.z,
			p1.y * b.x + p2.y * b.y + p3.y * b.z);
}
bool Contains(float2 p, float2 pt1, float2 pt2, float2 pt3){
	return (crossMag(p-pt1,pt2-pt1)<=0.0f&&crossMag(p-pt2,pt3-pt2)<=0.0f&&crossMag(p-pt3,pt1-pt3)<=0.0f);
}
float3 FromBary(double3 b, float3 p1, float3 p2, float3 p3) {
	return float3((double)p1.x * b.x + (double)p2.x * b.y + (double)p3.x * b.z,
			(double)p1.y * b.x + p2.y * b.y + (double)p3.y * b.z,
			(double)p1.z * b.x + p2.z * b.y + (double)p3.z * b.z);
}
float2 FromBary(double3 b, float2 p1, float2 p2, float2 p3) {
	return float2((double)p1.x * b.x + (double)p2.x * b.y + (double)p3.x * b.z,
			(double)p1.y * b.x + (double)p2.y * b.y + (double)p3.y * b.z);
}
template<class T, int m, int n> void SVD_INTERNAL(const matrix<T, m, n>& M,
		matrix<T, m, m>& U, matrix<T, m, n>& D, matrix<T, n, n>& Vt) {
	double v[n][n];
	double u[m][m];
	double w[n];
	double rv1[n];
	int flag, i, its, j, jj, k, l, nm;
	double c, f, h, s, x, y, z;
	double anorm = 0.0, g = 0.0, scale = 0.0;
	if (m < n) {
		throw std::runtime_error(
				"SVD error, rows must be greater than or equal to cols.");
	}
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			u[i][j] = M(i, j);
		}
	}
	for (i = 0; i < n; i++) {
		l = i + 1;
		rv1[i] = scale * g;
		g = s = scale = 0.0;
		if (i < m) {
			for (k = i; k < m; k++)
				scale += std::abs((double) u[k][i]);
			if (scale) {
				for (k = i; k < m; k++) {
					u[k][i] = ((double) u[k][i] / scale);
					s += ((double) u[k][i] * (double) u[k][i]);
				}
				f = (double) u[i][i];
				g = -sign(std::sqrt(s), f);
				h = f * g - s;
				u[i][i] = (f - g);
				if (i != n - 1) {
					for (j = l; j < n; j++) {
						for (s = 0.0, k = i; k < m; k++)
							s += ((double) u[k][i] * (double) u[k][j]);
						f = s / h;
						for (k = i; k < m; k++)
							u[k][j] += (f * (double) u[k][i]);
					}
				}
				for (k = i; k < m; k++)
					u[k][i] = ((double) u[k][i] * scale);
			}
		}
		w[i] = (scale * g);
		g = s = scale = 0.0;
		if (i < m && i != n - 1) {
			for (k = l; k < n; k++)
				scale += std::abs((double) u[i][k]);
			if (scale) {
				for (k = l; k < n; k++) {
					u[i][k] = ((double) u[i][k] / scale);
					s += ((double) u[i][k] * (double) u[i][k]);
				}
				f = (double) u[i][l];
				g = -sign(std::sqrt(s), f);
				h = f * g - s;
				u[i][l] = (f - g);
				for (k = l; k < n; k++)
					rv1[k] = (double) u[i][k] / h;
				if (i != m - 1) {
					for (j = l; j < m; j++) {
						for (s = 0.0, k = l; k < n; k++)
							s += ((double) u[j][k] * (double) u[i][k]);
						for (k = l; k < n; k++)
							u[j][k] += (s * rv1[k]);
					}
				}
				for (k = l; k < n; k++)
					u[i][k] = ((double) u[i][k] * scale);
			}
		}
		anorm = aly::max(anorm, (std::abs((double) w[i]) + std::abs(rv1[i])));
	}
	for (i = n - 1; i >= 0; i--) {
		if (i < n - 1) {
			if (g) {
				for (j = l; j < n; j++)
					v[j][i] = (((double) u[i][j] / (double) u[i][l]) / g);
				for (j = l; j < n; j++) {
					for (s = 0.0, k = l; k < n; k++)
						s += ((double) u[i][k] * (double) v[k][j]);
					for (k = l; k < n; k++)
						v[k][j] += (s * (double) v[k][i]);
				}
			}
			for (j = l; j < n; j++)
				v[i][j] = v[j][i] = 0.0;
		}
		v[i][i] = 1.0;
		g = rv1[i];
		l = i;
	}
	for (i = n - 1; i >= 0; i--) {
		l = i + 1;
		g = (double) w[i];
		if (i < n - 1)
			for (j = l; j < n; j++)
				u[i][j] = 0.0;
		if (g) {
			g = 1.0 / g;
			if (i != n - 1) {
				for (j = l; j < n; j++) {
					for (s = 0.0, k = l; k < m; k++)
						s += ((double) u[k][i] * (double) u[k][j]);
					f = (s / (double) u[i][i]) * g;
					for (k = i; k < m; k++)
						u[k][j] += (f * (double) u[k][i]);
				}
			}
			for (j = i; j < m; j++)
				u[j][i] = ((double) u[j][i] * g);
		} else {
			for (j = i; j < m; j++)
				u[j][i] = 0.0;
		}
		++u[i][i];
	}
	for (k = n - 1; k >= 0; k--) {
		for (its = 0; its < 30; its++) {
			flag = 1;
			for (l = k; l >= 0; l--) {
				nm = l - 1;
				if (std::abs(rv1[l]) + anorm == anorm) {
					flag = 0;
					break;
				}
				if (nm >= 0 && std::abs(w[nm]) + anorm == anorm)
					break;
			}
			if (flag) {
				c = 0.0;
				s = 1.0;
				for (i = l; i <= k; i++) {
					f = s * rv1[i];
					if (std::abs(f) + anorm != anorm) {
						g = (double) w[i];
						h = pythag(f, g);
						w[i] = h;
						h = 1.0 / h;
						c = g * h;
						s = (-f * h);
						for (j = 0; j < m; j++) {
							y = (double) u[j][nm];
							z = (double) u[j][i];
							u[j][nm] = (y * c + z * s);
							u[j][i] = (z * c - y * s);
						}
					}
				}
			}
			z = (double) w[k];
			if (l == k) {
				if (z < 0.0) {
					w[k] = (-z);
					for (j = 0; j < n; j++)
						v[j][k] = (-v[j][k]);
				}
				int iii, jjj;
				for (iii = k; (iii < n - 1) && (w[iii] < w[iii + 1]); iii++) {
					std::swap(w[iii], w[iii + 1]);
					for (jjj = 0; jjj < m; jjj++)
						std::swap(u[jjj][iii], u[jjj][iii + 1]);
					for (jjj = 0; jjj < n; jjj++)
						std::swap(v[jjj][iii], v[jjj][iii + 1]);
				}
				break;
			}
			if (its >= 30) {
				throw runtime_error("SVD did not converge.");
			}
			x = (double) w[l];
			nm = k - 1;
			y = (double) w[nm];
			g = rv1[nm];
			h = rv1[k];
			f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
			g = pythag(f, 1.0);
			f = ((x - z) * (x + z) + h * ((y / (f + sign(g, f))) - h)) / x;
			c = s = 1.0;
			for (j = l; j <= nm; j++) {
				i = j + 1;
				g = rv1[i];
				y = (double) w[i];
				h = s * g;
				g = c * g;
				z = pythag(f, h);
				rv1[j] = z;
				c = f / z;
				s = h / z;
				f = x * c + g * s;
				g = g * c - x * s;
				h = y * s;
				y = y * c;
				for (jj = 0; jj < n; jj++) {
					x = (double) v[jj][j];
					z = (double) v[jj][i];
					v[jj][j] = (x * c + z * s);
					v[jj][i] = (z * c - x * s);
				}
				z = pythag(f, h);
				w[j] = z;
				if (z) {
					z = 1.0 / z;
					c = f * z;
					s = h * z;
				}
				f = (c * g) + (s * y);
				x = (c * y) - (s * g);
				for (jj = 0; jj < m; jj++) {
					y = (double) u[jj][j];
					z = (double) u[jj][i];
					u[jj][j] = (y * c + z * s);
					u[jj][i] = (z * c - y * s);
				}
			}
			rv1[l] = 0.0;
			rv1[k] = f;
			w[k] = x;
		}
	}
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			if (i == j) {
				D(i, j) = (T) w[j];
			} else {
				D(i, j) = T(0);
			}
		}
	}
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < m; j++) {
			U(i, j) = (T) u[i][j];
		}
	}
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			Vt(i, j) = (T) v[j][i];
		}
	}
}
void SVD(const matrix<float, 2, 2> &A, matrix<float, 2, 2>& U,
		matrix<float, 2, 2>& D, matrix<float, 2, 2>& Vt) {
	D(0, 1) = 0;
	D(1, 0) = 0;
	svd2(
	// input A
			A(0, 0), A(0, 1), A(1, 0), A(1, 1),
			// output U
			U(0, 0), U(0, 1), U(1, 0), U(1, 1),
			// output S
			D(0, 0), D(1, 1),
			// output V
			Vt(0, 0), Vt(1, 0), Vt(0, 1), Vt(1, 1));
}
void SVD(const matrix<float, 3, 3> &A, matrix<float, 3, 3>& U,
		matrix<float, 3, 3>& D, matrix<float, 3, 3>& Vt) {
	svd3(
			// input A
			A(0, 0), A(0, 1), A(0, 2), A(1, 0), A(1, 1), A(1, 2), A(2, 0),
			A(2, 1), A(2, 2),
			// output U
			U(0, 0), U(0, 1), U(0, 2), U(1, 0), U(1, 1), U(1, 2), U(2, 0),
			U(2, 1), U(2, 2),
			// output S
			D(0, 0), D(0, 1), D(0, 2), D(1, 0), D(1, 1), D(1, 2), D(2, 0),
			D(2, 1), D(2, 2),
			// output V
			Vt(0, 0), Vt(1, 0), Vt(2, 0), Vt(0, 1), Vt(1, 1), Vt(2, 1),
			Vt(0, 2), Vt(1, 2), Vt(2, 2));
}
void SVD(const matrix<float, 4, 4> &A, matrix<float, 4, 4>& U,
		matrix<float, 4, 4>& D, matrix<float, 4, 4>& Vt) {
	SVD_INTERNAL(A, U, D, Vt);
}
void SVD(const matrix<double, 2, 2> &A, matrix<double, 2, 2>& U,
		matrix<double, 2, 2>& D, matrix<double, 2, 2>& Vt) {
	SVD_INTERNAL(A, U, D, Vt);
}
void SVD(const matrix<double, 3, 3> &A, matrix<double, 3, 3>& U,
		matrix<double, 3, 3>& D, matrix<double, 3, 3>& Vt) {
	SVD_INTERNAL(A, U, D, Vt);
}
void SVD(const matrix<double, 4, 4> &A, matrix<double, 4, 4>& U,
		matrix<double, 4, 4>& D, matrix<double, 4, 4>& Vt) {
	SVD_INTERNAL(A, U, D, Vt);
}

bool ClipLine(float2& pt1, float2& pt2, const float2& minPt,
		const float2& maxPt) {
	const int INSIDE = 0; // 0000
	const int LEFT = 1;   // 0001
	const int RIGHT = 2;  // 0010
	const int BOTTOM = 4; // 0100
	const int TOP = 8;    // 1000
	std::function<
			int(float x, float y, const float2& minPt, const float2& maxPt)> ComputeOutCode =
			[=](float x, float y,const float2& minPt,const float2& maxPt) {
				int code = INSIDE; // initialised as being inside of [[clip window]]
				if (x < minPt.x)// to the left of clip window
				code |= LEFT;
				else if (x > maxPt.x)// to the right of clip window
				code |= RIGHT;
				if (y < minPt.y)// below the clip window
				code |= BOTTOM;
				else if (y > maxPt.y)// above the clip window
				code |= TOP;
				return code;
			};
	// compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
	int outcode0 = ComputeOutCode(pt1.x, pt1.y, minPt, maxPt);
	int outcode1 = ComputeOutCode(pt2.x, pt2.y, minPt, maxPt);
	bool accept = false;
	while (true) {
		if (!(outcode0 | outcode1)) { // Bitwise OR is 0. Trivially accept and get out of loop
			accept = true;
			break;
		} else if (outcode0 & outcode1) { // Bitwise AND is not 0. (implies both end points are in the same region outside the window). Reject and get out of loop
			break;
		} else {
			// failed both tests, so calculate the line segment to clip
			// from an outside point to an intersection with clip edge
			float x = 0.0f, y = 0.0f;
			// At least one endpoint is outside the clip rectangle; pick it.
			int outcodeOut = outcode0 ? outcode0 : outcode1;
			// Now find the intersection point;
			// use formulas y = pt1.y + slope * (x - pt1.x), x = pt1.x + (1 / slope) * (y - pt1.y)
			if (outcodeOut & TOP) {         // point is above the clip rectangle
				x = pt1.x
						+ (pt2.x - pt1.x) * (maxPt.y - pt1.y) / (pt2.y - pt1.y);
				y = maxPt.y;
			} else if (outcodeOut & BOTTOM) { // point is below the clip rectangle
				x = pt1.x
						+ (pt2.x - pt1.x) * (minPt.y - pt1.y) / (pt2.y - pt1.y);
				y = minPt.y;
			} else if (outcodeOut & RIGHT) { // point is to the right of clip rectangle
				y = pt1.y
						+ (pt2.y - pt1.y) * (maxPt.x - pt1.x) / (pt2.x - pt1.x);
				x = maxPt.x;
			} else if (outcodeOut & LEFT) { // point is to the left of clip rectangle
				y = pt1.y
						+ (pt2.y - pt1.y) * (minPt.x - pt1.x) / (pt2.x - pt1.x);
				x = minPt.x;
			}
			// Now we move outside point to intersection point to clip
			// and get ready for next pass.
			if (outcodeOut == outcode0) {
				pt1.x = x;
				pt1.y = y;
				outcode0 = ComputeOutCode(pt1.x, pt1.y, minPt, maxPt);
			} else {
				pt2.x = x;
				pt2.y = y;
				outcode1 = ComputeOutCode(pt2.x, pt2.y, minPt, maxPt);
			}
		}
	}
	return accept;
}
bool ClipLine(double2& pt1, double2& pt2, const double2& minPt,
		const double2& maxPt) {
	const int INSIDE = 0; // 0000
	const int LEFT = 1;   // 0001
	const int RIGHT = 2;  // 0010
	const int BOTTOM = 4; // 0100
	const int TOP = 8;    // 1000
	std::function<
			int(double x, double y, const double2& minPt, const double2& maxPt)> ComputeOutCode =
			[=](double x, double y,const double2& minPt,const double2& maxPt) {
				int code = INSIDE; // initialised as being inside of [[clip window]]
				if (x < minPt.x)// to the left of clip window
				code |= LEFT;
				else if (x > maxPt.x)// to the right of clip window
				code |= RIGHT;
				if (y < minPt.y)// below the clip window
				code |= BOTTOM;
				else if (y > maxPt.y)// above the clip window
				code |= TOP;
				return code;
			};
	// compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
	int outcode0 = ComputeOutCode(pt1.x, pt1.y, minPt, maxPt);
	int outcode1 = ComputeOutCode(pt2.x, pt2.y, minPt, maxPt);
	bool accept = false;
	while (true) {
		if (!(outcode0 | outcode1)) { // Bitwise OR is 0. Trivially accept and get out of loop
			accept = true;
			break;
		} else if (outcode0 & outcode1) { // Bitwise AND is not 0. (implies both end points are in the same region outside the window). Reject and get out of loop
			break;
		} else {
			// failed both tests, so calculate the line segment to clip
			// from an outside point to an intersection with clip edge
			double x = 0.0, y = 0.0;
			// At least one endpoint is outside the clip rectangle; pick it.
			int outcodeOut = outcode0 ? outcode0 : outcode1;
			// Now find the intersection point;
			// use formulas y = pt1.y + slope * (x - pt1.x), x = pt1.x + (1 / slope) * (y - pt1.y)
			if (outcodeOut & TOP) {         // point is above the clip rectangle
				x = pt1.x
						+ (pt2.x - pt1.x) * (maxPt.y - pt1.y) / (pt2.y - pt1.y);
				y = maxPt.y;
			} else if (outcodeOut & BOTTOM) { // point is below the clip rectangle
				x = pt1.x
						+ (pt2.x - pt1.x) * (minPt.y - pt1.y) / (pt2.y - pt1.y);
				y = minPt.y;
			} else if (outcodeOut & RIGHT) { // point is to the right of clip rectangle
				y = pt1.y
						+ (pt2.y - pt1.y) * (maxPt.x - pt1.x) / (pt2.x - pt1.x);
				x = maxPt.x;
			} else if (outcodeOut & LEFT) { // point is to the left of clip rectangle
				y = pt1.y
						+ (pt2.y - pt1.y) * (minPt.x - pt1.x) / (pt2.x - pt1.x);
				x = minPt.x;
			}
			// Now we move outside point to intersection point to clip
			// and get ready for next pass.
			if (outcodeOut == outcode0) {
				pt1.x = x;
				pt1.y = y;
				outcode0 = ComputeOutCode(pt1.x, pt1.y, minPt, maxPt);
			} else {
				pt2.x = x;
				pt2.y = y;
				outcode1 = ComputeOutCode(pt2.x, pt2.y, minPt, maxPt);
			}
		}
	}
	return accept;
}
float3 MakeOrthogonalComplement(const float3& v) {
	if (std::abs(v.x) > std::abs(v.y)) {
		return normalize(float3(-v.z, 0.0f, v.x));
	} else {
		return normalize(float3(0.0f, v.z, v.y));
	}
}
float2 MakeOrthogonalComplement(const float2& v) {
	return normalize(float2(-v.y, v.x));
}
float IntersectCylinder(aly::float3 pt, aly::float3 ray, float3 start,
		float3 end, float radius) {
	float t0, t1;
	float3 yaxis = normalize(end - start);
	float3 xaxis = MakeOrthogonalComplement(yaxis);
	float3 zaxis = cross(xaxis, yaxis);
	float3x3 Rt = transpose(float3x3(xaxis, yaxis, zaxis));
	float3 T = start;
	pt = Rt * (pt - T);
	start = Rt * (start - T);
	end = Rt * (end - T);
	ray = Rt * ray;
	float minY = start.y;
	float maxY = end.y;
	// a=xD2+yD2, b=2xExD+2yEyD, and c=xE2+yE2-1.
	float a = ray.x * ray.x + ray.z * ray.z;
	float b = 2 * pt.x * ray.x + 2 * pt.z * ray.z;
	float c = pt.x * pt.x + pt.z * pt.z - radius * radius;
	float b24ac = b * b - 4 * a * c;
	if (b24ac < 0)
		return -1.0f;
	float sqb24ac = std::sqrt(b24ac);
	t0 = (-b + sqb24ac) / (2 * a);
	t1 = (-b - sqb24ac) / (2 * a);
	if (t0 > t1) {
		std::swap(t0, t1);
	}
	float y0 = pt.y + t0 * ray.y;
	float y1 = pt.y + t1 * ray.y;
	if (y0 < start.y) {
		if (y1 < start.y)
			return -1.0f;
		else {
			// hit the cap
			float th = t0 + (t1 - t0) * (y0 - start.y) / (y0 - y1);
			if (th <= 0) {
				return -1.0f;
			}
			return th;
		}
	} else if (y0 >= start.y && y0 <= end.y) {
		// hit the cylinder bit
		if (t0 <= 0) {
			return -1.0f;
		}
		return t0;
	} else if (y0 > end.y) {
		if (y1 > end.y) {
			return -1.0f;
		} else {
			// hit the cap
			float th = t0 + (t1 - t0) * (y0 - end.y) / (y0 - y1);
			if (th <= 0) {
				return -1.0f;
			}
			return th;
		}
	}
	return -1.0f;
}
float IntersectPlane(const float3& pt, const float3& ray, const float4& plane) {
	const float3 n = plane.xyz();
	return -(plane.w + dot(n, pt)) / dot(n, ray);
}
bool IntersectBox(float3 rayOrig, float3 rayDir, const box3f& bbox, float& near,
		float& far) {
	// find the ray intersections with the grid
	float3 botc = (bbox.min() - rayOrig) / rayDir;
	float3 topc = (bbox.max() - rayOrig) / rayDir;
	// find the range of intersections
	float3 minc = aly::min(botc, topc);
	float3 maxc = aly::max(botc, topc);

	// now find the closest and furthest
	near = std::max(std::max(minc.x, minc.y), minc.z);
	far = std::min(std::min(maxc.x, maxc.y), maxc.z);
	return far > near;
}
float RandomUniform(float min, float max) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<float> noise(min, max);
	return noise(gen);
}
int RandomUniform(int min, int max) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_int_distribution<int> noise(min, max);
	return noise(gen);
}
double RandomUniform(double min, double max) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<double> noise(min, max);
	return noise(gen);
}
double RandomGaussian(double mean, double stddev) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::normal_distribution<double> noise(mean, stddev);
	return noise(gen);
}
float RandomGaussian(float mean, float stddev) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::normal_distribution<float> noise(mean, stddev);
	return noise(gen);
}
float InvSqrt(float x) {
	float xhalf = 0.5f * x;
	int i = *(int*) &x;
	i = 0x5f3759df - (i >> 1);
	x = *(float*) &i;
	x = x * (1.5f - xhalf * x * x);
	return x;
}

bool SANITY_CHECK_SVD() {
	std::cout << "Sanity Check SVD" << std::endl;
	int N = 100;
	std::uniform_real_distribution<float> r(-1.0f, 1.0f);
	std::random_device rd;
	std::mt19937 gen(rd());
	float3x3 M = float3x3::identity();
	float3x3 R = SubMatrix(
			MakeRotation(normalize(float3(r(gen), r(gen), r(gen))),
					(float) (r(gen) * ALY_PI * 2)));
	float3x3 S = SubMatrix(MakeScale(float3(r(gen), r(gen), r(gen))));
	std::vector<float3> in(N);
	float3 avgIn;
	float3 avgOut;
	for (int n = 0; n < N; n++) {
		float x = 10 * r(gen);
		float y = 10 * r(gen);
		float z = 10 * r(gen);
		float3 pt(x, y, z);
		float3 qt = R * pt - pt;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				M(i, j) += qt[i] * qt[j];
			}
		}
	}
	float2x2 M2 = SubMatrix(M);
	std::cout << "M=\n" << M << std::endl;
	float3x3 Q, D;
	Eigen(M, Q, D);
	std::cout << "Q=\n" << Q << std::endl;
	std::cout << "R=\n" << R << std::endl;
	std::cout << "D=\n" << D << std::endl;
	float3x3 QDQt = Q * D * transpose(Q);
	std::cout << "QDQt=\n" << QDQt * inverse(M) << std::endl;
	float3x3 U, Vt;

	float2x2 U2, Vt2, D2;

	SVD(M, U, D, Vt);
	std::cout << "SVD3: U=\n" << U << std::endl;
	std::cout << "SVD3: D=\n" << D << std::endl;
	std::cout << "SVD3: Vt=\n" << Vt << std::endl;
	std::cout << "SVD3: UDVt=\n" << U * D * Vt * inverse(M) << std::endl;

	SVD_INTERNAL(M, U, D, Vt);
	std::cout << "U=\n" << U << std::endl;
	std::cout << "D=\n" << D << std::endl;
	std::cout << "Vt=\n" << Vt << std::endl;
	std::cout << "UDVt=\n" << U * D * Vt * inverse(M) << std::endl;

	SVD(M2, U2, D2, Vt2);
	std::cout << "SVD3: U=\n" << U2 << std::endl;
	std::cout << "SVD3: D=\n" << D2 << std::endl;
	std::cout << "SVD3: Vt=\n" << Vt2 << std::endl;
	std::cout << "SVD3: UDVt=\n" << U2 * D2 * Vt2 * inverse(M2) << std::endl;

	SVD_INTERNAL(M2, U2, D2, Vt2);
	std::cout << "U=\n" << U2 << std::endl;
	std::cout << "D=\n" << D2 << std::endl;
	std::cout << "Vt=\n" << Vt2 << std::endl;
	std::cout << "UDVt=\n" << U2 * D2 * Vt2 * inverse(M2) << std::endl;

	float3x3 Rest = FactorRotation(M);
	float3x3 A1 = U * D * Vt;
	float3x3 A2 = U * MakeDiagonal(float3(1, 1, -1)) * D * Vt;
	std::cout << "Determinant " << determinant(A1) << " " << determinant(A2)
			<< std::endl;
	std::cout << "Rotation " << Rest << std::endl;
	return true;
}
float DistanceToEdgeSqr(const float3& pt, const float3& pt1, const float3& pt2,
		float3* lastClosestSegmentPoint) {
	float3 dir = pt2 - pt1;
	float len = length(dir);
	dir = normalize(dir);
	float3 diff = pt - pt1;
	float mSegmentParameter = dot(dir, diff);
	if (0 < mSegmentParameter) {
		if (mSegmentParameter < len) {
			*lastClosestSegmentPoint = dir * mSegmentParameter + pt1;
		} else {
			*lastClosestSegmentPoint = pt2;
		}
	} else {
		*lastClosestSegmentPoint = pt1;
	}
	return lengthSqr(pt - (*lastClosestSegmentPoint));
}
float DistanceToEdgeSqr(const float3& pt, const float3& pt1,
		const float3& pt2) {
	float3 tmp;
	return DistanceToEdgeSqr(pt, pt1, pt2, &tmp);
}
//Implementation from geometric tools (http://www.geometrictools.com)
inline float3 parametricTriangle(float3 e0, float3 e1, float s, float t,
		float3 B) {
	float3 Bsum = B + s * e0 + t * e1;
	return Bsum;
}
inline float2 parametricTriangle(float2 e0, float2 e1, float s, float t,
		float2 B) {
	float2 Bsum = B + s * e0 + t * e1;
	return Bsum;
}
float Angle(const float3& v0, const float3& v1, const float3& v2) {
	float3 v = v0 - v1;
	float3 w = v2 - v1;
	float len1 = length(v);
	float len2 = length(w);
	return std::acos(dot(v, w) / std::max(1E-8f, len1 * len2));
}
float DistanceToTriangleSqr(const float2& p, const float2& v0, const float2& v1,const float2& v2, float2* closestPoint) {
	float distanceSquared = 0;
	int region_id = 0;

	float2 P = p;
	float2 B = v0;
	float2 e0 = v1 - v0;
	float2 e1 = v2 - v0;
	float a = dot(e0, e0);
	float b = dot(e0, e1);
	float c = dot(e1, e1);
	float2 dv = B - P;
	float d = dot(e0, dv);
	float e = dot(e1, dv);
	// Determine which region_id contains s, t

	float det = a * c - b * b;
	float s = b * e - c * d;
	float t = b * d - a * e;

	if (s + t <= det) {
		if (s < 0) {
			if (t < 0) {
				region_id = 4;
			} else {
				region_id = 3;
			}
		} else if (t < 0) {
			region_id = 5;
		} else {
			region_id = 0;
		}
	} else {
		if (s < 0) {
			region_id = 2;
		} else if (t < 0) {
			region_id = 6;
		} else {
			region_id = 1;
		}
	}

	// Parametric Triangle Point
	float2 T(0.0f);

	if (region_id == 0) { // Region 0
		float invDet = (float) 1 / (float) det;
		s *= invDet;
		t *= invDet;

		// Find point on parametric triangle based on s and t
		T = parametricTriangle(e0, e1, s, t, B);
		// Find distance from P to T
		distanceSquared = 0;
	} else if (region_id == 1) { // Region 1
		float numer = c + e - b - d;

		if (numer < +0) {
			s = 0;
		} else {
			float denom = a - 2 * b + c;
			s = (numer >= denom ? 1 : numer / denom);
		}
		t = 1 - s;

		// Find point on parametric triangle based on s and t
		T = parametricTriangle(e0, e1, s, t, B);
		// Find distance from P to T
		float2 tmp = P - T;
		distanceSquared = lengthSqr(tmp);
	} else if (region_id == 2) { // Region 2
		float tmp0 = b + d;
		float tmp1 = c + e;

		if (tmp1 > tmp0) {
			float numer = tmp1 - tmp0;
			float denom = a - 2 * b + c;
			s = (numer >= denom ? 1 : numer / denom);
			t = 1 - s;
		} else {
			s = 0;
			t = (tmp1 <= 0 ? 1 : (e >= 0 ? 0 : -e / c));
		}

		// Find point on parametric triangle based on s and t
		T = parametricTriangle(e0, e1, s, t, B);
		// Find distance from P to T
		float2 tmp = P - T;
		distanceSquared = lengthSqr(tmp);
	} else if (region_id == 3) { // Region 3
		s = 0;
		t = (e >= 0 ? 0 : (-e >= c ? 1 : -e / c));

		// Find point on parametric triangle based on s and t
		T = parametricTriangle(e0, e1, s, t, B);
		// Find distance from P to T
		float2 tmp = P - T;
		distanceSquared = lengthSqr(tmp);

	} else if (region_id == 4) { // Region 4
		float tmp0 = c + e;
		float tmp1 = a + d;

		if (tmp0 > tmp1) {
			s = 0;
			t = (tmp1 <= 0 ? 1 : (e >= 0 ? 0 : -e / c));
		} else {
			t = 0;
			s = (tmp1 <= 0 ? 1 : (d >= 0 ? 0 : -d / a));
		}

		// Find point on parametric triangle based on s and t
		T = parametricTriangle(e0, e1, s, t, B);
		// Find distance from P to T
		float2 tmp = P - T;
		distanceSquared = lengthSqr(tmp);
	} else if (region_id == 5) { // Region 5
		t = 0;
		s = (d >= 0 ? 0 : (-d >= a ? 1 : -d / a));

		// Find point on parametric triangle based on s and t
		T = parametricTriangle(e0, e1, s, t, B);
		// Find distance from P to T
		float2 tmp = P - T;
		distanceSquared = lengthSqr(tmp);
	} else { // Region 6
		float tmp0 = b + e;
		float tmp1 = a + d;

		if (tmp1 > tmp0) {
			float numer = tmp1 - tmp0;
			float denom = c - 2 * b + a;
			t = (numer >= denom ? 1 : numer / denom);
			s = 1 - t;
		} else {
			t = 0;
			s = (tmp1 <= 0 ? 1 : (d >= 0 ? 0 : -d / a));
		}

		// Find point on parametric triangle based on s and t
		T = parametricTriangle(e0, e1, s, t, B);
		// Find distance from P to T
		float2 tmp = P - T;
		distanceSquared = lengthSqr(tmp);
	}
	(*closestPoint) = T;
	return distanceSquared;
}
float DistanceToTriangleSqr(const float3& p, const float3& v0, const float3& v1,
		const float3& v2, float3* closestPoint) {
	float distanceSquared = 0;
	int region_id = 0;

	float3 P = p;
	float3 B = v0;
	float3 e0 = v1 - v0;
	float3 e1 = v2 - v0;
	float a = dot(e0, e0);
	float b = dot(e0, e1);
	float c = dot(e1, e1);
	float3 dv = B - P;
	float d = dot(e0, dv);
	float e = dot(e1, dv);
	// Determine which region_id contains s, t

	float det = a * c - b * b;
	float s = b * e - c * d;
	float t = b * d - a * e;

	if (s + t <= det) {
		if (s < 0) {
			if (t < 0) {
				region_id = 4;
			} else {
				region_id = 3;
			}
		} else if (t < 0) {
			region_id = 5;
		} else {
			region_id = 0;
		}
	} else {
		if (s < 0) {
			region_id = 2;
		} else if (t < 0) {
			region_id = 6;
		} else {
			region_id = 1;
		}
	}

	// Parametric Triangle Point
	float3 T(0.0f);

	if (region_id == 0) { // Region 0
		float invDet = (float) 1 / (float) det;
		s *= invDet;
		t *= invDet;

		// Find point on parametric triangle based on s and t
		T = parametricTriangle(e0, e1, s, t, B);
		// Find distance from P to T
		float3 tmp = P - T;
		distanceSquared = lengthSqr(tmp);
	} else if (region_id == 1) { // Region 1
		float numer = c + e - b - d;

		if (numer < +0) {
			s = 0;
		} else {
			float denom = a - 2 * b + c;
			s = (numer >= denom ? 1 : numer / denom);
		}
		t = 1 - s;

		// Find point on parametric triangle based on s and t
		T = parametricTriangle(e0, e1, s, t, B);
		// Find distance from P to T
		float3 tmp = P - T;
		distanceSquared = lengthSqr(tmp);
	} else if (region_id == 2) { // Region 2
		float tmp0 = b + d;
		float tmp1 = c + e;

		if (tmp1 > tmp0) {
			float numer = tmp1 - tmp0;
			float denom = a - 2 * b + c;
			s = (numer >= denom ? 1 : numer / denom);
			t = 1 - s;
		} else {
			s = 0;
			t = (tmp1 <= 0 ? 1 : (e >= 0 ? 0 : -e / c));
		}

		// Find point on parametric triangle based on s and t
		T = parametricTriangle(e0, e1, s, t, B);
		// Find distance from P to T
		float3 tmp = P - T;
		distanceSquared = lengthSqr(tmp);
	} else if (region_id == 3) { // Region 3
		s = 0;
		t = (e >= 0 ? 0 : (-e >= c ? 1 : -e / c));

		// Find point on parametric triangle based on s and t
		T = parametricTriangle(e0, e1, s, t, B);
		// Find distance from P to T
		float3 tmp = P - T;
		distanceSquared = lengthSqr(tmp);

	} else if (region_id == 4) { // Region 4
		float tmp0 = c + e;
		float tmp1 = a + d;

		if (tmp0 > tmp1) {
			s = 0;
			t = (tmp1 <= 0 ? 1 : (e >= 0 ? 0 : -e / c));
		} else {
			t = 0;
			s = (tmp1 <= 0 ? 1 : (d >= 0 ? 0 : -d / a));
		}

		// Find point on parametric triangle based on s and t
		T = parametricTriangle(e0, e1, s, t, B);
		// Find distance from P to T
		float3 tmp = P - T;
		distanceSquared = lengthSqr(tmp);
	} else if (region_id == 5) { // Region 5
		t = 0;
		s = (d >= 0 ? 0 : (-d >= a ? 1 : -d / a));

		// Find point on parametric triangle based on s and t
		T = parametricTriangle(e0, e1, s, t, B);
		// Find distance from P to T
		float3 tmp = P - T;
		distanceSquared = lengthSqr(tmp);
	} else { // Region 6
		float tmp0 = b + e;
		float tmp1 = a + d;

		if (tmp1 > tmp0) {
			float numer = tmp1 - tmp0;
			float denom = c - 2 * b + a;
			t = (numer >= denom ? 1 : numer / denom);
			s = 1 - t;
		} else {
			t = 0;
			s = (tmp1 <= 0 ? 1 : (d >= 0 ? 0 : -d / a));
		}

		// Find point on parametric triangle based on s and t
		T = parametricTriangle(e0, e1, s, t, B);
		// Find distance from P to T
		float3 tmp = P - T;
		distanceSquared = lengthSqr(tmp);
	}
	(*closestPoint) = T;
	return distanceSquared;
}

//What if quad is non-convex? Does this hold?
float DistanceToQuadSqr(const float3& p, const float3& v0, const float3& v1,
		const float3& v2, const float3& v3, const float3& norm,
		float3* closestPoint) {
	float3 cp1;
	float3 cp2;
	float d1, d2;
	if (dot(cross((v2 - v0), (v1 - v0)), norm) > 0) {
		d1 = DistanceToTriangleSqr(p, v0, v1, v2, &cp1);
		d2 = DistanceToTriangleSqr(p, v2, v3, v0, &cp2);
	} else {
		d1 = DistanceToTriangleSqr(p, v1, v2, v3, &cp1);
		d2 = DistanceToTriangleSqr(p, v3, v0, v1, &cp2);
	}

	if (d1 < d2) {
		*closestPoint = cp1;
		return d1;
	} else {
		*closestPoint = cp2;
		return d2;
	}
}
}
