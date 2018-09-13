/*M///////////////////////////////////////////////////////////////////////////////////////
 //
 //  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
 //
 //  By downloading, copying, installing or using the software you agree to this license.
 //  If you do not agree to this license, do not download, install,
 //  copy or use the software.
 //
 //
 //                           License Agreement
 //                For Open Source Computer Vision Library
 //
 // Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
 // Copyright (C) 2009, Willow Garage Inc., all rights reserved.
 // Third party copyrights are property of their respective owners.
 //
 // Redistribution and use in source and binary forms, with or without modification,
 // are permitted provided that the following conditions are met:
 //
 //   * Redistribution's of source code must retain the above copyright notice,
 //     this list of conditions and the following disclaimer.
 //
 //   * Redistribution's in binary form must reproduce the above copyright notice,
 //     this list of conditions and the following disclaimer in the documentation
 //     and/or other materials provided with the distribution.
 //
 //   * The name of the copyright holders may not be used to endorse or promote products
 //     derived from this software without specific prior written permission.
 //
 // This software is provided by the copyright holders and contributors "as is" and
 // any express or implied warranties, including, but not limited to, the implied
 // warranties of merchantability and fitness for a particular purpose are disclaimed.
 // In no event shall the Intel Corporation or contributors be liable for any direct,
 // indirect, incidental, special, exemplary, or consequential damages
 // (including, but not limited to, procurement of substitute goods or services;
 // loss of use, data, or profits; or business interruption) however caused
 // and on any theory of liability, whether in contract, strict liability,
 // or tort (including negligence or otherwise) arising in any way out of
 // the use of this software, even if advised of the possibility of such damage.
 //
 //M*/

/****************************************************************************************\
    A part of the file implements TIFF reader on base of libtiff library
 (see otherlibs/_graphics/readme.txt for copyright notice)
 \****************************************************************************************/
#include <limits>
#include "tiff.h"
#include "tiffio.h"
#include "math/AlloyVecMath.h"
#include "system/AlloyFileUtil.h"
#include "image/TiffReader.h"
/*
// native simple TIFF codec
enum TiffCompression {
	TIFF_UNCOMP = 1, TIFF_HUFFMAN = 2, TIFF_PACKBITS = 32773
};

enum TiffByteOrder {
	TIFF_ORDER_II = 0x4949, TIFF_ORDER_MM = 0x4d4d
};

enum TiffTag {
	TIFF_TAG_WIDTH = 256,
	TIFF_TAG_HEIGHT = 257,
	TIFF_TAG_BITS_PER_SAMPLE = 258,
	TIFF_TAG_COMPRESSION = 259,
	TIFF_TAG_PHOTOMETRIC = 262,
	TIFF_TAG_STRIP_OFFSETS = 273,
	TIFF_TAG_STRIP_COUNTS = 279,
	TIFF_TAG_SAMPLES_PER_PIXEL = 277,
	TIFF_TAG_ROWS_PER_STRIP = 278,
	TIFF_TAG_PLANAR_CONFIG = 284,
	TIFF_TAG_COLOR_MAP = 320
};

enum TiffFieldType {
	TIFF_TYPE_BYTE = 1, TIFF_TYPE_SHORT = 3, TIFF_TYPE_LONG = 4
};
*/
#define  descale(x,n)  (((x) + (1 << ((n)-1))) >> (n))
#define  saturate(x)   (ubyte)(((x) & ~255) == 0 ? (x) : ~((x)>>31))

