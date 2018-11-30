/*
 Copyright (C) 2016 Rocco Meli (rocco.meli@epfl.ch)

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "CubeLUT.h"
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <utility>
namespace aly {
void ReadCubeFromFile(const std::string& f,CubeLUT& lut){
	lut.load(f);
}
double Gaussian1D(double r, double mu, double sigma) {
	double norm(1. / (std::sqrt(2. * M_PI) * sigma));
	return norm * std::exp(-std::pow(r - mu, 2) / (2. * std::pow(sigma, 2)));
}
double Gaussian3D(double3 r, double3 mu, double3 sigma) {
	double g = 1;
	g *= Gaussian1D(r.x, mu.x, sigma.x);
	g *= Gaussian1D(r.x, mu.y, sigma.y);
	g *= Gaussian1D(r.x, mu.z, sigma.z);
	return g;
}
double CubeLUT::evaluate(double ri, double gi, double bi) const {
	return 0;
}
CubeLUT::CubeLUT() :
		atomCount(0), rows(0), cols(0), slices(0)  {
}
CubeLUT::CubeLUT(std::string fname) :
		atomCount(0), rows(0), cols(0), slices(0) {
	load(fname);
}

void CubeLUT::load(std::string fname) {
	// Open CubeLUT file
	std::fstream in(fname);

	if (!in.is_open()) {
		std::cerr << "ERROR: Impossible to open cube file." << std::endl;
		std::exit(-1);
	}

	// Store first two comment lines
	std::getline(in, comment1);
	std::getline(in, comment2);

	// Number of atoms and origin
	in >> atomCount >> origin[0] >> origin[1] >> origin[2];

	// Voxel properties (number and axis)
	in >> rows >> xAxis[0] >> xAxis[1] >> xAxis[2];
	in >> cols >> yAxis[0] >> yAxis[1] >> yAxis[2];
	in >> slices >> zAxis[0] >> zAxis[1] >> zAxis[2];

	// Preallocate vector for atoms
	atoms.resize(atomCount);

	// Store atoms
	for (unsigned int i(0); i < atomCount; i++) {
		// Store atom
		in >> atoms[i][0] >> atoms[i][1] >> atoms[i][2] >> atoms[i][3]
				>> atoms[i][4];
	}

	// Total number of voxels
	unsigned long int Nvol(rows * cols * slices);

	// Preallocate vector for volumetric data
	data.resize(Nvol);

	// Store volumetric data
	for (unsigned long int i(0); i < Nvol; i++) {
		in >> data[i];
	}
}

void CubeLUT::print(std::ostream& out) const {
	printHeader(out);
	printAtoms(out);
	printData(out);
}

void CubeLUT::printHeader(std::ostream& out) const {
	// Print first two comment lines
	out << comment1 << std::endl;
	out << comment2 << std::endl;

	// Print number of atoms and origin
	out << std::setw(5) << atomCount;
	for (unsigned int i(0); i < 3; i++) {
		out << std::fixed << std::setprecision(6) << std::setw(12) << origin[i];
	}
	out << std::endl;

	// Print voxel properties (number and axis): A
	out << std::setw(5) << rows;
	for (unsigned int i(0); i < 3; i++) {
		out << std::fixed << std::setprecision(6) << std::setw(12) << xAxis[i];
	}
	out << std::endl;

	// Print voxel properties (number and axis): B
	out << std::setw(5) << cols;
	for (unsigned int i(0); i < 3; i++) {
		out << std::fixed << std::setprecision(6) << std::setw(12) << yAxis[i];
	}
	out << std::endl;

	// Print voxel properties (number and axis): C
	out << std::setw(5) << slices;
	for (unsigned int i(0); i < 3; i++) {
		out << std::fixed << std::setprecision(6) << std::setw(12) << zAxis[i];
	}
	out << std::endl;
}

void CubeLUT::printAtoms(std::ostream& out) const {
	// Print atoms
	for (unsigned int i(0); i < atomCount; i++) {
		out << std::fixed << std::setprecision(0) << std::setw(5)
				<< atoms[i][0];

		for (unsigned int j(1); j < 5; j++) {
			out << std::fixed << std::setprecision(6) << std::setw(12)
					<< atoms[i][j];
		}

		out << std::endl;
	}
}

void CubeLUT::printData(std::ostream& out, int lines) const {
	// Total number of voxels
	unsigned long int Nvol(rows * cols * slices);

	// Voxel to print
	unsigned long int min(0);
	unsigned long int max(Nvol);

	if (lines < 0) {
		min = Nvol + lines * 6;
	} else if (lines > 0) {
		max = lines * 6;
	}

	// Print volumetric data
	for (unsigned int i(min); i < max; i++) {
		out << std::fixed << std::scientific << std::setprecision(5)
				<< std::setw(13) << data[i];

		if ((i + 1) % 6 == 0) {
			out << std::endl;
		}
	}
}

bool CubeLUT::header_is_compatible(const CubeLUT& cube,
		double threshold) const {
	// Compare origin coordinate by coordinate
	for (unsigned int i(0); i < 3; i++) {
		if (std::abs(origin[i] - cube.origin[i]) > threshold) {
			return false;
		}
	}

	// Compare number of voxels in every direction
	if ((rows != cube.rows) || (cols != cube.cols) || (slices != cube.slices)) {
		return false;
	}

	// Compare axis A
	for (unsigned int i(0); i < 3; i++) {
		if (std::abs(xAxis[i] - cube.xAxis[i]) > threshold) {
			return false;
		}
	}

	// Compare axis B
	for (unsigned int i(0); i < 3; i++) {
		if (std::abs(yAxis[i] - cube.yAxis[i]) > threshold) {
			return false;
		}
	}

	// Compare axis C
	for (unsigned int i(0); i < 3; i++) {
		if (std::abs(zAxis[i] - cube.zAxis[i]) > threshold) {
			return false;
		}
	}

	return true;
}

bool CubeLUT::header_is_equal(const CubeLUT& cube, double threshold) const {
	// Compare number of atoms
	if (atomCount != cube.atomCount) {
		return false;
	}

	// Compare origin, number of voxels and axis
	if (!header_is_compatible(cube, threshold)) {
		return false;
	}

	// Compare atoms
	for (unsigned int i(0); i < atomCount; i++) {
		for (unsigned int j(0); j < 5; j++) {
			if (std::abs(atoms[i][j] - cube.atoms[i][j]) > threshold) {
				return false;
			}
		}
	}

	return true;
}

void CubeLUT::copy_header(const CubeLUT& cube) {
	// Copy first two comment lines
	comment1 = cube.comment1;
	comment2 = cube.comment2;

	// Copy number of atoms and origin
	atomCount = cube.atomCount;
	origin = cube.origin;

	// Copy voxel properties
	rows = cube.rows;
	cols = cube.cols;
	slices = cube.slices;
	xAxis = cube.xAxis;
	yAxis = cube.yAxis;
	zAxis = cube.zAxis;

	// Copy atoms
	atoms = cube.atoms;

	// Total number of voxels
	unsigned long int Nvol(rows * cols * slices);

	// Resize data and initialize all elements to zero
	data.clear();
	data.resize(Nvol, 0.0);

}

CubeLUT CubeLUT::addsub(const CubeLUT& cube, int pm) const {
	if (std::abs(pm) != 1) {
		std::cerr << "\nERROR: PM should be +1 or -1.\n" << std::endl;

		std::exit(-1);
	}

	// Check if the headers of the two cube files (*THIS and CUBE) are compatible
	if (header_is_compatible(cube)) {
		// Check if the headers of the two cube files (*THIS and CUBE) are equal
		if (!header_is_equal(cube)) {
			// Warn the user that the cube files contain different atoms and/or atomic positions
			std::cerr
					<< "\nWARNING: CubeLUT files contains different atoms or atomic positions."
					<< std::endl;
			std::cerr
					<< "         The header of the first CubeLUT will be considered for the sum.\n"
					<< std::endl;
		}
	} else // CubeLUT file are not compatible and can't be summed (TODO: origin shift)
	{
		std::cerr << "\nERROR: CubeLUT files are not compatible.\n"
				<< std::endl;

		std::exit(-1);
	}

	// Create empty cube file
	CubeLUT sumsub;

	// Copy current header to new cube file
	sumsub.copy_header(*this);

	// Total number of voxels
	unsigned long int Nvol(rows * cols * slices);

	// Preallocate data of new cube file
	sumsub.data.resize(Nvol);

	// Sum voxel by voxel
	for (unsigned long int i(0); i < Nvol; i++) {
		// Sum (pm=1) or subtract (pm=-1) input data from data
		sumsub.data[i] = data[i] + pm * cube.data[i]; // PM is +1 (addition) or -1 (subtraction)
	}

	return sumsub;
}

CubeLUT CubeLUT::operator+(const CubeLUT& cube) const {
	// Call addsub as add (pm=+1)
	return addsub(cube, +1);
}

CubeLUT CubeLUT::operator-(const CubeLUT& cube) const {
	// Call addsub as subtract (pm=-1)
	return addsub(cube, -1);
}

CubeLUT CubeLUT::operator*(double m) const {
	// Create empty cube file
	CubeLUT mult;

	// Copy current header to new cube file
	mult.copy_header(*this);

	// Total number of voxels
	unsigned long int Nvol(rows * cols * slices);

	// Preallocate data of new cube file
	mult.data.resize(Nvol, 0.0);

	// Multiply voxel by voxel
	for (unsigned long int i(0); i < Nvol; i++) {
		mult.data[i] = data[i] * m;
	}

	return mult;
}

CubeTensor CubeLUT::reshape() const {
	CubeTensor datat(rows,
			std::vector<std::vector<double>>(cols,
					std::vector<double>(slices, 0.0)));
	// Loop over axis a
	for (unsigned int i(0); i < rows; i++) {
		// Loop over axis b
		for (unsigned int j(0); j < cols; j++) {
			// Loop over axis c
			for (unsigned int k(0); k < slices; k++) {
				// Tranform linear array DATA into rank-3 tensor DATAR
				datat[i][j][k] = data[i * cols * slices + j * slices + k];
			}
		}
	}

	return datat;
}

std::vector<double> CubeLUT::getData() const {
	return data;
}

double3 CubeLUT::getOrigin() const {
	// Return origin of CubeLUT file
	return origin;
}

double3 CubeLUT::getAxisX() const {
	// Return axis s
	return xAxis;
}

double3 CubeLUT::getAxisY() const {
	// Return axis b
	return yAxis;
}

double3 CubeLUT::getAxisZ() const {
	// Return axis c
	return zAxis;
}

long int CubeLUT::getRows() const {
	// Return number of voxels along a
	return rows;
}

long int CubeLUT::getCols() const {
	// Return number of voxels along b
	return cols;
}

long int CubeLUT::getSlices() const {
	// Return number of voxels along c
	return slices;
}

double CubeLUT::getResolutionX() const {
	// Compute length of axis a
	return std::sqrt(
			xAxis[0] * xAxis[0] + xAxis[1] * xAxis[1] + xAxis[2] * xAxis[2]);
}

double CubeLUT::getResolutionY() const {
	// Compute length of axis b
	return std::sqrt(
			yAxis[0] * yAxis[0] + yAxis[1] * yAxis[1] + yAxis[2] * yAxis[2]);
}

double CubeLUT::getResolutionZ() const {
	// Compute length of axis c
	return std::sqrt(
			zAxis[0] * zAxis[0] + zAxis[1] * zAxis[1] + zAxis[2] * zAxis[2]);
}

double CubeLUT::periodicTransform(double d, unsigned int idir) const {
	if ((idir < 1) or (idir > 3)) {
		std::cerr << "ERROR: Invalid IDIR." << std::endl;
		std::cerr << "       IDIR should be 1 (x), 2 (y) or 3 (z)" << std::endl;

		std::exit(-1);
	}

	// Cell dimension along IDIR
	double L(0);

	// Voxel dimension along IDIR
	double dd(0);

	if (idir == 1) {
		dd = getResolutionX();
		L = rows * dd;
	}
	if (idir == 2) {
		dd = getResolutionY();
		L = cols * dd;
	}
	if (idir == 3) {
		dd = getResolutionZ();
		L = slices * dd;
	}

	return d - L * static_cast<int>(d / L + 0.5);
}

std::vector<std::array<double, 5>> CubeLUT::getAtoms() const {
	return atoms;
}

std::vector<std::array<double, 5>> CubeLUT::shiftAtoms(double aa, double bb,
		double cc) {
	double La(rows * getResolutionX());
	double Lb(cols * getResolutionY());
	double Lc(slices * getResolutionZ());

	double da(0);
	double db(0);
	double dc(0);

	std::vector<std::array<double, 5>> shifted_atoms(atomCount,
			std::array<double, 5> { { 0, 0, 0, 0, 0 } });

	for (unsigned int i(0); i < atomCount; i++) {
		shifted_atoms[i][0] = atoms[i][0];
		shifted_atoms[i][1] = atoms[i][0];

		shifted_atoms[i][2] = atoms[i][2] + aa * La;
		shifted_atoms[i][3] = atoms[i][3] + bb * Lb;
		shifted_atoms[i][4] = atoms[i][4] + cc * Lc;

		da = shifted_atoms[i][2] - origin[0];

		if (da > La) {
			shifted_atoms[i][2] -= La;
		}

		db = shifted_atoms[i][3] - origin[1];

		if (db > Lb) {
			shifted_atoms[i][3] -= Lb;
		}

		dc = shifted_atoms[i][4] - origin[2];

		if (dc > Lc) {
			shifted_atoms[i][4] -= Lc;
		}
	}

	return shifted_atoms;
}
}
