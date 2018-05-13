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
#include "segmentation/MultiActiveContour3D.h"
namespace aly {
void MultiActiveContour3D::rebuildNarrowBand() {
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
	deltaLevelSet.resize(7 * activeList.size(), 0.0f);
	objectIds.resize(7 * activeList.size(), -1);
}
void MultiActiveContour3D::plugLevelSet(int i, int j, int k, size_t index) {
	int label = labelImage(i, j, k);
	int activeLabels[26];
	activeLabels[0] = labelImage(i + 1, j, k - 1);
	activeLabels[1] = labelImage(i - 1, j, k - 1);
	activeLabels[2] = labelImage(i, j + 1, k - 1);
	activeLabels[3] = labelImage(i, j - 1, k - 1);
	activeLabels[4] = labelImage(i - 1, j - 1, k - 1);
	activeLabels[5] = labelImage(i + 1, j - 1, k - 1);
	activeLabels[6] = labelImage(i - 1, j + 1, k - 1);
	activeLabels[7] = labelImage(i + 1, j + 1, k - 1);
	activeLabels[8] = labelImage(i, j, k - 1);

	activeLabels[9] = labelImage(i + 1, j, k);
	activeLabels[10] = labelImage(i - 1, j, k);
	activeLabels[11] = labelImage(i, j + 1, k);
	activeLabels[12] = labelImage(i, j - 1, k);
	activeLabels[13] = labelImage(i - 1, j - 1, k);
	activeLabels[14] = labelImage(i + 1, j - 1, k);
	activeLabels[15] = labelImage(i - 1, j + 1, k);
	activeLabels[16] = labelImage(i + 1, j + 1, k);

	activeLabels[17] = labelImage(i + 1, j, k + 1);
	activeLabels[18] = labelImage(i - 1, j, k + 1);
	activeLabels[19] = labelImage(i, j + 1, k + 1);
	activeLabels[20] = labelImage(i, j - 1, k + 1);
	activeLabels[21] = labelImage(i - 1, j - 1, k + 1);
	activeLabels[22] = labelImage(i + 1, j - 1, k + 1);
	activeLabels[23] = labelImage(i - 1, j + 1, k + 1);
	activeLabels[24] = labelImage(i + 1, j + 1, k + 1);
	activeLabels[25] = labelImage(i, j, k + 1);

	int count = 0;
	for (int index = 0; index < 26; index++) {
		if (label == activeLabels[index])
			count++;
	}
	//Pick any label other than this to fill the hole
	if (count == 0) {
		labelImage(i, j, k) = (i > 0) ? activeLabels[1] : activeLabels[0];
		levelSet(i, j, k) = 3.0f;
	}
}
void MultiActiveContour3D::cleanup() {
	if (cache.get() != nullptr)
		cache->clear();
}
bool MultiActiveContour3D::updateSurface() {
	if (requestUpdateSurface) {
		std::lock_guard<std::mutex> lockMe(contourLock);
		Mesh mesh;
		std::map<int,std::pair<size_t,size_t>> regions;
		isoSurface.solve(levelSet, labelImage, mesh, MeshType::Triangle, regions, true);
		mesh.updateVertexNormals(false, 4);
		contour.vertexColors.resize(mesh.vertexLocations.size());
		contour.vertexLabels.resize(mesh.vertexLocations.size());
		for(auto pr:regions){
			int l=pr.first;
			RGBAf c=objectColors[l].toRGBAf();
			for(size_t n=pr.second.first;n<pr.second.second;n++){
				contour.vertexColors[n]=c;
				contour.vertexLabels[n]=l;
			}
		}
		contour.vertexLocations = mesh.vertexLocations;
		contour.vertexNormals = mesh.vertexNormals;
		contour.triIndexes = mesh.triIndexes;
		contour.quadIndexes = mesh.quadIndexes;
		requestUpdateSurface = false;
		return true;
	}
	return false;
}
Manifold3D* MultiActiveContour3D::getSurface() {
	return &contour;
}
Volume1f& MultiActiveContour3D::getLevelSet() {
	return levelSet;
}
const Volume1f& MultiActiveContour3D::getLevelSet() const {
	return levelSet;
}
MultiActiveContour3D::MultiActiveContour3D(
		const std::shared_ptr<ManifoldCache3D>& cache) :
		Simulation("Active Contour 3D"), cache(cache), clampSpeed(false), requestUpdateSurface(
				false) {
	advectionParam = Float(1.0f);
	pressureParam = Float(0.0f);
	targetPressureParam = Float(0.5f);
	curvatureParam = Float(0.3f);
}

MultiActiveContour3D::MultiActiveContour3D(const std::string& name,
		const std::shared_ptr<ManifoldCache3D>& cache) :
		Simulation(name), cache(cache), clampSpeed(false), requestUpdateSurface(
				false) {
	advectionParam = Float(1.0f);
	pressureParam = Float(0.0f);
	targetPressureParam = Float(0.5f);
	curvatureParam = Float(0.3f);
}
float MultiActiveContour3D::evolve() {
	return evolve(0.5f);
}
Volume1f& MultiActiveContour3D::getPressureImage() {
	return pressureImage;
}

const Volume1f& MultiActiveContour3D::getPressureImage() const {
	return pressureImage;
}
void MultiActiveContour3D::setup(const aly::ParameterPanePtr& pane) {
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
void MultiActiveContour3D::setInitialLabels(const Volume1i& labels) {
	this->initialLabels = labels;
	this->swapLabelImage = labels;
	this->labelImage = labels;
	levelSet.resize(labels.rows, labels.cols, labels.slices);
	swapLevelSet.resize(labels.rows, labels.cols, labels.slices);
#pragma omp parallel for
	for (int k = 0; k < labels.slices; k++) {
		for (int j = 0; j < labels.cols; j++) {
			int activeLabels[6];
			for (int i = 0; i < labels.rows; i++) {
				int currentLabel = labels(i, j, k).x;
				activeLabels[0] = labels(i + 1, j, k).x;
				activeLabels[1] = labels(i - 1, j, k).x;
				activeLabels[2] = labels(i, j + 1, k).x;
				activeLabels[3] = labels(i, j - 1, k).x;
				activeLabels[4] = labels(i, j, k - 1).x;
				activeLabels[5] = labels(i, j, k + 1).x;
				float val = 1.0f;
				for (int n = 0; n < 6; n++) {
					if (currentLabel < activeLabels[n]) {
						val = 0.01f;
						break;
					}
				}
				levelSet(i, j, k) = float1(val);
				swapLevelSet(i, j, k) = float1(val);
			}
		}
	}
	for (int band = 1; band <= 2 * maxLayers; band++) {
#pragma omp parallel for
		for (int k = 0; k < labels.slices; k++) {
			for (int j = 0; j < labels.cols; j++) {
				for (int i = 0; i < labels.rows; i++) {
					updateDistanceField(i, j, k, band);
				}
			}
		}
	}
	initialLevelSet = levelSet;
}

bool MultiActiveContour3D::init() {
	int3 dims = initialLevelSet.dimensions();
	if (dims.x == 0 || dims.y == 0 || dims.z == 0)
		return false;
	simulationDuration = std::max(std::max(dims.x, dims.y), dims.z) * 1.75f;
	simulationIteration = 0;
	simulationTime = 0;
	timeStep = 1.0f;
	levelSet.resize(dims.x, dims.y, dims.z);
	swapLevelSet.resize(dims.x, dims.y, dims.z);
	labelImage.resize(dims.x, dims.y, dims.z);
	swapLabelImage.resize(dims.x, dims.y, dims.z);
#pragma omp parallel for
	for (int i = 0; i < (int) initialLevelSet.size(); i++) {
		float val = aly::clamp(initialLevelSet[i], 0.0f, (maxLayers + 1.0f));
		levelSet[i] = val;
		swapLevelSet[i] = val;
	}
	labelImage = initialLabels;
	swapLabelImage = initialLabels;
	std::set<int> labelSet;
	int L = 1;
	for (int1 l : initialLabels.data) {
		if (l.x != 0) {
			labelSet.insert(l.x);
			L = std::max(L, l.x + 1);
		}
	}
	forceIndexes.resize(L, -1);
	labelList.clear();
	labelList.assign(labelSet.begin(), labelSet.end());
	for (int i = 0; i < (int) labelList.size(); i++) {
		forceIndexes[labelList[i]] = i;
	}
	L = (int) labelList.size();
	if (L < 256) {
		objectColors.clear();
		objectColors[0] = RGBAf(0.0f, 0.0f, 0.0f, 0.0f);
		int CL = std::min(256, L);
		for (int i = 0; i < L; i++) {
			int l = labelList[i];
			objectColors[l] = HSVAtoColor(HSVA((l % CL) / (float) CL,0.7f,0.2*(l%2)+0.5f,1.0f));
		}
	} else {
		if ((int) objectColors.size() != L + 1) {
			objectColors.clear();
			objectColors[0] = RGBAf(0.0f, 0.0f, 0.0f, 0.0f);
			for (int i = 0; i < L; i++) {
				int l = labelList[i];
				HSV hsv = HSV(RandomUniform(0.0f, 1.0f),
						RandomUniform(0.5f, 1.0f), RandomUniform(0.5f, 1.0f));
				objectColors[l] = HSVtoColor(hsv);
			}
		}
	}
	rebuildNarrowBand();
	requestUpdateSurface = true;
	if (cache.get() != nullptr) {
		updateSurface();
		contour.setFile(
				MakeString() << GetDesktopDirectory() << ALY_PATH_SEPARATOR<< "surface" << std::setw(4) << std::setfill('0') << simulationIteration << ".bin");
		cache->set((int) simulationIteration, contour);
	}
	return true;
}
float MultiActiveContour3D::getSwapLevelSetValue(int i, int j, int k,
		int l) const {
	if (swapLabelImage(i, j, k).x == l) {
		return -swapLevelSet(i, j, k);
	} else {
		return swapLevelSet(i, j, k);
	}
}
float MultiActiveContour3D::getLevelSetValue(int i, int j, int k, int l) const {
	if (labelImage(i, j, k).x == l) {
		return -levelSet(i, j, k);
	} else {
		return levelSet(i, j, k);
	}
}
float MultiActiveContour3D::getUnionLevelSetValue(int i, int j, int k,
		int l) const {
	int c = labelImage(i, j, k).x;
	if (c == l || c == 0) {
		return -levelSet(i, j, k);
	} else {
		return levelSet(i, j, k);
	}
}
float MultiActiveContour3D::getLevelSetValue(float x, float y, float z,
		int l) const {
	int i = static_cast<int>(std::floor(x));
	int j = static_cast<int>(std::floor(y));
	int k = static_cast<int>(std::floor(z));
	float rgb000 = getLevelSetValue(i, j, k, l);
	float rgb100 = getLevelSetValue(i + 1, j, k, l);
	float rgb110 = getLevelSetValue(i + 1, j + 1, k, l);
	float rgb010 = getLevelSetValue(i, j + 1, k, l);
	float rgb001 = getLevelSetValue(i, j, k + 1, l);
	float rgb101 = getLevelSetValue(i + 1, j, k + 1, l);
	float rgb111 = getLevelSetValue(i + 1, j + 1, k + 1, l);
	float rgb011 = getLevelSetValue(i, j + 1, k + 1, l);
	float dx = x - i;
	float dy = y - j;
	float dz = z - k;
	float lower = ((rgb000 * (1.0f - dx) + rgb100 * dx) * (1.0f - dy)
			+ (rgb010 * (1.0f - dx) + rgb110 * dx) * dy);
	float upper = ((rgb001 * (1.0f - dx) + rgb101 * dx) * (1.0f - dy)
			+ (rgb011 * (1.0f - dx) + rgb111 * dx) * dy);
	return (1.0f - dz) * lower + dz * upper;

}
float MultiActiveContour3D::getUnionLevelSetValue(float x, float y, float z,
		int l) const {
	int i = static_cast<int>(std::floor(x));
	int j = static_cast<int>(std::floor(y));
	int k = static_cast<int>(std::floor(z));
	float rgb000 = getUnionLevelSetValue(i, j, k, l);
	float rgb100 = getUnionLevelSetValue(i + 1, j, k, l);
	float rgb110 = getUnionLevelSetValue(i + 1, j + 1, k, l);
	float rgb010 = getUnionLevelSetValue(i, j + 1, k, l);
	float rgb001 = getUnionLevelSetValue(i, j, k + 1, l);
	float rgb101 = getUnionLevelSetValue(i + 1, j, k + 1, l);
	float rgb111 = getUnionLevelSetValue(i + 1, j + 1, k + 1, l);
	float rgb011 = getUnionLevelSetValue(i, j + 1, k + 1, l);
	float dx = x - i;
	float dy = y - j;
	float dz = z - k;
	float lower = ((rgb000 * (1.0f - dx) + rgb100 * dx) * (1.0f - dy)
			+ (rgb010 * (1.0f - dx) + rgb110 * dx) * dy);
	float upper = ((rgb001 * (1.0f - dx) + rgb101 * dx) * (1.0f - dy)
			+ (rgb011 * (1.0f - dx) + rgb111 * dx) * dy);
	return (1.0f - dz) * lower + dz * upper;
}
void MultiActiveContour3D::pressureAndAdvectionMotion(int i, int j, int k,
		size_t gid) {
	float v111 = swapLevelSet(i, j, k).x;
	float2 grad;
	if (v111 > 0.5f) {
		for (int index = 0; index < 7; index++) {
			deltaLevelSet[7 * gid + index] = 0;
		}
		return;
	}
	int activeLabels[7];
	activeLabels[0] = swapLabelImage(i, j, k).x;
	activeLabels[1] = swapLabelImage(i + 1, j, k).x;
	activeLabels[2] = swapLabelImage(i - 1, j, k).x;
	activeLabels[3] = swapLabelImage(i, j + 1, k).x;
	activeLabels[4] = swapLabelImage(i, j - 1, k).x;
	activeLabels[5] = swapLabelImage(i, j, k + 1).x;
	activeLabels[6] = swapLabelImage(i, j, k - 1).x;
	int label;
	float3 vec = vecFieldImage(i, j, k);
	float forceX = advectionParam.toFloat() * vec.x;
	float forceY = advectionParam.toFloat() * vec.y;
	float forceZ = advectionParam.toFloat() * vec.z;
	float pressureValue = pressureImage(i, j, k).x;
	for (int index = 0; index < 7; index++) {
		label = activeLabels[index];
		if (label == 0) {
			objectIds[7 * gid + index] = 0;
			deltaLevelSet[7 * gid + index] = 0;
		} else {
			objectIds[7 * gid + index] = label;
			if (forceIndexes[label] < 0) {
				deltaLevelSet[7 * gid + index] = 0.999f;
			} else {
				float v010 = getSwapLevelSetValue(i - 1, j, k - 1, label);
				float v120 = getSwapLevelSetValue(i, j + 1, k - 1, label);
				float v110 = getSwapLevelSetValue(i, j, k - 1, label);
				float v100 = getSwapLevelSetValue(i, j - 1, k - 1, label);
				float v210 = getSwapLevelSetValue(i + 1, j, k - 1, label);
				float v001 = getSwapLevelSetValue(i - 1, j - 1, k, label);
				float v011 = getSwapLevelSetValue(i - 1, j, k, label);
				float v101 = getSwapLevelSetValue(i, j - 1, k, label);
				float v211 = getSwapLevelSetValue(i + 1, j, k, label);
				float v201 = getSwapLevelSetValue(i + 1, j - 1, k, label);
				float v221 = getSwapLevelSetValue(i + 1, j + 1, k, label);
				float v021 = getSwapLevelSetValue(i - 1, j + 1, k, label);
				float v121 = getSwapLevelSetValue(i, j + 1, k, label);
				float v012 = getSwapLevelSetValue(i - 1, j, k + 1, label);
				float v122 = getSwapLevelSetValue(i, j + 1, k + 1, label);
				float v112 = getSwapLevelSetValue(i, j, k + 1, label);
				float v102 = getSwapLevelSetValue(i, j - 1, k + 1, label);
				float v212 = getSwapLevelSetValue(i + 1, j, k + 1, label);

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
						+ DyNegMax * DyNegMax + DyPosMin * DyPosMin
						+ DzNegMax * DzNegMax + DzPosMin * DzPosMin;
				float GradientSqrNeg = DxPosMax * DxPosMax + DxNegMin * DxNegMin
						+ DyPosMax * DyPosMax + DyNegMin * DyNegMin
						+ DzPosMax * DzPosMax + DzNegMin * DzNegMin;

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
								- 2 * DxCtr * DyCtr * DxyCtr
								- 2 * DxCtr * DzCtr * DxzCtr
								- 2 * DyCtr * DzCtr * DyzCtr);
				float denom = DxCtr * DxCtr + DyCtr * DyCtr + DzCtr * DzCtr;
				float kappa = 0;
				const float maxCurvatureForce = 10.0f;
				if (fabs(denom) > 1E-5f) {
					kappa = curvatureParam.toFloat() * numer / denom;
				} else {
					kappa = curvatureParam.toFloat() * numer * sign(denom)
							* 1E5;
				}
				if (kappa < -maxCurvatureForce) {
					kappa = -maxCurvatureForce;
				} else if (kappa > maxCurvatureForce) {
					kappa = maxCurvatureForce;
				}
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
				float force = pressureParam.toFloat() * pressureValue;
				float pressure = 0;
				if (force > 0) {
					pressure = -force * std::sqrt(GradientSqrPos);
				} else if (force < 0) {
					pressure = -force * std::sqrt(GradientSqrNeg);
				}
				deltaLevelSet[7 * gid + index] = -advection + kappa + pressure;
			}
		}
	}
}
void MultiActiveContour3D::advectionMotion(int i, int j, int k, size_t gid) {
	float v111 = swapLevelSet(i, j, k).x;
	float2 grad;
	if (v111 > 0.5f) {
		for (int index = 0; index < 7; index++) {
			deltaLevelSet[7 * gid + index] = 0;
		}
		return;
	}
	int activeLabels[7];
	activeLabels[0] = swapLabelImage(i, j, k).x;
	activeLabels[1] = swapLabelImage(i + 1, j, k).x;
	activeLabels[2] = swapLabelImage(i - 1, j, k).x;
	activeLabels[3] = swapLabelImage(i, j + 1, k).x;
	activeLabels[4] = swapLabelImage(i, j - 1, k).x;
	activeLabels[5] = swapLabelImage(i, j, k + 1).x;
	activeLabels[6] = swapLabelImage(i, j, k - 1).x;
	int label;
	float3 vec = vecFieldImage(i, j, k);
	float forceX = advectionParam.toFloat() * vec.x;
	float forceY = advectionParam.toFloat() * vec.y;
	float forceZ = advectionParam.toFloat() * vec.z;
	for (int index = 0; index < 7; index++) {
		label = activeLabels[index];
		if (label == 0) {
			objectIds[7 * gid + index] = 0;
			deltaLevelSet[7 * gid + index] = 0;
		} else {
			objectIds[7 * gid + index] = label;
			if (forceIndexes[label] < 0) {
				deltaLevelSet[7 * gid + index] = 0.999f;
			} else {
				float v010 = getSwapLevelSetValue(i - 1, j, k - 1, label);
				float v120 = getSwapLevelSetValue(i, j + 1, k - 1, label);
				float v110 = getSwapLevelSetValue(i, j, k - 1, label);
				float v100 = getSwapLevelSetValue(i, j - 1, k - 1, label);
				float v210 = getSwapLevelSetValue(i + 1, j, k - 1, label);
				float v001 = getSwapLevelSetValue(i - 1, j - 1, k, label);
				float v011 = getSwapLevelSetValue(i - 1, j, k, label);
				float v101 = getSwapLevelSetValue(i, j - 1, k, label);
				float v211 = getSwapLevelSetValue(i + 1, j, k, label);
				float v201 = getSwapLevelSetValue(i + 1, j - 1, k, label);
				float v221 = getSwapLevelSetValue(i + 1, j + 1, k, label);
				float v021 = getSwapLevelSetValue(i - 1, j + 1, k, label);
				float v121 = getSwapLevelSetValue(i, j + 1, k, label);
				float v012 = getSwapLevelSetValue(i - 1, j, k + 1, label);
				float v122 = getSwapLevelSetValue(i, j + 1, k + 1, label);
				float v112 = getSwapLevelSetValue(i, j, k + 1, label);
				float v102 = getSwapLevelSetValue(i, j - 1, k + 1, label);
				float v212 = getSwapLevelSetValue(i + 1, j, k + 1, label);

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
								- 2 * DxCtr * DyCtr * DxyCtr
								- 2 * DxCtr * DzCtr * DxzCtr
								- 2 * DyCtr * DzCtr * DyzCtr);
				float denom = DxCtr * DxCtr + DyCtr * DyCtr + DzCtr * DzCtr;
				float kappa = 0;
				const float maxCurvatureForce = 10.0f;
				if (fabs(denom) > 1E-5f) {
					kappa = curvatureParam.toFloat() * numer / denom;
				} else {
					kappa = curvatureParam.toFloat() * numer * sign(denom)
							* 1E5;
				}
				if (kappa < -maxCurvatureForce) {
					kappa = -maxCurvatureForce;
				} else if (kappa > maxCurvatureForce) {
					kappa = maxCurvatureForce;
				}
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
				deltaLevelSet[7 * gid + index] = -advection + kappa;
			}
		}
	}

}
void MultiActiveContour3D::applyForces(int i, int j, int k, size_t gid,
		float timeStep) {
	if (swapLevelSet(i, j, k).x > 0.5f)
		return;
	float minValue1 = 1E10f;
	float minValue2 = 1E10f;
	int minLabel1 = -1;
	int minLabel2 = -1;
	int mask = 0;
	float delta;
	float update = 0;
	for (int l = 0; l < 7; l++) {
		mask = objectIds[7 * gid + l];
		if (clampSpeed) {
			delta = timeStep * clamp(deltaLevelSet[7 * gid + l], -1.0f, 1.0f);
		} else {
			delta = timeStep * deltaLevelSet[7 * gid + l];
		}
		if (mask != -1) {
			update = getLevelSetValue(i, j, k, mask) + delta;
			if (mask != minLabel1 && mask != minLabel2) {
				if (update < minValue1) {
					minValue2 = minValue1;
					minLabel2 = minLabel1;
					minValue1 = update;
					minLabel1 = mask;
				} else if (update < minValue2) {
					minValue2 = update;
					minLabel2 = mask;
				}
			}
		}
	}
	if (minLabel2 >= 0) {
		if (minValue1 == minValue2) {
			labelImage(i, j, k).x = min(minLabel1, minLabel2);
		} else {
			labelImage(i, j, k).x = minLabel1;
		}
		levelSet(i, j, k).x = std::abs(0.5f * (float) (minValue1 - minValue2));
	} else if (minValue1 < 1E10f) {
		labelImage(i, j, k) = minLabel1;
		levelSet(i, j, k) = std::abs(minValue1);
	}
}

