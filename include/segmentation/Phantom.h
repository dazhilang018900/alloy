/*
 * Phantom.h
 *
 *  Created on: Mar 25, 2018
 *      Author: blake
 */

#ifndef INCLUDE_SEGMENTATION_PHANTOM_H_
#define INCLUDE_SEGMENTATION_PHANTOM_H_

#include <AlloyVolume.h>
namespace aly {
enum class Heaviside {
	ARCTAN = 0, BINARY = 1, SIGMOID = 2, SIN = 3
};
class Phantom {
protected:
	float maxDistance;
	int rows, cols, slices;
	float noiseLevel;
	bool invert;
	float fuzziness;
	Heaviside heavisideType;
	Volume1f levelSet;
	void finish(const Volume1f& vol);
	float heaviside(float val, float fuzziness, Heaviside heavy);
	virtual void solveInternal(Volume1f& vol) = 0;
	const Volume1f& solve(bool distanceFieldOnly = false);
public:
	Phantom(int rows, int cols, int slices, float maxDist = 10.0f);
	inline const Volume1f& solveDistanceField() {
		return solve(true);
	}
	inline const Volume1f& solveLevelSet() {
		return solve(false);
	}
	const Volume1f& getLevelSet() const {
		return levelSet;
	}
	inline void setFuzziness(float fuzz) {
		fuzziness = fuzz;
	}
	inline void setHeaviside(Heaviside type) {
		heavisideType = type;
	}
	inline void setInvertContrast(bool invert) {
		this->invert = invert;
	}
	inline void setNoiseLevel(float level) {
		noiseLevel = level;
	}
	virtual ~Phantom() {
	}
};
class PhantomBubbles: public Phantom {
protected:

	float minRadius;
	float maxRadius;
	int numBubbles;
	virtual void solveInternal(Volume1f& vol) override;
public:
	PhantomBubbles(int rows, int cols, int slices, float maxDistance = 10.0f) :
			Phantom(rows, cols, slices, maxDistance), minRadius(0.025f), maxRadius(
					0.2f), numBubbles(10) {
	}
	inline void setMaxRadius(float maxRadius) {
		this->maxRadius = maxRadius;
	}
	inline void setMinRadius(float minRadius) {
		this->minRadius = minRadius;
	}
	inline void setNumberOfBubbles(int numBubbles) {
		this->numBubbles = numBubbles;
	}

};

class PhantomTorus: public Phantom {
protected:
	float3 center;
	float innerRadius;
	float outerRadius;
	virtual void solveInternal(Volume1f& vol) override;
public:
	PhantomTorus(int rows, int cols, int slices, float maxDistance = 10.0f) :
			Phantom(rows, cols, slices, maxDistance), center(0.0f), innerRadius(
					0.2f), outerRadius(0.6f) {
	}
	inline void setCenter(float3 c) {
		center = c;
	}
	inline void setInnerRadius(float r) {
		innerRadius = r;
	}
	inline void setOuterRadius(float r) {
		outerRadius = r;
	}
};

class PhantomSphere: public Phantom {
protected:
	float3 center;
	float radius;
	virtual void solveInternal(Volume1f& vol) override;
public:
	PhantomSphere(int rows, int cols, int slices, float maxDistance = 10.0f) :
			Phantom(rows, cols, slices, maxDistance), center(0.0f), radius(0.5f) {
	}
	inline void setRadius(float r) {
		radius = r;
	}
	inline void setCenter(float3 c) {
		center = c;
	}
};

class PhantomCube: public Phantom {
protected:
	float3 center;
	float width;
	virtual void solveInternal(Volume1f& vol) override;
public:
	PhantomCube(int rows, int cols, int slices, float maxDistance = 10.0f) :
			Phantom(rows, cols, slices, maxDistance), center(0.0f), width(0.5f) {
	}
	inline void setCenter(float3 c) {
		center = c;
	}
	inline void setWidth(float w) {
		width = w;
	}
};

class PhantomMetasphere: public Phantom {
protected:
	float3 center;
	float frequency;
	float maxAmplitude;
	float minAmplitude;
	virtual void solveInternal(Volume1f& vol) override;
public:
	PhantomMetasphere(int rows, int cols, int slices, float maxDistance = 10.0f) :
			Phantom(rows, cols, slices, maxDistance), center(0.0f), frequency(
					6.0f), minAmplitude(0.7f), maxAmplitude(0.9f) {
	}
	inline void setCenter(float3 c) {
		center = c;
	}
	inline void setFrequency(float f) {
		frequency = f;
	}
	inline void setMinAmplitude(float amp) {
		minAmplitude = amp;
	}
	inline void setMaxAmplitude(float amp) {
		maxAmplitude = amp;
	}
};

class PhantomCheckerBoard: public Phantom {
protected:
	float3 center;
	float width;
	float xFrequency, yFrequency, zFrequency;
	virtual void solveInternal(Volume1f& vol) override;

public:
	PhantomCheckerBoard(int rows, int cols, int slices, float maxDistance =
			10.0f) :
			Phantom(rows, cols, slices, maxDistance), center(0.0f), width(0.7f), xFrequency(
					1), yFrequency(1), zFrequency(1) {

	}
	inline void setCenter(float3 c) {
		center = c;
	}
	inline void setFrequency(float x, float y, float z) {
		xFrequency = x;
		yFrequency = y;
		zFrequency = z;
	}
	inline void setWidth(float w) {
		width = w;
	}
};

class PhantomSphereCollection {
protected:
	Volume1f distfieldImage;
	Volume1i labelImage;
public:
	PhantomSphereCollection(int rows, int cols, int slices, float radius);
	const Volume1f& getDistanceField()const {
		return distfieldImage;
	}
	const Volume1i& getLabels() const {;
		return labelImage;
	}
};

}
#endif /* INCLUDE_SEGMENTATION_PHANTOM_H_ */
