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
#include "SimulationObjects.h"
#include <AlloyImage.h>
#include <vector>
#ifndef _PARTICLE_LOCATOR_H
#define _PARTICLE_LOCATOR_H
namespace aly {
typedef std::vector<FluidParticle*> FluidCell;
class ParticleLocator {
protected:
	int2 mGridSize;
	float mVoxelSize;
	std::vector<FluidCell> cells;
public:
	ParticleLocator(int2 dims, float voxelSize);
	~ParticleLocator();

	void update(std::vector<FluidParticlePtr>& particles);
	FluidCell getNeigboringWallParticles(int i, int j,int w = 1, int h = 1);
	FluidCell getNeigboringCellParticles(int i, int j,int w = 1, int h = 1);
	float getLevelSetValue(int i, int j, Image1f& halfwall,float density);
	const int2& getGridSize() {
		return mGridSize;
	}
	float getVoxelSize() {
		return mVoxelSize;
	}
	size_t getParticleCount(int i, int j) const ;
	void markAsWater(Image1ub& A, Image1f& halfwall,float density);
	void deleteAllParticles();
	inline FluidCell& getCell(int i,int j){
		return cells[clamp(i,0,mGridSize.x-1)+clamp(j,0,mGridSize.y-1)*mGridSize.x];
	}
	inline const FluidCell& getCell(int i,int j) const {
		return cells[clamp(i,0,mGridSize.x-1)+clamp(j,0,mGridSize.y-1)*mGridSize.x];
	}
};
}
#endif
