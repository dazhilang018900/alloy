#include "image/AlloyImageProcessing.h"
#include "image/AlloyAnisotropicFilter.h"
#include "math/AlloyDenseMatrix.h"
#include "math/AlloyDenseSolve.h"
#include "image/AlloyDistanceField.h"
#include "image/AlloyGradientVectorFlow.h"
#include "image/AlloyVolume.h"
#include "math/AlloySparseMatrix.h"
#include "math/AlloySparseSolve.h"
#include "graphics/AlloyLocator.h"
#include "graphics/AlloyDelaunay.h"
#include "vision/MagicPixels.h"
#include <queue>
#include <set>
namespace aly {
MagicPixels::MagicPixels(bool upsample):upsample(upsample) {
	numLabels = 0;
	medianScore = 0;
}
void MagicPixels::edgeFilter(const ImageRGBf& in, Image1f& out) {
	const int K = 1;
	out.resize(in.width, in.height);
	out.set(float1(1.0f));
	int KK = (2 * K + 1) * (2 * K + 1);
#pragma omp parallel for
	for (int j = K; j < in.height - K; j++) {
		for (int i = K; i < in.width - K; i++) {
			float3 sum(0.0f);
			for (int jj = -K; jj <= K; jj++) {
				for (int ii = -K; ii <= K; ii++) {
					sum += in(i + ii, j + jj);
				}
			}
			sum /= (float) KK;
			float3 res(0.0f);
			for (int jj = -K; jj <= K; jj++) {
				for (int ii = -K; ii <= K; ii++) {
					res += aly::abs(sum - in(i + ii, j + jj));
				}
			}
			out(i, j).x = lengthL1(res) / (3.0f * KK);
		}
	}
	medianScore = out.median().x;
}

float MagicPixels::updateClusters(const Image1i& labelImage, int labelOffset) {
	colorCenters.resize(numLabels);
	pixelCenters.resize(numLabels);
	std::vector<int> clustersize(numLabels, 0);
	Vector2f pixelMeans(numLabels);
	Vector3f colorMeans(numLabels);
	pixelMeans.set(float2(0.0f));
	for (int j = 0; j < labelImage.height; j++) {
		for (int i = 0; i < labelImage.width; i++) {
			int idx = labelImage(i, j).x + labelOffset;
			if (idx >= 0) {
				if (idx >= numLabels) {
					throw std::runtime_error(
							MakeString() << "Invalid cluster id. " << idx << "/"
									<< numLabels);
				}
				pixelMeans[idx] += float2((float) i, (float) j);
				colorMeans[idx] += filtered(i, j);
				clustersize[idx]++;
			}
		}
	}
	float E = 0.0f;
	for (int k = 0; k < numLabels; k++) {
		if (clustersize[k] > 0) {
			pixelCenters[k] = pixelMeans[k] / (float) clustersize[k];
			colorCenters[k] = colorMeans[k] / (float) clustersize[k];
		} else {
			colorCenters[k] = RGBf(0.0f);
		}
	}
	return E;
}

void MagicPixels::regionGrow(Image1i& labelImage, const ImageRGBf& image,
		int offset, float colorThreshold, float maxDistance) {

	auto cmp =
			[this](int3 left, int3 right) {
				return lengthSqr(float2(left.x,left.y)-pixelCenters[left.z])>lengthSqr(float2(right.x,right.y)-pixelCenters[right.z]);
			};
	std::priority_queue<int3, std::vector<int3>, decltype(cmp)> nbrQueue(cmp);
	for (int l = offset; l < (int) pixelCenters.size(); l++) {
		float2 center = pixelCenters[l];
		int3 pos = int3(center.x, center.y, l);
		nbrQueue.push(pos);
		//labelImage(pos.xy()).x=l;
	}
	float3 nrgb;
	while (nbrQueue.size() > 0) {
		int3 pos = nbrQueue.top();
		int l = pos.z;
		nbrQueue.pop();
		if (labelImage(pos.xy()).x != -1)
			continue;
		nrgb = image(pos.xy());
		float2 center = pixelCenters[l];
		float d = length(float2(pos.x, pos.y) - center);
		float3 crgb = image(center);
		if (0.33333f * lengthL1(crgb - nrgb) < colorThreshold
				&& d < maxDistance) {
			labelImage(pos.x, pos.y).x = l;
			int3 tst = int3(pos.x + 1, pos.y, l);
			if (labelImage(tst.xy()).x == -1) {
				nbrQueue.push(tst);
			}
			tst = int3(pos.x - 1, pos.y, l);
			if (labelImage(tst.xy()).x == -1) {
				nbrQueue.push(tst);
			}
			tst = int3(pos.x, pos.y + 1, l);
			if (labelImage(tst.xy()).x == -1) {
				nbrQueue.push(tst);
			}
			tst = int3(pos.x, pos.y - 1, l);
			if (labelImage(tst.xy()).x == -1) {
				nbrQueue.push(tst);
			}
		}
	}
}
void MagicPixels::clusterCenterRefinement(Vector2f& pts,
		const Image1f& distField, int offset, float minDistance) {
	float kappa = 1.0f / minDistance;
	float gamma = 2.0f / medianScore;
	float dt = 0.5f;
	int iters = 128;
	Vector2f tmp = pts;
	for (int iter = 0; iter < iters; iter++) {
		float avgScore = 0.0f;
		float avgReg = 0.0f;
		float avgDist = 0.0f;
#pragma omp parallel for
		for (int n = offset; n < (int) pts.size(); n++) {
			float2 pt = pts[n];
			float s = edges(pt.x, pt.y).x;
			avgScore += s;
			float2 grad =
					-std::min(gamma * s, 1.0f)
							* normalize(
									float2(
											0.5f
													* (edges(pt.x + 1, pt.y).x
															- edges(pt.x - 1,
																	pt.y).x),
											0.5f
													* (edges(pt.x, pt.y + 1).x
															- edges(pt.x,
																	pt.y - 1).x)));
			float2 reg;
			reg.x = distField(pt.x + 1, pt.y).x - distField(pt.x - 1, pt.y).x;
			reg.y = distField(pt.x, pt.y + 1).x - distField(pt.x, pt.y - 1).x;
			float d = std::abs(distField(pt.x, pt.y).x);
			reg = -normalize(reg);
			reg = kappa * std::max(minDistance - d, 0.0f) * reg;
			tmp[n] = clamp(pts[n] + dt * (grad + reg), float2(1.0f, 1.0f),
					float2(edges.width - 2.0f, edges.height - 2.0f));
			avgReg += length(reg);
			avgDist += d;
		}
		/*
		avgScore /= (pts.size() - offset);
		avgReg /= (pts.size() - offset);
		avgDist /= (pts.size() - offset);
		if (iter == 0 || iter >= iters - 10)
			std::cout << iter << ") Score:" << avgScore << " Reg:" << avgReg
					<< " Dist: " << avgDist << " " << minDistance << std::endl;
					*/
		pts = tmp;
	}
}

void MagicPixels::clusterCenterRefinement(Vector2f& pts, int offset,
		float minDistance, float searchDistance) {
	float kappa = 1.0f / minDistance;
	float gamma = 0.5f / medianScore;
	float dt = 0.5f;
	int iters = 128;
	std::vector<std::vector<int>> nbrs(pts.size());
	Locator2f locator(pts);
	for (int n = offset; n < (int) pts.size(); n++) {
		std::vector<std::pair<float2i, float>> results;
		locator.closest(pts[n], searchDistance, results);
		for (auto pr : results) {
			if (pr.first.index != n) {
				nbrs[n].push_back(pr.first.index);
			}
		}
	}
	Vector2f tmp = pts;
	for (int iter = 0; iter < iters; iter++) {
		float avgDistance = 0.0f;
		float avgScore = 0.0f;
		float minEstDistance = 1E30f;
		int count = 0;
#pragma omp parallel for
		for (int n = offset; n < (int) pts.size(); n++) {
			float2 pt = pts[n];
			float s = edges(pt.x, pt.y).x;
			avgScore += s;
			float2 grad =
					-std::min(gamma * s, 1.0f)
							* normalize(
									float2(
											0.5f
													* (edges(pt.x + 1, pt.y).x
															- edges(pt.x - 1,
																	pt.y).x),
											0.5f
													* (edges(pt.x, pt.y + 1).x
															- edges(pt.x,
																	pt.y - 1).x)));
			float2 reg(0.0f);
			for (int nbr : nbrs[n]) {
				float2 dir = (pt - pts[nbr]);
				float len = length(dir);
				reg += kappa * std::max(minDistance - len, 0.0f) * dir
						/ std::max(1E-3f, len);
				avgDistance += len;
				minEstDistance = std::min(len, minEstDistance);
				count++;
			}
			tmp[n] = clamp(pts[n] + dt * (grad + reg), float2(1.0f, 1.0f),
					float2(edges.width - 2.0f, edges.height - 2.0f));
			//std::cout<<"Location "<<tmp[n]<<" "<<pts[n]<<" "<<K<<std::endl;
		}
		//avgScore /= (pts.size() - offset);
		//avgDistance /= count;
		//std::cout << iter << ") " << avgScore << " " << avgDistance << " "<< minEstDistance <<" "<<count<< std::endl;
		pts = tmp;
	}

}
int MagicPixels::computeConnectedComponents(const Image1i& labels,
		Image1i& outLabels, std::vector<int> &compCounts) {
	const int xShift[8] = { -1, 1, 0, 0, 1, -1, -1, 1 };
	const int yShift[8] = { 0, 0, -1, 1, -1, 1, -1, 1 };
	outLabels.resize(labels.width, labels.height);
	outLabels.set(int1(-1));
	std::list<int2> queue;
	int2 pivot(0, 0);
	int cc = 0;
	int ccCount = 0;
	int pivotLabel = 0;
	compCounts.clear();
	for (int j = 1; j < labels.height - 1; j++) {
		for (int i = 1; i < labels.width - 1; i++) {
			if (labels(i, j).x >= 0) {
				pivot = int2(i, j);
				break;
			}
		}
	}
	do {
		queue.clear();
		outLabels(pivot) = int1(cc);
		pivotLabel = labels(pivot).x;
		ccCount = 1;
		queue.push_back(pivot);
		while (queue.size() > 0) {
			int2 v = queue.front();
			queue.pop_front();
			for (int s = 0; s < 8; s++) {
				int2 nbr = int2(v.x + xShift[s], v.y + yShift[s]);
				if (nbr.x >= 0 && nbr.y >= 0 && nbr.x < outLabels.width
						&& nbr.y < outLabels.height) {
					if (outLabels(nbr).x < 0 && labels(nbr).x == pivotLabel) {
						outLabels(nbr).x = cc;
						ccCount++;
						queue.push_back(nbr);
					}
				}
			}
		}
		compCounts.push_back(ccCount);
		pivot = int2(-1, -1);
		for (int t = 0; t < 16; t++) {
			int i = RandomUniform(0, labels.width - 1);
			int j = RandomUniform(0, labels.height - 1);
			if (outLabels(i, j).x < 0 && labels(i, j).x >= 0) {
				pivot = int2(i, j);
				break;
			}
		}
		if (pivot.x < 0) {
			for (int j = 1; j < labels.height - 1; j++) {
				for (int i = 1; i < labels.width - 1; i++) {
					if (outLabels(i, j).x < 0 && labels(i, j).x >= 0) {
						pivot = int2(i, j);
						break;
					}
				}
			}
		}
		cc++;
	} while (pivot.x >= 0);
	return (int) compCounts.size();
}
int MagicPixels::makeLabelsUnique(Image1i& outImage) {
	std::map<int, int> lookup;
	int counter = 0;
	lookup[-1] = -1;
	for (int j = 0; j < outImage.height; j++) {
		for (int i = 0; i < outImage.width; i++) {
			int l = outImage(i, j).x;
			if (l >= 0) {
				if (lookup.find(l) == lookup.end()) {
					lookup[l] = counter++;
				}
			}
		}
	}
	for (int j = 0; j < outImage.height; j++) {
		for (int i = 0; i < outImage.width; i++) {
			outImage(i, j).x = lookup[outImage(i, j).x];
		}
	}
	return counter;
}
int MagicPixels::removeSmallConnectedComponents(const Image1i& labelImage,
		Image1i& outImage, int minSize) {
	std::vector<int> compCounts;
	std::vector<int> labels;
	std::set<int> removeList;
	computeConnectedComponents(labelImage, outImage, compCounts);
	for (int l = 0; l < (int) compCounts.size(); l++) {
		if (compCounts[l] < minSize) {
			removeList.insert(l);
		}
	}
	for (int j = 0; j < outImage.height; j++) {
		for (int i = 0; i < outImage.width; i++) {
			int l = outImage(i, j).x;
			if (removeList.find(l) != removeList.end()) {
				outImage(i, j).x = -1;
			}
		}
	}
	return (int) removeList.size();
}
int MagicPixels::fill(Image1i& labelImage, float spacing, float Tile,
		float colorThreshold) {
	Image1i newLabels;
	Locator2f lookup(pixelCenters);
	std::vector<int2> order;
	order.reserve(labelImage.size());
	Image1f tmp(labelImage.width, labelImage.height);
	tmp.set(float1(1.0f));
	int L = pixelCenters.size();
	for (int j = 2; j < labelImage.height - 2; j++) {
		for (int i = 2; i < labelImage.width - 2; i++) {
			float s = aly::sign(labelImage(i, j).x);
			;
			if (s < 0.0f) {
				order.push_back(int2(i, j));
			}
			tmp(i, j).x = 0.5f * s;
		}
	}

	df.solve(tmp, distField, 1.5f * Tile);
	std::sort(order.begin(), order.end(), [this](const int2& a,const int2& b) {
		return (distField(a).x<distField(b).x);
	});
	for (int2 pix : order) {
		float2 pt = float2(pix.x, pix.y);
		if (lookup.closest(pt, spacing) == Locator2f::NO_POINT_FOUND) {
			lookup.insert(pt);
			pixelCenters.push_back(pt);
		}
	}
	if (L == (int) pixelCenters.size())
		return 0;
	clusterCenterRefinement(pixelCenters, distField, L, spacing);
	colorCenters.resize(pixelCenters.size());
	for (int l = 0; l < (int) pixelCenters.size(); l++) {
		float3 c = filtered(pixelCenters[l]);
		colorCenters[l] = c;
	}
	regionGrow(labelImage, filtered, L, colorThreshold, Tile);
	numLabels = pixelCenters.size();
	return (pixelCenters.size() - L);
}

int MagicPixels::prune(Image1i& labelImage, int minSize) {
	int L = pixelCenters.size();
	Image1i newLabels = labelImage;
	removeSmallConnectedComponents(labelImage, newLabels, minSize * minSize);
	numLabels = makeLabelsUnique(newLabels);
	labelImage = newLabels;
	updateClusters(labelImage, 0);
	return (L - (int) pixelCenters.size());
}
float MagicPixels::distance(int x, int y, int label,
		float colorTolerance) const {
	float3 c = filtered(x, y);
	float3 colorCenter = colorCenters[label];
	float d = (0.33333f * lengthL1(c - colorCenter)) / colorTolerance;
	return d - 1.0f;
}
void MagicPixels::fill(Image1i& labelImage, int minSize, float Tile,
		float colorThreshold) {
	float spacing = Tile;
	for (int k = 0; k < 2 * Tile; k++) {
		int L = pixelCenters.size();
		//std::cout<<"Filling "<<k<<std::endl;
		if (fill(labelImage, spacing, Tile, colorThreshold)) {
			prune(labelImage, minSize);
		}
		if (L <= (int) pixelCenters.size()) {
			spacing *= 0.75f;
		}
		if (spacing <= 2)
			break;
	}
}
void MagicPixels::set(const ImageRGBA& image, int smoothIterations,int diffuseIterations) {
	ImageRGBf colorImage;
	ConvertImage(image, colorImage);
	std::string f = MakeString() << GetDesktopDirectory() << ALY_PATH_SEPARATOR<<"diffused_"<<image.height<<"_"<<smoothIterations<<".png";
	if (FileExists(f)) {
		ReadImageFromFile(f, filtered);
	} else {
		std::cout << "Filter Image" << std::endl;
		if(smoothIterations>0){
			AnisotropicDiffusion(colorImage, filtered, smoothIterations,AnisotropicKernel::Gaussian, 0.01f, 1.0f);
		} else  {
			filtered=colorImage;
		}
		WriteImageToFile(f, filtered);
	}
	std::cout << "Edge Filter ..." << std::endl;
	if(upsample){
		ImageRGBf tmp3;
		Image1f tmp1;
		Image2f tmp2;
		UpSample(filtered,tmp3);
		edgeFilter(tmp3, tmp1);
		std::cout << "Gradient Flow Filter ..." << std::endl;
		SolveGradientVectorFlow(tmp1, tmp2, 0.1f,diffuseIterations, true);
		DownSample3x3(tmp1,edges);
		DownSample3x3(tmp2,vecFieldImage);
	} else {
		edgeFilter(filtered,edges);
		std::cout << "Gradient Flow Filter ..." << std::endl;
		SolveGradientVectorFlow(edges,vecFieldImage, 0.1f,diffuseIterations, true);
	}
	std::cout<<"Done Filter!"<<std::endl;
	f = MakeString() << GetDesktopDirectory() << ALY_PATH_SEPARATOR<<"edges_"<<image.height<<"_"<<smoothIterations<<".xml";
	WriteImageToRawFile(f, edges);
	f = MakeString() << GetDesktopDirectory() << ALY_PATH_SEPARATOR<<"vecfield_"<<image.height<<"_"<<smoothIterations<<".xml";
	WriteImageToRawFile(f, vecFieldImage);
}
void MagicPixels::solve(Image1i& labelImage, int Tile, int minSize,
		float colorThreshold) {
	colorCenters.clear();
	pixelCenters.clear();
	labelImage.resize(filtered.width, filtered.height);
	labelImage.set(int1(-1));
	for (int j = 0; j < filtered.height; j += Tile) {
		for (int i = 0; i < filtered.width; i += Tile) {
			float shift = 0.25f * ((j / Tile) % 2) + 0.5f;
			pixelCenters.push_back(float2(i + shift * Tile, j + Tile / 2.0f));
		}
	}
	clusterCenterRefinement(pixelCenters, 0, Tile, 1.5f * Tile);
	colorCenters.resize(pixelCenters.size());
	for (int l = 0; l < (int) pixelCenters.size(); l++) {
		float2 center = pixelCenters[l];
		float3 c = filtered(center);
		colorCenters[l] = c;
	}
	regionGrow(labelImage, filtered, 0, colorThreshold, Tile);
	fill(labelImage, minSize, Tile, colorThreshold);
	numLabels = pixelCenters.size();
}
}
