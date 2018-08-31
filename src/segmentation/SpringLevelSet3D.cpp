/*
 * Copyright(C) 2016, Blake C. Lucas, Ph.D. (img.science@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "segmentation/SpringLevelSet3D.h"
#include "AlloyApplication.h"
#include "AlloyDistanceField.h"
namespace aly {

float SpringLevelSet3D::MIN_ANGLE_TOLERANCE = (float) (ALY_PI * 20 / 180.0f);
float SpringLevelSet3D::NEAREST_NEIGHBOR_DISTANCE = std::sqrt(2.0f) * 0.5f;
float SpringLevelSet3D::PARTICLE_RADIUS = 0.05f;
float SpringLevelSet3D::REST_RADIUS = 0.1f;
float SpringLevelSet3D::SPRING_CONSTANT = 0.3f;
float SpringLevelSet3D::EXTENT = 0.5f;
float SpringLevelSet3D::SHARPNESS = 5.0f;
int SpringLevelSet3D::MAX_NEAREST_NEIGHBORS = 2;

SpringLevelSet3D::SpringLevelSet3D(
		const std::shared_ptr<ManifoldCache3D>& cache) :
		ActiveManifold3D("Spring Level Set 3D", cache), resampleEnabled(true) {
	contour.meshType = MeshType::Quad;
}
void SpringLevelSet3D::setSpringls(const Vector3f& particles,
		const Vector3f& points) {
	contour.particles = particles;
	contour.correspondence = particles;
	contour.updateNormals();
}

void SpringLevelSet3D::updateUnsignedLevelSet(float narrowBand) {
	size_t N = contour.particles.size();
	unsignedLevelSet.resize(levelSet.rows, levelSet.cols, levelSet.slices);
	unsignedLevelSet.set(narrowBand);
	if (contour.meshType == MeshType::Triangle) {
#pragma omp parallel for
		for (size_t n = 0; n < N; n++) {
			float3 v1 = contour.vertexes[n * 3];
			float3 v2 = contour.vertexes[n * 3 + 1];
			float3 v3 = contour.vertexes[n * 3 + 2];
			float3 minPt = aly::min(aly::min(v1, v2), v3);
			float3 maxPt = aly::max(aly::max(v1, v2), v3);
			int3 dims = int3(aly::round(maxPt - minPt) + 0.5f + 2 * narrowBand);
			int3 pos = int3(aly::floor(minPt) - 0.25f - narrowBand);
			float3 closestPoint;
			for (int k = 0; k <= dims.z; k++) {
				for (int j = 0; j <= dims.y; j++) {
					for (int i = 0; i <= dims.x; i++) {
						if (unsignedLevelSet.contains((i + pos.x), (j + pos.y),
								(k + pos.z))) {
							float3 pt = float3((i + pos.x), (j + pos.y),
									(k + pos.z));
							float d = std::sqrt(
									DistanceToTriangleSqr(pt, v1, v2, v3,
											&closestPoint));

							float& old = unsignedLevelSet((i + pos.x),
									(j + pos.y), (k + pos.z)).x;
							old = std::min(old, d);
						}
					}
				}
			}
		}
	} else {
#pragma omp parallel for
		for (size_t n = 0; n < N; n++) {
			float3 v1 = contour.vertexes[n * 4];
			float3 v2 = contour.vertexes[n * 4 + 1];
			float3 v3 = contour.vertexes[n * 4 + 2];
			float3 v4 = contour.vertexes[n * 4 + 3];
			float3 minPt = aly::min(aly::min(v1, v2), aly::min(v3, v4));
			float3 maxPt = aly::max(aly::max(v1, v2), aly::max(v3, v4));
			int3 dims = int3(aly::round(maxPt - minPt) + 0.5f + 2 * narrowBand);
			int3 pos = int3(aly::floor(minPt) - 0.25f - narrowBand);
			float3 closestPoint;
			for (int k = 0; k <= dims.z; k++) {
				for (int j = 0; j <= dims.y; j++) {
					for (int i = 0; i <= dims.x; i++) {
						if (unsignedLevelSet.contains((i + pos.x), (j + pos.y),
								(k + pos.z))) {
							float3 pt = float3((i + pos.x), (j + pos.y),
									(k + pos.z));
							float d = std::sqrt(
									DistanceToQuadSqr(pt, v1, v2, v3, v4,
											&closestPoint));
							float& old = unsignedLevelSet((i + pos.x),
									(j + pos.y), (k + pos.z)).x;
							old = std::min(old, d);
						}
					}
				}
			}
		}
	}
}
float3 SpringLevelSet3D::traceUnsigned(float3 pt) {
	float disp = 0.0f;
	int iter = 0;
	const float timeStep = 0.5f;
	float3 grad;
	float v111, v211, v121, v101, v011, v110, v112;
	do {
		v211 = unsignedLevelSet(pt.x + 1, pt.y, pt.z).x;
		v121 = unsignedLevelSet(pt.x, pt.y + 1, pt.z).x;
		v101 = unsignedLevelSet(pt.x, pt.y - 1, pt.z).x;
		v011 = unsignedLevelSet(pt.x - 1, pt.y, pt.z).x;
		v111 = unsignedLevelSet(pt.x, pt.y, pt.z).x;
		v110 = unsignedLevelSet(pt.x, pt.y, pt.z - 1).x;
		v112 = unsignedLevelSet(pt.x, pt.y, pt.z + 1).x;
		grad.x = 0.5f * (v211 - v011);
		grad.y = 0.5f * (v121 - v101);
		grad.z = 0.5f * (v112 - v110);
		grad *= v111;
		disp = length(grad);
		pt = pt - timeStep * grad;
		iter++;
	} while (disp > 1E-5f && iter < 30);
	return pt;
}

float3 SpringLevelSet3D::traceInitial(float3 pt) {
	float disp = 0.0f;
	int iter = 0;
	const float timeStep = 0.5f;
	float3 grad;
	float v111, v211, v121, v101, v011, v110, v112;
	do {
		v211 = std::abs(initialLevelSet(pt.x + 1, pt.y, pt.z).x);
		v121 = std::abs(initialLevelSet(pt.x, pt.y + 1, pt.z).x);
		v101 = std::abs(initialLevelSet(pt.x, pt.y - 1, pt.z).x);
		v011 = std::abs(initialLevelSet(pt.x - 1, pt.y, pt.z).x);
		v110 = std::abs(initialLevelSet(pt.x, pt.y, pt.z - 1).x);
		v112 = std::abs(initialLevelSet(pt.x, pt.y, pt.z + 1).x);
		v111 = std::abs(initialLevelSet(pt.x, pt.y, pt.z).x);
		grad.x = 0.5f * (v211 - v011);
		grad.y = 0.5f * (v121 - v101);
		grad.z = 0.5f * (v112 - v110);
		grad *= v111;
		disp = length(grad);
		pt = pt - timeStep * grad;
		iter++;
	} while (disp > 1E-5f && iter < 30);
	return pt;
}
void SpringLevelSet3D::updateNearestNeighbors(float maxDistance) {
	nearestNeighbors.clear();
	if (contour.vertexes.size() == 0)
		return;
	std::cout << "Build matcher" << std::endl;
	matcher.reset(new Matcher3f(contour.vertexes));
	nearestNeighbors.resize(contour.vertexes.size(),
			std::vector<SpringlEdge>());
	int N = (int) contour.particles.size();
	int K = (contour.meshType == MeshType::Triangle) ? 3 : 4;
#pragma omp parallel for
	for (int n = 0; n < N; n++) {
		for (int k = 0; k < K; k++) {
			std::vector<std::pair<size_t, float>> result;
			size_t index = n * K + k;
			float3 q = contour.vertexes[index];
			matcher->closest(q, maxDistance, result);
			std::vector<SpringlEdge>& edges = nearestNeighbors[index];
			std::map<uint32_t, SpringlEdge> nMap;
			for (auto pr : result) {
				if (pr.first < n * K && pr.first >= (n + 1) * K) {
					uint32_t nn = pr.first / K;
					int8_t kk = pr.first - K * nn;
					SpringlEdge edge(nn, kk,
							DistanceToEdgeSqr(q, contour.vertexes[pr.first],
									contour.vertexes[nn * K + (kk + 1) % K]));
					auto pos = nMap.find(nn);
					if (pos != nMap.end()) {
						if (edge.distance < pos->second.distance) {
							nMap[nn] = edge;
						}
					} else {
						nMap[nn] = edge;
					}
				}
			}
			edges.clear();
			edges.reserve(nMap.size());
			for (auto pr : nMap) {
				edges.push_back(pr.second);
			}
			std::sort(edges.begin(), edges.end());
			if (edges.size() > MAX_NEAREST_NEIGHBORS) {
				edges.erase(edges.begin() + MAX_NEAREST_NEIGHBORS, edges.end());
			}
		}
	}
}
int SpringLevelSet3D::fill() {
	{
		std::lock_guard < std::mutex > lockMe(contourLock);
		isoSurface.solve(levelSet, activeList, contour.vertexes,
				contour.meshType, false, 0.0f);
		requestUpdateSurface = false;
	}

	int fillCount = 0;
	/*
	 for (std::vector<uint32_t> curve : contour.indexes) {
	 size_t count = 0;
	 uint32_t first = 0, prev = 0;
	 if (curve.size() > 1) {
	 for (uint32_t idx : curve) {
	 if (count != 0) {
	 float2 pt = 0.5f*(contour.vertexes[prev] + contour.vertexes[idx]);
	 if (unsignedLevelSet(pt.x, pt.y).x > 0.5f*(NEAREST_NEIGHBOR_DISTANCE+EXTENT)) {
	 contour.particles.push_back(pt);
	 for (Vector2f& vel : contour.velocities) {
	 vel.push_back(float2(0.0f));
	 }
	 contour.points.push_back(contour.vertexes[prev]);
	 contour.points.push_back(contour.vertexes[idx]);
	 contour.correspondence.push_back(float2(std::numeric_limits<float>::infinity()));
	 fillCount++;
	 }
	 if (idx == first) break;
	 }
	 else {
	 first = idx;
	 }
	 count++;
	 prev = idx;
	 }
	 }
	 }
	 */
	return fillCount;
}
void SpringLevelSet3D::updateTracking(float maxDistance) {
	int tries = 0;
	int invalid = 0;
	/*
	 do {
	 invalid = 0;
	 matcher.reset(new Matcher3f(oldPoints));
	 std::vector<int> retrack;
	 for (size_t i = 0;i < contour.particles.size();i++) {
	 if (std::isinf(contour.correspondence[i].x)) {
	 retrack.push_back((int)i);
	 }
	 }
	 int N = (int)retrack.size();
	 for (int i = 0;i < N;i++) {
	 int pid = retrack[i];
	 int eid1 = pid * 2;
	 int eid2 = pid * 2 + 1;
	 float3 pt0 = contour.points[eid1];
	 float3 pt1 = contour.points[eid2];
	 std::vector<std::pair<size_t, float>> result;
	 float3 q1(std::numeric_limits<float>::infinity());
	 float3 q2(std::numeric_limits<float>::infinity());
	 matcher->closest(pt0, maxDistance, result);
	 std::array<float3, 4> velocities;
	 for (auto pr : result) {
	 q1 = oldCorrespondences[pr.first / 2];
	 if (!std::isinf(q1.x)) {
	 for (int nn = 0;nn < 4;nn++) {
	 velocities[nn] = oldVelocities[nn][pr.first / 2];
	 }
	 break;
	 }
	 }
	 result.clear();
	 matcher->closest(pt1, maxDistance, result);
	 for (auto pr : result) {
	 q2 = oldCorrespondences[pr.first / 2];
	 if (!std::isinf(q2.x)) {
	 for (int nn = 0;nn < 4;nn++) {
	 velocities[nn] += oldVelocities[nn][pr.first / 2];
	 }
	 break;
	 }
	 }

	 if (!std::isinf(q1.x)) {
	 if (!std::isinf(q2.x)) {
	 for (int nn = 0;nn < 4;nn++) {
	 contour.velocities[nn][pid] = 0.5f*velocities[nn];
	 oldVelocities[nn].push_back(0.5f*velocities[nn]);
	 }

	 q1 = traceInitial(0.5f*(q1 + q2));
	 contour.correspondence[pid] = q1;
	 oldCorrespondences.push_back(q1);

	 oldPoints.push_back(pt0);
	 oldPoints.push_back(pt1);
	 }
	 else {
	 for (int nn = 0;nn < 4;nn++) {
	 contour.velocities[nn][pid] = velocities[nn];
	 oldVelocities[nn].push_back(velocities[nn]);
	 }
	 contour.correspondence[pid] = q1;
	 oldCorrespondences.push_back(q1);
	 oldPoints.push_back(pt0);
	 oldPoints.push_back(pt1);
	 }
	 }
	 else if (!std::isinf(q2.x)) {
	 for (int nn = 0;nn < 4;nn++) {
	 contour.velocities[nn][pid] = velocities[nn];
	 oldVelocities[nn].push_back(velocities[nn]);
	 }
	 contour.correspondence[pid] = q2;
	 oldCorrespondences.push_back(q2);
	 oldPoints.push_back(pt0);
	 oldPoints.push_back(pt1);
	 }
	 else {
	 invalid++;
	 }
	 }
	 tries++;
	 } while (invalid > 0 && tries<4);
	 */
}
int SpringLevelSet3D::contract() {
	int contractCount = 0;
	Vector3f particles;
	Vector3f points;
	Vector3f normals;
	Vector3f correspondence;
	std::array<Vector2f, 4> velocities;
	particles.data.reserve(contour.particles.size());
	points.data.reserve(contour.particles.size());
	normals.data.reserve(contour.particles.size());
	/*
	 for (int i = 0;i<(int)contour.particles.size();i++) {
	 float2 pt = contour.particles[i];
	 float d1 = distance(contour.points[2 * i + 1], pt);
	 float d2 = distance(contour.points[2 * i], pt);
	 if (std::abs(levelSet(pt.x, pt.y).x) <= 1.25f*EXTENT
	 &&d1>3.0f*PARTICLE_RADIUS
	 &&d2 > 3.0f*PARTICLE_RADIUS
	 &&d1 < 1.5f
	 &&d2 < 1.5f
	 ) {
	 particles.push_back(pt);
	 points.push_back(contour.points[2 * i]);
	 points.push_back(contour.points[2 * i + 1]);
	 normals.push_back(contour.normals[i]);
	 for (int nn = 0;nn < 4;nn++){
	 velocities[nn].push_back(contour.velocities[nn][i]);
	 }
	 correspondence.push_back(contour.correspondence[i]);
	 }
	 else {
	 contractCount++;
	 }
	 }
	 if (contractCount > 0) {
	 contour.points = points;
	 contour.normals = normals;
	 contour.particles = particles;
	 contour.velocities = velocities;
	 contour.correspondence = correspondence;
	 contour.setDirty(true);
	 }
	 */
	return contractCount;
}
void SpringLevelSet3D::computeForce(size_t idx, float3& f1, float3& f2,
		float3& f) {
	f1 = float3(0.0f);
	f2 = float3(0.0f);
	f = float3(0.0f);
	float3 p = contour.particles[idx];
	/*
	 float3 p1 = contour.vertexLocations[2 * idx];
	 float3 p2 = contour.vertexLocations[2 * idx + 1];
	 if (pressureImage.size() > 0&& pressureParam.toFloat()!=0.0f) {
	 float2 v1 = normalize(contour.points[2 * idx + 1] - contour.points[2 * idx]);
	 float2 norm = float2(-v1.y, v1.x);
	 float2 pres = pressureParam.toFloat()*norm*pressureImage(p.x, p.y).x;
	 f = pres;
	 f1 = f;
	 f2 = f;
	 }

	 float2x2 M,A;
	 if (vecFieldImage.size() > 0&& advectionParam.toFloat()!=0.0f) {
	 float w = advectionParam.toFloat();
	 f1 += vecFieldImage(p1.x, p1.y)*w;
	 f2 += vecFieldImage(p2.x, p2.y)*w;
	 f  += vecFieldImage(p.x , p.y )*w;
	 }
	 float2 k1, k2, k3, k4;
	 k4 = contour.velocities[2][idx];
	 k3 = contour.velocities[1][idx];
	 k2 = contour.velocities[0][idx];
	 k1 = f;

	 contour.velocities[3][idx] = k4;
	 contour.velocities[2][idx] = k3;
	 contour.velocities[1][idx] = k2;
	 contour.velocities[0][idx] = k1;
	 if (simulationIteration>=4) {
	 f = (1.0f / 6.0f)*(k1 + 2.0f * k2 + 2.0f * k3 + k4);
	 } else if (simulationIteration == 3) {
	 f = (1.0f / 4.0f)*(k1 + 2.0f * k2 + k3);
	 } if (simulationIteration == 2) {
	 f = (1.0f / 2.0f)*(k1 + k2);
	 }
	 float2 v1 = p1 +f1;
	 float2 v2 = p2 +f2;
	 float2 v = p + f;
	 //Correction keeps particle and points on same line segment
	 float2 t = normalize(v2 - v1);
	 float2 correction=v-(v1+dot(t,v-v1)*t);
	 f1 += correction;
	 f2 += correction;
	 */
}
void SpringLevelSet3D::updateSignedLevelSet(float maxStep) {
#pragma omp parallel for
	for (int i = 0; i < (int) activeList.size(); i++) {
		int3 pos = activeList[i];
		distanceFieldMotion(pos.x, pos.y, pos.z, i);
	}
	float timeStep = (float) maxStep;
	if (!clampSpeed) {
		float maxDelta = 0.0f;
		for (float delta : deltaLevelSet) {
			maxDelta = std::max(std::abs(delta), maxDelta);
		}
		const float maxSpeed = 0.999f;
		timeStep = (float) (maxStep
				* ((maxDelta > maxSpeed) ? (maxSpeed / maxDelta) : maxSpeed));
	}
	contourLock.lock();

#pragma omp parallel for
	for (int i = 0; i < (int) activeList.size(); i++) {
		int3 pos = activeList[i];
		applyForces(pos.x, pos.y, pos.z, i, timeStep);
	}
	for (int band = 1; band <= maxLayers; band++) {
#pragma omp parallel for
		for (int i = 0; i < (int) activeList.size(); i++) {
			int3 pos = activeList[i];
			updateDistanceField(pos.x, pos.y, pos.z, band);
		}
	}
#pragma omp parallel for
	for (int i = 0; i < (int) activeList.size(); i++) {
		int3 pos = activeList[i];
		plugLevelSet(pos.x, pos.y, pos.z, i);
	}
	requestUpdateSurface = true;
	contourLock.unlock();

#pragma omp parallel for
	for (int i = 0; i < (int) activeList.size(); i++) {
		int3 pos = activeList[i];
		swapLevelSet(pos.x, pos.y, pos.z) = levelSet(pos.x, pos.y, pos.z);
	}
	deleteElements();
	addElements();
	deltaLevelSet.clear();
	deltaLevelSet.resize(activeList.size(), 0.0f);
}
float SpringLevelSet3D::advect(float maxStep) {
	Vector3f f(contour.particles.size());
	Vector3f f1(contour.particles.size());
	Vector3f f2(contour.particles.size());
#pragma omp parallel for
	for (int i = 0; i < (int) f.size(); i++) {
		computeForce(i, f1[i], f2[i], f[i]);
	}
	float maxForce = 0.0f;
	for (int i = 0; i < (int) f.size(); i++) {
		maxForce = std::max(maxForce, lengthSqr(f[i]));
	}
	maxForce = std::sqrt(maxForce);
	float timeStep = (maxForce > 1.0f) ? maxStep / (maxForce) : maxStep;
#pragma omp parallel for
	for (int i = 0; i < (int) f.size(); i++) {
		contour.particles[2 * i] += timeStep * f1[i];
		contour.particles[2 * i + 1] += timeStep * f2[i];
		contour.particles[i] += timeStep * f[i];
	}
	contour.updateNormals();
	contour.setDirty(true);
	return timeStep;
}
void SpringLevelSet3D::relax(float timeStep) {
	Vector3f updates(contour.vertexes.size());
#pragma omp parallel for
	for (int i = 0; i < (int) contour.particles.size(); i++) {
		relax(i, timeStep, updates);
	}
	contour.vertexes = updates;
	contour.updateNormals();
	contour.setDirty(true);
}
void SpringLevelSet3D::relax(size_t idx, float timeStep, Vector3f& updates) {
	float w, len;
	float3 tanget;
	float3 dir;
	const int K = (contour.meshType == MeshType::Triangle) ? 3 : 4;
	std::vector<float3> vertexVelocity(K);
	std::vector<float3> tangets(K);
	std::vector<float> springForce(K);
	std::vector<float> tangetLengths(K);
	float3 particlePt = contour.particles[idx];
	float3 startVelocity(0.0f);
	float3 resultantMoment(0.0f);
	const float MAX_FORCE = 0.999f;
	float3 start;
	float dotProd;
	float3 pt2;
	for (int k = 0; k < K; k++) {
		const std::vector<SpringlEdge>& map = nearestNeighbors[idx * K + k];
		start = contour.vertexes[idx * K + k];
		// edge from pivot to magnet
		tanget = (start - particlePt);
		tangetLengths[k] = length(tanget);
		if (tangetLengths[k] > 1E-6f) {
			tanget *= (1.0f / tangetLengths[k]);
		}
		tangets[k] = tanget;
		startVelocity = float3(0.0f);
		// Sum forces
		//unroll loop
		for (SpringlEdge ci : map) {
			//Closest point should be recomputed each time and does not need to be stored
			DistanceToEdgeSqr(start, contour.vertexes[ci.id * K + ci.edgeId],
					contour.vertexes[ci.id * K + (ci.edgeId + 1) % K], &pt2);
			dir = (pt2 - start);
			len = length(dir);
			w = ((len - 2 * PARTICLE_RADIUS) / (EXTENT + 2 * PARTICLE_RADIUS));
			w = atanh(MAX_FORCE * aly::clamp(w, -1.0f, 1.0f));
			startVelocity += (w * dir);
		}
		if (map.size() > 0)
			startVelocity /= float(map.size());
		vertexVelocity[k] = timeStep * startVelocity * SHARPNESS;
		springForce[k] =timeStep * SPRING_CONSTANT
				* (2 * PARTICLE_RADIUS - tangetLengths[k]);
		resultantMoment += cross(vertexVelocity[k], tangets[k]);
	}
	float4x4 rot = MakeRotation(resultantMoment, -length(resultantMoment));
	for (int k = 0; k < K; k++) {
		start = contour.vertexes[idx * K + k] - particlePt;
		dotProd = std::max(
				length(start) + dot(vertexVelocity[k], tangets[k])
						+ springForce[k], 0.001f);
		start = dotProd * tangets[k];

		//disable rotation
		start = Transform(rot, start);
		updates[idx * K + k] = start + particlePt;
	}

}
float3 SpringLevelSet3D::getScaledGradientValue(int i, int j, int k) {
	float v211 = unsignedLevelSet(i + 1, j, k).x;
	float v121 = unsignedLevelSet(i, j + 1, k).x;
	float v101 = unsignedLevelSet(i, j - 1, k).x;
	float v011 = unsignedLevelSet(i - 1, j, k).x;
	float v110 = unsignedLevelSet(i, j, k - 1).x;
	float v112 = unsignedLevelSet(i, j, k + 1).x;
	float v111 = unsignedLevelSet(i, j, k).x;
	float3 grad;
	grad.x = 0.5f * (v211 - v011);
	grad.y = 0.5f * (v121 - v101);
	grad.z = 0.5f * (v112 - v110);
	float len = max(1E-6f, length(grad));
	return -(v111 * grad / len);
}
float3 SpringLevelSet3D::getScaledGradientValue(float i, float j, float k,
		bool signedIso) {
	float3 grad;
	float v211;
	float v121;
	float v101;
	float v011;
	float v111;
	float v110;
	float v112;
	if (signedIso) {
		v211 = std::abs(levelSet(i + 1, j, k).x);
		v121 = std::abs(levelSet(i, j + 1, k).x);
		v101 = std::abs(levelSet(i, j - 1, k).x);
		v011 = std::abs(levelSet(i - 1, j, k).x);
		v110 = std::abs(levelSet(i, j, k - 1).x);
		v112 = std::abs(levelSet(i, j, k + 1).x);
		v111 = std::abs(levelSet(i, j, k).x);
	} else {
		v211 = std::abs(unsignedLevelSet(i + 1, j, k).x);
		v121 = std::abs(unsignedLevelSet(i, j + 1, k).x);
		v101 = std::abs(unsignedLevelSet(i, j - 1, k).x);
		v011 = std::abs(unsignedLevelSet(i - 1, j, k).x);
		v110 = std::abs(unsignedLevelSet(i, j, k - 1).x);
		v112 = std::abs(unsignedLevelSet(i, j, k + 1).x);
		v111 = std::abs(unsignedLevelSet(i, j, k).x);
	}
	grad.x = 0.5f * (v211 - v011);
	grad.y = 0.5f * (v121 - v101);
	grad.y = 0.5f * (v112 - v110);
	float len = max(1E-6f, length(grad));
	return -(v111 * grad / len);
}
void SpringLevelSet3D::distanceFieldMotion(int i, int j, int k, size_t gid) {
	float v111 = swapLevelSet(i, j, k).x;
	if (v111 > 0.5f || v111 < -0.5f) {
		deltaLevelSet[gid] = 0;
		return;
	}
	float v010 = swapLevelSet(i - 1, j, k - 1);
	float v120 = swapLevelSet(i, j + 1, k - 1);
	float v110 = swapLevelSet(i, j, k - 1);
	float v100 = swapLevelSet(i, j - 1, k - 1);
	float v210 = swapLevelSet(i + 1, j, k - 1);
	float v001 = swapLevelSet(i - 1, j - 1, k);
	float v011 = swapLevelSet(i - 1, j, k);
	float v101 = swapLevelSet(i, j - 1, k);
	float v211 = swapLevelSet(i + 1, j, k);
	float v201 = swapLevelSet(i + 1, j - 1, k);
	float v221 = swapLevelSet(i + 1, j + 1, k);
	float v021 = swapLevelSet(i - 1, j + 1, k);
	float v121 = swapLevelSet(i, j + 1, k);
	float v012 = swapLevelSet(i - 1, j, k + 1);
	float v122 = swapLevelSet(i, j + 1, k + 1);
	float v112 = swapLevelSet(i, j, k + 1);
	float v102 = swapLevelSet(i, j - 1, k + 1);
	float v212 = swapLevelSet(i + 1, j, k + 1);

	float DxNeg = v111 - v011;
	float DxPos = v211 - v111;
	float DyNeg = v111 - v101;
	float DyPos = v121 - v111;
	float DzNeg = v111 - v110;
	float DzPos = v112 - v111;
	float DxNegMin = min(DxNeg, 0.0f);
	float DxNegMax = max(DxNeg, 0.0f);
	float DxPosMin = min(DxPos, 0.0f);
	float DxPosMax = max(DxPos, 0.0f);
	float DyNegMin = min(DyNeg, 0.0f);
	float DyNegMax = max(DyNeg, 0.0f);
	float DyPosMin = min(DyPos, 0.0f);
	float DyPosMax = max(DyPos, 0.0f);
	float DzNegMin = min(DzNeg, 0.0f);
	float DzNegMax = max(DzNeg, 0.0f);
	float DzPosMin = min(DzPos, 0.0f);
	float DzPosMax = max(DzPos, 0.0f);

	float GradientSqrPos = DxNegMax * DxNegMax + DxPosMin * DxPosMin
			+ DyNegMax * DyNegMax + DyPosMin * DyPosMin + DzNegMax * DzNegMax
			+ DzPosMin * DzPosMin;
	float GradientSqrNeg = DxPosMax * DxPosMax + DxNegMin * DxNegMin
			+ DyPosMax * DyPosMax + DyNegMin * DyNegMin + DzPosMax * DzPosMax
			+ DzNegMin * DzNegMin;

	float DxCtr = 0.5f * (v211 - v011);
	float DyCtr = 0.5f * (v121 - v101);
	float DzCtr = 0.5f * (v112 - v110);
	float DxxCtr = v211 - v111 - v111 + v011;
	float DyyCtr = v121 - v111 - v111 + v101;
	float DzzCtr = v112 - v111 - v111 + v110;
	float DxyCtr = (v221 - v021 - v201 + v001) * 0.25f;
	float DxzCtr = (v212 - v012 - v210 + v010) * 0.25f;
	float DyzCtr = (v122 - v102 - v120 + v100) * 0.25f;

	float numer = 0.5f
			* ((DyyCtr + DzzCtr) * DxCtr * DxCtr
					+ (DxxCtr + DzzCtr) * DyCtr * DyCtr
					+ (DxxCtr + DyyCtr) * DzCtr * DzCtr
					- 2 * DxCtr * DyCtr * DxyCtr - 2 * DxCtr * DzCtr * DxzCtr
					- 2 * DyCtr * DzCtr * DyzCtr);
	float denom = DxCtr * DxCtr + DyCtr * DyCtr + DzCtr * DzCtr;
	float kappa = 0;
	const float maxCurvatureForce = 10.0f;
	if (std::abs(denom) > 1E-5f) {
		kappa = curvatureParam.toFloat() * numer / denom;
	} else {
		kappa = curvatureParam.toFloat() * numer * sign(denom) * 1E5;
	}
	if (kappa < -maxCurvatureForce) {
		kappa = -maxCurvatureForce;
	} else if (kappa > maxCurvatureForce) {
		kappa = maxCurvatureForce;
	}
	float pressure = 0;
	// Level set force should be the opposite sign of advection force so it
	// moves in the direction of the force.
	float3 grad = getScaledGradientValue(i, j, k);
	float advection = 0;
	// Dot product force with upwind gradient
	if (grad.x > 0) {
		advection = grad.x * DxNeg;
	} else if (grad.x < 0) {
		advection = grad.x * DxPos;
	}
	if (grad.y > 0) {
		advection += grad.y * DyNeg;
	} else if (grad.y < 0) {
		advection += grad.y * DyPos;
	}
	deltaLevelSet[gid] = -advection + kappa;
}
bool SpringLevelSet3D::init() {
	ActiveManifold3D::init();
	if (contour.meshType == MeshType::Triangle) {
		int N = (int) contour.triIndexes.size();
		contour.particles.resize(N);
		contour.vertexes.resize(N * 3);
		for (int n = 0; n < N; n++) {
			uint3 tri = contour.triIndexes[n];
			float3 v1 = contour.vertexLocations[tri.x];
			float3 v2 = contour.vertexLocations[tri.y];
			float3 v3 = contour.vertexLocations[tri.z];
			contour.particles[n] = 0.33333f * (v1 + v2 + v3);
			contour.vertexes[n * 3] = v1;
			contour.vertexes[n * 3 + 1] = v2;
			contour.vertexes[n * 3 + 2] = v3;
		}
	} else if (contour.meshType == MeshType::Quad) {
		int N = (int) contour.quadIndexes.size();
		contour.particles.resize(N);
		contour.vertexes.resize(N * 4);
		for (int n = 0; n < (int) contour.particles.size(); n++) {
			uint4 quad = contour.quadIndexes[n];
			float3 v1 = contour.vertexLocations[quad.x];
			float3 v2 = contour.vertexLocations[quad.y];
			float3 v3 = contour.vertexLocations[quad.z];
			float3 v4 = contour.vertexLocations[quad.w];
			contour.particles[n] = 0.25f * (v1 + v2 + v3 + v4);
			contour.vertexes[n * 4] = v1;
			contour.vertexes[n * 4 + 1] = v2;
			contour.vertexes[n * 4 + 2] = v3;
			contour.vertexes[n * 4 + 3] = v4;
		}
	}
	for (Vector3f& vel : contour.velocities) {
		vel.clear();
		vel.resize(contour.particles.size());
	}
	contour.correspondence = contour.particles;
	std::cout<<"Update Normals "<<std::endl;
	contour.updateNormals();
	contour.setDirty(true);
	if (cache.get() != nullptr) {
		std::cout << "Cache surface" << std::endl;
		Manifold3D* contour = getSurface();
		contour->setFile(
				MakeString() << GetDesktopDirectory() << ALY_PATH_SEPARATOR<< "contour" << std::setw(4) << std::setfill('0') << simulationIteration << ".bin");
	}
	std::cout << "Relax" << std::endl;
	relax();
	//contour.stashSpringls(MakeDesktopFile("init_springls.ply"));
	//contour.stashIsoSurface(MakeDesktopFile("isosurf.ply"));
	std::cout << "update nearest neighbors" << std::endl;
	updateNearestNeighbors();
	std::cout << "Update unsigned" << std::endl;
	updateUnsignedLevelSet();
	//WriteVolumeToFile(MakeDesktopFile("unsigned.xml"), unsignedLevelSet);
	std::cout << "set cache" << std::endl;
	cache->set((int) simulationIteration, contour);
	std::cout<<"Done Init "<<std::endl;
	return true;
}
void SpringLevelSet3D::relax() {
	const int maxIterations = 8;
	const float timeStep = 0.1f;
	updateNearestNeighbors();
	for (int i = 0; i < maxIterations; i++) {
		relax(timeStep);
	}
}
void SpringLevelSet3D::cleanup() {
	ActiveManifold3D::cleanup();
}
bool SpringLevelSet3D::stepInternal() {
	double remaining = timeStep;
	double t = 0.0;
	const int evolveIterations = 8;
	std::cout<<"Step Internal"<<std::endl;
	do {
		float timeStep = advect(std::min(0.33333f, (float) remaining));
		t += (double) timeStep;
		if (resampleEnabled) {
			relax();
		}
		updateUnsignedLevelSet();
		for (int i = 0; i < evolveIterations; i++) {
			updateSignedLevelSet();
		}
		if (resampleEnabled) {
			oldPoints = contour.particles;
			oldCorrespondences = contour.correspondence;
			oldVelocities = contour.velocities;
			contract();
			updateNearestNeighbors();
			int fillCount = 0;
			do {
				updateUnsignedLevelSet();
				fillCount = fill();
				relax();
			} while (fillCount > 0); //Continue filling until all gaps are closed
			contour.updateNormals();
			contour.setDirty(true);
			updateTracking();
		} else {
			std::lock_guard < std::mutex > lockMe(contourLock);
			isoSurface.solve(levelSet, activeList, contour.vertexes,
					contour.meshType, true, 0.0f);
			contour.updateNormals();
			contour.setDirty(true);
			requestUpdateSurface = false;
		}
		remaining = timeStep - t;
	} while (remaining > 1E-5f);
	simulationTime += t;
	simulationIteration++;
	if (cache.get() != nullptr) {
		Manifold3D* contour = getSurface();
		contour->setFile(
				MakeString() << GetDesktopDirectory() << ALY_PATH_SEPARATOR<< "contour" << std::setw(4) << std::setfill('0') << simulationIteration << ".bin");
		cache->set((int) simulationIteration, *contour);
	}
	return (simulationTime < simulationDuration);
}

void SpringLevelSet3D::setup(const aly::ParameterPanePtr& pane) {
	ActiveManifold3D::setup(pane);
	pane->addCheckBox("Re-sampling", resampleEnabled);
}
}
