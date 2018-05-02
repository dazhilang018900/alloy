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

#ifndef INCLUDE_FLUID_SIMULATIONOBJECTS_H_
#define INCLUDE_FLUID_SIMULATIONOBJECTS_H_
#include <AlloyMath.h>
#include <AlloyImage.h>
namespace aly {
enum class ObjectType {
	AIR = 0, FLUID = 1, WALL = 2
};
enum class ObjectMaterial {
	GLASS = 0, OPAQUE = 1, RED = 2
};

enum class ObjectShape {
	BOX = 0, SPHERE = 1, MESH = 2
};

struct SimulationObject {
public:
	ObjectType mType;
	ObjectShape mShape;
	ObjectMaterial mMaterial;
	bool mVisible;
	float mThickness;
	virtual float signedDistance(float2& pt) {
		return 0;
	}
	virtual bool inside(float2& pt) {
		return false;
	}
	virtual bool insideShell(float2& pt) {
		return false;
	}
	virtual ~SimulationObject() {
	}
	;
	SimulationObject(ObjectShape shape) :
			mShape(shape), mVisible(true), mThickness(0.0f), mType(
					ObjectType::AIR), mMaterial(ObjectMaterial::OPAQUE) {

	}
};
struct SphereObject: public SimulationObject {
public:
	float mRadius;
	float2 mCenter;
	float mVoxelSize;
	SphereObject() :
			SimulationObject(ObjectShape::SPHERE), mRadius(0), mCenter() {
	}
	virtual float signedDistance(float2& pt);
	virtual bool inside(float2& pt);
	virtual bool insideShell(float2& pt);
};
struct BoxObject: public SimulationObject {
public:
	float2 mMin;
	float2 mMax;
	float mVoxelSize;
	BoxObject() :
			SimulationObject(ObjectShape::BOX), mMin(), mMax(), mVoxelSize(1.0f) {
	}
	virtual bool inside(float2& pt);
	virtual float signedDistance(float2& pt);
	virtual bool insideShell(float2& pt);
};
struct MeshObject: public SimulationObject {
public:
	float mRadius;
	float mVoxelSize;
	float2 mCenter;
	Image1f* mSignedLevelSet;
	MeshObject() :SimulationObject(ObjectShape::MESH), mVoxelSize(1.0f), mRadius(1.0f), mCenter(), mSignedLevelSet(nullptr) {
	}
	virtual float signedDistance(float2& pt);
	virtual bool inside(float2& pt);
	virtual bool insideShell(float2& pt);
};
struct FluidParticle {
	float2 mLocation;
	float2 mVelocity;
	float2 mNormal;
	ObjectType mObjectType;
	//char mVisible;
	bool mRemoveIndicator;
	float2 mTmp[2];
	float mMass;
	float mDensity;
};
typedef std::shared_ptr<SimulationObject> SimulationObjectPtr;
typedef std::shared_ptr<FluidParticle> FluidParticlePtr;
}
#endif /* INCLUDE_FLUID_SIMULATIONOBJECTS_H_ */
