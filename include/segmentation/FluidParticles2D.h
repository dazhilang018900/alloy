/*
 * FluidParticles.h
 *
 *  Created on: Jun 7, 2017
 *      Author: blake
 */

#ifndef INCLUDE_SEGMENTATION_FLUIDPARTICLES2D_H_
#define INCLUDE_SEGMENTATION_FLUIDPARTICLES2D_H_
#include <AlloyVector.h>
namespace aly {
class FluidParticles2D {
public:
	Vector2f particles;
	Vector2f velocities;
	float radius;
	box2f bbox;
	void updateBounds();
	FluidParticles2D();
	virtual ~FluidParticles2D();
	template<class Archive> void save(Archive & archive) const {
		archive(CEREAL_NVP(radius), CEREAL_NVP(particles), CEREAL_NVP(velocities));
	}
	template<class Archive> void load(Archive & archive)
	{
		archive(CEREAL_NVP(radius),CEREAL_NVP(particles), CEREAL_NVP(velocities));
		updateBounds();
	}
};

}

#endif /* INCLUDE_SEGMENTATION_FLUIDPARTICLES2D_H_ */
