#include "AlloyImageFeatures.h"
#include <AlloyImageProcessing.h>
namespace aly {
const float Daisy::sigma_0 = 1.0f;
const float Daisy::sigma_1 = std::sqrt(2.0f);
const float Daisy::sigma_2 = 8.0f;
const float Daisy::sigma_init = 1.6f;
const float Daisy::sigma_step = (float) std::pow(2, 1.0f / 2);
const int Daisy::scale_st = int(
		(std::log(sigma_1 / sigma_0)) / (float) std::log(sigma_step));
void WriteOrientationImagesToFile(const std::string& file,
		const OrientationImages& img) {
	std::ostringstream vstr;
	std::string fileName = GetFileWithoutExtension(file);
	vstr << fileName << ".raw";
	FILE* f = fopen(vstr.str().c_str(), "wb");
	if (f == NULL) {
		throw std::runtime_error(
				MakeString() << "Could not open " << vstr.str().c_str()
						<< " for writing.");
	}
	int w = 0, h = 0;
	if (img.size() > 0) {
		w = img[0].width;
		h = img[0].height;
	}
	for (int k = 0; k < img.size(); k++) {
		for (int j = 0; j < h; j++) {
			for (int i = 0; i < w; i++) {
				float val = img[k](i, j);
				fwrite(&val, sizeof(float), 1, f);
			}
		}
	}
	fclose(f);
	std::string typeName = "Float";
	std::stringstream sstr;
	sstr << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	sstr << "<!-- MIPAV header file -->\n";
	sstr
			<< "<image xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" nDimensions=\"3\">\n";
	sstr << "	<Dataset-attributes>\n";
	sstr << "		<Image-offset>0</Image-offset>\n";
	sstr << "		<Data-type>" << typeName << "</Data-type>\n";
	sstr << "		<Endianess>Little</Endianess>\n";
	sstr << "		<Extents>" << img[0].width << "</Extents>\n";
	sstr << "		<Extents>" << img[0].height << "</Extents>\n";
	sstr << "		<Extents>" << img.size() << "</Extents>\n";
	sstr << "		<Resolutions>\n";
	sstr << "			<Resolution>1.0</Resolution>\n";
	sstr << "			<Resolution>1.0</Resolution>\n";
	sstr << "			<Resolution>1.0</Resolution>\n";
	sstr << "		</Resolutions>\n";
	sstr << "		<Slice-spacing>1.0</Slice-spacing>\n";
	sstr << "		<Slice-thickness>0.0</Slice-thickness>\n";
	sstr << "		<Units>Millimeters</Units>\n";
	sstr << "		<Units>Millimeters</Units>\n";
	sstr << "		<Units>Millimeters</Units>\n";
	sstr << "		<Compression>none</Compression>\n";
	sstr << "		<Orientation>Unknown</Orientation>\n";
	sstr << "		<Subject-axis-orientation>Unknown</Subject-axis-orientation>\n";
	sstr << "		<Subject-axis-orientation>Unknown</Subject-axis-orientation>\n";
	sstr << "		<Subject-axis-orientation>Unknown</Subject-axis-orientation>\n";
	sstr << "		<Origin>0.0</Origin>\n";
	sstr << "		<Origin>0.0</Origin>\n";
	sstr << "		<Origin>0.0</Origin>\n";
	sstr << "		<Modality>Unknown Modality</Modality>\n";
	sstr << "	</Dataset-attributes>\n";
	sstr << "</image>\n";
	std::ofstream myfile;
	std::stringstream xmlFile;
	xmlFile << fileName << ".xml";
	myfile.open(xmlFile.str().c_str(), std::ios_base::out);
	if (!myfile.is_open()) {
		throw std::runtime_error(
				MakeString() << "Could not open " << xmlFile.str()
						<< " for writing.");
	}
	myfile << sstr.str();
	myfile.close();
}
void Convolve(const ImageLayer& image, ImageLayer& out,
		const std::vector<float>& filter, int M, int N) {
	out.resize(image.width, image.height);
	int w = image.width;
	int h = image.height;
	out.resize(w, h);
#pragma omp parallel for
	for (int j = 0; j < image.height; j++) {
		for (int i = 0; i < image.width; i++) {
			float vsum(0.0);
			for (int ii = 0; ii < (int) M; ii++) {
				for (int jj = 0; jj < (int) N; jj++) {
					vsum += filter[ii + M * jj]
							* image(i + ii - (int) M / 2, j + jj - (int) N / 2);
				}
			}
			out[i + j * w] = vsum;
		}
	}
}
void Smooth(const ImageLayer & image, ImageLayer & out, float sigma) {
	int fsz = (int) (5 * sigma);
	if (fsz % 2 == 0)
		fsz++;
	if (fsz < 3)
		fsz = 3;
	std::vector<float> filter;
	GaussianKernel(filter, fsz, fsz, sigma, sigma);
	Convolve(image, out, filter, fsz, fsz);
}
Daisy::Daisy(int orientResolutions) :
		width(0), height(0), numberOfGridPoints(0), histogramBins(0), angleBins(
				0), radiusBins(0), descriptorRadius(0.0f), descriptorSize(0) {
}
void Daisy::getHistogram(float* histogram, int x, int y,
		const std::vector<ImageLayer>& hcube) const {
	for (int h = 0; h < histogramBins; h++) {
		histogram[h] = hcube[h](x, y);
	}
}
void Daisy::getHistogram(float* histogram, float x, float y,
		const std::vector<ImageLayer>& hcube) const {
	for (int h = 0; h < histogramBins; h++) {
		histogram[h] = hcube[h](x, y);
	}
}
void Daisy::getDescriptors(DaisyDescriptorField& field,DaisyNormalization normalizationType){
	field.resize(width,height);
#pragma omp parallel for
	for(int j=0;j<height;j++){
		for(int i=0;i<width;i++){
			getDescriptor(i,j,field(i,j),normalizationType,true);
			//std::cout<<i<<","<<j<<": Descriptor "<<field(i,j)<<std::endl;
		}
	}
}
void Daisy::getDescriptor(float x, float y, DaisyDescriptor& descriptor) const {
	descriptor.resize(descriptorSize);
	getHistogram(descriptor.data(), x, y, smoothLayers[selectedCubes[0]]);
	int r, rdt, region;
	for (r = 0; r < radiusBins; r++) {
		rdt = r * angleBins + 1;
		for (region = rdt; region < rdt + angleBins; region++) {
			float2 gpt = gridPoints[region];
			getHistogram(&descriptor[region*histogramBins], x + gpt.x, y + gpt.y,
					smoothLayers[selectedCubes[r]]);

		}
	}
}
void Daisy::getDescriptor(int x, int y, DaisyDescriptor& descriptor) const {
	descriptor.resize(descriptorSize);
	getHistogram(descriptor.data(), x, y, smoothLayers[selectedCubes[0]]);
	int r, rdt, region;
	for (r = 0; r < radiusBins; r++) {
		rdt = r * angleBins + 1;
		for (region = rdt; region < rdt + angleBins; region++) {
			float2 gpt = gridPoints[region];
			getHistogram(&descriptor[region*histogramBins], (int) aly::round(x + gpt.x),(int) aly::round(y + gpt.y),smoothLayers[selectedCubes[r]]);
		}
	}
}
void Daisy::initialize(const Image1f& image, float _descriptorRadius,
		int _radiusBins, int _angleBins, int _histogramBins,
		bool smoothFirstLayer) {
	width = image.width;
	height = image.height;
	angleBins = _angleBins;
	histogramBins = _histogramBins;
	radiusBins = _radiusBins;
	descriptorRadius = _descriptorRadius;
	numberOfGridPoints = angleBins * radiusBins + 1; // +1 is for center pixel
	descriptorSize = numberOfGridPoints * _histogramBins;
	computeCubeSigmas();
	computeGridPoints();
	initialize(image, smoothFirstLayer);
}
void Daisy::initialize(const ImageRGB& _image, float _descriptorRadius,
		int _radiusBins, int _angleBins, int _histogramBins,
		bool smoothFirstLayer) {
	Image1f image;
	ConvertImage(_image, image);
	initialize(image, _descriptorRadius, _radiusBins, _angleBins,
			_histogramBins, smoothFirstLayer);
}
void Daisy::initialize(const ImageRGBA& _image, float _descriptorRadius,
		int _radiusBins, int _angleBins, int _histogramBins,
		bool smoothFirstLayer) {
	Image1f image;
	ConvertImage(_image, image);
	initialize(image, _descriptorRadius, _radiusBins, _angleBins,
			_histogramBins, smoothFirstLayer);
}
void Daisy::initialize(const ImageRGBAf& _image, float _descriptorRadius,
		int _radiusBins, int _angleBins, int _histogramBins,
		bool smoothFirstLayer) {
	Image1f image;
	ConvertImage(_image, image);
	initialize(image, _descriptorRadius, _radiusBins, _angleBins,
			_histogramBins, smoothFirstLayer);
}
int Daisy::quantizeRadius(float rad) {
	if (rad <= sigmas[0])
		return 0;
	if (rad >= sigmas[sigmas.size() - 1])
		return (int) (sigmas.size() - 1);
	float dist;
	float mindist = std::numeric_limits<float>::max();
	int mini = 0;
	for (int c = 0; c < sigmas.size(); c++) {
		dist = std::abs(sigmas[c] - rad);
		if (dist < mindist) {
			mindist = dist;
			mini = c;
		}
	}
	return mini;
}

void Daisy::layeredGradient(const Image1f& image, OrientationImages& layers,
		int layer_no) const {
	ImageLayer bdata;
	layers.resize(layer_no);
	Image2f dxy(image.width, image.height);
#pragma omp parallel for
	for (int j = 0; j < image.height; j++) {
		for (int i = 0; i < image.width; i++) {
			float dx = 0.5f * (image(i + 1, j) - image(i - 1, j));
			float dy = 0.5f * (image(i, j + 1) - image(i, j - 1));
			dxy(i, j) = float2(dx, dy);
		}
	}
	for (int l = 0; l < layer_no; l++) {
		ImageLayer & layer_l = layers[l];
		layer_l.resize(image.width, image.height);
		float angle = 2.0f * l * ALY_PI / layer_no;
		float cosa = std::cos(angle);
		float sina = std::sin(angle);
#pragma omp parallel for
		for (int j = 0; j < image.height; j++) {
			for (int i = 0; i < image.width; i++) {
				float2 grad = dxy(i, j);
				float value = cosa * grad.x + sina * grad.y;
				if (value > 0) {
					layer_l(i, j) = value;
				} else {
					layer_l(i, j) = 0.0f;
				}
			}
		}
	}
}

void Daisy::computeSmoothedGradientLayers() {
	float sigma;
	for (int r = 1; r < smoothLayers.size(); r++) {
		smoothLayers[r].resize(histogramBins);
		for (ImageLayer & img : smoothLayers[r]) {
			img.resize(width, height);
		}
	}
	for (int r = 0; r < radiusBins; r++) {
		OrientationImages& prev_cube = smoothLayers[r];
		OrientationImages& cube = smoothLayers[r + 1];
		if (r == 0) {
			sigma = sigmas[0];
		} else {
			sigma = std::sqrt(
					sigmas[r] * sigmas[r] - sigmas[r - 1] * sigmas[r - 1]);
		}
		for (int th = 0; th < histogramBins; th++) {
			Smooth(prev_cube[th], cube[th], sigma);
		}
	}
}
void Daisy::initialize(const Image1f& image, bool smoothFirstLayer) {
	smoothLayers.resize(radiusBins + 1);
	layeredGradient(image, smoothLayers[0], histogramBins);
	float sigma = std::sqrt(sigma_init * sigma_init - 0.25f);
	if (smoothFirstLayer) {
		for (int i = 0; i < histogramBins; i++) {
			ImageLayer tmp = smoothLayers[0][i];
			Smooth(tmp, smoothLayers[0][i], sigma);
		}
	}
	computeSmoothedGradientLayers();
	for (int i = 0; i < smoothLayers.size(); i++) {
		std::vector<ImageLayer>& orientations = smoothLayers[i];
		int N = width * height;
		int K = orientations.size();
#pragma omp parallel for
		for (int n = 0; n < N; n++) {
			double sum = 0;
			for (int k = 0; k < K; k++) {
				sum += orientations[k][n];
			}
			if (sum > 1E-7f) {
				sum = 1.0 / sum;
				for (int k = 0; k < K; k++) {
					orientations[k][n] *= sum;
				}
			}
		}
	}
}
void Daisy::normalizeDescriptor(DaisyDescriptor& desc,
		DaisyNormalization nrm_type) const {
	if (nrm_type == DaisyNormalization::Partial)
		normalizePartial(desc);
	else if (nrm_type == DaisyNormalization::Full)
		normalizeFull(desc);
	else if (nrm_type == DaisyNormalization::Sift)
		normalizeSiftWay(desc);
}

void Daisy::normalizePartial(DaisyDescriptor& desc) const {
	float norm;
	for (int h = 0; h < numberOfGridPoints; h++) {
		norm = 0.0f;
		for (int i = 0; i < histogramBins; i++) {
			float val = desc[h * histogramBins + i];
			norm += val * val;
		}
		if (norm != 0.0) {
			norm = std::sqrt(norm);
			for (int i = 0; i < histogramBins; i++) {
				desc[h * histogramBins + i] /= norm;
			}
		}
	}
}
void Daisy::normalizeFull(DaisyDescriptor& desc) const {
	float norm = 0.0f;
	for (int i = 0; i < (int) desc.size(); i++) {
		float val = desc[i];
		norm += val * val;
	}
	if (norm != 0.0) {
		norm = std::sqrt(norm);
		for (int i = 0; i < (int) desc.size(); i++) {
			desc[i] /= norm;
		}
	}
}
void Daisy::normalizeSiftWay(DaisyDescriptor& desc) const {
	bool changed = true;
	int iter = 0;
	float norm;
	int h;
	const int MAX_NORMALIZATION_ITER = 5;
	const float m_descriptor_normalization_threshold = 0.154f; // sift magical number
	while (changed && iter < MAX_NORMALIZATION_ITER) {
		iter++;
		changed = false;
		norm = 0.0f;
		for (int i = 0; i < (int) desc.size(); i++) {
			float val = desc[i];
			norm += val * val;
		}
		norm = std::sqrt(norm);
		if (norm > 1e-5) {
			for (int i = 0; i < (int) desc.size(); i++) {
				desc[i] /= norm;
			}
		}
		for (h = 0; h < (int) desc.size(); h++) {
			if (desc[h] > m_descriptor_normalization_threshold) {
				desc[h] = m_descriptor_normalization_threshold;
				changed = true;
			}
		}
	}
}
void Daisy::getDescriptor(float x, float y, DaisyDescriptor& descriptor,
		DaisyNormalization normalizationType, bool disableInterpolation) const {
	if (disableInterpolation) {
		getDescriptor(int(x), int(y), descriptor);
	} else {
		getDescriptor(x, y, descriptor);
	}
	normalizeDescriptor(descriptor, normalizationType);
}
void Daisy::updateSelectedCubes() {
	selectedCubes.resize(radiusBins);
	for (int r = 0; r < radiusBins; r++) {
		float seed_sigma = (r + 1) * descriptorRadius / (radiusBins * 2.0f);
		selectedCubes[r] = quantizeRadius(seed_sigma);
	}
}
void Daisy::computeCubeSigmas() {
	sigmas.resize(radiusBins);
	float r_step = descriptorRadius / (float) radiusBins;
	for (int r = 0; r < radiusBins; r++) {
		sigmas[r] = (r + 1) * r_step / 2.0f;
	}
	updateSelectedCubes();
}

inline double lengthL2(const DaisyDescriptor& a) {
	double ret = 0.0;
	int N = (int) a.size();
	for (int i = 0; i < N; i++) {
		ret += a[i] * a[i];
	}
	return std::sqrt(ret);
}
double lengthSqr(const DaisyDescriptor& a) {
	double ret = 0.0;
	int N = (int) a.size();
	for (int i = 0; i < N; i++) {
		ret += a[i] * a[i];
	}
	return ret;
}
double lengthL1(const DaisyDescriptor& a) {
	double ret = 0.0;
	int N = (int) a.size();
	for (int i = 0; i < N; i++) {
		ret += std::abs(a[i]);
	}
	return ret;
}
double angle(const DaisyDescriptor& a, const DaisyDescriptor& b) {
	return std::acos(dot(a, b) / (lengthL2(a) * lengthL2(b)));
}
void Daisy::computeGridPoints() {
	double r_step = descriptorRadius / radiusBins;
	double t_step = 2 * ALY_PI / angleBins;
	gridPoints.resize(numberOfGridPoints, float2(0.0f, 0.0f));
	for (int r = 0; r < radiusBins; r++) {
		int region = r * angleBins + 1;
		for (int t = 0; t < angleBins; t++) {
			float x, y;
			gridPoints[region + t] = polar2cartesian((r + 1) * r_step,t * t_step);
		}
	}
}
}

