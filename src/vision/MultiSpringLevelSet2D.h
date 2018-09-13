/*
* Copyright(C) 2016, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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

#ifndef INCLUDE_MULTISPRINGLEVELSET2D_H_
#define INCLUDE_MULTISPRINGLEVELSET2D_H_
#include "ui/AlloySimulation.h"
#include "vision/ManifoldCache2D.h"
#include "vision/MultiActiveContour2D.h"
#include "graphics/shaders/ContourShaders.h"
#include "graphics/AlloyLocator.h"
namespace aly {
	class MultiSpringLevelSet2D : public MultiActiveContour2D {
	public:
		static float MIN_ANGLE_TOLERANCE;
		static float NEAREST_NEIGHBOR_DISTANCE;
		static float PARTICLE_RADIUS;
		static float REST_RADIUS;
		static float EXTENT;
		static float SPRING_CONSTANT;
		static float SHARPNESS;
	protected:
		std::shared_ptr<Locator2f> locator;
		aly::Vector2f oldCorrespondences;
		std::array<Vector2f, 4> oldVelocities;
		aly::Vector2f oldVertexes;
		aly::Vector2f oldParticles;
		aly::Vector2f oldNormals;
		std::vector<int> oldLabels;
		aly::Image1f unsignedLevelSet;
		std::vector<std::vector<uint32_t>> nearestNeighbors;
		virtual bool stepInternal() override;
		float2 traceInitial(float2 pt);
		float2 traceUnsigned(float2 pt);
		void refineContour(bool signedIso);
		void refineContour(int iterations,float proximity,float stepSize);
		void updateNearestNeighbors(float maxDistance = NEAREST_NEIGHBOR_DISTANCE);
		void updateUnsignedLevelSet(float maxDistance = 4.0f*EXTENT,int label=-1);
		void relax(float timeStep);
		void relax();
		int fill();
		int contract();
		void updateTracking(float maxDistance = 2.0f*NEAREST_NEIGHBOR_DISTANCE);
		float advect(float maxStep = 0.33333f);
		void updateSignedLevelSet(float maxStep = 0.5f);
		float2 getScaledGradientValue(int i, int j);
		float2 getScaledGradientValue(float i, float j, bool signedIso);
		void distanceFieldMotion(int i, int j, size_t index);
		virtual void computeForce(size_t idx, float2& p1, float2& p2, float2& p);
		void relax(size_t idx, float timeStep, float2& f1, float2& f2);
		std::shared_ptr<UnsignedDistanceShader> unsignedShader;
		bool resampleEnabled;

	public:

		MultiSpringLevelSet2D(const std::shared_ptr<ManifoldCache2D>& cache = nullptr);
		void setSpringls(const Vector2f& particles, const Vector2f& points);
		virtual bool init() override;
		virtual void cleanup() override;
		virtual void setup(const aly::ParameterPanePtr& pane) override;
	};
}

#endif /* INCLUDE_SPRINGLEVELSET2D_H_ */
