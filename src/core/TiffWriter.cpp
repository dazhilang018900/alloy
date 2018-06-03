#include "TiffWriter.h"
#include "tiff.h"
#include "tiffio.h"
namespace aly {

#ifdef TIFF_BIGENDIAN
template<class T, int C, ImageType I> bool WriteTiffImageInternal(const std::string& name, const Image<T, C, I>& img,int compressionLevel=9) {
	const int m_alpha_channel=(C==4)?1:0;
	int m_rowsperstrip = 64;
	while(img.height%m_rowsperstrip!=0){
		m_rowsperstrip>>=1;
	}
	// Check for things this format doesn't support
	if (img.width < 1 || img.height < 1) {
		std::runtime_error("Image resolution must be at least 1x1");
		return false;
	}
	TIFF* m_tif = nullptr;
	// Open the file
#ifdef _WIN32
	std::wstring wname = Strutil::utf8_to_utf16 (name);
	m_tif = TIFFOpenW (wname.c_str(),"w");
#else
	m_tif = TIFFOpen(name.c_str(), "w");
#endif
	if (!m_tif) {
		throw std::runtime_error(MakeString() << "Can't open " << name);
		return false;
	}
	TIFFSetField(m_tif, TIFFTAG_IMAGEWIDTH, img.width);
	TIFFSetField(m_tif, TIFFTAG_IMAGELENGTH, img.height);
	TIFFSetField(m_tif, TIFFTAG_ROWSPERSTRIP, m_rowsperstrip);
	TIFFSetField(m_tif, TIFFTAG_SAMPLESPERPIXEL, C);
	const int orientation= ORIENTATION_TOPLEFT;
	TIFFSetField(m_tif, TIFFTAG_ORIENTATION, orientation);
	int m_bitspersample =sizeof(T)*8;
	int sampformat;
	switch (I) {
	case ImageType::BYTE:
		m_bitspersample = 8;
		sampformat = SAMPLEFORMAT_INT;
		break;
	case ImageType::UBYTE:
		if (m_bitspersample != 2 && m_bitspersample != 4)
			m_bitspersample = 8;
		sampformat = SAMPLEFORMAT_UINT;
		break;
	case ImageType::SHORT:
		m_bitspersample = 16;
		sampformat = SAMPLEFORMAT_INT;
		break;
	case ImageType::USHORT:
		if (m_bitspersample != 10 && m_bitspersample != 12)
			m_bitspersample = 16;
		sampformat = SAMPLEFORMAT_UINT;
		break;
	case ImageType::INT:
		m_bitspersample = 32;
		sampformat = SAMPLEFORMAT_INT;
		break;
	case ImageType::UINT:
		m_bitspersample = 32;
		sampformat = SAMPLEFORMAT_UINT;
		break;
	case ImageType::FLOAT:
		m_bitspersample = 32;
		sampformat = SAMPLEFORMAT_IEEEFP;
		break;
	case ImageType::DOUBLE:
		m_bitspersample = 64;
		sampformat = SAMPLEFORMAT_IEEEFP;
		break;
	default:
		throw std::runtime_error("Image type unsupported.");
		break;
	}
	TIFFSetField(m_tif, TIFFTAG_BITSPERSAMPLE, m_bitspersample);
	TIFFSetField(m_tif, TIFFTAG_SAMPLEFORMAT, sampformat);
	int m_photometric = (C >= 3 ? PHOTOMETRIC_RGB : PHOTOMETRIC_MINISBLACK);
	int m_compression;
	if(compressionLevel<0){
		m_compression = COMPRESSION_LZW;
	} else if(compressionLevel==0){
		m_compression=  COMPRESSION_NONE;
	} else {
		m_compression = COMPRESSION_ADOBE_DEFLATE;
	}
	TIFFSetField(m_tif, TIFFTAG_COMPRESSION, m_compression);
	// Use predictor when using compression
	int planarconfig=PLANARCONFIG_CONTIG;
	int m_predictor = PREDICTOR_NONE;
	if (I == ImageType::FLOAT ||I == ImageType::DOUBLE) {
		m_predictor = PREDICTOR_FLOATINGPOINT;
		m_photometric = PHOTOMETRIC_MINISBLACK;
		// N.B. Very old versions of libtiff did not support this
		// predictor.  It's possible that certain apps can't read
		// floating point TIFFs with this set.  But since it's been
		// documented since 2005, let's take our chances.  Comment
		// out the above line if this is problematic.
	} else if (m_bitspersample == 8 || m_bitspersample == 16) {
		// predictors not supported for unusual bit depths (e.g. 10)
		m_predictor = PREDICTOR_HORIZONTAL;
	}
	if (m_predictor != PREDICTOR_NONE)
		TIFFSetField(m_tif, TIFFTAG_PREDICTOR, m_predictor);

	if (m_compression == COMPRESSION_ADOBE_DEFLATE) {
		TIFFSetField(m_tif, TIFFTAG_ZIPQUALITY, compressionLevel);
	}

	const int m_outputchans = C;
	TIFFSetField(m_tif, TIFFTAG_PHOTOMETRIC, m_photometric);
	// ExtraSamples tag
	if ((m_alpha_channel >= 0 || C > 3) && m_photometric != PHOTOMETRIC_SEPARATED) {
		int defaultchans = C >= 3 ? 3 : 1;
		short e = C - defaultchans;
		std::vector<unsigned short> extra(e);
		//what the hell does this do?
		for (int c = 0; c < e; ++c) {
			if (m_alpha_channel == (c + defaultchans)){
				extra[c] = EXTRASAMPLE_ASSOCALPHA;
			} else {
				extra[c] = EXTRASAMPLE_UNSPECIFIED;
			}
		}
		TIFFSetField(m_tif, TIFFTAG_EXTRASAMPLES, e, &extra[0]);
	}
	TIFFSetField(m_tif,TIFFTAG_PLANARCONFIG,planarconfig);
	TIFFSetField(m_tif, TIFFTAG_XPOSITION, 0);
	TIFFSetField(m_tif, TIFFTAG_YPOSITION, 0);
	int NS=(img.height%m_rowsperstrip==0)?img.height/m_rowsperstrip:img.height/m_rowsperstrip+1;
	size_t rowSize=img.width*m_rowsperstrip;
	std::vector<vec<T,C>> scratch(rowSize);
	auto bgn=img.data.begin();
	for(int strip=0;strip<NS;strip++){
		scratch.assign(bgn+strip*rowSize,std::min(bgn+(strip+1)*rowSize,img.data.end()));
		if(TIFFWriteEncodedStrip(m_tif, strip, scratch.data(),rowSize*img.typeSize()) <0){
			throw std::runtime_error(MakeString()<<"Could not write strip "<<strip);
		}
	}
	if (m_tif) {
		TIFFClose(m_tif);    // N.B. TIFFClose doesn't return a status code
	}
}

#else
template<class T, int C, ImageType I> bool WriteTiffImageInternal(const std::string& file,const Image<T, C, I>& img) {
	throw std::runtime_error("LibTIFF unavailable in this build.");
}
#endif

bool WriteTiffImage(const std::string& file, const Image1us& img,int compressionLevel) {
	return WriteTiffImageInternal(file, img,compressionLevel);
}
bool WriteTiffImage(const std::string& file, const Image2us& img,int compressionLevel) {
	return WriteTiffImageInternal(file, img,compressionLevel);
}
bool WriteTiffImage(const std::string& file, const Image1ui& img,int compressionLevel) {
	return WriteTiffImageInternal(file, img,compressionLevel);
}
bool WriteTiffImage(const std::string& file, const Image1ub& img,int compressionLevel) {
	return WriteTiffImageInternal(file, img,compressionLevel);
}
bool WriteTiffImage(const std::string& file, const ImageRGB& img,int compressionLevel) {
	return WriteTiffImageInternal(file, img,compressionLevel);
}
bool WriteTiffImage(const std::string& file, const ImageRGBA& img,int compressionLevel) {
	return WriteTiffImageInternal(file, img,compressionLevel);
}
bool WriteTiffImage(const std::string& file, const Image3us& img,int compressionLevel) {
	return WriteTiffImageInternal(file, img,compressionLevel);
}
bool WriteTiffImage(const std::string& file, const Image4us& img,int compressionLevel) {
	return WriteTiffImageInternal(file, img,compressionLevel);
}
/*
bool WriteTiffImage(const std::string& file, const Image1f& img,int compressionLevel) {
	return WriteTiffImageInternal(file, img,compressionLevel);
}
bool WriteTiffImage(const std::string& file, const ImageRGBf& img,int compressionLevel) {
	return WriteTiffImageInternal(file, img,compressionLevel);
}
bool WriteTiffImage(const std::string& file, const ImageRGBAf& img,int compressionLevel) {
	return WriteTiffImageInternal(file, img,compressionLevel);
}
*/
}
