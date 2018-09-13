/*
 * Copyright (C) 2015, Simon Fuhrmann
 * TU Darmstadt - Graphics, Capture and Massively Parallel Computing
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD 3-Clause license. See the LICENSE.txt file for details.
 */

#include <iostream>
#include <fstream>
#include <stdexcept>
#include "image/AlloyImageProcessing.h"
#include <omp.h>
#include "vision/Sift.h"
#define MATH_POW2(x) ((x)*(x))
/* Floating-point epsilon comparisons. */
#define MATH_EPSILON_EQ(x,v,eps) (((v - eps) <= x) && (x <= (v + eps)))
#define MATH_EPSILON_LESS(x,v,eps) ((x + eps) < v)
#define MATH_FLOAT_EQ(x,v) MATH_EPSILON_EQ(x,v,MATH_FLT_EPS)
#define MATH_DOUBLE_EQ(x,v) MATH_EPSILON_EQ(x,v,MATH_DBL_EPS)
#define MATH_FLOAT_LESS(x,v) MATH_EPSILON_LESS(x,v,MATH_FLT_EPS)
#define MATH_DOUBLE_LESS(x,v) MATH_EPSILON_LESS(x,v,MATH_DBL_EPS)
#define MATH_SQRT2      1.41421356237309504880168872420969808   // sqrt(2)

/**
 * Gaussian function that expects x to be squared.
 * g(x) = exp( -1/2 * xx / sigma^2 ).
 * Gaussian with bell height y=1, bell center x=0 and bell "width" sigma.
 * Useful for at least float and double types.
 */
