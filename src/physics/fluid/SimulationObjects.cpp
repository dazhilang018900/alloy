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
 *
 *  This implementation of a PIC/FLIP fluid simulator is derived from:
 *
 *  Ando, R., Thurey, N., & Tsuruno, R. (2012). Preserving fluid sheets with adaptively sampled anisotropic particles.
 *  Visualization and Computer Graphics, IEEE Transactions on, 18(8), 1202-1214.
 */
#include "physics/fluid/SimulationObjects.h"
namespace aly {
bool BoxObject::inside(float2& pt) {
	float delta = -0.25f * mVoxelSize;
	if (pt[0] > mMin[0] + delta && pt[0] < mMax[0] - delta
			&& pt[1] > mMin[1] + delta && pt[1] < mMax[1] - delta) {
		return true;
	} else {
		return false;
	}
}
float BoxObject::signedDistance(float2& pt) {
	if (pt[0] > mMin[0] && pt[0] < mMax[0] && pt[1] > mMin[1]
			&& pt[1] < mMax[1]) {
		float dx = std::min(pt[0] - mMax[0], mMin[0] - pt[0]);
		float dy = std::min(pt[1] - mMax[1], mMin[1] - pt[1]);
		return std::min(dx, dy);
	} else {
		float dx = std::max(pt[0] - mMax[0], mMin[0] - pt[0]);
		float dy = std::max(pt[1] - mMax[1], mMin[1] - pt[1]);
		return std::max(dx, dy);
	}
}
bool BoxObject::insideShell(float2& pt) {
	float delta = -0.25f * mVoxelSize;
	if (pt[0] > mMin[0] + delta && pt[0] < mMax[0] - delta
			&& pt[1] > mMin[1] + delta && pt[1] < mMax[1] - delta) {
		if (pt[0] > mMin[0] + mThickness + delta
				&& pt[0] < mMax[0] - mThickness - delta
				&& pt[1] > mMin[1] + mThickness + delta
				&& pt[1] < mMax[1] - mThickness - delta) {
			return false;
		} else {
			return true;
		}
	} else {
		return false;
	}
}
float MeshObject::signedDistance(float2& pt) {
	float localVoxelSize = mSignedLevelSet->width;
	float2 lpt = localVoxelSize * ((pt - mCenter) / (2 * mRadius) + 0.5f);
	return (*mSignedLevelSet)(lpt[0], lpt[1]).x * mVoxelSize * (2 * mRadius);
}
bool MeshObject::inside(float2& pt) {
	float localVoxelSize = mSignedLevelSet->width;
	float2 lpt = localVoxelSize * ((pt - mCenter) / (2 * mRadius) + 0.5f);
	if ((*mSignedLevelSet)(lpt[0], lpt[1]).x < -0.5f) {
		return true;
	} else {
		return false;
	}
}
bool MeshObject::insideShell(float2& pt) {
	float localVoxelSize = mSignedLevelSet->width;
	float2 lpt = localVoxelSize * ((pt - mCenter) * mRadius + 0.5f);
	float val = (*mSignedLevelSet)(lpt[0], lpt[1]).x;
	if (val > -mThickness - 0.5f && val < -0.5f) {
		return true;
	} else {
		return false;
	}
}
float SphereObject::signedDistance(float2& pt) {
	float len = length(pt - mCenter);
	return len - mRadius;
}
bool SphereObject::inside(float2& pt) {
	float len = length(pt - mCenter);
	if (len < mRadius - 0.25f * mVoxelSize) {
		return true;
	} else {
		return false;
	}
}
bool SphereObject::insideShell(float2& pt) {
	float len = length(pt - mCenter);
	if (len < mRadius) {
		if (len < mRadius - mThickness - 0.25f * mVoxelSize) {
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}
}

