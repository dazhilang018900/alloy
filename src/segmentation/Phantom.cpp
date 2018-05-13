/*
 * Phantom.cpp
 *
 *  Created on: Mar 25, 2018
 *      Author: blake
 */

#include "segmentation/Phantom.h"
#include <AlloyDistanceField.h>
namespace aly {
Phantom::Phantom(int rows, int cols, int slices,float maxDist) :
		rows(rows), cols(cols), slices(slices), noiseLevel(0.1f), invert(false), fuzziness(
				2.0f), heavisideType(Heaviside::ARCTAN),maxDistance(maxDist) {

}
float Phantom::heaviside(float val, float fuzziness, Heaviside heavy) {
	float invPI = 1.0 / ALY_PI;
	float invFuzziness = 1.0 / fuzziness;
	switch (heavy) {
	case Heaviside::ARCTAN:
		return (0.5 + invPI * std::atan(invFuzziness * val));

	case Heaviside::BINARY:
		return (val > 0) ? 1 : 0;

	case Heaviside::SIGMOID: {
		float e = std::exp(-val / fuzziness);
		return ((1 - e) / (1 + e));
	}
	case Heaviside::SIN:
		if (val > fuzziness) {
			return 1;
		} else if (val < -fuzziness) {
			return 0;
		} else {
			return (0.5
					* (1 + val * invFuzziness
							+ invPI * std::sin(invFuzziness * val * ALY_PI)));
		}
	default:
		return (val > 0) ? 1 : 0;

	}
}
const Volume1f& Phantom::solve(bool distanceFieldOnly) {
	Volume1f vol(rows, cols, slices);
	solveInternal(vol);
	if(distanceFieldOnly){
		DistanceField3f df;
		df.solve(vol, levelSet, maxDistance);
	} else {
		finish(vol);
	}
	return levelSet;
}
void Phantom::finish(const Volume1f& vol) {
	DistanceField3f df;
	df.solve(vol, levelSet, maxDistance);
#pragma omp parallel for
	for (int k = 0; k < slices; k++) {
		for (int j = 0; j < cols; j++) {
			for (int i = 0; i < rows; i++) {
				float noise = RandomUniform(-noiseLevel, noiseLevel);
				float val = noise
						+ heaviside(levelSet(i, j, k).x, fuzziness,
								heavisideType);
				if (invert) {
					levelSet(i, j, k).x = 1.0f - val;
				} else {
					levelSet(i, j, k).x = val;
				}
			}
		}
	}
}

void PhantomBubbles::solveInternal(Volume1f& levelset) {
	float scale = 2.0 / std::min(rows, std::min(cols, slices));
#pragma omp parallel for
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			for (int k = 0; k < slices; k++) {
				levelset(i, j, k).x = 1E10;
			}
		}
	}
	for (int n = 0; n < numBubbles; n++) {
		float v = RandomUniform(0.0f, 1.0f);
		float ra = (1 - v) * minRadius + v * maxRadius;
		float3 center((RandomUniform(-1.0f, 1.0f)) * (1 - ra - 4.0 / rows),
				(RandomUniform(-1.0f, 1.0f)) * (1 - ra - 4.0 / cols),
				(RandomUniform(-1.0f, 1.0f)) * (1 - ra - 4.0 / slices));
#pragma omp parallel for
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				for (int k = 0; k < slices; k++) {
					float x = (i - 0.5 * rows) * scale;
					float y = (j - 0.5 * cols) * scale;
					float z = (k - 0.5 * slices) * scale;
					float r = std::sqrt(
							(x - center.x) * (x - center.x)
									+ (y - center.y) * (y - center.y)
									+ (z - center.z) * (z - center.z));
					levelset(i, j, k).x = std::min(levelset(i, j, k).x, r - ra);
				}
			}
		}
	}

}
void PhantomTorus::solveInternal(Volume1f& levelset) {
	float scale = 2.0 / std::min(rows, std::min(cols, slices));
#pragma omp parallel for
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			for (int k = 0; k < slices; k++) {
				float x = (i - 0.5 * rows) * scale;
				float y = (j - 0.5 * cols) * scale;
				float z = (k - 0.5 * slices) * scale;
				float xp = (x - center.x);
				float yp = (y - center.y);
				float zp = (z - center.z);
				float tmp = (outerRadius - std::sqrt(zp * zp + yp * yp));
				levelset(i, j, k,
						tmp * tmp + xp * xp - innerRadius * innerRadius);
			}
		}
	}
}

void PhantomSphere::solveInternal(Volume1f& levelset) {
	float scale = 2.0 / std::min(rows, std::min(cols, slices));
#pragma omp parallel for
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			for (int k = 0; k < slices; k++) {
				float x = (i - 0.5 * rows) * scale;
				float y = (j - 0.5 * cols) * scale;
				float z = (k - 0.5 * slices) * scale;
				float r = std::sqrt(
						(x - center.x) * (x - center.x)
								+ (y - center.y) * (y - center.y)
								+ (z - center.z) * (z - center.z));
				levelset(i, j, k).x = (r - radius) / scale;
			}
		}
	}
}

