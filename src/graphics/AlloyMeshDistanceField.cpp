/*
 * AlloyMeshDistanceField.cpp
 *
 *  Created on: Nov 2, 2018
 *      Author: blake
 */

#include "AlloyMeshDistanceField.h"
namespace aly {
const uint8_t AlloyMeshDistanceField::ACTIVE = 1;
const uint8_t AlloyMeshDistanceField::NARROW_BAND = 2;
const uint8_t AlloyMeshDistanceField::FAR_AWAY = 3;
const float AlloyMeshDistanceField::MAX_VALUE = 1E30f;

void AlloyMeshDistanceField::solve(const aly::Mesh& mesh,
		const std::vector<size_t>& zeroIndexes,
		std::vector<float>& distanceField, float maxDistance) {
	MakeOrderedVertexNeighborTable(mesh, nbrTable, false);
	distanceField.resize(mesh.vertexLocations.size());
	float tmpValue, newValue;
	VertexIndexed* he;
	int heapindex;
	//Speed of evolving front, could be variable
	const float currentF = 1.0f;
	size_t VN;
	size_t i, j, k;
	size_t NN, NN1, NN2, n0, n1;
	uint32_t VId, neighVId;
	VN = nbrTable.size();
	std::vector<uint8_t> label(VN);
	indexes.resize(VN);
	for (i = 0; i < VN; i++) {
		label[i] = FAR_AWAY;
		distanceField[i] = MAX_VALUE;
		indexes[i].index=i;
	}
	for (i = 0; i < zeroIndexes.size(); i++) {
		label[zeroIndexes[i]] = ACTIVE; /* all vertices on contour are alive */
		distanceField[zeroIndexes[i]] = 0; /* all vertices on contour have distance zero */
	}
	BinaryMinHeapPtr<float> heap(VN);
	for (i = 0; i < VN; i++) {
		if (label[i] == ACTIVE) {
			for (uint32_t neighVId : nbrTable[i]) {
				if (label[neighVId] == FAR_AWAY) {
					label[neighVId] = NARROW_BAND;
					std::vector<uint32_t> nbrs(nbrTable[neighVId].begin(),nbrTable[neighVId].end());
					newValue = MAX_VALUE;
					NN2 = nbrs.size();
					for (k = 0; k < NN2; k++) {
						n0 = nbrs[k];
						n1 = nbrs[(k + 1) % NN2];
						tmpValue = ComputeDistance(neighVId, n0, n1, mesh,
								distanceField, label, currentF, heap);
						if (tmpValue < newValue)
							newValue = tmpValue;
					}
					distanceField[neighVId] = newValue;
					VertexIndexed* vox =&indexes[neighVId];
					vox->value=newValue;
					vox->index = neighVId;
					heap.add(vox);

				}
			}
		}
	}
	while (!heap.isEmpty()) {
		he = heap.remove();
		VId = he->index;
		distanceField[VId] = he->value;
		label[VId] = ACTIVE;
		for (uint32_t neighVId : nbrTable[VId]) {
			if (label[neighVId] != ACTIVE) {
				std::vector<uint32_t> nbrs(nbrTable[neighVId].begin(),nbrTable[neighVId].end());
				NN2 = nbrs.size();
				newValue = MAX_VALUE;
				for (j = 0; j < NN2; j++) {
					n0 = nbrs[j];
					n1 = nbrs[(j + 1) % NN2];
					tmpValue = ComputeDistance(neighVId, n0, n1, mesh,
							distanceField, label, currentF, heap);
					if (tmpValue < newValue)
						newValue = tmpValue;
				}

				VertexIndexed* vox =&indexes[neighVId];
				vox->value=newValue;
				if (label[neighVId] == NARROW_BAND)
					heap.change(neighVId, vox);
				else {
					heap.add(vox);
					label[neighVId] = NARROW_BAND;
				}

			}
		}
	}
	for (VId = 0; VId < VN; VId++) {
		if (distanceField[VId] == MAX_VALUE) {
			newValue = MAX_VALUE;
			for (uint32_t neighVId : nbrTable[VId]) {
				std::vector<uint32_t> nbrs(nbrTable[VId].begin(),
						nbrTable[VId].end());
				NN2 = nbrs.size();
				for (j = 0; j < NN2; j++) {
					n0 = nbrs[j];
					n1 = nbrs[(j + 1) % NN2];
					tmpValue = ComputeDistance(neighVId, n0, n1, mesh,
							distanceField, label, currentF, heap);
					if (tmpValue < newValue)
						newValue = tmpValue;
				}
			}
			distanceField[VId] = newValue;
		}
	}
	indexes.clear();
	nbrTable.clear();
}

float AlloyMeshDistanceField::ComputeDistance(int vIDc, int vIDa, int vIDb,
		const aly::Mesh& mesh, std::vector<float>& distanceField,
		std::vector<uint8_t>& label, float Fc, BinaryMinHeapPtr<float>& heap) {
	float u, a, b, c;
	float3 Va, Vb, Vc;
	byte la, lb;
	float Ta, Tb;
	float t1, t2, t;
	int tmpint;
	float tmpdouble;

	int tmpvIDa;
	float tmpa;
	float tmpTa;
	byte tmpla;
	int NN, neighVId, i, UNotID, UID = 0, P1ID, P2ID;
	float P1A, P1B, P1C, P2A, P2B, P2C, UA, UB, UC, P1P2, P1U, P2U;
	float cos12A, sin12A, cos12B, sin12B, cos12C, sin12C, cos12U, sin12U;
	float AC, BC, AB;

	float3 P1, P2, U;
	float3 vP1U, vP2U;

	int iters;

	la = label[vIDa];
	lb = label[vIDb];

	if ((la != ACTIVE) && (lb != ACTIVE))
		return MAX_VALUE;

	if (Fc == 0)
		return MAX_VALUE;

	Ta = distanceField[vIDa];
	Tb = distanceField[vIDb];

	if (la == ACTIVE) {
		if (Ta == MAX_VALUE)
			return MAX_VALUE;
	} else
		Ta = MAX_VALUE;

	if (lb == ACTIVE) {
		if (Tb == MAX_VALUE)
			return MAX_VALUE;
	} else
		Tb = MAX_VALUE;

	Va = mesh.vertexLocations[vIDa];
	Vb = mesh.vertexLocations[vIDb];
	Vc = mesh.vertexLocations[vIDc];

	a = distance(Vb, Vc);
	b = distance(Va, Vc);

	if (Ta > Tb) {
		tmpTa = Ta;
		Ta = Tb;
		Tb = tmpTa;

		tmpla = la;
		la = lb;
		lb = tmpla;

		tmpa = a;
		a = b;
		b = tmpa;

		tmpvIDa = vIDa;
		vIDa = vIDb;
		vIDb = tmpvIDa;

		Va = mesh.vertexLocations[vIDa];
		Vb = mesh.vertexLocations[vIDb];
	}

	c = distance(Va, Vb);
	AC = b * b;
	BC = a * a;
	AB = c * c;
	if (AB < AC + BC) {
		if (la == ACTIVE && lb == ACTIVE) {
			t = ComputeDistanceForAcute(Ta, Tb, a, b, c, Fc);
			return t;
		} else
			return (std::min(Ta + b / Fc, Tb + a / Fc));
	}
	/* Initialization */
	P1ID = vIDa;
	P2ID = vIDb;
	UNotID = vIDc;

	P1A = 0;
	P1B = AB;
	P1C = AC;
	P2B = 0;
	P2A = P1B;
	P2C = BC;
	P1P2 = P1B;

	cos12A = 1;
	sin12A = 0;
	cos12B = 1;
	sin12B = 0;
	cos12C = (P1P2 + P2C - P1C) / (2 * std::sqrt(P1P2 * P2C));
	sin12C = (1 - cos12C * cos12C); /* Notice: Square of sine */
	/* Now iteratively unfolding */
	iters = 0;
	const int MAX_UNFOLD_ITERS=128;
	while (iters < MAX_UNFOLD_ITERS) {
		std::vector<uint32_t> nbrs(nbrTable[P1ID].begin(),nbrTable[P1ID].end());
		NN = nbrs.size();
		for (uint32_t i = 0; i < NN; i++) {
			neighVId = nbrs[i];
			if (neighVId == P2ID) {
				if (nbrs[(i + NN - 1) % NN] == UNotID) {
					UID = nbrs[(i + 1) % NN];
					break;
				}
				if (nbrs[(i + 1) % NN] == UNotID) {
					UID = nbrs[(i + NN - 1) % NN];
					break;
				}
			}
		}

		P1 = mesh.vertexLocations[P1ID];
		P2 = mesh.vertexLocations[P2ID];
		U = mesh.vertexLocations[UID];

		vP1U = P1 - U;
		vP2U = P2 - U;

		P1U = (vP1U.x * vP1U.x + vP1U.y * vP1U.y + vP1U.z * vP1U.z);
		P2U = (vP2U.x * vP2U.x + vP2U.y * vP2U.y + vP2U.z * vP2U.z);

		cos12U = (P1P2 + P2U - P1U) / (2 * std::sqrt(P1P2 * P2U));
		sin12U = (1 - cos12U * cos12U); /* Notice: Square of sine */

		/* Now compute three lengthes (squared) */
		UA = P2U + P2A
				- 2 * std::sqrt(P2U * P2A)
						* (cos12A * cos12U - std::sqrt(sin12A * sin12U));
		UB = P2U + P2B
				- 2 * std::sqrt(P2U * P2B)
						* (cos12B * cos12U - std::sqrt(sin12B * sin12U));
		UC = P2U + P2C
				- 2 * std::sqrt(P2U * P2C)
						* (cos12C * cos12U - std::sqrt(sin12C * sin12U));

		/* Now Judge Which Side to continue unfolding */
		if (UA > (UC + AC)) {/* Unfold along P1U */
			UNotID = P2ID;
			P2ID = UID;
			P1P2 = P1U;
			P2A = UA;
			P2B = UB;
			P2C = UC;
		} else if (UB > (UC + BC)) { /* Unfold along P2U */
			UNotID = P1ID;
			P1ID = UID;
			P1P2 = P2U;
			P1A = UA;
			P1B = UB;
			P1C = UC;
		} else { /* Stop Unfolding and compute distanceField */
			UC = std::sqrt(UC);
			UA = std::sqrt(UA);
			UB = std::sqrt(UB);
			if (label[UID] == ACTIVE) {
				if (la == ACTIVE) {
					t1 = ComputeDistanceForAcute(Ta, distanceField[UID], UC, b, UA,
							Fc);
					if (t1 < Ta) { /* Reset A to NBAND and put into Heap */
						VertexIndexed* vox =&indexes[vIDa];
						vox->value=Ta;
						vox->index = vIDa;
						heap.add(vox);
						label[vIDa] = NARROW_BAND;
					}
				} else
					t1 = MAX_VALUE;
				if (lb == ACTIVE) {
					t2 = ComputeDistanceForAcute(Tb, distanceField[UID], UC, a, UB,
							Fc);
					if (t2 < Tb) { /* Reset B to NBAND and put into Heap */
						VertexIndexed* vox =&indexes[vIDb];
						vox->value=Tb;
						vox->index = vIDb;
						heap.add(vox);
						label[vIDb] = NARROW_BAND;
					}
				} else
					t2 = MAX_VALUE;
				return std::min(t1, t2);
			} else
				return (std::min(Ta + b / Fc, Tb + a / Fc));
		}

		/* Update angles */
		cos12A = (P1P2 + P2A - P1A) / (2 * std::sqrt(P1P2 * P2A));
		if (P2B != 0)
			cos12B = (P1P2 + P2B - P1B) / (2 * std::sqrt(P1P2 * P2B));
		cos12C = (P1P2 + P2C - P1C) / (2 * std::sqrt(P1P2 * P2C));

		sin12A = 1 - cos12A * cos12A;
		sin12B = 1 - cos12B * cos12B;
		sin12C = 1 - cos12C * cos12C;

		iters++;
	}
	return (std::min(Ta + b / Fc, Tb + a / Fc));
}
float AlloyMeshDistanceField::ComputeDistanceForAcute(float Ta, float Tb, float a,
		float b, float c, float Fc) {
	float t1, t2, t, CD, costheta;
	float aa, bb, cc, u, tmp;
	costheta = (a * a + b * b - c * c) / (2 * a * b);

	u = Tb - Ta;

	Fc = 1 / Fc; /* Inverted here ! */

	aa = a * a + b * b - 2 * a * b * costheta;
	bb = 2 * b * u * (a * costheta - b);
	cc = b * b * (u * u - a * a * Fc * Fc * (1 - costheta * costheta));

	tmp = bb * bb - 4 * aa * cc;
	if (tmp < 0)
		return (std::min(b * Fc + Ta, a * Fc + Tb));
	tmp = std::sqrt(tmp);

	t1 = (-bb + tmp) / (2 * aa);
	t2 = (-bb - tmp) / (2 * aa);
	t = std::max(t1, t2);
	CD = (b * (t - u)) / t;

	if ((u < t) && (a * costheta < CD) && (CD < (a / costheta)))
		return (t + Ta);
	else
		return (std::min(b * Fc + Ta, a * Fc + Tb));
}

} /* namespace intel */
