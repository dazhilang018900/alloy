/*
 * Copyright(C) 2015, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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

#include "AlloyDistanceField.h"
#include "BinaryMinHeap.h"
#include <list>
using namespace std;
namespace aly {
const ubyte1 DistanceField3f::ALIVE = ubyte1((uint8_t) 1);
const ubyte1 DistanceField3f::NARROW_BAND = ubyte1((uint8_t) 2);
const ubyte1 DistanceField3f::FAR_AWAY = ubyte1((uint8_t) 3);
const float DistanceField3f::DISTANCE_UNDEFINED =
		std::numeric_limits<float>::max();

const ubyte1 DistanceField2f::ALIVE = ubyte1((uint8_t) 1);
const ubyte1 DistanceField2f::NARROW_BAND = ubyte1((uint8_t) 2);
const ubyte1 DistanceField2f::FAR_AWAY = ubyte1((uint8_t) 3);
const float DistanceField2f::DISTANCE_UNDEFINED =
		std::numeric_limits<float>::max();
/*
 Sethian, J. A. (1999). Level set methods and fast marching methods: evolving interfaces
 in computational geometry, fluid mechanics, computer vision, and materials science (Vol. 3).
 Cambridge university press.
 */
struct DfElem {
	float dist;
	ubyte label;
	int8_t sign;
	DfElem(float dist = DistanceField3f::DISTANCE_UNDEFINED, ubyte label =
			DistanceField3f::FAR_AWAY.x, byte sign = 0) :
			dist(dist), label(label), sign(sign) {

	}
};
float DistanceField3f::march(float IMv, float IPv, float JMv, float JPv,
		float KMv, float KPv, int IMl, int IPl, int JMl, int JPl, int KMl,
		int KPl) {
	double s, s2;
	double tmp;
	int count;
	s = 0;
	s2 = 0;
	count = 0;
	if (IMl == ALIVE && IPl == ALIVE) {
		tmp = std::min(IMv, IPv);
		s += tmp;
		s2 += tmp * tmp;
		count++;
	} else if (IMl == ALIVE) {
		s += IMv;
		s2 += IMv * IMv;
		count++;
	} else if (IPl == ALIVE) {
		s += IPv;
		s2 += IPv * IPv;
		count++;
	}
	if (JMl == ALIVE && JPl == ALIVE) {
		tmp = std::min(JMv, JPv);
		s += tmp;
		s2 += tmp * tmp;
		count++;
	} else if (JMl == ALIVE) {
		s += JMv;
		s2 += JMv * JMv;
		count++;
	} else if (JPl == ALIVE) {
		s += JPv;
		s2 += JPv * JPv;
		count++;
	}
	if (KMl == ALIVE && KPl == ALIVE) {
		tmp = std::min(KMv, KPv);
		s += tmp;
		s2 += tmp * tmp;
		count++;
	} else if (KMl == ALIVE) {
		s += KMv;
		s2 += KMv * KMv;
		count++;
	} else if (KPl == ALIVE) {
		s += KPv;
		s2 += KPv * KPv;
		count++;
	}
	tmp = (s + std::sqrt(std::max(0.0, s * s - count * (s2 - 1.0f)))) / count;
	return (float) tmp;
}
void DistanceField3f::solve(const Volume1f& vol, Volume1f& distVol,
		float maxDistance) {
	const int rows = vol.rows;
	const int cols = vol.cols;
	const int slices = vol.slices;
	BinaryMinHeap<float, 3> heap;
	distVol.resize(rows, cols, slices);
	distVol.set(float1(DISTANCE_UNDEFINED));

	static const int neighborsX[6] = { 1, 0, -1, 0, 0, 0 };
	static const int neighborsY[6] = { 0, 1, 0, -1, 0, 0 };
	static const int neighborsZ[6] = { 0, 0, 0, 0, 1, -1 };
	std::list<VoxelIndex> voxelList;
	VoxelIndex* he = nullptr;

	Volume1ub labelVol(rows, cols, slices);
	Volume1b signVol(rows, cols, slices);
	labelVol.set(FAR_AWAY);
	size_t countAlive = 0;
#pragma omp parallel for
	for (int k = 0; k < slices; k++) {
		int LX, HX, LY, HY, LZ, HZ;
		short NSFlag, WEFlag, FBFlag;
		float s = 0, t = 0, w = 0;
		float JMv = 0, JPv = 0, IMv = 0, IPv = 0, KPv = 0, KMv = 0, Cv = 0;
		for (int j = 0; j < cols; j++) {
			for (int i = 0; i < rows; i++) {
				Cv = vol(i, j, k).x;
				if (Cv == 0) {
					signVol(i, j, k).x = 0;
					distVol(i, j, k).x = 0;
					labelVol(i, j, k) = ALIVE;
#pragma omp atomic
					countAlive++;
				} else {
					if (Cv != DISTANCE_UNDEFINED) {
						signVol(i, j, k).x = (int8_t) aly::sign(Cv);
						LX = (i == 0) ? 1 : 0;
						HX = (i == (rows - 1)) ? 1 : 0;

						LY = (j == 0) ? 1 : 0;
						HY = (j == (cols - 1)) ? 1 : 0;

						LZ = (k == 0) ? 1 : 0;
						HZ = (k == (slices - 1)) ? 1 : 0;

						NSFlag = 0;
						WEFlag = 0;
						FBFlag = 0;

						JMv = vol(i, j - 1 + LY, k).x;
						JPv = vol(i, j + 1 - HY, k).x;
						IMv = vol(i - 1 + LX, j, k).x;
						IPv = vol(i + 1 - HX, j, k).x;
						KPv = vol(i, j, k + 1 - HZ).x;
						KMv = vol(i, j, k - 1 + LZ).x;
						if (JMv * Cv < 0 && JMv != DISTANCE_UNDEFINED) {
							NSFlag = 1;
							s = JMv;
						}
						if (JPv * Cv < 0 && JPv != DISTANCE_UNDEFINED) {
							if (NSFlag == 0) {
								NSFlag = 1;
								s = JPv;
							} else {
								s = (std::abs(JMv) > std::abs(JPv)) ? JMv : JPv;
							}
						}
						if (IMv * Cv < 0 && IMv != DISTANCE_UNDEFINED) {
							WEFlag = 1;
							t = IMv;
						}
						if (IPv * Cv < 0 && IPv != DISTANCE_UNDEFINED) {
							if (WEFlag == 0) {
								WEFlag = 1;
								t = IPv;
							} else {
								t = (std::abs(IPv) > std::abs(IMv)) ? IPv : IMv;
							}
						}
						if (KPv * Cv < 0 && KPv != DISTANCE_UNDEFINED) {
							FBFlag = 1;
							w = KPv;
						}
						if (KMv * Cv < 0 && KMv != DISTANCE_UNDEFINED) {
							if (FBFlag == 0) {
								FBFlag = 1;
								w = KMv;
							} else {
								w = (std::abs(KPv) > std::abs(KMv)) ? KPv : KMv;
							}
						}
						float result = 0;
						if (NSFlag != 0) {
							s = Cv / (Cv - s);
							result += 1.0f / (s * s);
						}
						if (WEFlag != 0) {
							t = Cv / (Cv - t);
							result += 1.0f / (t * t);
						}
						if (FBFlag != 0) {
							w = Cv / (Cv - w);
							result += 1.0f / (w * w);
						}
						if (result == 0) {
							distVol(i, j, k).x = 0;
						} else {
#pragma omp atomic
							countAlive++;
							labelVol(i, j, k) = ALIVE;
							result = std::sqrt(result);
							distVol(i, j, k).x = (float) (1.0 / result);
						}
					} else {
						signVol(i, j, k).x = 0;
					}
				}
			}
		}
	}
	heap.reserve(countAlive);
	{
		int koff;
		int nj, nk, ni;
		float newvalue;
		float JMv = 0, JPv = 0, KMv = 0, KPv = 0, IPv = 0, IMv = 0;
		int8_t JMs = 0, JPs = 0, KMs = 0, KPs = 0, IPs = 0, IMs = 0;
		ubyte1 JMl = ubyte1((uint8_t) 0);
		ubyte1 JPl = ubyte1((uint8_t) 0);
		ubyte1 KMl = ubyte1((uint8_t) 0);
		ubyte1 KPl = ubyte1((uint8_t) 0);
		ubyte1 IPl = ubyte1((uint8_t) 0);
		ubyte1 IMl = ubyte1((uint8_t) 0);
		for (int k = 0; k < slices; k++) {
			for (int j = 0; j < cols; j++) {
				for (int i = 0; i < rows; i++) {
					if (labelVol(i, j, k) != ALIVE) {
						continue;
					}
					for (koff = 0; koff < 6; koff++) {
						ni = i + neighborsX[koff];
						nj = j + neighborsY[koff];
						nk = k + neighborsZ[koff];
						if (nj < 0 || nj >= cols || nk < 0 || nk >= slices
								|| ni < 0 || ni >= rows) {
							continue;
						}
						if (labelVol(ni, nj, nk) != FAR_AWAY) {
							continue;
						}
						labelVol(ni, nj, nk) = NARROW_BAND;
						if (nj > 0) {
							JMv = distVol(ni, nj - 1, nk).x;
							JMs = signVol(ni, nj - 1, nk).x;
							JMl = labelVol(ni, nj - 1, nk);
						} else {
							JMs = 0;
							JMl = 0;
						}
						if (nj < cols - 1) {
							JPv = distVol(ni, nj + 1, nk).x;
							JPs = signVol(ni, nj + 1, nk).x;
							JPl = labelVol(ni, nj + 1, nk);
						} else {
							JPs = 0;
							JPl = 0;
						}
						if (nk < slices - 1) {
							KPv = distVol(ni, nj, nk + 1).x;
							KPs = signVol(ni, nj, nk + 1).x;
							KPl = labelVol(ni, nj, nk + 1);
						} else {
							KPs = 0;
							KPl = 0;
						}
						if (nk > 0) {
							KMv = distVol(ni, nj, nk - 1).x;
							KMs = signVol(ni, nj, nk - 1).x;
							KMl = labelVol(ni, nj, nk - 1);
						} else {
							KMs = 0;
							KMl = 0;
						}
						if (ni < rows - 1) {
							IPv = distVol(ni + 1, nj, nk).x;
							IPs = signVol(ni + 1, nj, nk).x;
							IPl = labelVol(ni + 1, nj, nk);
						} else {
							IPs = 0;
							IPl = 0;
						}
						if (ni > 0) {
							IMv = distVol(ni - 1, nj, nk).x;
							IMs = signVol(ni - 1, nj, nk).x;
							IMl = labelVol(ni - 1, nj, nk);
						} else {
							IMs = 0;
							IMl = 0;
						}
						signVol(ni, nj, nk).x = aly::sign(
								JMs + JPs + IMs + IPs + KPs + KMs);
						newvalue = march(JMv, JPv, KPv, KMv, IPv, IMv, JMl, JPl,
								KPl, KMl, IPl, IMl);
						distVol(ni, nj, nk).x = (float) (newvalue);
						voxelList.push_back(
								VoxelIndex(Coord(ni, nj, nk),
										(float) newvalue));
						heap.add(&voxelList.back());
					}
				};
			}
		}
		while (!heap.isEmpty()) {
			int i, j, k;
			he = heap.remove();
			i = he->index[0];
			j = he->index[1];
			k = he->index[2];
			if (he->value > maxDistance) {
				break;
			}
			distVol(i, j, k).x = (he->value);
			labelVol(i, j, k) = ALIVE;
			for (koff = 0; koff < 6; koff++) {
				ni = i + neighborsX[koff];
				nj = j + neighborsY[koff];
				nk = k + neighborsZ[koff];
				if (nj < 0 || nj >= cols || nk < 0 || nk >= slices || ni < 0
						|| ni >= rows) {
					continue;
				}
				ubyte1& label = labelVol(ni, nj, nk);
				if (label == ALIVE) {
					continue;
				}
				if (nj > 0) {
					JMv = distVol(ni, nj - 1, nk).x;
					JMs = signVol(ni, nj - 1, nk).x;
					JMl = labelVol(ni, nj - 1, nk);
				} else {
					JMs = 0;
					JMl = 0;
				}

				if (nj < cols - 1) {
					JPv = distVol(ni, nj + 1, nk).x;
					JPs = signVol(ni, nj + 1, nk).x;
					JPl = labelVol(ni, nj + 1, nk);
				} else {
					JPs = 0;
					JPl = 0;
				}

				if (nk < slices - 1) {
					KPv = distVol(ni, nj, nk + 1).x;
					KPs = signVol(ni, nj, nk + 1).x;
					KPl = labelVol(ni, nj, nk + 1);
				} else {
					KPs = 0;
					KPl = 0;
				}

				if (nk > 0) {
					KMv = distVol(ni, nj, nk - 1).x;
					KMs = signVol(ni, nj, nk - 1).x;
					KMl = labelVol(ni, nj, nk - 1);
				} else {
					KMs = 0;
					KMl = 0;
				}

				if (ni < rows - 1) {
					IPv = distVol(ni + 1, nj, nk).x;
					IPs = signVol(ni + 1, nj, nk).x;
					IPl = labelVol(ni + 1, nj, nk);
				} else {
					IPs = 0;
					IPl = 0;
				}

				if (ni > 0) {
					IMv = distVol(ni - 1, nj, nk).x;
					IMs = signVol(ni - 1, nj, nk).x;
					IMl = labelVol(ni - 1, nj, nk);
				} else {
					IMs = 0;
					IMl = 0;
				}
				signVol(ni, nj, nk).x = aly::sign(
						JMs + JPs + IMs + IPs + KPs + KMs);
				newvalue = march(JMv, JPv, KPv, KMv, IPv, IMv, JMl, JPl, KPl,
						KMl, IPl, IMl);
				voxelList.push_back(
						VoxelIndex(Coord(ni, nj, nk), (float) newvalue));
				VoxelIndex* vox = &voxelList.back();
				if (label == NARROW_BAND) {
					heap.change(Coord(ni, nj, nk), vox);
				} else {
					heap.add(vox);
					label = NARROW_BAND;
				}
			}
		}
	}
#pragma omp parallel for
	for (int k = 0; k < slices; k++) {
		for (int j = 0; j < cols; j++) {
			for (int i = 0; i < rows; i++) {
				int8_t s = signVol(i, j, k).x;
				if (labelVol(i, j, k) != ALIVE) {
					distVol(i, j, k).x = maxDistance
							* aly::sign(vol(i, j, k).x);
				} else {
					distVol(i, j, k).x *= s;
				}

			}
		}
	}
	heap.clear();
}

