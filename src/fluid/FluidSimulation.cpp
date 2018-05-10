/*
 * Copyright(C) 2014, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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
#include "fluid/FluidSimulation.h"
#include "fluid/LaplaceSolver.h"
namespace aly {
const float FluidSimulation::GRAVITY = 9.8067f;
const float RELAXATION_KERNEL_WIDTH = 1.4;
const float SPRING_STIFFNESS = 50.0;
static int frameCounter = 0;
void FluidSimulation::setup(const aly::ParameterPanePtr& pane) {
}
FluidSimulation::FluidSimulation(const int2& dims, float voxelSize,
		MotionScheme scheme, const std::shared_ptr<ManifoldCache2D>& cache) :
		Simulation("Fluid_Simulation"), cache(cache), maxDensity(0.0), stuckParticleCount(
				0), picFlipBlendWeight(0.95f), fluidParticleDiameter(0.5f), fluidVoxelSize(
				voxelSize), gridSize(dims), wallNormalImage(dims), fluidLevelSet(
				2 * dims), densityMapImage(2 * dims), labelImage(dims), laplacianImage(dims), divergenceImage(
				dims), pessureImage(dims), lastVelocityImage(
				dims + 1), wallWeightImage(dims), signedLevelSet(2 * dims) {
	contour.fluidParticles.velocityImage.resize(dims.x+1,dims.y+1);
	contour.fluidParticles.velocityImage.set(float2(0,0));
	wallThickness = fluidVoxelSize;
	srand(52372143L);
	maxLevelSet=2.5f;
	requestUpdateContour = false;
	timeStep = 0.5 * fluidVoxelSize;
	domainSize = float2(dims[0] * fluidVoxelSize, dims[1] * fluidVoxelSize);
	simulationDuration = 4.0f;
}
void FluidSimulation::computeParticleDensity(float maxDensity) {
	float scale = 1.0f / fluidVoxelSize;
#pragma omp parallel for
	for (int n = 0; n < (int) particles.size(); n++) {
		FluidParticlePtr& p = particles[n];
		if (p->mObjectType == ObjectType::WALL) {
			p->mDensity = 1.0;
			continue;
		}
		float2& pt = p->mLocation;
		int i = clamp((int) (scale * p->mLocation[0]), 0, gridSize.x - 1);
		int j = clamp((int) (scale * p->mLocation[1]), 0, gridSize.y - 1);
		//Search in small region
		std::vector<FluidParticle*> neighbors =
				particleLocator->getNeigboringCellParticles(i, j, 1, 1);
		float wsum = 0.0;
		//Density a function of how close particles are to their neighbors.
		for (FluidParticle* np : neighbors) {
			if (np->mObjectType == ObjectType::WALL)
				continue;
			float d2 = distanceSquared(np->mLocation, pt);
			float w = np->mMass
					* smoothKernel(d2,
							4.0f * fluidParticleDiameter * fluidVoxelSize);
			wsum += w;
		}
		//Estimate density in region using current particle configuration.
		p->mDensity = wsum / maxDensity;
	}
}
void FluidSimulation::placeWalls() {
	BoxObject* obj;
	int2 dims = labelImage.dimensions();
	float mx = fluidVoxelSize * dims[0];
	float my = fluidVoxelSize * dims[1];
	obj = new BoxObject;
	obj->mThickness = 3 * fluidVoxelSize;
	// Left Wall
	obj->mType = ObjectType::WALL;
	obj->mMaterial = ObjectMaterial::GLASS;
	obj->mVisible = false;
	obj->mMin = float2(0.0f, 0.0f);
	obj->mMax = float2(wallThickness, my);
	obj->mVoxelSize = fluidVoxelSize;
	addSimulationObject(static_cast<SimulationObject*>(obj));
	obj = new BoxObject;
	// Right Wall
	obj->mThickness = 3 * fluidVoxelSize;
	obj->mType = ObjectType::WALL;
	obj->mMaterial = ObjectMaterial::GLASS;
	obj->mVisible = false;
	obj->mMin = float2(mx - wallThickness, 0.0);
	obj->mMax = float2(mx, my);
	obj->mVoxelSize = fluidVoxelSize;
	addSimulationObject(static_cast<SimulationObject*>(obj));
	obj = new BoxObject;
	// Floor Wall
	obj->mThickness = 3 * fluidVoxelSize;
	obj->mType = ObjectType::WALL;
	obj->mMaterial = ObjectMaterial::GLASS;
	obj->mVisible = false;
	obj->mMin = float2(0.0, 0.0);
	obj->mMax = float2(mx, wallThickness);
	obj->mVoxelSize = fluidVoxelSize;
	addSimulationObject(static_cast<SimulationObject*>(obj));

	obj = new BoxObject;
	// Ceiling Wall
	obj->mThickness = 3 * fluidVoxelSize;
	obj->mType = ObjectType::WALL;
	obj->mMaterial = ObjectMaterial::SOLID;
	obj->mVisible = false;
	obj->mMin = float2(0.0, my - wallThickness);
	obj->mMax = float2(mx, my);
	obj->mVoxelSize = fluidVoxelSize;
	addSimulationObject(static_cast<SimulationObject*>(obj));
}
void FluidSimulation::placeObjects() {
	placeWalls();
	addFluid();
}
void FluidSimulation::addFluid() {
	//replace with level set for falling object
	BoxObject* obj = new BoxObject;
	int2 dims = labelImage.dimensions();
	float my = fluidVoxelSize * dims[1];

	obj->mType = ObjectType::FLUID;
	obj->mVisible = true;
	obj->mMin = float2(0.2f *dims[0]* fluidVoxelSize,my- 0.4f * fluidVoxelSize * dims[1]);
	obj->mMax = float2(0.4f *dims[0]* fluidVoxelSize,my- wallThickness);
	obj->mVoxelSize = fluidVoxelSize;
	addSimulationObject(static_cast<SimulationObject*>(obj));
	obj = new BoxObject;
	obj->mType = ObjectType::FLUID;
	obj->mVisible = true;
	obj->mMin = float2(wallThickness,my- 0.06 * fluidVoxelSize * dims[1]);
	obj->mMax = float2(fluidVoxelSize * dims[0] - wallThickness,my-  wallThickness);
	obj->mVoxelSize = fluidVoxelSize;
	addSimulationObject(static_cast<SimulationObject*>(obj));
}

void FluidSimulation::addSimulationObject(SimulationObject* obj) {
	switch (obj->mType) {
	case ObjectType::AIR:
		airObjects.push_back(std::shared_ptr<SimulationObject>(obj));
		break;
	case ObjectType::FLUID:
		fluidObjects.push_back(std::shared_ptr<SimulationObject>(obj));
		break;
	case ObjectType::WALL:
		wallObjects.push_back(std::shared_ptr<SimulationObject>(obj));
		break;

	}
}
void FluidSimulation::repositionParticles(std::vector<int>& indices) {
	if (indices.empty())
		return;
	// First Search for Deep Water
	std::vector<int2> waters;
	while (waters.size() < indices.size()) {
#pragma omp parallel
		for (int j = 0; j < labelImage.height; j++) {
			for (int i = 0; i < labelImage.width; i++) {
				if (i > 0
						&& labelImage(i - 1, j).x
								!= static_cast<char>(ObjectType::FLUID))
					continue;
				if (i < gridSize.x - 1
						&& labelImage(i + 1, j).x
								!= static_cast<char>(ObjectType::FLUID))
					continue;
				if (j > 0
						&& labelImage(i, j - 1).x
								!= static_cast<char>(ObjectType::FLUID))
					continue;
				if (j < gridSize.y - 1
						&& labelImage(i, j + 1).x
								!= static_cast<char>(ObjectType::FLUID))
					continue;
				if (labelImage(i, j).x != static_cast<char>(ObjectType::FLUID))
					continue;
				int2 aPos(i, j);
				waters.push_back(aPos);
				if (waters.size() >= indices.size()) {
					//Water is larger than particles! Set to max to break out of triple loop.
					i = gridSize.x;
					j = gridSize.y;
				}
			}
		}
	}
	if (waters.empty())
		return;
// Shuffle
	shuffleCoordinates(waters);
	for (int n = 0; n < indices.size(); n++) {
		FluidParticlePtr& p = particles[indices[n]];
		p->mLocation[0] = fluidVoxelSize
				* (waters[n][0] + 0.25 + 0.5 * (rand() % 101) / 100);
		p->mLocation[1] = fluidVoxelSize
				* (waters[n][1] + 0.25 + 0.5 * (rand() % 101) / 100);
	}
	particleLocator->update(particles);
	for (int n = 0; n < indices.size(); n++) {
		FluidParticlePtr &p = particles[indices[n]];
		float2 u(0.0f);
		resampleParticles(p->mLocation, u, fluidVoxelSize);
		p->mVelocity = u;
	}
}
void FluidSimulation::addParticle(float2 pt, float2 center, ObjectType type) {
	SimulationObject *inside_obj = nullptr;
	const int MAX_INT = std::numeric_limits<int>::max();
	const float MAX_ANGLE = 30.0f * ALY_PI / 180.0f;
	bool found = false;
	if (type == ObjectType::FLUID) {
		for (std::shared_ptr<SimulationObject>& obj : fluidObjects) {
			found = obj->inside(pt);
			if (found) {
				inside_obj = obj.get(); // Found
				break;
			}
		}
	} else if (type == ObjectType::WALL) {
		for (std::shared_ptr<SimulationObject>& obj : wallObjects) {
			found = obj->insideShell(pt);
			if (found) {
				inside_obj = obj.get(); // Found
				break;
			}
		}
	}
	if (inside_obj) {
		FluidParticle *p = new FluidParticle;
		//float2 axis(((rand() % MAX_INT) / (MAX_INT - 1.0)) * 2.0f - 1.0f,((rand() % MAX_INT) / (MAX_INT - 1.0)) * 2.0f - 1.0f);
		//axis=normalize(axis);
		float ang = MAX_ANGLE * (rand() % MAX_INT) / (MAX_INT - 1.0);
		float2x2 R;
		R(0, 0) = std::cos(ang);
		R(1, 0) = std::sin(ang);
		R(0, 1) = -std::sin(ang);
		R(1, 1) = std::cos(ang);
		if (inside_obj->mType == ObjectType::FLUID) {
			p->mLocation = center + R * (pt - center);
		} else {
			p->mLocation = pt;
		}
		p->mVelocity = float2(0.0);
		p->mNormal = float2(0.0);
		p->mDensity = 10.0;
		p->mObjectType = inside_obj->mType;
		p->mMass = 1.0;
		particles.push_back(FluidParticlePtr(p));
	}
}
bool FluidSimulation::init() {
	simulationTime = 0;
	simulationIteration = 0;
	pessureImage.set(float1(0.0f));
	particleLocator = std::unique_ptr<ParticleLocator>(
			new ParticleLocator(gridSize, fluidVoxelSize));
	placeObjects();
// This Is A Test Part. We Generate Pseudo Particles To Measure Maximum Particle Density
	float h = fluidParticleDiameter * fluidVoxelSize;
	for (int j = 0; j < 10; j++) {
		for (int i = 0; i < 10; i++) {
			FluidParticle *p = new FluidParticle;
			p->mLocation = float2((i + 0.5) * h, (j + 0.5) * h);
			p->mObjectType = ObjectType::FLUID;
			p->mMass = 1.0;
			particles.push_back(std::unique_ptr<FluidParticle>(p));
		}
	}
	particleLocator->update(particles);
	computeParticleDensity(1.0f);
	maxDensity = 0.0;
	for (FluidParticlePtr& p : particles) {
		maxDensity = max(maxDensity, p->mDensity);
	}
	particles.clear();
	float2 center;
	float2 pt;
// Place Fluid Particles And Walls
	int2 dims = labelImage.dimensions();
	float mx = fluidVoxelSize * dims[0];
	float my = fluidVoxelSize * dims[1];
	double wx = fluidParticleDiameter * fluidVoxelSize;
	double wy = fluidParticleDiameter * fluidVoxelSize;
	for (int j = 0; j < labelImage.height; j++) {
		for (int i = 0; i < labelImage.width; i++) {
			for (int ii = 0; ii < 2; ii++) {
				for (int jj = 0; jj < 2; jj++) {
					double x = wx * (2 * i + ii + 0.5);
					double y = wy * (2 * j + jj + 0.5);
					if (x > wallThickness && x < mx - wallThickness&& y > wallThickness && y < my - wallThickness) {
						center = float2(wx * (2 * i + 1), wy * (2 * j + 1));
						addParticle(float2(x, y), center, ObjectType::FLUID);
					}
				}
			}
		}
	}
	for (int j = 0; j < labelImage.height; j++) {
		for (int i = 0; i < labelImage.width; i++) {
			float x = wx * (2 * i + 1);
			float y = wy * (2 * j + 1);
			addParticle(float2(x, y), float2(x, y), ObjectType::WALL);
		}
	}
	particleLocator->update(particles);
	particleLocator->markAsWater(labelImage, wallWeightImage, fluidParticleDiameter);
// Remove Particles That Stuck On Wal Cells
	float scale = 1.0f / fluidVoxelSize;
	int eraseCount=0;
	for (std::vector<FluidParticlePtr>::iterator iter = particles.begin();
			iter != particles.end();) {
		FluidParticlePtr& p = *iter;
		if (p->mObjectType == ObjectType::WALL) {
			iter++;
			continue;
		}
		int i = clamp((int) (scale * p->mLocation[0]), 0, gridSize.x - 1);
		int j = clamp((int) (scale * p->mLocation[1]), 0, gridSize.y - 1);
		if (labelImage(i, j).x == static_cast<char>(ObjectType::WALL)) {
			iter = particles.erase(iter);
			eraseCount++;
		} else {
			iter++;
		}
	}
	computeWallNormals();
	updateParticleVolume();
	computeParticleDensity(maxDensity);
	solvePicFlip();
	initLevelSet();
	if (cache.get() != nullptr) {
		updateContour();
		contour.setFile(
				MakeString() << GetDesktopDirectory() << ALY_PATH_SEPARATOR<< "contour" << std::setw(4) << std::setfill('0') << simulationIteration << ".bin");
		cache->set((int) simulationIteration, contour);
	}
	return true;
}
void FluidSimulation::pourWater(int limit, float maxDensity) {
	if (simulationIteration > limit)
		return;
	float2 mPourPosition(0.0, 0.0);
	float mPourRadius(0.12);
	int cnt = 0;
	double w = fluidParticleDiameter * fluidVoxelSize;
	for (float x = w + w / 2.0; x < 1.0 - w / 2.0; x += w) {
		for (float z = w + w / 2.0; z < 1.0 - w / 2.0; z += w) {
			if (hypot(x - mPourPosition[0], z - mPourPosition[1])
					< mPourRadius) {
				FluidParticle *p = new FluidParticle;
				p->mLocation = float2(x,
						1.0 - wallThickness
								- 2.5 * fluidParticleDiameter
										* fluidVoxelSize);
				p->mVelocity = float2(0.0,
						-0.5 * fluidVoxelSize * fluidParticleDiameter
								/ timeStep);
				p->mNormal = float2(0.0);
				p->mObjectType = ObjectType::FLUID;
				p->mDensity = maxDensity;
				p->mMass = 1.0;
				particles.push_back(FluidParticlePtr(p));
				cnt++;
			}
		}
	}
}
void FluidSimulation::addExternalForce() {
	float velocity = timeStep * GRAVITY;
//Add gravity acceleration to all particles
	int count = 0;
#pragma omp parallel for
	for (int n = 0; n < (int) particles.size(); n++) {
		if (particles[n]->mObjectType == ObjectType::FLUID) {
			particles[n]->mVelocity[1] += velocity;
			count++;
		}
	}
}
float2 FluidSimulation::interpolate(const Image2f& img, float2 p) {
	float2 u;
	float scale = 1.0 / fluidVoxelSize;
	u.x = img(scale * p[0], scale * p[1] - 0.5, 0);
	u.y = img(scale * p[0] - 0.5, scale * p[1], 1);
	return u;
}
void FluidSimulation::advectParticles() {
// Advect Particle Through Grid
#pragma omp parallel for
	for (int n = 0; n < (int) particles.size(); n++) {
		FluidParticlePtr& p = particles[n];
		if (p->mObjectType == ObjectType::FLUID) {
			p->mLocation += ((float) timeStep)
					* interpolate(contour.fluidParticles.velocityImage, p->mLocation);
		}
	}
//Update localization
	particleLocator->update(particles);
	float re = 1.5f * fluidParticleDiameter * fluidVoxelSize;
	float r = wallThickness;
	float scale = 1.0f / fluidVoxelSize;
	float mx = fluidVoxelSize * gridSize.x;
	float my = fluidVoxelSize * gridSize.y;
//Correct particle locations
#pragma omp parallel for
	for (int n = 0; n < (int) particles.size(); n++) {
		FluidParticlePtr& p = particles[n];
		if (p->mObjectType == ObjectType::FLUID) {
			p->mLocation[0] = clamp(p->mLocation[0], r, mx - r);
			p->mLocation[1] = clamp(p->mLocation[1], r, my - r);
			int i = clamp((int) (p->mLocation[0] * scale), 0, gridSize.x - 1);
			int j = clamp((int) (p->mLocation[1] * scale), 0, gridSize.y - 1);
			std::vector<FluidParticle*> neighbors =
					particleLocator->getNeigboringCellParticles(i, j, 1, 1);
			for (int n = 0; n < neighbors.size(); n++) {
				FluidParticle *np = neighbors[n];
				if (np->mObjectType == ObjectType::WALL) {
					float dist = distance(p->mLocation, np->mLocation);
					if (dist < re) {
						float2 normal = np->mNormal;
						if (normal[0] == 0.0 && normal[1] == 0.0
								&& normal[2] == 0.0 && dist) {
							normal = (p->mLocation - np->mLocation) / dist;
						}
						p->mLocation += (re - dist) * normal;
						float dotprod = dot(p->mVelocity, normal);
						p->mVelocity -= dotprod * normal;
					}
				}
			}
		}
	}

// Remove Particles That Stuck On The Up-Down Wall Cells...
#pragma omp parallel for
	for (int n = 0; n < (int) particles.size(); n++) {
		FluidParticlePtr& p = particles[n];
		p->mRemoveIndicator = false;
		// Focus on Only Fluid Particle
		if (p->mObjectType == ObjectType::FLUID) {
			int i = clamp((int) (p->mLocation[0] * scale), 0, gridSize.x - 1);
			int j = clamp((int) (p->mLocation[1] * scale), 0, gridSize.y - 1);
			// If Stuck On Wall Cells Just Reposition
			if (labelImage(i, j).x == static_cast<char>(ObjectType::WALL)) {
				p->mRemoveIndicator = true;
			}
			i = clamp((int) (p->mLocation[0] * scale), 2, gridSize.x - 3);
			j = clamp((int) (p->mLocation[1] * scale), 2, gridSize.y - 3);
			if (p->mDensity < 0.04
					&& (labelImage(i, max(0, j - 1)).x
							== static_cast<char>(ObjectType::WALL)
							|| labelImage(i, min(gridSize.y - 1, j + 1)).x
									== static_cast<char>(ObjectType::WALL))) {
				// Put Into Reposition List
				p->mRemoveIndicator = true;
			}
		}

	}
// Reposition If Necessary
	std::vector<int> reposition_indices;
	size_t n = 0;
	for (FluidParticlePtr& p : particles) {
		if (p->mRemoveIndicator) {
			p->mRemoveIndicator = false;
			reposition_indices.push_back(n);
		}
		n++;
	}

// Store Stuck Particle Number
	stuckParticleCount = reposition_indices.size();
	repositionParticles(reposition_indices);
}
void FluidSimulation::cleanup() {
	if (cache.get() != nullptr)
		cache->clear();
	wallObjects.clear();
	fluidObjects.clear();
	airObjects.clear();
	particles.clear();
}
bool FluidSimulation::stepInternal() {
//Rebuild location data structure
	particleLocator->update(particles);
//Compute density for each cell, capped by max density as pre-computed
	computeParticleDensity(maxDensity);
//Add external gravity force
	addExternalForce();
	solvePicFlip();
	advectParticles();
	correctParticles(particles, timeStep, fluidParticleDiameter * fluidVoxelSize);
	createLevelSet();
	simulationIteration++;
	simulationTime = simulationIteration * timeStep;
	if (cache.get() != nullptr) {
		updateContour();
		contour.setFile(
				MakeString() << GetDesktopDirectory() << ALY_PATH_SEPARATOR<< "contour" << std::setw(4) << std::setfill('0') << simulationIteration << ".bin");
		cache->set((int) simulationIteration, contour);
	}
	return (simulationTime < simulationDuration);

	if (simulationTime <= simulationDuration && running) {
		return true;
	} else {
		return false;
	}
}
void FluidSimulation::copyGridToBuffer() {
	lastVelocityImage=contour.fluidParticles.velocityImage;
}

void FluidSimulation::subtractGrid() {
	contour.fluidParticles.velocityImage -= lastVelocityImage;
}
void FluidSimulation::enforceBoundaryCondition() {
// Set Boundary Velocity Zero
#pragma omp parallel for
	for (int j = 0; j < contour.fluidParticles.velocityImage.height; j++) {
		for (int i = 0; i < contour.fluidParticles.velocityImage.width; i++) {
			if (i == 0 || i == gridSize.x)
				contour.fluidParticles.velocityImage(i, j, 0) = 0.0f;
			if (i < gridSize.x && i > 0
					&& isWallIndicator(labelImage(i, j).x)
							* isWallIndicator(labelImage(i - 1, j).x) < 0) {
				contour.fluidParticles.velocityImage(i, j, 0) = 0.0f;
			}
		}
	}
#pragma omp parallel for
	for (int j = 0; j < contour.fluidParticles.velocityImage.height; j++) {
		for (int i = 0; i < contour.fluidParticles.velocityImage.width; i++) {
			if (j == 0 || j == gridSize.y)
				contour.fluidParticles.velocityImage(i, j, 1) = 0.0;
			if (j < gridSize.y && j > 0
					&& isWallIndicator(labelImage(i, j))
							* isWallIndicator(labelImage(i, j - 1)) < 0) {
				contour.fluidParticles.velocityImage(i, j, 1) = 0.0f;
			}
		}
	}
}
void FluidSimulation::project() {
// Cell Width
// Compute Divergence

#pragma omp parallel for
	for (int j = 0; j < labelImage.height; j++) {
		for (int i = 0; i < labelImage.width; i++) {
			if (labelImage(i, j).x == static_cast<char>(ObjectType::FLUID)) {
				divergenceImage(i, j).x = -((contour.fluidParticles.velocityImage(i + 1, j, 0)- contour.fluidParticles.velocityImage(i, j, 0) + contour.fluidParticles.velocityImage(i, j + 1, 1)- contour.fluidParticles.velocityImage(i, j, 1)) / fluidVoxelSize);
			} else {
				divergenceImage(i, j).x =0.0f;
			}
		}
	}
// Compute LevelSet
#pragma omp parallel for
	for (int j = 0; j < laplacianImage.height; j++) {
		for (int i = 0; i < laplacianImage.width; i++) {
			laplacianImage(i, j).x = particleLocator->getLevelSetValue(i, j,
					wallWeightImage, fluidParticleDiameter);
		}
	}
	SolveLaplace2d(labelImage, laplacianImage, pessureImage, divergenceImage, fluidVoxelSize);
// Subtract Pressure Gradient
#pragma omp parallel for
	for (int j = 0; j < contour.fluidParticles.velocityImage.height; j++) {
		for (int i = 0; i < contour.fluidParticles.velocityImage.width; i++) {
			if (i > 0 && i < gridSize.x) {
				float pf = pessureImage(i, j);
				float pb = pessureImage(i - 1, j);
				if (laplacianImage(i, j).x * laplacianImage(i - 1, j).x < 0.0) {
					pf = laplacianImage(i, j) < 0.0 ?
							pessureImage(i, j) :
							laplacianImage(i, j)
									/ std::min(1.0e-3f, laplacianImage(i - 1, j).x)
									* pessureImage(i - 1, j);
					pb = laplacianImage(i - 1, j) < 0.0 ?
							pessureImage(i - 1, j) :
							laplacianImage(i - 1, j)
									/ std::min(1.0e-6f, laplacianImage(i, j).x)
									* pessureImage(i, j);
				}
				contour.fluidParticles.velocityImage(i, j, 0) -= (pf - pb) / fluidVoxelSize;
			}
		}
	}
#pragma omp parallel for
	for (int j = 0; j < contour.fluidParticles.velocityImage.height; j++) {
		for (int i = 0; i < contour.fluidParticles.velocityImage.width; i++) {
			if (j > 0 && j < gridSize.y) {
				float pf = pessureImage(i, j);
				float pb = pessureImage(i, j - 1);
				if (laplacianImage(i, j) * laplacianImage(i, j - 1) < 0.0) {
					pf = laplacianImage(i, j) < 0.0 ?
							pessureImage(i, j) :
							laplacianImage(i, j)
									/ std::min(1.0e-3f, laplacianImage(i, j - 1).x)
									* pessureImage(i, j - 1);
					pb = laplacianImage(i, j - 1) < 0.0 ?
							pessureImage(i, j - 1) :
							laplacianImage(i, j - 1)
									/ std::min(1.0e-6f, laplacianImage(i, j).x)
									* pessureImage(i, j);
				}
				contour.fluidParticles.velocityImage(i, j, 1) -= (pf - pb) / fluidVoxelSize;
			}
		}
	}
}
void FluidSimulation::extrapolateVelocity() {
// Mark Fluid Cell Face
	Image2ub mark(gridSize + 1, fluidVoxelSize);
	Image2ub wall_mark(gridSize + 1, fluidVoxelSize);
#pragma omp parallel for
	for (int j = 0; j < mark.height; j++) {
		for (int i = 0; i < mark.width; i++) {
			mark(i, j, 0) = (i > 0
					&& labelImage(i - 1, j).x
							== static_cast<char>(ObjectType::FLUID))
					|| (i < gridSize.x
							&& labelImage(i, j).x
									== static_cast<char>(ObjectType::FLUID));
			wall_mark(i, j, 0) =
					(i <= 0
							|| labelImage(i - 1, j).x
									== static_cast<char>(ObjectType::WALL))
							&& (i >= gridSize.x
									|| labelImage(i, j).x
											== static_cast<char>(ObjectType::WALL));
		}
	}
#pragma omp parallel for
	for (int j = 0; j < mark.height; j++) {
		for (int i = 0; i < mark.width; i++) {
			mark(i, j, 1) = (j > 0
					&& labelImage(i, j - 1) == static_cast<char>(ObjectType::FLUID))
					|| (j < gridSize.y
							&& labelImage(i, j).x
									== static_cast<char>(ObjectType::FLUID));
			wall_mark(i, j, 1) = (j <= 0
					|| labelImage(i, j - 1) == static_cast<char>(ObjectType::WALL))
					&& (j >= gridSize.y
							|| labelImage(i, j).x
									== static_cast<char>(ObjectType::WALL));
		}
	}
#pragma omp parallel for
	for (int j = 0; j < mark.height; j++) {
		for (int i = 0; i < mark.width; i++) {
			for (int n = 0; n < 3; n++) {
				if (n != 0 && i > gridSize.x - 1)
					continue;
				if (n != 1 && j > gridSize.y - 1)
					continue;
				if (!mark(i, j, n) && wall_mark(i, j, n)) {
					int wsum = 0;
					float sum = 0.0;
					int q[][2] = { { i - 1, j }, { i + 1, j }, { i, j - 1 }, {
							i, j + 1 } };
					for (int qk = 0; qk < 4; qk++) {
						if (q[qk][0] >= 0
								&& q[qk][0] < gridSize.x + ((n == 0) ? 1 : 0)
								&& q[qk][1] >= 0
								&& q[qk][1]
										< gridSize.y + ((n == 1) ? 1 : 0)) {
							if (mark(q[qk][0], q[qk][1], n)) {
								wsum++;
								sum += contour.fluidParticles.velocityImage(q[qk][0], q[qk][1], n);
							}
						}
					}
					if (wsum)
						contour.fluidParticles.velocityImage(i, j, n) = sum / wsum;
				}
			}
		}
	}
}
void FluidSimulation::solvePicFlip() {
	particleLocator->update(particles);
	mapParticlesToGrid();
	//WriteImageToRawFile(MakeString()<<GetDesktopDirectory()<<ALY_PATH_SEPARATOR<<"velocity_"<<mSimulationIteration<<".xml",mVelocity);
	particleLocator->markAsWater(labelImage, wallWeightImage, fluidParticleDiameter);
	copyGridToBuffer();
	enforceBoundaryCondition();
	//WriteImageToRawFile(MakeString()<<GetDesktopDirectory()<<ALY_PATH_SEPARATOR<<"velocity_enforced"<<mSimulationIteration<<".xml",mVelocity);
	project();
	enforceBoundaryCondition();
	extrapolateVelocity();
#pragma omp parallel for
	for (int n = 0; n < (int) particles.size(); n++) {
		FluidParticlePtr& p = particles[n];
		float2 currentVelocity = interpolate(contour.fluidParticles.velocityImage, p->mLocation);
		float2 velocity = p->mVelocity + currentVelocity
				- interpolate(lastVelocityImage, p->mLocation);
		p->mVelocity = (1.0f - picFlipBlendWeight) * currentVelocity
				+ picFlipBlendWeight * velocity;
	}
}
void FluidSimulation::updateParticleVolume() {
	float voxelSize = getFluidVoxelSize();
	contour.fluidParticles.particles.clear();
	contour.fluidParticles.velocities.clear();
	contour.fluidParticles.radius = 0.5f * fluidParticleDiameter;
	for (int n = 0; n < (int) particles.size(); n++) {
		FluidParticle* p = particles[n].get();
		if (p->mObjectType == ObjectType::FLUID) {
			float2 l = p->mLocation / voxelSize;
			contour.fluidParticles.particles.push_back(l);
			contour.fluidParticles.velocities.push_back(p->mVelocity);
		}
	}
	/*
	std::cout << "Updated Particle Volume "
			<< contour.fluidParticles.particles.size() << " "
			<< contour.fluidParticles.particles.min() << " "
			<< contour.fluidParticles.particles.max() << std::endl;
			*/
}
void FluidSimulation::createLevelSet() {
// Create Density Field
	updateParticleVolume();
	float voxelSize = 0.5f * fluidVoxelSize;
#pragma omp parallel for
	for (int j = 0; j < fluidLevelSet.height; j++) {
		for (int i = 0; i < fluidLevelSet.width; i++) {
			float x = i * voxelSize;
			float y = j * voxelSize;
			float2 p(x, y);
			if (i == 0 || i == fluidLevelSet.width - 1 || j == 0
					|| j == fluidLevelSet.height - 1) {
				fluidLevelSet(i, j).x = maxLevelSet;
			} else {
				float value = implicit_func(p, fluidParticleDiameter);
				fluidLevelSet(i, j).x = clamp(2.0f * value, -maxLevelSet,
						maxLevelSet);
			}
		}
	}
	distanceField.solve(fluidLevelSet, signedLevelSet, 2 * maxLevelSet);
	requestUpdateContour = true;
	frameCounter++;
}
void FluidSimulation::initLevelSet() {
	float voxelSize = 0.5f * fluidVoxelSize;
#pragma omp parallel for
	for (int j = 0; j < fluidLevelSet.height; j++) {
		for (int i = 0; i < fluidLevelSet.width; i++) {
			float x = i * voxelSize;
			float y = j * voxelSize;
			float2 p(x, y);
			if (i == 0 || i == fluidLevelSet.width - 1 || j == 0
					|| j == fluidLevelSet.height - 1) {
				fluidLevelSet(i, j).x = maxLevelSet;
			} else {
				float value = 2 * maxLevelSet;
				for (std::shared_ptr<SimulationObject>& obj : fluidObjects) {
					value = std::min(obj->signedDistance(p), value);
				}
				fluidLevelSet(i, j).x = clamp(value / fluidVoxelSize,
						-maxLevelSet, maxLevelSet);
			}
		}
	}
	distanceField.solve(fluidLevelSet, signedLevelSet, 2 * maxLevelSet);
	requestUpdateContour = true;
}
float FluidSimulation::lengthSquared(float a, float b, float c) {
	return a * a + b * b + c * c;
}

