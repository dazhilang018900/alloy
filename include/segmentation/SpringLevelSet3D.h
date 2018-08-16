/*
 * Copyright(C) 2018, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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

#ifndef INCLUDE_SPRINGLEVELSET3D_H_
#define INCLUDE_SPRINGLEVELSET3D_H_
#include <segmentation/ActiveManifold3D.h>
#include <segmentation/ManifoldCache3D.h>
#include "Simulation.h"
#include "ContourShaders.h"
#include "AlloyLocator.h"

namespace aly {
	void Decompose(const float2x2& M, float& theta, float& phi, float& sx, float& sy);
	float2x2 Compose(const float& theta,const float& phi,const float& sx,const float& sy);
	float2x2 MakeRigid(const float2x2& M);
	float2x2 MakeSimilarity(const float2x2& M);
	class SpringLevelSet3D : public ActiveManifold3D {
	public:
		static float MIN_ANGLE_TOLERANCE;
		static float NEAREST_NEIGHBOR_DISTANCE;
		static float PARTICLE_RADIUS;
		static float REST_RADIUS;
		static float EXTENT;
		static float SPRING_CONSTANT;
		static float SHARPNESS;
	protected:
		std::shared_ptr<Matcher3f> matcher;
		aly::Vector3f oldCorrespondences;
		std::array<Vector3f, 4> oldVelocities;
		aly::Vector3f oldPoints;
		aly::Volume1f unsignedLevelSet;
		std::vector<std::vector<uint32_t>> nearestNeighbors;
		virtual bool stepInternal() override;
		float3 traceInitial(float3 pt);
		float3 traceUnsigned(float3 pt);
		void refineContour(bool signedIso);
		void updateNearestNeighbors(float maxDistance = NEAREST_NEIGHBOR_DISTANCE);
		void updateUnsignedLevelSet(float maxDistance= 4.0f*EXTENT);
		void relax(float timeStep);
		void relax();
		int fill();
		int contract();
		void updateTracking(float maxDistance = 2.0f*NEAREST_NEIGHBOR_DISTANCE);
		float advect(float maxStep=0.33333f);
		void updateSignedLevelSet(float maxStep=0.5f);
		float3 getScaledGradientValue(int i, int j, int k);
		float3 getScaledGradientValue(float i, float j,float k, bool signedIso);
		void distanceFieldMotion(int i, int j, int k, size_t index);
		virtual void computeForce(size_t idx, float3& p1, float3& p2, float3& p);
		void relax(size_t idx, float timeStep, float3& f1, float3& f2);
		bool resampleEnabled;

	public:

		SpringLevelSet3D(const std::shared_ptr<ManifoldCache3D>& cache = nullptr);
		void setSpringls(const Vector3f& particles, const Vector3f& points);
		virtual bool init() override;
		virtual void cleanup() override;
		virtual void setup(const aly::ParameterPanePtr& pane) override;
	};
}

#endif /* INCLUDE_SPRINGLEVELSET2D_H_ */
