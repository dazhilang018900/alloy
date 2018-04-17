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
#include <ShadowCastShader.h>
namespace aly {
ShadowCastShader::ShadowCastShader(bool onScreen, const std::shared_ptr<AlloyContext>& context) :
		GLShader(onScreen, context),blurShader(ImageShader::Filter::MEDIUM_BLUR,onScreen,context) ,imageShader(ImageShader::Filter::NONE,onScreen,context) {
	initialize({}, R"(#version 330
layout(location = 0) in vec3 vp; 
layout(location = 1) in vec2 vt; 
out vec2 uv;
void main() {
	uv=vt;
	gl_Position = vec4(2*vp.x-1.0,1.0-2*vp.y,0,1);
})",R"(
#version 330
in vec2 uv;
out vec4 FragColor;
uniform ivec2 shadowBufferSize;
uniform ivec2 depthBufferSize;
uniform float MIN_DEPTH;
uniform float MAX_DEPTH;
uniform float spread;
uniform float depthTolerance;
uniform int samples;
uniform float alpha;
uniform sampler2D objectColorTexture;
uniform sampler2D objectPositionTexture;
uniform sampler2D shadowDepthTexture;
uniform mat4 ProjMat,ViewModelMat,ViewModelInverseMat,CameraViewModelMat;
float rand(vec2 n) {
    return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}
float noise(vec2 p){
    vec2 ip = floor(p);
    vec2 u = fract(p);
    u = u*u*(3.0-2.0*u);

    float res = mix(
        mix(rand(ip),rand(ip+vec2(1.0,0.0)),u.x),
        mix(rand(ip+vec2(0.0,1.0)),rand(ip+vec2(1.0,1.0)),u.x),u.y);
    return res*res;
}
float texture2DCompare(sampler2D depths, vec2 uv, float compare){
	float depth = texture(depths, uv).w;//*(MAX_DEPTH-MIN_DEPTH)+MIN_DEPTH;
	return step(compare, depth);
}
float texture2DShadowLerp(sampler2D depths, vec2 size, vec2 uv, float compare){
	vec2 texelSize = vec2(1.0)/size;
	vec2 f = fract(uv*size+0.5);
	vec2 centroidUV = floor(uv*size+0.5)/size;
	float lb = texture2DCompare(depths, centroidUV+texelSize*vec2(0.0, 0.0), compare);
	float lt = texture2DCompare(depths, centroidUV+texelSize*vec2(0.0, 1.0), compare);
	float rb = texture2DCompare(depths, centroidUV+texelSize*vec2(1.0, 0.0), compare);
	float rt = texture2DCompare(depths, centroidUV+texelSize*vec2(1.0, 1.0), compare);
	float a = mix(lb, lt, f.y);
	float b = mix(rb, rt, f.y);
	float c = mix(a, b, f.x);
	return c;
}
float PCF(sampler2D depths, vec2 size, vec2 uv,vec2 pt, float compare){
	float result = 0.0;
	float seed=noise(vec2(noise(size*uv),noise(pt)));
	for(int n=0;n<samples;n++){
		float dx=noise(vec2(seed*size.x,n));
		float dy=noise(vec2(seed*size.x,samples-1-n));
		vec2 off = spread*(2*vec2(dx,dy)-1)/size;
		result += texture2DShadowLerp(depths, size, uv+off, compare);
	}
	return result/samples;
}
void main() {
	mat4 PVM=ProjMat*ViewModelMat;
	mat4 VM=ViewModelMat;
	ivec2 pix=ivec2(uv.x*depthBufferSize.x,uv.y*depthBufferSize.y);
	vec4 pos=texelFetch(objectPositionTexture, pix, 0);
	vec4 cpos=CameraViewModelMat*vec4(pos.xyz,1.0);
	vec4 lpos=CameraViewModelMat*ViewModelInverseMat[3];
	lpos=lpos/lpos.w;
	vec4 color=texelFetch(objectColorTexture, pix, 0);
	gl_FragDepth=color.w;
	color.w=1.0;
	float visible=0.0f;
	vec4 c;
	if(pos.w>0.0){
		vec4 mpos=PVM*vec4(pos.xyz,1);
		pos=VM*vec4(pos.xyz,1);
		vec2 proj=mpos.xy/mpos.w;
		if(proj.x>-1.0&&proj.x<1.0&&proj.y>-1.0&&proj.y<1.0){
			proj.x=0.5*(proj.x+1);
			proj.y=0.5*(proj.y+1);
			visible=PCF(shadowDepthTexture,vec2(shadowBufferSize.x,shadowBufferSize.y),proj,vec2(uv.x*depthBufferSize.x,uv.y*depthBufferSize.y), (-pos.z-MIN_DEPTH)/(MAX_DEPTH-MIN_DEPTH)-depthTolerance);
		}
		c=(1.0-alpha)*color;
		c.w=1.0;
		c=(1.0-visible)*c;
		FragColor=c;
	} else {
		FragColor=vec4(0.0,0.0,0.0,0.0);
		discard;
	}
})");
}

void ShadowCastShader::draw(
		const GLTextureRGBAf& objectColorTexture,
		const GLTextureRGBAf& objectPositionTexture,
		const GLTextureRGBAf& shadowDepthTexture,
		CameraParameters& camera,
		CameraParameters& lightCamera,
	    float spread,
		float depthTolerance,
		float alpha,
		int samples) {
	if(objectColorTexture.dimensions()!=frameBuffer.bounds.dimensions){
		frameBuffer.initialize(objectColorTexture.getWidth(),objectColorTexture.getHeight());
	}
	frameBuffer.begin();
	begin() .set("objectColorTexture",objectColorTexture,0)
			.set("objectPositionTexture", objectPositionTexture,1)
			.set("shadowDepthTexture", shadowDepthTexture,2)
			.set("depthBufferSize", objectPositionTexture.dimensions())
			.set("shadowBufferSize", shadowDepthTexture.dimensions())
			.set("CameraViewModelMat",camera.ViewModel)
		    .set("spread", spread)
			.set("depthTolerance",depthTolerance)
			.set("alpha",alpha)
			.set("samples",samples)
			.set("MIN_DEPTH",lightCamera.getNearPlane())
			.set("MAX_DEPTH",lightCamera.getFarPlane())
			.set("ViewModelMat",lightCamera.ViewModel)
			.set("ViewModelInverseMat",lightCamera.ViewModelInverse)
			.set("ProjMat",lightCamera.Projection)
			.draw(objectPositionTexture)
			.end();
	frameBuffer.end();
	glEnable(GL_BLEND);
	imageShader.draw(objectColorTexture,1.0f,false);
	blurShader.draw(frameBuffer.getTexture(),1.0,false);
}

}
