/*
 * DualQuatSkinning.cpp
 *
 *  Created on: Oct 18, 2018
 *      Author: blake
 */
#include "DualQuatSkinning.h"

/* dqconv.c

  Conversion routines between (regular quaternion, translation) and dual quaternion.

  Version 1.0.0, February 7th, 2007

  Copyright (C) 2006-2007 University of Dublin, Trinity College, All Rights
  Reserved

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the author(s) be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Author: Ladislav Kavan, ladislav.kavan@gmail.com

*/

namespace aly {


// input: unit quaternion 'q0', translation vector 't'
// output: unit dual quaternion 'dq'
float2x4 MakeDualQuat(const float4& q0, const float3& t){
   float2x4 dq;
   // non-dual part (just copy q0):
   for (int i=0; i<4; i++) dq(0,i) = q0[i];
   // dual part:
   dq(1,0) = -0.5*(t[0]*q0[1] + t[1]*q0[2] + t[2]*q0[3]);
   dq(1,1) = 0.5*( t[0]*q0[0] + t[1]*q0[3] - t[2]*q0[2]);
   dq(1,2) = 0.5*(-t[0]*q0[3] + t[1]*q0[0] + t[2]*q0[1]);
   dq(1,3) = 0.5*( t[0]*q0[2] - t[1]*q0[1] + t[2]*q0[0]);
   return dq;
}

// input: unit dual quaternion 'dq'
// output: unit quaternion 'q0', translation vector 't'
void DqToRotAndTrans(const float2x4 dq,float4& q0, float3& t){
   // regular quaternion (just copy the non-dual part):
   for (int i=0; i<4; i++) q0[i] = dq(0,i) ;
   // translation vector:
   t[0] = 2.0*(-dq(1,0)*dq(0,1) + dq(1,1)*dq(0,0) - dq(1,2)*dq(0,3) + dq(1,3)*dq(0,2));
   t[1] = 2.0*(-dq(1,0)*dq(0,2) + dq(1,1)*dq(0,3) + dq(1,2)*dq(0,0) - dq(1,3)*dq(0,1));
   t[2] = 2.0*(-dq(1,0)*dq(0,3) - dq(1,1)*dq(0,2) + dq(1,2)*dq(0,1) + dq(1,3)*dq(0,0));
}

// input: dual quat. 'dq' with non-zero non-dual part
// output: unit quaternion 'q0', translation vector 't'
void DqToUnitRotAndTrans(const float2x4& dq,float4& q0, float3& t){
   float len = 0.0;
   for (int i=0; i<4; i++) len += aly::sqr(dq(0,i));
   len = std::sqrt(len);
   for (int i=0; i<4; i++) q0[i] = dq(0,i) / len;
   t[0] = 2.0*(-dq(1,0)*dq(0,1) + dq(1,1)*dq(0,0) - dq(1,2)*dq(0,3) + dq(1,3)*dq(0,2)) / len;
   t[1] = 2.0*(-dq(1,0)*dq(0,2) + dq(1,1)*dq(0,3) + dq(1,2)*dq(0,0) - dq(1,3)*dq(0,1)) / len;
   t[2] = 2.0*(-dq(1,0)*dq(0,3) - dq(1,1)*dq(0,2) + dq(1,2)*dq(0,1) + dq(1,3)*dq(0,0)) / len;
}

/* dqs.cg

  Dual quaternion skinning vertex shaders (no shading computations)

  Version 1.0.3, November 1st, 2007

  Copyright (C) 2006-2007 University of Dublin, Trinity College, All Rights
  Reserved

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the author(s) be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Author: Ladislav Kavan, ladislav.kavan@gmail.com

*/


float4x4 DqToMatrix(float4 Qn, float4 Qd)
{
	float4x4 M=float4x4::identity();
	float len2 = dot(Qn, Qn);
	float w = Qn.x, x = Qn.y, y = Qn.z, z = Qn.w;
	float t0 = Qd.x, t1 = Qd.y, t2 = Qd.z, t3 = Qd.w;
	M(0,0) = w*w + x*x - y*y - z*z; M(0,1) = 2*x*y - 2*w*z; M(0,2) = 2*x*z + 2*w*y;
	M(1,0) = 2*x*y + 2*w*z; M(1,1) = w*w + y*y - x*x - z*z; M(1,2) = 2*y*z - 2*w*x;
	M(2,0) = 2*x*z - 2*w*y; M(2,1) = 2*y*z + 2*w*x; M(2,2) = w*w + z*z - x*x - y*y;

	M(0,3) = -2*t0*x + 2*w*t1 - 2*t2*z + 2*y*t3;
	M(1,3) = -2*t0*y + 2*t1*z - 2*x*t3 + 2*w*t2;
	M(2,3) = -2*t0*z + 2*x*t2 + 2*w*t3 - 2*t1*y;
	M /= len2;
	return M;
}

// basic dual quaternion skinning:
QuatOutputs DqSkinning(QuatInputs qin,
			float4x4 modelViewProj,
			float4x4 modelViewIT,
			const std::vector<float2x4>& boneDQ)
{
	QuatOutputs qout;
	float2x4 blendDQ = qin.weights[0]*boneDQ[qin.indexes[0]];
	blendDQ += qin.weights[1]*boneDQ[qin.indexes[1]];
	blendDQ += qin.weights[2]*boneDQ[qin.indexes[2]];
	blendDQ += qin.weights[3]*boneDQ[qin.indexes[3]];
	float4x4 M = DqToMatrix(blendDQ.row(0), blendDQ.row(1));
	float3 position = Transform(M, qin.position);
	float3 normal =Transform(M, qin.normal);
	qout.position = mul(modelViewProj, float4(position, 1.0));
	qout.normal = mul(modelViewIT, float4(normal, 0.0));
	return qout;
}

// per-vertex antipodality handling (this is the most robust, but not the most efficient way):
QuatOutputs DqAntipod(QuatInputs IN,
			float4x4 modelViewProj,
			float4x4 modelViewIT,
			const std::vector<float2x4>& boneDQ)
{
	QuatOutputs OUT;

	float2x4 dq0 = boneDQ[IN.indexes[0]];
	float2x4 dq1 = boneDQ[IN.indexes[1]];
	float2x4 dq2 = boneDQ[IN.indexes[2]];
	float2x4 dq3 = boneDQ[IN.indexes[3]];
	if (dot(dq0.row(0), dq1.row(0)) < 0.0) dq1 *= -1.0f;
	if (dot(dq0.row(0), dq2.row(0)) < 0.0) dq2 *= -1.0f;
	if (dot(dq0.row(0), dq3.row(0)) < 0.0) dq3 *= -1.0f;
	float2x4 blendDQ = IN.weights[0]*dq0;
	blendDQ += IN.weights[1]*dq1;
	blendDQ += IN.weights[2]*dq2;
	blendDQ += IN.weights[3]*dq3;

	float4x4 M = DqToMatrix(blendDQ.row(0), blendDQ.row(1));
	float3 position = Transform(M, IN.position);
	float3 normal = Transform(M, IN.normal);

	OUT.position = mul(modelViewProj, float4(position, 1.0));
	OUT.normal = mul(modelViewIT, float4(normal, 0.0));

	return OUT;
}

// optimized version (avoids dual quaternion - matrix conversion):
QuatOutputs DqFast(QuatInputs IN,
			float4x4 modelViewProj,
			float4x4 modelViewIT,
			const std::vector<float2x4>& boneDQ)
{
	QuatOutputs OUT;

	float2x4 blendDQ = IN.weights[0]*boneDQ[IN.indexes[0]];
	blendDQ += IN.weights[1]*boneDQ[IN.indexes[1]];
	blendDQ += IN.weights[2]*boneDQ[IN.indexes[2]];
	blendDQ += IN.weights[3]*boneDQ[IN.indexes[3]];

	float len = length(blendDQ[0]);
	blendDQ /= len;

	float3 position = IN.position + 2.0f*cross(blendDQ.row(0).yzw(), cross(blendDQ.row(0).yzw(), IN.position) + blendDQ[0].x*IN.position);
	float3 trans = 2.0f*(blendDQ.row(0).x*blendDQ.row(1).yzw() - blendDQ.row(1).x*blendDQ.row(0).yzw() + cross(blendDQ.row(0).yzw(), blendDQ.row(1).yzw()));
	position += trans;
	float3 inpNormal = IN.normal;
	float3 normal = inpNormal + 2.0f*cross(blendDQ.row(0).yzw(), cross(blendDQ.row(0).yzw(), inpNormal) + blendDQ[0].x*inpNormal);
	OUT.position = modelViewProj* float4(position, 1.0);
	OUT.normal = modelViewIT* float4(normal, 0.0);

	return OUT;
}

// two-phase skinning: dqsFast combined with scale/shear transformations:
QuatOutputs DqScale(QuatInputs IN,
			float4x4 modelViewProj,
			float4x4 modelViewIT,
			const std::vector<float2x4>& boneDQ,
			const std::vector<float4x4>& scaleM)
{
	QuatOutputs OUT;
	// first pass:
	float4x4 blendS = IN.weights[0]*scaleM[IN.indexes[0]];
	blendS += IN.weights[1]*scaleM[IN.indexes[1]];
	blendS += IN.weights[2]*scaleM[IN.indexes[2]];
	blendS += IN.weights[3]*scaleM[IN.indexes[3]];

	float3 pass1_position = Transform(blendS, IN.position);
	float3x3 blendSrotAT = adjugate(SubMatrix(blendS));
	float3 pass1_normal = normalize(blendSrotAT* IN.normal);
	// second pass:
	float2x4 blendDQ = IN.weights[0]*boneDQ[IN.indexes[0]];
	blendDQ += IN.weights[1]*boneDQ[IN.indexes[1]];
	blendDQ += IN.weights[2]*boneDQ[IN.indexes[2]];
	blendDQ += IN.weights[3]*boneDQ[IN.indexes[3]];
	float len = length(blendDQ[0]);
	blendDQ /= len;

	float3 position = pass1_position + 2.0f*cross(blendDQ.row(0).yzw(), cross(blendDQ.row(0).yzw(), pass1_position) + blendDQ.row(0).x*pass1_position);
	float3 trans = 2.0f*(blendDQ.row(0).x*blendDQ.row(1).yzw() - blendDQ.row(1).x*blendDQ.row(0).yzw() + cross(blendDQ.row(0).yzw(), blendDQ.row(1).yzw()));
	position += trans;
	float3 normal = pass1_normal + 2.0f*cross(blendDQ.row(0).yzw(), cross(blendDQ.row(0).yzw(), pass1_normal) + blendDQ.row(0).x*pass1_normal);
	OUT.position = modelViewProj* float4(position, 1.0);
	OUT.normal = modelViewIT* float4(normal, 0.0);
	return OUT;
}

}

