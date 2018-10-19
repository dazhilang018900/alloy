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
void QuatTrans2UDQ(const float q0[4], const float t[3],
                  float dq[2][4])
{
   // non-dual part (just copy q0):
   for (int i=0; i<4; i++) dq[0][i] = q0[i];
   // dual part:
   dq[1][0] = -0.5*(t[0]*q0[1] + t[1]*q0[2] + t[2]*q0[3]);
   dq[1][1] = 0.5*( t[0]*q0[0] + t[1]*q0[3] - t[2]*q0[2]);
   dq[1][2] = 0.5*(-t[0]*q0[3] + t[1]*q0[0] + t[2]*q0[1]);
   dq[1][3] = 0.5*( t[0]*q0[2] - t[1]*q0[1] + t[2]*q0[0]);
}

// input: unit dual quaternion 'dq'
// output: unit quaternion 'q0', translation vector 't'
void UDQ2QuatTrans(const float dq[2][4],
                  float q0[4], float t[3])
{
   // regular quaternion (just copy the non-dual part):
   for (int i=0; i<4; i++) q0[i] = dq[0][i];
   // translation vector:
   t[0] = 2.0*(-dq[1][0]*dq[0][1] + dq[1][1]*dq[0][0] - dq[1][2]*dq[0][3] + dq[1][3]*dq[0][2]);
   t[1] = 2.0*(-dq[1][0]*dq[0][2] + dq[1][1]*dq[0][3] + dq[1][2]*dq[0][0] - dq[1][3]*dq[0][1]);
   t[2] = 2.0*(-dq[1][0]*dq[0][3] - dq[1][1]*dq[0][2] + dq[1][2]*dq[0][1] + dq[1][3]*dq[0][0]);
}

