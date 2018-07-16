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
*      Author: Blake Lucas
*/
#include "PointShader.h"
namespace aly {

PointColorShader::PointColorShader(bool onScreen,
		const std::shared_ptr<AlloyContext>& context) :
		GLShader(onScreen, context) {

	initialize( { },
			R"(
			#version 330 core
			#extension GL_ARB_separate_shader_objects : enable
			layout(location = 0) in vec3 vp;
            layout(location = 1) in vec3 vn;
			layout(location = 2) in vec4 vc;
			uniform float radius;
			out VS_OUT {
				vec3 pos;
                vec3 norm;
				vec4 color;
			} vs_out;
			void main(void) {
				vs_out.pos=vp;
                vs_out.norm=vn;
				vs_out.color=vc;
			}
		)",
			R"(
#version 330 core
in vec2 uv;
in vec4 vp;
in vec4 color;
out vec4 FragColor;
uniform float MIN_DEPTH;
uniform float MAX_DEPTH;
void main(void) {
	float radius=length(uv);
	if(radius>1.0){
		discard;
	} else {
		vec4 pos=vp/vp.w;
		float d=(-pos.z-MIN_DEPTH)/(MAX_DEPTH-MIN_DEPTH);
        if(d<=0.0||d>=1.0)discard;
		FragColor=color;
		gl_FragDepth=d;
	}
}
)",
			R"(
#version 330 core
#extension GL_ARB_separate_shader_objects : enable
	out vec2 uv;
	out vec4 vp;
	out vec4 color;
	layout(points) in;
	layout(triangle_strip, max_vertices = 4) out;
	in VS_OUT {
	  vec3 pos;
      vec3 norm;
	  vec4 color;
	} pc[];
    uniform float MIN_DEPTH;
    uniform int TWO_SIDED;
    uniform float POINT_SCALE;
	uniform mat4 ProjMat, ViewMat, ModelMat,ViewModelMat,NormalMat,PoseMat; 
	uniform vec4 bounds;
	uniform vec4 viewport;
void main() {
		color=pc[0].color;
        float radius=POINT_SCALE*color.w;
        color.w=1.0;
		mat4 PVM=ProjMat*ViewModelMat*PoseMat;
		mat4 VM=ViewModelMat*PoseMat;
		vec3 pt = pc[0].pos;
        vec3 zaxis=normalize(vec3(VM[0][2],VM[1][2],VM[2][2]));
        vec3 xaxis, yaxis;
		vec2 pos;
		vec4 vx;
        vec3 vn=normalize((VM*vec4(pc[0].norm,0)).xyz);
        if(TWO_SIDED==0&&vn.z<0.0)return;
        vp=VM*(vec4(pt, 1.0));
		if(-vp.z<=MIN_DEPTH)return;

		xaxis=vec3(-zaxis.z,0.0f,zaxis.x);
        if(length(xaxis)<1E-3f){
          xaxis=normalize(vec3(0.0f,zaxis.z,zaxis.y));
        } else {
          xaxis=normalize(xaxis);
        }
        yaxis=cross(zaxis,xaxis);
		vp=VM*(vec4(pt-xaxis*radius-yaxis*radius, 1.0));
		vx=ProjMat*vp;
		vx=vx/vx.w;
		vx.x=0.5*(vx.x+1);
		vx.y=0.5*(1-vx.y);
		pos=vx.xy*bounds.zw+bounds.xy;
		gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
		uv = vec2(-1.0, -1.0);
		EmitVertex();

		vp=VM*(vec4(pt+xaxis*radius-yaxis*radius, 1.0));
		vx=ProjMat*(vp);
        vx=vx/vx.w;
		vx.x=0.5*(vx.x+1);
		vx.y=0.5*(1-vx.y);
		pos=vx.xy*bounds.zw+bounds.xy;
		gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
		uv = vec2(1.0, -1.0);
		EmitVertex();
		
        vp=VM*(vec4(pt-xaxis*radius+yaxis*radius, 1.0));
		vx=ProjMat*(vp);
        vx=vx/vx.w;
		vx.x=0.5*(vx.x+1);
		vx.y=0.5*(1-vx.y);
		pos=vx.xy*bounds.zw+bounds.xy;
		gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
		uv = vec2(-1.0, 1.0);
		EmitVertex();
		
        vp=VM*(vec4(pt+xaxis*radius+yaxis*radius, 1.0));
		vx=ProjMat*(vp);
        vx=vx/vx.w;
		vx.x=0.5*(vx.x+1);
		vx.y=0.5*(1-vx.y);
		pos=vx.xy*bounds.zw+bounds.xy;
		gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
		uv = vec2(1.0, 1.0);
		EmitVertex();

		EndPrimitive();

})");

}