int MultiActiveContour3D::deleteElements() {
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
int MultiActiveContour3D::addElements() {
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
void MultiActiveContour3D::pressureMotion(int i, int j, int k, size_t gid) {
	float v111 = swapLevelSet(i, j, k).x;
	float2 grad;
	if (v111 > 0.5f) {
		for (int index = 0; index < 7; index++) {
			deltaLevelSet[7 * gid + index] = 0;
		}
		return;
	}
	int activeLabels[7];
	activeLabels[0] = swapLabelImage(i, j, k).x;
	activeLabels[1] = swapLabelImage(i + 1, j, k).x;
	activeLabels[2] = swapLabelImage(i - 1, j, k).x;
	activeLabels[3] = swapLabelImage(i, j + 1, k).x;
	activeLabels[4] = swapLabelImage(i, j - 1, k).x;
	activeLabels[5] = swapLabelImage(i, j, k + 1).x;
	activeLabels[6] = swapLabelImage(i, j, k - 1).x;
	int label;
	float pressureValue =
			(pressureImage.size() > 0) ? pressureImage(i, j, k).x : 0.0f;
	for (int index = 0; index < 7; index++) {
		label = activeLabels[index];
		if (label == 0) {
			objectIds[7 * gid + index] = 0;
			deltaLevelSet[7 * gid + index] = 0;
		} else {
			objectIds[7 * gid + index] = label;
			if (forceIndexes[label] < 0) {
				deltaLevelSet[7 * gid + index] = 0.999f;
			} else {
				float v010 = getSwapLevelSetValue(i - 1, j, k - 1, label);
				float v120 = getSwapLevelSetValue(i, j + 1, k - 1, label);
				float v110 = getSwapLevelSetValue(i, j, k - 1, label);
				float v100 = getSwapLevelSetValue(i, j - 1, k - 1, label);
				float v210 = getSwapLevelSetValue(i + 1, j, k - 1, label);
				float v001 = getSwapLevelSetValue(i - 1, j - 1, k, label);
				float v011 = getSwapLevelSetValue(i - 1, j, k, label);
				float v101 = getSwapLevelSetValue(i, j - 1, k, label);
				float v211 = getSwapLevelSetValue(i + 1, j, k, label);
				float v201 = getSwapLevelSetValue(i + 1, j - 1, k, label);
				float v221 = getSwapLevelSetValue(i + 1, j + 1, k, label);
				float v021 = getSwapLevelSetValue(i - 1, j + 1, k, label);
				float v121 = getSwapLevelSetValue(i, j + 1, k, label);
				float v012 = getSwapLevelSetValue(i - 1, j, k + 1, label);
				float v122 = getSwapLevelSetValue(i, j + 1, k + 1, label);
				float v112 = getSwapLevelSetValue(i, j, k + 1, label);
				float v102 = getSwapLevelSetValue(i, j - 1, k + 1, label);
				float v212 = getSwapLevelSetValue(i + 1, j, k + 1, label);

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
						+ DyNegMax * DyNegMax + DyPosMin * DyPosMin
						+ DzNegMax * DzNegMax + DzPosMin * DzPosMin;
				float GradientSqrNeg = DxPosMax * DxPosMax + DxNegMin * DxNegMin
						+ DyPosMax * DyPosMax + DyNegMin * DyNegMin
						+ DzPosMax * DzPosMax + DzNegMin * DzNegMin;

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
								- 2 * DxCtr * DyCtr * DxyCtr
								- 2 * DxCtr * DzCtr * DxzCtr
								- 2 * DyCtr * DzCtr * DyzCtr);
				float denom = DxCtr * DxCtr + DyCtr * DyCtr + DzCtr * DzCtr;
				float kappa = 0;
				const float maxCurvatureForce = 10.0f;
				if (fabs(denom) > 1E-5f) {
					kappa = curvatureParam.toFloat() * numer / denom;
				} else {
					kappa = curvatureParam.toFloat() * numer * sign(denom)
							* 1E5;
				}
				if (kappa < -maxCurvatureForce) {
					kappa = -maxCurvatureForce;
				} else if (kappa > maxCurvatureForce) {
					kappa = maxCurvatureForce;
				}
				float force = pressureParam.toFloat() * pressureValue;
				float pressure = 0;
				if (force > 0) {
					pressure = -force * std::sqrt(GradientSqrPos);
				} else if (force < 0) {
					pressure = -force * std::sqrt(GradientSqrNeg);
				}
				deltaLevelSet[7 * gid + index] = kappa + pressure;
			}
		}
	}
}
void MultiActiveContour3D::updateDistanceField(int i, int j, int k, int band) {
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
	int label = labelImage(i, j, k);
	v011 = getLevelSetValue(i - 1, j, k, label);
	v121 = getLevelSetValue(i, j + 1, k, label);
	v101 = getLevelSetValue(i, j - 1, k, label);
	v211 = getLevelSetValue(i + 1, j, k, label);
	v110 = getLevelSetValue(i, j, k - 1, label);
	v112 = getLevelSetValue(i, j, k + 1, label);
	if (levelSet(i, j, k).x > band - 0.5f) {
		v111 = (MAX_DISTANCE + 0.5f);
		v111 = min(std::abs(v011 - 1), v111);
		v111 = min(std::abs(v121 - 1), v111);
		v111 = min(std::abs(v101 - 1), v111);
		v111 = min(std::abs(v211 - 1), v111);
		v111 = min(std::abs(v110 - 1), v111);
		v111 = min(std::abs(v112 - 1), v111);
		levelSet(i, j, k) = v111;
	}
}

float MultiActiveContour3D::evolve(float maxStep) {
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
		swapLabelImage(pos.x, pos.y, pos.z) = labelImage(pos.x, pos.y, pos.z);
	}
	deleteElements();
	addElements();
	deltaLevelSet.resize(7 * activeList.size(), 0.0f);
	objectIds.resize(7 * activeList.size(), -1);
	return timeStep;
}
bool MultiActiveContour3D::stepInternal() {
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
void MultiActiveContour3D::rescale(aly::Volume1f& pressureForce) {
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
