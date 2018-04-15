/*
 * SkyShader.cpp
 *
 *  Created on: Apr 15, 2018
 *      Author: blake
 */

#include "SkyShader.h"
#include <AlloyMeshPrimitives.h>
namespace aly {
float sky_zenith_luminance(float sunTheta, float turbidity) {
	float chi = (4.f / 9.f - turbidity / 120) * (ALY_PI - 2 * sunTheta);
	return (4.0453 * turbidity - 4.9710) * tan(chi) - 0.2155 * turbidity
			+ 2.4192;
}
float sky_zenith_chromacity(const float4 & c0, const float4 & c1, const float4 & c2,
		float sunTheta, float turbidity) {
	float4 thetav = float4(sunTheta * sunTheta * sunTheta, sunTheta * sunTheta,
			sunTheta, 1);
	return dot(float3(turbidity * turbidity, turbidity, 1),
			float3(dot(thetav, c0), dot(thetav, c1), dot(thetav, c2)));
}
double sky_evaluate_spline(const double * spline, size_t stride, double value) {
	return 1 * std::pow(1 - value, 5) * spline[0 * stride]
			+ 5 * std::pow(1 - value, 4) * std::pow(value, 1)
					* spline[1 * stride]
			+ 10 * std::pow(1 - value, 3) * std::pow(value, 2)
					* spline[2 * stride]
			+ 10 * std::pow(1 - value, 2) * std::pow(value, 3)
					* spline[3 * stride]
			+ 5 * std::pow(1 - value, 1) * std::pow(value, 4)
					* spline[4 * stride]
			+ 1 * std::pow(value, 5) * spline[5 * stride];
}
double sky_evaluate(const double * dataset, size_t stride, float turbidity,
		float albedo, float sunTheta) {
	// splines are functions of elevation^1/3
	double elevationK = std::pow(std::max(0.f, 1.f - sunTheta / (ALY_PI / 2.f)),
			1.f / 3.0f);

	// table has values for turbidity 1..10
	int turbidity0 = clamp(static_cast<int>(turbidity), 1, 10);
	int turbidity1 = std::min(turbidity0 + 1, 10);
	float turbidityK = clamp(turbidity - turbidity0, 0.f, 1.f);

	const double * datasetA0 = dataset;
	const double * datasetA1 = dataset + stride * 6 * 10;

	double a0t0 = sky_evaluate_spline(datasetA0 + stride * 6 * (turbidity0 - 1),
			stride, elevationK);
	double a1t0 = sky_evaluate_spline(datasetA1 + stride * 6 * (turbidity0 - 1),
			stride, elevationK);
	double a0t1 = sky_evaluate_spline(datasetA0 + stride * 6 * (turbidity1 - 1),
			stride, elevationK);
	double a1t1 = sky_evaluate_spline(datasetA1 + stride * 6 * (turbidity1 - 1),
			stride, elevationK);

	return a0t0 * (1 - albedo) * (1 - turbidityK)
			+ a1t0 * albedo * (1 - turbidityK)
			+ a0t1 * (1 - albedo) * turbidityK + a1t1 * albedo * turbidityK;
}
float3 sky_hosek_wilkie(float cos_theta, float gamma, float cos_gamma, float3 A,
		float3 B, float3 C, float3 D, float3 E, float3 F, float3 G, float3 H,
		float3 I) {
	float3 chi = (1.f + cos_gamma * cos_gamma)
			/ aly::pow(1.f + H * H - 2.f * cos_gamma * H, float3(1.5f));
	return (1.f + A * aly::exp(B / (cos_theta + 0.01f)))
			* (C + D * aly::exp(E * gamma) + F * (cos_gamma * cos_gamma)
					+ G * chi + I * (float) std::sqrt(std::max(0.f, cos_theta)));
}

float sky_perez(float theta, float gamma, float A, float B, float C, float D,
		float E) {
	return (1.f + A * std::exp(B / (cos(theta) + 0.01)))
			* (1.f + C * std::exp(D * gamma) + E * cos(gamma) * cos(gamma));
}

HosekSkyRadianceData HosekSkyRadianceData::compute(float3 sun_direction,
		float turbidity, float albedo, float normalizedSunY) {
	float3 A, B, C, D, E, F, G, H, I;
	float3 Z;

	const float sunTheta = std::acos(clamp(sun_direction.y, 0.f, 1.f));

	for (int i = 0; i < 3; ++i) {
		A[i] = sky_evaluate(datasetsRGB[i] + 0, 9, turbidity, albedo, sunTheta);
		B[i] = sky_evaluate(datasetsRGB[i] + 1, 9, turbidity, albedo, sunTheta);
		C[i] = sky_evaluate(datasetsRGB[i] + 2, 9, turbidity, albedo, sunTheta);
		D[i] = sky_evaluate(datasetsRGB[i] + 3, 9, turbidity, albedo, sunTheta);
		E[i] = sky_evaluate(datasetsRGB[i] + 4, 9, turbidity, albedo, sunTheta);
		F[i] = sky_evaluate(datasetsRGB[i] + 5, 9, turbidity, albedo, sunTheta);
		G[i] = sky_evaluate(datasetsRGB[i] + 6, 9, turbidity, albedo, sunTheta);

		// Swapped in the dataset
		H[i] = sky_evaluate(datasetsRGB[i] + 8, 9, turbidity, albedo, sunTheta);
		I[i] = sky_evaluate(datasetsRGB[i] + 7, 9, turbidity, albedo, sunTheta);

		Z[i] = sky_evaluate(datasetsRGBRad[i], 1, turbidity, albedo, sunTheta);
	}

	if (normalizedSunY) {
		float3 S = sky_hosek_wilkie(std::cos(sunTheta), 0, 1.f, A, B, C, D, E, F, G,
				H, I) * Z;
		Z /= dot(S, float3(0.2126, 0.7152, 0.0722));
		Z *= normalizedSunY;
	}

	return {A, B, C, D, E, F, G, H, I, Z};
}

PreethamSkyRadianceData PreethamSkyRadianceData::compute(float3 sun_direction,
		float turbidity, float albedo, float normalizedSunY) {
	assert(turbidity >= 1);

	const float sunTheta = std::acos(clamp(sun_direction.y, 0.f, 1.f));

	// A.2 Skylight Distribution Coefficients and Zenith Values: compute Perez distribution coefficients
	float3 A = float3(-0.0193, -0.0167, 0.1787) * turbidity
			+ float3(-0.2592, -0.2608, -1.4630);
	float3 B = float3(-0.0665, -0.0950, -0.3554) * turbidity
			+ float3(0.0008, 0.0092, 0.4275);
	float3 C = float3(-0.0004, -0.0079, -0.0227) * turbidity
			+ float3(0.2125, 0.2102, 5.3251);
	float3 D = float3(-0.0641, -0.0441, 0.1206) * turbidity
			+ float3(-0.8989, -1.6537, -2.5771);
	float3 E = float3(-0.0033, -0.0109, -0.0670) * turbidity
			+ float3(0.0452, 0.0529, 0.3703);

	// A.2 Skylight Distribution Coefficients and Zenith Values: compute zenith color
	float3 Z;
	Z.x = sky_zenith_chromacity(float4(0.00166, -0.00375, 0.00209, 0),
			float4(-0.02903, 0.06377, -0.03202, 0.00394),
			float4(0.11693, -0.21196, 0.06052, 0.25886), sunTheta, turbidity);
	Z.y = sky_zenith_chromacity(float4(0.00275, -0.00610, 0.00317, 0),
			float4(-0.04214, 0.08970, -0.04153, 0.00516),
			float4(0.15346, -0.26756, 0.06670, 0.26688), sunTheta, turbidity);
	Z.z = sky_zenith_luminance(sunTheta, turbidity);
	Z.z *= 1000; // conversion from kcd/m^2 to cd/m^2

	// 3.2 Skylight Model: pre-divide zenith color by distribution denominator
	Z.x /= sky_perez(0, sunTheta, A.x, B.x, C.x, D.x, E.x);
	Z.y /= sky_perez(0, sunTheta, A.y, B.y, C.y, D.y, E.y);
	Z.z /= sky_perez(0, sunTheta, A.z, B.z, C.z, D.z, E.z);

	// For low dynamic range simulation, normalize luminance to have a fixed value for sun
	if (normalizedSunY)
		Z.z = normalizedSunY / sky_perez(sunTheta, 0, A.z, B.z, C.z, D.z, E.z);

	return {A, B, C, D, E, Z};
}

ProceduralSky::ProceduralSky(bool onScreen,const std::shared_ptr<AlloyContext>& context):GLShader(onScreen,context) {
	skyMesh = Sphere(1.0f, 64, 128);
	setSunPositionDegrees(50, 110);
}
void ProceduralSky::render(float4x4 viewProj, float3 eyepoint, float farClip) {
	GLboolean blendEnabled;
	glGetBooleanv(GL_BLEND, &blendEnabled);

	GLboolean cullFaceEnabled;
	glGetBooleanv(GL_CULL_FACE, &cullFaceEnabled);

	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);