void PointColorShader::draw(Mesh* meshes,CameraParameters& camera, const box2px& bounds, const box2px& viewport,float scale,bool twoSided) {
	begin();
	glEnable(GL_SCISSOR_TEST);
	glScissor((int) bounds.position.x,
			(int) (viewport.dimensions.y - bounds.position.y
					- bounds.dimensions.y), (int) (bounds.dimensions.x),
			(int) (bounds.dimensions.y));
	set("TWO_SIDED",twoSided?1:0).set("POINT_SCALE",scale).set("MIN_DEPTH", camera.getNearPlane()).set("MAX_DEPTH",
			camera.getFarPlane()).set("bounds", bounds).set("viewport", viewport).set(camera, viewport).set("PoseMat",float4x4::identity());
	GLShader::draw(meshes, GLMesh::PrimitiveType::POINTS);
	glScissor((int) viewport.position.x, (int) viewport.position.x,
			(int) viewport.dimensions.x, (int) viewport.dimensions.y);
	glDisable(GL_SCISSOR_TEST);
	end();
}

PointDepthShader::PointDepthShader(bool onScreen,
		const std::shared_ptr<AlloyContext>& context) :
		GLShader(onScreen, context) {

	initialize( { },
			R"(
			#version 330 core
			#extension GL_ARB_separate_shader_objects : enable
			layout(location = 0) in vec3 vp;
            layout(location = 1) in vec3 vn;
			layout(location = 2) in vec4 vc;
			uniform float radius;
			out VS_OUT {
				vec3 pos;
                vec3 norm;
				vec4 color;
			} vs_out;
			void main(void) {
				vs_out.pos=vp;
                vs_out.norm=vn;
				vs_out.color=vc;
			}
		)",
			R"(
#version 330 core
in vec2 uv;
in vec3 vn;
in vec4 vp;
in vec4 color;
out vec4 FragColor;
uniform float MIN_DEPTH;
uniform float MAX_DEPTH;
void main(void) {
	float radius=length(uv);
	if(radius>1.0){
		discard;
	} else {
		vec4 pos=vp/vp.w;
		float d=(-pos.z-MIN_DEPTH)/(MAX_DEPTH-MIN_DEPTH);
        if(d<=0.0||d>=1.0)discard;
		FragColor=vec4(vn,d);
		gl_FragDepth=d;
	}
}
)",
			R"(
#version 330 core
#extension GL_ARB_separate_shader_objects : enable
	out vec2 uv;
    out vec3 vn;
	out vec4 vp;
	out vec4 color;
	layout(points) in;
	layout(triangle_strip, max_vertices = 4) out;
	in VS_OUT {
	  vec3 pos;
      vec3 norm;
	  vec4 color;
	} pc[];
    uniform float MIN_DEPTH;
    uniform int TWO_SIDED;
    uniform float POINT_SCALE;
	uniform mat4 ProjMat, ViewMat, ModelMat,ViewModelMat,NormalMat,PoseMat; 
	uniform vec4 bounds;
	uniform vec4 viewport;