void DistanceField3f::solve(EndlessGridFloat& vol, float maxDistance) {
	BinaryMinHeap<float, 3> heap;
	float BG_VALUE = maxDistance + 0.5f;
	EndlessGrid<DfElem> distVol(vol.getLevelSizes(),
			DfElem(BG_VALUE, FAR_AWAY, 0));
	vol.setBackgroundValue(BG_VALUE);
	static const int neighborsX[6] = { 1, 0, -1, 0, 0, 0 };
	static const int neighborsY[6] = { 0, 1, 0, -1, 0, 0 };
	static const int neighborsZ[6] = { 0, 0, 0, 0, 1, -1 };
	std::list<VoxelIndex> voxelList;
	VoxelIndex* he = nullptr;
	size_t countAlive = 0;
	int dim;
	int3 pos;

	short NSFlag, WEFlag, FBFlag;
	float s = 0, t = 0, w = 0;
	float JMv = 0, JPv = 0, IMv = 0, IPv = 0, KPv = 0, KMv = 0, Cv = 0;
	int i, j, k;
	for (EndlessNodeFloat* leaf : vol.getLeafNodes()) {
		dim = leaf->dim;
		pos = leaf->location;
		for (int kk = 0; kk < dim; kk++) {
			for (int jj = 0; jj < dim; jj++) {
				for (int ii = 0; ii < dim; ii++) {
					i = pos.x + ii;
					j = pos.y + jj;
					k = pos.z + kk;
					Cv = vol.getLeafValue(i, j, k, leaf);
					if (Cv == 0) {
						DfElem& elem = distVol.getLeafValue(i, j, k);
						elem.dist = 0;
						elem.sign = 0;
						elem.label = ALIVE;
						countAlive++;
					} else {
						DfElem& elem = distVol.getLeafValue(i, j, k);
						if (std::abs(Cv) < BG_VALUE) {
							elem.sign = (int8_t) aly::sign(Cv);
							NSFlag = 0;
							WEFlag = 0;
							FBFlag = 0;
							JMv = vol.getLeafValue(i, j - 1, k, leaf);
							JPv = vol.getLeafValue(i, j + 1, k, leaf);
							IMv = vol.getLeafValue(i - 1, j, k, leaf);
							IPv = vol.getLeafValue(i + 1, j, k, leaf);
							KPv = vol.getLeafValue(i, j, k + 1, leaf);
							KMv = vol.getLeafValue(i, j, k - 1, leaf);
							if (JMv * Cv < 0 && JMv != BG_VALUE) {
								NSFlag = 1;
								s = JMv;
							}
							if (JPv * Cv < 0 && JPv != BG_VALUE) {
								if (NSFlag == 0) {
									NSFlag = 1;
									s = JPv;
								} else {
									s = (std::abs(JMv) > std::abs(JPv)) ?
											JMv : JPv;
								}
							}
							if (IMv * Cv < 0 && IMv != BG_VALUE) {
								WEFlag = 1;
								t = IMv;
							}
							if (IPv * Cv < 0 && IPv != BG_VALUE) {
								if (WEFlag == 0) {
									WEFlag = 1;
									t = IPv;
								} else {
									t = (std::abs(IPv) > std::abs(IMv)) ?
											IPv : IMv;
								}
							}
							if (KPv * Cv < 0 && KPv != BG_VALUE) {
								FBFlag = 1;
								w = KPv;
							}
							if (KMv * Cv < 0 && KMv != BG_VALUE) {
								if (FBFlag == 0) {
									FBFlag = 1;
									w = KMv;
								} else {
									w = (std::abs(KPv) > std::abs(KMv)) ?
											KPv : KMv;
								}
							}
							float result = 0;
							if (NSFlag != 0) {
								s = Cv / (Cv - s);
								result += 1.0f / (s * s);
							}
							if (WEFlag != 0) {
								t = Cv / (Cv - t);
								result += 1.0f / (t * t);
							}
							if (FBFlag != 0) {
								w = Cv / (Cv - w);
								result += 1.0f / (w * w);
							}
							if (result == 0) {
								elem.dist = 0;
							} else {
								countAlive++;
								elem.label = ALIVE;
								result = std::sqrt(result);
								elem.dist = (float) (1.0f / result);
							}
						} else {
							elem.sign = 0;
						}
					}
				}
			}
		}
	}
	heap.reserve(countAlive);
	int koff;
	int nj, nk, ni;
	float newvalue;
	JMv = 0;
	JPv = 0;
	KMv = 0;
	KPv = 0;
	IPv = 0;
	IMv = 0;
	int8_t JMs = 0, JPs = 0, KMs = 0, KPs = 0, IPs = 0, IMs = 0;
	ubyte JMl = 0;
	ubyte JPl = 0;
	ubyte KMl = 0;
	ubyte KPl = 0;
	ubyte IPl = 0;
	ubyte IMl = 0;
	for (EndlessNode<DfElem>* leaf : distVol.getLeafNodes()) {
		dim = leaf->dim;
		pos = leaf->location;
		for (int kk = 0; kk < dim; kk++) {
			for (int jj = 0; jj < dim; jj++) {
				for (int ii = 0; ii < dim; ii++) {
					i = pos.x + ii;
					j = pos.y + jj;
					k = pos.z + kk;
					DfElem& elem = distVol.getLeafValue(i, j, k);
					if (elem.label != ALIVE) {
						continue;
					}
					for (koff = 0; koff < 6; koff++) {
						ni = i + neighborsX[koff];
						nj = j + neighborsY[koff];
						nk = k + neighborsZ[koff];
						DfElem& nelem = distVol.getLeafValue(ni, nj, nk);
						if (nelem.label != FAR_AWAY) {
							continue;
						}
						nelem.label = NARROW_BAND;
						DfElem JM = distVol.getLeafValue(ni, nj - 1, nk, leaf);
						JMv = JM.dist;
						JMs = JM.sign;
						JMl = JM.label;

						DfElem JP = distVol.getLeafValue(ni, nj + 1, nk, leaf);
						JPv = JP.dist;
						JPs = JP.sign;
						JPl = JP.label;

						DfElem KP = distVol.getLeafValue(ni, nj, nk + 1, leaf);
						KPv = KP.dist;
						KPs = KP.sign;
						KPl = KP.label;

						DfElem KM = distVol.getLeafValue(ni, nj, nk - 1, leaf);
						KMv = KM.dist;
						KMs = KM.sign;
						KMl = KM.label;

						DfElem IP = distVol.getLeafValue(ni + 1, nj, nk, leaf);
						IPv = IP.dist;
						IPs = IP.sign;
						IPl = IP.label;

						DfElem IM = distVol.getLeafValue(ni - 1, nj, nk, leaf);
						IMv = IM.dist;
						IMs = IM.sign;
						IMl = IM.label;

						nelem.sign = aly::sign(
								JMs + JPs + IMs + IPs + KPs + KMs);
						newvalue = march(JMv, JPv, KPv, KMv, IPv, IMv, JMl, JPl,
								KPl, KMl, IPl, IMl);
						nelem.dist = newvalue;
						voxelList.push_back(
								VoxelIndex(Coord(ni, nj, nk),
										(float) newvalue));
						heap.add(&voxelList.back());
					}
				}
			}
		}
	}
	vol.clear();
	vol.setBackgroundValue(BG_VALUE);
	while (!heap.isEmpty()) {
		int i, j, k;
		he = heap.remove();
		i = he->index[0];
		j = he->index[1];
		k = he->index[2];
		if (he->value > maxDistance) {
			break;
		}
		DfElem elem;
		EndlessNode<DfElem>* leaf = nullptr;
		distVol.getLeafValue(i, j, k, leaf, elem);
		elem.dist = he->value;
		elem.label = ALIVE;
		distVol.setLeafValue(i, j, k, elem, leaf);
		for (koff = 0; koff < 6; koff++) {
			ni = i + neighborsX[koff];
			nj = j + neighborsY[koff];
			nk = k + neighborsZ[koff];
			DfElem& nelem = distVol.getLeafValue(ni, nj, nk);
			if (nelem.label == ALIVE) {
				continue;
			}
			DfElem JM = distVol.getLeafValue(ni, nj - 1, nk, leaf);
			JMv = JM.dist;
			JMs = JM.sign;
			JMl = JM.label;

			DfElem JP = distVol.getLeafValue(ni, nj + 1, nk, leaf);
			JPv = JP.dist;
			JPs = JP.sign;
			JPl = JP.label;

			DfElem KP = distVol.getLeafValue(ni, nj, nk + 1, leaf);
			KPv = KP.dist;
			KPs = KP.sign;
			KPl = KP.label;

			DfElem KM = distVol.getLeafValue(ni, nj, nk - 1, leaf);
			KMv = KM.dist;
			KMs = KM.sign;
			KMl = KM.label;

			DfElem IP = distVol.getLeafValue(ni + 1, nj, nk, leaf);
			IPv = IP.dist;
			IPs = IP.sign;
			IPl = IP.label;

			DfElem IM = distVol.getLeafValue(ni - 1, nj, nk, leaf);
			IMv = IM.dist;
			IMs = IM.sign;
			IMl = IM.label;

			nelem.sign = aly::sign(JMs + JPs + IMs + IPs + KPs + KMs);
			newvalue = march(JMv, JPv, KPv, KMv, IPv, IMv, JMl, JPl, KPl, KMl,
					IPl, IMl);
			voxelList.push_back(
					VoxelIndex(Coord(ni, nj, nk), (float) newvalue));
			VoxelIndex* vox = &voxelList.back();
			if (nelem.label == NARROW_BAND) {
				heap.change(Coord(ni, nj, nk), vox);
			} else {
				heap.add(vox);
				nelem.label = NARROW_BAND;
			}
		}
	}
	heap.clear();
	for (EndlessNode<DfElem>* leaf : distVol.getLeafNodes()) {
		dim = leaf->dim;
		pos = leaf->location;
		for (int kk = 0; kk < dim; kk++) {
			for (int jj = 0; jj < dim; jj++) {
				for (int ii = 0; ii < dim; ii++) {
					i = pos.x + ii;
					j = pos.y + jj;
					k = pos.z + kk;
					DfElem* elem = distVol.getLeafValuePtr(i, j, k, leaf);
					if (elem->label == ALIVE) {
						vol.getLeafValue(i, j, k) = elem->dist * elem->sign;
					}
				}
			}
		}
	}
}
float DistanceField2f::march(float IMv, float IPv, float JMv, float JPv,
		int IMl, int IPl, int JMl, int JPl) {
	double s, s2;
	double tmp;
	int count;
	s = 0;
	s2 = 0;
	count = 0;
	if (IMl == ALIVE && IPl == ALIVE) {
		tmp = std::min(IMv, IPv);
		s += tmp;
		s2 += tmp * tmp;
		count++;
	} else if (IMl == ALIVE) {
		s += IMv;
		s2 += IMv * IMv;
		count++;
	} else if (IPl == ALIVE) {
		s += IPv;
		s2 += IPv * IPv;
		count++;
	}
	if (JMl == ALIVE && JPl == ALIVE) {
		tmp = std::min(JMv, JPv);
		s += tmp;
		s2 += tmp * tmp;
		count++;
	} else if (JMl == ALIVE) {
		s += JMv;
		s2 += JMv * JMv;
		count++;
	} else if (JPl == ALIVE) {
		s += JPv;
		s2 += JPv * JPv;
		count++;
	}
	tmp = (s + std::sqrt(std::max(0.0, s * s - count * (s2 - 1.0f)))) / count;
	return (float) tmp;
}

