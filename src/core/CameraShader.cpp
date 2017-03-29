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
*  Created on: Jan 30, 2017
*      Author: Blake Lucas
*/

#include "CameraShader.h"

namespace aly{
CameraShader::CameraShader(bool onScreen,const std::shared_ptr<AlloyContext>& context) :
		GLShader(onScreen, context) {
	initialize({},
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
			R"(	#version 330
						in vec3 normal;
						in vec3 vert;

						uniform float MIN_DEPTH;
						uniform float MAX_DEPTH;
                        uniform vec3 lightDirection;
                        uniform vec4 diffuseColor;
                        uniform vec4 ambientColor;
                        //uniform mat4 ViewModelMat, PoseMat; 
						out vec4 FragColor;
						void main() {
                            //mat4 VM=ViewModelMat*PoseMat;
							vec3 norm = normalize(normal);
                            float diffuse = max(dot(lightDirection,norm), 0.0);
                            float d=(-vert.z-MIN_DEPTH)/(MAX_DEPTH-MIN_DEPTH);
                            gl_FragDepth=d; 
                            vec4 outColor=(ambientColor.w*ambientColor+diffuseColor.w*diffuse*diffuseColor)/(ambientColor.w+diffuseColor.w);
							FragColor = vec4(outColor.xyz,1.0);
						}
						)",
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
						  
	    normal= (VM*vec4(quad[0].n0,0.0)).xyz;					  
		gl_Position=PVM*vec4(p0,1);  
		vert = v0;
		EmitVertex();
		gl_Position=PVM*vec4(p1,1);  
		vert = v1;
		EmitVertex();
		gl_Position=PVM*vec4(p3,1);  
		vert = v3;
		EmitVertex();
		gl_Position=PVM*vec4(p2,1);  
		vert = v2;
		EmitVertex();
		EndPrimitive();
})");


}
void CameraShader::draw(const std::vector<std::shared_ptr<Frustum>>& cameras,CameraParameters& camera, GLFrameBuffer& frameBuffer,int selected) {
	frameBuffer.begin(float4(0.0f,0.0f,0.0f,1.0f));
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	begin().set("MIN_DEPTH", camera.getNearPlane()).set("MAX_DEPTH", camera.getFarPlane())
			.set(camera, frameBuffer.getViewport()).set("lightDirection",lightDirection)
			.set("ambientColor",ambientColor)
			.set("PoseMat",float4x4::identity());
	int index=0;
	for (std::shared_ptr<Frustum> frust : cameras) {
		set("diffuseColor",(selected==index)?selectedCameraColor:diffuseColor);
		GLShader::draw({frust.get()},GLMesh::PrimitiveType::QUADS);
		index++;
	}
	end();

	glEnable(GL_BLEND);
	frameBuffer.end();
}
void CameraShader::draw(const std::vector<std::shared_ptr<Frustum>>& cameras,CameraParameters& camera, const box2px& bounds,int selected) {
	begin().set("MIN_DEPTH", camera.getNearPlane()).set("MAX_DEPTH", camera.getFarPlane())
				.set(camera, bounds).set("lightDirection",lightDirection)
				.set("ambientColor",ambientColor)
				.set("PoseMat",float4x4::identity());
		int index=0;
		for (std::shared_ptr<Frustum> frust : cameras) {
			set("diffuseColor",(selected==index)?selectedCameraColor:diffuseColor);
			GLShader::draw({frust.get()},GLMesh::PrimitiveType::QUADS);
			index++;
		}
	end();
}

}