float FluidSimulation::distanceSquared(const float2& p0, const float2& p1) {
	return lengthSqr(p0 - p1);
}

float FluidSimulation::distance(const float2& p0, const float2& p1) {
	return length(p0 - p1);
}

void FluidSimulation::shuffleCoordinates(std::vector<int2> &waters) {
	random_shuffle(waters.begin(), waters.end());
}

float FluidSimulation::smoothKernel(float r2, float h) {
	return max(1.0 - r2 / (h * h), 0.0);
}
float FluidSimulation::sharpKernel(float r2, float h) {
	return max(h * h / fmax(r2, 1.0e-5) - 1.0, 0.0);
}

void FluidSimulation::mapParticlesToGrid() {
// Compute Mapping
	int2 dims(contour.fluidParticles.velocityImage.width, contour.fluidParticles.velocityImage.height);
	float scale = 1.0f / fluidVoxelSize;
#pragma omp parallel for
	for (int j = 0; j < contour.fluidParticles.velocityImage.height; j++) {
		for (int i = 0; i < contour.fluidParticles.velocityImage.width; i++) {
			// Variales for Particle Sorter
			std::vector<FluidParticle*> neighbors;
			// Map X Grids
			if (j < dims[1]) {
				float2 px(i, j + 0.5);
				float sumw = 0.0;
				float sumx = 0.0;
				neighbors = particleLocator->getNeigboringWallParticles(i, j,
						1, 2);
				for (int n = 0; n < neighbors.size(); n++) {
					FluidParticle *p = neighbors[n];
					if (p->mObjectType == ObjectType::FLUID) {
						float x = clamp(scale * p->mLocation[0], 0.0f,
								(float) dims[0]);
						float y = clamp(scale * p->mLocation[1], 0.0f,
								(float) dims[1]);
						float2 pos(x, y);
						float w = p->mMass
								* sharpKernel(distanceSquared(pos, px),
										RELAXATION_KERNEL_WIDTH);
						sumx += w * p->mVelocity[0];
						sumw += w;
					}
				}
				contour.fluidParticles.velocityImage(i, j, 0) = sumw ? sumx / sumw : 0.0;
			}
			// Map Y Grids
			if (i < dims[0]) {
				float2 py(i + 0.5, j);
				float sumw = 0.0;
				float sumy = 0.0;
				neighbors = particleLocator->getNeigboringWallParticles(i, j,
						2, 1);
				for (int n = 0; n < neighbors.size(); n++) {
					FluidParticle *p = neighbors[n];
					if (p->mObjectType == ObjectType::FLUID) {
						float x = clamp(scale * p->mLocation[0], 0.0f,
								(float) dims[0]);
						float y = clamp(scale * p->mLocation[1], 0.0f,
								(float) dims[1]);
						float z = clamp(scale * p->mLocation[2], 0.0f,
								(float) dims[2]);
						float2 pos(x, y);
						float w = p->mMass
								* sharpKernel(distanceSquared(pos, py),
										RELAXATION_KERNEL_WIDTH);
						sumy += w * p->mVelocity[1];
						sumw += w;
					}
				}
				contour.fluidParticles.velocityImage(i, j, 1) = sumw ? sumy / sumw : 0.0;
			}
		}
	}
}
Manifold2D * FluidSimulation::getContour() {
	return &contour;
}
bool FluidSimulation::updateContour() {
	if (requestUpdateContour) {
		ImageRGBA& overlay=contour.overlay;
		overlay.resize(pessureImage.width,pessureImage.height);
		float maxP=fluidVoxelSize*1.5f;//pessureImage.max(0.0f).x;
		if(maxP>0.0f){
			for (int j = 0; j < pessureImage.height; j++) {
				for (int i = 0; i < pessureImage.width; i++) {
					overlay(i,j)=ColorMapToRGBA(0.5f+0.5f*clamp(pessureImage(i,j).x/maxP,0.0f,1.0f),ColorMap::FirebrickToBlue);
				}
			}
		} else {
			overlay.set(ColorMapToRGBA(0.5f,ColorMap::FirebrickToBlue));
		}
		std::lock_guard<std::mutex> lockMe(contourLock);
		isoContour.solve(signedLevelSet, contour.vertexes, contour.indexes,
				0.0f, TopologyRule2D::Unconstrained, Winding::Clockwise);
		for (float2& pt : contour.vertexes.data) {
			pt *= fluidVoxelSize * 0.5f;
		}
		requestUpdateContour = false;
		return true;
	}
	return false;
}
void FluidSimulation::resampleParticles(float2& p, float2& u, float re) {
// Variables for Neighboring Particles
	std::vector<FluidParticle*> neighbors;
	int2 cell_size = particleLocator->getGridSize();
	float wsum = 0.0;
	float2 save(u);
	u[0] = u[1] = 0.0;
	float scale = particleLocator->getVoxelSize();
	int i = clamp((int) (p[0] * scale), 0, cell_size[0] - 1);
	int j = clamp((int) (p[1] * scale), 0, cell_size[1] - 1);
// Gather Neighboring Particles
	neighbors = particleLocator->getNeigboringCellParticles(i, j, 1, 1);
	for (FluidParticle *np : neighbors) {
		if (np->mObjectType == ObjectType::FLUID) {
			float dist2 = distanceSquared(p, np->mLocation);
			float w = np->mMass * sharpKernel(dist2, re);
			u += w * np->mVelocity;
			wsum += w;
		}
	}
	if (wsum) {
		u /= wsum;
	} else {
		u = save;
	}
}