void DistanceField2f::solve(const Image1f& vol, Image1f& distVol,
		float maxDistance) {
	const int width = vol.width;
	const int height = vol.height;
	BinaryMinHeap<float, 2> heap;
	distVol.resize(width, height);
	distVol.set(float1(DISTANCE_UNDEFINED));

	static const int neighborsX[4] = { 1, 0, -1, 0 };
	static const int neighborsY[4] = { 0, 1, 0, -1 };
	std::list<PixelIndex> voxelList;
	PixelIndex* he = nullptr;

	Image1ub labelVol(width, height);
	Image1b signVol(width, height);
	labelVol.set(FAR_AWAY);
	size_t countAlive = 0;
#pragma omp parallel for
	for (int j = 0; j < height; j++) {
		int LX, HX, LY, HY;
		short NSFlag, WEFlag;
		float s = 0, t = 0;
		float JMv = 0, JPv = 0, IMv = 0, IPv = 0, Cv = 0;
		for (int i = 0; i < width; i++) {
			Cv = vol(i, j).x;
			if (Cv == 0) {
				signVol(i, j).x = 0;
				distVol(i, j).x = 0;
				labelVol(i, j) = ALIVE;
#pragma omp atomic
				countAlive++;
			} else {
				if (Cv != DISTANCE_UNDEFINED) {
					signVol(i, j).x = (int8_t) aly::sign(Cv);
					LX = (i == 0) ? 1 : 0;
					HX = (i == (width - 1)) ? 1 : 0;

					LY = (j == 0) ? 1 : 0;
					HY = (j == (height - 1)) ? 1 : 0;

					NSFlag = 0;
					WEFlag = 0;

					JMv = vol(i, j - 1 + LY).x;
					JPv = vol(i, j + 1 - HY).x;
					IMv = vol(i - 1 + LX, j).x;
					IPv = vol(i + 1 - HX, j).x;
					if (JMv * Cv < 0 && JMv != DISTANCE_UNDEFINED) {
						NSFlag = 1;
						s = JMv;
					}
					if (JPv * Cv < 0 && JPv != DISTANCE_UNDEFINED) {
						if (NSFlag == 0) {
							NSFlag = 1;
							s = JPv;
						} else {
							s = (std::abs(JMv) > std::abs(JPv)) ? JMv : JPv;
						}
					}
					if (IMv * Cv < 0 && IMv != DISTANCE_UNDEFINED) {
						WEFlag = 1;
						t = IMv;
					}
					if (IPv * Cv < 0 && IPv != DISTANCE_UNDEFINED) {
						if (WEFlag == 0) {
							WEFlag = 1;
							t = IPv;
						} else {
							t = (std::abs(IPv) > std::abs(IMv)) ? IPv : IMv;
						}
					}
					float result = 0;
					if (NSFlag != 0) {
						s = Cv / (Cv - s);
						result += 1.0f / (s * s);
					}
					if (WEFlag != 0) {
						t = Cv / (Cv - t);
						result += 1.0f / (t * t);
					}
					if (result == 0) {
						distVol(i, j).x = 0;
					} else {
#pragma omp atomic
						countAlive++;
						labelVol(i, j) = ALIVE;
						result = std::sqrt(result);
						distVol(i, j).x = (float) (1.0 / result);
					}
				} else {
					signVol(i, j).x = 0;
				}
			}
		}
	}

	heap.reserve(countAlive);
	{
		int koff;
		int nj, ni;
		float newvalue;
		float JMv = 0, JPv = 0, IMv = 0, IPv = 0;
		int8_t JMs = 0, JPs = 0, IMs = 0, IPs = 0;
		ubyte1 JMl = ubyte1((uint8_t) 0);
		ubyte1 JPl = ubyte1((uint8_t) 0);
		ubyte1 IMl = ubyte1((uint8_t) 0);
		ubyte1 IPl = ubyte1((uint8_t) 0);
		for (int j = 0; j < height; j++) {
			for (int i = 0; i < width; i++) {
				if (labelVol(i, j) != ALIVE) {
					continue;
				}
				for (koff = 0; koff < 4; koff++) {
					ni = i + neighborsX[koff];
					nj = j + neighborsY[koff];
					if (nj < 0 || nj >= height || ni < 0 || ni >= width) {
						continue;
					}
					if (labelVol(ni, nj) != FAR_AWAY) {
						continue;
					}
					labelVol(ni, nj) = NARROW_BAND;
					if (nj > 0) {
						JMv = distVol(ni, nj - 1).x;
						JMs = signVol(ni, nj - 1).x;
						JMl = labelVol(ni, nj - 1);
					} else {
						JMv = maxDistance;
						JMs = 0;
						JMl = 0;
					}
					if (nj < height - 1) {
						JPv = distVol(ni, nj + 1).x;
						JPs = signVol(ni, nj + 1).x;
						JPl = labelVol(ni, nj + 1);
					} else {
						JPv = maxDistance;
						JPs = 0;
						JPl = 0;
					}
					if (ni < width - 1) {
						IPv = distVol(ni + 1, nj).x;
						IPs = signVol(ni + 1, nj).x;
						IPl = labelVol(ni + 1, nj);
					} else {
						IPv = maxDistance;
						IPs = 0;
						IPl = 0;
					}
					if (ni > 0) {
						IMv = distVol(ni - 1, nj).x;
						IMs = signVol(ni - 1, nj).x;
						IMl = labelVol(ni - 1, nj);
					} else {
						IMv = maxDistance;
						IMs = 0;
						IMl = 0;
					}
					signVol(ni, nj).x = aly::sign(JMs + JPs + IPs + IMs);
					newvalue = march(JMv, JPv, IMv, IPv, JMl, JPl, IMl, IPl);
					distVol(ni, nj).x = (float) (newvalue);
					voxelList.push_back(
							PixelIndex(Coord(ni, nj), (float) newvalue));
					heap.add(&voxelList.back());
				}
			}
		}

		while (!heap.isEmpty()) {
			int i, j;
			he = heap.remove();
			i = he->index[0];
			j = he->index[1];
			if (he->value > maxDistance) {
				break;
			}
			distVol(i, j).x = (he->value);
			labelVol(i, j) = ALIVE;
			for (koff = 0; koff < 4; koff++) {
				ni = i + neighborsX[koff];
				nj = j + neighborsY[koff];
				if (nj < 0 || nj >= height || ni < 0 || ni >= width) {
					continue;
				}
				ubyte1& label = labelVol(ni, nj);
				if (label == ALIVE) {
					continue;
				}
				if (nj > 0) {
					JMv = distVol(ni, nj - 1).x;
					JMs = signVol(ni, nj - 1).x;
					JMl = labelVol(ni, nj - 1);
				} else {
					JMv = maxDistance;
					JMs = 0;
					JMl = 0;
				}
				if (nj < height - 1) {
					JPv = distVol(ni, nj + 1).x;
					JPs = signVol(ni, nj + 1).x;
					JPl = labelVol(ni, nj + 1);
				} else {
					JPv = maxDistance;
					JPs = 0;
					JPl = 0;
				}

				if (ni < width - 1) {
					IPv = distVol(ni + 1, nj).x;
					IPs = signVol(ni + 1, nj).x;
					IPl = labelVol(ni + 1, nj);
				} else {
					IPv = maxDistance;
					IPs = 0;
					IPl = 0;
				}
				if (ni > 0) {
					IMv = distVol(ni - 1, nj).x;
					IMs = signVol(ni - 1, nj).x;
					IMl = labelVol(ni - 1, nj);
				} else {
					IMv = maxDistance;
					IMs = 0;
					IMl = 0;
				}
				signVol(ni, nj).x = aly::sign(JMs + JPs + IPs + IMs);
				newvalue = march(JMv, JPv, IMv, IPv, JMl, JPl, IMl, IPl);
				voxelList.push_back(
						PixelIndex(Coord(ni, nj), (float) newvalue));
				PixelIndex* vox = &voxelList.back();
				if (label == NARROW_BAND) {
					heap.change(Coord(ni, nj), vox);
				} else {
					heap.add(vox);
					label = NARROW_BAND;
				}
			}
		}
	}
#pragma omp parallel for
	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			int8_t s = signVol(i, j).x;
			if (labelVol(i, j) != ALIVE) {
				distVol(i, j).x = maxDistance * aly::sign(vol(i, j).x);
			} else {
				distVol(i, j).x *= s;
			}

		}
	}
	heap.clear();
}
}