// input: dual quat. 'dq' with non-zero non-dual part
// output: unit quaternion 'q0', translation vector 't'
void DQ2QuatTrans(const float dq[2][4],
                  float q0[4], float t[3])
{
   float len = 0.0;
   for (int i=0; i<4; i++) len += dq[0][i] * dq[0][i];
   len = sqrt(len);
   for (int i=0; i<4; i++) q0[i] = dq[0][i] / len;
   t[0] = 2.0*(-dq[1][0]*dq[0][1] + dq[1][1]*dq[0][0] - dq[1][2]*dq[0][3] + dq[1][3]*dq[0][2]) / len;
   t[1] = 2.0*(-dq[1][0]*dq[0][2] + dq[1][1]*dq[0][3] + dq[1][2]*dq[0][0] - dq[1][3]*dq[0][1]) / len;
   t[2] = 2.0*(-dq[1][0]*dq[0][3] - dq[1][1]*dq[0][2] + dq[1][2]*dq[0][1] + dq[1][3]*dq[0][0]) / len;
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

struct inputs
{
	float4 position;
	float4 normal;
	float4 weights;
	int4 matrixIndices;
};

struct outputs
{
	float4 hPosition;
	float4 hNormal;
};

float3x4 DQToMatrix(float4 Qn, float4 Qd)
{
	float3x4 M;
	float len2 = dot(Qn, Qn);
	float w = Qn.x, x = Qn.y, y = Qn.z, z = Qn.w;
	float t0 = Qd.x, t1 = Qd.y, t2 = Qd.z, t3 = Qd.w;

	M[0][0] = w*w + x*x - y*y - z*z; M[0][1] = 2*x*y - 2*w*z; M[0][2] = 2*x*z + 2*w*y;
	M[1][0] = 2*x*y + 2*w*z; M[1][1] = w*w + y*y - x*x - z*z; M[1][2] = 2*y*z - 2*w*x;
	M[2][0] = 2*x*z - 2*w*y; M[2][1] = 2*y*z + 2*w*x; M[2][2] = w*w + z*z - x*x - y*y;

	M[0][3] = -2*t0*x + 2*w*t1 - 2*t2*z + 2*y*t3;
	M[1][3] = -2*t0*y + 2*t1*z - 2*x*t3 + 2*w*t2;
	M[2][3] = -2*t0*z + 2*x*t2 + 2*w*t3 - 2*t1*y;

	M /= len2;

	return M;
}

// basic dual quaternion skinning:
outputs dqs(inputs IN,
			float4x4 modelViewProj,
			float4x4 modelViewIT,
			float2x4 boneDQ[100])
{
	outputs OUT;

	float2x4 blendDQ = IN.weights.x*boneDQ[IN.matrixIndices.x];
	blendDQ += IN.weights.y*boneDQ[IN.matrixIndices.y];
	blendDQ += IN.weights.z*boneDQ[IN.matrixIndices.z];
	blendDQ += IN.weights.w*boneDQ[IN.matrixIndices.w];

	float3x4 M = DQToMatrix(blendDQ.row(0), blendDQ.row(1));
	float3 position = mul(M, IN.position);
	float3 normal = mul(M, IN.normal);

	OUT.hPosition = mul(modelViewProj, float4(position, 1.0));
	OUT.hNormal = mul(modelViewIT, float4(normal, 0.0));

	return OUT;
}

// per-vertex antipodality handling (this is the most robust, but not the most efficient way):
outputs dqsAntipod(inputs IN,
			float4x4 modelViewProj,
			float4x4 modelViewIT,
			float2x4 boneDQ[100])
{
	outputs OUT;

	float2x4 dq0 = boneDQ[IN.matrixIndices.x];
	float2x4 dq1 = boneDQ[IN.matrixIndices.y];
	float2x4 dq2 = boneDQ[IN.matrixIndices.z];
	float2x4 dq3 = boneDQ[IN.matrixIndices.w];

	if (dot(dq0.row(0), dq1.row(0)) < 0.0) dq1 *= -1.0f;
	if (dot(dq0.row(0), dq2.row(0)) < 0.0) dq2 *= -1.0f;
	if (dot(dq0.row(0), dq3.row(0)) < 0.0) dq3 *= -1.0f;

	float2x4 blendDQ = IN.weights.x*dq0;
	blendDQ += IN.weights.y*dq1;
	blendDQ += IN.weights.z*dq2;
	blendDQ += IN.weights.w*dq3;

	float3x4 M = DQToMatrix(blendDQ.row(0), blendDQ.row(1));
	float3 position = mul(M, IN.position);
	float3 normal = mul(M, IN.normal);

	OUT.hPosition = mul(modelViewProj, float4(position, 1.0));
	OUT.hNormal = mul(modelViewIT, float4(normal, 0.0));

	return OUT;
}

// optimized version (avoids dual quaternion - matrix conversion):
outputs dqsFast(inputs IN,
			float4x4 modelViewProj,
			float4x4 modelViewIT,
			float2x4 boneDQ[100])
{
	outputs OUT;

	float2x4 blendDQ = IN.weights.x*boneDQ[IN.matrixIndices.x];
	blendDQ += IN.weights.y*boneDQ[IN.matrixIndices.y];
	blendDQ += IN.weights.z*boneDQ[IN.matrixIndices.z];
	blendDQ += IN.weights.w*boneDQ[IN.matrixIndices.w];

	float len = length(blendDQ[0]);
	blendDQ /= len;

	float3 position = IN.position.xyz() + 2.0f*cross(blendDQ.row(0).yzw(), cross(blendDQ.row(0).yzw(), IN.position.xyz()) + blendDQ[0].x*IN.position.xyz());
	float3 trans = 2.0f*(blendDQ.row(0).x*blendDQ.row(1).yzw() - blendDQ.row(1).x*blendDQ.row(0).yzw() + cross(blendDQ.row(0).yzw(), blendDQ.row(1).yzw()));
	position += trans;

	float3 inpNormal = IN.normal.xyz();
	float3 normal = inpNormal + 2.0f*cross(blendDQ.row(0).yzw(), cross(blendDQ.row(0).yzw(), inpNormal) + blendDQ[0].x*inpNormal);

	OUT.hPosition = modelViewProj* float4(position, 1.0);
	OUT.hNormal = modelViewIT* float4(normal, 0.0);

	return OUT;
}

float3x3 adjointTransposeMatrix(float3x3 M)
{
	return aly::adjugate(M);
}

// two-phase skinning: dqsFast combined with scale/shear transformations:
outputs dqsScale(inputs IN,
			float4x4 modelViewProj,
			float4x4 modelViewIT,
			float2x4 boneDQ[100],
			float3x4 scaleM[100])
{
	outputs OUT;

	// first pass:
	float3x4 blendS = IN.weights.x*scaleM[IN.matrixIndices.x];
	blendS += IN.weights.y*scaleM[IN.matrixIndices.y];
	blendS += IN.weights.z*scaleM[IN.matrixIndices.z];
	blendS += IN.weights.w*scaleM[IN.matrixIndices.w];

	float3 pass1_position = mul(blendS, IN.position);
	float3x3 blendSrotAT = adjointTransposeMatrix(SubColMatrix(blendS));
	float3 pass1_normal = normalize(blendSrotAT* IN.normal.xyz());

	// second pass:
	float2x4 blendDQ = IN.weights.x*boneDQ[IN.matrixIndices.x];
	blendDQ += IN.weights.y*boneDQ[IN.matrixIndices.y];
	blendDQ += IN.weights.z*boneDQ[IN.matrixIndices.z];
	blendDQ += IN.weights.w*boneDQ[IN.matrixIndices.w];

	float len = length(blendDQ[0]);
	blendDQ /= len;

	float3 position = pass1_position + 2.0f*cross(blendDQ.row(0).yzw(), cross(blendDQ.row(0).yzw(), pass1_position) + blendDQ.row(0).x*pass1_position);
	float3 trans = 2.0f*(blendDQ.row(0).x*blendDQ.row(1).yzw() - blendDQ.row(1).x*blendDQ.row(0).yzw() + cross(blendDQ.row(0).yzw(), blendDQ.row(1).yzw()));
	position += trans;
	float3 normal = pass1_normal + 2.0f*cross(blendDQ.row(0).yzw(), cross(blendDQ.row(0).yzw(), pass1_normal) + blendDQ.row(0).x*pass1_normal);
	OUT.hPosition = modelViewProj* float4(position, 1.0);
	OUT.hNormal = modelViewIT* float4(normal, 0.0);
	return OUT;
}

}

