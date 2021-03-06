/*
 * DualQuatSkinning.h
 *
 *  Created on: Oct 18, 2018
 *      Author: blake
 */

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
#ifndef SRC_MATH_DUALQUATSKINNING_H_
#define SRC_MATH_DUALQUATSKINNING_H_
#include "math/AlloyMath.h"
#include "math/AlloyVector.h"

namespace aly {
struct QuatInputs {
	float3 position;
	float3 normal;
	float weights[4];
	int indexes[4];
};

struct QuatOutputs {
	float4 position;
	float4 normal;
};
float2x4 MakeDualQuat(const float4& q0, const float3& t);
void DqToRotAndTrans(const float2x4 dq, float4& q0, float3& t);
void DqToUnitRotAndTrans(const float2x4& dq, float4& q0, float3& t);
float4x4 DqToMatrix(float4 Qn, float4 Qd);
QuatOutputs DqSkinning(QuatInputs qin, float4x4 modelViewProj,
		float4x4 modelViewIT, const std::vector<float2x4>& boneDQ);
QuatOutputs DqAntipod(QuatInputs IN, float4x4 modelViewProj,
		float4x4 modelViewIT, const std::vector<float2x4>& boneDQ);
QuatOutputs DqFast(QuatInputs IN, float4x4 modelViewProj, float4x4 modelViewIT,
		const std::vector<float2x4>& boneDQ);
QuatOutputs DqScale(QuatInputs IN, float4x4 modelViewProj, float4x4 modelViewIT,
		const std::vector<float2x4>& boneDQ,
		const std::vector<float4x4>& scaleM);
}
#endif /* SRC_MATH_DUALQUATSKINNING_H_ */
