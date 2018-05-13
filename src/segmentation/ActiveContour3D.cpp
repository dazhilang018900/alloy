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
#include "segmentation/ActiveContour3D.h"

namespace aly {

void ActiveContour3D::rebuildNarrowBand() {
	activeList.clear();
	for (int band = 1; band <= maxLayers; band++) {
#pragma omp parallel for
		for (int i = 0; i < (int) activeList.size(); i++) {
			int3 pos = activeList[i];
			updateDistanceField(pos.x, pos.y, pos.z, band);
		}
	}
	for (int k = 0; k < swapLevelSet.slices; k++) {
		for (int j = 0; j < swapLevelSet.cols; j++) {
			for (int i = 0; i < swapLevelSet.rows; i++) {
				if (std::abs(swapLevelSet(i, j, k).x) <= MAX_DISTANCE) {
					activeList.push_back(int3(i, j, k));
				}
			}
		}
	}
	deltaLevelSet.clear();
	deltaLevelSet.resize(activeList.size(), 0.0f);
}
void ActiveContour3D::plugLevelSet(int i, int j, int k, size_t index) {
	float v111;
	float v011;
	float v121;
	float v101;
	float v211;
	float v112;
	float v110;
	v111 = levelSet(i, j, k);
	float sgn = aly::sign(v111);
	v111 = sgn * v111;
	v011 = sgn * levelSet(i - 1, j, k);
	v121 = sgn * levelSet(i, j + 1, k);
	v101 = sgn * levelSet(i, j - 1, k);
	v211 = sgn * levelSet(i + 1, j, k);
	v110 = sgn * levelSet(i, j, k - 1);
	v112 = sgn * levelSet(i, j, k + 1);
	//if (v111 > 0 && v111 < 0.5f && v011 > 0 && v121 > 0 && v101 > 0 && v211 > 0 && v112 > 0 && v110 > 0) {
	if (v111 <= 0 && v111 >= -0.5f && v011 < 0 && v121 < 0 && v101 < 0
			&& v211 < 0 && v112 < 0 && v110 < 0) {
		levelSet(i, j, k) = sgn * MAX_DISTANCE;
	}
}
void ActiveContour3D::cleanup() {
	if (cache.get() != nullptr)
		cache->clear();
}
bool ActiveContour3D::updateSurface() {
	if (requestUpdateSurface) {
		std::lock_guard<std::mutex> lockMe(contourLock);
		Mesh mesh;
		isoSurface.solve(levelSet, activeList, mesh, MeshType::Triangle, true, 0.0f);
		mesh.updateVertexNormals(false, 4);
		contour.vertexLocations = mesh.vertexLocations;
		contour.vertexNormals = mesh.vertexNormals;
		contour.triIndexes = mesh.triIndexes;
		contour.quadIndexes=mesh.quadIndexes;
		requestUpdateSurface = false;
		return true;
	}
	return false;
}
Manifold3D* ActiveContour3D::getSurface() {
	return &contour;
}
Volume1f& ActiveContour3D::getLevelSet() {
	return levelSet;
}
const Volume1f& ActiveContour3D::getLevelSet() const {
	return levelSet;
}
ActiveContour3D::ActiveContour3D(
		const std::shared_ptr<ManifoldCache3D>& cache) :
		Simulation("Active Contour 3D"), cache(cache), clampSpeed(false), requestUpdateSurface(
				false) {
	advectionParam = Float(1.0f);
	pressureParam = Float(0.0f);
	targetPressureParam = Float(0.5f);
	curvatureParam = Float(0.3f);
}

ActiveContour3D::ActiveContour3D(const std::string& name,
		const std::shared_ptr<ManifoldCache3D>& cache) :
		Simulation(name), cache(cache), clampSpeed(false), requestUpdateSurface(
				false) {
	advectionParam = Float(1.0f);
	pressureParam = Float(0.0f);
	targetPressureParam = Float(0.5f);
	curvatureParam = Float(0.3f);
}
float ActiveContour3D::evolve() {
	return evolve(0.5f);
}
Volume1f& ActiveContour3D::getPressureImage() {
	return pressureImage;
}

const Volume1f& ActiveContour3D::getPressureImage() const {
	return pressureImage;
}
void ActiveContour3D::setup(const aly::ParameterPanePtr& pane) {
	pane->addNumberField("Target Pressure", targetPressureParam, Float(0.0f),
			Float(1.0f));
	pane->addNumberField("Pressure Weight", pressureParam, Float(-2.0f),
			Float(2.0f));
	pane->addNumberField("Advection Weight", advectionParam, Float(-1.0f),
			Float(1.0f));
	pane->addNumberField("Curvature Weight", curvatureParam, Float(0.0f),
			Float(4.0f));
	pane->addCheckBox("Clamp Speed", clampSpeed);
}
bool ActiveContour3D::init() {
	int3 dims = initialLevelSet.dimensions();
	if (dims.x == 0 || dims.y == 0 || dims.z == 0)
		return false;
	simulationDuration = std::max(std::max(dims.x, dims.y), dims.z) * 1.75f;
	simulationIteration = 0;
	simulationTime = 0;
	timeStep = 1.0f;
	levelSet.resize(dims.x, dims.y, dims.z);
	swapLevelSet.resize(dims.x, dims.y, dims.z);
#pragma omp parallel for
	for (int i = 0; i < (int) initialLevelSet.size(); i++) {
		float val = clamp(initialLevelSet[i], -(maxLayers + 1.0f),
				(maxLayers + 1.0f));
		levelSet[i] = val;
		swapLevelSet[i] = val;
	}
	rebuildNarrowBand();
	requestUpdateSurface = true;
	if (cache.get() != nullptr) {
		cache->clear();
		updateSurface();
		contour.setFile(
				MakeString() << GetDesktopDirectory() << ALY_PATH_SEPARATOR<< "surface" << std::setw(4) << std::setfill('0') << simulationIteration << ".bin");
		cache->set((int) simulationIteration, contour);
	}
	return true;
}
void ActiveContour3D::pressureAndAdvectionMotion(int i, int j, int k,
		size_t gid) {
	float v111 = swapLevelSet(i, j, k).x;
	float2 grad;
	if (v111 > 0.5f || v111 < -0.5f) {
		deltaLevelSet[gid] = 0;
		return;
	}
	float v010 = swapLevelSet(i - 1, j, k - 1);
	float v120 = swapLevelSet(i, j + 1, k - 1);
	float v110 = swapLevelSet(i, j, k - 1);
	float v100 = swapLevelSet(i, j - 1, k - 1);
	float v210 = swapLevelSet(i + 1, j, k - 1);
	float v001 = swapLevelSet(i - 1, j - 1, k);
	float v011 = swapLevelSet(i - 1, j, k);
	float v101 = swapLevelSet(i, j - 1, k);
	float v211 = swapLevelSet(i + 1, j, k);
	float v201 = swapLevelSet(i + 1, j - 1, k);
	float v221 = swapLevelSet(i + 1, j + 1, k);
	float v021 = swapLevelSet(i - 1, j + 1, k);
	float v121 = swapLevelSet(i, j + 1, k);
	float v012 = swapLevelSet(i - 1, j, k + 1);
	float v122 = swapLevelSet(i, j + 1, k + 1);
	float v112 = swapLevelSet(i, j, k + 1);
	float v102 = swapLevelSet(i, j - 1, k + 1);
	float v212 = swapLevelSet(i + 1, j, k + 1);

	float DxNeg = v111 - v011;
	float DxPos = v211 - v111;
	float DyNeg = v111 - v101;
	float DyPos = v121 - v111;
	float DzNeg = v111 - v110;
	float DzPos = v112 - v111;
	float DxNegMin = min(DxNeg, 0.0f);
	float DxNegMax = max(DxNeg, 0.0f);
	float DxPosMin = min(DxPos, 0.0f);
	float DxPosMax = max(DxPos, 0.0f);
	float DyNegMin = min(DyNeg, 0.0f);
	float DyNegMax = max(DyNeg, 0.0f);
	float DyPosMin = min(DyPos, 0.0f);
	float DyPosMax = max(DyPos, 0.0f);
	float DzNegMin = min(DzNeg, 0.0f);
	float DzNegMax = max(DzNeg, 0.0f);
	float DzPosMin = min(DzPos, 0.0f);
	float DzPosMax = max(DzPos, 0.0f);

	float GradientSqrPos = DxNegMax * DxNegMax + DxPosMin * DxPosMin
			+ DyNegMax * DyNegMax + DyPosMin * DyPosMin + DzNegMax * DzNegMax
			+ DzPosMin * DzPosMin;
	float GradientSqrNeg = DxPosMax * DxPosMax + DxNegMin * DxNegMin
			+ DyPosMax * DyPosMax + DyNegMin * DyNegMin + DzPosMax * DzPosMax
			+ DzNegMin * DzNegMin;

	float DxCtr = 0.5f * (v211 - v011);
	float DyCtr = 0.5f * (v121 - v101);
	float DzCtr = 0.5f * (v112 - v110);
	float DxxCtr = v211 - v111 - v111 + v011;
	float DyyCtr = v121 - v111 - v111 + v101;
	float DzzCtr = v112 - v111 - v111 + v110;
	float DxyCtr = (v221 - v021 - v201 + v001) * 0.25f;
	float DxzCtr = (v212 - v012 - v210 + v010) * 0.25f;
	float DyzCtr = (v122 - v102 - v120 + v100) * 0.25f;

	float numer = 0.5f
			* ((DyyCtr + DzzCtr) * DxCtr * DxCtr
					+ (DxxCtr + DzzCtr) * DyCtr * DyCtr
					+ (DxxCtr + DyyCtr) * DzCtr * DzCtr
					- 2 * DxCtr * DyCtr * DxyCtr - 2 * DxCtr * DzCtr * DxzCtr
					- 2 * DyCtr * DzCtr * DyzCtr);
	float denom = DxCtr * DxCtr + DyCtr * DyCtr + DzCtr * DzCtr;
	float kappa = 0;
	const float maxCurvatureForce = 10.0f;
	if (fabs(denom) > 1E-5f) {
		kappa = curvatureParam.toFloat() * numer / denom;
	} else {
		kappa = curvatureParam.toFloat() * numer * sign(denom) * 1E5;
	}
	if (kappa < -maxCurvatureForce) {
		kappa = -maxCurvatureForce;
	} else if (kappa > maxCurvatureForce) {
		kappa = maxCurvatureForce;
	}
	float pressure = 0;
	// Level set force should be the opposite sign of advection force so it
	// moves in the direction of the force.

	float3 vec = vecFieldImage(i, j, k);
	float forceX = advectionParam.toFloat() * vec.x;
	float forceY = advectionParam.toFloat() * vec.y;
	float forceZ = advectionParam.toFloat() * vec.z;

	float advection = 0;
	// Dot product force with upwind gradient
	if (forceX > 0) {
		advection = forceX * DxNeg;
	} else if (forceX < 0) {
		advection = forceX * DxPos;
	}
	if (forceY > 0) {
		advection += forceY * DyNeg;
	} else if (forceY < 0) {
		advection += forceY * DyPos;
	}
	if (forceZ > 0) {
		advection += forceZ * DzNeg;
	} else if (forceZ < 0) {
		advection += forceZ * DzPos;
	}
	float force = pressureParam.toFloat() * pressureImage(i, j, k).x;
	if (force > 0) {
		pressure = -force * std::sqrt(GradientSqrPos);
	} else if (force < 0) {
		pressure = -force * std::sqrt(GradientSqrNeg);
	}
	deltaLevelSet[gid] = -advection + kappa + pressure;
}
void ActiveContour3D::advectionMotion(int i, int j, int k, size_t gid) {
	float v111 = swapLevelSet(i, j, k).x;
	float2 grad;
	if (v111 > 0.5f || v111 < -0.5f) {
		deltaLevelSet[gid] = 0;
		return;
	}

	float v010 = swapLevelSet(i - 1, j, k - 1);
	float v120 = swapLevelSet(i, j + 1, k - 1);
	float v110 = swapLevelSet(i, j, k - 1);
	float v100 = swapLevelSet(i, j - 1, k - 1);
	float v210 = swapLevelSet(i + 1, j, k - 1);
	float v001 = swapLevelSet(i - 1, j - 1, k);
	float v011 = swapLevelSet(i - 1, j, k);
	float v101 = swapLevelSet(i, j - 1, k);
	float v211 = swapLevelSet(i + 1, j, k);
	float v201 = swapLevelSet(i + 1, j - 1, k);
	float v221 = swapLevelSet(i + 1, j + 1, k);
	float v021 = swapLevelSet(i - 1, j + 1, k);
	float v121 = swapLevelSet(i, j + 1, k);
	float v012 = swapLevelSet(i - 1, j, k + 1);
	float v122 = swapLevelSet(i, j + 1, k + 1);
	float v112 = swapLevelSet(i, j, k + 1);
	float v102 = swapLevelSet(i, j - 1, k + 1);
	float v212 = swapLevelSet(i + 1, j, k + 1);

	float DxNeg = v111 - v011;
	float DxPos = v211 - v111;
	float DyNeg = v111 - v101;
	float DyPos = v121 - v111;
	float DzNeg = v111 - v110;
	float DzPos = v112 - v111;

	float DxCtr = 0.5f * (v211 - v011);
	float DyCtr = 0.5f * (v121 - v101);
	float DzCtr = 0.5f * (v112 - v110);
	float DxxCtr = v211 - v111 - v111 + v011;
	float DyyCtr = v121 - v111 - v111 + v101;
	float DzzCtr = v112 - v111 - v111 + v110;
	float DxyCtr = (v221 - v021 - v201 + v001) * 0.25f;
	float DxzCtr = (v212 - v012 - v210 + v010) * 0.25f;
	float DyzCtr = (v122 - v102 - v120 + v100) * 0.25f;

	float numer = 0.5f
			* ((DyyCtr + DzzCtr) * DxCtr * DxCtr
					+ (DxxCtr + DzzCtr) * DyCtr * DyCtr
					+ (DxxCtr + DyyCtr) * DzCtr * DzCtr
					- 2 * DxCtr * DyCtr * DxyCtr - 2 * DxCtr * DzCtr * DxzCtr
					- 2 * DyCtr * DzCtr * DyzCtr);
	float denom = DxCtr * DxCtr + DyCtr * DyCtr + DzCtr * DzCtr;

	float kappa = 0;

	const float maxCurvatureForce = 10.0f;
	if (fabs(denom) > 1E-5f) {
		kappa = curvatureParam.toFloat() * numer / denom;
	} else {
		kappa = curvatureParam.toFloat() * numer * sign(denom) * 1E5f;
	}
	if (kappa < -maxCurvatureForce) {
		kappa = -maxCurvatureForce;
	} else if (kappa > maxCurvatureForce) {
		kappa = maxCurvatureForce;
	}

	// Level set force should be the opposite sign of advection force so it
	// moves in the direction of the force.

	float3 vec = vecFieldImage(i, j, k);
	float forceX = advectionParam.toFloat() * vec.x;
	float forceY = advectionParam.toFloat() * vec.y;
	float forceZ = advectionParam.toFloat() * vec.z;
	float advection = 0;
	// Dot product force with upwind gradient
	if (forceX > 0) {
		advection = forceX * DxNeg;
	} else if (forceX < 0) {
		advection = forceX * DxPos;
	}
	if (forceY > 0) {
		advection += forceY * DyNeg;
	} else if (forceY < 0) {
		advection += forceY * DyPos;
	}
	if (forceZ > 0) {
		advection += forceZ * DzNeg;
	} else if (forceZ < 0) {
		advection += forceZ * DzPos;
	}

	deltaLevelSet[gid] = -advection + kappa;
}
void ActiveContour3D::applyForces(int i, int j, int k, size_t index,
		float timeStep) {
	float delta;
	float old = swapLevelSet(i, j, k);
	if (std::abs(old) > 0.5f)
		return;
	if (clampSpeed) {
		delta = timeStep * clamp(deltaLevelSet[index], -1.0f, 1.0f);
	} else {
		delta = timeStep * deltaLevelSet[index];
	}
	old += delta;
	levelSet(i, j, k) = float1(old);
}

int ActiveContour3D::deleteElements() {
	std::vector<int3> newList;
	for (int i = 0; i < (int) activeList.size(); i++) {
		int3 pos = activeList[i];
		float val = swapLevelSet(pos.x, pos.y, pos.z);
		if (std::abs(val) <= MAX_DISTANCE) {
			newList.push_back(pos);
		} else {
			val = sign(val) * (MAX_DISTANCE + 0.5f);
			levelSet(pos.x, pos.y, pos.z) = val;
			swapLevelSet(pos.x, pos.y, pos.z) = val;
		}
	}
	int diff = (int) (activeList.size() - newList.size());
	activeList = newList;
	return diff;
}
int ActiveContour3D::addElements() {
	const int xNeighborhood[6] = { -1, 1, 0, 0, 0, 0 };
	const int yNeighborhood[6] = { 0, 0, -1, 1, 0, 0 };
	const int zNeighborhood[6] = { 0, 0, 0, 0, -1, 1 };
	std::vector<int2> newList;
	int sz = (int) activeList.size();
	float INDICATOR = (float) std::max(std::max(levelSet.rows, levelSet.cols),
			levelSet.slices);
	for (int offset = 0; offset < 6; offset++) {
		int xOff = xNeighborhood[offset];
		int yOff = yNeighborhood[offset];
		int zOff = zNeighborhood[offset];
		for (int n = 0; n < sz; n++) {
			int3 pos = activeList[n];
			int3 pos2 = int3(pos.x + xOff, pos.y + yOff, pos.z + zOff);
			float val1 = std::abs(levelSet(pos.x, pos.y, pos.z));
			float val2 = std::abs(levelSet(pos2.x, pos2.y, pos2.z));
			if (val1 <= MAX_DISTANCE - 1.0f && val2 >= MAX_DISTANCE
					&& val2 < INDICATOR) {
				levelSet(pos2.x, pos2.y, pos2.z) = INDICATOR + offset;
			}
		}
	}
	for (int offset = 0; offset < 6; offset++) {
		int xOff = xNeighborhood[offset];
		int yOff = yNeighborhood[offset];
		int zOff = zNeighborhood[offset];
		for (int n = 0; n < sz; n++) {
			int3 pos = activeList[n];
			int3 pos2 = int3(pos.x + xOff, pos.y + yOff, pos.z + zOff);
			float val1 = levelSet(pos.x, pos.y, pos.z);
			float val2 = levelSet(pos2.x, pos2.y, pos2.z);
			if (std::abs(val1) <= MAX_DISTANCE - 1.0f
					&& val2 == INDICATOR + offset) {
				activeList.push_back(pos2);
				val2 = swapLevelSet(pos2.x, pos2.y, pos2.z);
				val2 = aly::sign(val2) * MAX_DISTANCE;
				swapLevelSet(pos2.x, pos2.y, pos2.z) = val2;
				levelSet(pos2.x, pos2.y, pos2.z) = val2;
			}
		}
	}
	return (int) (activeList.size() - sz);
}
void ActiveContour3D::pressureMotion(int i, int j, int k, size_t gid) {
	float v111 = swapLevelSet(i, j, k).x;
	float3 grad;
	if (std::abs(v111) > 0.5f) {
		deltaLevelSet[gid] = 0;
		return;
	}
	float v010 = swapLevelSet(i - 1, j, k - 1).x;
	float v120 = swapLevelSet(i, j + 1, k - 1).x;
	float v110 = swapLevelSet(i, j, k - 1).x;
	float v100 = swapLevelSet(i, j - 1, k - 1).x;
	float v210 = swapLevelSet(i + 1, j, k - 1).x;
	float v001 = swapLevelSet(i - 1, j - 1, k).x;
	float v011 = swapLevelSet(i - 1, j, k).x;
	float v101 = swapLevelSet(i, j - 1, k).x;
	float v211 = swapLevelSet(i + 1, j, k).x;
	float v201 = swapLevelSet(i + 1, j - 1, k).x;
	float v221 = swapLevelSet(i + 1, j + 1, k).x;
	float v021 = swapLevelSet(i - 1, j + 1, k).x;
	float v121 = swapLevelSet(i, j + 1, k).x;
	float v012 = swapLevelSet(i - 1, j, k + 1).x;
	float v122 = swapLevelSet(i, j + 1, k + 1).x;
	float v112 = swapLevelSet(i, j, k + 1).x;
	float v102 = swapLevelSet(i, j - 1, k + 1).x;
	float v212 = swapLevelSet(i + 1, j, k + 1).x;

	float DxNeg = v111 - v011;
	float DxPos = v211 - v111;
	float DyNeg = v111 - v101;
	float DyPos = v121 - v111;
	float DzNeg = v111 - v110;
	float DzPos = v112 - v111;

	float DxNegMin = std::min(DxNeg, 0.0f);
	float DxNegMax = std::max(DxNeg, 0.0f);
	float DxPosMin = std::min(DxPos, 0.0f);
	float DxPosMax = std::max(DxPos, 0.0f);
	float DyNegMin = std::min(DyNeg, 0.0f);
	float DyNegMax = std::max(DyNeg, 0.0f);
	float DyPosMin = std::min(DyPos, 0.0f);
	float DyPosMax = std::max(DyPos, 0.0f);
	float DzNegMin = std::min(DzNeg, 0.0f);
	float DzNegMax = std::max(DzNeg, 0.0f);
	float DzPosMin = std::min(DzPos, 0.0f);
	float DzPosMax = std::max(DzPos, 0.0f);

	float DxCtr = 0.5f * (v211 - v011);
	float DyCtr = 0.5f * (v121 - v101);
	float DzCtr = 0.5f * (v112 - v110);
	float DxxCtr = v211 - v111 - v111 + v011;
	float DyyCtr = v121 - v111 - v111 + v101;
	float DzzCtr = v112 - v111 - v111 + v110;
	float DxyCtr = (v221 - v021 - v201 + v001) * 0.25f;
	float DxzCtr = (v212 - v012 - v210 + v010) * 0.25f;
	float DyzCtr = (v122 - v102 - v120 + v100) * 0.25f;

	float numer = 0.5f
			* ((DyyCtr + DzzCtr) * DxCtr * DxCtr
					+ (DxxCtr + DzzCtr) * DyCtr * DyCtr
					+ (DxxCtr + DyyCtr) * DzCtr * DzCtr
					- 2 * DxCtr * DyCtr * DxyCtr - 2 * DxCtr * DzCtr * DxzCtr
					- 2 * DyCtr * DzCtr * DyzCtr);
	float denom = DxCtr * DxCtr + DyCtr * DyCtr + DzCtr * DzCtr;
	float kappa = 0;
	const float maxCurvatureForce = 10.0f;

	if (std::abs(denom) > 1E-5f) {
		kappa = curvatureParam.toFloat() * numer / denom;
	} else {
		kappa = curvatureParam.toFloat() * numer * sign(denom) * 1E5f;
	}
	if (kappa < -maxCurvatureForce) {
		kappa = -maxCurvatureForce;
	} else if (kappa > maxCurvatureForce) {
		kappa = maxCurvatureForce;
	}
	float force = pressureParam.toFloat() * pressureImage(i, j, k).x;
	float pressure = 0;
	if (force > 0) {
		float GradientSqrPos = DxNegMax * DxNegMax + DxPosMin * DxPosMin
				+ DyNegMax * DyNegMax + DyPosMin * DyPosMin
				+ DzNegMax * DzNegMax + DzPosMin * DzPosMin;
		pressure = -force * std::sqrt(GradientSqrPos);
	} else if (force < 0) {
		float GradientSqrNeg = DxPosMax * DxPosMax + DxNegMin * DxNegMin
				+ DyPosMax * DyPosMax + DyNegMin * DyNegMin
				+ DzPosMax * DzPosMax + DzNegMin * DzNegMin;
		pressure = -force * std::sqrt(GradientSqrNeg);
	}
	deltaLevelSet[gid] = kappa + pressure;
}
void ActiveContour3D::updateDistanceField(int i, int j, int k, int band) {
	float v111;
	float v011;
	float v121;
	float v101;
	float v211;
	float v110;
	float v112;
	float activeLevelSet = swapLevelSet(i, j, k).x;
	if (std::abs(activeLevelSet) <= 0.5f) {
		return;
	}
	v111 = levelSet(i, j, k);
	float oldVal = v111;
	v011 = levelSet(i - 1, j, k);
	v121 = levelSet(i, j + 1, k);
	v101 = levelSet(i, j - 1, k);
	v211 = levelSet(i + 1, j, k);
	v110 = levelSet(i, j, k - 1);
	v112 = levelSet(i, j, k + 1);

	if (v111 < -band + 0.5f) {
		v111 = -(MAX_DISTANCE + 0.5f);
		v111 = (v011 > 1) ? v111 : max(v011, v111);
		v111 = (v121 > 1) ? v111 : max(v121, v111);
		v111 = (v101 > 1) ? v111 : max(v101, v111);
		v111 = (v211 > 1) ? v111 : max(v211, v111);
		v111 = (v110 > 1) ? v111 : max(v110, v111);
		v111 = (v112 > 1) ? v111 : max(v112, v111);
		v111 -= 1.0f;
	} else if (v111 > band - 0.5f) {
		v111 = (MAX_DISTANCE + 0.5f);
		v111 = (v011 < -1) ? v111 : min(v011, v111);
		v111 = (v121 < -1) ? v111 : min(v121, v111);
		v111 = (v101 < -1) ? v111 : min(v101, v111);
		v111 = (v211 < -1) ? v111 : min(v211, v111);
		v111 = (v110 < -1) ? v111 : min(v110, v111);
		v111 = (v112 < -1) ? v111 : min(v112, v111);
		v111 += 1.0f;
	}

	if (oldVal * v111 > 0) {
		levelSet(i, j, k).x = v111;
	} else {
		levelSet(i, j, k).x = oldVal;
	}
}

float ActiveContour3D::evolve(float maxStep) {
	if (pressureImage.size() > 0) {
		if (vecFieldImage.size() > 0) {
#pragma omp parallel for
			for (int i = 0; i < (int) activeList.size(); i++) {
				int3 pos = activeList[i];
				pressureAndAdvectionMotion(pos.x, pos.y, pos.z, i);
			}
		} else {
#pragma omp parallel for
			for (int i = 0; i < (int) activeList.size(); i++) {
				int3 pos = activeList[i];
				pressureMotion(pos.x, pos.y, pos.z, i);
			}
		}
	} else if (vecFieldImage.size() > 0) {
#pragma omp parallel for
		for (int i = 0; i < (int) activeList.size(); i++) {
			int3 pos = activeList[i];
			advectionMotion(pos.x, pos.y, pos.z, i);
		}
	}
	float timeStep = (float) maxStep;
	if (!clampSpeed) {
		float maxDelta = 0.0f;
		for (float delta : deltaLevelSet) {
			maxDelta = std::max(std::abs(delta), maxDelta);
		}
		const float maxSpeed = 0.999f;
		timeStep = (float) (maxStep
				* ((maxDelta > maxSpeed) ? (maxSpeed / maxDelta) : maxSpeed));
	}
	contourLock.lock();
#pragma omp parallel for
	for (int i = 0; i < (int) activeList.size(); i++) {
		int3 pos = activeList[i];
		applyForces(pos.x, pos.y, pos.z, i, timeStep);
	}
	for (int band = 1; band <= maxLayers; band++) {
#pragma omp parallel for
		for (int i = 0; i < (int) activeList.size(); i++) {
			int3 pos = activeList[i];
			updateDistanceField(pos.x, pos.y, pos.z, band);
		}
	}
#pragma omp parallel for
	for (int i = 0; i < (int) activeList.size(); i++) {
		int3 pos = activeList[i];
		plugLevelSet(pos.x, pos.y, pos.z, i);
	}
	requestUpdateSurface = true;
	contourLock.unlock();

#pragma omp parallel for
	for (int i = 0; i < (int) activeList.size(); i++) {
		int3 pos = activeList[i];
		swapLevelSet(pos.x, pos.y, pos.z) = levelSet(pos.x, pos.y, pos.z);
	}
	deleteElements();
	addElements();
	deltaLevelSet.clear();
	deltaLevelSet.resize(activeList.size(), 0.0f);
	return timeStep;
}
bool ActiveContour3D::stepInternal() {
	double remaining = timeStep;
	double t = 0.0;
	do {
		float timeStep = evolve(std::min(0.5f, (float) remaining));
		t += (double) timeStep;
		remaining = timeStep - t;
	} while (remaining > 1E-5f);
	//WriteImageToRawFile(MakeDesktopFile(MakeString()<<"current_levelse"<<std::setw(4)<<std::setfill('0')<<mSimulationIteration<<".xml"),levelSet);
	simulationTime += t;
	simulationIteration++;
	if (cache.get() != nullptr) {
		updateSurface();
		contour.setFile(
				MakeString() << GetDesktopDirectory() << ALY_PATH_SEPARATOR<< "surface" << std::setw(4) << std::setfill('0') << simulationIteration << ".bin");
		cache->set((int) simulationIteration, contour);
	}
	return (simulationTime < simulationDuration);
}
void ActiveContour3D::rescale(aly::Volume1f& pressureForce) {
	float minValue = 1E30f;
	float maxValue = -1E30f;
	if (!std::isnan(targetPressureParam.toFloat())) {
		for (int i = 0; i < (int) pressureForce.size(); i++) {
			float val = pressureForce[i] - targetPressureParam.toFloat();
			minValue = std::min(val, minValue);
			maxValue = std::max(val, maxValue);
		}
	}
	float normMin = (std::abs(minValue) > 1E-4) ? 1 / std::abs(minValue) : 1;
	float normMax = (std::abs(maxValue) > 1E-4) ? 1 / std::abs(maxValue) : 1;
#pragma omp parallel for
	for (int i = 0; i < (int) pressureForce.size(); i++) {
		float val = pressureForce[i] - targetPressureParam.toFloat();
		if (val < 0) {
			pressureForce[i] = (float) (val * normMin);
		} else {
			pressureForce[i] = (float) (val * normMax);
		}
	}
}
}
