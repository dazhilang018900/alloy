/*
 * Copyright(C) 2015, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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

#ifndef ALLOYCAMERA_H_
#define ALLOYCAMERA_H_
#include <cereal/cereal.hpp>
#include "AlloyMath.h"
#include "AlloyContext.h"
#include <fstream>
namespace aly {
class Mesh;
class Region;
enum class CameraType {
	Perspective, Orthographic
};
template<class C, class R> std::basic_ostream<C, R> & operator <<(std::basic_ostream<C, R> & ss, const CameraType& type) {
	switch (type) {
	case CameraType::Perspective:
		return ss << "Perspective";
	case CameraType::Orthographic:
		return ss << "Orthographic";
	}
	return ss;
}
struct CameraProjector;
struct CameraParameters {
	bool changed;
	float nearPlane, farPlane;
	aly::int2 dimensions;
	float4x4 Projection, View, Model;
	float4x4 ViewModel, NormalViewModel, NormalView;
	float4x4 ViewInverse, ViewModelInverse;
	CameraParameters();
	template<class Archive>
	void save(Archive & archive) const {
		archive(CEREAL_NVP(Projection), CEREAL_NVP(View), CEREAL_NVP(Model), CEREAL_NVP(nearPlane), CEREAL_NVP(farPlane), CEREAL_NVP(dimensions));
	}

	template<class Archive>
	void load(Archive & archive) {
		archive(CEREAL_NVP(Projection), CEREAL_NVP(View), CEREAL_NVP(Model), CEREAL_NVP(nearPlane), CEREAL_NVP(farPlane), CEREAL_NVP(dimensions));
		ViewModel = View * Model;
		ViewModelInverse = inverse(ViewModel);
		NormalViewModel = transpose(ViewModelInverse);
		ViewInverse = inverse(View);
		NormalView = transpose(ViewInverse);
	}
	float3 transformWorldToScreen(const float3& pt) const;

	float3 transformWorldToNormalizedImage(const float3& pt) const;
	float3 transformWorldToImage(const float3& pt, int w, int h, bool flip = true) const;
	float2x3 differentiateWorldToImage(const float3& pt, int w, int h, bool flip = true) const;
	float2x3 differentiateCameraToImage(const float3& pt, int w, int h, bool flip = true) const;
	float2x3 differentiateWorldToScreen(const float3& pt) const;
	float3 transformWorldToNormalizedDepth(const float3& pt, int w, int h, bool flip = true) const;
	float3 transformNormalizedImageToWorld(const float3& pt, bool flip = true) const;
	float3 transformImageToWorld(const float3& pt, int w, int h, bool flip = true) const;
	float3 transformScreenToWorld(const float3& pt) const;
	float3 transformScreenDepthToWorld(const float3& pt) const;
	float3 transformNormalizedImageDepthToWorld(const float3& pt, bool flip = true) const;
	float3 transformImageDepthToWorld(const float3& pt, int w, int h, bool flip = true) const;
	CameraProjector getProjector() const;
	virtual void aim(const aly::box2px& bounds = box2px(pixel2(0.0f, 0.0f), pixel2(1.0f, 1.0f)));
	virtual ~CameraParameters() {
	}
	inline float2 getFocalLength() const {
		return float2(Projection(0, 0), Projection(1, 1));
	}
	virtual void setNearFarPlanes(float n, float f);
	float getNearPlane() const {
		return nearPlane;
	}
	float getFarPlane() const {
		return farPlane;
	}
	float2 getZRange() const {
		return float2(nearPlane, farPlane);
	}
	virtual void setDirty(bool d) {
		changed = true;
	}
	virtual bool isDirty() const {
		return changed;
	}
	virtual float getScale() const;
};
class Camera: public CameraParameters, public EventHandler {
protected:
	// Camera parameters
	float4x4 Rw, Rm;
	float3 cameraTrans;
	float mouseXPos;
	float mouseYPos;
	float fov;
	float3 lookAtPoint, eye;
	float tumblingSpeed, zoomSpeed, strafeSpeed;
	float distanceToObject;
	bool mouseDown, startTumbling, zoomMode, needsDisplay;
	CameraType cameraType;
	Region* activeRegion;
	bool includeParentRegion;
	float minScale;
	float maxScale;
	float3 minTranslation;
	float3 maxTranslation;
	void handleKeyEvent(GLFWwindow* win, int key, int action);
	void handleButtonEvent(int button, int action);
	void handleCursorEvent(float x, float y);
	void handleScrollEvent(int pos);
public:
	Camera();
	void reset();
	void setZoomRange(float mn,float mx){
		minScale=mn;
		maxScale=mx;
	}
	void setTranslationRange(float3 mn,float3 mx){
		minTranslation=mn;
		maxTranslation=mx;
	}
	std::string getName() const override {
		return "Camera";
	}
	void setActiveRegion(Region* region, bool includeParent = true) {
		activeRegion = region;
		includeParentRegion = includeParent;
	}
	void aim(const aly::box2px& bounds) override;
	void setPose(const float4x4& m) {
		Model = m;
	}
	void setCameraType(const CameraType& type) {
		cameraType = type;
		changed = true;
	}
	inline void rotateModelX(float angle) {
		Rm = MakeRotationX(angle) * Rm;
		changed = true;
	}
	inline void rotateModelY(float angle) {
		Rm = MakeRotationY(angle) * Rm;
		changed = true;
	}
	inline void rotateModelZ(float angle) {
		Rm = MakeRotationZ(angle) * Rm;
		changed = true;
	}
	inline void rotateWorldX(float angle) {
		Rw = MakeRotationX(angle) * Rw;
		changed = true;
	}
	inline void rotateWorldY(float angle) {
		Rw = MakeRotationY(angle) * Rw;
		changed = true;
	}
	inline void rotateWorldZ(float angle) {
		Rw = MakeRotationZ(angle) * Rw;
		changed = true;
	}
	inline void setModelRotation(const float4x4& M) {
		Rm = M;
		changed = true;
	}
	inline void setWorldRotation(const float4x4& M) {
		Rw = M;
		changed = true;
	}
	inline void setTranslation(const float3& t) {
		cameraTrans = t;
	}
	float4x4 getWorldRotation() const {
		return Rw;
	}
	float4x4 getModelRotation() const {
		return Rm;
	}
	float3 getTranslation() const {
		return cameraTrans;
	}
	virtual float getScale() const override {
		return distanceToObject;
	}
	float4x4 getPose() const {
		return Model;
	}
	virtual bool isDirty() const override {
		return needsDisplay || changed;
	}
	virtual void setDirty(bool d) override {
		needsDisplay = d;
		if (d)
			changed = true;
	}

	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event) override;

	void setFieldOfView(float degrees) {
		fov = degrees;
		changed = true;
	}
	void setSpeed(float zoomSpeed, float strafeSpeed, float tumblingSpeed);
	void lookAt(const float3& p, float dist);
	void lookAt(const float3& p) {
		lookAtPoint = p;
		changed = true;
	}

	float getNormalizedDepth(const float4& pt) const {
		float4 out = ViewModel * pt;
		return (-out.z - nearPlane) / (farPlane - nearPlane);
	}
	float2 computeNormalizedDepthRange(const Mesh& mesh);

	void resetTranslation() {
		cameraTrans = float3(0, 0, 0);
		lookAtPoint = float3(0, 0, 0);
		changed = true;
	}
	Camera& operator=(const CameraParameters& cam);
	void setZoom(float z) {
		distanceToObject = z;
		changed = true;
	}
	static const float sDeg2rad;
};
struct CameraProjector {
	//Do not write directly because they are interdependent, use setters instead!
	aly::float3x3 K;
	aly::float3x3 Kinv;
	aly::float3x3 R;
	aly::float3 T;
	aly::float4x4 Pose;
	aly::float3 origin;
	aly::int2 dimensions;
	template<class Archive>
	void save(Archive & archive) const {
		archive(CEREAL_NVP(K), CEREAL_NVP(Pose),CEREAL_NVP(dimensions));
	}
	template<class Archive>
	void load(Archive & archive) {
		archive(CEREAL_NVP(K), CEREAL_NVP(Pose),CEREAL_NVP(dimensions));
		R=SubMatrix(Pose);
		T=Pose.w.xyz();
		Kinv=inverse(K);
		origin=-transpose(R)*T;
	}
	CameraProjector():K(aly::float3x3::identity()),Pose(aly::float4x4::identity()){}
	CameraProjector(const aly::CameraParameters& cam);
	void setDimensions(const aly::int2& dims);
	void setTranslation(const aly::float3& t);
	void setRotation(const aly::float3x3& R);
	void setPose(const aly::float4x4& P);
	void setIntrinsics(const aly::float3x3& K);
	void setFocalLength(float fx,float fy) ;
	bool isVisible(const aly::float3& pt,const aly::float3& normal,float tol=0.0f)  const ;
	bool isVisible(const aly::float3& pt)  const ;
	bool isVisible(const aly::float2& pt)  const ;

	aly::CameraParameters getCameraParameters(float zNear=0.001f,float zFar=1.0f) const;
	aly::float3 transformImageToWorld(const aly::float3& pt) const;
	aly::float2 transformWorldToImage(const aly::float3& pt) const;
	aly::lineseg2f transformWorldToImage(const aly::lineseg3f& pt) const;
	aly::float3 getOrigin() const;
	aly::float3x4 getProjection() const;
	aly::float3 getDirection(const aly::float2& q) const;
	aly::float2 getFocalLength() const;
	aly::float2 getPrincipalPoint() const;
	const aly::float3x3& getRotation() const {return R;}
	const aly::float3& getTranslation()	const { return T;}
	const aly::float4x4& getPose() const {return Pose;}
	const aly::float3x3& getCameraIntrinsics() const {return K;}
};

void WriteCameraParametersToFile(const std::string& file, const CameraParameters& params);
void ReadCameraParametersFromFile(const std::string& file, CameraParameters& params);
void WriteCameraProjectorToFile(const std::string& file, const CameraProjector& params);
void ReadCameraProjectorFromFile(const std::string& file, CameraProjector& params);
// class Camera
}
#endif /* ALLOYCAMERA_H_ */
