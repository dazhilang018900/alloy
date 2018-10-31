#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include "image/tinyexr.h"
#include "common/miniz.h"
#ifdef _OPENMP
#include <omp.h>
#endif

namespace {

bool IsBigEndian(void) {
union {
	unsigned int i;
	char c[4];
} bint = { 0x01020304 };

return bint.c[0] == 1;
}

void swap2(unsigned short *val) {
unsigned short tmp = *val;
unsigned char *dst = (unsigned char *) val;
unsigned char *src = (unsigned char *) &tmp;

dst[0] = src[1];
dst[1] = src[0];
}

void swap4(unsigned int *val) {
unsigned int tmp = *val;
unsigned char *dst = (unsigned char *) val;
unsigned char *src = (unsigned char *) &tmp;

dst[0] = src[3];
dst[1] = src[2];
dst[2] = src[1];
dst[3] = src[0];
}

void swap8(unsigned long long *val) {
unsigned long long tmp = (*val);
unsigned char *dst = (unsigned char *) val;
unsigned char *src = (unsigned char *) &tmp;

dst[0] = src[7];
dst[1] = src[6];
dst[2] = src[5];
dst[3] = src[4];
dst[4] = src[3];
dst[5] = src[2];
dst[6] = src[1];
dst[7] = src[0];
}

// https://gist.github.com/rygorous/2156668
// Reuse MINIZ_LITTLE_ENDIAN flag from miniz.
union FP32 {
unsigned int u;
float f;
struct {
#if MINIZ_LITTLE_ENDIAN
	unsigned int Mantissa :23;
	unsigned int Exponent :8;
	unsigned int Sign :1;
#else
	unsigned int Sign : 1;
	unsigned int Exponent : 8;
	unsigned int Mantissa : 23;
#endif
} s;
};

union FP16 {
unsigned short u;
struct {
#if MINIZ_LITTLE_ENDIAN
	unsigned int Mantissa :10;
	unsigned int Exponent :5;
	unsigned int Sign :1;
#else
	unsigned int Sign : 1;
	unsigned int Exponent : 5;
	unsigned int Mantissa : 10;
#endif
} s;
};

FP32 half_to_float(FP16 h) {
static const FP32 magic = { 113 << 23 };
static const unsigned int shifted_exp = 0x7c00 << 13; // exponent mask after shift
FP32 o;

o.u = (h.u & 0x7fff) << 13;            // exponent/mantissa bits
unsigned int exp_ = shifted_exp & o.u; // just the exponent
o.u += (127 - 15) << 23;               // exponent adjust

// handle exponent special cases
if (exp_ == shifted_exp)   // Inf/NaN?
	o.u += (128 - 16) << 23; // extra exp adjust
else if (exp_ == 0)        // Zero/Denormal?
		{
	o.u += 1 << 23; // extra exp adjust
	o.f -= magic.f; // renormalize
}

o.u |= (h.u & 0x8000) << 16; // sign bit
return o;
}

FP16 float_to_half_full(FP32 f) {
FP16 o = { 0 };

// Based on ISPC reference code (with minor modifications)
if (f.s.Exponent == 0) // Signed zero/denormal (which will underflow)
	o.s.Exponent = 0;
else if (f.s.Exponent == 255) // Inf or NaN (all exponent bits set)
		{
	o.s.Exponent = 31;
	o.s.Mantissa = f.s.Mantissa ? 0x200 : 0; // NaN->qNaN and Inf->Inf
} else                                     // Normalized number
{
	// Exponent unbias the single, then bias the halfp
	int newexp = f.s.Exponent - 127 + 15;
	if (newexp >= 31) // Overflow, return signed infinity
		o.s.Exponent = 31;
	else if (newexp <= 0) // Underflow
			{
		if ((14 - newexp) <= 24) // Mantissa might be non-zero
				{
			unsigned int mant = f.s.Mantissa | 0x800000; // Hidden 1 bit
			o.s.Mantissa = mant >> (14 - newexp);
			if ((mant >> (13 - newexp)) & 1) // Check for rounding
				o.u++; // Round, might overflow into exp bit, but this is OK
		}
	} else {
		o.s.Exponent = newexp;
		o.s.Mantissa = f.s.Mantissa >> 13;
		if (f.s.Mantissa & 0x1000) // Check for rounding
			o.u++;                   // Round, might overflow to inf, this is OK
	}
}

o.s.Sign = f.s.Sign;
return o;
}

// NOTE: From OpenEXR code
// #define IMF_INCREASING_Y  0
// #define IMF_DECREASING_Y  1
// #define IMF_RAMDOM_Y    2
//
// #define IMF_NO_COMPRESSION  0
// #define IMF_RLE_COMPRESSION 1
// #define IMF_ZIPS_COMPRESSION  2
// #define IMF_ZIP_COMPRESSION 3
// #define IMF_PIZ_COMPRESSION 4
// #define IMF_PXR24_COMPRESSION 5
// #define IMF_B44_COMPRESSION 6
// #define IMF_B44A_COMPRESSION  7

const char *ReadString(std::string &s, const char *ptr) {
// Read untile NULL(\0).
const char *p = ptr;
const char *q = ptr;
while ((*q) != 0)
	q++;

s = std::string(p, q);

return q + 1; // skip '\0'
}

const char *ReadAttribute(std::string &name, std::string &ty,
	std::vector<unsigned char> &data, const char *ptr) {

if ((*ptr) == 0) {
	// end of attribute.
	return NULL;
}

const char *p = ReadString(name, ptr);

p = ReadString(ty, p);

int dataLen;
memcpy(&dataLen, p, sizeof(int));
p += 4;

if (IsBigEndian()) {
	swap4(reinterpret_cast<unsigned int *>(&dataLen));
}

data.resize(dataLen);
memcpy(&data.at(0), p, dataLen);
p += dataLen;

return p;
}

void WriteAttribute(FILE *fp, const char *name, const char *type,
	const unsigned char *data, int len) {
size_t n = fwrite(name, 1, strlen(name) + 1, fp);
assert(n == strlen(name) + 1);

n = fwrite(type, 1, strlen(type) + 1, fp);
assert(n == strlen(type) + 1);

int outLen = len;
if (IsBigEndian()) {
	swap4(reinterpret_cast<unsigned int *>(&outLen));
}
n = fwrite(&outLen, 1, sizeof(int), fp);
assert(n == sizeof(int));

n = fwrite(data, 1, len, fp);
assert(n == (size_t )len);

(void) n;
}

void WriteAttributeToMemory(std::vector<unsigned char> &out, const char *name,
	const char *type, const unsigned char *data, int len) {
out.insert(out.end(), name, name + strlen(name) + 1);
out.insert(out.end(), type, type + strlen(type) + 1);

int outLen = len;
if (IsBigEndian()) {
	swap4(reinterpret_cast<unsigned int *>(&outLen));
}
out.insert(out.end(), reinterpret_cast<unsigned char *>(&outLen),
		reinterpret_cast<unsigned char *>(&outLen) + sizeof(int));
out.insert(out.end(), data, data + len);
}

typedef struct {
std::string name; // less than 255 bytes long
int pixelType;
unsigned char pLinear;
int xSampling;
int ySampling;
} ChannelInfo;

void ReadChannelInfo(std::vector<ChannelInfo> &channels,
	const std::vector<unsigned char> &data) {
const char *p = reinterpret_cast<const char *>(&data.at(0));

for (;;) {
	if ((*p) == 0) {
		break;
	}
	ChannelInfo info;
	p = ReadString(info.name, p);

	memcpy(&info.pixelType, p, sizeof(int));
	p += 4;
	info.pLinear = p[0];                     // uchar
	p += 1 + 3;                              // reserved: uchar[3]
	memcpy(&info.xSampling, p, sizeof(int)); // int
	p += 4;
	memcpy(&info.ySampling, p, sizeof(int)); // int
	p += 4;

	if (IsBigEndian()) {
		swap4(reinterpret_cast<unsigned int *>(&info.pixelType));
		swap4(reinterpret_cast<unsigned int *>(&info.xSampling));
		swap4(reinterpret_cast<unsigned int *>(&info.ySampling));
	}

	channels.push_back(info);
}
}

void WriteChannelInfo(std::vector<unsigned char> &data,
	const std::vector<ChannelInfo> &channels) {

size_t sz = 0;

// Calculate total size.
for (size_t c = 0; c < channels.size(); c++) {
	sz += strlen(channels[c].name.c_str()) + 1; // +1 for \0
	sz += 16;                                   // 4 * int
}
data.resize(sz + 1);

unsigned char *p = &data.at(0);

for (size_t c = 0; c < channels.size(); c++) {
	memcpy(p, channels[c].name.c_str(), strlen(channels[c].name.c_str()));
	p += strlen(channels[c].name.c_str());
	(*p) = '\0';
	p++;

	int pixelType = channels[c].pixelType;
	int xSampling = channels[c].xSampling;
	int ySampling = channels[c].ySampling;
	if (IsBigEndian()) {
		swap4(reinterpret_cast<unsigned int *>(&pixelType));
		swap4(reinterpret_cast<unsigned int *>(&xSampling));
		swap4(reinterpret_cast<unsigned int *>(&ySampling));
	}

	memcpy(p, &pixelType, sizeof(int));
	p += sizeof(int);

	(*p) = channels[c].pLinear;
	p += 4;

	memcpy(p, &xSampling, sizeof(int));
	p += sizeof(int);

	memcpy(p, &ySampling, sizeof(int));
	p += sizeof(int);
}

(*p) = '\0';
}



//
// PIZ compress/uncompress, based on OpenEXR's ImfPizCompressor.cpp
//
// -----------------------------------------------------------------
// Copyright (c) 2004, Industrial Light & Magic, a division of Lucas
// Digital Ltd. LLC)
// (3 clause BSD license)
//

struct PIZChannelData {
unsigned short *start;
unsigned short *end;
int nx;
int ny;
int ys;
int size;
};

//-----------------------------------------------------------------------------
//
//  16-bit Haar Wavelet encoding and decoding
//
//  The source code in this file is derived from the encoding
//  and decoding routines written by Christian Rouet for his
//  PIZ image file format.
//
//-----------------------------------------------------------------------------

//
// Wavelet basis functions without modulo arithmetic; they produce
// the best compression ratios when the wavelet-transformed data are
// Huffman-encoded, but the wavelet transform works only for 14-bit
// data (untransformed data values must be less than (1 << 14)).
//

#if 0 // @todo
inline void wenc14(unsigned short a, unsigned short b, unsigned short &l,
	unsigned short &h) {
short as = a;
short bs = b;

short ms = (as + bs) >> 1;
short ds = as - bs;

l = ms;
h = ds;
}
#endif

inline void wdec14(unsigned short l, unsigned short h, unsigned short &a,
	unsigned short &b) {
short ls = l;
short hs = h;

int hi = hs;
int ai = ls + (hi & 1) + (hi >> 1);

short as = ai;
short bs = ai - hi;

a = as;
b = bs;
}

//
// Wavelet basis functions with modulo arithmetic; they work with full
// 16-bit data, but Huffman-encoding the wavelet-transformed data doesn't
// compress the data quite as well.
//

const int NBITS = 16;
const int A_OFFSET = 1 << (NBITS - 1);
//const int M_OFFSET = 1 << (NBITS - 1);
const int MOD_MASK = (1 << NBITS) - 1;

#if 0 // @ood
inline void wenc16(unsigned short a, unsigned short b, unsigned short &l,
	unsigned short &h) {
int ao = (a + A_OFFSET) & MOD_MASK;
int m = ((ao + b) >> 1);
int d = ao - b;

if (d < 0)
m = (m + M_OFFSET) & MOD_MASK;

d &= MOD_MASK;

l = m;
h = d;
}
#endif

inline void wdec16(unsigned short l, unsigned short h, unsigned short &a,
	unsigned short &b) {
int m = l;
int d = h;
int bb = (m - (d >> 1)) & MOD_MASK;
int aa = (d + bb - A_OFFSET) & MOD_MASK;
b = bb;
a = aa;
}

//
// 2D Wavelet encoding:
//

#if 0 // @todo
void wav2Encode(unsigned short *in, // io: values are transformed in place
	int nx,// i : x size
	int ox,// i : x offset
	int ny,// i : y size
	int oy,// i : y offset
	unsigned short mx)// i : maximum in[x][y] value
{
bool w14 = (mx < (1 << 14));
int n = (nx > ny) ? ny : nx;
int p = 1;  // == 1 <<  level
int p2 = 2;// == 1 << (level+1)

//
// Hierachical loop on smaller dimension n
//

while (p2 <= n) {
	unsigned short *py = in;
	unsigned short *ey = in + oy * (ny - p2);
	int oy1 = oy * p;
	int oy2 = oy * p2;
	int ox1 = ox * p;
	int ox2 = ox * p2;
	unsigned short i00, i01, i10, i11;

	//
	// Y loop
	//

	for (; py <= ey; py += oy2) {
		unsigned short *px = py;
		unsigned short *ex = py + ox * (nx - p2);

		//
		// X loop
		//

		for (; px <= ex; px += ox2) {
			unsigned short *p01 = px + ox1;
			unsigned short *p10 = px + oy1;
			unsigned short *p11 = p10 + ox1;

			//
			// 2D wavelet encoding
			//

			if (w14) {
				wenc14(*px, *p01, i00, i01);
				wenc14(*p10, *p11, i10, i11);
				wenc14(i00, i10, *px, *p10);
				wenc14(i01, i11, *p01, *p11);
			} else {
				wenc16(*px, *p01, i00, i01);
				wenc16(*p10, *p11, i10, i11);
				wenc16(i00, i10, *px, *p10);
				wenc16(i01, i11, *p01, *p11);
			}
		}

		//
		// Encode (1D) odd column (still in Y loop)
		//

		if (nx & p) {
			unsigned short *p10 = px + oy1;

			if (w14)
			wenc14(*px, *p10, i00, *p10);
			else
			wenc16(*px, *p10, i00, *p10);

			*px = i00;
		}
	}

	//
	// Encode (1D) odd line (must loop in X)
	//

	if (ny & p) {
		unsigned short *px = py;
		unsigned short *ex = py + ox * (nx - p2);

		for (; px <= ex; px += ox2) {
			unsigned short *p01 = px + ox1;

			if (w14)
			wenc14(*px, *p01, i00, *p01);
			else
			wenc16(*px, *p01, i00, *p01);

			*px = i00;
		}
	}

	//
	// Next level
	//

	p = p2;
	p2 <<= 1;
}
}
#endif

//
// 2D Wavelet decoding:
//

void wav2Decode(unsigned short *in, // io: values are transformed in place
	int nx,             // i : x size
	int ox,             // i : x offset
	int ny,             // i : y size
	int oy,             // i : y offset
	unsigned short mx)  // i : maximum in[x][y] value
	{
bool w14 = (mx < (1 << 14));
int n = (nx > ny) ? ny : nx;
int p = 1;
int p2;

//
// Search max level
//

while (p <= n)
	p <<= 1;

p >>= 1;
p2 = p;
p >>= 1;

//
// Hierarchical loop on smaller dimension n
//

while (p >= 1) {
	unsigned short *py = in;
	unsigned short *ey = in + oy * (ny - p2);
	int oy1 = oy * p;
	int oy2 = oy * p2;
	int ox1 = ox * p;
	int ox2 = ox * p2;
	unsigned short i00, i01, i10, i11;

	//
	// Y loop
	//

	for (; py <= ey; py += oy2) {
		unsigned short *px = py;
		unsigned short *ex = py + ox * (nx - p2);

		//
		// X loop
		//

		for (; px <= ex; px += ox2) {
			unsigned short *p01 = px + ox1;
			unsigned short *p10 = px + oy1;
			unsigned short *p11 = p10 + ox1;

			//
			// 2D wavelet decoding
			//

			if (w14) {
				wdec14(*px, *p10, i00, i10);
				wdec14(*p01, *p11, i01, i11);
				wdec14(i00, i01, *px, *p01);
				wdec14(i10, i11, *p10, *p11);
			} else {
				wdec16(*px, *p10, i00, i10);
				wdec16(*p01, *p11, i01, i11);
				wdec16(i00, i01, *px, *p01);
				wdec16(i10, i11, *p10, *p11);
			}
		}

		//
		// Decode (1D) odd column (still in Y loop)
		//

		if (nx & p) {
			unsigned short *p10 = px + oy1;

			if (w14)
				wdec14(*px, *p10, i00, *p10);
			else
				wdec16(*px, *p10, i00, *p10);

			*px = i00;
		}
	}

	//
	// Decode (1D) odd line (must loop in X)
	//

	if (ny & p) {
		unsigned short *px = py;
		unsigned short *ex = py + ox * (nx - p2);

		for (; px <= ex; px += ox2) {
			unsigned short *p01 = px + ox1;

			if (w14)
				wdec14(*px, *p01, i00, *p01);
			else
				wdec16(*px, *p01, i00, *p01);

			*px = i00;
		}
	}

	//
	// Next level
	//

	p2 = p;
	p >>= 1;
}
}

//-----------------------------------------------------------------------------
//
//	16-bit Huffman compression and decompression.
//
//	The source code in this file is derived from the 8-bit
//	Huffman compression and decompression routines written
//	by Christian Rouet for his PIZ image file format.
//
//-----------------------------------------------------------------------------

// Adds some modification for tinyexr.

const int HUF_ENCBITS = 16; // literal (value) bit length
const int HUF_DECBITS = 14; // decoding bit size (>= 8)

const int HUF_ENCSIZE = (1 << HUF_ENCBITS) + 1; // encoding table size
const int HUF_DECSIZE = 1 << HUF_DECBITS;       // decoding table size
const int HUF_DECMASK = HUF_DECSIZE - 1;

struct HufDec { // short code		long code
//-------------------------------
int len :8;  // code length		0
int lit :24; // lit			p size
int *p;       // 0			lits
};

inline long long hufLength(long long code) {
return code & 63;
}

inline long long hufCode(long long code) {
return code >> 6;
}

#if 0
inline void outputBits(int nBits, long long bits, long long &c, int &lc,
	char *&out) {
c <<= nBits;
lc += nBits;

c |= bits;

while (lc >= 8)
*out++ = (c >> (lc -= 8));
}
#endif

inline long long getBits(int nBits, long long &c, int &lc, const char *&in) {
while (lc < nBits) {
	c = (c << 8) | *(unsigned char *) (in++);
	lc += 8;
}

lc -= nBits;
return (c >> lc) & ((1 << nBits) - 1);
}

//
// ENCODING TABLE BUILDING & (UN)PACKING
//

//
// Build a "canonical" Huffman code table:
//	- for each (uncompressed) symbol, hcode contains the length
//	  of the corresponding code (in the compressed data)
//	- canonical codes are computed and stored in hcode
//	- the rules for constructing canonical codes are as follows:
//	  * shorter codes (if filled with zeroes to the right)
//	    have a numerically higher value than longer codes
//	  * for codes with the same length, numerical values
//	    increase with numerical symbol values
//	- because the canonical code table can be constructed from
//	  symbol lengths alone, the code table can be transmitted
//	  without sending the actual code values
//	- see http://www.compressconsult.com/huffman/
//

void hufCanonicalCodeTable(long long hcode[HUF_ENCSIZE]) {
long long n[59];

//
// For each i from 0 through 58, count the
// number of different codes of length i, and
// store the count in n[i].
//

for (int i = 0; i <= 58; ++i)
	n[i] = 0;

for (int i = 0; i < HUF_ENCSIZE; ++i)
	n[hcode[i]] += 1;

//
// For each i from 58 through 1, compute the
// numerically lowest code with length i, and
// store that code in n[i].
//

long long c = 0;

for (int i = 58; i > 0; --i) {
	long long nc = ((c + n[i]) >> 1);
	n[i] = c;
	c = nc;
}

//
// hcode[i] contains the length, l, of the
// code for symbol i.  Assign the next available
// code of length l to the symbol and store both
// l and the code in hcode[i].
//

for (int i = 0; i < HUF_ENCSIZE; ++i) {
	int l =(int) hcode[i];

	if (l > 0)
		hcode[i] = l | (n[l]++ << 6);
}
}

//
// Compute Huffman codes (based on frq input) and store them in frq:
//	- code structure is : [63:lsb - 6:msb] | [5-0: bit length];
//	- max code length is 58 bits;
//	- codes outside the range [im-iM] have a null length (unused values);
//	- original frequencies are destroyed;
//	- encoding tables are used by hufEncode() and hufBuildDecTable();
//
#if 0 // @todo

struct FHeapCompare {
bool operator()(long long *a, long long *b) {return *a > *b;}
};

void hufBuildEncTable(
	long long *frq, // io: input frequencies [HUF_ENCSIZE], output table
	int *im,//  o: min frq index
	int *iM)//  o: max frq index
{
//
// This function assumes that when it is called, array frq
// indicates the frequency of all possible symbols in the data
// that are to be Huffman-encoded.  (frq[i] contains the number
// of occurrences of symbol i in the data.)
//
// The loop below does three things:
//
// 1) Finds the minimum and maximum indices that point
//    to non-zero entries in frq:
//
//     frq[im] != 0, and frq[i] == 0 for all i < im
//     frq[iM] != 0, and frq[i] == 0 for all i > iM
//
// 2) Fills array fHeap with pointers to all non-zero
//    entries in frq.
//
// 3) Initializes array hlink such that hlink[i] == i
//    for all array entries.
//

int hlink[HUF_ENCSIZE];
long long *fHeap[HUF_ENCSIZE];

*im = 0;

while (!frq[*im])
(*im)++;

int nf = 0;

for (int i = *im; i < HUF_ENCSIZE; i++) {
	hlink[i] = i;

	if (frq[i]) {
		fHeap[nf] = &frq[i];
		nf++;
		*iM = i;
	}
}

//
// Add a pseudo-symbol, with a frequency count of 1, to frq;
// adjust the fHeap and hlink array accordingly.  Function
// hufEncode() uses the pseudo-symbol for run-length encoding.
//

(*iM)++;
frq[*iM] = 1;
fHeap[nf] = &frq[*iM];
nf++;

//
// Build an array, scode, such that scode[i] contains the number
// of bits assigned to symbol i.  Conceptually this is done by
// constructing a tree whose leaves are the symbols with non-zero
// frequency:
//
//     Make a heap that contains all symbols with a non-zero frequency,
//     with the least frequent symbol on top.
//
//     Repeat until only one symbol is left on the heap:
//
//         Take the two least frequent symbols off the top of the heap.
//         Create a new node that has first two nodes as children, and
//         whose frequency is the sum of the frequencies of the first
//         two nodes.  Put the new node back into the heap.
//
// The last node left on the heap is the root of the tree.  For each
// leaf node, the distance between the root and the leaf is the length
// of the code for the corresponding symbol.
//
// The loop below doesn't actually build the tree; instead we compute
// the distances of the leaves from the root on the fly.  When a new
// node is added to the heap, then that node's descendants are linked
// into a single linear list that starts at the new node, and the code
// lengths of the descendants (that is, their distance from the root
// of the tree) are incremented by one.
//

std::make_heap(&fHeap[0], &fHeap[nf], FHeapCompare());

long long scode[HUF_ENCSIZE];
memset(scode, 0, sizeof(long long) * HUF_ENCSIZE);

while (nf > 1) {
	//
	// Find the indices, mm and m, of the two smallest non-zero frq
	// values in fHeap, add the smallest frq to the second-smallest
	// frq, and remove the smallest frq value from fHeap.
	//

	int mm = fHeap[0] - frq;
	std::pop_heap(&fHeap[0], &fHeap[nf], FHeapCompare());
	--nf;

	int m = fHeap[0] - frq;
	std::pop_heap(&fHeap[0], &fHeap[nf], FHeapCompare());

	frq[m] += frq[mm];
	std::push_heap(&fHeap[0], &fHeap[nf], FHeapCompare());

	//
	// The entries in scode are linked into lists with the
	// entries in hlink serving as "next" pointers and with
	// the end of a list marked by hlink[j] == j.
	//
	// Traverse the lists that start at scode[m] and scode[mm].
	// For each element visited, increment the length of the
	// corresponding code by one bit. (If we visit scode[j]
	// during the traversal, then the code for symbol j becomes
	// one bit longer.)
	//
	// Merge the lists that start at scode[m] and scode[mm]
	// into a single list that starts at scode[m].
	//

	//
	// Add a bit to all codes in the first list.
	//

	for (int j = m; true; j = hlink[j]) {
		scode[j]++;

		assert(scode[j] <= 58);

		if (hlink[j] == j) {
			//
			// Merge the two lists.
			//

			hlink[j] = mm;
			break;
		}
	}

	//
	// Add a bit to all codes in the second list
	//

	for (int j = mm; true; j = hlink[j]) {
		scode[j]++;

		assert(scode[j] <= 58);

		if (hlink[j] == j)
		break;
	}
}

//
// Build a canonical Huffman code table, replacing the code
// lengths in scode with (code, code length) pairs.  Copy the
// code table from scode into frq.
//

hufCanonicalCodeTable(scode);
memcpy(frq, scode, sizeof(long long) * HUF_ENCSIZE);
}
#endif

//
// Pack an encoding table:
//	- only code lengths, not actual codes, are stored
//	- runs of zeroes are compressed as follows:
//
//	  unpacked		packed
//	  --------------------------------
//	  1 zero		0	(6 bits)
//	  2 zeroes		59
//	  3 zeroes		60
//	  4 zeroes		61
//	  5 zeroes		62
//	  n zeroes (6 or more)	63 n-6	(6 + 8 bits)
//

const int SHORT_ZEROCODE_RUN = 59;
const int LONG_ZEROCODE_RUN = 63;
const int SHORTEST_LONG_RUN = 2 + LONG_ZEROCODE_RUN - SHORT_ZEROCODE_RUN;
//const int LONGEST_LONG_RUN = 255 + SHORTEST_LONG_RUN;

#if 0
void hufPackEncTable(const long long *hcode, // i : encoding table [HUF_ENCSIZE]
	int im,// i : min hcode index
	int iM,// i : max hcode index
	char **pcode)//  o: ptr to packed table (updated)
{
char *p = *pcode;
long long c = 0;
int lc = 0;

for (; im <= iM; im++) {
	int l = hufLength(hcode[im]);

	if (l == 0) {
		int zerun = 1;

		while ((im < iM) && (zerun < LONGEST_LONG_RUN)) {
			if (hufLength(hcode[im + 1]) > 0)
			break;
			im++;
			zerun++;
		}

		if (zerun >= 2) {
			if (zerun >= SHORTEST_LONG_RUN) {
				outputBits(6, LONG_ZEROCODE_RUN, c, lc, p);
				outputBits(8, zerun - SHORTEST_LONG_RUN, c, lc, p);
			} else {
				outputBits(6, SHORT_ZEROCODE_RUN + zerun - 2, c, lc, p);
			}
			continue;
		}
	}

	outputBits(6, l, c, lc, p);
}

if (lc > 0)
*p++ = (unsigned char)(c << (8 - lc));

*pcode = p;
}
#endif

//
// Unpack an encoding table packed by hufPackEncTable():
//

bool hufUnpackEncTable(const char **pcode, // io: ptr to packed table (updated)
	int ni,             // i : input size (in bytes)
	int im,             // i : min hcode index
	int iM,             // i : max hcode index
	long long *hcode)   //  o: encoding table [HUF_ENCSIZE]
	{
memset(hcode, 0, sizeof(long long) * HUF_ENCSIZE);

const char *p = *pcode;
long long c = 0;
int lc = 0;

for (; im <= iM; im++) {
	if (p - *pcode > ni) {
		return false;
	}

	long long l = hcode[im] = getBits(6, c, lc, p); // code length

	if (l == (long long) LONG_ZEROCODE_RUN) {
		if (p - *pcode > ni) {
			return false;
		}

		int zerun = (int)(getBits(8, c, lc, p) + SHORTEST_LONG_RUN);

		if (im + zerun > iM + 1) {
			return false;
		}

		while (zerun--)
			hcode[im++] = 0;

		im--;
	} else if (l >= (long long) SHORT_ZEROCODE_RUN) {
		int zerun = (int)(l - SHORT_ZEROCODE_RUN + 2);

		if (im + zerun > iM + 1) {
			return false;
		}

		while (zerun--)
			hcode[im++] = 0;

		im--;
	}
}

*pcode = const_cast<char *>(p);

hufCanonicalCodeTable(hcode);

return true;
}

//
// DECODING TABLE BUILDING
//

//
// Clear a newly allocated decoding table so that it contains only zeroes.
//

void hufClearDecTable(HufDec *hdecod) // io: (allocated by caller)
									  //     decoding table [HUF_DECSIZE]
	{
for (int i = 0; i < HUF_DECSIZE; i++) {
	hdecod[i].len = 0;
	hdecod[i].lit = 0;
	hdecod[i].p = NULL;
}
//memset(hdecod, 0, sizeof(HufDec) * HUF_DECSIZE);
}

//
// Build a decoding hash table based on the encoding table hcode:
//	- short codes (<= HUF_DECBITS) are resolved with a single table access;
//	- long code entry allocations are not optimized, because long codes are
//	  unfrequent;
//	- decoding tables are used by hufDecode();
//

bool hufBuildDecTable(const long long *hcode, // i : encoding table
	int im,                 // i : min index in hcode
	int iM,                 // i : max index in hcode
	HufDec *hdecod)         //  o: (allocated by caller)
//     decoding table [HUF_DECSIZE]
	{
//
// Init hashtable & loop on all codes.
// Assumes that hufClearDecTable(hdecod) has already been called.
//

for (; im <= iM; im++) {
	long long c = hufCode(hcode[im]);
	int l = (int)hufLength(hcode[im]);

	if (c >> l) {
		//
		// Error: c is supposed to be an l-bit code,
		// but c contains a value that is greater
		// than the largest l-bit number.
		//

		// invalidTableEntry();
		return false;
	}

	if (l > HUF_DECBITS) {
		//
		// Long code: add a secondary entry
		//

		HufDec *pl = hdecod + (c >> (l - HUF_DECBITS));

		if (pl->len) {
			//
			// Error: a short code has already
			// been stored in table entry *pl.
			//

			// invalidTableEntry();
			return false;
		}

		pl->lit++;

		if (pl->p) {
			int *p = pl->p;
			pl->p = new int[pl->lit];

			for (int i = 0; i < pl->lit - 1; ++i)
				pl->p[i] = p[i];

			delete[] p;
		} else {
			pl->p = new int[1];
		}

		pl->p[pl->lit - 1] = im;
	} else if (l) {
		//
		// Short code: init all primary entries
		//

		HufDec *pl = hdecod + (c << (HUF_DECBITS - l));

		for (long long i = ((size_t)1) << (size_t)(HUF_DECBITS - l); i > 0; i--, pl++) {
			if (pl->len || pl->p) {
				//
				// Error: a short code or a long code has
				// already been stored in table entry *pl.
				//

				// invalidTableEntry();
				return false;
			}

			pl->len = l;
			pl->lit = im;
		}
	}
}

return true;
}

//
// Free the long code entries of a decoding table built by hufBuildDecTable()
//

void hufFreeDecTable(HufDec *hdecod) // io: Decoding table
	{
for (int i = 0; i < HUF_DECSIZE; i++) {
	if (hdecod[i].p) {
		delete[] hdecod[i].p;
		hdecod[i].p = 0;
	}
}
}

//
// ENCODING
//

#if 0 // @todo
inline void outputCode(long long code, long long &c, int &lc, char *&out) {
outputBits(hufLength(code), hufCode(code), c, lc, out);
}

inline void sendCode(long long sCode, int runCount, long long runCode,
	long long &c, int &lc, char *&out) {
//
// Output a run of runCount instances of the symbol sCount.
// Output the symbols explicitly, or if that is shorter, output
// the sCode symbol once followed by a runCode symbol and runCount
// expressed as an 8-bit number.
//

if (hufLength(sCode) + hufLength(runCode) + 8 < hufLength(sCode) * runCount) {
	outputCode(sCode, c, lc, out);
	outputCode(runCode, c, lc, out);
	outputBits(8, runCount, c, lc, out);
} else {
	while (runCount-- >= 0)
	outputCode(sCode, c, lc, out);
}
}

//
// Encode (compress) ni values based on the Huffman encoding table hcode:
//

int hufEncode// return: output size (in bits)
(const long long *hcode,// i : encoding table
	const unsigned short *in,// i : uncompressed input buffer
	const int ni,// i : input buffer size (in bytes)
	int rlc,// i : rl code
	char *out)//  o: compressed output buffer
{
char *outStart = out;
long long c = 0; // bits not yet written to out
int lc = 0;// number of valid bits in c (LSB)
int s = in[0];
int cs = 0;

//
// Loop on input values
//

for (int i = 1; i < ni; i++) {
	//
	// Count same values or send code
	//

	if (s == in[i] && cs < 255) {
		cs++;
	} else {
		sendCode(hcode[s], cs, hcode[rlc], c, lc, out);
		cs = 0;
	}

	s = in[i];
}

//
// Send remaining code
//

sendCode(hcode[s], cs, hcode[rlc], c, lc, out);

if (lc)
*out = (c << (8 - lc)) & 0xff;

return (out - outStart) * 8 + lc;
}
#endif

//
// DECODING
//

//
// In order to force the compiler to inline them,
// getChar() and getCode() are implemented as macros
// instead of "inline" functions.
//

#define getChar(c, lc, in)                                                     \
  {                                                                            \
    c = (c << 8) | *(unsigned char *)(in++);                                   \
    lc += 8;                                                                   \
  }

#define getCode(po, rlc, c, lc, in, out, oe)                                   \
  {                                                                            \
    if (po == rlc) {                                                           \
      if (lc < 8)                                                              \
        getChar(c, lc, in);                                                    \
                                                                               \
      lc -= 8;                                                                 \
                                                                               \
      unsigned char cs =(unsigned char) (c >> lc);                             \
                                                                               \
      if (out + cs > oe)                                                       \
        return false;                                                          \
                                                                               \
      unsigned short s = out[-1];                                              \
                                                                               \
      while (cs-- > 0)                                                         \
        *out++ = s;                                                            \
    } else if (out < oe) {                                                     \
      *out++ = po;                                                             \
    } else {                                                                   \
      return false;                                                            \
    }                                                                          \
  }

//
// Decode (uncompress) ni bits based on encoding & decoding tables:
//

bool hufDecode(const long long *hcode, // i : encoding table
	const HufDec *hdecod,   // i : decoding table
	const char *in,         // i : compressed input buffer
	int ni,                 // i : input size (in bits)
	int rlc,                // i : run-length code
	int no,                 // i : expected output size (in bytes)
	unsigned short *out)    //  o: uncompressed output buffer
	{
long long c = 0;
int lc = 0;
unsigned short *outb = out;
unsigned short *oe = out + no;
const char *ie = in + (ni + 7) / 8; // input byte size

//
// Loop on input bytes
//

while (in < ie) {
	getChar(c, lc, in);

	//
	// Access decoding table
	//

	while (lc >= HUF_DECBITS) {
		const HufDec pl = hdecod[(c >> (lc - HUF_DECBITS)) & HUF_DECMASK];

		if (pl.len) {
			//
			// Get short code
			//

			lc -= pl.len;
			getCode(pl.lit, rlc, c, lc, in, out, oe);
		} else {
			if (!pl.p) {
				return false;
			}
			// invalidCode(); // wrong code

			//
			// Search long code
			//

			int j;

			for (j = 0; j < pl.lit; j++) {
				int l = (int)hufLength(hcode[pl.p[j]]);

				while (lc < l && in < ie) // get more bits
					getChar(c, lc, in);

				if (lc >= l) {
					if (hufCode(hcode[pl.p[j]])
							== ((c >> (lc - l)) & (((long long) (1) << l) - 1))) {
						//
						// Found : get long code
						//

						lc -= l;
						getCode(pl.p[j], rlc, c, lc, in, out, oe);
						break;
					}
				}
			}

			if (j == pl.lit) {
				return false;
				// invalidCode(); // Not found
			}
		}
	}
}

//
// Get remaining (short) codes
//

int i = (8 - ni) & 7;
c >>= i;
lc -= i;

while (lc > 0) {
	const HufDec pl = hdecod[(c << (HUF_DECBITS - lc)) & HUF_DECMASK];

	if (pl.len) {
		lc -= pl.len;
		getCode(pl.lit, rlc, c, lc, in, out, oe);
	} else {
		return false;
		// invalidCode(); // wrong (long) code
	}
}

if (out - outb != no) {
	return false;
}
// notEnoughData ();

return true;
}

#if 0 // @todo
void countFrequencies(long long freq[HUF_ENCSIZE],
	const unsigned short data[/*n*/], int n) {
for (int i = 0; i < HUF_ENCSIZE; ++i)
freq[i] = 0;

for (int i = 0; i < n; ++i)
++freq[data[i]];
}

void writeUInt(char buf[4], unsigned int i) {
unsigned char *b = (unsigned char *)buf;

b[0] = i;
b[1] = i >> 8;
b[2] = i >> 16;
b[3] = i >> 24;
}
#endif

unsigned int readUInt(const char buf[4]) {
const unsigned char *b = (const unsigned char *) buf;

return (b[0] & 0x000000ff) | ((b[1] << 8) & 0x0000ff00)
		| ((b[2] << 16) & 0x00ff0000) | ((b[3] << 24) & 0xff000000);
}

//
// EXTERNAL INTERFACE
//

#if 0 // @todo
int hufCompress(const unsigned short raw[], int nRaw, char compressed[]) {
if (nRaw == 0)
return 0;

long long freq[HUF_ENCSIZE];

countFrequencies(freq, raw, nRaw);

int im = 0;
int iM = 0;
hufBuildEncTable(freq, &im, &iM);

char *tableStart = compressed + 20;
char *tableEnd = tableStart;
hufPackEncTable(freq, im, iM, &tableEnd);
int tableLength = tableEnd - tableStart;

char *dataStart = tableEnd;
int nBits = hufEncode(freq, raw, nRaw, iM, dataStart);
int dataLength = (nBits + 7) / 8;

writeUInt(compressed, im);
writeUInt(compressed + 4, iM);
writeUInt(compressed + 8, tableLength);
writeUInt(compressed + 12, nBits);
writeUInt(compressed + 16, 0); // room for future extensions

return dataStart + dataLength - compressed;
}
#endif

bool hufUncompress(const char compressed[], int nCompressed,
	unsigned short raw[], int nRaw) {
if (nCompressed == 0) {
	if (nRaw != 0)
		return false;

	return false;
}

int im = readUInt(compressed);
int iM = readUInt(compressed + 4);
// int tableLength = readUInt (compressed + 8);
int nBits = readUInt(compressed + 12);

if (im < 0 || im >= HUF_ENCSIZE || iM < 0 || iM >= HUF_ENCSIZE)
	return false;

const char *ptr = compressed + 20;

//
// Fast decoder needs at least 2x64-bits of compressed data, and
// needs to be run-able on this platform. Otherwise, fall back
// to the original decoder
//

// if (FastHufDecoder::enabled() && nBits > 128)
//{
//    FastHufDecoder fhd (ptr, nCompressed - (ptr - compressed), im, iM, iM);
//    fhd.decode ((unsigned char*)ptr, nBits, raw, nRaw);
//}
// else
{
	std::vector<long long> freq(HUF_ENCSIZE);
	std::vector<HufDec> hdec(HUF_DECSIZE);

	hufClearDecTable(&hdec.at(0));

	hufUnpackEncTable(&ptr, (int)(nCompressed - (ptr - compressed)), im, iM,
			&freq.at(0));

	{
		if (nBits > 8 * (nCompressed - (ptr - compressed))) {
			return false;
		}

		hufBuildDecTable(&freq.at(0), im, iM, &hdec.at(0));
		hufDecode(&freq.at(0), &hdec.at(0), ptr, nBits, iM, nRaw, raw);
	}
	// catch (...)
	//{
	//    hufFreeDecTable (hdec);
	//    throw;
	//}

	hufFreeDecTable(&hdec.at(0));
}

return true;
}

//
// Functions to compress the range of values in the pixel data
//

const int USHORT_RANGE = (1 << 16);
const int BITMAP_SIZE = (USHORT_RANGE >> 3);

#if 0 // @todo

void bitmapFromData(const unsigned short data[/*nData*/], int nData,
	unsigned char bitmap[BITMAP_SIZE],
	unsigned short &minNonZero, unsigned short &maxNonZero) {
for (int i = 0; i < BITMAP_SIZE; ++i)
bitmap[i] = 0;

for (int i = 0; i < nData; ++i)
bitmap[data[i] >> 3] |= (1 << (data[i] & 7));

bitmap[0] &= ~1; // zero is not explicitly stored in
// the bitmap; we assume that the
// data always contain zeroes
minNonZero = BITMAP_SIZE - 1;
maxNonZero = 0;

for (int i = 0; i < BITMAP_SIZE; ++i) {
	if (bitmap[i]) {
		if (minNonZero > i)
		minNonZero = i;
		if (maxNonZero < i)
		maxNonZero = i;
	}
}
}

unsigned short forwardLutFromBitmap(const unsigned char bitmap[BITMAP_SIZE],
	unsigned short lut[USHORT_RANGE]) {
int k = 0;

for (int i = 0; i < USHORT_RANGE; ++i) {
	if ((i == 0) || (bitmap[i >> 3] & (1 << (i & 7))))
	lut[i] = k++;
	else
	lut[i] = 0;
}

return k - 1; // maximum value stored in lut[],
} // i.e. number of ones in bitmap minus 1
#endif

unsigned short reverseLutFromBitmap(const unsigned char bitmap[BITMAP_SIZE],
	unsigned short lut[USHORT_RANGE]) {
int k = 0;

for (int i = 0; i < USHORT_RANGE; ++i) {
	if ((i == 0) || (bitmap[i >> 3] & (1 << (i & 7))))
		lut[k++] = i;
}

int n = k - 1;

while (k < USHORT_RANGE)
	lut[k++] = 0;

return n; // maximum k where lut[k] is non-zero,
} // i.e. number of ones in bitmap minus 1

void applyLut(const unsigned short lut[USHORT_RANGE],
	unsigned short data[/*nData*/], int nData) {
for (int i = 0; i < nData; ++i)
	data[i] = lut[data[i]];
}

#if 0 // @todo
bool CompressPiz(unsigned char *outPtr, unsigned int &outSize) {
unsigned char bitmap[BITMAP_SIZE];
unsigned short minNonZero;
unsigned short maxNonZero;

if (IsBigEndian()) {
	// @todo { PIZ compression on BigEndian architecture. }
	assert(0);
	return false;
}

std::vector<unsigned short> tmpBuffer;
int nData = tmpBuffer.size();

bitmapFromData(&tmpBuffer.at(0), nData, bitmap, minNonZero, maxNonZero);

unsigned short lut[USHORT_RANGE];
//unsigned short maxValue = forwardLutFromBitmap(bitmap, lut);
applyLut(lut, &tmpBuffer.at(0), nData);

//
// Store range compression info in _outBuffer
//

char *buf = reinterpret_cast<char *>(outPtr);

memcpy(buf, &minNonZero, sizeof(unsigned short));
buf += sizeof(unsigned short);
memcpy(buf, &maxNonZero, sizeof(unsigned short));
buf += sizeof(unsigned short);

if (minNonZero <= maxNonZero) {
	memcpy(buf, (char *)&bitmap[0] + minNonZero, maxNonZero - minNonZero + 1);
	buf += maxNonZero - minNonZero + 1;
}

#if 0 // @todo
//
// Apply wavelet encoding
//

for (int i = 0; i < channels; ++i)
{
	ChannelData &cd = _channelData[i];

	for (int j = 0; j < cd.size; ++j)
	{
		wav2Encode (cd.start + j,
				cd.nx, cd.size,
				cd.ny, cd.nx * cd.size,
				maxValue);
	}
}

//
// Apply Huffman encoding; append the result to _outBuffer
//

char *lengthPtr = buf;
int zero = 0;
memcpy(buf, &zero, sizeof(int)); buf += sizeof(int);

int length = hufCompress (_tmpBuffer, tmpBufferEnd - _tmpBuffer, buf);
memcpy(lengthPtr, tmpBuffer, length);
//Xdr::write <CharPtrIO> (lengthPtr, length);

outPtr = _outBuffer;
return buf - _outBuffer + length;
#endif
assert(0);

return true;
}
#endif

bool DecompressPiz(unsigned char *outPtr, unsigned int &outSize,
	const unsigned char *inPtr, size_t tmpBufSize,
	const std::vector<ChannelInfo> &channelInfo, int dataWidth, int numLines) {
unsigned char bitmap[BITMAP_SIZE];
unsigned short minNonZero;
unsigned short maxNonZero;

if (IsBigEndian()) {
	// @todo { PIZ compression on BigEndian architecture. }
	assert(0);
	return false;
}

memset(bitmap, 0, BITMAP_SIZE);

const unsigned char *ptr = inPtr;
minNonZero = *(reinterpret_cast<const unsigned short *>(ptr));
maxNonZero = *(reinterpret_cast<const unsigned short *>(ptr + 2));
ptr += 4;

if (maxNonZero >= BITMAP_SIZE) {
	return false;
}

if (minNonZero <= maxNonZero) {
	memcpy((char *) &bitmap[0] + minNonZero, ptr, maxNonZero - minNonZero + 1);
	ptr += maxNonZero - minNonZero + 1;
}

unsigned short lut[USHORT_RANGE];
memset(lut, 0, sizeof(unsigned short) * USHORT_RANGE);
unsigned short maxValue = reverseLutFromBitmap(bitmap, lut);

//
// Huffman decoding
//

int length;

length = *(reinterpret_cast<const int *>(ptr));
ptr += sizeof(int);

std::vector<unsigned short> tmpBuffer(tmpBufSize);
hufUncompress(reinterpret_cast<const char *>(ptr), length, &tmpBuffer.at(0),
		(int)tmpBufSize);

//
// Wavelet decoding
//

std::vector<PIZChannelData> channelData(channelInfo.size());

unsigned short *tmpBufferEnd = &tmpBuffer.at(0);

for (size_t i = 0; i < channelInfo.size(); ++i) {
	const ChannelInfo &chan = channelInfo[i];

	int pixelSize = sizeof(int); // UINT and FLOAT
	if (chan.pixelType == TINYEXR_PIXELTYPE_HALF) {
		pixelSize = sizeof(short);
	}

	channelData[i].start = tmpBufferEnd;
	channelData[i].end = channelData[i].start;
	channelData[i].nx = dataWidth;
	channelData[i].ny = numLines;
	// channelData[i].ys = 1;
	channelData[i].size = pixelSize / sizeof(short);

	tmpBufferEnd += channelData[i].nx * channelData[i].ny * channelData[i].size;
}

for (size_t i = 0; i < channelData.size(); ++i) {
	PIZChannelData &cd = channelData[i];

	for (int j = 0; j < cd.size; ++j) {
		wav2Decode(cd.start + j, cd.nx, cd.size, cd.ny, cd.nx * cd.size,
				maxValue);
	}
}

//
// Expand the pixel data to their original range
//

applyLut(lut, &tmpBuffer.at(0),(int) tmpBufSize);

// @todo { Xdr }

for (int y = 0; y < numLines; y++) {
	for (size_t i = 0; i < channelData.size(); ++i) {
		PIZChannelData &cd = channelData[i];

		// if (modp (y, cd.ys) != 0)
		//    continue;

		int n = cd.nx * cd.size;
		memcpy(outPtr, cd.end, n * sizeof(unsigned short));
		outPtr += n * sizeof(unsigned short);
		cd.end += n;
	}
}

return true;
}
//
// -----------------------------------------------------------------
//

}// namespace

