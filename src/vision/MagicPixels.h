/*
 * ManifoldDescriptors.h
 *
 *  Created on: May 17, 2017
 *      Author: blake
 */

#ifndef INCLUDE_MAGICPIXELS_H_
#define INCLUDE_MAGICPIXELS_H_
#include "common/AlloyUnits.h"
#include "image/AlloyImage.h"
#include "image/AlloyDistanceField.h"
#include "math/AlloyVector.h"
#include "math/AlloyVecMath.h"
namespace aly {
class MagicPixels {
protected:

	DistanceField2f df;
	Vector3f colorCenters;
	Vector2f pixelCenters;
	float medianScore;
	Image2f vecFieldImage;
	Image1f edges;
	Image1f distField;
	ImageRGBf filtered;
	int numLabels;
	bool upsample;
	void edgeFilter(const ImageRGBf& in, Image1f& out);
	void clusterCenterRefinement(Vector2f& pts, int offset, float minDistance,float searchDistance);
	void clusterCenterRefinement(Vector2f& pts, const Image1f& distField,int offset, float minDistance);
	void regionGrow(Image1i& labelImage, const ImageRGBf& image, int offset,float colorTolerance, float maxDistance);
public:
	int computeConnectedComponents(const Image1i& labels, Image1i& outLabels,std::vector<int> &compCounts);
	int removeSmallConnectedComponents(const Image1i& labels, Image1i& outImage,int minSize);
	int makeLabelsUnique(Image1i& outImage);
	int fill(Image1i& labelImage,float minDistance, float spacing,float colorThreshold);
	int prune(Image1i& labelImage, int minSize);
	void solve(Image1i& labelImage, int Tile = 32,int minSize=4, float colorThreshold = 0.025f);
	void set(const ImageRGBA& image, int smoothIterations = 8,int diffuseIterations=128);
	float distance(int x, int y, int label, float colorThreshold) const;
	float updateClusters(const Image1i& labelImage, int labelOffset);
	void fill(Image1i& labelImage,int minSize,  float Tile, float colorThreshold);
	Vector3f& getColorCenters() {
		return colorCenters;
	}
	const Vector3f& getColorCenters() const {
		return colorCenters;
	}
	Vector2f& getPixelCenters() {
		return pixelCenters;
	}
	const Image2f& getVectorField() const {
		return vecFieldImage;
	}
	Image2f& getVectorField() {
		return vecFieldImage;
	}
	const Vector2f& getPixelCenters() const {
		return pixelCenters;
	}

	float2 getPixelCenter(int label) const {
		return pixelCenters[label];
	}
	aly::RGBf getColorCenter(int label) const {
		return colorCenters[label];
	}
	MagicPixels(bool upsample=false);
};

}

#endif /* INCLUDE_MAGICPIXELS_H_ */