	// Largest non-clipped sphere
	float4x4 world = MakeTranslation(eyepoint) * MakeScale(farClip * .99f);
	renderInternal(viewProj, getSunDirection(), world);
	if (blendEnabled)
		glEnable(GL_BLEND);
	if (cullFaceEnabled)
		glEnable(GL_CULL_FACE);
}

// Set in degrees. Theta = 0 - 90, Phi = 0 - 360
void ProceduralSky::setSunPositionDegrees(float theta, float phi) {
	sunPosition = {ToRadians(theta), ToRadians(phi)};
}
void ProceduralSky::setSunPosition(float theta, float phi) {
	sunPosition = {theta,phi};
}
void ProceduralSky::setSunPosition(float3 dir){
	dir=normalize(dir);
	sunPosition={std::atan2(dir.y,dir.x),std::acos(dir.z)};
}
// Get in degrees
float2 ProceduralSky::getSunPosition() const {
	return float2(ToDegrees(sunPosition.x), ToDegrees(sunPosition.y));
}
float3 ProceduralSky::getSunDirection() const {
	float sp = std::sin(sunPosition.y);
	return float3(std::cos(sunPosition.x) * sp, std::sin(sunPosition.x) * sp,
			std::cos(sunPosition.y));
}

void HosekProceduralSky::recompute(float turbidity, float albedo,
		float normalizedSunY) {
	data = HosekSkyRadianceData::compute(getSunDirection(), turbidity, albedo,
			normalizedSunY);
	if (onParametersChanged)
		onParametersChanged();
}

