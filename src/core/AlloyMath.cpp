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
	D(0,1)=0;
	D(1,0)=0;
	svd2(
			// input A
			A(0,0),A(0,1),
			A(1,0),A(1,1),
			// output U
			U(0,0),U(0,1),
			U(1,0),U(1,1),
			// output S
			D(0,0),D(1,1),
			// output V
			Vt(0,0),Vt(1,0),
			Vt(0,1),Vt(1,1));
}
void SVD(const matrix<float, 3, 3> &A, matrix<float, 3, 3>& U,
		matrix<float, 3, 3>& D, matrix<float, 3, 3>& Vt) {
	svd3(
			// input A
			A(0,0),A(0,1),A(0,2),
			A(1,0),A(1,1),A(1,2),
			A(2,0),A(2,1),A(2,2),
			// output U
			U(0,0),U(0,1),U(0,2),
			U(1,0),U(1,1),U(1,2),
			U(2,0),U(2,1),U(2,2),
			// output S
			D(0,0),D(0,1),D(0,2),
			D(1,0),D(1,1),D(1,2),
			D(2,0),D(2,1),D(2,2),
			// output V
			Vt(0,0),Vt(1,0),Vt(2,0),
			Vt(0,1),Vt(1,1),Vt(2,1),
			Vt(0,2),Vt(1,2),Vt(2,2));
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
float3 MakeOrthogonalComplement(const float3& v){
	if (std::abs(v.x) > std::abs(v.y)){
		return normalize(float3(-v.z,0.0f,v.x));
    } else {
        return normalize(float3(0.0f,v.z,v.y));
    }
}
float2 MakeOrthogonalComplement(const float2& v){
	return normalize(float2(-v.y,v.x));
}
float IntersectCylinder(aly::float3 pt,aly::float3 ray, float3 start, float3 end, float radius) {
		float t0, t1;
		float3 yaxis = normalize(end - start);
		float3 xaxis=MakeOrthogonalComplement(yaxis);
		float3 zaxis=cross(xaxis,yaxis);
		float3x3 Rt=transpose(float3x3(xaxis,yaxis,zaxis));
		float3 T=start;
		pt=Rt*(pt-T);
		start=Rt*(start-T);
		end=Rt*(end-T);
		ray=Rt*ray;
		float minY=start.y;
		float maxY=end.y;
		// a=xD2+yD2, b=2xExD+2yEyD, and c=xE2+yE2-1.
		float a = ray.x * ray.x + ray.z * ray.z;
		float b = 2 * pt.x * ray.x + 2 * pt.z * ray.z;
		float c = pt.x * pt.x + pt.z * pt.z - radius*radius;
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
				if (th <= 0){
					return -1.0f;
				}
				return th;
			}
		} else if (y0 >= start.y && y0 <= end.y) {
			// hit the cylinder bit
			if (t0 <= 0){
				return -1.0f;
			}
			return t0;
		} else if (y0 > end.y) {
			if (y1 > end.y){
				return -1.0f;
			} else {
				// hit the cap
				float th = t0 + (t1 - t0) * (y0 - end.y) / (y0 - y1);
				if (th <= 0){
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
}
