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

#ifndef CUBELUT_H
#define CUBELUT_H
#include <iostream>
#include <string>
#include <array>
#include <vector>
#include "math/AlloyMath.h"

namespace aly {
double Gaussian1D(double r, double mu, double sigma);
double Gaussian3D(double3 R, double3 mu, double3 sigma);

typedef std::vector<std::vector<std::vector<double>>> CubeTensor;
class CubeLUT {
public:
	// Default constructor (use copy_header to compy the header of a CubeLUT and fill data with zeros)
	CubeLUT();
	// Create cube file from data
	CubeLUT(std::string fname);

	// Load CubeLUT fro file
	void load(std::string filename);

	// Printing routines (whole cube, only header or only data)
	void print(std::ostream& out) const;
	void printHeader(std::ostream& out) const;
	void printAtoms(std::ostream& out) const;
	void printData(std::ostream& out, int lines = 0) const;
	double evaluate(double x, double y, double z) const;
	// COmpare headers
	bool header_is_compatible(const CubeLUT& cube,
			double threshold = 1e-12) const;
	bool header_is_equal(const CubeLUT& cube, double threshold = 1e-12) const;
	// Copy header (reinitialize data with the correct number of voxels)
	void copy_header(const CubeLUT& cube);
	// Sum two cube files
	CubeLUT operator+(const CubeLUT& cube) const;
	// Subtract two cube files
	CubeLUT operator-(const CubeLUT& cube) const;
	// Multiply cube data by number
	CubeLUT operator*(double) const;

	// Return reshaped DATA (rank-3 tensor instead of vector)
	CubeTensor reshape() const;

	// Return data
	std::vector<double> getData() const;

	// Return CubeLUT file origin
	double3 getOrigin() const;

	// Return axis
	double3 getAxisX() const;
	double3 getAxisY() const;
	double3 getAxisZ() const;

	// Return number of voxels
	long int getRows() const;
	long int getCols() const;
	long int getSlices() const;

	// Length of the voxel along a, b or c
	double getResolutionX() const;
	double getResolutionY() const;
	double getResolutionZ() const;

	// Periodic boundary conditions
	double periodicTransform(double d, unsigned int idir) const;

	// Get atoms
	std::vector<std::array<double, 5>> getAtoms() const;

	// Shift atoms
	std::vector<std::array<double, 5>> shiftAtoms(double aa, double bb,
			double cc);

private:
	// Sum (PM=+1) or subtract (PM=-1) two cube files
	CubeLUT addsub(const CubeLUT& cube, int pm) const;

	// Fist two lines of the CubeLUT file (comments)
	std::string comment1, comment2;

	// Number of atoms
	unsigned int atomCount;

	// Origin
	double3 origin;

	// Number of voxels
	int rows, cols, slices;

	// Axis vectors
	double3 xAxis, yAxis, zAxis;

	// Atoms
	std::vector<std::array<double, 5>> atoms;

	// Data
	std::vector<double> data;

};
void ReadCubeFromFile(const std::string& f,CubeLUT& lut);
}
#endif