void main() {
		color=pc[0].color;
        float radius=POINT_SCALE*color.w;
        color.w=1.0;
		mat4 PVM=ProjMat*ViewModelMat*PoseMat;
		mat4 VM=ViewModelMat*PoseMat;
		vec3 pt = pc[0].pos;
        vec3 zaxis=normalize(vec3(VM[0][2],VM[1][2],VM[2][2]));
        vec3 xaxis, yaxis;
		vec2 pos;
		vec4 vx;
		xaxis=vec3(-zaxis.z,0.0f,zaxis.x);
        if(length(xaxis)<1E-3f){
          xaxis=normalize(vec3(0.0f,zaxis.z,zaxis.y));
        } else {
          xaxis=normalize(xaxis);
        }
        vn=normalize((VM*vec4(pc[0].norm,0)).xyz);
        if(TWO_SIDED==0&&vn.z<0.0)return;
		if(vn.z<0){
			vn.z=-vn.z;
		}
        vp=VM*(vec4(pt, 1.0));
		if(-vp.z<=MIN_DEPTH)return;
        yaxis=cross(zaxis,xaxis);
		vp=VM*(vec4(pt-xaxis*radius-yaxis*radius, 1.0));
		vx=ProjMat*vp;
		vx=vx/vx.w;
		vx.x=0.5*(vx.x+1);
		vx.y=0.5*(1-vx.y);
		pos=vx.xy*bounds.zw+bounds.xy;
		gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
		uv = vec2(-1.0, -1.0);
		EmitVertex();

		vp=VM*(vec4(pt+xaxis*radius-yaxis*radius, 1.0));
		vx=ProjMat*(vp);
        vx=vx/vx.w;
		vx.x=0.5*(vx.x+1);
		vx.y=0.5*(1-vx.y);
		pos=vx.xy*bounds.zw+bounds.xy;
		gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
		uv = vec2(1.0, -1.0);
		EmitVertex();
		
        vp=VM*(vec4(pt-xaxis*radius+yaxis*radius, 1.0));
		vx=ProjMat*(vp);
        vx=vx/vx.w;
		vx.x=0.5*(vx.x+1);
		vx.y=0.5*(1-vx.y);
		pos=vx.xy*bounds.zw+bounds.xy;
		gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
		uv = vec2(-1.0, 1.0);
		EmitVertex();
		
        vp=VM*(vec4(pt+xaxis*radius+yaxis*radius, 1.0));
		vx=ProjMat*(vp);
        vx=vx/vx.w;
		vx.x=0.5*(vx.x+1);
		vx.y=0.5*(1-vx.y);
		pos=vx.xy*bounds.zw+bounds.xy;
		gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
		uv = vec2(1.0, 1.0);
		EmitVertex();

		EndPrimitive();

})");

}

void PointDepthShader::draw(Mesh* meshes,CameraParameters& camera, const box2px& bounds, const box2px& viewport,float scale,bool twoSided) {
	begin();
	glEnable(GL_SCISSOR_TEST);
	glScissor((int) bounds.position.x,
			(int) (viewport.dimensions.y - bounds.position.y
					- bounds.dimensions.y), (int) (bounds.dimensions.x),
			(int) (bounds.dimensions.y));
	set("TWO_SIDED",twoSided?1:0).set("POINT_SCALE",scale).set("MIN_DEPTH", camera.getNearPlane()).set("MAX_DEPTH",
			camera.getFarPlane()).set("bounds", bounds).set("viewport", viewport).set(camera, viewport).set("PoseMat",float4x4::identity());
	GLShader::draw(meshes, GLMesh::PrimitiveType::POINTS);
	glScissor((int) viewport.position.x, (int) viewport.position.x,
			(int) viewport.dimensions.x, (int) viewport.dimensions.y);
	glDisable(GL_SCISSOR_TEST);
	end();

}

