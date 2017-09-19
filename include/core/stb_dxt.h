// stb_dxt.h - Real-Time DXT1/DXT5 compressor 
// Based on original by fabian "ryg" giesen v1.04
// Custom version, modified by Yann Collet
//
/*
 BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:

 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above
 copyright notice, this list of conditions and the following disclaimer
 in the documentation and/or other materials provided with the
 distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 You can contact the author at :
 - RygsDXTc source repository : http://code.google.com/p/rygsdxtc/

 */
// use '#define STB_DXT_IMPLEMENTATION' before including to create the implementation
//
// USAGE:
//   call stb_compress_dxt_block() for every block (you must pad)
//     source should be a 4x4 block of RGBA data in row-major order;
//     A is ignored if you specify alpha=0; you can turn on dithering
//     and "high quality" using mode.
//
// version history:
//   v1.06  - (cyan) implement Fabian Giesen's comments
//   v1.05  - (cyan) speed optimizations
//   v1.04  - (ryg) default to no rounding bias for lerped colors (as per S3TC/DX10 spec);
//            single color match fix (allow for inexact color interpolation);
//            optimal DXT5 index finder; "high quality" mode that runs multiple refinement steps.
//   v1.03  - (stb) endianness support
//   v1.02  - (stb) fix alpha encoding bug
//   v1.01  - (stb) fix bug converting to RGB that messed up quality, thanks ryg & cbloom
//   v1.00  - (stb) first release
#ifndef STB_INCLUDE_STB_DXT_H
#define STB_INCLUDE_STB_DXT_H

//*******************************************************************
// Enable custom Optimisations
// Comment this define if you want to revert to ryg's original code
#define NEW_OPTIMISATIONS
//*******************************************************************

// compression mode (bitflags)
#define STB_DXT_NORMAL    0
#define STB_DXT_DITHER    1   // use dithering. dubious win. never use for normal maps and the like!
#define STB_DXT_HIGHQUAL  2   // high quality mode, does two refinement steps instead of 1. ~30-40% slower.

void rygCompress(unsigned char *dst, unsigned char *src, int w, int h,
		int isDxt5);

// TODO remove these, not working properly..
void rygCompressYCoCg(unsigned char *dst, unsigned char *src, int w, int h);
void linearize(unsigned char * dst, const unsigned char * src, int n);

void stb_compress_dxt_block(unsigned char *dest, const unsigned char *src,
		int alpha, int mode);
#define STB_COMPRESS_DXT_BLOCK

// configuration options for DXT encoder. set them in the project/makefile or just define
// them at the top.

// STB_DXT_USE_ROUNDING_BIAS
//     use a rounding bias during color interpolation. this is closer to what "ideal"
//     interpolation would do but doesn't match the S3TC/DX10 spec. old versions (pre-1.03)
//     implicitly had this turned on. 
//
//     in case you're targeting a specific type of hardware (e.g. console programmers):
//     NVidia and Intel GPUs (as of 2010) as well as DX9 ref use DXT decoders that are closer
//     to STB_DXT_USE_ROUNDING_BIAS. AMD/ATI, S3 and DX10 ref are closer to rounding with no bias.
//     you also see "(a*5 + b*3) / 8" on some old GPU designs.
// #define STB_DXT_USE_ROUNDING_BIAS

#include <stdlib.h>
#include <math.h>
#include <stddef.h>
#include <string.h> // memset
#include <assert.h>
#include <iostream>
#include <algorithm>
namespace aly {
unsigned char stb__Expand5[32];
unsigned char stb__Expand6[64];
unsigned char stb__OMatch5[256][2];
unsigned char stb__OMatch6[256][2];
unsigned char stb__QuantRBTab[256 + 16];
unsigned char stb__QuantGTab[256 + 16];

int stb__Mul8Bit(int a, int b);
int stb__sclamp(float y, int p0, int p1);

void stb__From16Bit(unsigned char *out, unsigned short v);
unsigned short stb__As16Bit(int r, int g, int b);

// linear interpolation at 1/3 point between a and b, using desired rounding type
int stb__Lerp13(int a, int b);

// lerp RGB color
void stb__Lerp13RGB(unsigned char *out, unsigned char *p1, unsigned char *p2);

/****************************************************************************/

// compute table to reproduce constant colors as accurately as possible
void stb__PrepareOptTable(unsigned char *Table, const unsigned char *expand,
		int size);
void stb__EvalColors(unsigned char *color, unsigned short c0,
		unsigned short c1);

// Block dithering function. Simply dithers a block to 565 RGB.
// (Floyd-Steinberg)
void stb__DitherBlock(unsigned char *dest, unsigned char *block);
// The color matching function
unsigned int stb__MatchColorsBlock(unsigned char *block, unsigned char *color,
		int dither);
// The color optimization function. (Clever code, part 1)
void stb__OptimizeColorsBlock(unsigned char *block, unsigned short *pmax16,
		unsigned short *pmin16);
int stb__sclamp(float y, int p0, int p1);
// The refinement function. (Clever code, part 2)
// Tries to optimize colors to suit block contents better.
// (By solving a least squares system via normal equations+Cramer's rule)
int stb__RefineBlock(unsigned char *block, unsigned short *pmax16,
		unsigned short *pmin16, unsigned int mask);

// Color block compression
void stb__CompressColorBlock(unsigned char *dest, unsigned char *block,
		int mode);

// Alpha block compression (this is easy for a change)
void stb__CompressAlphaBlock(unsigned char *dest, unsigned char *src, int mode);

void stb__InitDXT();
void stb_compress_dxt_block(unsigned char *dest, const unsigned char *src,
		int alpha, int mode);
int imin(int x, int y);
void extractBlock(const unsigned char *src, int x, int y, int w, int h,
		unsigned char *block);

// should be a pretty optimized 0-255 clamper
unsigned char clamp255(int n);

void rgbToYCoCgBlock(unsigned char * dst, const unsigned char * src);

void rygCompress(unsigned char *dst, unsigned char *src, int w, int h,
		int isDxt5);

void rygCompressYCoCg(unsigned char *dst, unsigned char *src, int w, int h);
void stbgl__compress(unsigned char *p, unsigned char *rgba, int w, int h,
		int isDxt5);
unsigned char linearize(unsigned char inByte);
void linearize(unsigned char * dst, const unsigned char * src, int n);

}
#endif // STB_INCLUDE_STB_DXT_H