int LoadEXR(float **out_rgba, int *width, int *height, const char *filename,
	const char **err) {

if (out_rgba == NULL) {
	if (err) {
		(*err) = "Invalid argument.\n";
	}
	return -1;
}

EXRImage exrImage;
InitEXRImage(&exrImage);

{
	int ret = ParseMultiChannelEXRHeaderFromFile(&exrImage, filename, err);
	if (ret != 0) {
		return ret;
	}
}

// Read HALF channel as FLOAT.
for (int i = 0; i < exrImage.num_channels; i++) {
	if (exrImage.pixel_types[i] == TINYEXR_PIXELTYPE_HALF) {
		exrImage.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
	}
}

{
	int ret = LoadMultiChannelEXRFromFile(&exrImage, filename, err);
	if (ret != 0) {
		return ret;
	}
}

// RGBA
int idxR = -1;
int idxG = -1;
int idxB = -1;
int idxA = -1;
for (int c = 0; c < exrImage.num_channels; c++) {
	if (strcmp(exrImage.channel_names[c], "R") == 0) {
		idxR = c;
	} else if (strcmp(exrImage.channel_names[c], "G") == 0) {
		idxG = c;
	} else if (strcmp(exrImage.channel_names[c], "B") == 0) {
		idxB = c;
	} else if (strcmp(exrImage.channel_names[c], "A") == 0) {
		idxA = c;
	}
}

if (idxR == -1) {
	if (err) {
		(*err) = "R channel not found\n";
	}

	// @todo { free exrImage }
	return -1;
}

if (idxG == -1) {
	if (err) {
		(*err) = "G channel not found\n";
	}
	// @todo { free exrImage }
	return -1;
}

if (idxB == -1) {
	if (err) {
		(*err) = "B channel not found\n";
	}
	// @todo { free exrImage }
	return -1;
}

(*out_rgba) = (float *) malloc(
		4 * sizeof(float) * exrImage.width * exrImage.height);
for (int i = 0; i < exrImage.width * exrImage.height; i++) {
	(*out_rgba)[4 * i + 0] =
			reinterpret_cast<float **>(exrImage.images)[idxR][i];
	(*out_rgba)[4 * i + 1] =
			reinterpret_cast<float **>(exrImage.images)[idxG][i];
	(*out_rgba)[4 * i + 2] =
			reinterpret_cast<float **>(exrImage.images)[idxB][i];
	if (idxA > 0) {
		(*out_rgba)[4 * i + 3] =
				reinterpret_cast<float **>(exrImage.images)[idxA][i];
	} else {
		(*out_rgba)[4 * i + 3] = 1.0;
	}
}

(*width) = exrImage.width;
(*height) = exrImage.height;

// @todo { free exrImage }
return 0;
}