void FluidSimulation::correctParticles(std::vector<FluidParticlePtr>& particles,
		float dt, float re) {
// Variables for Neighboring Particles
	int2 cell_size = particleLocator->getGridSize();
	particleLocator->update(particles);
	float scale = 1.0f / particleLocator->getVoxelSize();
// Compute Pseudo Moved Point
#pragma omp parallel for
	for (int n = 0; n < (int) particles.size(); n++) {
		if (particles[n]->mObjectType == ObjectType::FLUID) {
			FluidParticle *p = particles[n].get();
			float2 spring(0.0f);
			int i = clamp((int) (p->mLocation[0] * scale), 0, cell_size[0] - 1);
			int j = clamp((int) (p->mLocation[1] * scale), 0, cell_size[1] - 1);
			std::vector<FluidParticle*> neighbors =
					particleLocator->getNeigboringCellParticles(i, j, 1, 1);
			for (int n = 0; n < neighbors.size(); n++) {
				FluidParticle *np = neighbors[n];
				if (p != np) {
					float dist = distance(p->mLocation, np->mLocation);
					float w = SPRING_STIFFNESS * np->mMass
							* smoothKernel(dist * dist, re);
					if (dist > 0.1 * re) {
						spring += w * (p->mLocation - np->mLocation) / dist
								* re;
					} else {
						if (np->mObjectType == ObjectType::FLUID) {
							spring += 0.01f * re / dt * (rand() % 101) / 100.0f;
						} else {
							spring += 0.05f * re / dt * np->mNormal;
						}
					}
				}
			}
			p->mTmp[0] = p->mLocation + dt * spring;
		}
	}
// Resample New Velocity
#pragma omp parallel for
	for (int n = 0; n < (int) particles.size(); n++) {
		if (particles[n]->mObjectType == ObjectType::FLUID) {
			FluidParticle *p = particles[n].get();
			p->mTmp[1] = p->mVelocity;
			resampleParticles(p->mTmp[0], p->mTmp[1], re);
		}
	}

// Update
#pragma omp parallel for
	for (int n = 0; n < (int) particles.size(); n++) {
		if (particles[n]->mObjectType == ObjectType::FLUID) {
			FluidParticle *p = particles[n].get();
			p->mLocation = p->mTmp[0];
			p->mVelocity = p->mTmp[1];
		}
	}
}
void FluidSimulation::mapGridToParticles() {
#pragma omp parallel for
	for (int n = 0; n < (int) particles.size(); n++) {
		FluidParticlePtr& p = particles[n];
		p->mVelocity = interpolate(contour.fluidParticles.velocityImage, p->mLocation);
	}
}
double FluidSimulation::implicit_func(std::vector<FluidParticle*>& neighbors,
		float2& p, float radius, float voxelSize) {
	double phi = 8.0f * radius;
	float scale = 1.0f / particleLocator->getVoxelSize();
	for (int m = 0; m < neighbors.size(); m++) {
		FluidParticle &np = *neighbors[m];
		double d = distance(np.mLocation, p) * scale;
		if (np.mObjectType == ObjectType::WALL) {
			if (d < radius)
				return 4.5 * radius;
			continue;
		}
		if (d < phi) {
			phi = d;
		}
	}
//std::cout<<"IMPLICIT "<<phi<<" "<<density<<" "<<voxelSize<<std::endl;
	return phi - radius;
}
double FluidSimulation::implicit_func(float2& p, float radius) {
	int2 cell_size = particleLocator->getGridSize();
	float scale = 1.0f / particleLocator->getVoxelSize();
	std::vector<FluidParticle *> neighbors =
			particleLocator->getNeigboringCellParticles(
					clamp((int) (p[0] * scale), 0, cell_size[0] - 1),
					clamp((int) (p[1] * scale), 0, cell_size[1] - 1), 2, 2);
	return implicit_func(neighbors, p, radius, particleLocator->getVoxelSize());
}
void FluidSimulation::computeWallNormals() {
// mParticleLocator Particles
	particleLocator->update(particles);
// Compute Wall Normal
	float scale = 1.0f / fluidVoxelSize;
	float mx = fluidVoxelSize * gridSize.x;
	float my = fluidVoxelSize * gridSize.y;
//#pragma omp parallel for
	for (int n = 0; n < (int) particles.size(); n++) {
		FluidParticlePtr& p = particles[n];
		int i = clamp((int) (p->mLocation[0] * scale), 0, gridSize.x - 1);
		int j = clamp((int) (p->mLocation[1] * scale), 0, gridSize.y - 1);
		wallNormalImage(i, j) = float2(0.0f);
		p->mNormal = float2(0.0);
		if (p->mObjectType == ObjectType::WALL) {
			if (p->mLocation[0] <= (mx + 0.1) * wallThickness) {
				p->mNormal[0] = 1.0;
			}
			if (p->mLocation[0] >= mx - (mx - 0.1) * wallThickness) {
				p->mNormal[0] = -1.0;
			}
			if (p->mLocation[1] <= (my + 0.1) * wallThickness) {
				p->mNormal[1] = 1.0;
			}
			if (p->mLocation[1] >= my - (my - 0.1) * wallThickness) {
				p->mNormal[1] = -1.0;
			}
			if (p->mNormal[0] == 0.0 && p->mNormal[1] == 0.0) {
				std::vector<FluidParticle*> neighbors =
						particleLocator->getNeigboringCellParticles(i, j, 3,
								3);
				for (int n = 0; n < (int) neighbors.size(); n++) {
					FluidParticle *np = neighbors[n];
					if (p.get() != np && np->mObjectType == ObjectType::WALL) {
						float d = distance(p->mLocation, np->mLocation);
						float w = 1.0 / d;
						p->mNormal += w * (p->mLocation - np->mLocation) / d;
					}
				}
			}
		}
		p->mNormal = normalize(p->mNormal);
		wallNormalImage(i, j) = p->mNormal;
	}

	particleLocator->update(particles);
	particleLocator->markAsWater(labelImage, wallWeightImage, fluidParticleDiameter);

// Compute Perimeter Normal
#pragma omp parallel for
	for (int j = 0; j < wallNormalImage.height; j++) {
		for (int i = 0; i < wallNormalImage.width; i++) {
			wallWeightImage(i, j) = 0.0f;
			if (labelImage(i, j).x != static_cast<char>(ObjectType::WALL)) {
				// For Every Nearby Cells
				int sum = 0;
				float2 norm(0.0f);
				int neighbors[][2] = { { i - 1, j }, { i + 1, j }, { i, j - 1 },
						{ i, j + 1 } };
				for (int m = 0; m < 4; m++) {
					int si = neighbors[m][0];
					int sj = neighbors[m][1];
					if (si < 0 || si > gridSize.x - 1 || sj < 0
							|| sj > gridSize.y - 1)
						continue;
					if (labelImage(si, sj).x
							== static_cast<char>(ObjectType::WALL)) {
						sum++;
						norm += wallNormalImage(si, sj);
					}
				}
				if (sum > 0) {
					norm = normalize(norm);
					wallNormalImage(i, j) = norm;
					wallWeightImage(i, j) = 1.0f;
				}
			}
		}
	}
}
FluidSimulation::~FluidSimulation() {
// TODO Auto-generated destructor stub
}
} /* namespace imagesci */
