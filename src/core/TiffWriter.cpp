#include "TiffWriter.h"
#include "tiff.h"
#include "tiffio.h"
namespace aly {

#ifdef TIFF_BIGENDIAN
template<class T, int C, ImageType I> bool WriteTiffImageInternal(const std::string& name, const Image<T, C, I>& img,int compressionLevel=9) {
	const int m_alpha_channel=(C==4)?1:0;
	const int m_rowsperstrip = 32;
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
	const int m_photometric = (C >= 3 ? PHOTOMETRIC_RGB : PHOTOMETRIC_MINISBLACK);
	const int m_compression = COMPRESSION_ADOBE_DEFLATE; //could also use COMPRESSION_LZW, but there's a patent.
	TIFFSetField(m_tif, TIFFTAG_COMPRESSION, m_compression);
	// Use predictor when using compression
	int m_predictor = PREDICTOR_NONE;
	if (I == ImageType::FLOAT ||I == ImageType::DOUBLE) {
		m_predictor = PREDICTOR_FLOATINGPOINT;
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
	TIFFSetField(m_tif,TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);
	TIFFSetField(m_tif, TIFFTAG_XPOSITION, 0);
	TIFFSetField(m_tif, TIFFTAG_YPOSITION, 0);
	std::vector<vec<T,C>> scratch=img.data;
    if(TIFFWriteEncodedStrip(m_tif, 0, scratch.data(), img.size()*img.typeSize()) == -1){
    	throw std::runtime_error("Could not write scanlines");
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

bool WriteTiffImage(const std::string& file, const Image1us& img) {
	return WriteTiffImageInternal(file, img);
}
bool WriteTiffImage(const std::string& file, const Image2us& img) {
	return WriteTiffImageInternal(file, img);
}
bool WriteTiffImage(const std::string& file, const Image1ui& img) {
	return WriteTiffImageInternal(file, img);
}
bool WriteTiffImage(const std::string& file, const Image1ub& img) {
	return WriteTiffImageInternal(file, img);
}
bool WriteTiffImage(const std::string& file, const ImageRGB& img) {
	return WriteTiffImageInternal(file, img);
}
bool WriteTiffImage(const std::string& file, const ImageRGBA& img) {
	return WriteTiffImageInternal(file, img);
}
bool WriteTiffImage(const std::string& file, const Image3us& img) {
	return WriteTiffImageInternal(file, img);
}
bool WriteTiffImage(const std::string& file, const Image4us& img) {
	return WriteTiffImageInternal(file, img);
}
bool WriteTiffImage(const std::string& file, const Image1f& img) {
	return WriteTiffImageInternal(file, img);
}
bool WriteTiffImage(const std::string& file, const ImageRGBf& img) {
	return WriteTiffImageInternal(file, img);
}
bool WriteTiffImage(const std::string& file, const ImageRGBAf& img) {
	return WriteTiffImageInternal(file, img);
}

}
