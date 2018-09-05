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
	struct SpringlEdge {
		uint32_t id;
		int8_t edgeId;
		float distance;
		SpringlEdge(uint32_t id = 0, int8_t nbr = -1, float _distance = 0) :id(id), edgeId(nbr), distance(_distance) {
		}
		friend bool operator<(const SpringlEdge& first,const SpringlEdge& second) {
			return (first.distance < second.distance);
		}
	};

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
		static float RELAX_TIMESTEP;
		static float SHARPNESS;
		static float FILL_DISTANCE;
		static float CONTRACT_DISTANCE;
		static float MIN_AREA;
		static float MAX_AREA;
		static float MIN_ASPECT_RATIO;
		static int MAX_NEAREST_NEIGHBORS;

	protected:
		std::shared_ptr<Matcher3f> matcher;
		aly::Vector3f oldCorrespondences;
		std::array<Vector3f, 4> oldVelocities;
		aly::Vector3f oldPoints;
		aly::Volume1f unsignedLevelSet;
		std::vector<std::vector<SpringlEdge>> nearestNeighbors;
		virtual bool stepInternal() override;
		float3 traceInitial(float3 pt);
		float3 traceUnsigned(float3 pt);
		void refineContour(aly::Mesh& isosurf,int iterations,float proximity,float stepSize);
		void updateNearestNeighbors(float maxDistance = NEAREST_NEIGHBOR_DISTANCE);
		void updateUnsignedLevelSet(float maxDistance= 3.5f);
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
		virtual void computeForce(size_t idx, float3& p1, float3& p2, float3& p3, float3& p);
		virtual void computeForce(size_t idx, float3& p1, float3& p2, float3& p3, float3& p4, float3& p);
		void relax(size_t idx, float timeStep, Vector3f& update);
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