inline double gaussian_xx(double const& xx, double const& sigma) {
	return std::exp(-(xx / (2 * sigma * sigma)));
}
namespace aly{
Sift::Sift(	SiftOptions options):options(options) {
}

/* ---------------------------------------------------------------- */

void Sift::solve(bool generateDescriptors) {
	if (this->options.minOctave < -1
			|| this->options.minOctave > this->options.maxOctave)
		throw std::invalid_argument("Invalid octave range");
	if (this->options.contrastThreshold < 0.0f)
		this->options.contrastThreshold = 0.02f
				/ static_cast<float>(this->options.samplesPerOctave);
	this->create();
	this->extremaDetection();
	this->keypointLocalization();
	if(generateDescriptors)this->descriptorGeneration();
}

/* ---------------------------------------------------------------- */

void Sift::create(void) {
	this->octaves.clear();
	this->octaves.reserve(options.maxOctave-options.minOctave);
	/*
	 * Create octave -1. The original image is assumed to have blur
	 * sigma = 0.5. The double size image therefore has sigma = 1.
	 */
	if (this->options.minOctave < 0) {
		aly::Image1f img;
		aly::UpSample(orig, img);
		this->add(img, this->options.inherentBlurSigma * 2.0f,
				this->options.baseBlurSigma);
	}

	/*
	 * Prepare image for the first positive octave by downsampling.
	 * This code is executed only if min_octave > 0.
	 */
	aly::Image1f img = this->orig;
	aly::Image1f tmp;
	for (int i = 0; i < this->options.minOctave; ++i) {
		aly::DownSample3x3(img, tmp);
		img = tmp;
	}
	/*
	 * Create new octave from 'img', then subsample octave image where
	 * sigma is doubled to get a new base image for the next octave.
	 */
	float img_sigma = this->options.inherentBlurSigma;
	for (int i = std::max(0, this->options.minOctave);i <= this->options.maxOctave; ++i) {
		this->add(img, img_sigma, this->options.baseBlurSigma);
		aly::DownSample3x3(img, tmp);
		img = tmp;
		img_sigma = this->options.baseBlurSigma;
	}
}

/* ---------------------------------------------------------------- */

void Sift::add(const Image1f& image, float has_sigma, float target_sigma) {
	aly::Image1f base;
	float sigma = std::sqrt(MATH_POW2(target_sigma) - MATH_POW2(has_sigma));
	this->octaves.push_back(std::shared_ptr<Octave>(new Octave()));
	OctavePtr oct = this->octaves.back();
	oct->gray.resize(this->options.samplesPerOctave + 3);
	oct->dog.resize(this->options.samplesPerOctave + 2);
	if (target_sigma > has_sigma) {
		aly::Smooth(image, base, target_sigma);
	} else {
		base = image;
	}
	oct->gray[0]=base;
	float const k = std::pow(2.0f, 1.0f / this->options.samplesPerOctave);
	sigma = target_sigma;
	for (int i = 1; i < this->options.samplesPerOctave + 3; ++i) {
		/* Calculate the blur sigma the image will get. */
		float sigmak = sigma * k;
		float blur_sigma = std::sqrt(MATH_POW2(sigmak) - MATH_POW2(sigma));
		aly::Smooth(base, oct->gray[i], blur_sigma);
		oct->dog[i-1]=oct->gray[i] - base;
		base = oct->gray[i];
		sigma = sigmak;
		//WriteImageToFile(MakeString()<<GetDesktopDirectory()<<ALY_PATH_SEPARATOR<<"sift_dog"<<this->octaves.size()<<"_"<<i-1<<".xml",oct->dog[i-1]);
	}
}

/* ---------------------------------------------------------------- */

void Sift::extremaDetection(void) {
	/* Delete previous keypoints. */
	this->keypoints.clear();
	/* Detect keypoints in each octave... */
	for (std::size_t i = 0; i < this->octaves.size(); ++i) {
		OctavePtr oct=this->octaves[i];
		/* In each octave, take three subsequent DoG images and detect. */
		for (int s = 0; s < (int) oct->dog.size() - 2; ++s) {
			const aly::Image1f* samples[3] = { &oct->dog[s + 0], &oct->dog[s + 1],&oct->dog[s + 2] };
			this->extremaDetection(samples,static_cast<int>(i) + this->options.minOctave, s);
		}
	}
}

/* ---------------------------------------------------------------- */

std::size_t Sift::extremaDetection(const aly::Image1f* s[3], int oi, int si) {
	int const w = s[1]->width;
	int const h = s[1]->height;
	const int noff[9] = { -1 - w, 0 - w, 1 - w, -1, 0, 1, -1 + w, 0 + w, 1 + w };
	int detected = 0;
	int off = w;
	for (int y = 1; y < h - 1; ++y, off += w){
		for (int x = 1; x < w - 1; ++x) {
			int idx = off + x;
			bool largest = true;
			bool smallest = true;
			float center_value = (*s[1])[idx].x;
			for (int l = 0; (largest || smallest) && l < 3; ++l)
				for (int i = 0; (largest || smallest) && i < 9; ++i) {
					if (l == 1 && i == 4) // Skip center pixel
						continue;
					if ((*s[l])[idx + noff[i]].x >= center_value)
						largest = false;
					if ((*s[l])[idx + noff[i]].x <= center_value)
						smallest = false;
				}

			/* Skip non-maximum values. */
			if (!smallest && !largest)
				continue;
			/* Yummy. Add detected scale space extremum. */
			SiftKeypoint kp;
			kp.octave = oi;
			kp.x = static_cast<float>(x);
			kp.y = static_cast<float>(y);
			kp.sample = static_cast<float>(si);
			this->keypoints.push_back(kp);
			detected += 1;
		}
	}
	return detected;
}

/* ---------------------------------------------------------------- */

void Sift::keypointLocalization(void) {
	/*
	 * Iterate over all keypoints, accurately localize minima and maxima
	 * in the DoG function by fitting a quadratic Taylor polynomial
	 * around the keypoint.
	 */

	int num_singular = 0;
	int num_keypoints = 0; // Write iterator
	for (std::size_t i = 0; i < this->keypoints.size(); ++i) {
		SiftKeypoint kp=this->keypoints[i];
		OctavePtr oct=this->octaves[kp.octave - this->options.minOctave];
		int sample = static_cast<int>(kp.sample);
		const aly::Image1f* dogs[3] = { &oct->dog[sample + 0], &oct->dog[sample+ 1], &oct->dog[sample + 2] };
		int const w = dogs[0]->width;
		int const h = dogs[0]->height;
		int ix = static_cast<int>(kp.x);
		int iy = static_cast<int>(kp.y);
		int is = static_cast<int>(kp.sample);
		float fx, fy, fs;
		float Dx, Dy, Ds;
		float Dxx, Dyy, Dss;
		float Dxy, Dxs, Dys;
		/*
		 * Locate the keypoint using second order Taylor approximation.
		 * The procedure might get iterated around a neighboring pixel if
		 * the accurate keypoint is off by >0.6 from the center pixel.
		 */
		float score_thres = MATH_POW2(this->options.edgeRatioThreshold + 1.0f)/ this->options.edgeRatioThreshold;
#       define AT(S,OFF) ((*dogs[S])[px + OFF].x)
		for (int j = 0; j < 5; ++j) {
			std::size_t px = iy * w + ix;

			/* Compute first and second derivatives. */
			Dx = (AT(1,1) - AT(1, -1)) * 0.5f;
			Dy = (AT(1,w) - AT(1, -w)) * 0.5f;
			Ds = (AT(2,0) - AT(0, 0)) * 0.5f;

			Dxx = AT(1,1) + AT(1, -1) - 2.0f * AT(1, 0);
			Dyy = AT(1,w) + AT(1, -w) - 2.0f * AT(1, 0);
			Dss = AT(2,0) + AT(0, 0) - 2.0f * AT(1, 0);

			Dxy = (AT(1,1+w) + AT(1, -1-w) - AT(1, -1+w) - AT(1, 1-w)) * 0.25f;
			Dxs = (AT(2,1) + AT(0, -1) - AT(2, -1) - AT(0, 1)) * 0.25f;
			Dys = (AT(2,w) + AT(0, -w) - AT(2, -w) - AT(0, w)) * 0.25f;

			/* Setup the Hessian matrix. */
			float3x3 A(Dxx, Dxy, Dxs, Dxy, Dyy, Dys, Dxs, Dys, Dss);
			try {
				/* Invert the matrix to get the accurate keypoint. */
				A = inverse(A);
				float3 b(-Dx, -Dy, -Ds);
				b = A * b;
				fx = b[0];
				fy = b[1];
				fs = b[2];
			} catch (...) {
				num_singular += 1;
				fx = fy = fs = 0.0f; // FIXME: Handle this case?
				break;
			}

			/* Check if accurate location is far away from pixel center. */
			int dx = (fx > 0.6f && ix < w - 2) * 1
					+ (fx < -0.6f && ix > 1) * -1;
			int dy = (fy > 0.6f && iy < h - 2) * 1
					+ (fy < -0.6f && iy > 1) * -1;

			/* If the accurate location is closer to another pixel,
			 * repeat localization around the other pixel. */
			if (dx != 0 || dy != 0) {
				ix += dx;
				iy += dy;
				continue;
			}
			/* Accurate location looks good. */
			break;
		}

		/* Calcualte function value D(x) at accurate keypoint x. */
		float val = (*dogs[1])(ix, iy).x + 0.5f * (Dx * fx + Dy * fy + Ds * fs);
		/* Calcualte edge response score Tr(H)^2 / Det(H), see Section 4.1. */
		float hessian_trace = Dxx + Dyy;
		float hessian_det = Dxx * Dyy - MATH_POW2(Dxy);
		float hessian_score = MATH_POW2(hessian_trace) / hessian_det;


		/*
		 * Set accurate final keypoint location.
		 */
		kp.x = (float) ix + fx;
		kp.y = (float) iy + fy;
		kp.sample = (float) is + fs;
		/*
		 * Discard keypoints with:
		 * 1. low contrast (value of DoG function at keypoint),
		 * 2. negative hessian determinant (curvatures with different sign),
		 *    Note that negative score implies negative determinant.
		 * 3. large edge response (large hessian score),
		 * 4. unstable keypoint accurate locations,
		 * 5. keypoints beyond the scale space boundary.
		 */
		if (std::abs(val) < this->options.contrastThreshold
				|| hessian_score < 0.0f || hessian_score > score_thres
				|| std::abs(fx) > 1.5f || std::abs(fy) > 1.5f
				|| std::abs(fs) > 1.0f || kp.sample < -1.0f
				|| kp.sample > (float) this->options.samplesPerOctave
				|| kp.x < 0.0f || kp.x > (float) (w - 1) || kp.y < 0.0f
				|| kp.y > (float) (h - 1)) {
			continue;
		}
		/* Keypoint is accepted, copy to write iter and advance. */
		this->keypoints[num_keypoints] = kp;
		num_keypoints += 1;
	}
	this->keypoints.erase(this->keypoints.begin()+num_keypoints,this->keypoints.end());//resize(num_keypoints);
}

/* ---------------------------------------------------------------- */
const SiftDescriptors& Sift::getDescriptors() const {
	return descriptors;
}
void Sift::descriptorGeneration(void) {
	if (this->octaves.empty())
		throw std::runtime_error("Octaves not available!");
	if (this->keypoints.empty())
		return;
	this->descriptors.clear();
	this->descriptors.reserve(this->keypoints.size() * 3 / 2);
	/*
	 * Keep a buffer of S+3 gradient and orientation images for the current
	 * octave. Once the octave is changed, these images are recomputed.
	 * To ensure efficiency, the octave index must always increase, never
	 * decrease, which is enforced during the algorithm.
	 */
	//int octave_index = this->keypoints[0].octave;
	for(int n=0;n<(int)octaves.size();n++){
		this->generateFeatureImages(octaves[n].get());
	}
	/* Walk over all keypoints and compute descriptors. */
#pragma omp parallel for
	for (int i = 0; i < this->keypoints.size(); ++i) {
		const SiftKeypoint& kp=this->keypoints[i];
		Octave* octave = this->octaves[kp.octave - this->options.minOctave].get();
		std::vector<float> orientations;
		orientations.reserve(8);
		this->orientationAssignment(kp, octave, orientations);
		/* Feature vector extraction. */
		for (std::size_t j = 0; j < orientations.size(); ++j) {
			SiftDescriptor desc;
			float const scale_factor = std::pow(2.0f, kp.octave);
			desc.x = scale_factor * (kp.x + 0.5f) - 0.5f;
			desc.y = scale_factor * (kp.y + 0.5f) - 0.5f;
			desc.scale = this->keypointAbsoluteScale(kp);
			desc.orientation = orientations[j];
			//std::cout<<"Descriptor "<<desc.x<<" "<<desc.y<<" "<<desc.scale<<" "<<desc.orientation<<std::endl;
			if (this->descriptorAssignment(kp, desc, octave)){
#pragma omp critical
				{
					this->descriptors.push_back(desc);
				}
			}
		}
	}
}

/* ---------------------------------------------------------------- */

void Sift::generateFeatureImages(Octave* octave) {
	int const width = octave->gray[0].width;
	int const height = octave->gray[0].height;
	octave->gradient.resize(octave->gray.size(),Image1f(width,height));
	octave->orientation.resize(octave->gray.size(),Image1f(width,height));
	//std::cout << "Generating gradient and orientation images..." << std::endl;
	for (std::size_t i = 0; i < octave->gray.size(); ++i) {
		aly::Image1f& img = octave->gray[i];
		aly::Image1f& grad=octave->gradient[i];
		aly::Image1f& ori=octave->orientation[i];
		int image_iter = width + 1;
		for (int y = 1; y < height - 1; ++y, image_iter += 2){
			for (int x = 1; x < width - 1; ++x, ++image_iter) {
				float m1x = img[image_iter - 1];
				float p1x = img[image_iter + 1];
				float m1y = img[image_iter - width];
				float p1y = img[image_iter + width];
				float dx = 0.5f * (p1x - m1x);
				float dy = 0.5f * (p1y - m1y);
				float atan2f = std::atan2(dy, dx);
				grad[image_iter].x = std::sqrt(dx * dx + dy * dy);
				ori[image_iter].x =atan2f < 0.0f ? atan2f + ALY_PI * 2.0f : atan2f;
			}
		}
	}
}

/* ---------------------------------------------------------------- */

void Sift::orientationAssignment(SiftKeypoint const& kp, Octave const* octave,
		std::vector<float>& orientations) {
	int const nbins = 36;
	float const nbinsf = static_cast<float>(nbins);

	/* Prepare 36-bin histogram. */
	float hist[nbins];
	std::fill(hist, hist + nbins, 0.0f);

	/* Integral x and y coordinates and closest scale sample. */
	int const ix = static_cast<int>(kp.x + 0.5f);
	int const iy = static_cast<int>(kp.y + 0.5f);
	int const is = static_cast<int>(aly::round(kp.sample));
	float const sigma = this->keypointRelativeScale(kp);

	/* Images with its dimension for the keypoint. */
	const aly::Image1f& grad = octave->gradient[is + 1];
	const aly::Image1f& ori = octave->orientation[is + 1];
	int const width = grad.width;
	int const height = grad.height;

	/*
	 * Compute window size 'win', the full window has  2 * win + 1  pixel.
	 * The factor 3 makes the window large enough such that the gaussian
	 * has very little weight beyond the window. The value 1.5 is from
	 * the SIFT paper. If the window goes beyond the image boundaries,
	 * the keypoint is discarded.
	 */
	float const sigma_factor = 1.5f;
	int win = static_cast<int>(sigma * sigma_factor * 3.0f);
	if (ix < win || ix + win >= width || iy < win || iy + win >= height)
		return;

	/* Center of keypoint index. */
	int center = iy * width + ix;
	float const dxf = kp.x - static_cast<float>(ix);
	float const dyf = kp.y - static_cast<float>(iy);
	float const maxdist = static_cast<float>(win * win) + 0.5f;

	/* Populate histogram over window, intersected with (1,1), (w-2,h-2). */
	for (int dy = -win; dy <= win; ++dy) {
		int const yoff = dy * width;
		for (int dx = -win; dx <= win; ++dx) {
			/* Limit to circular window (centered at accurate keypoint). */
			float const dist = MATH_POW2(dx - dxf) + MATH_POW2(dy - dyf);
			if (dist > maxdist)
				continue;

			float gm = grad[center + yoff + dx].x; // gradient magnitude
			float go = ori[center + yoff + dx].x; // gradient orientation
			float weight = (float)gaussian_xx(dist, sigma * sigma_factor);
			int bin = static_cast<int>(nbinsf * go / (2.0f * ALY_PI));
			bin = aly::clamp(bin, 0, nbins - 1);
			hist[bin] += gm * weight;
		}
	}

	/* Smooth histogram. */
	for (int i = 0; i < 6; ++i) {
		float first = hist[0];
		float prev = hist[nbins - 1];
		for (int j = 0; j < nbins - 1; ++j) {
			float current = hist[j];
			hist[j] = (prev + current + hist[j + 1]) / 3.0f;
			prev = current;
		}
		hist[nbins - 1] = (prev + hist[nbins - 1] + first) / 3.0f;
	}

	/* Find maximum element. */
	float maxh = *std::max_element(hist, hist + nbins);

	/* Find peaks within 80% of max element. */
	for (int i = 0; i < nbins; ++i) {
		float h0 = hist[(i + nbins - 1) % nbins];
		float h1 = hist[i];
		float h2 = hist[(i + 1) % nbins];

		/* These peaks must be a local maximum! */
		if (h1 <= 0.8f * maxh || h1 <= h0 || h1 <= h2)
			continue;

		/*
		 * Quadratic interpolation to find accurate maximum.
		 * f(x) = ax^2 + bx + c, f(-1) = h0, f(0) = h1, f(1) = h2
		 * --> a = 1/2 (h0 - 2h1 + h2), b = 1/2 (h2 - h0), c = h1.
		 * x = f'(x) = 2ax + b = 0 --> x = -1/2 * (h2 - h0) / (h0 - 2h1 + h2)
		 */
		float x = -0.5f * (h2 - h0) / (h0 - 2.0f * h1 + h2);
		float o = 2.0f * ALY_PI * (x + (float) i + 0.5f) / nbinsf;
		orientations.push_back(o);
	}
}

/* ---------------------------------------------------------------- */

bool Sift::descriptorAssignment(SiftKeypoint const& kp, SiftDescriptor& desc,
		Octave const* octave) {
	/*
	 * The final feature vector has size PXB * PXB * OHB.
	 * The following constants should not be changed yet, as the
	 * (PXB^2 * OHB = 128) element feature vector is still hard-coded.
	 */
	//int const PIX = 16; // Descriptor region with 16x16 pixel
	int const PXB = 4; // Pixel bins with 4x4 bins
	int const OHB = 8; // Orientation histogram with 8 bins

	/* Integral x and y coordinates and closest scale sample. */
	int const ix = static_cast<int>(kp.x + 0.5f);
	int const iy = static_cast<int>(kp.y + 0.5f);
	int const is = static_cast<int>(aly::round(kp.sample));
	float const dxf = kp.x - static_cast<float>(ix);
	float const dyf = kp.y - static_cast<float>(iy);
	float const sigma = this->keypointRelativeScale(kp);
	/* Images with its dimension for the keypoint. */
	const aly::Image1f& grad = octave->gradient[is + 1];
	const aly::Image1f& ori = octave->orientation[is + 1];
	int const width = grad.width;
	int const height = grad.height;
	/* Clear feature vector. */
	desc.data.fill(0.0f);
	/* Rotation constants given by descriptor orientation. */
	float const sino = std::sin(desc.orientation);
	float const coso = std::cos(desc.orientation);

	/*
	 * Compute window size.
	 * Each spacial bin has an extension of 3 * sigma (sigma is the scale
	 * of the keypoint). For interpolation we need another half bin at
	 * both ends in each dimension. And since the window can be arbitrarily
	 * rotated, we need to multiply with sqrt(2). The window size is:
	 * 2W = sqrt(2) * 3 * sigma * (PXB + 1).
	 */
	float const binsize = 3.0f * sigma;
	int win =(int)( MATH_SQRT2 * binsize * (float) (PXB + 1) * 0.5f);
	if (ix < win || ix + win >= width || iy < win || iy + win >= height)
		return false;

	/*
	 * Iterate over the window, intersected with the image region
	 * from (1,1) to (w-2, h-2) since gradients/orientations are
	 * not defined at the boundary pixels. Add all samples to the
	 * corresponding bin.
	 */
	int const center = iy * width + ix; // Center pixel at KP location
	for (int dy = -win; dy <= win; ++dy) {
		int const yoff = dy * width;
		for (int dx = -win; dx <= win; ++dx) {
			/* Get pixel gradient magnitude and orientation. */
			float const mod = grad[center + yoff + dx].x;
			float const angle = ori[center + yoff + dx].x;
			float theta = angle - desc.orientation;
			if (theta < 0.0f)
				theta += 2.0f * ALY_PI;

			/* Compute fractional coordinates w.r.t. the window. */
			float const winx = (float) dx - dxf;
			float const winy = (float) dy - dyf;

			/*
			 * Compute normalized coordinates w.r.t. bins. The window
			 * coordinates are rotated around the keypoint. The bins are
			 * chosen such that 0 is the coordinate of the first bins center
			 * in each dimension. In other words, (0,0,0) is the coordinate
			 * of the first bin center in the three dimensional histogram.
			 */
			float binoff = (float) (PXB - 1) / 2.0f;
			float binx = (coso * winx + sino * winy) / binsize + binoff;
			float biny = (-sino * winx + coso * winy) / binsize + binoff;
			float bint = theta * (float) OHB / (2.0f * ALY_PI) - 0.5f;

			/* Compute circular window weight for the sample. */
			float gaussian_sigma = 0.5f * (float) PXB;
			float gaussian_weight = (float)gaussian_xx(
			MATH_POW2(binx - binoff) + MATH_POW2(biny - binoff),
					gaussian_sigma);

			/* Total contribution of the sample in the histogram is now: */
			float contrib = mod * gaussian_weight;

			/*
			 * Distribute values into bins (using trilinear interpolation).
			 * Each sample is inserted into 8 bins. Some of these bins may
			 * not exist, because the sample is outside the keypoint window.
			 */
			int bxi[2] = { (int) std::floor(binx), (int) std::floor(binx) + 1 };
			int byi[2] = { (int) std::floor(biny), (int) std::floor(biny) + 1 };
			int bti[2] = { (int) std::floor(bint), (int) std::floor(bint) + 1 };

			float weights[3][2] = { { (float) bxi[1] - binx, 1.0f
					- ((float) bxi[1] - binx) }, { (float) byi[1] - biny, 1.0f
					- ((float) byi[1] - biny) }, { (float) bti[1] - bint, 1.0f
					- ((float) bti[1] - bint) } };

			// Wrap around orientation histogram
			if (bti[0] < 0)
				bti[0] += OHB;
			if (bti[1] >= OHB)
				bti[1] -= OHB;

			/* Iterate the 8 bins and add weighted contrib to each. */
			int const xstride = OHB;
			int const ystride = OHB * PXB;
			for (int y = 0; y < 2; ++y)
				for (int x = 0; x < 2; ++x)
					for (int t = 0; t < 2; ++t) {
						if (bxi[x] < 0 || bxi[x] >= PXB || byi[y] < 0
								|| byi[y] >= PXB)
							continue;

						int idx = bti[t] + bxi[x] * xstride + byi[y] * ystride;
						desc.data[idx] += contrib * weights[0][x]
								* weights[1][y] * weights[2][t];
					}
		}
	}

	/* Normalize the feature vector. */
	desc.normalize();

	/* Truncate descriptor values to 0.2. */
	for (int i = 0; i < PXB * PXB * OHB; ++i)
		desc.data[i] = std::min(desc.data[i], 0.2f);

	/* Normalize once again. */
	desc.normalize();

	return true;
}

/* ---------------------------------------------------------------- */

/*
 * The scale of a keypoint is: scale = sigma0 * 2^(octave + (s+1)/S).
 * sigma0 is the initial blur (1.6), octave the octave index of the
 * keypoint (-1, 0, 1, ...) and scale space sample s in [-1,S+1] where
 * S is the amount of samples per octave. Since the initial blur 1.6
 * corresponds to scale space sample -1, we add 1 to the scale index.
 */

float Sift::keypointRelativeScale(SiftKeypoint const& kp) {
	return this->options.baseBlurSigma
			* std::pow(2.0f,
					(kp.sample + 1.0f) / this->options.samplesPerOctave);
}

float Sift::keypointAbsoluteScale(SiftKeypoint const& kp) {
	return this->options.baseBlurSigma
			* std::pow(2.0f,
					kp.octave
							+ (kp.sample + 1.0f)
									/ this->options.samplesPerOctave);
}

/* ---------------------------------------------------------------- */
void Sift::loadLoweDescriptors(std::string const& filename,
		SiftDescriptors* result) {
	std::ifstream in(filename.c_str());
	if (!in.good())
		throw std::runtime_error("Cannot open descriptor file");

	int num_descriptors;
	int num_dimensions;
	in >> num_descriptors >> num_dimensions;
	if (num_descriptors > 100000 || num_dimensions != 128) {
		in.close();
		throw std::runtime_error("Invalid number of descriptors/dimensions");
	}
	result->clear();
	result->reserve(num_descriptors);
	for (int i = 0; i < num_descriptors; ++i) {
		SiftDescriptor descriptor;
		in >> descriptor.y >> descriptor.x >> descriptor.scale
				>> descriptor.orientation;
		for (int j = 0; j < 128; ++j)
			in >> descriptor.data[j];
		descriptor.normalize();
		result->push_back(descriptor);
	}

	if (!in.good()) {
		result->clear();
		in.close();
		throw std::runtime_error("Error while reading descriptors");
	}

	in.close();
}

}