int ParseEXRHeaderFromMemory(int *width, int *height,
	const unsigned char *memory) {

if (memory == NULL) {
	// Invalid argument
	return -1;
}

const char *buf = reinterpret_cast<const char *>(memory);

const char *marker = &buf[0];

// Header check.
{
	const char header[] = { 0x76, 0x2f, 0x31, 0x01 };

	if (memcmp(marker, header, 4) != 0) {
		// if (err) {
		//  (*err) = "Header mismatch.";
		//}
		return -3;
	}
	marker += 4;
}

// Version, scanline.
{
	// must be [2, 0, 0, 0]
	if (marker[0] != 2 || marker[1] != 0 || marker[2] != 0 || marker[3] != 0) {
		// if (err) {
		//  (*err) = "Unsupported version or scanline.";
		//}
		return -4;
	}

	marker += 4;
}

int dx = -1;
int dy = -1;
int dw = -1;
int dh = -1;
int numChannels = -1;
std::vector<ChannelInfo> channels;

// Read attributes
for (;;) {
	std::string attrName;
	std::string attrType;
	std::vector<unsigned char> data;
	const char *marker_next = ReadAttribute(attrName, attrType, data, marker);
	if (marker_next == NULL) {
		marker++; // skip '\0'
		break;
	}

	if (attrName.compare("compression") == 0) {
		// must be 0:No compression, 1: RLE or 3: ZIP
		//      if (data[0] != 0 && data[0] != 1 && data[0] != 3) {

		//	mwkm
		//	0 : NO_COMPRESSION
		//	1 : RLE
		//	2 : ZIPS (Single scanline)
		//	3 : ZIP (16-line block)
		//	4 : PIZ (32-line block)
		if (data[0] > 4) {
			// if (err) {
			//  (*err) = "Unsupported compression type.";
			//}
			return -5;
		}

	} else if (attrName.compare("channels") == 0) {

		// name: zero-terminated string, from 1 to 255 bytes long
		// pixel type: int, possible values are: UINT = 0 HALF = 1 FLOAT = 2
		// pLinear: unsigned char, possible values are 0 and 1
		// reserved: three chars, should be zero
		// xSampling: int
		// ySampling: int

		ReadChannelInfo(channels, data);

		numChannels = (int)channels.size();

		if (numChannels < 1) {
			// if (err) {
			//  (*err) = "Invalid channels format.";
			//}
			return -6;
		}

	} else if (attrName.compare("dataWindow") == 0) {
		memcpy(&dx, &data.at(0), sizeof(int));
		memcpy(&dy, &data.at(4), sizeof(int));
		memcpy(&dw, &data.at(8), sizeof(int));
		memcpy(&dh, &data.at(12), sizeof(int));
		if (IsBigEndian()) {
			swap4(reinterpret_cast<unsigned int *>(&dx));
			swap4(reinterpret_cast<unsigned int *>(&dy));
			swap4(reinterpret_cast<unsigned int *>(&dw));
			swap4(reinterpret_cast<unsigned int *>(&dh));
		}
	} else if (attrName.compare("displayWindow") == 0) {
		int x, y, w, h;
		memcpy(&x, &data.at(0), sizeof(int));
		memcpy(&y, &data.at(4), sizeof(int));
		memcpy(&w, &data.at(8), sizeof(int));
		memcpy(&h, &data.at(12), sizeof(int));
		if (IsBigEndian()) {
			swap4(reinterpret_cast<unsigned int *>(&x));
			swap4(reinterpret_cast<unsigned int *>(&y));
			swap4(reinterpret_cast<unsigned int *>(&w));
			swap4(reinterpret_cast<unsigned int *>(&h));
		}
	}

	marker = marker_next;
}

assert(dx >= 0);
assert(dy >= 0);
assert(dw >= 0);
assert(dh >= 0);
assert(numChannels >= 1);

int dataWidth = dw - dx + 1;
int dataHeight = dh - dy + 1;

(*width) = dataWidth;
(*height) = dataHeight;

return 0;
}

