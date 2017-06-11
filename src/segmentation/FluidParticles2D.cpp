/*
 * FluidParticles.cpp
 *
 *  Created on: Jun 7, 2017
 *      Author: blake
 */

#include <segmentation/FluidParticles2D.h>

namespace aly {
FluidParticles2D::FluidParticles2D():radius(1.0f) {
}
void FluidParticles2D::updateBounds() {
	float2 minPt = particles.min(1E30f);
	float2 maxPt = particles.max(-1E30f);
	bbox.position = minPt;
	bbox.dimensions = maxPt - minPt;
}
FluidParticles2D::~FluidParticles2D() {
	// TODO Auto-generated destructor stub
}

} /* namespace intel */
