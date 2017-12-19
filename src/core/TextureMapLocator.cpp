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
*/
#include "TextureMapLocator.h"
namespace aly {

TextureMapLocator::TextureMapLocator(int dim):dim(dim) {
}
double3 TextureMapLocator::ToBaryNoNormalize(float2 f, float2 p1, float2 p2, float2 p3) {
	float2 f1 = p1 - f;
	float2 f2 = p2 - f;
	float2 f3 = p3 - f;
	double va =  crossMag(p1 - p2,p1 - p3); // main triangle cross product
	double va1 = crossMag(f2, f3); // p1's triangle cross product
	double va2 =  crossMag(f3, f1); // p2's triangle cross product
	double va3 =  crossMag(f1, f2); // p3's triangle cross product
	float a1 = va1 / va * aly::sign(va*va1);
	float a2 = va2 / va * aly::sign(va*va2);
	float a3 = va3 / va * aly::sign(va*va3);
	return double3(a1,a2,a3);
}
void TextureMapLocator::build(const Mesh& mesh,Image4f& positionMap,Image3f& normalMap){
	const Vector2f& uvs=mesh.textureMap;
	const Vector3f& verts=mesh.vertexLocations;
	const Vector3f& norms=mesh.vertexNormals;
	const Vector3ui& tris=mesh.triIndexes;
	float2 maxUv =uvs.max();
	size_t N=uvs.size();
	const float delta=1.0f/(dim);
	positionMap.resize(dim,dim);
	positionMap.set(float4(0,0,0,-1.0f));
	normalMap.resize(dim,dim);
	normalMap.set(float3(0,0,0));
	for(size_t n=0;n<N;n+=3){
		float2 uv1=uvs[n];
		float2 uv2=uvs[n+1];
		float2 uv3=uvs[n+2];

		uint3 tri=tris[n/3];
		float3 vr1=verts[tri.x];
		float3 vr2=verts[tri.y];
		float3 vr3=verts[tri.z];
		float3 nr1=norms[tri.x];
		float3 nr2=norms[tri.y];
		float3 nr3=norms[tri.z];
		int2 minPix=aly::max(int2(0),int2(aly::min(aly::min(uv1,uv2),uv3)*(float)dim)-1);
		int2 maxPix=aly::min(int2(dim-1),int2(aly::max(aly::max(uv1,uv2),uv3)*(float)dim)+1);
		int2 boxSize=maxPix-minPix;
		float2 closestPoint;
		for(int j=0;j<boxSize.y;j++){
			for(int i=0;i<boxSize.x;i++){
				float dist=DistanceToTriangleSqr(float2((i+minPix.x)*delta,(j+minPix.y)*delta),uv1,uv2,uv3,&closestPoint);
				float4& dmap=positionMap(i+minPix.x,dim-1-(j+minPix.y));
				if(dist<dmap.w||dmap.w==-1){
					double3 b=ToBaryNoNormalize(closestPoint,uv1,uv2,uv3);
					float3 pt=FromBary(b,vr1,vr2,vr3);
					float3 norm=normalize(FromBary(b,nr1,nr2,nr3));
					dmap=float4(pt,dist);
					normalMap(i+minPix.x,dim-1-(j+minPix.y))=norm;
				}
			}
		}
	}
}

} /* namespace intel */