PointPositionShader::PointPositionShader(bool onScreen,
		const std::shared_ptr<AlloyContext>& context) :
		GLShader(onScreen, context) {

	initialize( { },
			R"(
			#version 330 core
			#extension GL_ARB_separate_shader_objects : enable
			layout(location = 0) in vec3 vp;
            layout(location = 1) in vec3 vn;
			layout(location = 2) in vec4 vc;
			uniform float radius;
			out VS_OUT {
				vec3 pos;
                vec3 norm;
				vec4 color;
			} vs_out;
			void main(void) {
				vs_out.pos=vp;
                vs_out.norm=vn;
				vs_out.color=vc;
			}
		)",
			R"(
#version 330 core
in vec2 uv;
in vec3 vn;
in vec4 vp;
in vec4 color;
in vec3 vert;
out vec4 FragColor;
uniform float MIN_DEPTH;
uniform float MAX_DEPTH;
void main(void) {
	float radius=length(uv);
	if(radius>1.0){
		discard;
	} else {
		vec4 pos=vp/vp.w;
		float d=(-pos.z-MIN_DEPTH)/(MAX_DEPTH-MIN_DEPTH);
        if(d<=0.0||d>=1.0)discard;
		FragColor=vec4(vert,1.0);
		gl_FragDepth=d;
	}
}
)",
			R"(
#version 330 core
#extension GL_ARB_separate_shader_objects : enable
	out vec2 uv;
    out vec3 vn;
	out vec4 vp;
	out vec4 color;
    out vec3 vert;
	layout(points) in;
	layout(triangle_strip, max_vertices = 4) out;
	in VS_OUT {
	  vec3 pos;
      vec3 norm;
	  vec4 color;
	} pc[];
    uniform float MIN_DEPTH;
    uniform int TWO_SIDED;
    uniform float POINT_SCALE;

	uniform mat4 ProjMat, ViewMat, ModelMat,ViewModelMat,NormalMat,PoseMat; 
	uniform vec4 bounds;
	uniform vec4 viewport;
void main() {
		color=pc[0].color;
        float radius=POINT_SCALE*color.w;
        color.w=1.0;
		mat4 PVM=ProjMat*ViewModelMat*PoseMat;
		mat4 VM=ViewModelMat*PoseMat;
		vec3 pt = pc[0].pos;
        vec3 zaxis=normalize(vec3(VM[0][2],VM[1][2],VM[2][2]));
        vec3 xaxis, yaxis;
		vec2 pos;
		vec4 vx;
		xaxis=vec3(-zaxis.z,0.0f,zaxis.x);
        if(length(xaxis)<1E-3f){
          xaxis=normalize(vec3(0.0f,zaxis.z,zaxis.y));
        } else {
          xaxis=normalize(xaxis);
        }
		vn=normalize((VM*vec4(pc[0].norm,0)).xyz);
        if(TWO_SIDED==0&&vn.z<0.0)return;
        vp=VM*(vec4(pt, 1.0));
		if(-vp.z<=MIN_DEPTH)return;

        yaxis=cross(zaxis,xaxis);
        vert=pt-xaxis*radius-yaxis*radius;
		vp=VM*(vec4(vert, 1.0));		
        vx=ProjMat*vp;
		vx=vx/vx.w;
		vx.x=0.5*(vx.x+1);
		vx.y=0.5*(1-vx.y);
		pos=vx.xy*bounds.zw+bounds.xy;
		gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
		uv = vec2(-1.0, -1.0);
		EmitVertex();

        vert=pt+xaxis*radius-yaxis*radius;
		vp=VM*(vec4(vert, 1.0));
		vx=ProjMat*(vp);
        vx=vx/vx.w;
		vx.x=0.5*(vx.x+1);
		vx.y=0.5*(1-vx.y);
		pos=vx.xy*bounds.zw+bounds.xy;
		gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
		uv = vec2(1.0, -1.0);
		EmitVertex();
		
        vert=pt-xaxis*radius+yaxis*radius;
        vp=VM*(vec4(vert, 1.0));
		vx=ProjMat*(vp);
        vx=vx/vx.w;
		vx.x=0.5*(vx.x+1);
		vx.y=0.5*(1-vx.y);
		pos=vx.xy*bounds.zw+bounds.xy;
		gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
		uv = vec2(-1.0, 1.0);
		EmitVertex();
		
        vert=pt+xaxis*radius+yaxis*radius;
        vp=VM*(vec4(vert, 1.0));
		vx=ProjMat*(vp);
        vx=vx/vx.w;
		vx.x=0.5*(vx.x+1);
		vx.y=0.5*(1-vx.y);
		pos=vx.xy*bounds.zw+bounds.xy;
		gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
		uv = vec2(1.0, 1.0);
		EmitVertex();

		EndPrimitive();

})");

}