int LoadEXRFromMemory(float *out_rgba, const unsigned char *memory,
	const char **err) {

if (out_rgba == NULL || memory == NULL) {
	if (err) {
		(*err) = "Invalid argument.\n";
	}
	return -1;
}

EXRImage exrImage;
InitEXRImage(&exrImage);
int ret = LoadMultiChannelEXRFromMemory(&exrImage, memory, err);
if (ret != 0) {
	return ret;
}

// RGBA
int idxR = -1;
int idxG = -1;
int idxB = -1;
int idxA = -1;
for (int c = 0; c < exrImage.num_channels; c++) {
	if (strcmp(exrImage.channel_names[c], "R") == 0) {
		idxR = c;
	} else if (strcmp(exrImage.channel_names[c], "G") == 0) {
		idxG = c;
	} else if (strcmp(exrImage.channel_names[c], "B") == 0) {
		idxB = c;
	} else if (strcmp(exrImage.channel_names[c], "A") == 0) {
		idxA = c;
	}
}

if (idxR == -1) {
	if (err) {
		(*err) = "R channel not found\n";
	}

	// @todo { free exrImage }
	return -1;
}

if (idxG == -1) {
	if (err) {
		(*err) = "G channel not found\n";
	}
	// @todo { free exrImage }
	return -1;
}

if (idxB == -1) {
	if (err) {
		(*err) = "B channel not found\n";
	}
	// @todo { free exrImage }
	return -1;
}

// Assume `out_rgba` have enough memory allocated.
for (int i = 0; i < exrImage.width * exrImage.height; i++) {
	out_rgba[4 * i + 0] = reinterpret_cast<float **>(exrImage.images)[idxR][i];
	out_rgba[4 * i + 1] = reinterpret_cast<float **>(exrImage.images)[idxG][i];
	out_rgba[4 * i + 2] = reinterpret_cast<float **>(exrImage.images)[idxB][i];
	if (idxA > 0) {
		out_rgba[4 * i + 3] =
				reinterpret_cast<float **>(exrImage.images)[idxA][i];
	} else {
		out_rgba[4 * i + 3] = 1.0;
	}
}

return 0;
}

int LoadMultiChannelEXRFromFile(EXRImage *exrImage, const char *filename,
	const char **err) {
if (exrImage == NULL) {
	if (err) {
		(*err) = "Invalid argument.";
	}
	return -1;
}

FILE *fp = fopen(filename, "rb");
if (!fp) {
	if (err) {
		(*err) = "Cannot read file.";
	}
	return -1;
}

size_t filesize;
// Compute size
fseek(fp, 0, SEEK_END);
filesize = ftell(fp);
fseek(fp, 0, SEEK_SET);

std::vector<unsigned char> buf(filesize); // @todo { use mmap }
{
	size_t ret;
	ret = fread(&buf[0], 1, filesize, fp);
	assert(ret == filesize);
	fclose(fp);
	(void) ret;
}

return LoadMultiChannelEXRFromMemory(exrImage, &buf.at(0), err);
}