void HosekProceduralSky::renderInternal(float4x4 viewProj, float3 sunDir,
		float4x4 world) {
	begin().set("ViewProjection", viewProj);
	set("World", world);
	set("A", data.A);
	set("B", data.B);
	set("C", data.C);
	set("D", data.D);
	set("E", data.E);
	set("F", data.F);
	set("G", data.G);
	set("H", data.H);
	set("I", data.I);
	set("Z", data.Z);
	set("SunDirection", sunDir).draw(skyMesh, GLMesh::PrimitiveType::ALL);
}

HosekProceduralSky::HosekProceduralSky(bool onScreen,
		const std::shared_ptr<AlloyContext>& context) :
		ProceduralSky(onScreen, context) {
	initialize( { },
			R"(	#version 330
					layout(location = 3) in vec3 vp0;
					layout(location = 4) in vec3 vp1;
					layout(location = 5) in vec3 vp2;

					layout(location = 7) in vec3 vn0;
					layout(location = 8) in vec3 vn1;
					layout(location = 9) in vec3 vn2;
					layout(location = 10) in vec3 vn3;
		
					out VS_OUT {
						vec3 p0;
						vec3 p1;
						vec3 p2;
						vec3 p3;
						vec3 n0;
						vec3 n1;
						vec3 n2;
						vec3 n3;
					} vs_out;
					void main(void) {
						vs_out.p0=vp0;
						vs_out.p1=vp1;
						vs_out.p2=vp2;
						vs_out.p3=vp3;
						vs_out.n0=vn0;
						vs_out.n1=vn1;
						vs_out.n2=vn2;
						vs_out.n3=vn3;
					})",

			R"(
#version 330
in vec3 vert;
in vec3 normal;
out vec4 FragColor;
uniform vec3 A, B, C, D, E, F, G, H, I, Z;
uniform vec3 SunDirection;
// ArHosekSkyModel_GetRadianceInternal
vec3 HosekWilkie(float cos_theta, float gamma, float cos_gamma)
{
	vec3 chi = (1 + cos_gamma * cos_gamma) / pow(1 + H * H - 2 * cos_gamma * H, vec3(1.5));
    return (1 + A * exp(B / (cos_theta + 0.01))) * (C + D * exp(E * gamma) + F * (cos_gamma * cos_gamma) + G * chi + I * sqrt(cos_theta));
}
void main()
{
	vec3 V = normalize(vert);
	float cos_theta = clamp(V.y, 0, 1);
	float cos_gamma = clamp(dot(V, SunDirection), 0, 1);
	float gamma_ = acos(cos_gamma);

	vec3 R = Z * HosekWilkie(cos_theta, gamma_, cos_gamma);
   FragColor= vec4(R, 1);
})",
R"(	#version 330
		layout (points) in;
		layout (triangle_strip, max_vertices=4) out;
		in VS_OUT {
			vec3 p0;
			vec3 p1;
			vec3 p2;
			vec3 p3;
			vec3 n0;
			vec3 n1;
			vec3 n2;
			vec3 n3;
		} quad[];
		out vec3 v0, v1, v2, v3;
		out vec3 normal;
		out vec3 vert;
		uniform int IS_QUAD;
        uniform int IS_FLAT;
	uniform mat4 ProjMat, ViewMat, ModelMat,ViewModelMat,NormalMat,PoseMat; 
		void main() {
		  mat4 PVM=ProjMat*ViewModelMat*PoseMat;
		  mat4 VM=ViewModelMat*PoseMat;
		  
		  vec3 p0=quad[0].p0;
		  vec3 p1=quad[0].p1;
		  vec3 p2=quad[0].p2;
          vec3 p3=quad[0].p3;
		
		  v0 = (VM*vec4(p0,1)).xyz;
		  v1 = (VM*vec4(p1,1)).xyz;
		  v2 = (VM*vec4(p2,1)).xyz;
          v3 = (VM*vec4(p3,1)).xyz;
		  
		  
if(IS_QUAD!=0){
gl_Position=PVM*vec4(p0,1);  
vert = v0;
if(IS_FLAT!=0){
vec3 pt=0.25*(p0+p1+p2+p3);
normal = cross(p0-pt, p1-pt)+cross(p1-pt, p2-pt)+cross(p2-pt, p3-pt)+cross(p3-pt, p0-pt);
normal = (VM*vec4(normalize(-normal),0.0)).xyz;
} else {
normal= (VM*vec4(quad[0].n0,0.0)).xyz;
}
EmitVertex();
} else {
gl_Position=PVM*vec4(p0,1);  
vert = v0;
if(IS_FLAT!=0){
normal = (VM*vec4(normalize(cross( p2-p0, p1-p0)),0.0)).xyz;
} else {
normal= (VM*vec4(quad[0].n0,0.0)).xyz;
}
EmitVertex();
}
gl_Position=PVM*vec4(p1,1);  
vert = v1;
if(IS_FLAT!=0){
//normal = (VM*vec4(normalize(cross( p0-p1, p2-p1)),0.0)).xyz;
} else {
normal= (VM*vec4(quad[0].n1,0.0)).xyz;
}
EmitVertex();
if(IS_QUAD!=0){
gl_Position=PVM*vec4(p3,1);  
vert = v3;
if(IS_FLAT!=0){
//normal = (VM*vec4(normalize(cross( p2-p3, p0-p3)),0.0)).xyz;
} else {
normal= (VM*vec4(quad[0].n3,0.0)).xyz;
}
EmitVertex();
gl_Position=PVM*vec4(p2,1);  
vert = v2;
if(IS_FLAT!=0){
//normal = (VM*vec4(normalize(cross( p1-p2, p3-p2)),0.0)).xyz;
} else {
normal= (VM*vec4(quad[0].n2,0.0)).xyz;
}
EmitVertex();
EndPrimitive();
} else {
gl_Position=PVM*vec4(p2,1);  
vert = v2;
if(IS_FLAT!=0){
//normal = (VM*vec4(normalize(cross( p1-p2, p0-p2)),0.0)).xyz;
} else {
normal= (VM*vec4(quad[0].n2,0.0)).xyz;
}
EmitVertex();
EndPrimitive();
}
})");

	recompute(turbidity, albedo, normalizedSunY);
}