namespace aly {


#ifdef TIFF_BIGENDIAN
static int grfmt_tiff_err_handler_init = 0;
static void GrFmtSilentTIFFErrorHandler(const char*, const char*, va_list) {
}

#define  SCALE  14
#define  cR  (int)(0.299*(1 << SCALE) + 0.5)
#define  cG  (int)(0.587*(1 << SCALE) + 0.5)
#define  cB  ((1 << SCALE) - cR - cG)

void icvCvt_BGR2Gray_8u_C3C1R(const ubyte* rgb, int rgb_step, ubyte* gray,
		int gray_step, int2 size, int _swap_rb) {
	int i;
	int swap_rb = _swap_rb ? 2 : 0;
	for (; size.y--; gray += gray_step) {
		for (i = 0; i < size.x; i++, rgb += 3) {
			int t = descale(
					rgb[swap_rb] * cB + rgb[1] * cG + rgb[swap_rb ^ 2] * cR,
					SCALE);
			gray[i] = (ubyte) t;
		}

		rgb += rgb_step - size.x * 3;
	}
}

void icvCvt_BGRA2Gray_16u_CnC1R(const ushort* rgb, int rgb_step, ushort* gray,
		int gray_step, int2 size, int ncn, int _swap_rb) {
	int i;
	int swap_rb = _swap_rb ? 2 : 0;
	for (; size.y--; gray += gray_step) {
		for (i = 0; i < size.x; i++, rgb += ncn) {
			int t = descale(
					rgb[swap_rb] * cB + rgb[1] * cG + rgb[swap_rb ^ 2] * cR,
					SCALE);
			gray[i] = (ushort) t;
		}

		rgb += rgb_step - size.x * ncn;
	}
}

void icvCvt_BGRA2Gray_8u_C4C1R(const ubyte* rgba, int rgba_step, ubyte* gray,
		int gray_step, int2 size, int _swap_rb) {
	int i;
	int swap_rb = _swap_rb ? 2 : 0;
	for (; size.y--; gray += gray_step) {
		for (i = 0; i < size.x; i++, rgba += 4) {
			int t = descale(
					rgba[swap_rb] * cB + rgba[1] * cG + rgba[swap_rb ^ 2] * cR,
					SCALE);
			gray[i] = (ubyte) t;
		}

		rgba += rgba_step - size.x * 4;
	}
}

void icvCvt_Gray2BGR_8u_C1C3R(const ubyte* gray, int gray_step, ubyte* bgr,
		int bgr_step, int2 size) {
	int i;
	for (; size.y--; gray += gray_step) {
		for (i = 0; i < size.x; i++, bgr += 3) {
			bgr[0] = bgr[1] = bgr[2] = gray[i];
		}
		bgr += bgr_step - size.x * 3;
	}
}

void icvCvt_Gray2BGR_16u_C1C3R(const ushort* gray, int gray_step, ushort* bgr,
		int bgr_step, int2 size) {
	int i;
	for (; size.y--; gray += gray_step / sizeof(gray[0])) {
		for (i = 0; i < size.x; i++, bgr += 3) {
			bgr[0] = bgr[1] = bgr[2] = gray[i];
		}
		bgr += bgr_step / sizeof(bgr[0]) - size.x * 3;
	}
}

void icvCvt_BGRA2BGR_8u_C4C3R(const ubyte* bgra, int bgra_step, ubyte* bgr,
		int bgr_step, int2 size, int _swap_rb) {
	int i;
	int swap_rb = _swap_rb ? 2 : 0;
	for (; size.y--;) {
		for (i = 0; i < size.x; i++, bgr += 3, bgra += 4) {
			ubyte t0 = bgra[swap_rb];
			ubyte t1 = bgra[1];
			bgr[0] = t0;
			bgr[1] = t1;
			t0 = bgra[swap_rb ^ 2];
			bgr[2] = t0;
		}
		bgr += bgr_step - size.x * 3;
		bgra += bgra_step - size.x * 4;
	}
}
void icvCvt_BGRA2RGB_8u_C4C3R(const ubyte* bgra, int bgra_step, ubyte* rgb,
		int bgr_step, int2 size, int _swap_rb) {
	int i;
	int swap_rb = _swap_rb ? 2 : 0;
	for (; size.y--;) {
		for (i = 0; i < size.x; i++, rgb += 3, bgra += 4) {
			ubyte t0 = bgra[swap_rb];
			ubyte t1 = bgra[1];
			rgb[2] = t0;
			rgb[1] = t1;
			t0 = bgra[swap_rb ^ 2];
			rgb[0] = t0;
		}
		rgb += bgr_step - size.x * 3;
		bgra += bgra_step - size.x * 4;
	}
}
void icvCvt_BGRA2BGR_16u_C4C3R(const ushort* bgra, int bgra_step, ushort* bgr,
		int bgr_step, int2 size, int _swap_rb) {
	int i;
	int swap_rb = _swap_rb ? 2 : 0;
	for (; size.y--;) {
		for (i = 0; i < size.x; i++, bgr += 3, bgra += 4) {
			ushort t0 = bgra[swap_rb], t1 = bgra[1];
			bgr[0] = t0;
			bgr[1] = t1;
			t0 = bgra[swap_rb ^ 2];
			bgr[2] = t0;
		}
		bgr += bgr_step / sizeof(bgr[0]) - size.x * 3;
		bgra += bgra_step / sizeof(bgra[0]) - size.x * 4;
	}
}
void icvCvt_BGRA2RGB_16u_C4C3R(const ushort* bgra, int bgra_step, ushort* bgr,
		int bgr_step, int2 size, int _swap_rb) {
	int i;
	int swap_rb = _swap_rb ? 2 : 0;
	for (; size.y--;) {
		for (i = 0; i < size.x; i++, bgr += 3, bgra += 4) {
			ushort t0 = bgra[swap_rb], t1 = bgra[1];
			bgr[2] = t0;
			bgr[1] = t1;
			t0 = bgra[swap_rb ^ 2];
			bgr[0] = t0;
		}
		bgr += bgr_step / sizeof(bgr[0]) - size.x * 3;
		bgra += bgra_step / sizeof(bgra[0]) - size.x * 4;
	}
}
void icvCvt_BGRA2RGBA_8u_C4R(const ubyte* bgra, int bgra_step, ubyte* rgba,
		int rgba_step, int2 size) {
	int i;
	for (; size.y--;) {
		for (i = 0; i < size.x; i++, bgra += 4, rgba += 4) {
			ubyte t0 = bgra[0], t1 = bgra[1];
			ubyte t2 = bgra[2], t3 = bgra[3];
			rgba[0] = t2;
			rgba[1] = t1;
			rgba[2] = t0;
			rgba[3] = t3;
		}
		bgra += bgra_step - size.x * 4;
		rgba += rgba_step - size.x * 4;
	}
}

void icvCvt_BGRA2RGBA_16u_C4R(const ushort* bgra, int bgra_step, ushort* rgba,
		int rgba_step, int2 size) {
	int i;
	for (; size.y--;) {
		for (i = 0; i < size.x; i++, bgra += 4, rgba += 4) {
			ushort t0 = bgra[0], t1 = bgra[1];
			ushort t2 = bgra[2], t3 = bgra[3];

			rgba[0] = t2;
			rgba[1] = t1;
			rgba[2] = t0;
			rgba[3] = t3;
		}
		bgra += bgra_step / sizeof(bgra[0]) - size.x * 4;
		rgba += rgba_step / sizeof(rgba[0]) - size.x * 4;
	}
}

void icvCvt_BGR2RGB_8u_C3R(const ubyte* bgr, int bgr_step, ubyte* rgb,
		int rgb_step, int2 size) {
	int i;
	for (; size.y--;) {
		for (i = 0; i < size.x; i++, bgr += 3, rgb += 3) {
			ubyte t0 = bgr[0], t1 = bgr[1], t2 = bgr[2];
			rgb[2] = t0;
			rgb[1] = t1;
			rgb[0] = t2;
		}
		bgr += bgr_step - size.x * 3;
		rgb += rgb_step - size.x * 3;
	}
}

void icvCvt_BGR2RGB_16u_C3R(const ushort* bgr, int bgr_step, ushort* rgb,
		int rgb_step, int2 size) {
	int i;
	for (; size.y--;) {
		for (i = 0; i < size.x; i++, bgr += 3, rgb += 3) {
			ushort t0 = bgr[0], t1 = bgr[1], t2 = bgr[2];
			rgb[2] = t0;
			rgb[1] = t1;
			rgb[0] = t2;
		}
		bgr += bgr_step - size.x * 3;
		rgb += rgb_step - size.x * 3;
	}
}

typedef unsigned short ushort;

void icvCvt_BGR5552Gray_8u_C2C1R(const ubyte* bgr555, int bgr555_step,
		ubyte* gray, int gray_step, int2 size) {
	int i;
	for (; size.y--; gray += gray_step, bgr555 += bgr555_step) {
		for (i = 0; i < size.x; i++) {
			int t =
					descale(
							((((ushort*) bgr555)[i] << 3) & 0xf8) * cB + ((((ushort*) bgr555)[i] >> 2) & 0xf8) * cG + ((((ushort*) bgr555)[i] >> 7) & 0xf8) * cR,
							SCALE);
			gray[i] = (ubyte) t;
		}
	}
}

void icvCvt_BGR5652Gray_8u_C2C1R(const ubyte* bgr565, int bgr565_step,
		ubyte* gray, int gray_step, int2 size) {
	int i;
	for (; size.y--; gray += gray_step, bgr565 += bgr565_step) {
		for (i = 0; i < size.x; i++) {
			int t =
					descale(
							((((ushort*) bgr565)[i] << 3) & 0xf8) * cB + ((((ushort*) bgr565)[i] >> 3) & 0xfc) * cG + ((((ushort*) bgr565)[i] >> 8) & 0xf8) * cR,
							SCALE);
			gray[i] = (ubyte) t;
		}
	}
}

void icvCvt_BGR5552BGR_8u_C2C3R(const ubyte* bgr555, int bgr555_step,
		ubyte* bgr, int bgr_step, int2 size) {
	int i;
	for (; size.y--; bgr555 += bgr555_step) {
		for (i = 0; i < size.x; i++, bgr += 3) {
			int t0 = (((ushort*) bgr555)[i] << 3) & 0xf8;
			int t1 = (((ushort*) bgr555)[i] >> 2) & 0xf8;
			int t2 = (((ushort*) bgr555)[i] >> 7) & 0xf8;
			bgr[0] = (ubyte) t0;
			bgr[1] = (ubyte) t1;
			bgr[2] = (ubyte) t2;
		}
		bgr += bgr_step - size.x * 3;
	}
}

void icvCvt_BGR5652BGR_8u_C2C3R(const ubyte* bgr565, int bgr565_step,
		ubyte* bgr, int bgr_step, int2 size) {
	int i;
	for (; size.y--; bgr565 += bgr565_step) {
		for (i = 0; i < size.x; i++, bgr += 3) {
			int t0 = (((ushort*) bgr565)[i] << 3) & 0xf8;
			int t1 = (((ushort*) bgr565)[i] >> 3) & 0xfc;
			int t2 = (((ushort*) bgr565)[i] >> 8) & 0xf8;
			bgr[0] = (ubyte) t0;
			bgr[1] = (ubyte) t1;
			bgr[2] = (ubyte) t2;
		}
		bgr += bgr_step - size.x * 3;
	}
}
bool ReadTiffHeader(const std::string& file, TiffHeader& header) {
	if (!grfmt_tiff_err_handler_init) {
		grfmt_tiff_err_handler_init = 1;
		TIFFSetErrorHandler(GrFmtSilentTIFFErrorHandler);
		TIFFSetWarningHandler(GrFmtSilentTIFFErrorHandler);
	}
	bool result = false;
	TIFF* tif = TIFFOpen(file.c_str(), "r");
	if (tif) {
		uint32_t wdth = 0, hght = 0;
		uint16_t photometric = 0;
		if (TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &wdth)
				&& TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &hght)
				&& TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric)) {
			uint16_t bpp = 8, ncn = photometric > 1 ? 3 : 1;
			TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bpp);
			TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &ncn);
			header.width = wdth;
			header.height = hght;
			if ((bpp == 32 && ncn == 3) || photometric == PHOTOMETRIC_LOGLUV) {
				header.type = ImageType::FLOAT;
				header.channels = 3;
				return true;
			}
			if (bpp > 8
					&& ((photometric != 2 && photometric != 1)
							|| (ncn != 1 && ncn != 3 && ncn != 4)))
				bpp = 8;

			header.channels = ncn;
			int wanted_channels = (ncn > 4) ? 4 : ncn;
			switch (bpp) {
			case 8:
				header.type = ImageType::UBYTE;
				header.channels = (photometric > 1 ? wanted_channels : 1);
				break;
			case 16:
				header.type = ImageType::USHORT;
				header.channels = (photometric > 1 ? wanted_channels : 1);
				break;
			case 32:
				header.type = ImageType::FLOAT;
				header.channels = (photometric > 1 ? 3 : 1);
				break;
			case 64:
				header.type = ImageType::DOUBLE;
				header.channels = (photometric > 1 ? 3 : 1);
				break;

			default:
				result = false;
			}
			result = true;
		}
	}
	if (result) {
		header.ptr = tif;
	} else {
		if(tif)TIFFClose(tif);
	}
	return result;
}