int LoadMultiChannelEXRFromMemory(EXRImage *exrImage,
	const unsigned char *memory, const char **err) {
if (exrImage == NULL || memory == NULL) {
	if (err) {
		(*err) = "Invalid argument.";
	}
	return -1;
}

const char *buf = reinterpret_cast<const char *>(memory);

const char *head = &buf[0];
const char *marker = &buf[0];

// Header check.
{
	const char header[] = { 0x76, 0x2f, 0x31, 0x01 };

	if (memcmp(marker, header, 4) != 0) {
		if (err) {
			(*err) = "Header mismatch.";
		}
		return -3;
	}
	marker += 4;
}

// Version, scanline.
{
	// must be [2, 0, 0, 0]
	if (marker[0] != 2 || marker[1] != 0 || marker[2] != 0 || marker[3] != 0) {
		if (err) {
			(*err) = "Unsupported version or scanline.";
		}
		return -4;
	}

	marker += 4;
}

int dx = -1;
int dy = -1;
int dw = -1;
int dh = -1;
int numScanlineBlocks = 1; // 16 for ZIP compression.
int compressionType = -1;
int numChannels = -1;
unsigned char lineOrder = 0; // 0 -> increasing y; 1 -> decreasing
std::vector<ChannelInfo> channels;

// Read attributes
for (;;) {
	std::string attrName;
	std::string attrType;
	std::vector<unsigned char> data;
	const char *marker_next = ReadAttribute(attrName, attrType, data, marker);
	if (marker_next == NULL) {
		marker++; // skip '\0'
		break;
	}

	if (attrName.compare("compression") == 0) {
		//	mwkm
		//	0 : NO_COMPRESSION
		//	1 : RLE
		//	2 : ZIPS (Single scanline)
		//	3 : ZIP (16-line block)
		//	4 : PIZ (32-line block)
		if (data[0] != 0 && data[0] != 2 && data[0] != 3 && data[0] != 4) {

			if (err) {
				(*err) = "Unsupported compression type.";
			}
			return -5;
		}

		compressionType = data[0];

		if (compressionType == 3) { // ZIP
			numScanlineBlocks = 16;
		} else if (compressionType == 4) { // PIZ
			numScanlineBlocks = 32;
		}

	} else if (attrName.compare("channels") == 0) {

		// name: zero-terminated string, from 1 to 255 bytes long
		// pixel type: int, possible values are: UINT = 0 HALF = 1 FLOAT = 2
		// pLinear: unsigned char, possible values are 0 and 1
		// reserved: three chars, should be zero
		// xSampling: int
		// ySampling: int

		ReadChannelInfo(channels, data);

		numChannels = (int)channels.size();

		if (numChannels < 1) {
			if (err) {
				(*err) = "Invalid channels format.";
			}
			return -6;
		}

	} else if (attrName.compare("dataWindow") == 0) {
		memcpy(&dx, &data.at(0), sizeof(int));
		memcpy(&dy, &data.at(4), sizeof(int));
		memcpy(&dw, &data.at(8), sizeof(int));
		memcpy(&dh, &data.at(12), sizeof(int));
		if (IsBigEndian()) {
			swap4(reinterpret_cast<unsigned int *>(&dx));
			swap4(reinterpret_cast<unsigned int *>(&dy));
			swap4(reinterpret_cast<unsigned int *>(&dw));
			swap4(reinterpret_cast<unsigned int *>(&dh));
		}
	} else if (attrName.compare("displayWindow") == 0) {
		int x, y, w, h;
		memcpy(&x, &data.at(0), sizeof(int));
		memcpy(&y, &data.at(4), sizeof(int));
		memcpy(&w, &data.at(8), sizeof(int));
		memcpy(&h, &data.at(12), sizeof(int));
		if (IsBigEndian()) {
			swap4(reinterpret_cast<unsigned int *>(&x));
			swap4(reinterpret_cast<unsigned int *>(&y));
			swap4(reinterpret_cast<unsigned int *>(&w));
			swap4(reinterpret_cast<unsigned int *>(&h));
		}
	} else if (attrName.compare("lineOrder") == 0) {
		memcpy(&lineOrder, &data.at(0), sizeof(lineOrder));
	}

	marker = marker_next;
}

assert(dx >= 0);
assert(dy >= 0);
assert(dw >= 0);
assert(dh >= 0);
assert(numChannels >= 1);

int dataWidth = dw - dx + 1;
int dataHeight = dh - dy + 1;

// Read offset tables.
int numBlocks = dataHeight / numScanlineBlocks;
if (numBlocks * numScanlineBlocks < dataHeight) {
	numBlocks++;
}

std::vector<long long> offsets(numBlocks);

for (int y = 0; y < numBlocks; y++) {
	long long offset;
	memcpy(&offset, marker, sizeof(long long));
	if (IsBigEndian()) {
		swap8(reinterpret_cast<unsigned long long *>(&offset));
	}
	marker += sizeof(long long); // = 8
	offsets[y] = offset;
}

//	mwkm
//	Supported : 0, 2(ZIPS), 3(ZIP), 4(PIZ)
if (compressionType != 0 && compressionType != 2 && compressionType != 3
		&& compressionType != 4) {
	if (err) {
		(*err) = "Unsupported format.";
	}
	return -10;
}

exrImage->images = reinterpret_cast<unsigned char **>((float **) malloc(
		sizeof(float *) * numChannels));

std::vector<size_t> channelOffsetList(numChannels);
int pixelDataSize = 0;
size_t channelOffset = 0;
for (int c = 0; c < numChannels; c++) {
	channelOffsetList[c] = channelOffset;
	if (channels[c].pixelType == TINYEXR_PIXELTYPE_HALF) {
		pixelDataSize += sizeof(unsigned short);
		channelOffset += sizeof(unsigned short);
		// Alloc internal image for half type.
		if (exrImage->requested_pixel_types[c] == TINYEXR_PIXELTYPE_HALF) {
			exrImage->images[c] =
					reinterpret_cast<unsigned char *>((unsigned short *) malloc(
							sizeof(unsigned short) * dataWidth * dataHeight));
		} else if (exrImage->requested_pixel_types[c] ==
		TINYEXR_PIXELTYPE_FLOAT) {
			exrImage->images[c] =
					reinterpret_cast<unsigned char *>((float *) malloc(
							sizeof(float) * dataWidth * dataHeight));
		} else {
			assert(0);
		}
	} else if (channels[c].pixelType == TINYEXR_PIXELTYPE_FLOAT) {
		pixelDataSize += sizeof(float);
		channelOffset += sizeof(float);
		exrImage->images[c] =
				reinterpret_cast<unsigned char *>((float *) malloc(
						sizeof(float) * dataWidth * dataHeight));
	} else if (channels[c].pixelType == TINYEXR_PIXELTYPE_UINT) {
		pixelDataSize += sizeof(unsigned int);
		channelOffset += sizeof(unsigned int);
		exrImage->images[c] =
				reinterpret_cast<unsigned char *>((unsigned int *) malloc(
						sizeof(unsigned int) * dataWidth * dataHeight));
	} else {
		assert(0);
	}
}

#ifdef _OPENMP
#pragma omp parallel for
#endif
for (int y = 0; y < numBlocks; y++) {
	const unsigned char *dataPtr = reinterpret_cast<const unsigned char *>(head
			+ offsets[y]);
	// 4 byte: scan line
	// 4 byte: data size
	// ~     : pixel data(uncompressed or compressed)
	int lineNo;
	memcpy(&lineNo, dataPtr, sizeof(int));
	int dataLen;
	memcpy(&dataLen, dataPtr + 4, sizeof(int));
	if (IsBigEndian()) {
		swap4(reinterpret_cast<unsigned int *>(&lineNo));
		swap4(reinterpret_cast<unsigned int *>(&dataLen));
	}

	int endLineNo = (std::min)(lineNo + numScanlineBlocks, dataHeight);

	int numLines = endLineNo - lineNo;

	if (compressionType == 4) { // PIZ
		// Allocate original data size.
		std::vector<unsigned char> outBuf(dataWidth * numLines * pixelDataSize);
		unsigned int dstLen;
		size_t tmpBufLen = dataWidth * numLines * pixelDataSize;

		DecompressPiz(reinterpret_cast<unsigned char *>(&outBuf.at(0)), dstLen,
				dataPtr + 8, tmpBufLen, channels, dataWidth, numLines);

		bool isBigEndian = IsBigEndian();

		// For ZIP_COMPRESSION:
		//   pixel sample data for channel 0 for scanline 0
		//   pixel sample data for channel 1 for scanline 0
		//   pixel sample data for channel ... for scanline 0
		//   pixel sample data for channel n for scanline 0
		//   pixel sample data for channel 0 for scanline 1
		//   pixel sample data for channel 1 for scanline 1
		//   pixel sample data for channel ... for scanline 1
		//   pixel sample data for channel n for scanline 1
		//   ...
		for (int c = 0; c < numChannels; c++) {

			if (channels[c].pixelType == TINYEXR_PIXELTYPE_HALF) {
				for (int v = 0; v < numLines; v++) {
					const unsigned short *linePtr =
							reinterpret_cast<unsigned short *>(&outBuf.at(
									v * pixelDataSize * dataWidth
											+ channelOffsetList[c] * dataWidth));
					for (int u = 0; u < dataWidth; u++) {
						FP16 hf;

						hf.u = linePtr[u];

						if (isBigEndian) {
							swap2(reinterpret_cast<unsigned short *>(&hf.u));
						}

						if (exrImage->requested_pixel_types[c] ==
						TINYEXR_PIXELTYPE_HALF) {
							unsigned short *image =
									reinterpret_cast<unsigned short **>(exrImage->images)[c];
							if (lineOrder == 0) {
								image += (lineNo + v) * dataWidth + u;
							} else {
								image += (dataHeight - 1 - (lineNo + v))
										* dataWidth + u;
							}
							*image = hf.u;
						} else { // HALF -> FLOAT
							FP32 f32 = half_to_float(hf);
							float *image =
									reinterpret_cast<float **>(exrImage->images)[c];
							if (lineOrder == 0) {
								image += (lineNo + v) * dataWidth + u;
							} else {
								image += (dataHeight - 1 - (lineNo + v))
										* dataWidth + u;
							}
							*image = f32.f;
						}
					}
				}
			} else if (channels[c].pixelType == TINYEXR_PIXELTYPE_UINT) {

				assert(
						exrImage->requested_pixel_types[c] == TINYEXR_PIXELTYPE_UINT);

				for (int v = 0; v < numLines; v++) {
					const unsigned int *linePtr =
							reinterpret_cast<unsigned int *>(&outBuf.at(
									v * pixelDataSize * dataWidth
											+ channelOffsetList[c] * dataWidth));
					for (int u = 0; u < dataWidth; u++) {

						unsigned int val = linePtr[u];

						if (isBigEndian) {
							swap4(&val);
						}

						unsigned int *image =
								reinterpret_cast<unsigned int **>(exrImage->images)[c];
						if (lineOrder == 0) {
							image += (lineNo + v) * dataWidth + u;
						} else {
							image += (dataHeight - 1 - (lineNo + v)) * dataWidth
									+ u;
						}
						*image = val;
					}
				}
			} else if (channels[c].pixelType == TINYEXR_PIXELTYPE_FLOAT) {
				assert(
						exrImage->requested_pixel_types[c] == TINYEXR_PIXELTYPE_FLOAT);
				for (int v = 0; v < numLines; v++) {
					const float *linePtr = reinterpret_cast<float *>(&outBuf.at(
							v * pixelDataSize * dataWidth
									+ channelOffsetList[c] * dataWidth));
					for (int u = 0; u < dataWidth; u++) {

						float val = linePtr[u];

						if (isBigEndian) {
							swap4(reinterpret_cast<unsigned int *>(&val));
						}

						float *image =
								reinterpret_cast<float **>(exrImage->images)[c];
						if (lineOrder == 0) {
							image += (lineNo + v) * dataWidth + u;
						} else {
							image += (dataHeight - 1 - (lineNo + v)) * dataWidth
									+ u;
						}
						*image = val;
					}
				}
			} else {
				assert(0);
			}
		}

		//	mwkm, ZIPS or ZIP both good to go
	} else if (compressionType == 2 || compressionType == 3) { // ZIP

	// Allocate original data size.
		std::vector<unsigned char> outBuf(dataWidth * numLines * pixelDataSize);

		unsigned long dstLen = (unsigned long)outBuf.size();
		aly::DecompressZipEXR(reinterpret_cast<unsigned char *>(&outBuf.at(0)), dstLen,dataPtr + 8, dataLen);

		bool isBigEndian = IsBigEndian();

		// For ZIP_COMPRESSION:
		//   pixel sample data for channel 0 for scanline 0
		//   pixel sample data for channel 1 for scanline 0
		//   pixel sample data for channel ... for scanline 0
		//   pixel sample data for channel n for scanline 0
		//   pixel sample data for channel 0 for scanline 1
		//   pixel sample data for channel 1 for scanline 1
		//   pixel sample data for channel ... for scanline 1
		//   pixel sample data for channel n for scanline 1
		//   ...
		for (int c = 0; c < numChannels; c++) {

			if (channels[c].pixelType == TINYEXR_PIXELTYPE_HALF) {
				for (int v = 0; v < numLines; v++) {
					const unsigned short *linePtr =
							reinterpret_cast<unsigned short *>(&outBuf.at(
									v * pixelDataSize * dataWidth
											+ channelOffsetList[c] * dataWidth));
					for (int u = 0; u < dataWidth; u++) {
						FP16 hf;

						hf.u = linePtr[u];

						if (isBigEndian) {
							swap2(reinterpret_cast<unsigned short *>(&hf.u));
						}

						if (exrImage->requested_pixel_types[c] ==
						TINYEXR_PIXELTYPE_HALF) {
							unsigned short *image =
									reinterpret_cast<unsigned short **>(exrImage->images)[c];
							if (lineOrder == 0) {
								image += (lineNo + v) * dataWidth + u;
							} else {
								image += (dataHeight - 1 - (lineNo + v))
										* dataWidth + u;
							}
							*image = hf.u;
						} else { // HALF -> FLOAT
							FP32 f32 = half_to_float(hf);
							float *image =
									reinterpret_cast<float **>(exrImage->images)[c];
							if (lineOrder == 0) {
								image += (lineNo + v) * dataWidth + u;
							} else {
								image += (dataHeight - 1 - (lineNo + v))
										* dataWidth + u;
							}
							*image = f32.f;
						}
					}
				}
			} else if (channels[c].pixelType == TINYEXR_PIXELTYPE_UINT) {

				assert(
						exrImage->requested_pixel_types[c] == TINYEXR_PIXELTYPE_UINT);

				for (int v = 0; v < numLines; v++) {
					const unsigned int *linePtr =
							reinterpret_cast<unsigned int *>(&outBuf.at(
									v * pixelDataSize * dataWidth
											+ channelOffsetList[c] * dataWidth));
					for (int u = 0; u < dataWidth; u++) {

						unsigned int val = linePtr[u];

						if (isBigEndian) {
							swap4(&val);
						}

						unsigned int *image =
								reinterpret_cast<unsigned int **>(exrImage->images)[c];
						if (lineOrder == 0) {
							image += (lineNo + v) * dataWidth + u;
						} else {
							image += (dataHeight - 1 - (lineNo + v)) * dataWidth
									+ u;
						}
						*image = val;
					}
				}
			} else if (channels[c].pixelType == TINYEXR_PIXELTYPE_FLOAT) {
				assert(
						exrImage->requested_pixel_types[c] == TINYEXR_PIXELTYPE_FLOAT);
				for (int v = 0; v < numLines; v++) {
					const float *linePtr = reinterpret_cast<float *>(&outBuf.at(
							v * pixelDataSize * dataWidth
									+ channelOffsetList[c] * dataWidth));
					for (int u = 0; u < dataWidth; u++) {

						float val = linePtr[u];

						if (isBigEndian) {
							swap4(reinterpret_cast<unsigned int *>(&val));
						}

						float *image =
								reinterpret_cast<float **>(exrImage->images)[c];
						if (lineOrder == 0) {
							image += (lineNo + v) * dataWidth + u;
						} else {
							image += (dataHeight - 1 - (lineNo + v)) * dataWidth
									+ u;
						}
						*image = val;
					}
				}
			} else {
				assert(0);
			}
		}

	} else if (compressionType == 0) { // No compression

		bool isBigEndian = IsBigEndian();

		for (int c = 0; c < numChannels; c++) {

			if (channels[c].pixelType == TINYEXR_PIXELTYPE_HALF) {

				const unsigned short *linePtr =
						reinterpret_cast<const unsigned short *>(dataPtr + 8
								+ c * dataWidth * sizeof(unsigned short));

				if (exrImage->requested_pixel_types[c] == TINYEXR_PIXELTYPE_HALF) {
					unsigned short *outLine =
							reinterpret_cast<unsigned short *>(exrImage->images[c]);
					if (lineOrder == 0) {
						outLine += y * dataWidth;
					} else {
						outLine += (dataHeight - 1 - y) * dataWidth;
					}

					for (int u = 0; u < dataWidth; u++) {
						FP16 hf;

						hf.u = linePtr[u];

						if (isBigEndian) {
							swap2(reinterpret_cast<unsigned short *>(&hf.u));
						}

						outLine[u] = hf.u;
					}
				} else if (exrImage->requested_pixel_types[c] ==
				TINYEXR_PIXELTYPE_FLOAT) {
					float *outLine =
							reinterpret_cast<float *>(exrImage->images[c]);
					if (lineOrder == 0) {
						outLine += y * dataWidth;
					} else {
						outLine += (dataHeight - 1 - y) * dataWidth;
					}

					for (int u = 0; u < dataWidth; u++) {
						FP16 hf;

						hf.u = linePtr[u];

						if (isBigEndian) {
							swap2(reinterpret_cast<unsigned short *>(&hf.u));
						}

						FP32 f32 = half_to_float(hf);

						outLine[u] = f32.f;
					}
				} else {
					assert(0);
				}
			} else if (channels[c].pixelType == TINYEXR_PIXELTYPE_FLOAT) {

				const float *linePtr = reinterpret_cast<const float *>(dataPtr
						+ 8 + c * dataWidth * sizeof(float));

				float *outLine = reinterpret_cast<float *>(exrImage->images[c]);
				if (lineOrder == 0) {
					outLine += y * dataWidth;
				} else {
					outLine += (dataHeight - 1 - y) * dataWidth;
				}

				for (int u = 0; u < dataWidth; u++) {
					float val = linePtr[u];

					if (isBigEndian) {
						swap4(reinterpret_cast<unsigned int *>(&val));
					}

					outLine[u] = val;
				}
			} else if (channels[c].pixelType == TINYEXR_PIXELTYPE_UINT) {

				const unsigned int *linePtr =
						reinterpret_cast<const unsigned int *>(dataPtr + 8
								+ c * dataWidth * sizeof(unsigned int));

				unsigned int *outLine =
						reinterpret_cast<unsigned int *>(exrImage->images[c]);
				if (lineOrder == 0) {
					outLine += y * dataWidth;
				} else {
					outLine += (dataHeight - 1 - y) * dataWidth;
				}

				for (int u = 0; u < dataWidth; u++) {
					unsigned int val = linePtr[u];

					if (isBigEndian) {
						swap4(reinterpret_cast<unsigned int *>(&val));
					}

					outLine[u] = val;
				}
			}
		}
	}
} // omp parallel

{
	exrImage->channel_names = (const char **) malloc(
			sizeof(const char *) * numChannels);
	for (int c = 0; c < numChannels; c++) {
#ifdef _WIN32
		exrImage->channel_names[c] = _strdup(channels[c].name.c_str());
#else
		exrImage->channel_names[c] = strdup(channels[c].name.c_str());
#endif
	}
	exrImage->num_channels = numChannels;

	exrImage->width = dataWidth;
	exrImage->height = dataHeight;

	// Fill with requested_pixel_types.
	exrImage->pixel_types = (int *) malloc(sizeof(int *) * numChannels);
	for (int c = 0; c < numChannels; c++) {
		exrImage->pixel_types[c] = exrImage->requested_pixel_types[c];
	}
}

return 0; // OK
}