PreethamProceduralSky::PreethamProceduralSky(bool onScreen,
		const std::shared_ptr<AlloyContext>& context) :
		ProceduralSky(onScreen, context) {
	initialize( { },
			R"(	#version 330
					layout(location = 3) in vec3 vp0;
					layout(location = 4) in vec3 vp1;
					layout(location = 5) in vec3 vp2;
					layout(location = 6) in vec3 vp3;

					layout(location = 7) in vec3 vn0;
					layout(location = 8) in vec3 vn1;
					layout(location = 9) in vec3 vn2;
					layout(location = 10) in vec3 vn3;
		
					out VS_OUT {
						vec3 p0;
						vec3 p1;
						vec3 p2;
						vec3 p3;
						vec3 n0;
						vec3 n1;
						vec3 n2;
						vec3 n3;
					} vs_out;
					void main(void) {
						vs_out.p0=vp0;
						vs_out.p1=vp1;
						vs_out.p2=vp2;
						vs_out.p3=vp3;
						vs_out.n0=vn0;
						vs_out.n1=vn1;
						vs_out.n2=vn2;
						vs_out.n3=vn3;
					})",
			R"(
#version 330
in vec3 vert;
in vec3 normal;
out vec4 FragColor;
uniform vec3 A, B, C, D, E, Z;
uniform vec3 SunDirection;
vec3 sky_perez(float cos_theta, float gamma, float cos_gamma, vec3 A, vec3 B, vec3 C, vec3 D, vec3 E)
{
    return (1 + A * exp(B / (cos_theta + 0.01))) * (1 + C * exp(D * gamma) + E * cos_gamma * cos_gamma);
}
void main()
{
    vec3 V = normalize(vert);
    float cos_theta = clamp(V.y, 0, 1);
    float cos_gamma = dot(V, SunDirection);
    float gamma = acos(cos_gamma);
    vec3 R_xyY = Z * sky_perez(cos_theta, gamma, cos_gamma, A, B, C, D, E);
    vec3 R_XYZ = vec3(R_xyY.x, R_xyY.y, 1 - R_xyY.x - R_xyY.y) * R_xyY.z / R_xyY.y;
    // Radiance
    float r = dot(vec3( 3.240479, -1.537150, -0.498535), R_XYZ);
    float g = dot(vec3(-0.969256,  1.875992,  0.041556), R_XYZ);
    float b = dot(vec3( 0.055648, -0.204043,  1.057311), R_XYZ);

    FragColor = vec4(vec3(r, g, b), 1);
})",
R"(	#version 330
		layout (points) in;
		layout (triangle_strip, max_vertices=4) out;
		in VS_OUT {
			vec3 p0;
			vec3 p1;
			vec3 p2;
			vec3 p3;
			vec3 n0;
			vec3 n1;
			vec3 n2;
			vec3 n3;
		} quad[];
		out vec3 v0, v1, v2, v3;
		out vec3 normal;
		out vec3 vert;
		uniform int IS_QUAD;
        uniform int IS_FLAT;
	uniform mat4 ProjMat, ViewMat, ModelMat,ViewModelMat,NormalMat,PoseMat; 
		void main() {
		  mat4 PVM=ProjMat*ViewModelMat*PoseMat;
		  mat4 VM=ViewModelMat*PoseMat;
		  
		  vec3 p0=quad[0].p0;
		  vec3 p1=quad[0].p1;
		  vec3 p2=quad[0].p2;
          vec3 p3=quad[0].p3;
		
		  v0 = (VM*vec4(p0,1)).xyz;
		  v1 = (VM*vec4(p1,1)).xyz;
		  v2 = (VM*vec4(p2,1)).xyz;
          v3 = (VM*vec4(p3,1)).xyz;
		  
		  
if(IS_QUAD!=0){
gl_Position=PVM*vec4(p0,1);  
vert = v0;
if(IS_FLAT!=0){
vec3 pt=0.25*(p0+p1+p2+p3);
normal = cross(p0-pt, p1-pt)+cross(p1-pt, p2-pt)+cross(p2-pt, p3-pt)+cross(p3-pt, p0-pt);
normal = (VM*vec4(normalize(-normal),0.0)).xyz;
} else {
normal= (VM*vec4(quad[0].n0,0.0)).xyz;
}
EmitVertex();
} else {
gl_Position=PVM*vec4(p0,1);  
vert = v0;
if(IS_FLAT!=0){
normal = (VM*vec4(normalize(cross( p2-p0, p1-p0)),0.0)).xyz;
} else {
normal= (VM*vec4(quad[0].n0,0.0)).xyz;
}
EmitVertex();
}
gl_Position=PVM*vec4(p1,1);  
vert = v1;
if(IS_FLAT!=0){
//normal = (VM*vec4(normalize(cross( p0-p1, p2-p1)),0.0)).xyz;
} else {
normal= (VM*vec4(quad[0].n1,0.0)).xyz;
}
EmitVertex();
if(IS_QUAD!=0){
gl_Position=PVM*vec4(p3,1);  
vert = v3;
if(IS_FLAT!=0){
//normal = (VM*vec4(normalize(cross( p2-p3, p0-p3)),0.0)).xyz;
} else {
normal= (VM*vec4(quad[0].n3,0.0)).xyz;
}
EmitVertex();
gl_Position=PVM*vec4(p2,1);  
vert = v2;
if(IS_FLAT!=0){
//normal = (VM*vec4(normalize(cross( p1-p2, p3-p2)),0.0)).xyz;
} else {
normal= (VM*vec4(quad[0].n2,0.0)).xyz;
}
EmitVertex();
EndPrimitive();
} else {
gl_Position=PVM*vec4(p2,1);  
vert = v2;
if(IS_FLAT!=0){
//normal = (VM*vec4(normalize(cross( p1-p2, p0-p2)),0.0)).xyz;
} else {
normal= (VM*vec4(quad[0].n2,0.0)).xyz;
}
EmitVertex();
EndPrimitive();
}
})");

	recompute(turbidity, albedo, normalizedSunY);
}
void PreethamProceduralSky::recompute(float turbidity, float albedo,
		float normalizedSunY) {
	data = PreethamSkyRadianceData::compute(getSunDirection(), turbidity,
			albedo, normalizedSunY);
	if (onParametersChanged)
		onParametersChanged();
}
void PreethamProceduralSky::renderInternal(float4x4 viewProj, float3 sunDir,
		float4x4 world) {
	begin();
	set("ViewProjection", viewProj);
	set("World", world);
	set("A", data.A);
	set("B", data.B);
	set("C", data.C);
	set("D", data.D);
	set("E", data.E);
	set("Z", data.Z);
	set("SunDirection", sunDir);
	draw(skyMesh, GLMesh::PrimitiveType::ALL);
}

}

