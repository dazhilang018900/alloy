/*
 * Copyright (C) 2015, Simon Fuhrmann, Ronny Klowsky
 * TU Darmstadt - Graphics, Capture and Massively Parallel Computing
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD 3-Clause license. See the LICENSE.txt file for details.
 *
 * Notes:
 * - The implementation allows a minimum octave of -1 only.
 * - The descriptor extraction supports 128 dimensions only.
 * - Coordinates in the keypoint are relative to the octave.
 *   Absolute coordinates are obtained by (TODO why? explain):
 *   (x + 0.5, y + 0.5) * 2^octave - (0.5, 0.5).
 * - Memory consumption is quite high, especially with large images.
 *   TODO: Find a more efficient code path to create octaves.
 */
#ifndef INCLUDE_SIFT_H
#define INCLUDE_SIFT_H

#include <AlloyVector.h>
#include <AlloyImage.h>
#include <AlloyArray.h>
#include <AlloyLocator.h>
#include <string>
#include <vector>
#include "cereal/types/array.hpp"
namespace aly {

/**
 * Implementation of the SIFT feature detector and descriptor.
 * The implementation follows the description of the journal article:
 *
 *   Distinctive Image Features from Scale-Invariant Keypoints,
 *   David G. Lowe, International Journal of Computer Vision, 2004.
 *
 * The implementation used the siftpp implementation as reference for some
 * parts of the algorithm. This implementation is available here:
 *
 *   http://www.vlfeat.org/~vedaldi/code/siftpp.html
 */
/**
 * SIFT options.
 */
struct SiftOptions {

	/**
	 * Sets the amount of samples per octave. This defaults to 3
	 * and results in 6 blurred and 5 DoG images per octave.
	 */
	int samplesPerOctave;

	/**
	 * Sets the minimum octave ID. Defaults to 0, which uses the input
	 * image size as base size. Values >0 causes the image to be
	 * down scaled by factors of two. This can be set to -1, which
	 * expands the original image by a factor of two.
	 */
	int minOctave;

	/**
	 * Sets the maximum octave. This defaults to 4 and corresponds
	 * to the base image half-sized four times.
	 */
	int maxOctave;

	/**
	 * Sets contrast threshold, i.e. thresholds the absolute DoG value
	 * at the interpolated keypoint location. Defaults to 0.02 / samples.
	 * The default is computed if the given threshold value is negative.
	 */
	float contrastThreshold;

	/**
	 * Sets the edge threshold to eliminate edge responses. The threshold
	 * is the ratio between the principal curvatures (variable "r" in
	 * SIFT), and defaults to 10.
	 */
	float edgeRatioThreshold;

	/**
	 * Sets the amount of desired base blur before constructing the
	 * octaves. Default sigma is 1.6. This determines how to blur the base
	 * image in each octave before creating more octave samples.
	 * This is a technical detail and can usually be left alone.
	 */
	float baseBlurSigma;

	/**
	 * Sets the inherent blur sigma in the input image. Default is 0.5.
	 * This is a technical detail and can usually be left alone.
	 */
	float inherentBlurSigma;

