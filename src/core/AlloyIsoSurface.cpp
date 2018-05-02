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
#include <AlloyIsoSurface.h>
#include <stdint.h>
#include <iostream>
#include <set>
#include <map>
#include <algorithm>
using namespace std;
namespace aly {
const int3 IsoSurface::AXIS_OFFSET[3] = { int3(1, 0, 0), int3(0, 1, 0), int3(0,
		0, 1) };

// ----------------------------------------------------------------------------

const int3 IsoSurface::EDGE_NODE_OFFSETS[3][4] = { { int3(0), int3(0, 0, 1),
		int3(0, 1, 0), int3(0, 1, 1) }, { int3(0), int3(1, 0, 0), int3(0, 0, 1),
		int3(1, 0, 1) },
		{ int3(0), int3(0, 1, 0), int3(1, 0, 0), int3(1, 1, 0) }, };

const uint32_t IsoSurface::ENCODED_EDGE_NODE_OFFSETS[12] = { 0x00000000,
		0x00100000, 0x00000400, 0x00100400, 0x00000000, 0x00000001, 0x00100000,
		0x00100001, 0x00000000, 0x00000400, 0x00000001, 0x00000401, };

const uint32_t IsoSurface::ENCODED_EDGE_OFFSETS[12] = { 0x00000000, 0x00100000,
		0x00000400, 0x00100400, 0x40000000, 0x40100000, 0x40000001, 0x40100001,
		0x80000000, 0x80000400, 0x80000001, 0x80000401, };

const float IsoSurface::edgeDirection[12][3] = { { 1.0f, 0.0f, 0.0f }, { 0.0f,
		1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 1.0f,
		0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f,
		-1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f,
		1.0f }, { 0.0f, 0.0f, 1.0f } };

/* Lists the positions, relative to vertex 0, of each of the 8 vertices of a cube. */
const int IsoSurface::vertexOffset[8][3] = { { 0, 0, 0 }, { 1, 0, 0 },
		{ 1, 1, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 1, 0, 1 }, { 1, 1, 1 }, { 0, 1,
				1 } };

/* Lists the index of the endpoint vertices for each of the 12 edges of the cube. */
const int IsoSurface::edgeConnection[12][2] = { { 0, 1 }, { 1, 2 }, { 2, 3 }, {
		3, 0 }, { 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 4 }, { 0, 4 }, { 1, 5 }, { 2,
		6 }, { 3, 7 } };

/* Lists the index of the endpoint vertices for each of the 6 edges of the tetrahedron. */
const int IsoSurface::tetrahedronEdgeConnection[6][2] = { { 0, 1 }, { 1, 2 }, {
		2, 0 }, { 0, 3 }, { 1, 3 }, { 2, 3 } };

/* Lists the index of verticies from a cube that made up each of the six tetrahedrons within the cube. */
const int IsoSurface::tetrahedronsInACube[6][4] = { { 0, 5, 1, 6 },
		{ 0, 1, 2, 6 }, { 0, 2, 3, 6 }, { 0, 3, 7, 6 }, { 0, 7, 4, 6 }, { 0, 4,
				5, 6 } };

const int IsoSurface::tetrahedronTriangles[16][7] = { { -1, -1, -1, -1, -1, -1,
		-1 }, { 0, 3, 2, -1, -1, -1, -1 }, { 0, 1, 4, -1, -1, -1, -1 }, { 1, 4,
		2, 2, 4, 3, -1 }, { 1, 2, 5, -1, -1, -1, -1 }, { 0, 3, 5, 0, 5, 1, -1 },
		{ 0, 2, 5, 0, 5, 4, -1 }, { 5, 4, 3, -1, -1, -1, -1 }, { 3, 4, 5, -1,
				-1, -1, -1 }, { 4, 5, 0, 5, 2, 0, -1 },
		{ 1, 5, 0, 5, 3, 0, -1 }, { 5, 2, 1, -1, -1, -1, -1 }, { 3, 4, 2, 2, 4,
				1, -1 }, { 4, 1, 0, -1, -1, -1, -1 },
		{ 2, 3, 0, -1, -1, -1, -1 }, { -1, -1, -1, -1, -1, -1, -1 } };

const int IsoSurface::cubeEdgeFlags[256] = { 0x000, 0x109, 0x203, 0x30a, 0x406,
		0x50f, 0x605, 0x70c, 0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09,
		0xf00, 0x190, 0x099, 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c, 0x99c,
		0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90, 0x230, 0x339, 0x033,
		0x13a, 0x636, 0x73f, 0x435, 0x53c, 0xa3c, 0xb35, 0x83f, 0x936, 0xe3a,
		0xf33, 0xc39, 0xd30, 0x3a0, 0x2a9, 0x1a3, 0x0aa, 0x7a6, 0x6af, 0x5a5,
		0x4ac, 0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0, 0x460,
		0x569, 0x663, 0x76a, 0x066, 0x16f, 0x265, 0x36c, 0xc6c, 0xd65, 0xe6f,
		0xf66, 0x86a, 0x963, 0xa69, 0xb60, 0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6,
		0x0ff, 0x3f5, 0x2fc, 0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9,
		0xaf0, 0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x055, 0x15c, 0xe5c,
		0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950, 0x7c0, 0x6c9, 0x5c3,
		0x4ca, 0x3c6, 0x2cf, 0x1c5, 0x0cc, 0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca,
		0xac3, 0x9c9, 0x8c0, 0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5,
		0xfcc, 0x0cc, 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0, 0x950,
		0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c, 0x15c, 0x055, 0x35f,
		0x256, 0x55a, 0x453, 0x759, 0x650, 0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6,
		0xfff, 0xcf5, 0xdfc, 0x2fc, 0x3f5, 0x0ff, 0x1f6, 0x6fa, 0x7f3, 0x4f9,
		0x5f0, 0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c, 0x36c,
		0x265, 0x16f, 0x066, 0x76a, 0x663, 0x569, 0x460, 0xca0, 0xda9, 0xea3,
		0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac, 0x4ac, 0x5a5, 0x6af, 0x7a6, 0x0aa,
		0x1a3, 0x2a9, 0x3a0, 0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35,
		0xa3c, 0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x033, 0x339, 0x230, 0xe90,
		0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c, 0x69c, 0x795, 0x49f,
		0x596, 0x29a, 0x393, 0x099, 0x190, 0xf00, 0xe09, 0xd03, 0xc0a, 0xb06,
		0xa0f, 0x905, 0x80c, 0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109,
		0x000 };

const int IsoSurface::cubeEdgeFlagsCC618[256] = { 0x0, 0x109, 0x203, 0x30a,
		0x406, 0x50f, 0x605, 0x70c, 0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03,
		0xe09, 0xf00, 0x190, 0x99, 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
		0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90, 0x230, 0x339,
		0x33, 0x13a, 0x636, 0x73f, 0x435, 0x53c, 0xa3c, 0xb35, 0x83f, 0x936,
		0xe3a, 0xf33, 0xc39, 0xd30, 0x3a0, 0x2a9, 0x1a3, 0xaa, 0x7a6, 0x6af,
		0x5a5, 0x4ac, 0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
		0x460, 0x569, 0x663, 0x76a, 0x66, 0x16f, 0x265, 0x36c, 0xc6c, 0xd65,
		0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60, 0x5f0, 0x4f9, 0x7f3, 0x6fa,
		0x1f6, 0xff, 0x3f5, 0x2fc, 0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3,
		0xbf9, 0xaf0, 0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55, 0x15c,
		0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950, 0x7c0, 0x6c9,
		0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc, 0xfcc, 0xec5, 0xdcf, 0xcc6,
		0xbca, 0xac3, 0x9c9, 0x8c0, 0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf,
		0xec5, 0xfcc, 0xcc, 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
		0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c, 0x15c, 0x55,
		0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650, 0xaf0, 0xbf9, 0x8f3, 0x9fa,
		0xef6, 0xfff, 0xcf5, 0xdfc, 0x2fc, 0x3f5, 0xff, 0x1f6, 0x6fa, 0x7f3,
		0x4f9, 0x5f0, 0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
		0x36c, 0x265, 0x16f, 0x66, 0x76a, 0x663, 0x569, 0x460, 0xca0, 0xda9,
		0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac, 0x4ac, 0x5a5, 0x6af, 0x7a6,
		0xaa, 0x1a3, 0x2a9, 0x3a0, 0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f,
		0xb35, 0xa3c, 0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33, 0x339, 0x230,
		0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c, 0x69c, 0x795,
		0x49f, 0x596, 0x29a, 0x393, 0x99, 0x190, 0xf00, 0xe09, 0xd03, 0xc0a,
		0xb06, 0xa0f, 0x905, 0x80c, 0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203,
		0x109, 0x0 };

const int IsoSurface::cubeEdgeFlagsCC626[256] = { 0x0, 0x109, 0x203, 0x30a,
		0x406, 0x50f, 0x605, 0x70c, 0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03,
		0xe09, 0xf00, 0x190, 0x99, 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
		0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90, 0x230, 0x339,
		0x33, 0x13a, 0x636, 0x73f, 0x435, 0x53c, 0xa3c, 0xb35, 0x83f, 0x936,
		0xe3a, 0xf33, 0xc39, 0xd30, 0x3a0, 0x2a9, 0x1a3, 0xaa, 0x7a6, 0x6af,
		0x5a5, 0x4ac, 0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
		0x460, 0x569, 0x663, 0x76a, 0x66, 0x16f, 0x265, 0x36c, 0xc6c, 0xd65,
		0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60, 0x5f0, 0x4f9, 0x7f3, 0x6fa,
		0x1f6, 0xff, 0x3f5, 0x2fc, 0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3,
		0xbf9, 0xaf0, 0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55, 0x15c,
		0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950, 0x7c0, 0x6c9,
		0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc, 0xfcc, 0xec5, 0xdcf, 0xcc6,
		0xbca, 0xac3, 0x9c9, 0x8c0, 0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf,
		0xec5, 0xfcc, 0xcc, 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
		0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c, 0x15c, 0x55,
		0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650, 0xaf0, 0xbf9, 0x8f3, 0x9fa,
		0xef6, 0xfff, 0xcf5, 0xdfc, 0x2fc, 0x3f5, 0xff, 0x1f6, 0x6fa, 0x7f3,
		0x4f9, 0x5f0, 0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
		0x36c, 0x265, 0x16f, 0x66, 0x76a, 0x663, 0x569, 0x460, 0xca0, 0xda9,
		0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac, 0x4ac, 0x5a5, 0x6af, 0x7a6,
		0xaa, 0x1a3, 0x2a9, 0x3a0, 0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f,
		0xb35, 0xa3c, 0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33, 0x339, 0x230,
		0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c, 0x69c, 0x795,
		0x49f, 0x596, 0x29a, 0x393, 0x99, 0x190, 0xf00, 0xe09, 0xd03, 0xc0a,
		0xb06, 0xa0f, 0x905, 0x80c, 0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203,
		0x109, 0x0 };
const int IsoSurface::triangleConnectionTable[4096] = { -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 8, 0, 3, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, 9, 1, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, 3, 8, 9, 9, 1, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, 10, 2, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 1,
		10, 10, 8, 0, 10, 2, 3, 3, 8, 10, -1, -1, -1, -1, 0, 9, 10, 10, 2, 0,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 8, 9, 10, 10, 2, 8, 2, 3, 8, -1,
		-1, -1, -1, -1, -1, -1, 11, 3, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, 2, 11, 8, 8, 0, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, 3, 0, 9, 9, 11, 3, 9, 1, 2, 2, 11, 9, -1, -1, -1, -1, 11, 8, 9, 9,
		1, 11, 1, 2, 11, -1, -1, -1, -1, -1, -1, -1, 1, 10, 11, 11, 3, 1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, 10, 11, 8, 8, 0, 10, 0, 1, 10, -1,
		-1, -1, -1, -1, -1, -1, 9, 10, 11, 11, 3, 9, 3, 0, 9, -1, -1, -1, -1,
		-1, -1, -1, 11, 8, 9, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		7, 4, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 3, 7, 7,
		4, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 4, 9, 1, 1, 7, 4, 1, 0, 8,
		8, 7, 1, -1, -1, -1, -1, 1, 3, 7, 7, 4, 1, 4, 9, 1, -1, -1, -1, -1, -1,
		-1, -1, 10, 7, 4, 10, 4, 1, 1, 4, 8, 1, 8, 2, 2, 8, 7, 2, 4, 0, 1, 4, 1,
		10, 4, 10, 7, 7, 10, 2, 7, 2, 3, -1, 2, 0, 8, 2, 8, 7, 2, 7, 10, 10, 7,
		4, 10, 4, 9, -1, 10, 2, 3, 4, 9, 10, 3, 7, 4, 4, 10, 3, -1, -1, -1, -1,
		8, 3, 2, 2, 4, 8, 2, 11, 7, 7, 4, 2, -1, -1, -1, -1, 4, 0, 2, 2, 11, 4,
		11, 7, 4, -1, -1, -1, -1, -1, -1, -1, 3, 0, 8, 11, 7, 4, 11, 4, 9, 11,
		9, 2, 1, 2, 9, -1, 11, 1, 2, 7, 9, 1, 7, 4, 9, 1, 11, 7, -1, -1, -1, -1,
		10, 11, 7, 10, 7, 4, 10, 4, 1, 1, 4, 8, 1, 8, 3, -1, 7, 4, 0, 10, 11, 7,
		0, 1, 10, 7, 0, 10, -1, -1, -1, -1, 8, 3, 0, 10, 11, 7, 4, 9, 10, 10, 7,
		4, -1, -1, -1, -1, 9, 10, 11, 11, 4, 9, 11, 7, 4, -1, -1, -1, -1, -1,
		-1, -1, 9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 8,
		4, 5, 5, 3, 8, 5, 9, 0, 0, 3, 5, -1, -1, -1, -1, 4, 5, 1, 1, 0, 4, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, 5, 1, 3, 3, 8, 5, 8, 4, 5, -1, -1,
		-1, -1, -1, -1, -1, 1, 9, 4, 4, 2, 1, 4, 5, 10, 10, 2, 4, -1, -1, -1,
		-1, 0, 1, 9, 8, 4, 5, 8, 5, 10, 8, 10, 3, 2, 3, 10, -1, 2, 0, 4, 4, 5,
		2, 5, 10, 2, -1, -1, -1, -1, -1, -1, -1, 5, 8, 4, 10, 3, 8, 10, 2, 3, 8,
		5, 10, -1, -1, -1, -1, 11, 4, 5, 11, 5, 2, 2, 5, 9, 2, 9, 3, 3, 9, 4, 3,
		11, 8, 4, 11, 4, 5, 11, 5, 2, 2, 5, 9, 2, 9, 0, -1, 5, 1, 2, 5, 2, 11,
		5, 11, 4, 4, 11, 3, 4, 3, 0, -1, 4, 5, 1, 11, 8, 4, 1, 2, 11, 4, 1, 11,
		-1, -1, -1, -1, 3, 1, 9, 3, 9, 4, 3, 4, 11, 11, 4, 5, 11, 5, 10, -1, 9,
		0, 1, 11, 8, 4, 5, 10, 11, 11, 4, 5, -1, -1, -1, -1, 11, 3, 0, 5, 10,
		11, 0, 4, 5, 5, 11, 0, -1, -1, -1, -1, 10, 11, 8, 8, 5, 10, 8, 4, 5, -1,
		-1, -1, -1, -1, -1, -1, 5, 9, 8, 8, 7, 5, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, 3, 7, 5, 5, 9, 3, 9, 0, 3, -1, -1, -1, -1, -1, -1, -1, 7, 5,
		1, 1, 0, 7, 0, 8, 7, -1, -1, -1, -1, -1, -1, -1, 5, 1, 3, 3, 7, 5, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, 7, 5, 10, 7, 10, 2, 7, 2, 8, 8, 2,
		1, 8, 1, 9, -1, 1, 9, 0, 7, 5, 10, 2, 3, 7, 7, 10, 2, -1, -1, -1, -1,
		10, 2, 0, 7, 5, 10, 0, 8, 7, 10, 0, 7, -1, -1, -1, -1, 3, 7, 5, 5, 2, 3,
		5, 10, 2, -1, -1, -1, -1, -1, -1, -1, 9, 8, 3, 9, 3, 2, 9, 2, 5, 5, 2,
		11, 5, 11, 7, -1, 2, 11, 7, 9, 0, 2, 7, 5, 9, 9, 2, 7, -1, -1, -1, -1,
		3, 0, 8, 5, 1, 2, 11, 7, 5, 5, 2, 11, -1, -1, -1, -1, 7, 5, 1, 1, 11, 7,
		1, 2, 11, -1, -1, -1, -1, -1, -1, -1, 7, 5, 10, 10, 11, 7, 1, 9, 8, 8,
		3, 1, -1, -1, -1, -1, 1, 9, 0, 7, 5, 10, 10, 11, 7, -1, -1, -1, -1, -1,
		-1, -1, 8, 3, 0, 10, 11, 7, 7, 5, 10, -1, -1, -1, -1, -1, -1, -1, 10,
		11, 7, 7, 5, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 6, 10, 5, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 8, 5, 6, 8, 6, 3, 3, 6,
		10, 3, 10, 0, 0, 10, 5, 0, 9, 5, 6, 6, 0, 9, 6, 10, 1, 1, 0, 6, -1, -1,
		-1, -1, 8, 9, 5, 8, 5, 6, 8, 6, 3, 3, 6, 10, 3, 10, 1, -1, 2, 1, 5, 5,
		6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 6, 2, 3, 6, 3, 8, 6, 8, 5,
		5, 8, 0, 5, 0, 1, -1, 6, 2, 0, 0, 9, 6, 9, 5, 6, -1, -1, -1, -1, -1, -1,
		-1, 3, 8, 9, 6, 2, 3, 9, 5, 6, 3, 9, 6, -1, -1, -1, -1, 2, 10, 5, 5, 3,
		2, 5, 6, 11, 11, 3, 5, -1, -1, -1, -1, 0, 2, 10, 0, 10, 5, 0, 5, 8, 8,
		5, 6, 8, 6, 11, -1, 1, 2, 10, 9, 5, 6, 9, 6, 11, 9, 11, 0, 3, 0, 11, -1,
		10, 1, 2, 8, 9, 5, 6, 11, 8, 8, 5, 6, -1, -1, -1, -1, 3, 1, 5, 5, 6, 3,
		6, 11, 3, -1, -1, -1, -1, -1, -1, -1, 8, 0, 1, 6, 11, 8, 1, 5, 6, 6, 8,
		1, -1, -1, -1, -1, 9, 3, 0, 5, 11, 3, 5, 6, 11, 3, 9, 5, -1, -1, -1, -1,
		11, 8, 9, 9, 6, 11, 9, 5, 6, -1, -1, -1, -1, -1, -1, -1, 7, 6, 10, 10,
		8, 7, 10, 5, 4, 4, 8, 10, -1, -1, -1, -1, 3, 7, 6, 3, 6, 10, 3, 10, 0,
		0, 10, 5, 0, 5, 4, -1, 9, 5, 4, 0, 8, 7, 0, 7, 6, 0, 6, 1, 10, 1, 6, -1,
		5, 4, 9, 3, 7, 6, 10, 1, 3, 3, 6, 10, -1, -1, -1, -1, 1, 5, 4, 1, 4, 8,
		1, 8, 2, 2, 8, 7, 2, 7, 6, -1, 1, 5, 4, 4, 0, 1, 7, 6, 2, 2, 3, 7, -1,
		-1, -1, -1, 4, 9, 5, 2, 0, 8, 7, 6, 2, 2, 8, 7, -1, -1, -1, -1, 5, 4, 9,
		3, 7, 6, 6, 2, 3, -1, -1, -1, -1, -1, -1, -1, 6, 11, 7, 5, 4, 8, 5, 8,
		3, 5, 3, 10, 2, 10, 3, -1, 6, 11, 7, 0, 2, 10, 5, 4, 0, 0, 10, 5, -1,
		-1, -1, -1, 2, 10, 1, 9, 5, 4, 11, 7, 6, 0, 8, 3, -1, -1, -1, -1, 2, 10,
		1, 9, 5, 4, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, 7, 6, 11, 1, 5, 4, 8,
		3, 1, 1, 4, 8, -1, -1, -1, -1, 7, 6, 11, 1, 5, 4, 4, 0, 1, -1, -1, -1,
		-1, -1, -1, -1, 0, 8, 3, 11, 7, 6, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1,
		9, 5, 4, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 9, 4, 6, 6,
		10, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 10, 9, 0, 10, 0, 3, 10,
		3, 6, 6, 3, 8, 6, 8, 4, -1, 0, 4, 6, 6, 10, 0, 10, 1, 0, -1, -1, -1, -1,
		-1, -1, -1, 6, 10, 1, 8, 4, 6, 1, 3, 8, 8, 6, 1, -1, -1, -1, -1, 4, 6,
		2, 2, 1, 4, 1, 9, 4, -1, -1, -1, -1, -1, -1, -1, 0, 1, 9, 6, 2, 3, 8, 4,
		6, 6, 3, 8, -1, -1, -1, -1, 2, 0, 4, 4, 6, 2, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, 4, 6, 2, 2, 8, 4, 2, 3, 8, -1, -1, -1, -1, -1, -1, -1,
		4, 6, 11, 4, 11, 3, 4, 3, 9, 9, 3, 2, 9, 2, 10, -1, 4, 6, 11, 11, 8, 4,
		2, 10, 9, 9, 0, 2, -1, -1, -1, -1, 2, 10, 1, 4, 6, 11, 3, 0, 4, 4, 11,
		3, -1, -1, -1, -1, 2, 10, 1, 4, 6, 11, 11, 8, 4, -1, -1, -1, -1, -1, -1,
		-1, 9, 4, 6, 3, 1, 9, 6, 11, 3, 9, 6, 3, -1, -1, -1, -1, 9, 0, 1, 11, 8,
		4, 4, 6, 11, -1, -1, -1, -1, -1, -1, -1, 0, 4, 6, 6, 3, 0, 6, 11, 3, -1,
		-1, -1, -1, -1, -1, -1, 11, 8, 4, 4, 6, 11, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, 10, 9, 8, 8, 7, 10, 7, 6, 10, -1, -1, -1, -1, -1, -1, -1, 0,
		3, 7, 10, 9, 0, 7, 6, 10, 0, 7, 10, -1, -1, -1, -1, 10, 7, 6, 1, 8, 7,
		1, 0, 8, 7, 10, 1, -1, -1, -1, -1, 1, 3, 7, 7, 10, 1, 7, 6, 10, -1, -1,
		-1, -1, -1, -1, -1, 8, 7, 6, 1, 9, 8, 6, 2, 1, 1, 8, 6, -1, -1, -1, -1,
		0, 1, 9, 6, 2, 3, 3, 7, 6, -1, -1, -1, -1, -1, -1, -1, 6, 2, 0, 0, 7, 6,
		0, 8, 7, -1, -1, -1, -1, -1, -1, -1, 3, 7, 6, 6, 2, 3, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, 11, 7, 6, 9, 8, 3, 2, 10, 9, 9, 3, 2, -1, -1,
		-1, -1, 6, 11, 7, 0, 2, 10, 10, 9, 0, -1, -1, -1, -1, -1, -1, -1, 6, 11,
		7, 8, 3, 0, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1, 1, 2, 10, 7, 6, 11,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 11, 7, 6, 9, 8, 3, 3, 1, 9, -1,
		-1, -1, -1, -1, -1, -1, 6, 11, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, 0, 8, 3, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 11, 6, 7,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 3, 11, 6, 6, 0, 3,
		6, 7, 8, 8, 0, 6, -1, -1, -1, -1, 9, 6, 7, 9, 7, 0, 0, 7, 11, 0, 11, 1,
		1, 11, 6, 1, 1, 3, 11, 1, 11, 6, 1, 6, 9, 9, 6, 7, 9, 7, 8, -1, 10, 6,
		7, 7, 1, 10, 7, 11, 2, 2, 1, 7, -1, -1, -1, -1, 2, 3, 11, 10, 6, 7, 10,
		7, 8, 10, 8, 1, 0, 1, 8, -1, 9, 10, 6, 9, 6, 7, 9, 7, 0, 0, 7, 11, 0,
		11, 2, -1, 11, 2, 3, 9, 10, 6, 7, 8, 9, 9, 6, 7, -1, -1, -1, -1, 6, 7,
		3, 3, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 2, 6, 6, 7, 0, 7,
		8, 0, -1, -1, -1, -1, -1, -1, -1, 7, 3, 0, 7, 0, 9, 7, 9, 6, 6, 9, 1, 6,
		1, 2, -1, 9, 1, 2, 7, 8, 9, 2, 6, 7, 7, 9, 2, -1, -1, -1, -1, 7, 3, 1,
		1, 10, 7, 10, 6, 7, -1, -1, -1, -1, -1, -1, -1, 0, 7, 8, 1, 6, 7, 1, 10,
		6, 7, 0, 1, -1, -1, -1, -1, 6, 7, 3, 9, 10, 6, 3, 0, 9, 6, 3, 9, -1, -1,
		-1, -1, 8, 9, 10, 10, 7, 8, 10, 6, 7, -1, -1, -1, -1, -1, -1, -1, 4, 8,
		11, 11, 6, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 6, 4, 0, 0, 3, 6,
		3, 11, 6, -1, -1, -1, -1, -1, -1, -1, 6, 4, 9, 6, 9, 1, 6, 1, 11, 11, 1,
		0, 11, 0, 8, -1, 11, 6, 4, 1, 3, 11, 4, 9, 1, 11, 4, 1, -1, -1, -1, -1,
		8, 11, 2, 8, 2, 1, 8, 1, 4, 4, 1, 10, 4, 10, 6, -1, 2, 3, 11, 4, 0, 1,
		10, 6, 4, 4, 1, 10, -1, -1, -1, -1, 6, 4, 9, 9, 10, 6, 0, 8, 11, 11, 2,
		0, -1, -1, -1, -1, 11, 2, 3, 9, 10, 6, 6, 4, 9, -1, -1, -1, -1, -1, -1,
		-1, 2, 6, 4, 4, 8, 2, 8, 3, 2, -1, -1, -1, -1, -1, -1, -1, 6, 4, 0, 0,
		2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 8, 3, 6, 4, 9, 1, 2, 6,
		6, 9, 1, -1, -1, -1, -1, 2, 6, 4, 4, 1, 2, 4, 9, 1, -1, -1, -1, -1, -1,
		-1, -1, 4, 8, 3, 10, 6, 4, 3, 1, 10, 10, 4, 3, -1, -1, -1, -1, 6, 4, 0,
		0, 10, 6, 0, 1, 10, -1, -1, -1, -1, -1, -1, -1, 0, 8, 3, 6, 4, 9, 9, 10,
		6, -1, -1, -1, -1, -1, -1, -1, 6, 4, 9, 9, 10, 6, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, 4, 7, 11, 11, 9, 4, 11, 6, 5, 5, 9, 11, -1, -1, -1,
		-1, 4, 7, 8, 9, 0, 3, 9, 3, 11, 9, 11, 5, 6, 5, 11, -1, 0, 4, 7, 0, 7,
		11, 0, 11, 1, 1, 11, 6, 1, 6, 5, -1, 7, 8, 4, 1, 3, 11, 6, 5, 1, 1, 11,
		6, -1, -1, -1, -1, 6, 5, 10, 11, 2, 1, 11, 1, 9, 11, 9, 7, 4, 7, 9, -1,
		1, 9, 0, 8, 4, 7, 10, 6, 5, 3, 11, 2, -1, -1, -1, -1, 6, 5, 10, 0, 4, 7,
		11, 2, 0, 0, 7, 11, -1, -1, -1, -1, 4, 7, 8, 3, 11, 2, 6, 5, 10, -1, -1,
		-1, -1, -1, -1, -1, 2, 6, 5, 2, 5, 9, 2, 9, 3, 3, 9, 4, 3, 4, 7, -1, 4,
		7, 8, 2, 6, 5, 9, 0, 2, 2, 5, 9, -1, -1, -1, -1, 7, 3, 0, 0, 4, 7, 1, 2,
		6, 6, 5, 1, -1, -1, -1, -1, 4, 7, 8, 2, 6, 5, 5, 1, 2, -1, -1, -1, -1,
		-1, -1, -1, 5, 10, 6, 3, 1, 9, 4, 7, 3, 3, 9, 4, -1, -1, -1, -1, 8, 4,
		7, 6, 5, 10, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1, 6, 5, 10, 0, 4, 7, 7,
		3, 0, -1, -1, -1, -1, -1, -1, -1, 8, 4, 7, 10, 6, 5, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, 9, 8, 11, 11, 6, 9, 6, 5, 9, -1, -1, -1, -1, -1, -1,
		-1, 9, 6, 5, 0, 11, 6, 0, 3, 11, 6, 9, 0, -1, -1, -1, -1, 11, 6, 5, 0,
		8, 11, 5, 1, 0, 0, 11, 5, -1, -1, -1, -1, 5, 1, 3, 3, 6, 5, 3, 11, 6,
		-1, -1, -1, -1, -1, -1, -1, 10, 6, 5, 8, 11, 2, 1, 9, 8, 8, 2, 1, -1,
		-1, -1, -1, 5, 10, 6, 11, 2, 3, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1, 10,
		6, 5, 8, 11, 2, 2, 0, 8, -1, -1, -1, -1, -1, -1, -1, 3, 11, 2, 5, 10, 6,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 3, 2, 6, 9, 8, 3, 6, 5, 9, 3, 6,
		9, -1, -1, -1, -1, 0, 2, 6, 6, 9, 0, 6, 5, 9, -1, -1, -1, -1, -1, -1,
		-1, 3, 0, 8, 5, 1, 2, 2, 6, 5, -1, -1, -1, -1, -1, -1, -1, 5, 1, 2, 2,
		6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 5, 10, 6, 3, 1, 9, 9, 8,
		3, -1, -1, -1, -1, -1, -1, -1, 0, 1, 9, 6, 5, 10, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, 5, 10, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 7,
		11, 10, 10, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 5, 7, 8, 5, 8,
		0, 5, 0, 10, 10, 0, 3, 10, 3, 11, -1, 11, 10, 1, 11, 1, 0, 11, 0, 7, 7,
		0, 9, 7, 9, 5, -1, 5, 7, 8, 8, 9, 5, 3, 11, 10, 10, 1, 3, -1, -1, -1,
		-1, 1, 5, 7, 7, 11, 1, 11, 2, 1, -1, -1, -1, -1, -1, -1, -1, 3, 11, 2,
		5, 7, 8, 0, 1, 5, 5, 8, 0, -1, -1, -1, -1, 0, 9, 5, 11, 2, 0, 5, 7, 11,
		11, 0, 5, -1, -1, -1, -1, 3, 11, 2, 5, 7, 8, 8, 9, 5, -1, -1, -1, -1,
		-1, -1, -1, 5, 7, 3, 3, 2, 5, 2, 10, 5, -1, -1, -1, -1, -1, -1, -1, 8,
		0, 2, 5, 7, 8, 2, 10, 5, 8, 2, 5, -1, -1, -1, -1, 1, 2, 10, 7, 3, 0, 9,
		5, 7, 7, 0, 9, -1, -1, -1, -1, 10, 1, 2, 8, 9, 5, 5, 7, 8, -1, -1, -1,
		-1, -1, -1, -1, 1, 5, 7, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, 1, 5, 7, 7, 0, 1, 7, 8, 0, -1, -1, -1, -1, -1, -1, -1, 5, 7, 3, 3,
		9, 5, 3, 0, 9, -1, -1, -1, -1, -1, -1, -1, 8, 9, 5, 5, 7, 8, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, 8, 11, 10, 10, 5, 8, 5, 4, 8, -1, -1, -1,
		-1, -1, -1, -1, 10, 5, 4, 3, 11, 10, 4, 0, 3, 3, 10, 4, -1, -1, -1, -1,
		9, 5, 4, 11, 10, 1, 0, 8, 11, 11, 1, 0, -1, -1, -1, -1, 9, 5, 4, 11, 10,
		1, 1, 3, 11, -1, -1, -1, -1, -1, -1, -1, 2, 1, 5, 8, 11, 2, 5, 4, 8, 2,
		5, 8, -1, -1, -1, -1, 2, 3, 11, 4, 0, 1, 1, 5, 4, -1, -1, -1, -1, -1,
		-1, -1, 4, 9, 5, 2, 0, 8, 8, 11, 2, -1, -1, -1, -1, -1, -1, -1, 4, 9, 5,
		2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 8, 5, 4, 3, 10, 5, 3,
		2, 10, 5, 8, 3, -1, -1, -1, -1, 4, 0, 2, 2, 5, 4, 2, 10, 5, -1, -1, -1,
		-1, -1, -1, -1, 4, 9, 5, 10, 1, 2, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1,
		2, 10, 1, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 3, 1, 5, 5,
		8, 3, 5, 4, 8, -1, -1, -1, -1, -1, -1, -1, 1, 5, 4, 4, 0, 1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, 3, 0, 8, 5, 4, 9, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, 11, 10, 9, 9, 4, 11, 4, 7, 11, -1, -1, -1, -1, -1, -1, -1, 8, 4, 7,
		10, 9, 0, 3, 11, 10, 10, 0, 3, -1, -1, -1, -1, 1, 0, 4, 11, 10, 1, 4, 7,
		11, 1, 4, 11, -1, -1, -1, -1, 7, 8, 4, 1, 3, 11, 11, 10, 1, -1, -1, -1,
		-1, -1, -1, -1, 11, 4, 7, 2, 9, 4, 2, 1, 9, 4, 11, 2, -1, -1, -1, -1, 7,
		8, 4, 9, 0, 1, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, 2, 0, 4, 4, 11, 2,
		4, 7, 11, -1, -1, -1, -1, -1, -1, -1, 4, 7, 8, 2, 3, 11, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, 9, 4, 7, 2, 10, 9, 7, 3, 2, 2, 9, 7, -1, -1, -1,
		-1, 8, 4, 7, 10, 9, 0, 0, 2, 10, -1, -1, -1, -1, -1, -1, -1, 1, 2, 10,
		7, 3, 0, 0, 4, 7, -1, -1, -1, -1, -1, -1, -1, 7, 8, 4, 1, 2, 10, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, 7, 3, 1, 1, 4, 7, 1, 9, 4, -1, -1, -1,
		-1, -1, -1, -1, 7, 8, 4, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, 7, 3, 0, 0, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 8, 4, 7,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 10, 9, 8, 8, 11, 10,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 11, 10, 9, 9, 3, 11, 9, 0, 3,
		-1, -1, -1, -1, -1, -1, -1, 8, 11, 10, 10, 0, 8, 10, 1, 0, -1, -1, -1,
		-1, -1, -1, -1, 11, 10, 1, 1, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, 9, 8, 11, 11, 1, 9, 11, 2, 1, -1, -1, -1, -1, -1, -1, -1, 11, 2, 3,
		9, 0, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 8, 11, 2, 2, 0, 8, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, 2, 3, 11, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, 10, 9, 8, 8, 2, 10, 8, 3, 2, -1, -1, -1, -1,
		-1, -1, -1, 10, 9, 0, 0, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		8, 3, 0, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, 2, 10, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 9, 8, 3, 3, 1, 9, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1 };

IsoSurface::IsoSurface() :
		isoLevel(0), rows(0), cols(0), slices(0), winding(Winding::Clockwise), backgroundValue(
				std::numeric_limits<float>::infinity()), skipHidden(true), triangleCount(
				0) {
}

IsoSurface::~IsoSurface() {
}
void IsoSurface::solveQuad(const float* data, const int& rows, const int& cols,
		const int& slices, const std::vector<int3>& indexList, Mesh& mesh,
		const float& isoLevel) {
	this->rows = rows;
	this->cols = cols;
	this->slices = slices;
	this->isoLevel = isoLevel;
	std::unordered_set<int3> activeVoxels;
	std::unordered_map<int4, EdgeInfo> activeEdges;
	std::unordered_map<int3, uint32_t> vertexIndices;
	findActiveVoxels(data, indexList, activeVoxels, activeEdges);
	generateVertexData(data, activeVoxels, activeEdges, vertexIndices, mesh);
	generateTriangles(activeEdges, vertexIndices, mesh);
}
void IsoSurface::solve(const float* data, const int& rows, const int& cols,
		const int& slices, const std::vector<int3>& indexList, Mesh& mesh,
		const MeshType& type, bool regularizeTest, const float& isoLevel) {
	mesh.clear();
	if (type == MeshType::Triangle) {
		solveTri(data, rows, cols, slices, indexList, mesh, isoLevel);
	} else {
		solveQuad(data, rows, cols, slices, indexList, mesh, isoLevel);
	}
	if (regularizeTest) {
		regularize(data, mesh);
	}
	mesh.updateBoundingBox();
}
void IsoSurface::solve(const Volume1f& data, Mesh& mesh, const MeshType& type,
		bool regularize, const float& isoLevel) {
	std::vector<int3> narrowBandList;
	backgroundValue=1E30f;
	for (int z = 0; z < data.slices; z++) {
		for (int y = 0; y < data.cols; y++) {
			for (int x = 0; x < data.rows; x++) {
				float val = data(x, y, z).x;
				if (std::abs(val) < 1.75f) {
					narrowBandList.push_back(int3(x, y, z));
				}
			}
		}
	}
	solve(data, narrowBandList, mesh, type, regularize, isoLevel);
}
void IsoSurface::solve(const Volume1f& data, const std::vector<int3>& indexList,
		Mesh& mesh, const MeshType& type, bool regularizeTest,
		const float& isoLevel) {
	mesh.clear();
	if (type == MeshType::Triangle) {
		solveTri(data.ptr(), data.rows, data.cols, data.slices, indexList, mesh,isoLevel);
	} else {
		solveQuad(data.ptr(), data.rows, data.cols, data.slices, indexList,mesh, isoLevel);
	}
	if (regularizeTest) {
		regularize(data.ptr(), mesh);
		//mesh.updateVertexNormals();
		//mesh.vertexNormals*=float3(-1.0f);
	}
	mesh.updateBoundingBox();
}
void IsoSurface::solve(const EndlessGridFloat& grid, Mesh& mesh,
		const MeshType& type, bool regularizeTest, const float& isoLevel) {
	mesh.clear();
	float oldBg = backgroundValue;
	backgroundValue = grid.getBackgroundValue();
	if (type == MeshType::Triangle) {
		solveTri(grid, mesh, isoLevel);

	} else {
		solveQuad(grid, mesh, isoLevel);
	}
	if (regularizeTest) {
		regularize(grid, mesh);
	}
	mesh.updateBoundingBox();
	backgroundValue = oldBg;
}
void IsoSurface::regularize(const EndlessGridFloat& grid, Mesh& mesh) {
	const int TRACE_ITERATIONS = 16;
	const int REGULARIZE_ITERATIONS = 3;
	const float TRACE_THRESHOLD = 1E-5f;
	std::vector<float3> tmpPoints(mesh.vertexLocations.size());
	std::vector<std::unordered_set<uint32_t>> vertNbrs;
	CreateVertexNeighborTable(mesh, vertNbrs);
	for (int c = 0; c < REGULARIZE_ITERATIONS; c++) {
#pragma omp parallel for
		for (int i = 0; i < (int) vertNbrs.size(); i++) {
			float3 pt(0.0f);
			int K=(int)vertNbrs[i].size();
			if(K>3){
				for (uint32_t nbr : vertNbrs[i]) {
					pt += mesh.vertexLocations[nbr];
				}
				pt /= (float) K;
			} else {
				pt = mesh.vertexLocations[i];
			}
			tmpPoints[i] = pt;
			mesh.vertexNormals[i] = GetNormal(grid, (int) round(pt.x),(int) round(pt.y), (int) round(pt.z));
		}
#pragma omp parallel for
		for (int i = 0; i < (int) mesh.vertexLocations.size(); i++) {
			float3 norm = mesh.vertexNormals[i];
			float3 pt = tmpPoints[i];
			bool converged = false;
			for (int n = 0; n < TRACE_ITERATIONS; n++) {
				float val = GetInterpolatedValue(grid, pt.x, pt.y, pt.z);
				pt -= 0.75f * clamp(val, -1.0f, 1.0f) * norm;
				if (std::abs(val) < TRACE_THRESHOLD) {
					converged = true;
					break;
				}
			}
			if (converged) {
				mesh.vertexLocations[i] = pt;
			}
			mesh.vertexNormals[i] = normalize(norm);
		}
	}
	const static float ANGLE_THRESH = std::acos(20.0f * ALY_PI / 180.0f);
	mesh.updateVertexNormals(true);
}
void IsoSurface::regularize(const float* data, Mesh& mesh) {
	const int TRACE_ITERATIONS = 16;
	const int REGULARIZE_ITERATIONS = 3;
	const float TRACE_THRESHOLD = 1E-5f;
	std::vector<float3> tmpPoints(mesh.vertexLocations.size());
	std::vector<std::unordered_set<uint32_t>> vertNbrs;
	CreateVertexNeighborTable(mesh, vertNbrs);
	for (int c = 0; c < REGULARIZE_ITERATIONS; c++) {
#pragma omp parallel for
		for (int i = 0; i < (int) vertNbrs.size(); i++) {
			float3 pt(0.0f);
			int K = (int)vertNbrs[i].size();
			if (K > 3) {
				for (uint32_t nbr : vertNbrs[i]) {
					pt += mesh.vertexLocations[nbr];
				}
				pt /= (float) K;
			} else {
				pt = mesh.vertexLocations[i];
			}
			tmpPoints[i] = pt;
			mesh.vertexNormals[i] = normalize(interpolateNormal(data, pt.x, pt.y, pt.z));
		}
#pragma omp parallel for
		for (int i = 0; i < (int) mesh.vertexLocations.size(); i++) {
			float3 norm = mesh.vertexNormals[i];
			float3 pt = tmpPoints[i];
			bool converged = false;
			for (int n = 0; n < TRACE_ITERATIONS; n++) {
				float val = interpolate(data, pt.x, pt.y, pt.z);
				pt -= 0.75f * aly::clamp(val, -1.0f, 1.0f) * norm;
				if (std::abs(val) < TRACE_THRESHOLD) {
					converged = true;
					break;
				}
			}
			if (converged) {
				mesh.vertexLocations[i] = pt;
			}
			mesh.vertexNormals[i] = normalize(norm);
		}
	}
}
void IsoSurface::solveTri(const float* vol, const int& rows, const int& cols,
		const int& slices, const std::vector<int3>& indexList, Mesh& mesh,
		const float& isoLevel) {
	this->rows = rows;
	this->cols = cols;
	this->slices = slices;
	this->isoLevel = isoLevel;
	size_t elements = indexList.size();
	std::vector<uint3> &indexes = mesh.triIndexes.data;
	std::vector<float3> &points = mesh.vertexLocations.data;
	std::vector<float3> &normals = mesh.vertexNormals.data;
	std::unordered_map<uint64_t, EdgeSplit3D> splits;
	std::vector<IsoTriangle> triangles;
	triangles.reserve(indexList.size() * 2);
	size_t vertexCount = 0;
	triangleCount = 0;
	for (size_t nn = 0; nn < elements; nn++) {
		int3 index = indexList[nn];
		if (index.x > 0 && index.y > 0 && index.z > 0 && index.x < rows - 1
				&& index.y < cols - 1 && index.z < slices - 1)
			triangulateUsingMarchingCubes(vol, splits, triangles, index.x,
					index.y, index.z, vertexCount);
	}
	indexes.resize(triangleCount);
	if (winding == Winding::Clockwise) {
		for (int k = 0; k < triangleCount; k++) {
			IsoTriangle* triPtr = &triangles[k];
			indexes[k] = uint3((uint32_t)triPtr->vertexIds[0], (uint32_t)triPtr->vertexIds[1],
				(uint32_t)triPtr->vertexIds[2]);
		}
	} else if (winding == Winding::CounterClockwise) {
		for (int k = 0; k < triangleCount; k++) {
			IsoTriangle* triPtr = &triangles[k];
			indexes[k] = uint3((uint32_t)triPtr->vertexIds[0], (uint32_t)triPtr->vertexIds[1],
				(uint32_t)triPtr->vertexIds[2]);
		}
	}
	const size_t splitCount = splits.size();
	points.resize(splitCount);
	normals.resize(splitCount);
	for (auto splitPtr = splits.begin(); splitPtr != splits.end(); ++splitPtr) {
		size_t index = (splitPtr->second).vertexId;
		aly::float3 pt = (splitPtr->second).point;
		aly::float3 norm = interpolateNormal(vol, pt.x, pt.y, pt.z);
		norm = norm / length(norm);
		normals[index] = norm;
		points[index] = pt;
	}
}
void IsoSurface::solveQuad(const EndlessGridFloat& grid, Mesh& mesh,
		const float& isoLevel) {
	auto leafs = grid.getLeafNodes();
	int dim = leafs.front()->dim;
	int bdim = dim + 1;
	this->rows = bdim;
	this->cols = bdim;
	this->slices = bdim;
	this->isoLevel = isoLevel;

	std::unordered_set<int3> activeVoxels;
	std::unordered_map<int4, EdgeInfo> activeEdges;
	std::unordered_map<int3, uint32_t> vertexIndices;
	findActiveVoxels(grid, leafs, activeVoxels, activeEdges);
	generateVertexData(grid, activeVoxels, activeEdges, vertexIndices, mesh);
	generateTriangles(activeEdges, vertexIndices, mesh);
}
void IsoSurface::solveTri(const EndlessGridFloat& grid, Mesh& mesh,
		const float& isoLevel) {
	this->isoLevel = isoLevel;
	std::vector<aly::float3> &points = mesh.vertexLocations.data;
	std::vector<uint3> &indexes = mesh.triIndexes.data;
	std::unordered_map<int4, EdgeSplit3D> splits;
	std::vector<IsoTriangle> triangles;
	size_t vertexCount = 0;
	triangleCount = 0;
	auto leafs = grid.getLeafNodes();
	int dim = leafs.front()->dim;
	int bdim = dim + 1;
	this->rows = bdim;
	this->cols = bdim;
	this->slices = bdim;
	std::vector<float> data(bdim * bdim * bdim);
	for (EndlessNodeFloat* leaf : leafs) {
		int dim = leaf->dim;
		int3 loc = leaf->location;
		for (int z = 0; z < bdim; z++) {
			for (int y = 0; y < bdim; y++) {
				for (int x = 0; x < bdim; x++) {
					float val;
					if (x >= dim || y >= dim || z >= dim) {
						val = grid.getLeafValue(loc.x + x, loc.y + y,
								loc.z + z);
					} else {
						val = leaf->data[x + y * dim + z * dim * dim];
					}
					data[x + y * bdim + z * bdim * bdim] = val;
				}
			}
		}
		for (int z = 0; z < dim; z++) {
			for (int y = 0; y < dim; y++) {
				for (int x = 0; x < dim; x++) {
					triangulateUsingMarchingCubes(data.data(), splits,
							triangles, x, y, z, loc.x, loc.y, loc.z,
							vertexCount);
				}
			}
		}
	}
	indexes.resize(triangleCount);
	if (winding == Winding::Clockwise) {
		for (int k = 0; k < triangleCount; k++) {
			IsoTriangle* triPtr = &triangles[k];
			indexes[k] = uint3((uint32_t)triPtr->vertexIds[0], (uint32_t)triPtr->vertexIds[1],
				(uint32_t)triPtr->vertexIds[2]);
		}
	} else if (winding == Winding::CounterClockwise) {
		for (int k = 0; k < triangleCount; k++) {
			IsoTriangle* triPtr = &triangles[k];
			indexes[k] = uint3((uint32_t)triPtr->vertexIds[0], (uint32_t)triPtr->vertexIds[1],
				(uint32_t)triPtr->vertexIds[2]);
		}
	}
	points.clear();
	const size_t splitCount = splits.size();
	points.resize(splitCount);
	for (auto splitPtr = splits.begin(); splitPtr != splits.end(); ++splitPtr) {
		size_t index = (splitPtr->second).vertexId;
		aly::float3 pt = (splitPtr->second).point;
		points[index] = pt;
	}
	mesh.updateVertexNormals(true);
}

size_t IsoSurface::getIndex(int i, int j, int k) {
	return k * (rows * (size_t) cols) + j * (size_t) rows + (size_t) i;
}

float IsoSurface::getValue(const float* vol, int i, int j, int k) {
	size_t in1 = getSafeIndex(i, j, k);
	float val = vol[in1];
	return val;
}

aly::float4 IsoSurface::getImageColor(const float4* image, int i, int j,
		int k) {
	size_t off = getSafeIndex(i, j, k);
	return image[off];
}
vector<int> IsoSurface::buildFaceNeighborTable(int vertexCount,
		const int* indexes, const int indexCount) {
	vector<int> faceTable(indexCount);
	int v1, v2, v3;
	int n1, n2, n3;
	int nfid, fid;
	vector<vector<int>> tmpTable(vertexCount);
	for (int i = 0; i < indexCount; i += 3) {
		v1 = indexes[i];
		v2 = indexes[i + 1];
		v3 = indexes[i + 2];
		fid = i / 3;
		tmpTable[v1].push_back(fid);
		tmpTable[v2].push_back(fid);
		tmpTable[v3].push_back(fid);
	}
	for (int i = 0; i < indexCount; i += 3) {
		v1 = indexes[i];
		v2 = indexes[i + 1];
		v3 = indexes[i + 2];
		fid = i / 3;
		vector<int>& nbrs1 = tmpTable[v1];
		vector<int>& nbrs2 = tmpTable[v2];
		vector<int>& nbrs3 = tmpTable[v3];
		faceTable[i] = -1;
		const size_t nbrs1Size = nbrs1.size();
		for (size_t j = 0; j < nbrs1Size; j++) {
			nfid = nbrs1[j];
			if (nfid == fid)
				continue;
			n1 = indexes[nfid * 3];
			n2 = indexes[nfid * 3 + 1];
			n3 = indexes[nfid * 3 + 2];
			if (n1 == v2 || n2 == v2 || n3 == v2) {
				faceTable[i] = nfid;
				break;
			}
		}
		faceTable[i + 1] = -1;
		const size_t nbrs2Size = nbrs2.size();
		for (size_t j = 0; j < nbrs2Size; j++) {
			nfid = nbrs2[j];
			if (nfid == fid)
				continue;
			n1 = indexes[nfid * 3];
			n2 = indexes[nfid * 3 + 1];
			n3 = indexes[nfid * 3 + 2];
			if (n1 == v3 || n2 == v3 || n3 == v3) {
				faceTable[i + 1] = nfid;
				break;
			}
		}
		faceTable[i + 2] = -1;
		const size_t nbrs3Size = nbrs3.size();
		for (size_t j = 0; j < nbrs3Size; j++) {
			nfid = nbrs3[j];
			if (nfid == fid)
				continue;
			n1 = indexes[nfid * 3];
			n2 = indexes[nfid * 3 + 1];
			n3 = indexes[nfid * 3 + 2];
			if (n1 == v1 || n2 == v1 || n3 == v1) {
				faceTable[i + 2] = nfid;
				break;
			}
		}
	}
	return faceTable;
}
// debugged, no problem need optimization, i.e. vector->stack
vector<vector<int>> IsoSurface::buildVertexNeighborTable(const int vertexCount,
		const int* indexes, const int indexCount, Winding direction) {
	int v1, v2, v3;
	vector<vector<int>> neighborTable(vertexCount);
	vector<vector<int>> tmpTable(vertexCount);

	if (direction == Winding::Clockwise) {
		for (int i = 0; i < indexCount; i += 3) {
			v1 = indexes[i];
			v2 = indexes[i + 1];
			v3 = indexes[i + 2];
			tmpTable[v1].push_back(v2);
			tmpTable[v1].push_back(v3);
			tmpTable[v2].push_back(v3);
			tmpTable[v2].push_back(v1);
			tmpTable[v3].push_back(v1);
			tmpTable[v3].push_back(v2);
		}
	} else if (direction == Winding::CounterClockwise) {
		for (int i = 0; i < indexCount; i += 3) {
			v1 = indexes[i];
			v2 = indexes[i + 1];
			v3 = indexes[i + 2];
			tmpTable[v1].push_back(v3);
			tmpTable[v1].push_back(v2);
			tmpTable[v2].push_back(v1);
			tmpTable[v2].push_back(v3);
			tmpTable[v3].push_back(v2);
			tmpTable[v3].push_back(v1);
		}
	}

	const int parallelism = 8;
	int stride = (vertexCount / parallelism) + 1;

	for (int n = 0; n < parallelism; ++n) {
		int K = std::min((n + 1) * stride, vertexCount);
		for (int i = n * stride; i < K; i++) {
			neighborTable[i].resize(tmpTable[i].size() / 2);

			vector<int> neighbors = tmpTable[i];
			int count = 0;
			if (neighbors.size() == 0)
				continue;
			neighbors.erase(neighbors.begin());
			int pivot;
			neighborTable[i][count++] = pivot = neighbors[0];
			neighbors.erase(neighbors.begin());
			while (neighbors.size() > 0) {
				bool found = false;
				for (unsigned int k = 0; k < neighbors.size(); k += 2) {
					if (neighbors[k] == pivot) {
						neighbors.erase(neighbors.begin() + k);
						neighborTable[i][count++] = pivot = neighbors[k];
						neighbors.erase(neighbors.begin() + k);
						found = true;
						break;
					}
				}
				if (!found) {
					neighbors.erase(neighbors.begin());
					neighborTable[i][count++] = pivot = neighbors[0];
					neighbors.erase(neighbors.begin());
				}
			}
		}
	}
	return neighborTable;
}
vector<vector<int>> IsoSurface::buildVertexNeighborTable(const int vertexCount,
		const int* indexes, const int indexCount) {
	int v1, v2, v3;
	vector<vector<int>> neighborTable(vertexCount);
	for (int i = 0; i < indexCount; i += 3) {
		v1 = indexes[i];
		v2 = indexes[i + 1];
		v3 = indexes[i + 2];
		if (v1 < v2) {
			neighborTable[v1].push_back(v2);
			neighborTable[v2].push_back(v1);
		}
		if (v2 < v3) {
			neighborTable[v2].push_back(v3);
			neighborTable[v3].push_back(v2);
		}
		if (v3 < v1) {
			neighborTable[v3].push_back(v1);
			neighborTable[v1].push_back(v3);
		}
	}
	return neighborTable;
}
void IsoSurface::generateVertexData(const float*data,
		const std::unordered_set<int3>& voxels, const std::unordered_map<int4, EdgeInfo>& edges,
		std::unordered_map<int3, uint32_t>& vertexIndices, Mesh& buffer) {
	Vector3f& vert = buffer.vertexLocations;
	Vector3f& norm = buffer.vertexNormals;
	float3 p[12];
	uint32_t idxCounter = 0;
	for (const auto& voxelID : voxels) {
		int idx = 0;
		for (int a = 0; a < 3; a++) {
			for (int i = 0; i < 4; i++) {
				int4 edgeID = int4(voxelID + EDGE_NODE_OFFSETS[a][i], a);
				const auto iter = edges.find(edgeID);
				if (iter != end(edges)) {
					const auto& info = iter->second;
					p[idx++] = info.point;
				}
			}
		}
		float3 nodePos(0.0f);
		for (int i = 0; i < idx; i++) {
			nodePos += p[i];
		}
		nodePos /= (float) idx;
		vertexIndices[voxelID] = idxCounter++;
		vert.push_back(nodePos);
		norm.push_back(
				interpolateNormal(data, nodePos.x, nodePos.y, nodePos.z));
	}
}
void IsoSurface::generateVertexData(
		const EndlessGridFloat& grid,
		const std::unordered_set<int3>& voxels,
		const std::unordered_map<int4, EdgeInfo>& edges,
		std::unordered_map<int3, uint32_t>& vertexIndices,
		Mesh& buffer) {
	Vector3f& vert = buffer.vertexLocations;
	Vector3f& norm = buffer.vertexNormals;
	float3 p[12];
	uint32_t idxCounter = 0;
	for (const auto& voxelID : voxels) {
		int idx = 0;
		for (int a = 0; a < 3; a++) {
			for (int i = 0; i < 4; i++) {
				int4 edgeID = int4(voxelID + EDGE_NODE_OFFSETS[a][i], a);
				const auto iter = edges.find(edgeID);
				if (iter != end(edges)) {
					const auto& info = iter->second;
					p[idx++] = info.point;
				}
			}
		}
		float3 nodePos(0.0f);
		for (int i = 0; i < idx; i++) {
			nodePos += p[i];
		}
		nodePos /= (float) idx;
		vertexIndices[voxelID] = idxCounter++;
		vert.push_back(nodePos);
		norm.push_back(
				GetInterpolatedNormal(grid, nodePos.x, nodePos.y, nodePos.z));
	}
}
void IsoSurface::generateTriangles(const std::unordered_map<int4, EdgeInfo>& edges,
		const std::unordered_map<int3, uint32_t>& vertexIndices, Mesh& mesh) {
	Vector4ui& quads = mesh.quadIndexes;
	for (const auto& pair : edges) {
		const int4& edge = pair.first;
		const EdgeInfo& info = pair.second;
		const int axis = edge.w;
		int3 pivot = edge.xyz();
		const int3 voxelIDs[4] =
				{ pivot - EDGE_NODE_OFFSETS[axis][0], pivot
						- EDGE_NODE_OFFSETS[axis][1], pivot
						- EDGE_NODE_OFFSETS[axis][2], pivot
						- EDGE_NODE_OFFSETS[axis][3] };
		// attempt to find the 4 voxels which share this edge
		int edgeVoxels[4];
		int numFoundVoxels = 0;
		for (int i = 0; i < 4; i++) {
			const auto iter = vertexIndices.find(voxelIDs[i]);
			if (iter != end(vertexIndices)) {
				edgeVoxels[numFoundVoxels++] = iter->second;
			}
		}
		// we can only generate a quad (or two triangles) if all 4 are found
		if (numFoundVoxels < 4) {
			continue;
		}
		uint4 quad;
		if (info.winding) {
			quad[0] = edgeVoxels[0];
			quad[1] = edgeVoxels[2];
			quad[2] = edgeVoxels[3];
			quad[3] = edgeVoxels[1];

		} else {

			quad[0] = edgeVoxels[0];
			quad[1] = edgeVoxels[1];
			quad[2] = edgeVoxels[3];
			quad[3] = edgeVoxels[2];
		}
		quads.push_back(quad);
	}
}
void IsoSurface::findActiveVoxels(const EndlessGridFloat& grid,
		const std::list<EndlessNodeFloat*>& leafs,
		std::unordered_set<int3>& activeVoxels, std::unordered_map<int4, EdgeInfo>& activeEdges) {
	int bdim = rows;
	std::vector<float> data(rows * cols * slices);
	for (EndlessNodeFloat* leaf : leafs) {
		int dim = leaf->dim;
		int3 loc = leaf->location;
		for (int z = 0; z < bdim; z++) {
			for (int y = 0; y < bdim; y++) {
				for (int x = 0; x < bdim; x++) {
					float val;
					if (x >= dim || y >= dim || z >= dim) {
						val = grid.getLeafValue(loc.x + x, loc.y + y,
								loc.z + z);
					} else {
						val = leaf->data[x + y * dim + z * dim * dim];
					}
					data[x + y * bdim + z * bdim * bdim] = val;
				}
			}
		}
		for (int z = 0; z < dim; z++) {
			for (int y = 0; y < dim; y++) {
				for (int x = 0; x < dim; x++) {
					float fValue1 = getValue(data.data(), x, y, z);
					if (fValue1 != backgroundValue) {
						int3 pivot = int3(x + loc.x, y + loc.y, z + loc.z);
						for (int a = 0; a < 3; a++) {
							int3 axis = AXIS_OFFSET[a];
							int3 nbr = int3(x, y, z) + axis;
							if (nbr.x >= 0 && nbr.y >= 0 && nbr.z >= 0
									&& nbr.x < rows && nbr.y < cols
									&& nbr.z < slices) {
								float fValue2 = getValue(data.data(), nbr.x,
										nbr.y, nbr.z);
								if (fValue2 != backgroundValue
										&& fValue1 * fValue2 < 0) {
									float3 crossing((float)pivot.x, (float)pivot.y, (float)pivot.z);
									double fDelta = fValue2 - fValue1;
									if (std::abs(fDelta) < 1E-3f) {
										crossing += float3(axis) * 0.5f;
									} else {
										crossing += float3(axis)
												* (float) ((isoLevel - fValue1)
														/ fDelta);
									}
									EdgeInfo info;
									info.point = crossing;
									if (winding == Winding::Clockwise) {
										info.winding = (fValue1 < 0.f);
									} else {
										info.winding = (fValue1 > 0.f);
									}
									activeEdges[int4(pivot.x, pivot.y, pivot.z,
											a)] = info;
									auto edgeNodes = EDGE_NODE_OFFSETS[a];
									for (int i = 0; i < 4; i++) {
										int3 id = pivot - edgeNodes[i];
										activeVoxels.insert(id);
									}
								}
							}
						}
					}
				}
			}
		}
	}
}
void IsoSurface::findActiveVoxels(const float* vol,
		const std::vector<int3>& indexList, std::unordered_set<int3>& activeVoxels,
		std::unordered_map<int4, EdgeInfo>& activeEdges) {
	for (int3 pivot : indexList) {
		float fValue1 = getValue(vol, pivot.x, pivot.y, pivot.z);
		if (fValue1 != backgroundValue) {
			for (int a = 0; a < 3; a++) {
				int3 axis = AXIS_OFFSET[a];
				int3 nbr = pivot + axis;
				if (nbr.x >= 0 && nbr.y >= 0 && nbr.z >= 0 && nbr.x < rows
						&& nbr.y < cols && nbr.z < slices) {
					float fValue2 = getValue(vol, nbr.x, nbr.y, nbr.z);
					if (fValue2 != backgroundValue && fValue1 * fValue2 < 0) {
						float3 crossing(pivot);
						double fDelta = fValue2 - fValue1;
						if (std::abs(fDelta) < 1E-3f) {
							crossing += float3(axis) * 0.5f;
						} else {
							crossing += float3(axis)
									* (float) ((isoLevel - fValue1) / fDelta);
						}
						EdgeInfo info;
						info.point = crossing;
						if (winding == Winding::Clockwise) {
							info.winding = (fValue1 < 0.f);
						} else {
							info.winding = (fValue1 > 0.f);
						}
						activeEdges[int4(pivot, a)] = info;
						auto edgeNodes = EDGE_NODE_OFFSETS[a];
						for (int i = 0; i < 4; i++) {
							int3 id = pivot - edgeNodes[i];
							activeVoxels.insert(id);
						}
					}
				}
			}
		}
	}

}
void IsoSurface::triangulateUsingMarchingCubes(const float* vol,
		std::unordered_map<uint64_t, EdgeSplit3D>& splits,
		std::vector<IsoTriangle>& triangles, int x, int y, int z,
		size_t& vertexCount) {
	int3 afCubeValue[8];
	int iFlagIndex = 0;
	for (int iVertex = 0; iVertex < 8; ++iVertex) {
		int3 v((x + vertexOffset[iVertex][0]), (y + vertexOffset[iVertex][1]),
				((z + vertexOffset[iVertex][2])));
		afCubeValue[iVertex] = v;
		float val = getValue(vol, v.x, v.y, v.z);
		if (val == backgroundValue)
			return;
		if (val < isoLevel)
			iFlagIndex |= 1 << iVertex;
	}
	int iEdgeFlags = cubeEdgeFlagsCC626[iFlagIndex];
	if (iEdgeFlags == 0)
		return;
	EdgeSplit3D split;
	EdgeSplit3D asEdgeVertex[12];
	for (int iEdge = 0; iEdge < 12; ++iEdge) {
		if ((iEdgeFlags & (1 << iEdge)) != 0) {
			int3 v = afCubeValue[edgeConnection[iEdge][0]];
			if (getValue(vol, v.x, v.y, v.z) < isoLevel) {
				split = EdgeSplit3D(afCubeValue[edgeConnection[iEdge][0]],
						afCubeValue[edgeConnection[iEdge][1]], rows, cols,
						slices);
			} else {
				split = EdgeSplit3D(afCubeValue[edgeConnection[iEdge][1]],
						afCubeValue[edgeConnection[iEdge][0]], rows, cols,
						slices);
			}

			uint64_t lHashValue = split.hashValue();

			std::unordered_map<uint64_t, EdgeSplit3D>::const_iterator itr;
			itr = splits.find(lHashValue);

			if (itr == splits.end()) {
				split.vertexId = vertexCount++;
				aly::float3 pt3d;
				float fOffset = getOffset(vol,
						afCubeValue[edgeConnection[iEdge][0]],
						afCubeValue[edgeConnection[iEdge][1]]);
				pt3d.x = (x
						+ (vertexOffset[edgeConnection[iEdge][0]][0]
								+ fOffset * edgeDirection[iEdge][0]));
				pt3d.y = (y
						+ (vertexOffset[edgeConnection[iEdge][0]][1]
								+ fOffset * edgeDirection[iEdge][1]));
				pt3d.z = (z
						+ (vertexOffset[edgeConnection[iEdge][0]][2]
								+ fOffset * edgeDirection[iEdge][2]));
				split.point = pt3d;
				splits.insert(std::pair<uint64_t, EdgeSplit3D>(lHashValue, split));
			} else {
				split = itr->second;
			}

			asEdgeVertex[iEdge] = split;
		}
	}

	for (int iTriangle = 0; iTriangle < 5; iTriangle++) {
		if (triangleConnectionTable[16 * iFlagIndex + 3 * iTriangle] < 0)
			break;
		IsoTriangle tri;
		for (int iCorner = 0; iCorner < 3; ++iCorner) {
			int iVertexIndex = triangleConnectionTable[16 * iFlagIndex
					+ 3 * iTriangle + iCorner];
			split = asEdgeVertex[iVertexIndex];
			tri.vertexIds[iCorner] = split.vertexId;
		}
		triangles.push_back(tri);
		triangleCount++;
	}

}

void IsoSurface::triangulateUsingMarchingCubes(const float* vol,
		std::unordered_map<int4, EdgeSplit3D>& splits, std::vector<IsoTriangle>& triangles,
		int x, int y, int z, int ox, int oy, int oz, size_t& vertexCount) {
	int3 afCubeValue[8];
	int3 off = int3(ox, oy, oz);
	int iFlagIndex = 0;
	for (int iVertex = 0; iVertex < 8; ++iVertex) {
		int3 v((x + vertexOffset[iVertex][0]), (y + vertexOffset[iVertex][1]),
				((z + vertexOffset[iVertex][2])));
		afCubeValue[iVertex] = v + off;
		float val = getValue(vol, v.x, v.y, v.z);
		if (val == backgroundValue)
			return;
		if (val < isoLevel)
			iFlagIndex |= 1 << iVertex;
	}
	int iEdgeFlags = cubeEdgeFlagsCC626[iFlagIndex];
	if (iEdgeFlags == 0)
		return;
	EdgeSplit3D split;
	EdgeSplit3D asEdgeVertex[12];
	for (int iEdge = 0; iEdge < 12; ++iEdge) {
		if ((iEdgeFlags & (1 << iEdge)) != 0) {
			int3 v = afCubeValue[edgeConnection[iEdge][0]];
			if (getValue(vol, v.x, v.y, v.z) < isoLevel) {
				split = EdgeSplit3D(afCubeValue[edgeConnection[iEdge][0]],
						afCubeValue[edgeConnection[iEdge][1]], rows, cols,
						slices);
			} else {
				split = EdgeSplit3D(afCubeValue[edgeConnection[iEdge][1]],
						afCubeValue[edgeConnection[iEdge][0]], rows, cols,
						slices);
			}

			int4 lHashValue = split.bigHashValue();
			std::unordered_map<int4, EdgeSplit3D>::const_iterator itr;
			itr = splits.find(lHashValue);
			if (itr == splits.end()) {
				split.vertexId = vertexCount++;
				aly::float3 pt3d;
				float fOffset = getOffset(vol,
						afCubeValue[edgeConnection[iEdge][0]] - off,
						afCubeValue[edgeConnection[iEdge][1]] - off);
				pt3d.x = (ox + x
						+ (vertexOffset[edgeConnection[iEdge][0]][0]
								+ fOffset * edgeDirection[iEdge][0]));
				pt3d.y = (oy + y
						+ (vertexOffset[edgeConnection[iEdge][0]][1]
								+ fOffset * edgeDirection[iEdge][1]));
				pt3d.z = (oz + z
						+ (vertexOffset[edgeConnection[iEdge][0]][2]
								+ fOffset * edgeDirection[iEdge][2]));
				split.point = pt3d;
				splits.insert(std::pair<int4, EdgeSplit3D>(lHashValue, split));
			} else {
				split = itr->second;
			}

			asEdgeVertex[iEdge] = split;
		}
	}

	for (int iTriangle = 0; iTriangle < 5; iTriangle++) {
		if (triangleConnectionTable[16 * iFlagIndex + 3 * iTriangle] < 0)
			break;
		IsoTriangle tri;
		for (int iCorner = 0; iCorner < 3; ++iCorner) {
			int iVertexIndex = triangleConnectionTable[16 * iFlagIndex
					+ 3 * iTriangle + iCorner];
			split = asEdgeVertex[iVertexIndex];
			tri.vertexIds[iCorner] = split.vertexId;
		}
		triangles.push_back(tri);
		triangleCount++;
	}
}

void IsoSurface::triangulateUsingMarchingCubes(const float* vol,
		std::vector<EdgeSplit3D>& splits, std::vector<IsoTriangle>& triangles,
		int x, int y, int z, size_t& vertexCount) {
	int3 afCubeValue[8];
	int iFlagIndex = 0;
	for (int iVertex = 0; iVertex < 8; ++iVertex) {
		int3 v((x + vertexOffset[iVertex][0]), (y + vertexOffset[iVertex][1]),
				((z + vertexOffset[iVertex][2])));
		afCubeValue[iVertex] = v;
		float val = getValue(vol, v.x, v.y, v.z);
		if (val == backgroundValue)
			return;
		if (val < isoLevel)
			iFlagIndex |= 1 << iVertex;
	}
	int iEdgeFlags = cubeEdgeFlagsCC626[iFlagIndex];
	if (iEdgeFlags == 0)
		return;
	EdgeSplit3D split;
	EdgeSplit3D asEdgeVertex[12];
	for (int iEdge = 0; iEdge < 12; ++iEdge) {
		if ((iEdgeFlags & (1 << iEdge)) != 0) {
			int3 v = afCubeValue[edgeConnection[iEdge][0]];
			if (getValue(vol, v.x, v.y, v.z) < isoLevel) {
				split = EdgeSplit3D(afCubeValue[edgeConnection[iEdge][0]],
						afCubeValue[edgeConnection[iEdge][1]], rows, cols,
						slices);
			} else {
				split = EdgeSplit3D(afCubeValue[edgeConnection[iEdge][1]],
						afCubeValue[edgeConnection[iEdge][0]], rows, cols,
						slices);
			}

			split.vertexId = vertexCount++;
			aly::float3 pt3d;
			float fOffset = getOffset(vol,
					afCubeValue[edgeConnection[iEdge][0]],
					afCubeValue[edgeConnection[iEdge][1]]);
			pt3d.x = (x
					+ (vertexOffset[edgeConnection[iEdge][0]][0]
							+ fOffset * edgeDirection[iEdge][0]));
			pt3d.y = (y
					+ (vertexOffset[edgeConnection[iEdge][0]][1]
							+ fOffset * edgeDirection[iEdge][1]));
			pt3d.z = (z
					+ (vertexOffset[edgeConnection[iEdge][0]][2]
							+ fOffset * edgeDirection[iEdge][2]));
			split.point = pt3d;
			splits.push_back(split);

			asEdgeVertex[iEdge] = split;
		}
	}

	for (int iTriangle = 0; iTriangle < 5; iTriangle++) {
		if (triangleConnectionTable[16 * iFlagIndex + 3 * iTriangle] < 0)
			break;
		IsoTriangle tri;
		for (int iCorner = 0; iCorner < 3; ++iCorner) {
			int iVertexIndex = triangleConnectionTable[16 * iFlagIndex
					+ 3 * iTriangle + iCorner];
			split = asEdgeVertex[iVertexIndex];
			tri.vertexIds[iCorner] = split.vertexId;
		}
		triangles.push_back(tri);
		triangleCount++;
	}
}
aly::float4 IsoSurface::interpolateColor(const float4 *data, float x, float y,
		float z) {
	int x1 = (int) std::ceil(x);
	int y1 = (int) std::ceil(y);
	int z1 = (int) std::ceil(z);
	int x0 = (int) std::floor(x);
	int y0 = (int) std::floor(y);
	int z0 = (int) std::floor(z);

	float dx = x - x0;
	float dy = y - y0;
	float dz = z - z0;

	float hx = 1.0f - dx;
	float hy = 1.0f - dy;
	float hz = 1.0f - dz;
	return aly::float4(
			(((getImageColor(data, x0, y0, z0) * hx
					+ getImageColor(data, x1, y0, z0) * dx) * hy
					+ (getImageColor(data, x0, y1, z0) * hx
							+ getImageColor(data, x1, y1, z0) * dx) * dy) * hz
					+ ((getImageColor(data, x0, y0, z1) * hx
							+ getImageColor(data, x1, y0, z1) * dx) * hy
							+ (getImageColor(data, x0, y1, z1) * hx
									+ getImageColor(data, x1, y1, z1) * dx) * dy)
							* dz));

}

float IsoSurface::getOffset(const float* vol, const int3& v1, const int3& v2) {
	float fValue1 = getValue(vol, v1.x, v1.y, v1.z);
	float fValue2 = getValue(vol, v2.x, v2.y, v2.z);
	double fDelta = fValue2 - fValue1;
	if (std::abs(fDelta) < 1E-3f)
		return 0.5f;
	return (float) ((isoLevel - fValue1) / fDelta);
}

aly::float3 IsoSurface::interpolateNormal(const float *vol, float x, float y,
		float z) {
	int x1 = (int) std::ceil(x);
	int y1 = (int) std::ceil(y);
	int z1 = (int) std::ceil(z);
	int x0 = (int) std::floor(x);
	int y0 = (int) std::floor(y);
	int z0 = (int) std::floor(z);
	float dx = x - x0;
	float dy = y - y0;
	float dz = z - z0;

	float hx = 1.0f - dx;
	float hy = 1.0f - dy;
	float hz = 1.0f - dz;

	return aly::float3(
			(((getNormal(vol, x0, y0, z0) * hx + getNormal(vol, x1, y0, z0) * dx)
					* hy
					+ (getNormal(vol, x0, y1, z0) * hx
							+ getNormal(vol, x1, y1, z0) * dx) * dy) * hz
					+ ((getNormal(vol, x0, y0, z1) * hx
							+ getNormal(vol, x1, y0, z1) * dx) * hy
							+ (getNormal(vol, x0, y1, z1) * hx
									+ getNormal(vol, x1, y1, z1) * dx) * dy)
							* dz));
}
size_t IsoSurface::getSafeIndex(int i, int j, int k) {
	return clamp(k, 0, slices - 1) * (size_t) rows * (size_t) cols
			+ clamp(j, 0, cols - 1) * (size_t) rows
			+ (size_t) clamp(i, 0, rows - 1);
}
aly::float3 IsoSurface::getNormal(const float *vol, int i, int j, int k) {
	float gx = getImageValue(vol, i + 1, j, k)
			- getImageValue(vol, i - 1, j, k);
	float gy = getImageValue(vol, i, j + 1, k)
			- getImageValue(vol, i, j - 1, k);
	float gz = getImageValue(vol, i, j, k + 1)
			- getImageValue(vol, i, j, k - 1);

	return float3(gx, gy, gz);
}

void IsoSurface::project(aly::float3* points, const int& numPoints,
		aly::float3* normals, float* levelset, const aly::box3f& bbox,
		const int& rows, const int& cols, const int& slices, int MAX_ITERATIONS,
		float LEVEL_SET_THRESH) {

	this->rows = rows;
	this->cols = cols;
	this->slices = slices;
	isoLevel = 0;
	for (int i = 0; i < numPoints; ++i) {
		aly::float3 pt = points[i];
		pt = (pt - bbox.position) / bbox.dimensions;
		pt.x *= rows;
		pt.y *= cols;
		pt.z *= slices;
		aly::float3 old = pt;
		aly::float3 norm;
		float lev;
		for (int n = 0; n < MAX_ITERATIONS; n++) {
			lev = interpolate(levelset, pt.x, pt.y, pt.z);
			norm = interpolateNormal(levelset, pt.x, pt.y, pt.z);
			norm = norm / std::max(1E-9f, length(norm));
			aly::float3 delta = norm * min(lev, 0.5f);
			pt = pt - delta;
			if (std::abs(lev) < LEVEL_SET_THRESH || pt.x >= rows - 1
					|| pt.y >= cols - 1 || pt.z >= slices - 1 || pt.x <= 0
					|| pt.y <= 0 || pt.z <= 0)
				break;
			break;
		}
		norm = interpolateNormal(levelset, pt.x, pt.y, pt.z);
		norm = norm / length(norm);
		normals[i] = norm;
		pt.x /= rows;
		pt.y /= cols;
		pt.z /= slices;
		pt = bbox.position + pt * bbox.dimensions;
		points[i] = pt;
	}
}
void IsoSurface::project(aly::float3* points, const int& numPoints,
		aly::float3* normals, float* levelset, const int& rows, const int& cols,
		const int& slices, int MAX_ITERATIONS, float LEVEL_SET_THRESH) {

	this->rows = rows;
	this->cols = cols;
	this->slices = slices;
	isoLevel = 0;
	for (int i = 0; i < numPoints; ++i) {
		aly::float3 pt = points[i];
		aly::float3 old = pt;
		aly::float3 norm;
		float lev;
		for (int n = 0; n < MAX_ITERATIONS; n++) {
			lev = interpolate(levelset, pt.x, pt.y, pt.z);
			norm = interpolateNormal(levelset, pt.x, pt.y, pt.z);
			norm = norm / std::max(1E-9f, length(norm));
			aly::float3 delta = norm * min(lev, 0.5f);
			pt = pt - delta;
			if (std::abs(lev) < LEVEL_SET_THRESH || pt.x >= rows - 1
					|| pt.y >= cols - 1 || pt.z >= slices - 1 || pt.x <= 0
					|| pt.y <= 0 || pt.z <= 0)
				break;
			break;
		}
		norm = interpolateNormal(levelset, pt.x, pt.y, pt.z);
		norm = norm / length(norm);
		normals[i] = norm;
		points[i] = pt;
	}
}
float IsoSurface::interpolate(const float *data, float x, float y, float z) {
	int x1 = (int) std::ceil(x);
	int y1 = (int) std::ceil(y);
	int z1 = (int) std::ceil(z);
	int x0 = (int) std::floor(x);
	int y0 = (int) std::floor(y);
	int z0 = (int) std::floor(z);
	float dx = x - x0;
	float dy = y - y0;
	float dz = z - z0;

	float hx = 1.0f - dx;
	float hy = 1.0f - dy;
	float hz = 1.0f - dz;

	return ((((getImageValue(data, x0, y0, z0) * hx
			+ getImageValue(data, x1, y0, z0) * dx) * hy
			+ (getImageValue(data, x0, y1, z0) * hx
					+ getImageValue(data, x1, y1, z0) * dx) * dy) * hz
			+ ((getImageValue(data, x0, y0, z1) * hx
					+ getImageValue(data, x1, y0, z1) * dx) * hy
					+ (getImageValue(data, x0, y1, z1) * hx
							+ getImageValue(data, x1, y1, z1) * dx) * dy) * dz));
}

float IsoSurface::getImageValue(const float* image, int i, int j, int k) {
	return image[getSafeIndex(i, j, k)];
}

}
