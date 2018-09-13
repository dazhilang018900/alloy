/*
 * FluidParticles.h
 *
 *  Created on: Jun 7, 2017
 *      Author: blake
 */

#ifndef INCLUDE_SEGMENTATION_FLUIDPARTICLES2D_H_
#define INCLUDE_SEGMENTATION_FLUIDPARTICLES2D_H_
#include "math/AlloyVector.h"
#include "image/AlloyImage.h"
namespace aly {
class FluidParticles2D {
public:
	Vector2f particles;
	Vector2f velocities;
	Image2f velocityImage;
	float radius;
	box2f bbox;
	void updateBounds();
	FluidParticles2D();
	virtual ~FluidParticles2D();
	template<class Archive> void save(Archive & archive) const {
		archive(CEREAL_NVP(radius), CEREAL_NVP(particles), CEREAL_NVP(velocities),CEREAL_NVP(velocityImage));
	}
	template<class Archive> void load(Archive & archive)
	{
		archive(CEREAL_NVP(radius),CEREAL_NVP(particles), CEREAL_NVP(velocities),CEREAL_NVP(velocityImage));
		updateBounds();
	}
};

}

#endif /* INCLUDE_SEGMENTATION_FLUIDPARTICLES2D_H_ */