void PhantomCube::solveInternal(Volume1f& levelset) {
#pragma omp parallel for
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			for (int k = 0; k < slices; k++) {
				double x = (i - 0.5 * rows) / (float) (0.5 * rows);
				double y = (j - 0.5 * cols) / (float) (0.5 * cols);
				double z = (k - 0.5 * slices) / (float) (0.5 * slices);
				levelset(i, j, k).x =
						((x - center.x) > -0.5 * width
								&& (x - center.x) < 0.5 * width
								&& (y - center.y) > -0.5 * width
								&& (y - center.y) < 0.5 * width
								&& (z - center.z) > -0.5 * width
								&& (z - center.z) < 0.5 * width) ? -1 : 1;
			}
		}
	}
}
void PhantomMetasphere::solveInternal(Volume1f& levelset) {
	float scale = 2.0 / std::min(rows, std::min(cols, slices));
#pragma omp parallel for
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			for (int k = 0; k < slices; k++) {
				float x = (i - 0.5 * rows) * scale;
				float y = (j - 0.5 * cols) * scale;
				float z = (k - 0.5 * slices) * scale;
				float rXY = std::sqrt(
						(x - center.x) * (x - center.x)
								+ (y - center.y) * (y - center.y));

				float rXYZ = std::sqrt(
						(x - center.x) * (x - center.x)
								+ (y - center.y) * (y - center.y)
								+ (z - center.z) * (z - center.z));
				float alpha = std::atan2(y, x);
				float r1 = std::sqrt(
						maxAmplitude * maxAmplitude
								- ((z - center.z) * (z - center.z)))
						/ maxAmplitude;
				float beta = std::atan2(z, rXY);
				float d = (minAmplitude
						+ (maxAmplitude - minAmplitude)
								* (std::cos(alpha * frequency)));
				float r2 = (minAmplitude
						+ (maxAmplitude - minAmplitude)
								* (std::cos(2 * beta * frequency)));
				levelset(i, j, k).x = 0.5 * ((rXY - r1 * d) + (rXYZ - r2));
			}
		}
	}
}
void PhantomCheckerBoard::solveInternal(Volume1f& levelset) {
	float scale = 2.0f / std::min(rows, std::min(cols, slices));
	float f = 2 * ALY_PI / width;
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			for (int k = 0; k < slices; k++) {
				float x = (i - 0.5 * rows) * scale;
				float y = (j - 0.5 * cols) * scale;
				float z = (k - 0.5 * slices) * scale;
				levelset(i, j, k).x = (
						((x - center.x) > -0.5 * width
								&& (x - center.x) < 0.5 * width
								&& (y - center.y) > -0.5 * width
								&& (y - center.y) < 0.5 * width
								&& (z - center.z) > -0.5 * width
								&& (z - center.z) < 0.5 * width) ?
								aly::sign(
										std::sin(f * xFrequency * x)
												* std::sin(f * yFrequency * y)
												* std::sin(
														f * zFrequency * z)) :
								1);
			}
		}
	}
}
PhantomSphereCollection::PhantomSphereCollection(int rows, int cols, int slices,float radius) {
	int gridRows = (int) std::floor(rows / (2 * radius + 3));
	int gridCols = (int) std::floor(cols / (2 * radius + 3));
	int gridSlices = (int) std::floor(slices / (2 * radius + 3));
	float deltaX = rows / (float) gridRows;
	float deltaY = cols / (float) gridCols;
	float deltaZ = slices / (float) gridSlices;
	distfieldImage.resize(rows, cols, slices);
	labelImage.resize(rows, cols, slices);
	std::vector<float3> gridPoints;
	for (int k = 0; k < gridSlices; k++) {
		for (int j = 0; j < gridCols; j++) {
			for (int i = 0; i < gridRows; i++) {
				gridPoints.push_back(
						float3((i + 0.5f) * deltaX, (j + 0.5f) * deltaY,
								(k + 0.5f) * deltaZ));
			}
		}
	}
	int numDots=gridPoints.size();
	Shuffle(gridPoints);
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			for (int k = 0; k < slices; k++) {
				float val = 10.0f;
				int label = 0;
				for (int l = 0; l < numDots; l++) {
					float3 pt = gridPoints[l];
					float dist = (pt.x - i) * (pt.x - i)
							+ (pt.y - j) * (pt.y - j)
							+ (pt.z - k) * (pt.z - k);
					if (dist < radius * radius) {
						label = l+1;
					}
					val = (float) std::min(val,
							std::abs(std::sqrt(dist) - radius));
				}
				labelImage(i, j, k) = label;
				distfieldImage(i, j, k) = val;
			}
		}
	}
}
}
