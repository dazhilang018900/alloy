#ifndef CubeLUT_H
#define CubeLUT_H
#include <string>
#include <vector>
#include <fstream>
#include "math/AlloyMath.h"
namespace aly{
class CubeLUT {
public:
	typedef std::vector<float3> table1D;
	typedef std::vector<table1D> table2D;
	typedef std::vector<table2D> table3D;
	enum class LUTState {
		OK = 0, NotInitialized = 1,
		ReadError = 2, WriteError=3, PrematureEndOfFile=4, LineError=5,
		UnknownOrRepeatedKeyword = 6, TitleMissingQuote=7, DomainBoundsReversed=8,
		LUTSizeOutOfRange=9, CouldNotParseTableData=10
	};
	LUTState status;
	std::string title;
	float3 domainMin;
	float3 domainMax;
	table1D LUT1D;
	table3D LUT3D;
	int rows,cols,slices;
	CubeLUT(void):rows(0),cols(0),slices(0) { status = LUTState::NotInitialized; };
	LUTState LoadCubeFile(std::ifstream & infile);
	LUTState SaveCubeFile(std::ofstream & outfile) const;
	float3 operator()(float x, float y, float z) const;
	float3 operator()(int i, int j, int k) const;
private:
	std::string ReadLine(std::ifstream & infile, char lineSeparator);
	float3 ParseTableRow(const std::string & lineOfText);
};
void ReadCubeFromFile(const std::string& f, CubeLUT& lut);
void WriteCubeFromFile(const std::string& f,const CubeLUT& lut);

}

#endif

