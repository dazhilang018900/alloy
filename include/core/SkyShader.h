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
 *
 */

/*
 Procedural Sky Shader implementation derived from Dimitri Diakopoulos's work that is <http://unlicense.org>
 https://github.com/ddiakopoulos/sandbox/blob/b346f56f47c4ef2dc72ba0c38696495a5b6cc9e9/gl/gl-procedural-sky.hpp
 */
#ifndef INCLUDE_CORE_SKYSHADER_H_
#define INCLUDE_CORE_SKYSHADER_H_
#include <GLShader.h>
#include <AlloyMesh.h>
#include <AlloyMath.h>
namespace aly {
double sky_evaluate_spline(const double * spline, size_t stride, double value);
double sky_evaluate(const double * dataset, size_t stride, float turbidity,float albedo, float sunTheta);
float3 sky_hosek_wilkie(float cos_theta, float gamma, float cos_gamma, float3 A,float3 B, float3 C, float3 D, float3 E, float3 F, float3 G, float3 H,float3 I);
float sky_perez(float theta, float gamma, float A, float B, float C, float D,float E);
float sky_zenith_luminance(float sunTheta, float turbidity);
float sky_zenith_chromacity(const float4 & c0, const float4 & c1, const float4 & c2,float sunTheta, float turbidity);
// An Analytic Model for Full Spectral Sky-Dome Radiance (Lukas Hosek, Alexander Wilkie)
struct HosekSkyRadianceData {
	float3 A, B, C, D, E, F, G, H, I;
	float3 Z;
	static HosekSkyRadianceData compute(float3 sun_direction, float turbidity,
			float albedo, float normalizedSunY);
};
// A Practical Analytic Model for Daylight (A. J. Preetham, Peter Shirley, Brian Smits)
struct PreethamSkyRadianceData {
	float3 A, B, C, D, E;
	float3 Z;
	static PreethamSkyRadianceData compute(float3 sun_direction,
			float turbidity, float albedo, float normalizedSunY);
};
class ProceduralSky: public GLShader {
protected:
	Camera camera;
	Mesh skyMesh;
	virtual void renderInternal(float3 sunDir) = 0;
	virtual void recompute(float turbidity, float albedo,float normalizedSunY) = 0;
public:
	float2 sunPosition;
	float normalizedSunY = 1.15f;
	float albedo = 0.1f;
	float turbidity = 4.f;
	std::function<void()> onParametersChanged;
	ProceduralSky(bool onScreen = true,const std::shared_ptr<AlloyContext>& context =AlloyDefaultContext());
	void draw(const box2px& viewport);
	void setSunPositionDegrees(float theta, float phi);
	void setSunPosition(float theta, float phi);
	float2 getSunPosition() const;
	float3 getSunDirection() const;
	void update();
};
class HosekProceduralSky: public ProceduralSky {
protected:
	HosekSkyRadianceData data;
	virtual void renderInternal(float3 sunDir) override;
	virtual void recompute(float turbidity, float albedo, float normalizedSunY)override;
public:
	HosekProceduralSky(bool onScreen = true,const std::shared_ptr<AlloyContext>& context =AlloyDefaultContext());
};
class PreethamProceduralSky: public ProceduralSky {
protected:
	PreethamSkyRadianceData data;
	virtual void renderInternal(float3 sunDir) override;
	virtual void recompute(float turbidity, float albedo, float normalizedSunY) override;
public:
	PreethamProceduralSky(bool onScreen = true,const std::shared_ptr<AlloyContext>& context =AlloyDefaultContext());
};
}
#endif /* INCLUDE_CORE_SKYSHADER_H_ */
