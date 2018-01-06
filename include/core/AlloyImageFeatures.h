#ifndef _IMAGEFEATURES_H
#define _IMAGEFEATURES_H
#include <AlloyImage.h>
#include <array>
#include <vector>
namespace aly {
enum class DaisyNormalization {UnNormalized = -1, Partial = 0, Full = 1, Sift = 2};
inline void cartesian2polar(const float2& pt, float &r, float &th) {
	r = length(pt);
	th = std::atan2(pt.y, pt.x);
}
inline float2 polar2cartesian(float r, float t) {
	return float2(r * std::cos(t), r * std::sin(t));
}
class DaisyDescriptor: public std::vector<float> {
public:
	DaisyDescriptor(size_t sz = 0) :
			std::vector<float>(sz) {
	}
	DaisyDescriptor(const std::vector<float>& data) :
			std::vector<float>(data.size()) {
		std::vector<float>::assign(data.begin(), data.end());
	}
};
class ImageLayer {
protected:
	std::vector<float> data;
public:
	int width, height;
	ImageLayer(int w = 0, int h = 0) :
			width(w), height(h) {

	}
	size_t size() const {
		return data.size();
	}
	void resize(int w, int h) {
		data.resize(w * h);
		data.shrink_to_fit();
		width = w;
		height = h;
	}
	inline void clear() {
		data.clear();
		data.shrink_to_fit();
		width = 0;
		height = 0;
	}
	inline float* ptr() {
		if (data.size() == 0)
			return nullptr;
		return data.data();
	}
	inline const float* ptr() const {
		if (data.size() == 0)
			return nullptr;
		return data.data();
	}
	inline const float& operator[](const size_t i) const {
		return data[i];
	}
	inline float& operator[](const size_t i) {
		return data[i];
	}
	inline float& operator()(int i, int j) {
		return data[clamp(i, 0, width - 1) + clamp(j, 0, height - 1) * width];
	}
	inline float& operator()(const int2 ij) {
		return data[clamp(ij.x, 0, width - 1)
				+ clamp(ij.y, 0, height - 1) * width];
	}
	inline const float& operator()(int i, int j) const {
		return data[clamp(i, 0, width - 1) + clamp(j, 0, height - 1) * width];
	}
	inline const float& operator()(const int2 ij) const {
		return data[clamp(ij.x, 0, width - 1)
				+ clamp(ij.y, 0, height - 1) * width];
	}
	void setZero() {
		data.assign(data.size(), 0.0f);
	}
	inline float operator()(float x, float y) const {
		int i = static_cast<int>(std::floor(x));
		int j = static_cast<int>(std::floor(y));
		const float rgb00 = operator()(i, j);
		const float rgb10 = operator()(i + 1, j);
		const float rgb11 = operator()(i + 1, j + 1);
		const float rgb01 = operator()(i, j + 1);
		float dx = x - i;
		float dy = y - j;
		return ((rgb00 * (1.0f - dx) + rgb10 * dx) * (1.0f - dy)
				+ (rgb01 * (1.0f - dx) + rgb11 * dx) * dy);
	}
	inline aly::float2 gradient(float x, float y) const {
		float v21 = (operator()(x + 1, y));
		float v12 = (operator()(x, y + 1));
		float v10 = (operator()(x, y - 1));
		float v01 = (operator()(x - 1, y));
		float dx = ((v21 - v01) * 0.5f);
		float dy = ((v12 - v10) * 0.5f);
		return float2(dx, dy);
	}
	inline aly::float2 gradient(int x, int y) const {
		float v21 = (operator()(x + 1, y));
		float v12 = (operator()(x, y + 1));
		float v10 = (operator()(x, y - 1));
		float v01 = (operator()(x - 1, y));
		float dx = ((v21 - v01) * 0.5f);
		float dy = ((v12 - v10) * 0.5f);
		return float2(dx, dy);
	}
};

typedef std::vector<ImageLayer> OrientationImages;
void WriteOrientationImagesToFile(const std::string& file,
		const OrientationImages& img);

inline double dot(const DaisyDescriptor& a, const DaisyDescriptor& b) {
	double ret = 0.0;
	int N = (int) std::min(a.size(), b.size());
	for (int i = 0; i < N; i++) {
		ret += a[i] * b[i];
	}
	return ret;
}
inline double lengthL2(const DaisyDescriptor& a);
inline double lengthSqr(const DaisyDescriptor& a);
inline double lengthL1(const DaisyDescriptor& a);
inline double angle(const DaisyDescriptor& a, const DaisyDescriptor& b);
void Smooth(const ImageLayer & image, ImageLayer & B, float sigma);
void Convolve(const ImageLayer& image, ImageLayer& out,
		const std::vector<float>& filter, int M, int N);

class DaisyDescriptorField {
protected:
	std::vector<DaisyDescriptor> data;
public:
	int width, height;
	DaisyDescriptorField(int w = 0, int h = 0) :
			data(w * h), width(w), height(h) {

	}
	size_t size() const {
		return data.size();
	}
	void resize(int w, int h) {
		data.resize(w * h);
		data.shrink_to_fit();
		width = w;
		height = h;
	}
	inline void clear() {
		data.clear();
		data.shrink_to_fit();
		width = 0;
		height = 0;
	}
	DaisyDescriptor* ptr() {
		if (data.size() == 0)
			return nullptr;
		return data.data();
	}
	const DaisyDescriptor* ptr() const {
		if (data.size() == 0)
			return nullptr;
		return data.data();
	}
	const DaisyDescriptor& operator[](const size_t i) const {
		return data[i];
	}
	DaisyDescriptor& operator[](const size_t i) {
		return data[i];
	}
	DaisyDescriptor& operator()(int i, int j) {
		return data[clamp(i, 0, width - 1) + clamp(j, 0, height - 1) * width];
	}
	DaisyDescriptor& operator()(const int2 ij) {
		return data[clamp(ij.x, 0, width - 1)
				+ clamp(ij.y, 0, height - 1) * width];
	}
	const DaisyDescriptor& operator()(int i, int j) const {
		return data[clamp(i, 0, width - 1) + clamp(j, 0, height - 1) * width];
	}
	const DaisyDescriptor& operator()(const int2 ij) const {
		return data[clamp(ij.x, 0, width - 1)
				+ clamp(ij.y, 0, height - 1) * width];
	}
	void get(float x, float y, DaisyDescriptor& out) const {
		int i = static_cast<int>(std::floor(x));
		int j = static_cast<int>(std::floor(y));
		const DaisyDescriptor& rgb00 = operator()(i, j);
		const DaisyDescriptor& rgb10 = operator()(i + 1, j);
		const DaisyDescriptor& rgb11 = operator()(i + 1, j + 1);
		const DaisyDescriptor& rgb01 = operator()(i, j + 1);
		float dx = x - i;
		float dy = y - j;
		int N = (int) rgb00.size();
		out.resize(N);
		for (int i = 0; i < N; i++) {
			out[i] = ((rgb00[i] * (1.0f - dx) + rgb10[i] * dx) * (1.0f - dy)
					+ (rgb01[i] * (1.0f - dx) + rgb11[i] * dx) * dy);
		}
	}
	DaisyDescriptor operator()(float x, float y) const {
		int i = static_cast<int>(std::floor(x));
		int j = static_cast<int>(std::floor(y));
		const DaisyDescriptor& rgb00 = operator()(i, j);
		const DaisyDescriptor& rgb10 = operator()(i + 1, j);
		const DaisyDescriptor& rgb11 = operator()(i + 1, j + 1);
		const DaisyDescriptor& rgb01 = operator()(i, j + 1);
		float dx = x - i;
		float dy = y - j;
		int N = (int) rgb00.size();
		DaisyDescriptor out(N);
		for (int i = 0; i < N; i++) {
			out[i] = ((rgb00[i] * (1.0f - dx) + rgb10[i] * dx) * (1.0f - dy)
					+ (rgb01[i] * (1.0f - dx) + rgb11[i] * dx) * dy);
		}
		return out;
	}
};
template<class L, class R> std::basic_ostream<L, R> & operator <<(
		std::basic_ostream<L, R> & ss, const DaisyDescriptor& A) {
	size_t index = 0;
	size_t N = A.size();
	ss << "[";
	for (size_t index = 0; index < N; index++) {
		if (index < N - 1) {
			ss << A[index] << ",";
		} else {
			ss << A[index] << "]";
		}
	}
	return ss;
}
typedef std::vector<DaisyDescriptor> DaisyDescriptors;
class Daisy {
protected:
	static const float sigma_0;
	static const float sigma_1;
	static const float sigma_2;
	static const float sigma_init;
	static const float sigma_step;
	static const int scale_st;
	std::vector<float2> gridPoints;
	std::vector<OrientationImages> smoothLayers;
	std::vector<int> selectedCubes;
	std::vector<float> sigmas;
	int numberOfGridPoints;
	int descriptorSize;
	float descriptorRadius;
	int radiusBins;
	int angleBins;
	int histogramBins;
	void computeCubeSigmas();
	void computeGridPoints();
	void updateSelectedCubes();
	void computeOrientedGridPoints();
	void computeSmoothedGradientLayers();
	void layeredGradient(const Image1f& image, OrientationImages& layers,
			int layer_no = 8) const;
	int quantizeRadius(float rad);
	void normalizeSiftWay(DaisyDescriptor& desc) const;
	void normalizePartial(DaisyDescriptor& desc) const;
	void normalizeFull(DaisyDescriptor& desc) const;
	void normalizeDescriptor(DaisyDescriptor& desc,
			DaisyNormalization nrm_type) const;
	void getDescriptor(float x, float y, DaisyDescriptor& descriptor) const;
	void getDescriptor(int x, int y, DaisyDescriptor& descriptor) const;
	void getHistogram(float* histogram, int x, int y,
			const std::vector<ImageLayer>& hcube) const;
	void getHistogram(float* histogram, float x, float y,
			const std::vector<ImageLayer>& hcube) const;
	void initialize(const Image1f& image, bool smoothFirstLayer);
	void initialize(const Image1f& image, float descriptorRadius,
			int radiusBins, int angleBins, int histogramBins,
			bool smoothFirstLayer);
public:
	int width, height;
	bool isInitialized() const {
		return (width != 0 && height != 0);
	}
	Daisy(int orientationResolutions = 8);
	void initialize(const ImageRGB& image, float descriptorRadius = 15.0f,
			int radiusBins = 3, int angleBins = 8, int histogramBins = 8,
			bool smoothFirstLayer = true);
	void initialize(const ImageRGBAf& image, float descriptorRadius = 15.0f,
			int radiusBins = 3, int angleBins = 8, int histogramBins = 8,
			bool smoothFirstLayer = true);
	void initialize(const ImageRGBA& image, float descriptorRadius = 15.0f,
			int radiusBins = 3, int angleBins = 8, int histogramBins = 8,
			bool smoothFirstLayer = true);
	void getDescriptor(float i, float j, DaisyDescriptor& out,
			DaisyNormalization normalizationType,
			bool disableInterpolation = false) const;
	inline void getDescriptor(float2 pix, DaisyDescriptor& out,
			DaisyNormalization normalizationType,
			bool disableInterpolation = false) const {
		getDescriptor(pix.x, pix.y, out, normalizationType,
				disableInterpolation);
	}
	void getDescriptors(DaisyDescriptorField& field,DaisyNormalization normalizationType);
};
}
#endif
