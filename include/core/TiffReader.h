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

#ifndef TIFF_DECODER_H_
#define TIFF_DECODER_H_
#include <AlloyImage.h>
#include <string>
#include <AlloyImageProcessing.h>
namespace aly {
struct TiffHeader{
	void* ptr=nullptr;
	int width=0;
	int height=0;
	int channels=0;
	ImageType type=ImageType::UBYTE;
};


void icvCvt_BGR2Gray_8u_C3C1R( const ubyte* bgr, int bgr_step,
                               ubyte* gray, int gray_step,
                               int2 size, int swap_rb=0 );
void icvCvt_BGRA2Gray_8u_C4C1R( const ubyte* bgra, int bgra_step,
                                ubyte* gray, int gray_step,
                                int2 size, int swap_rb=0 );
void icvCvt_BGRA2Gray_16u_CnC1R( const ushort* bgra, int bgra_step,
                               ushort* gray, int gray_step,
                               int2 size, int ncn, int swap_rb=0 );

void icvCvt_Gray2BGR_8u_C1C3R( const ubyte* gray, int gray_step,
                               ubyte* bgr, int bgr_step, int2 size );
void icvCvt_Gray2BGR_16u_C1C3R( const ushort* gray, int gray_step,
                               ushort* bgr, int bgr_step, int2 size );

void icvCvt_BGRA2BGR_8u_C4C3R( const ubyte* bgra, int bgra_step,
                               ubyte* bgr, int bgr_step,
                               int2 size, int swap_rb=0 );
void icvCvt_BGRA2RGB_8u_C4C3R( const ubyte* bgra, int bgra_step,
                               ubyte* bgr, int bgr_step,
                               int2 size, int swap_rb=0 );

void icvCvt_BGRA2BGR_16u_C4C3R( const ushort* bgra, int bgra_step,
                               ushort* bgr, int bgr_step,
                               int2 size, int _swap_rb );
void icvCvt_BGRA2RGB_16u_C4C3R( const ushort* bgra, int bgra_step,
                               ushort* bgr, int bgr_step,
                               int2 size, int _swap_rb );

void icvCvt_BGR2RGB_8u_C3R( const ubyte* bgr, int bgr_step,
                            ubyte* rgb, int rgb_step, int2 size );
#define icvCvt_RGB2BGR_8u_C3R icvCvt_BGR2RGB_8u_C3R
void icvCvt_BGR2RGB_16u_C3R( const ushort* bgr, int bgr_step,
                             ushort* rgb, int rgb_step, int2 size );
#define icvCvt_RGB2BGR_16u_C3R icvCvt_BGR2RGB_16u_C3R

void icvCvt_BGRA2RGBA_8u_C4R( const ubyte* bgra, int bgra_step,
                              ubyte* rgba, int rgba_step, int2 size );
#define icvCvt_RGBA2BGRA_8u_C4R icvCvt_BGRA2RGBA_8u_C4R

void icvCvt_BGRA2RGBA_16u_C4R( const ushort* bgra, int bgra_step,
                               ushort* rgba, int rgba_step, int2 size );
#define icvCvt_RGBA2BGRA_16u_C4R icvCvt_BGRA2RGBA_16u_C4R

void icvCvt_BGR5552Gray_8u_C2C1R( const ubyte* bgr555, int bgr555_step,
                                  ubyte* gray, int gray_step, int2 size );
void icvCvt_BGR5652Gray_8u_C2C1R( const ubyte* bgr565, int bgr565_step,
                                  ubyte* gray, int gray_step, int2 size );
void icvCvt_BGR5552BGR_8u_C2C3R( const ubyte* bgr555, int bgr555_step,
                                 ubyte* bgr, int bgr_step, int2 size );
void icvCvt_BGR5652BGR_8u_C2C3R( const ubyte* bgr565, int bgr565_step,
                                 ubyte* bgr, int bgr_step, int2 size );
void icvCvt_CMYK2BGR_8u_C4C3R( const ubyte* cmyk, int cmyk_step,
                               ubyte* bgr, int bgr_step, int2 size );
void icvCvt_CMYK2Gray_8u_C4C1R( const ubyte* ycck, int ycck_step,
                                ubyte* gray, int gray_step, int2 size );

bool ReadTiffHeader(const std::string& file,TiffHeader& header);


bool ReadTiffImage(const std::string& file,Image1ub& img);
bool ReadTiffImage(const std::string& file,Image1us& img);
bool ReadTiffImage(const std::string& file,Image2us& img);
bool ReadTiffImage(const std::string& file,Image3us& img);
bool ReadTiffImage(const std::string& file,Image4us& img);
bool ReadTiffImage(const std::string& file,Image1ui& img);
bool ReadTiffImage(const std::string& file,ImageRGB& img);
bool ReadTiffImage(const std::string& file,ImageRGBA& img);
bool ReadTiffImage(const std::string& file,Image1f& img);
bool ReadTiffImage(const std::string& file,ImageRGBf& img);
bool ReadTiffImage(const std::string& file,ImageRGBAf& img);
void ReadImageFromFile(const std::string& file, aly::ImageRGB& image,aly::BayerFilter filter);
void ReadImageFromFile(const std::string& file, aly::ImageRGBf& image, aly::BayerFilter filter);

}
#endif/*_GRFMT_TIFF_H_*/
