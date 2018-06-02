
#ifndef TIFF_WRITER_H_
#define TIFF_WRITER_H_
#include <AlloyImage.h>
#include <string>
namespace aly {
bool WriteTiffImage(const std::string& file,const Image1ub& img);
bool WriteTiffImage(const std::string& file,const Image1us& img);
bool WriteTiffImage(const std::string& file,const Image3us& img);
bool WriteTiffImage(const std::string& file,const Image4us& img);
bool WriteTiffImage(const std::string& file,const Image1ui& img);
bool WriteTiffImage(const std::string& file,const ImageRGB& img);
bool WriteTiffImage(const std::string& file,const ImageRGBA& img);
bool WriteTiffImage(const std::string& file,const Image1f& img);
bool WriteTiffImage(const std::string& file,const ImageRGBf& img);
bool WriteTiffImage(const std::string& file,const ImageRGBAf& img);
}
#endif/*_GRFMT_TIFF_H_*/
