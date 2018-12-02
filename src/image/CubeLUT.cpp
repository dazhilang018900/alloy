#include "CubeLUT.h"
#include <iostream>
#include <sstream>
//Based on Adobe's Specification:
//http://wwwimages.adobe.com/content/dam/Adobe/en/products/speedgrade/cc/pdfs/cube-lut-specification-1.0.pdf
namespace aly {
void ReadCubeFromFile(const std::string& f, CubeLUT& lut) {
	std::ifstream in(f);
	if (!in.is_open()) {
		throw std::runtime_error("ERROR: Impossible to open cube file.");
	}
	CubeLUT::LUTState status=lut.load(in);
	if(status!=CubeLUT::LUTState::OK){
		throw std::runtime_error(MakeString()<<"Failed to load cube file. Error "<<static_cast<int>(status));
	}
}

void WriteCubeFromFile(const std::string& f,const CubeLUT& lut){
	std::ofstream out(f);
	if (!out.is_open()) {
		throw std::runtime_error("ERROR: Impossible to write cube file.");
	}
	CubeLUT::LUTState status=lut.save(out);
	if(status!=CubeLUT::LUTState::OK){
		throw std::runtime_error(MakeString()<<"Failed to save cube file. Error "<<static_cast<int>(status));
	}
}
float3 CubeLUT::operator()(float x, float y, float z) const {
	int i = static_cast<int>(std::floor(x));
	int j = static_cast<int>(std::floor(y));
	int k = static_cast<int>(std::floor(z));
	float3 rgb000 = (operator()(i, j, k));
	float3 rgb100 = (operator()(i + 1, j, k));
	float3 rgb110 = (operator()(i + 1, j + 1, k));
	float3 rgb010 = (operator()(i, j + 1, k));
	float3 rgb001 = (operator()(i, j, k + 1));
	float3 rgb101 = (operator()(i + 1, j, k + 1));
	float3 rgb111 = (operator()(i + 1, j + 1, k + 1));
	float3 rgb011 = (operator()(i, j + 1, k + 1));
	float dx = x - i;
	float dy = y - j;
	float dz = z - k;
	float3 lower = ((rgb000 * (1.0f - dx) + rgb100 * dx) * (1.0f - dy)
			+ (rgb010 * (1.0f - dx) + rgb110 * dx) * dy);
	float3 upper = ((rgb001 * (1.0f - dx) + rgb101 * dx) * (1.0f - dy)
			+ (rgb011 * (1.0f - dx) + rgb111 * dx) * dy);
	return (1.0f - dz) * lower + dz * upper;

}
float3 CubeLUT::operator()(int i, int j, int k) const {
	return (LUT3D.size() > 0) ? LUT3D[clamp(i,0,rows-1)][clamp(j,0,cols-1)][clamp(k,0,slices-1)] : LUT1D[clamp(i,0,rows-1)];
}
std::string CubeLUT::ReadLine(std::ifstream & infile, char lineSeparator) {
	// Skip empty lines and comments
	const char CommentMarker = '#';
	std::string textLine("");
	while (textLine.size() == 0 || textLine[0] == CommentMarker) {
		if (infile.eof()) {
			status = LUTState::PrematureEndOfFile;
			break;
		}
		std::getline(infile, textLine, lineSeparator);
		if (infile.fail()) {
			status = LUTState::ReadError;
			break;
		}
	}
	return textLine;
} // ReadLine
float3 CubeLUT::ParseTableRow(const std::string & lineOfText) {
	float3 f;
	std::istringstream line(lineOfText);
	for (int i = 0; i < 3; i++) {
		line >> f[i];
		if (line.fail()) {
			status = LUTState::CouldNotParseTableData;
			break;
		};
	}
	return f;
} // ParseTableRow
CubeLUT::LUTState CubeLUT::load(std::ifstream & infile) {
	// Set defaults
	status = LUTState::OK;
	title.clear();
	domainMin = float3( 0.0f);
	domainMax = float3( 1.0f);
	LUT1D.clear();
	LUT3D.clear();
	// Read file data line by line
	const char NewlineCharacter = '\n';
	char lineSeparator = NewlineCharacter;
	// sniff use of legacy lineSeparator
	const char CarriageReturnCharacter = '\r';
	for (int i = 0; i < 255; i++) {
		char inc = infile.get();
		if (inc == NewlineCharacter)
			break;
		if (inc == CarriageReturnCharacter) {
			if (infile.get() == NewlineCharacter)
				break;
			lineSeparator = CarriageReturnCharacter;
			std::cout
					<< "INFO: This file uses non-compliant line separator \\r (0x0D)"
					<< std::endl;
			break;
		}
		if (i > 250) {
			status = LUTState::LineError;
			break;
		}
	}
	infile.seekg(0);
	infile.clear();
	// read keywords
	int N, CntTitle, CntSize, CntMin, CntMax;

	// each keyword to occur zero or one time
	N = CntTitle = CntSize = CntMin = CntMax = 0;
	while (status == LUTState::OK) {
		long linePos = infile.tellg();
		std::string lineOfText = ReadLine(infile, lineSeparator);
		if (status != LUTState::OK)
			break;
		// Parse keywords and parameters
		std::istringstream line(lineOfText);
		std::string keyword;
		line >> keyword;
		aly::Trim(keyword);
		if(keyword.size()==0)continue;
		if ("+" < keyword && keyword < ":") {
			// lines of table data come after keywords
			// restore stream pos to re-read line of data
			infile.seekg(linePos);
			break;
		} else if (keyword == "TITLE" && CntTitle++ == 0) {
			const char QUOTE = '"';
			char startOfTitle;
			line >> startOfTitle;
			if (startOfTitle != QUOTE) {
				status = LUTState::TitleMissingQuote;
				break;
			}
			std::getline(line, title, QUOTE); // read to "
		} else if (keyword == "DOMAIN_MIN" && CntMin++ == 0) {
			line >> domainMin[0] >> domainMin[1] >> domainMin[2];
		} else if (keyword == "DOMAIN_MAX" && CntMax++ == 0) {
			line >> domainMax[0] >> domainMax[1] >> domainMax[2];
		} else if (keyword == "LUT_1D_SIZE" && CntSize++ == 0) {
			line >> N;
			if (N < 2 || N > 65536) {
				status = LUTState::LUTSizeOutOfRange;
				break;
			}
			LUT1D = table1D(N, float3(0.0f));
		} else if (keyword == "LUT_3D_SIZE" && CntSize++ == 0) {
			line >> N;
			if (N < 2 || N > 256) {
				status = LUTState::LUTSizeOutOfRange;
				break;
			}
			LUT3D = table3D(N, table2D(N, table1D(N, float3(0.0f))));
		} else {
			std::cerr<<"Bad Keyword ["<<keyword<<"]"<<std::endl;
			status = LUTState::UnknownOrRepeatedKeyword;
			break;
		}
		if (line.fail()) {
			status = LUTState::ReadError;
			break;
		}
	} // read keywords
	if (status ==LUTState::OK && CntSize == 0)
		status = LUTState::LUTSizeOutOfRange;
	if (status == LUTState::OK
			&& (domainMin[0] >= domainMax[0] || domainMin[1] >= domainMax[1]
					|| domainMin[2] >= domainMax[2]))
		status = LUTState::DomainBoundsReversed;
	// read lines of table data
	if (LUT1D.size() > 0) {
		N = LUT1D.size();
		rows = N;
		cols = 0;
		slices = 0;
		for (int i = 0; i < N && status == LUTState::OK; i++) {
			LUT1D[i] = ParseTableRow(ReadLine(infile, lineSeparator));
		}
	} else {
		N = LUT3D.size();
		rows = cols = slices = N;
		// NOTE that r loops fastest
		for (int b = 0; b < N && status == LUTState::OK; b++) {
			for (int g = 0; g < N && status == LUTState::OK; g++) {
				for (int r = 0; r < N && status == LUTState::OK; r++) {
					LUT3D[r][g][b] = ParseTableRow(
							ReadLine(infile, lineSeparator));
				}
			}
		}
	}
	return status;
} // LoadCubeFile
CubeLUT::LUTState CubeLUT::save(std::ofstream & outfile) const {
	if (status != LUTState::OK)
		return status; // Write only good Cubes
	// Write keywords
	const char SPACE = ' ';
	const char QUOTE = '"';
	if (title.size() > 0)
		outfile << "TITLE" << SPACE << QUOTE << title << QUOTE << std::endl;
	outfile << "# Created by CubeLUT.cpp" << std::endl;
	outfile << "DOMAIN_MIN" << SPACE << domainMin[0] << SPACE << domainMin[1]
			<< SPACE << domainMin[2] << std::endl;
	outfile << "DOMAIN_MAX" << SPACE << domainMax[0] << SPACE << domainMax[1]
			<< SPACE << domainMax[2] << std::endl;
	// Write LUT data
	if (LUT1D.size() > 0) {
		int N = LUT1D.size();
		outfile << "LUT_1D_SIZE" << SPACE << N << std::endl;
		for (int i = 0; i < N && outfile.good(); i++) {
			outfile << LUT1D[i][0] << SPACE << LUT1D[i][1] << SPACE
					<< LUT1D[i][2] << std::endl;
		}
	} else {
		int N = LUT3D.size();
		outfile << "LUT_3D_SIZE" << SPACE << N << std::endl;
		// NOTE that r loops fastest
		for (int b = 0; b < N && outfile.good(); b++) {
			for (int g = 0; g < N && outfile.good(); g++) {
				for (int r = 0; r < N && outfile.good(); r++) {
					outfile << LUT3D[r][g][b][0] << SPACE << LUT3D[r][g][b][1]
							<< SPACE << LUT3D[r][g][b][2] << std::endl;
				}
			}
		}
	} // write 3D LUT
	outfile.flush();
	return (outfile.good() ? LUTState::OK : LUTState::WriteError);
} // SaveCubeFile
}