template<class T, int C, ImageType I> bool ReadTiffImageInternal(
		const std::string& file, Image<T, C, I>& img) {
	TiffHeader header;
	if (!ReadTiffHeader(file, header)) {
		return false;
	}
	bool result = false;
	bool color = img.channels > 1;
	if (C != header.channels || I != header.type || header.width == 0
			|| header.height == 0) {
		TIFFClose((TIFF*) header.ptr);
		return false;
	}
	img.resize(header.width, header.height);
	ubyte* data = (ubyte*) img.ptr();
	TIFF* m_tif = (TIFF*) header.ptr;
	const int m_width = header.width;
	const int m_height = header.height;
	size_t lineStep=img.width * img.typeSize();
	TIFF* tif = (TIFF*) header.ptr;
	int tile_width0 = m_width;
	uint32_t tile_height0 = 0;
	int x, y, i;
	int is_tiled = TIFFIsTiled(tif);
	uint16_t photometric;
	TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric);
	uint16_t bpp = 8;
	uint16_t ncn = photometric > 1 ? 3 : 1;
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bpp);
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &ncn);
	const int bitsPerByte = 8;
	int dst_bpp = (int) (sizeof(T) * 8);
	int wanted_channels = img.channels;
	if (dst_bpp == 8) {
		char errmsg[1024];
		if (!TIFFRGBAImageOK(tif, errmsg)) {
			TIFFClose(m_tif);
			return false;
		}
	}
	if ((!is_tiled)
			|| (is_tiled && TIFFGetField(tif, TIFFTAG_TILEWIDTH, &tile_width0)
					&& TIFFGetField(tif, TIFFTAG_TILELENGTH, &tile_height0))) {
		if (!is_tiled){
			TIFFGetField(tif, TIFFTAG_ROWSPERSTRIP, &tile_height0);
		}
		if (tile_width0 <= 0){
			tile_width0 = m_width;
		}
		if (tile_height0 <= 0|| (!is_tiled && tile_height0 == std::numeric_limits<uint32_t>::max()))
			tile_height0 = m_height;

		if (dst_bpp == 8) {
			// we will use TIFFReadRGBA* functions, so allocate temporary buffer for 32bit RGBA
			bpp = 8;
			ncn = 4;
		}
		const size_t buffer_size = (bpp / bitsPerByte) * ncn * tile_height0* tile_width0;
		std::vector<ubyte> _buffer(buffer_size);
		ubyte* buffer = _buffer.data();
		ushort* buffer16 = (ushort*) buffer;
		float* buffer32 = (float*) buffer;
		double* buffer64 = (double*) buffer;
		int tileidx = 0;
		for (y = 0; y < m_height;y += tile_height0, data += lineStep * tile_height0) {
			int tile_height = tile_height0;
			if (y + tile_height > m_height)
				tile_height = m_height - y;

			for (x = 0; x < m_width; x += tile_width0, tileidx++) {
				int tile_width = tile_width0, ok;

				if (x + tile_width > m_width)
					tile_width = m_width - x;

				switch (dst_bpp) {
				case 8: {
					ubyte * bstart = buffer;
					if (!is_tiled)
						ok = TIFFReadRGBAStrip(tif, y, (uint32_t*) buffer);
					else {
						ok = TIFFReadRGBATile(tif, x, y, (uint32_t*) buffer);
						//Tiles fill the buffer from the bottom up
						bstart += (tile_height0 - tile_height) * tile_width0
								* 4;
					}
					if (!ok) {
						TIFFClose(m_tif);
						return false;
					}

					for (i = 0; i < tile_height; i++)
						if (color) {
							if (wanted_channels == 4) {
								icvCvt_BGRA2RGBA_8u_C4R(
										bstart + i * tile_width0 * 4, 0,
										data + x * 4
												+ lineStep
														* (tile_height - i - 1),
										0, int2(tile_width, 1));
							} else {
								icvCvt_BGRA2RGB_8u_C4C3R(
										bstart + i * tile_width0 * 4, 0,
										data + x * 3
												+ lineStep
														* (tile_height - i - 1),
										0, int2(tile_width, 1), 2);
							}
						} else
							icvCvt_BGRA2Gray_8u_C4C1R(
									bstart + i * tile_width0 * 4, 0,
									data + x + lineStep * (tile_height - i - 1),
									0, int2(tile_width, 1), 2);
					break;
				}

				case 16: {
					if (!is_tiled)
						ok = (int) TIFFReadEncodedStrip(tif, tileidx,
								(uint32_t*) buffer, buffer_size) >= 0;
					else
						ok = (int) TIFFReadEncodedTile(tif, tileidx,
								(uint32_t*) buffer, buffer_size) >= 0;

					if (!ok) {
						TIFFClose(m_tif);
						return false;
					}
					for (i = 0; i < tile_height; i++) {
						if (color) {
							if (ncn == 1) {
								icvCvt_Gray2BGR_16u_C1C3R(
										buffer16 + i * tile_width0 * ncn, 0,
										(ushort*) (data + lineStep* i) + x * 3,
										0, int2(tile_width, 1));
							} else if (ncn == 3) {
								icvCvt_RGB2BGR_16u_C3R(
										buffer16 + i * tile_width0 * ncn, 0,
										(ushort*) (data + lineStep* i) + x * 3,
										0, int2(tile_width, 1));
							} else if (ncn == 4) {
								if (wanted_channels == 4) {
									icvCvt_BGRA2RGBA_16u_C4R(
											buffer16 + i * tile_width0 * ncn, 0,
											(ushort*) (data + lineStep * i)
													+ x * 4, 0,
											int2(tile_width, 1));
								} else {
									icvCvt_BGRA2RGB_16u_C4C3R(
											buffer16 + i * tile_width0 * ncn, 0,
											(ushort*) (data +lineStep * i)
													+ x * 3, 0,
											int2(tile_width, 1), 2);
								}
							} else {
								icvCvt_BGRA2RGB_16u_C4C3R(
										buffer16 + i * tile_width0 * ncn, 0,
										(ushort*) (data +lineStep * i) + x * 3,
										0, int2(tile_width, 1), 2);
							}
						} else {
							if (ncn == 1) {
								memcpy((ushort*) (data + lineStep * i) + x,
										buffer16 + i * tile_width0 * ncn,
										tile_width * sizeof(buffer16[0]));
							} else {
								icvCvt_BGRA2Gray_16u_CnC1R(
										buffer16 + i * tile_width0 * ncn, 0,
										(ushort*) (data + lineStep * i) + x, 0,
										int2(tile_width, 1), ncn, 2);
							}
						}
					}
					break;
				}

				case 32:
				case 64: {
					if (!is_tiled)
						ok = (int) TIFFReadEncodedStrip(tif, tileidx, buffer,
								buffer_size) >= 0;
					else
						ok = (int) TIFFReadEncodedTile(tif, tileidx, buffer,
								buffer_size) >= 0;

					if (!ok || ncn != 1) {
						TIFFClose(m_tif);
						return false;
					}
					for (i = 0; i < tile_height; i++) {
						if (dst_bpp == 32) {
							memcpy((float*) (data + lineStep * i) + x,
									buffer32 + i * tile_width0 * ncn,
									tile_width * sizeof(buffer32[0]));
						} else {
							memcpy((double*) (data + lineStep * i) + x,
									buffer64 + i * tile_width0 * ncn,
									tile_width * sizeof(buffer64[0]));
						}
					}

					break;
				}
				default: {
					TIFFClose(m_tif);
					return false;
				}
				}
			}
		}
		result = true;
	}
	if (m_tif) {
		TIFFClose(m_tif);
	}
	return result;
}
#else
template<class T, int C, ImageType I> bool ReadTiffImageInternal(
		const std::string& file, Image<T, C, I>& img) {
	throw std::runtime_error("LibTIFF unavailable in this build.");
}

