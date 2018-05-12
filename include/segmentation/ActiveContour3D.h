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

#ifndef INCLUDE_ACTIVECONTOUR3D_H_
#define INCLUDE_ACTIVECONTOUR3D_H_
#include <AlloyVector.h>
#include <AlloyVolume.h>
#include <AlloyIsoSurface.h>
#include <vector>
#include <list>
#include <tuple>
#include <mutex>
#include "Simulation.h"
#include "Manifold3D.h"
#include "ManifoldCache3D.h"
namespace aly {
class ActiveContour3D: public Simulation {
protected:
	std::shared_ptr<ManifoldCache3D> cache;
	IsoSurface isoSurface;
	Manifold3D contour;

	bool clampSpeed;

	Number advectionParam;
	Number pressureParam;
	Number curvatureParam;
	Number targetPressureParam;

	const float MAX_DISTANCE = 3.5f;
	const int maxLayers = 3;
	bool requestUpdateSurface;
	Volume1f initialLevelSet;
	Volume1f levelSet;
	Volume1f swapLevelSet;
	Volume1f pressureImage;
	Volume3f vecFieldImage;
	std::vector<float> deltaLevelSet;
	std::vector<int3> activeList;
	std::mutex contourLock;
	aly::HorizontalSliderPtr pressureSlider,curavtureSlider,advectionSlider;
	bool getBitValue(int i);
	void rescale(aly::Volume1f& pressureForce);
	void pressureMotion(int i, int j, int k, size_t index);
	void pressureAndAdvectionMotion(int i, int j, int k, size_t index);
	void advectionMotion(int i, int j, int k, size_t index);
	void applyForces(int i, int j, int k, size_t index, float timeStep);
	void plugLevelSet(int i, int j, int k, size_t index);
	void updateDistanceField(int i, int j, int k, int band);
	int deleteElements();
	int addElements();
	virtual float evolve(float maxStep);
	void rebuildNarrowBand();

	bool updateSurface();
	virtual bool stepInternal() override;
public:
	ActiveContour3D(const std::shared_ptr<ManifoldCache3D>& cache = nullptr);
	ActiveContour3D(const std::string& name,const std::shared_ptr<ManifoldCache3D>& cache = nullptr);
	float evolve();
	Volume1f& getPressureImage();
	const Volume1f& getPressureImage() const;
	void setCurvature(float c) {
		curvatureParam.setValue(c);
	}
	void setClampSpeed(bool b) {
		clampSpeed = b;
	}
	void setPressure(const Volume1f& img, float weight, float target) {
		pressureParam.setValue(weight);
		targetPressureParam.setValue(target);
		pressureImage = img;
		rescale(pressureImage);
	}
	void setPressure(const Volume1f& img, float weight) {
		pressureParam.setValue(weight);
		pressureImage = img;
		rescale(pressureImage);
	}
	void setPressure(const Volume1f& img) {
		pressureImage = img;
		rescale(pressureImage);
	}
	void setPressureWeight(float weight) {
		pressureParam.setValue(weight);
	}
	void setVectorField(const Volume3f& img, float f) {
		advectionParam.setValue(f);
		vecFieldImage = img;
	}
	void setVectorFieldWeight(float c) {
		advectionParam.setValue(c);
	}
	void setAdvection(float c) {
		advectionParam.setValue(c);
	}
	Manifold3D* getSurface();
	Volume1f& getLevelSet();
	const Volume1f& getLevelSet() const;
	virtual bool init() override;
	virtual void cleanup() override;
	std::shared_ptr<ManifoldCache3D> getCache() const {
		return cache;
	}
	virtual void setup(const aly::ParameterPanePtr& pane) override;
	void setInitialDistanceField(const Volume1f& img) {
		initialLevelSet = img;
	}
};
}

#endif /* INCLUDE_ACTIVEManifold2D_H_ */
