
#ifndef TIFF_WRITER_H_
#define TIFF_WRITER_H_
#include "image/AlloyImage.h"
#include <string>
namespace aly {
bool WriteTiffImage(const std::string& file,const Image1ub& img,int compressionLevel);
bool WriteTiffImage(const std::string& file,const Image1us& img,int compressionLevel);
bool WriteTiffImage(const std::string& file,const Image2us& img,int compressionLevel);
bool WriteTiffImage(const std::string& file,const Image3us& img,int compressionLevel);
bool WriteTiffImage(const std::string& file,const Image4us& img,int compressionLevel);
bool WriteTiffImage(const std::string& file,const Image1ui& img,int compressionLevel);
bool WriteTiffImage(const std::string& file,const ImageRGB& img,int compressionLevel);
bool WriteTiffImage(const std::string& file,const ImageRGBA& img,int compressionLevel);
/*
bool WriteTiffImage(const std::string& file,const Image1f& img,int compressionLevel);
bool WriteTiffImage(const std::string& file,const ImageRGBf& img,int compressionLevel);
bool WriteTiffImage(const std::string& file,const ImageRGBAf& img,int compressionLevel);
*/
}
#endif/*_GRFMT_TIFF_H_*/