// @deprecated
#if 0
int SaveEXR(const float *in_rgba, int width, int height, const char *filename,
	const char **err) {
if (in_rgba == NULL || filename == NULL) {
	if (err) {
		(*err) = "Invalid argument.";
	}
	return -1;
}

FILE *fp = fopen(filename, "wb");
if (!fp) {
	if (err) {
		(*err) = "Cannot write a file.";
	}
	return -1;
}

// Header
{
	const char header[] = {0x76, 0x2f, 0x31, 0x01};
	size_t n = fwrite(header, 1, 4, fp);
	assert(n == 4);
}

// Version, scanline.
{
	const char marker[] = {2, 0, 0, 0};
	size_t n = fwrite(marker, 1, 4, fp);
	assert(n == 4);
}

int numScanlineBlocks = 16; // 16 for ZIP compression.

// Write attributes.
{
	unsigned char data[] = {
		'A', 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 'B',
		0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 'G', 0,
		1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 'R', 0, 1,
		0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0}; // last 0 =
														 // terminator.

	WriteAttribute(fp, "channels", "chlist", data, 18 * 4 + 1);// +1 = null
}

{
	int compressionType = 3; // ZIP compression
	WriteAttribute(fp, "compression", "compression",
			reinterpret_cast<const unsigned char *>(&compressionType),
			1);
}

{
	int data[4] = {0, 0, width - 1, height - 1};
	WriteAttribute(fp, "dataWindow", "box2i",
			reinterpret_cast<const unsigned char *>(data),
			sizeof(int) * 4);
	WriteAttribute(fp, "displayWindow", "box2i",
			reinterpret_cast<const unsigned char *>(data),
			sizeof(int) * 4);
}

{
	unsigned char lineOrder = 0; // increasingY
	WriteAttribute(fp, "lineOrder", "lineOrder", &lineOrder, 1);
}

{
	float aspectRatio = 1.0f;
	WriteAttribute(fp, "pixelAspectRatio", "float",
			reinterpret_cast<const unsigned char *>(&aspectRatio),
			sizeof(float));
}

{
	float center[2] = {0.0f, 0.0f};
	WriteAttribute(fp, "screenWindowCenter", "v2f",
			reinterpret_cast<const unsigned char *>(center),
			2 * sizeof(float));
}

{
	float w = (float)width;
	WriteAttribute(fp, "screenWindowWidth", "float",
			reinterpret_cast<const unsigned char *>(&w), sizeof(float));
}

{ // end of header
	unsigned char e = 0;
	fwrite(&e, 1, 1, fp);
}

int numBlocks = height / numScanlineBlocks;
if (numBlocks * numScanlineBlocks < height) {
	numBlocks++;
}

std::vector<long long> offsets(numBlocks);

size_t headerSize = ftell(fp); // sizeof(header)
long long offset =
headerSize +
numBlocks * sizeof(long long);// sizeof(header) + sizeof(offsetTable)

std::vector<unsigned char> data;

for (int i = 0; i < numBlocks; i++) {
	int startY = numScanlineBlocks * i;
	int endY = (std::min)(numScanlineBlocks * (i + 1), height);
	int h = endY - startY;

	std::vector<unsigned short> buf(4 * width * h);

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < width; x++) {
			FP32 r, g, b, a;
			r.f = in_rgba[4 * ((y + startY) * width + x) + 0];
			g.f = in_rgba[4 * ((y + startY) * width + x) + 1];
			b.f = in_rgba[4 * ((y + startY) * width + x) + 2];
			a.f = in_rgba[4 * ((y + startY) * width + x) + 3];

			FP16 hr, hg, hb, ha;
			hr = float_to_half_full(r);
			hg = float_to_half_full(g);
			hb = float_to_half_full(b);
			ha = float_to_half_full(a);

			// Assume increasing Y
			buf[4 * y * width + 3 * width + x] = hr.u;
			buf[4 * y * width + 2 * width + x] = hg.u;
			buf[4 * y * width + 1 * width + x] = hb.u;
			buf[4 * y * width + 0 * width + x] = ha.u;
		}
	}

	int bound = mz_compressBound(buf.size() * sizeof(unsigned short));

	std::vector<unsigned char> block(
			mz_compressBound(buf.size() * sizeof(unsigned short)));
	unsigned long long outSize = block.size();

	aly::CompressZipEXR(&block.at(0), outSize,
			reinterpret_cast<const unsigned char *>(&buf.at(0)),
			buf.size() * sizeof(unsigned short));

	// 4 byte: scan line
	// 4 byte: data size
	// ~     : pixel data(compressed)
	std::vector<unsigned char> header(8);
	unsigned int dataLen = outSize;// truncate
	memcpy(&header.at(0), &startY, sizeof(int));
	memcpy(&header.at(4), &dataLen, sizeof(unsigned int));

	data.insert(data.end(), header.begin(), header.end());
	data.insert(data.end(), block.begin(), block.begin() + dataLen);

	offsets[i] = offset;
	offset += dataLen + 8;// 8 = sizeof(blockHeader)
}

fwrite(&offsets.at(0), 1, sizeof(unsigned long long) * numBlocks, fp);

fwrite(&data.at(0), 1, data.size(), fp);

fclose(fp);

return 0; // OK
}
#endif

size_t SaveMultiChannelEXRToMemory(const EXRImage *exrImage,
	unsigned char **memory_out, const char **err) {
if (exrImage == NULL || memory_out == NULL) {
	if (err) {
		(*err) = "Invalid argument.";
	}
	return -1;
}
std::vector<unsigned char> memory;

// Header
{
	const char header[] = { 0x76, 0x2f, 0x31, 0x01 };
	memory.insert(memory.end(), header, header + 4);
}

// Version, scanline.
{
	const char marker[] = { 2, 0, 0, 0 };
	memory.insert(memory.end(), marker, marker + 4);
}

int numScanlineBlocks = 16; // 1 for no compress & ZIPS, 16 for ZIP compression.

// Write attributes.
{
	std::vector<unsigned char> data;

	std::vector<ChannelInfo> channels;
	for (int c = 0; c < exrImage->num_channels; c++) {
		ChannelInfo info;
		info.pLinear = 0;
		info.pixelType = exrImage->requested_pixel_types[c];
		info.xSampling = 1;
		info.ySampling = 1;
		info.name = std::string(exrImage->channel_names[c]);
		channels.push_back(info);
	}

	WriteChannelInfo(data, channels);
	WriteAttributeToMemory(memory, "channels", "chlist", &data.at(0),
			(int)data.size()); // +1 = null
}
{
	int compressionType = 3; // ZIP compression
	if (IsBigEndian()) {
		swap4(reinterpret_cast<unsigned int *>(&compressionType));
	}
	WriteAttributeToMemory(memory, "compression", "compression",
			reinterpret_cast<const unsigned char *>(&compressionType), 1);
}

{
	int data[4] = { 0, 0, exrImage->width - 1, exrImage->height - 1 };
	if (IsBigEndian()) {
		swap4(reinterpret_cast<unsigned int *>(&data[0]));
		swap4(reinterpret_cast<unsigned int *>(&data[1]));
		swap4(reinterpret_cast<unsigned int *>(&data[2]));
		swap4(reinterpret_cast<unsigned int *>(&data[3]));
	}
	WriteAttributeToMemory(memory, "dataWindow", "box2i",
			reinterpret_cast<const unsigned char *>(data), sizeof(int) * 4);
	WriteAttributeToMemory(memory, "displayWindow", "box2i",
			reinterpret_cast<const unsigned char *>(data), sizeof(int) * 4);
}

{
	unsigned char lineOrder = 0; // increasingY
	WriteAttributeToMemory(memory, "lineOrder", "lineOrder", &lineOrder, 1);
}

{
	float aspectRatio = 1.0f;
	if (IsBigEndian()) {
		swap4(reinterpret_cast<unsigned int *>(&aspectRatio));
	}
	WriteAttributeToMemory(memory, "pixelAspectRatio", "float",
			reinterpret_cast<const unsigned char *>(&aspectRatio),
			sizeof(float));
}

{
	float center[2] = { 0.0f, 0.0f };
	if (IsBigEndian()) {
		swap4(reinterpret_cast<unsigned int *>(&center[0]));
		swap4(reinterpret_cast<unsigned int *>(&center[1]));
	}
	WriteAttributeToMemory(memory, "screenWindowCenter", "v2f",
			reinterpret_cast<const unsigned char *>(center), 2 * sizeof(float));
}

{
	float w = (float) exrImage->width;
	if (IsBigEndian()) {
		swap4(reinterpret_cast<unsigned int *>(&w));
	}
	WriteAttributeToMemory(memory, "screenWindowWidth", "float",
			reinterpret_cast<const unsigned char *>(&w), sizeof(float));
}

{ // end of header
	unsigned char e = 0;
	memory.push_back(e);
}

int numBlocks = exrImage->height / numScanlineBlocks;
if (numBlocks * numScanlineBlocks < exrImage->height) {
	numBlocks++;
}

std::vector<long long> offsets(numBlocks);

size_t headerSize = memory.size();
long long offset = headerSize + numBlocks * sizeof(long long); // sizeof(header) + sizeof(offsetTable)

std::vector<unsigned char> data;

bool isBigEndian = IsBigEndian();

std::vector<std::vector<unsigned char> > dataList(numBlocks);
std::vector<size_t> channelOffsetList(exrImage->num_channels);

int pixelDataSize = 0;
size_t channelOffset = 0;
for (int c = 0; c < exrImage->num_channels; c++) {
	channelOffsetList[c] = channelOffset;
	if (exrImage->requested_pixel_types[c] == TINYEXR_PIXELTYPE_HALF) {
		pixelDataSize += sizeof(unsigned short);
		channelOffset += sizeof(unsigned short);
	} else if (exrImage->requested_pixel_types[c] == TINYEXR_PIXELTYPE_FLOAT) {
		pixelDataSize += sizeof(float);
		channelOffset += sizeof(float);
	} else if (exrImage->requested_pixel_types[c] == TINYEXR_PIXELTYPE_UINT) {
		pixelDataSize += sizeof(unsigned int);
		channelOffset += sizeof(unsigned int);
	} else {
		assert(0);
	}
}

#ifdef _OPENMP
#pragma omp parallel for
#endif
for (int i = 0; i < numBlocks; i++) {
	int startY = numScanlineBlocks * i;
	int endY = (std::min)(numScanlineBlocks * (i + 1), exrImage->height);
	int h = endY - startY;

	std::vector<unsigned char> buf(exrImage->width * h * pixelDataSize);

	for (int c = 0; c < exrImage->num_channels; c++) {
		if (exrImage->pixel_types[c] == TINYEXR_PIXELTYPE_HALF) {

			if (exrImage->requested_pixel_types[c] == TINYEXR_PIXELTYPE_FLOAT) {
				for (int y = 0; y < h; y++) {
					for (int x = 0; x < exrImage->width; x++) {
						FP16 h16;
						h16.u =
								reinterpret_cast<unsigned short **>(exrImage->images)[c][(y
										+ startY) * exrImage->width + x];

						FP32 f32 = half_to_float(h16);

						if (isBigEndian) {
							swap4(reinterpret_cast<unsigned int *>(&f32.f));
						}

						// Assume increasing Y
						float *linePtr = reinterpret_cast<float *>(&buf.at(
								pixelDataSize * y * exrImage->width
										+ channelOffsetList[c]
												* exrImage->width));
						linePtr[x] = f32.f;
					}
				}
			} else if (exrImage->requested_pixel_types[c] ==
			TINYEXR_PIXELTYPE_HALF) {
				for (int y = 0; y < h; y++) {
					for (int x = 0; x < exrImage->width; x++) {
						unsigned short val =
								reinterpret_cast<unsigned short **>(exrImage->images)[c][(y
										+ startY) * exrImage->width + x];

						if (isBigEndian) {
							swap2(&val);
						}

						// Assume increasing Y
						unsigned short *linePtr =
								reinterpret_cast<unsigned short *>(&buf.at(
										pixelDataSize * y * exrImage->width
												+ channelOffsetList[c]
														* exrImage->width));
						linePtr[x] = val;
					}
				}
			} else {
				assert(0);
			}

		} else if (exrImage->pixel_types[c] == TINYEXR_PIXELTYPE_FLOAT) {

			if (exrImage->requested_pixel_types[c] == TINYEXR_PIXELTYPE_HALF) {
				for (int y = 0; y < h; y++) {
					for (int x = 0; x < exrImage->width; x++) {
						FP32 f32;
						f32.f =
								reinterpret_cast<float **>(exrImage->images)[c][(y
										+ startY) * exrImage->width + x];

						FP16 h16;
						h16 = float_to_half_full(f32);

						if (isBigEndian) {
							swap2(reinterpret_cast<unsigned short *>(&h16.u));
						}

						// Assume increasing Y
						unsigned short *linePtr =
								reinterpret_cast<unsigned short *>(&buf.at(
										pixelDataSize * y * exrImage->width
												+ channelOffsetList[c]
														* exrImage->width));
						linePtr[x] = h16.u;
					}
				}
			} else if (exrImage->requested_pixel_types[c] ==
			TINYEXR_PIXELTYPE_FLOAT) {
				for (int y = 0; y < h; y++) {
					for (int x = 0; x < exrImage->width; x++) {
						float val =
								reinterpret_cast<float **>(exrImage->images)[c][(y
										+ startY) * exrImage->width + x];

						if (isBigEndian) {
							swap4(reinterpret_cast<unsigned int *>(&val));
						}

						// Assume increasing Y
						float *linePtr = reinterpret_cast<float *>(&buf.at(
								pixelDataSize * y * exrImage->width
										+ channelOffsetList[c]
												* exrImage->width));
						linePtr[x] = val;
					}
				}
			} else {
				assert(0);
			}
		} else if (exrImage->pixel_types[c] == TINYEXR_PIXELTYPE_UINT) {

			for (int y = 0; y < h; y++) {
				for (int x = 0; x < exrImage->width; x++) {
					unsigned int val =
							reinterpret_cast<unsigned int **>(exrImage->images)[c][(y
									+ startY) * exrImage->width + x];

					if (isBigEndian) {
						swap4(&val);
					}

					// Assume increasing Y
					unsigned int *linePtr =
							reinterpret_cast<unsigned int *>(&buf.at(
									pixelDataSize * y * exrImage->width
											+ channelOffsetList[c]
													* exrImage->width));
					linePtr[x] = val;
				}
			}
		}
	}

	std::vector<unsigned char> block(mz_compressBound(::mz_ulong(buf.size())));
	unsigned long long outSize = (unsigned long long)block.size();

	aly::CompressZipEXR(&block.at(0), outSize,
			reinterpret_cast<const unsigned char *>(&buf.at(0)), (unsigned long)buf.size());

	// 4 byte: scan line
	// 4 byte: data size
	// ~     : pixel data(compressed)
	std::vector<unsigned char> header(8);
	unsigned int dataLen = (unsigned int)outSize; // truncate
	memcpy(&header.at(0), &startY, sizeof(int));
	memcpy(&header.at(4), &dataLen, sizeof(unsigned int));

	if (IsBigEndian()) {
		swap4(reinterpret_cast<unsigned int *>(&header.at(0)));
		swap4(reinterpret_cast<unsigned int *>(&header.at(4)));
	}

	dataList[i].insert(dataList[i].end(), header.begin(), header.end());
	dataList[i].insert(dataList[i].end(), block.begin(),
			block.begin() + dataLen);

	// data.insert(data.end(), header.begin(), header.end());
	// data.insert(data.end(), block.begin(), block.begin() + dataLen);

	// offsets[i] = offset;
	// if (IsBigEndian()) {
	//  swap8(reinterpret_cast<unsigned long long*>(&offsets[i]));
	//}
	// offset += dataLen + 8; // 8 = sizeof(blockHeader)
} // omp parallel

for (int i = 0; i < numBlocks; i++) {

	data.insert(data.end(), dataList[i].begin(), dataList[i].end());

	offsets[i] = offset;
	if (IsBigEndian()) {
		swap8(reinterpret_cast<unsigned long long *>(&offsets[i]));
	}
	offset += dataList[i].size();
}

{
	memory.insert(memory.end(),
			reinterpret_cast<unsigned char *>(&offsets.at(0)),
			reinterpret_cast<unsigned char *>(&offsets.at(0))
					+ sizeof(unsigned long long) * numBlocks);
}

{
	memory.insert(memory.end(), data.begin(), data.end());
}

assert(memory.size() > 0);

(*memory_out) = (unsigned char *) malloc(memory.size());
memcpy((*memory_out), &memory.at(0), memory.size());

return memory.size(); // OK
}