#endif
bool ReadTiffImage(const std::string& file, Image1us& img) {
	return ReadTiffImageInternal(file, img);
}
bool ReadTiffImage(const std::string& file, Image2us& img) {
	return ReadTiffImageInternal(file, img);
}
bool ReadTiffImage(const std::string& file, Image1ui& img) {
	return ReadTiffImageInternal(file, img);
}
bool ReadTiffImage(const std::string& file, Image1ub& img) {
	return ReadTiffImageInternal(file, img);
}
bool ReadTiffImage(const std::string& file, ImageRGB& img) {
	return ReadTiffImageInternal(file, img);
}
bool ReadTiffImage(const std::string& file, ImageRGBA& img) {
	return ReadTiffImageInternal(file, img);
}
bool ReadTiffImage(const std::string& file, Image3us& img) {
	return ReadTiffImageInternal(file, img);
}
bool ReadTiffImage(const std::string& file, Image4us& img) {
	return ReadTiffImageInternal(file, img);
}
bool ReadTiffImage(const std::string& file, Image1f& img) {
	return ReadTiffImageInternal(file, img);
}
bool ReadTiffImage(const std::string& file, ImageRGBf& img) {
	return ReadTiffImageInternal(file, img);
}
bool ReadTiffImage(const std::string& file, ImageRGBAf& img) {
	return ReadTiffImageInternal(file, img);
}
void ReadImageFromFile(const std::string& file, aly::ImageRGB& image,aly::BayerFilter filter) {
	Image1ub img;
	ReadTiffImage(file, img);
	Demosaic(img,image,filter);
}
void ReadImageFromFile(const std::string& file, aly::ImageRGBf& image, aly::BayerFilter filter) {
	Image1ub img;
	ReadTiffImage(file, img);
	Demosaic(img, image, filter);
}
}