	SiftOptions(void) :
			samplesPerOctave(3), minOctave(0), maxOctave(4), contrastThreshold(
					-1.0f), edgeRatioThreshold(10.0f), baseBlurSigma(1.6f), inherentBlurSigma(
					0.5f) {
	}
};

/**
 * Representation of a SIFT keypoint.
 * The keypoint locations are relative to the resampled size in
 * the image pyramid. To get the size relative to the input image,
 * each of (ix,iy,x,y) need to be multiplied with 2^o, where o
 * is the octave index of the keypoint. The octave index is -1 for the
 * upsampled image, 0 for the input image and >0 for subsampled images.
 * Note that the scale of the KP is already relative to the input image.
 */
struct SiftKeypoint : public aly::float2 {
	/** Octave index of the keypoint. Can be negative. */
	int octave;
	/** Sample index. Initally integer in {0 ... S-1}, later in [-1,S]. */
	float sample;
	template<class Archive> void serialize(Archive & archive) {
		archive(CEREAL_NVP(x), CEREAL_NVP(y),CEREAL_NVP(octave),CEREAL_NVP(sample));
	}
	SiftKeypoint(float x=0.0f,float y=0.0f): aly::float2(x,y),octave(-1),sample(-1.0f){
	}
};

/**
 * Representation of the SIFT descriptor.
 * The descriptor is created in a rotation invariant way. The resulting
 * vector is unsigned and normalized, and has 128 dimensions.
 */
struct SiftDescriptor : public aly::float2 {
	/** The scale (or sigma value) of the keypoint. */
	float scale;
	/** The orientation of the image keypoint in [0, 2PI]. */
	float orientation;
	/** The descriptor data, elements are unsigned in [0.0, 1.0]. */
	aly::Array<float, 128> data;
	template<class Archive> void serialize(Archive & archive) {
		archive(CEREAL_NVP(x), CEREAL_NVP(y),CEREAL_NVP(orientation),CEREAL_NVP(scale),CEREAL_NVP(data));
	}
	SiftDescriptor(float x=0.0f,float y=0.0f): float2(x,y),scale(1.0f),orientation(0.0f){
	}
	double length() const {
		double len = 0;
		for (float val : data) {
			len += val * val;
		}
		return std::sqrt(len);
	}
	void normalize() {
		double len = length();
		if (len > 1E-16) {
			len = 1.0 / len;
			for (float& val : data) {
				val *= (float) len;
			}
		} else {
			for (float& val : data) {
				val = 0.0;
			}
		}
	}
};
struct Octave {
	std::vector<aly::Image1f> gray; ///< S+3 images per octave
	std::vector<aly::Image1f> dog; ///< S+2 difference of gaussian images
	std::vector<aly::Image1f> gradient; ///< S+3 gradient images
	std::vector<aly::Image1f> orientation; ///< S+3 orientation images
};
struct ImageFeatures{
public:
	std::vector<SiftDescriptor> descriptors;
	std::vector<SiftKeypoint> keypoints;
	std::vector<std::shared_ptr<Octave>> octaves;
	ImageFeatures(const std::vector<SiftDescriptor>& d=std::vector<SiftDescriptor>(),
			const std::vector<SiftKeypoint>& k=std::vector<SiftKeypoint>(),
			const std::vector<std::shared_ptr<Octave>>& octaves=std::vector<std::shared_ptr<Octave>>()):descriptors(d),keypoints(k),octaves(octaves){
	}
};
typedef std::vector<std::shared_ptr<Octave>> Octaves;
typedef std::shared_ptr<Octave> OctavePtr;
typedef std::vector<SiftKeypoint> Keypoints;
typedef std::vector<SiftDescriptor> SiftDescriptors;
class Sift {
public:

protected:
	aly::Image1f orig; // Original input image
	void setImage(const aly::ImageRGB& img) {
		aly::ConvertImage(img, orig);
	}
	void setImage(const aly::ImageRGBA& img) {
		aly::ConvertImage(img, orig);
	}
	void setImage(const aly::ImageRGBf& img) {
		aly::ConvertImage(img, orig);
	}
	void setImage(const aly::ImageRGBAf& img) {
		aly::ConvertImage(img, orig);
	}
	void setImage(const aly::Image1f& img) {
		orig = img;
	}
	void solve(bool generateDesriptors);
public:
	Sift(SiftOptions options=SiftOptions());
	template<class T, int C, ImageType I> void solve(const Image<T,C,I>& img,bool generateDescriptors=true){
		setImage(img);
		solve(generateDescriptors);
	}
	Keypoints const& getKeypoints(void) const{
		return keypoints;
	}
	Octaves const& getOctaves(void) const{
		return octaves;
	}
	ImageFeatures const& getImageFeatures(void) const{
		return ImageFeatures(descriptors,keypoints,octaves);
	}
	Keypoints& getKeypoints(void) {
		return keypoints;
	}
	Octaves& getOctaves(void){
		return octaves;
	}
	SiftDescriptors const& getDescriptors(void) const;

	/**
	 * Helper function that creates SIFT descriptors from David Lowe's
	 * SIFT descriptor files.
	 */
	static void loadLoweDescriptors(std::string const& filename,
			SiftDescriptors* result);


protected:
	void create(void);
	void add(const aly::Image1f& image, float has_sigma, float target_sigma);
	void extremaDetection(void);
	std::size_t extremaDetection(const aly::Image1f* s[3], int oi, int si);
	void keypointLocalization(void);

	void descriptorGeneration(void);
	void generateFeatureImages(Octave* octave);
	void orientationAssignment(SiftKeypoint const& kp, Octave const* octave,
			std::vector<float>& orientations);
	bool descriptorAssignment(SiftKeypoint const& kp, SiftDescriptor& desc,
			Octave const* octave);

	float keypointRelativeScale(SiftKeypoint const& kp);
	float keypointAbsoluteScale(SiftKeypoint const& kp);

private:
	SiftOptions options;
	Octaves octaves; // The image pyramid (the octaves)
	Keypoints keypoints; // Detected keypoints
	SiftDescriptors descriptors; // Final SIFT descriptors
};

}

#endif /* SFM_SIFT_HEADER */