int SaveMultiChannelEXRToFile(const EXRImage *exrImage, const char *filename,
	const char **err) {
if (exrImage == NULL || filename == NULL) {
	if (err) {
		(*err) = "Invalid argument.";
	}
	return -1;
}

FILE *fp = fopen(filename, "wb");
if (!fp) {
	if (err) {
		(*err) = "Cannot write a file.";
	}
	return -1;
}

unsigned char *mem = NULL;
size_t mem_size = SaveMultiChannelEXRToMemory(exrImage, &mem, err);

if ((mem_size > 0) && mem) {

	fwrite(mem, 1, mem_size, fp);
}
free(mem);

fclose(fp);

return 0; // OK
}

int LoadDeepEXR(DeepImage *deepImage, const char *filename, const char **err) {
if (deepImage == NULL) {
	if (err) {
		(*err) = "Invalid argument.";
	}
	return -1;
}

FILE *fp = fopen(filename, "rb");
if (!fp) {
	if (err) {
		(*err) = "Cannot read file.";
	}
	return -1;
}

size_t filesize;
// Compute size
fseek(fp, 0, SEEK_END);
filesize = ftell(fp);
fseek(fp, 0, SEEK_SET);

if (filesize == 0) {
	fclose(fp);
	if (err) {
		(*err) = "File size is zero.";
	}
	return -1;
}

std::vector<char> buf(filesize); // @todo { use mmap }
{
	size_t ret;
	ret = fread(&buf[0], 1, filesize, fp);
	assert(ret == filesize);
	(void) ret;
}
fclose(fp);

const char *head = &buf[0];
const char *marker = &buf[0];

// Header check.
{
	const char header[] = { 0x76, 0x2f, 0x31, 0x01 };

	if (memcmp(marker, header, 4) != 0) {
		if (err) {
			(*err) = "Header mismatch.";
		}
		return -3;
	}
	marker += 4;
}

// Version, scanline.
{
	// ver 2.0, scanline, deep bit on(0x800)
	// must be [2, 0, 0, 0]
	if (marker[0] != 2 || marker[1] != 8 || marker[2] != 0 || marker[3] != 0) {
		if (err) {
			(*err) = "Unsupported version or scanline.";
		}
		return -4;
	}

	marker += 4;
}

int dx = -1;
int dy = -1;
int dw = -1;
int dh = -1;
int numScanlineBlocks = 1; // 16 for ZIP compression.
int compressionType = -1;
int numChannels = -1;
std::vector<ChannelInfo> channels;

// Read attributes
for (;;) {
	std::string attrName;
	std::string attrType;
	std::vector<unsigned char> data;
	const char *marker_next = ReadAttribute(attrName, attrType, data, marker);
	if (marker_next == NULL) {
		marker++; // skip '\0'
		break;
	}

	if (attrName.compare("compression") == 0) {
		// must be 0:No compression, 1: RLE, 2: ZIPs or 3: ZIP
		if (data[0] > 3) {
			if (err) {
				(*err) = "Unsupported compression type.";
			}
			return -5;
		}

		compressionType = data[0];

		if (compressionType == 3) { // ZIP
			numScanlineBlocks = 16;
		}

	} else if (attrName.compare("channels") == 0) {

		// name: zero-terminated string, from 1 to 255 bytes long
		// pixel type: int, possible values are: UINT = 0 HALF = 1 FLOAT = 2
		// pLinear: unsigned char, possible values are 0 and 1
		// reserved: three chars, should be zero
		// xSampling: int
		// ySampling: int

		ReadChannelInfo(channels, data);

		numChannels = (int)channels.size();

		if (numChannels < 1) {
			if (err) {
				(*err) = "Invalid channels format.";
			}
			return -6;
		}

	} else if (attrName.compare("dataWindow") == 0) {
		memcpy(&dx, &data.at(0), sizeof(int));
		memcpy(&dy, &data.at(4), sizeof(int));
		memcpy(&dw, &data.at(8), sizeof(int));
		memcpy(&dh, &data.at(12), sizeof(int));
		if (IsBigEndian()) {
			swap4(reinterpret_cast<unsigned int *>(&dx));
			swap4(reinterpret_cast<unsigned int *>(&dy));
			swap4(reinterpret_cast<unsigned int *>(&dw));
			swap4(reinterpret_cast<unsigned int *>(&dh));
		}

	} else if (attrName.compare("displayWindow") == 0) {
		int x;
		int y;
		int w;
		int h;
		memcpy(&x, &data.at(0), sizeof(int));
		memcpy(&y, &data.at(4), sizeof(int));
		memcpy(&w, &data.at(8), sizeof(int));
		memcpy(&h, &data.at(12), sizeof(int));
		if (IsBigEndian()) {
			swap4(reinterpret_cast<unsigned int *>(&x));
			swap4(reinterpret_cast<unsigned int *>(&y));
			swap4(reinterpret_cast<unsigned int *>(&w));
			swap4(reinterpret_cast<unsigned int *>(&h));
		}
	}

	marker = marker_next;
}

assert(dx >= 0);
assert(dy >= 0);
assert(dw >= 0);
assert(dh >= 0);
assert(numChannels >= 1);

int dataWidth = dw - dx + 1;
int dataHeight = dh - dy + 1;

std::vector<float> image(dataWidth * dataHeight * 4); // 4 = RGBA

// Read offset tables.
int numBlocks = dataHeight / numScanlineBlocks;
if (numBlocks * numScanlineBlocks < dataHeight) {
	numBlocks++;
}

std::vector<long long> offsets(numBlocks);

for (int y = 0; y < numBlocks; y++) {
	long long offset;
	memcpy(&offset, marker, sizeof(long long));
	if (IsBigEndian()) {
		swap8(reinterpret_cast<unsigned long long *>(&offset));
	}
	marker += sizeof(long long); // = 8
	offsets[y] = offset;
}

if (compressionType != 0 && compressionType != 2 && compressionType != 3) {
	if (err) {
		(*err) = "Unsupported format.";
	}
	return -10;
}

deepImage->image = (float ***) malloc(sizeof(float **) * numChannels);
for (int c = 0; c < numChannels; c++) {
	deepImage->image[c] = (float **) malloc(sizeof(float *) * dataHeight);
	for (int y = 0; y < dataHeight; y++) {
	}
}

deepImage->offset_table = (int **) malloc(sizeof(int *) * dataHeight);
for (int y = 0; y < dataHeight; y++) {
	deepImage->offset_table[y] = (int *) malloc(sizeof(int) * dataWidth);
}

for (int y = 0; y < numBlocks; y++) {
	const unsigned char *dataPtr = reinterpret_cast<const unsigned char *>(head
			+ offsets[y]);

	// int: y coordinate
	// int64: packed size of pixel offset table
	// int64: packed size of sample data
	// int64: unpacked size of sample data
	// compressed pixel offset table
	// compressed sample data
	int lineNo;
	long long packedOffsetTableSize;
	long long packedSampleDataSize;
	long long unpackedSampleDataSize;
	memcpy(&lineNo, dataPtr, sizeof(int));
	memcpy(&packedOffsetTableSize, dataPtr + 4, sizeof(long long));
	memcpy(&packedSampleDataSize, dataPtr + 12, sizeof(long long));
	memcpy(&unpackedSampleDataSize, dataPtr + 20, sizeof(long long));

	if (IsBigEndian()) {
		swap4(reinterpret_cast<unsigned int *>(&lineNo));
		swap8(reinterpret_cast<unsigned long long *>(&packedOffsetTableSize));
		swap8(reinterpret_cast<unsigned long long *>(&packedSampleDataSize));
		swap8(reinterpret_cast<unsigned long long *>(&unpackedSampleDataSize));
	}

	std::vector<int> pixelOffsetTable(dataWidth);

	// decode pixel offset table.
	{
		unsigned long dstLen = (unsigned long )(pixelOffsetTable.size() * sizeof(int));
		aly::DecompressZipEXR(
				reinterpret_cast<unsigned char *>(&pixelOffsetTable.at(0)),
				dstLen, dataPtr + 28, (unsigned long)packedOffsetTableSize);

		assert(dstLen == pixelOffsetTable.size() * sizeof(int));
		for (int i = 0; i < dataWidth; i++) {
			deepImage->offset_table[y][i] = pixelOffsetTable[i];
		}
	}

	std::vector<unsigned char> sampleData(unpackedSampleDataSize);

	// decode sample data.
	{
		unsigned long dstLen = (unsigned long)unpackedSampleDataSize;
		aly::DecompressZipEXR(reinterpret_cast<unsigned char *>(&sampleData.at(0)),
				dstLen, dataPtr + 28 + packedOffsetTableSize,
			(unsigned long)packedSampleDataSize);
		assert(dstLen == (unsigned long )unpackedSampleDataSize);
	}

	// decode sample
	int sampleSize = -1;
	std::vector<int> channelOffsetList(numChannels);
	{
		int channelOffset = 0;
		for (int i = 0; i < numChannels; i++) {
			channelOffsetList[i] = channelOffset;
			if (channels[i].pixelType == TINYEXR_PIXELTYPE_UINT) { // UINT
				channelOffset += 4;
			} else if (channels[i].pixelType == TINYEXR_PIXELTYPE_HALF) { // half
				channelOffset += 2;
			} else if (channels[i].pixelType == TINYEXR_PIXELTYPE_FLOAT) { // float
				channelOffset += 4;
			} else {
				assert(0);
			}
		}
		sampleSize = channelOffset;
	}
	assert(sampleSize >= 2);

	assert(
			(size_t )(pixelOffsetTable[dataWidth - 1] * sampleSize)
					== sampleData.size());
	int samplesPerLine = (int)(sampleData.size() / sampleSize);

	//
	// Alloc memory
	//

	//
	// pixel data is stored as image[channels][pixel_samples]
	//
	{
		unsigned long long dataOffset = 0;
		for (int c = 0; c < numChannels; c++) {

			deepImage->image[c][y] = (float *) malloc(
					sizeof(float) * samplesPerLine);

			if (channels[c].pixelType == 0) { // UINT
				for (int x = 0; x < samplesPerLine; x++) {
					unsigned int ui =
							*reinterpret_cast<unsigned int *>(&sampleData.at(
									dataOffset + x * sizeof(int)));
					deepImage->image[c][y][x] = (float) ui; // @fixme
				}
				dataOffset += sizeof(unsigned int) * samplesPerLine;
			} else if (channels[c].pixelType == 1) { // half
				for (int x = 0; x < samplesPerLine; x++) {
					FP16 f16;
					f16.u = *reinterpret_cast<unsigned short *>(&sampleData.at(
							dataOffset + x * sizeof(short)));
					FP32 f32 = half_to_float(f16);
					deepImage->image[c][y][x] = f32.f;
				}
				dataOffset += sizeof(short) * samplesPerLine;
			} else { // float
				for (int x = 0; x < samplesPerLine; x++) {
					float f = *reinterpret_cast<float *>(&sampleData.at(
							dataOffset + x * sizeof(float)));
					deepImage->image[c][y][x] = f;
				}
				dataOffset += sizeof(float) * samplesPerLine;
			}
		}
	}

} // y

deepImage->width = dataWidth;
deepImage->height = dataHeight;

deepImage->channel_names = (const char **) malloc(
		sizeof(const char *) * numChannels);
for (int c = 0; c < numChannels; c++) {
#ifdef _WIN32
	deepImage->channel_names[c] = _strdup(channels[c].name.c_str());
#else
	deepImage->channel_names[c] = strdup(channels[c].name.c_str());
#endif
}
deepImage->num_channels = numChannels;

return 0; // OK
}