void PointPositionShader::draw(Mesh* meshes,CameraParameters& camera, const box2px& bounds, const box2px& viewport,float scale,bool twoSided) {
	begin();
	glEnable(GL_SCISSOR_TEST);
	glScissor((int) bounds.position.x,
			(int) (viewport.dimensions.y - bounds.position.y
					- bounds.dimensions.y), (int) (bounds.dimensions.x),
			(int) (bounds.dimensions.y));
	set("TWO_SIDED",twoSided?1:0).set("POINT_SCALE",scale).set("MIN_DEPTH", camera.getNearPlane()).set("MAX_DEPTH",
			camera.getFarPlane()).set("bounds", bounds).set("viewport", viewport).set(camera, viewport).set("PoseMat",float4x4::identity());
	GLShader::draw(meshes, GLMesh::PrimitiveType::POINTS);
	glScissor((int) viewport.position.x, (int) viewport.position.x,
			(int) viewport.dimensions.x, (int) viewport.dimensions.y);
	glDisable(GL_SCISSOR_TEST);
	end();
}
PointFaceIdShader::PointFaceIdShader(bool onScreen,
		const std::shared_ptr<AlloyContext>& context) :
		GLShader(onScreen, context) {

	initialize( { },
			R"(
			#version 330 core
			#extension GL_ARB_separate_shader_objects : enable
			layout(location = 0) in vec3 vp;
            layout(location = 1) in vec3 vn;
			layout(location = 2) in vec4 vc;
			uniform float radius;
			out VS_OUT {
				vec3 pos;
                vec3 norm;
				vec4 color;
                int vertId;
			} vs_out;
			void main(void) {
				vs_out.pos=vp;
                vs_out.norm=vn;
				vs_out.color=vc;
                vs_out.vertId=int(gl_VertexID);
			}
		)",
			R"(
#version 330 core
in vec2 uv;
in vec3 vn;
in vec4 vp;
in vec4 color;
flat in int vertId;
out vec4 FragColor;
uniform float MIN_DEPTH;
uniform float MAX_DEPTH;
void main(void) {
	float radius=length(uv);
	if(radius>1.0){
		discard;
	} else {
		vec4 pos=vp/vp.w;
		float d=(-pos.z-MIN_DEPTH)/(MAX_DEPTH-MIN_DEPTH);
        if(d<=0.0||d>=1.0)discard;
		FragColor = vec4(uint(vertId) & uint(0x00000FFF), ((uint(vertId) & uint(0x00FFF000)) >> uint(12)), ((uint(vertId) & uint(0xFF000000) ) >> uint(24)), 1);
		gl_FragDepth=d;
	}
}
)",
			R"(
#version 330 core
#extension GL_ARB_separate_shader_objects : enable
	out vec2 uv;
    out vec3 vn;
	out vec4 vp;
	out vec4 color;
    flat out int vertId;
	layout(points) in;
	layout(triangle_strip, max_vertices = 4) out;
	in VS_OUT {
	  vec3 pos;
      vec3 norm;
	  vec4 color;
      int vertId;
	} pc[];
    uniform float MIN_DEPTH;
    uniform int TWO_SIDED;
    uniform float POINT_SCALE;

	uniform mat4 ProjMat, ViewMat, ModelMat,ViewModelMat,NormalMat,PoseMat; 
	uniform vec4 bounds;
	uniform vec4 viewport;
