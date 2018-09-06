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
#ifndef FLIPFLUIDSOLVER_H_
#define FLIPFLUIDSOLVER_H_
#include "segmentation/Simulation.h"
#include "ParticleLocator.h"
#include "SimulationObjects.h"
#include <AlloyDistanceField.h>
#include <AlloyMesh.h>
#include <AlloyIsoContour.h>
#include <segmentation/Manifold2D.h>
#include <segmentation/FluidParticles2D.h>
#include <segmentation/ManifoldCache2D.h>
namespace aly {
enum class MotionScheme {
	UNDEFINED, IMPLICIT, SEMI_IMPLICIT, EXPLICIT
};
class FluidSimulation: public Simulation {
protected:
	std::shared_ptr<ManifoldCache2D> cache;
	//Constant, even though gravity really isn't constant on earth.
	const static float GRAVITY;
	float maxDensity;
	float picFlipBlendWeight;
	float fluidParticleDiameter;
	float maxLevelSet;
	Manifold2D contour;
	IsoContour isoContour;
	DistanceField2f distanceField;
	Image2f lastVelocityImage;
	Image1ub labelImage;
	Image1f laplacianImage;
	Image1f divergenceImage;
	Image1f pessureImage;
	Image2f wallNormalImage;
	Image2f densityMapImage;
	Image1f wallWeightImage;
	Image1f fluidLevelSet;
	Image1f signedLevelSet;
	float2 interpolate(const Image2f& img, float2 pos);
	bool requestUpdateContour;
	int2 gridSize;
	float2 domainSize;
	int stuckParticleCount;
	float fluidVoxelSize;
	float wallThickness;
	std::mutex contourLock;
	std::unique_ptr<ParticleLocator> particleLocator;
	std::vector<std::shared_ptr<SimulationObject>> fluidObjects;
	std::vector<std::shared_ptr<SimulationObject>> wallObjects;
	std::vector<std::shared_ptr<SimulationObject>> airObjects;
	std::vector<FluidParticlePtr> particles;
	void copyGridToBuffer();
	void subtractGrid();
	void placeObjects();
	void placeWalls();
	void damBreakTest();
	void computeParticleDensity(float maxDensity);
	void computeWallNormals();
	void advectParticles();
	void solvePicFlip();
	void addExternalForce();
	void pourWater(int limit, float maxDensity);
	void extrapolateVelocity();
	void repositionParticles(std::vector<int>& indices);
	void addParticle(float2 pt, float2 center, ObjectType type);
	void project();
	void createLevelSet();
	void initLevelSet();
	void updateParticleVolume();
	void enforceBoundaryCondition();
	float smoothKernel(float r2, float h);
	float sharpKernel(float r2, float h);
	float distance(const float2& p0, const float2& p1);
	float distanceSquared(const float2& p0, const float2& p1);
	float lengthSquared(float a, float b, float c);
	void shuffleCoordinates(std::vector<int2> &waters);
	float linear(Image1f& q, float x, float y, float z);
	void resampleParticles(float2& p, float2& u, float re);
	void correctParticles(std::vector<FluidParticlePtr>& particle, float dt,
			float re);
	bool updateContour();
	double implicit_func(float2& p, float density);
	double implicit_func(std::vector<FluidParticle*> &neighbors, float2& p,
			float density, float voxelSize);
	void mapParticlesToGrid();
	void mapGridToParticles();

	inline float isWallIndicator(char a) {
		return ((a == static_cast<char>(ObjectType::WALL)) ? 1.0f : -1.0f);
	}
	void addFluid();
	void addSimulationObject(SimulationObject* obj);
	virtual bool stepInternal() override;
public:
	Manifold2D* getManifold();
	inline float getFluidVoxelSize() const {
		return fluidVoxelSize;
	}
	inline float getLevelSetVoxelSize() const {
		return fluidVoxelSize * 0.5f;
	}
	inline const aly::Image2f& getVelocity() const {
		return contour.fluidParticles.velocityImage;
	}
	inline const aly::Image1f& getPressure() const {
		return pessureImage;
	}
	inline const aly::Image1ub& getLabels() const {
		return labelImage;
	}
	const std::vector<std::shared_ptr<SimulationObject>>& getWallObjects() const {
		return wallObjects;
	}
	std::vector<std::shared_ptr<SimulationObject>>& getWallObjects() {
		return wallObjects;
	}

	inline const aly::Image1f& getSignedLevelSet() const {
		return signedLevelSet;
	}
	int2 dimensions() const {
		return gridSize;
	}
	FluidSimulation(const int2& dims, float voxelSize, MotionScheme scheme,
			const std::shared_ptr<ManifoldCache2D>& cache = nullptr);
	virtual bool init() override;
	virtual void cleanup() override;
	virtual void setup(const aly::ParameterPanePtr& pane) override;
	virtual ~FluidSimulation();
};

} /* namespace imagesci */

#endif /* FLIPFLUIDSOLVER_H_ */
