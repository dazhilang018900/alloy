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
#include "physics/fluid/ParticleLocator.h"

#include <math.h>
using namespace std;
namespace aly {
ParticleLocator::ParticleLocator(int2 dims, float voxelSize) :
		mVoxelSize(voxelSize), mGridSize(dims) {
	cells.resize(dims.x * dims.y, FluidCell());
}
ParticleLocator::~ParticleLocator() {
}

void ParticleLocator::update(std::vector<FluidParticlePtr>& particles) {
	// Clear All Cells
#pragma omp parallel for
	for (int n = 0; n < (int) cells.size(); n++) {
		cells[n].clear();
	}
	float scale = 1.0f / mVoxelSize;
	// Store Into The Cells
	for (FluidParticlePtr& p : particles) {
		int i = clamp((int) (scale * p->mLocation[0]), 0, mGridSize[0] - 1);
		int j = clamp((int) (scale * p->mLocation[1]), 0, mGridSize[1] - 1);
		int k = clamp((int) (scale * p->mLocation[2]), 0, mGridSize[2] - 1);
		getCell(i, j).push_back(p.get());
	}
}

FluidCell ParticleLocator::getNeigboringWallParticles(int i, int j, int w,
		int h) {
	FluidCell res;
	for (int si = max(i - w, 0); si <= min(i + w - 1, mGridSize[0] - 1); si++) {
		for (int sj = max(j - h, 0); sj <= min(j + h - 1, mGridSize[1] - 1);
				sj++) {
			for (FluidParticle* p : getCell(si, sj)) {
				res.push_back(p);
			}
		}
	}
	return res;
}

FluidCell ParticleLocator::getNeigboringCellParticles(int i, int j, int w,
		int h) {
	FluidCell res;
	for (int si = max(i - w, 0); si <= min(i + w, mGridSize[0] - 1); si++) {
		for (int sj = max(j - h, 0); sj <= min(j + h, mGridSize[1] - 1); sj++) {
			for (FluidParticle* p : getCell(si, sj)) {
				res.push_back(p);
			}
		}
	}
	return res;
}
size_t ParticleLocator::getParticleCount(int i, int j) const {
	return getCell(i, j).size();
}

float ParticleLocator::getLevelSetValue(int i, int j, Image1f& halfwall,
		float density) {
	float accm = 0.0;
	for (FluidParticle* p : getCell(i, j)) {
		if (p->mObjectType == ObjectType::FLUID) {
			accm += p->mDensity;
		} else {
			return 1.0;
		}
	}
	float MAX_VOLUME = 1.0 / (density * density * density);
	const float alpha = 0.2f;
	return alpha * MAX_VOLUME - accm;
}
void ParticleLocator::markAsWater(Image1ub& A, Image1f& halfwall,
		float density) {
#pragma omp parallel for
	for (int j = 0; j < A.height; j++) {
		for (int i = 0; i < A.width; i++) {
			A(i, j).x = static_cast<char>(ObjectType::AIR);
			for (FluidParticle* p : getCell(i, j)) {
				if (p->mObjectType == ObjectType::WALL) {
					A(i, j) = static_cast<char>(ObjectType::WALL);
					break;
				}
			}
			if (A(i, j).x != static_cast<char>(ObjectType::WALL))
				A(i, j).x = static_cast<char>(
						getLevelSetValue(i, j, halfwall, density) < 0.0 ?
								ObjectType::FLUID : ObjectType::AIR);
		}
	}
}
void ParticleLocator::deleteAllParticles() {
#pragma omp parallel for
	for (int n = 0; n < (int) cells.size(); n++) {
		cells[n].clear();
	}
}

}