void main() {
        vertId=pc[0].vertId;
		color=pc[0].color;
        float radius=POINT_SCALE*color.w;
        color.w=1.0;
		mat4 PVM=ProjMat*ViewModelMat*PoseMat;
		mat4 VM=ViewModelMat*PoseMat;
		vec3 pt = pc[0].pos;
        vec3 zaxis=normalize(vec3(VM[0][2],VM[1][2],VM[2][2]));
        vec3 xaxis, yaxis;
		vec2 pos;
		vec4 vx;
		xaxis=vec3(-zaxis.z,0.0f,zaxis.x);
        if(length(xaxis)<1E-3f){
          xaxis=normalize(vec3(0.0f,zaxis.z,zaxis.y));
        } else {
          xaxis=normalize(xaxis);
        }
        vn=normalize((VM*vec4(zaxis,0.0)).xyz);
        if(TWO_SIDED==0&&vn.z<0.0)return;
        vp=VM*(vec4(pt, 1.0));
		if(-vp.z<=MIN_DEPTH)return;

        yaxis=cross(zaxis,xaxis);
		vp=VM*(vec4(pt-xaxis*radius-yaxis*radius, 1.0));
		vx=ProjMat*vp;
		vx=vx/vx.w;
		vx.x=0.5*(vx.x+1);
		vx.y=0.5*(1-vx.y);
		pos=vx.xy*bounds.zw+bounds.xy;
		gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
		uv = vec2(-1.0, -1.0);
		EmitVertex();

		vp=VM*(vec4(pt+xaxis*radius-yaxis*radius, 1.0));
		vx=ProjMat*(vp);
        vx=vx/vx.w;
		vx.x=0.5*(vx.x+1);
		vx.y=0.5*(1-vx.y);
		pos=vx.xy*bounds.zw+bounds.xy;
		gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
		uv = vec2(1.0, -1.0);
		EmitVertex();
		
        vp=VM*(vec4(pt-xaxis*radius+yaxis*radius, 1.0));
		vx=ProjMat*(vp);
        vx=vx/vx.w;
		vx.x=0.5*(vx.x+1);
		vx.y=0.5*(1-vx.y);
		pos=vx.xy*bounds.zw+bounds.xy;
		gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
		uv = vec2(-1.0, 1.0);
		EmitVertex();
		
        vp=VM*(vec4(pt+xaxis*radius+yaxis*radius, 1.0));
		vx=ProjMat*(vp);
        vx=vx/vx.w;
		vx.x=0.5*(vx.x+1);
		vx.y=0.5*(1-vx.y);
		pos=vx.xy*bounds.zw+bounds.xy;
		gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
		uv = vec2(1.0, 1.0);
		EmitVertex();

		EndPrimitive();

})");

}
void PointFaceIdShader::read(GLFrameBuffer& framebuffer,Image1i& faceIdMap) {
	faceIdMap.resize(framebuffer.getWidth(), framebuffer.getHeight());
	ImageRGBAf irgba;
	framebuffer.getTexture().read(irgba);
	size_t idx = 0;
	int hash;
	int oid;
	for (RGBAf rgbaf : irgba.data) {
		int3 rgba = int3((int) rgbaf.x, (int) rgbaf.y, (int) rgbaf.z);
		if (rgbaf.w > 0.0f) {
			hash = (rgba.x) | (rgba.y << 12) | (rgba.z << 24);
		} else {
			hash = -1;
		}
		faceIdMap[idx++].x = hash;
	}
	FlipVertical(faceIdMap);
}
void PointFaceIdShader::draw(Mesh* meshes,CameraParameters& camera, const box2px& bounds, const box2px& viewport,float scale,bool twoSided) {
	begin();
	glEnable(GL_SCISSOR_TEST);
	if(meshes->vertexLocations.size()>(1<<24)){
		throw std::runtime_error("Too many points for Face ID shader.");
	}
	glScissor((int) bounds.position.x,
			(int) (viewport.dimensions.y - bounds.position.y
					- bounds.dimensions.y), (int) (bounds.dimensions.x),
			(int) (bounds.dimensions.y));
	set("TWO_SIDED",twoSided?1:0).set("POINT_SCALE",scale).set("MIN_DEPTH", camera.getNearPlane()).set("MAX_DEPTH",
			camera.getFarPlane()).set("bounds", bounds).set("viewport", viewport).set(camera, viewport).set("PoseMat",float4x4::identity());
	GLShader::draw(meshes, GLMesh::PrimitiveType::POINTS);
	glScissor((int) viewport.position.x, (int) viewport.position.x,
			(int) viewport.dimensions.x, (int) viewport.dimensions.y);
	glDisable(GL_SCISSOR_TEST);
	end();
}

} /* namespace intel */
