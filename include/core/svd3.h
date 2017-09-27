/**************************************************************************
 **
 **  svd3
 **
 ** Quick singular value decomposition as described by:
 ** A. McAdams, A. Selle, R. Tamstorf, J. Teran and E. Sifakis,
 ** Computing the Singular Value Decomposition of 3x3 matrices
 ** with minimal branching and elementary floating point operations,
 **  University of Wisconsin - Madison technical report TR1690, May 2011
 **
 **	OPTIMIZED GPU VERSION
 ** 	Implementation by: Eric Jang
 **
 **  13 Apr 2014
 **
 **************************************************************************/

#ifndef INCLUDE_CORE_SVD3_H_
#define INCLUDE_CORE_SVD3_H_
namespace aly {
void svd2(
		float a11,float a12,float a21,float a22,
		float& u11,float& u12,float& u21,float& u22,
		float& s1,float& s2,
		float& v11,float& v12,float& v21,float& v22);
void svd3(
		// input A
		float a11, float a12, float a13, float a21, float a22, float a23,
		float a31, float a32, float a33,
		// output U
		float &u11, float &u12, float &u13, float &u21, float &u22, float &u23,
		float &u31, float &u32, float &u33,
		// output S
		float &s11, float &s12, float &s13, float &s21, float &s22, float &s23,
		float &s31, float &s32, float &s33,
		// output V
		float &v11, float &v12, float &v13, float &v21, float &v22, float &v23,
		float &v31, float &v32, float &v33);
}

#endif /* INCLUDE_CORE_SVD3_H_ */