int SaveDeepEXR(const DeepImage *deepImage, const char *filename,
	const char **err) {
if (deepImage == NULL || filename == NULL) {
	if (err) {
		(*err) = "Invalid argument.";
	}
	return -1;
}

FILE *fp = fopen(filename, "rb");
if (!fp) {
	if (err) {
		(*err) = "Cannot write file.";
	}
	return -1;
}

// Write header check.
{
	const char header[] = { 0x76, 0x2f, 0x31, 0x01 };
	size_t n = fwrite(header, 1, 4, fp);
	if (n != 4) {
		if (err) {
			(*err) = "Header write failed.";
		}
		fclose(fp);
		return -3;
	}
}

// Version, scanline.
{
	// ver 2.0, scanline, deep bit on(0x800)
	const char data[] = { 2, 8, 0, 0 };
	size_t n = fwrite(data, 1, 4, fp);
	if (n != 4) {
		if (err) {
			(*err) = "Flag write failed.";
		}
		fclose(fp);
		return -3;
	}
}

// Write attributes.
{
	int data = 2; // ZIPS
	WriteAttribute(fp, "compression", "compression",
			reinterpret_cast<const unsigned char *>(&data), sizeof(int));
}

{
	int data[4] = { 0, 0, deepImage->width - 1, deepImage->height - 1 };
	WriteAttribute(fp, "dataWindow", "box2i",
			reinterpret_cast<const unsigned char *>(data), sizeof(int) * 4);
	WriteAttribute(fp, "displayWindow", "box2i",
			reinterpret_cast<const unsigned char *>(data), sizeof(int) * 4);
}

int numScanlineBlocks = 1;
// Write offset tables.
int numBlocks = deepImage->height / numScanlineBlocks;
if (numBlocks * numScanlineBlocks < deepImage->height) {
	numBlocks++;
}

#if 0 // @todo
std::vector<long long> offsets(numBlocks);

//std::vector<int> pixelOffsetTable(dataWidth);

// compress pixel offset table.
{
	unsigned long dstLen = pixelOffsetTable.size() * sizeof(int);
	Compresses(reinterpret_cast<unsigned char *>(&pixelOffsetTable.at(0)),
			dstLen, dataPtr + 28, packedOffsetTableSize);

	assert(dstLen == pixelOffsetTable.size() * sizeof(int));
	//      int ret =
	//          mz_uncompress(reinterpret_cast<unsigned char
	//          *>(&pixelOffsetTable.at(0)), &dstLen, dataPtr + 28,
	//          packedOffsetTableSize);
	//      printf("ret = %d, dstLen = %d\n", ret, (int)dstLen);
	//
	for (int i = 0; i < dataWidth; i++) {
		// printf("offt[%d] = %d\n", i, pixelOffsetTable[i]);
		deepImage->offset_table[y][i] = pixelOffsetTable[i];
	}
}

for (int y = 0; y < numBlocks; y++) {
	//long long offset = *(reinterpret_cast<const long long *>(marker));
	// printf("offset[%d] = %lld\n", y, offset);
	//marker += sizeof(long long); // = 8
	offsets[y] = offset;
}

// Write offset table.
fwrite(&offsets.at(0), sizeof(long long), numBlocks, fp);

for (int y = 0; y < numBlocks; y++) {
	const unsigned char *dataPtr =
	reinterpret_cast<const unsigned char *>(head + offsets[y]);

	// int: y coordinate
	// int64: packed size of pixel offset table
	// int64: packed size of sample data
	// int64: unpacked size of sample data
	// compressed pixel offset table
	// compressed sample data
	int lineNo = *reinterpret_cast<const int *>(dataPtr);
	long long packedOffsetTableSize =
	*reinterpret_cast<const long long *>(dataPtr + 4);
	long long packedSampleDataSize =
	*reinterpret_cast<const long long *>(dataPtr + 12);
	long long unpackedSampleDataSize =
	*reinterpret_cast<const long long *>(dataPtr + 20);
	// printf("line: %d, %lld/%lld/%lld\n", lineNo, packedOffsetTableSize,
	// packedSampleDataSize, unpackedSampleDataSize);

	int endLineNo = (std::min)(lineNo + numScanlineBlocks, dataHeight);

	int numLines = endLineNo - lineNo;
	// printf("numLines: %d\n", numLines);

	std::vector<int> pixelOffsetTable(dataWidth);

	// decode pixel offset table.
	{
		unsigned long dstLen = pixelOffsetTable.size() * sizeof(int);
		aly::DecompressZipEXR(reinterpret_cast<unsigned char *>(&pixelOffsetTable.at(0)),
				dstLen, dataPtr + 28, packedOffsetTableSize);

		assert(dstLen == pixelOffsetTable.size() * sizeof(int));
		//      int ret =
		//          mz_uncompress(reinterpret_cast<unsigned char
		//          *>(&pixelOffsetTable.at(0)), &dstLen, dataPtr + 28,
		//          packedOffsetTableSize);
		//      printf("ret = %d, dstLen = %d\n", ret, (int)dstLen);
		//
		for (int i = 0; i < dataWidth; i++) {
			// printf("offt[%d] = %d\n", i, pixelOffsetTable[i]);
			deepImage->offset_table[y][i] = pixelOffsetTable[i];
		}
	}

	std::vector<unsigned char> sampleData(unpackedSampleDataSize);

	// decode sample data.
	{
		unsigned long dstLen = unpackedSampleDataSize;
		// printf("dstLen = %d\n", dstLen);
		// printf("srcLen = %d\n", packedSampleDataSize);
		aly::DecompressZipEXR(reinterpret_cast<unsigned char *>(&sampleData.at(0)),
				dstLen, dataPtr + 28 + packedOffsetTableSize,
				packedSampleDataSize);
		assert(dstLen == unpackedSampleDataSize);
	}

	// decode sample
	int sampleSize = -1;
	std::vector<int> channelOffsetList(numChannels);
	{
		int channelOffset = 0;
		for (int i = 0; i < numChannels; i++) {
			// printf("offt[%d] = %d\n", i, channelOffset);
			channelOffsetList[i] = channelOffset;
			if (channels[i].pixelType == 0) { // UINT
				channelOffset += 4;
			} else if (channels[i].pixelType == 1) { // half
				channelOffset += 2;
			} else if (channels[i].pixelType == 2) { // float
				channelOffset += 4;
			} else {
				assert(0);
			}
		}
		sampleSize = channelOffset;
	}
	assert(sampleSize >= 2);

	assert(pixelOffsetTable[dataWidth - 1] * sampleSize == sampleData.size());
	int samplesPerLine = sampleData.size() / sampleSize;

	//
	// Alloc memory
	//

	//
	// pixel data is stored as image[channels][pixel_samples]
	//
	{
		unsigned long long dataOffset = 0;
		for (int c = 0; c < numChannels; c++) {

			deepImage->image[c][y] =
			(float *)malloc(sizeof(float) * samplesPerLine);

			// unsigned int channelOffset = channelOffsetList[c];
			// unsigned int i = channelOffset;
			// printf("channel = %d. name = %s. ty = %d\n", c,
			// channels[c].name.c_str(), channels[c].pixelType);

			// printf("dataOffset = %d\n", (int)dataOffset);

			if (channels[c].pixelType == 0) { // UINT
				for (int x = 0; x < samplesPerLine; x++) {
					unsigned int ui = *reinterpret_cast<unsigned int *>(
							&sampleData.at(dataOffset + x * sizeof(int)));
					deepImage->image[c][y][x] = (float)ui; // @fixme
				}
				dataOffset += sizeof(unsigned int) * samplesPerLine;
			} else if (channels[c].pixelType == 1) { // half
				for (int x = 0; x < samplesPerLine; x++) {
					FP16 f16;
					f16.u = *reinterpret_cast<unsigned short *>(
							&sampleData.at(dataOffset + x * sizeof(short)));
					FP32 f32 = half_to_float(f16);
					deepImage->image[c][y][x] = f32.f;
					// printf("c[%d]  f(half) = %f (0x%08x)\n", c, f32.f, f16.u);
				}
				dataOffset += sizeof(short) * samplesPerLine;
			} else { // float
				for (int x = 0; x < samplesPerLine; x++) {
					float f = *reinterpret_cast<float *>(
							&sampleData.at(dataOffset + x * sizeof(float)));
					// printf("  f = %f(0x%08x)\n", f, *((unsigned int *)&f));
					deepImage->image[c][y][x] = f;
				}
				dataOffset += sizeof(float) * samplesPerLine;
			}
		}
		// printf("total: %d\n", dataOffset);
	}

} // y
#endif
fclose(fp);

return 0; // OK
}

void InitEXRImage(EXRImage *exrImage) {
if (exrImage == NULL) {
	return;
}

exrImage->num_channels = 0;
exrImage->channel_names = NULL;
exrImage->images = NULL;
exrImage->pixel_types = NULL;
exrImage->requested_pixel_types = NULL;
}

int FreeEXRImage(EXRImage *exrImage) {

if (exrImage == NULL) {
	return -1; // Err
}

for (int i = 0; i < exrImage->num_channels; i++) {

	if (exrImage->channel_names && exrImage->channel_names[i]) {
		free((char *) exrImage->channel_names[i]); // remove const
	}

	if (exrImage->images && exrImage->images[i]) {
		free(exrImage->images[i]);
	}
}

if (exrImage->channel_names) {
	free(exrImage->channel_names);
}

if (exrImage->images) {
	free(exrImage->images);
}

if (exrImage->pixel_types) {
	free(exrImage->pixel_types);
}

if (exrImage->requested_pixel_types) {
	free(exrImage->requested_pixel_types);
}

return 0;
}

int ParseMultiChannelEXRHeaderFromFile(EXRImage *exrImage, const char *filename,
	const char **err) {
if (exrImage == NULL) {
	if (err) {
		(*err) = "Invalid argument.";
	}
	return -1;
}

FILE *fp = fopen(filename, "rb");
if (!fp) {
	if (err) {
		(*err) = "Cannot read file.";
	}
	return -1;
}

size_t filesize;
// Compute size
fseek(fp, 0, SEEK_END);
filesize = ftell(fp);
fseek(fp, 0, SEEK_SET);

std::vector<unsigned char> buf(filesize); // @todo { use mmap }
{
	size_t ret;
	ret = fread(&buf[0], 1, filesize, fp);
	assert(ret == filesize);
	fclose(fp);
	(void) ret;
}

return ParseMultiChannelEXRHeaderFromMemory(exrImage, &buf.at(0), err);
}

int ParseMultiChannelEXRHeaderFromMemory(EXRImage *exrImage,
	const unsigned char *memory, const char **err) {
if (exrImage == NULL || memory == NULL) {
	if (err) {
		(*err) = "Invalid argument.";
	}
	return -1;
}

const char *buf = reinterpret_cast<const char *>(memory);

const char *marker = &buf[0];

// Header check.
{
	const char header[] = { 0x76, 0x2f, 0x31, 0x01 };

	if (memcmp(marker, header, 4) != 0) {
		if (err) {
			(*err) = "Header mismatch.";
		}
		return -3;
	}
	marker += 4;
}

// Version, scanline.
{
	// must be [2, 0, 0, 0]
	if (marker[0] != 2 || marker[1] != 0 || marker[2] != 0 || marker[3] != 0) {
		if (err) {
			(*err) = "Unsupported version or scanline.";
		}
		return -4;
	}

	marker += 4;
}

int dx = -1;
int dy = -1;
int dw = -1;
int dh = -1;
int numChannels = -1;
unsigned char lineOrder = 0; // 0 -> increasing y; 1 -> decreasing
std::vector<ChannelInfo> channels;

// Read attributes
for (;;) {
	std::string attrName;
	std::string attrType;
	std::vector<unsigned char> data;
	const char *marker_next = ReadAttribute(attrName, attrType, data, marker);
	if (marker_next == NULL) {
		marker++; // skip '\0'
		break;
	}

	if (attrName.compare("compression") == 0) {
		// must be 0:No compression, 1: RLE, 2: ZIPs, 3: ZIP or 4: PIZ
		if (data[0] > 4) {
			if (err) {
				(*err) = "Unsupported compression type.";
			}
			return -5;
		}

	} else if (attrName.compare("channels") == 0) {

		// name: zero-terminated string, from 1 to 255 bytes long
		// pixel type: int, possible values are: UINT = 0 HALF = 1 FLOAT = 2
		// pLinear: unsigned char, possible values are 0 and 1
		// reserved: three chars, should be zero
		// xSampling: int
		// ySampling: int

		ReadChannelInfo(channels, data);

		numChannels =(int) channels.size();

		if (numChannels < 1) {
			if (err) {
				(*err) = "Invalid channels format.";
			}
			return -6;
		}

	} else if (attrName.compare("dataWindow") == 0) {
		memcpy(&dx, &data.at(0), sizeof(int));
		memcpy(&dy, &data.at(4), sizeof(int));
		memcpy(&dw, &data.at(8), sizeof(int));
		memcpy(&dh, &data.at(12), sizeof(int));
		if (IsBigEndian()) {
			swap4(reinterpret_cast<unsigned int *>(&dx));
			swap4(reinterpret_cast<unsigned int *>(&dy));
			swap4(reinterpret_cast<unsigned int *>(&dw));
			swap4(reinterpret_cast<unsigned int *>(&dh));
		}
	} else if (attrName.compare("displayWindow") == 0) {
		int x, y, w, h;
		memcpy(&x, &data.at(0), sizeof(int));
		memcpy(&y, &data.at(4), sizeof(int));
		memcpy(&w, &data.at(8), sizeof(int));
		memcpy(&h, &data.at(12), sizeof(int));
		if (IsBigEndian()) {
			swap4(reinterpret_cast<unsigned int *>(&x));
			swap4(reinterpret_cast<unsigned int *>(&y));
			swap4(reinterpret_cast<unsigned int *>(&w));
			swap4(reinterpret_cast<unsigned int *>(&h));
		}
	} else if (attrName.compare("lineOrder") == 0) {
		memcpy(&lineOrder, &data.at(0), sizeof(lineOrder));
	}

	marker = marker_next;
}

assert(dx >= 0);
assert(dy >= 0);
assert(dw >= 0);
assert(dh >= 0);
assert(numChannels >= 1);

int dataWidth = dw - dx + 1;
int dataHeight = dh - dy + 1;

{
	exrImage->channel_names = (const char **) malloc(
			sizeof(const char *) * numChannels);
	for (int c = 0; c < numChannels; c++) {
#ifdef _WIN32
		exrImage->channel_names[c] = _strdup(channels[c].name.c_str());
#else
		exrImage->channel_names[c] = strdup(channels[c].name.c_str());
#endif
	}
	exrImage->num_channels = numChannels;

	exrImage->width = dataWidth;
	exrImage->height = dataHeight;

	exrImage->pixel_types = (int *) malloc(sizeof(int) * numChannels);
	for (int c = 0; c < numChannels; c++) {
		exrImage->pixel_types[c] = channels[c].pixelType;
	}

	// Initially fill with values of `pixel-types`
	exrImage->requested_pixel_types = (int *) malloc(sizeof(int) * numChannels);
	for (int c = 0; c < numChannels; c++) {
		exrImage->requested_pixel_types[c] = channels[c].pixelType;
	}
}

return 0; // OK
}
