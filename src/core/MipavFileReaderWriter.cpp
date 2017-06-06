/*
 * MipavFileReader.cpp
 *
 *  Created on: Jun 6, 2017
 *      Author: blake
 */

#include <AlloyFileUtil.h>
#include <AlloyCommon.h>
#include <MipavFileReaderWriter.h>
#include "tinyxml2.h"
using namespace tinyxml2;
namespace aly {
bool SANITY_CHECK_XML() {
	const std::string txt =
			R"(<?xml version="1.0" encoding="UTF-8"?>
<!-- MIPAV header file -->
<image xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" nDimensions="3">
	<Dataset-attributes>
		<Image-offset>0</Image-offset>
		<Data-type>Float</Data-type>
		<Endianess>Little</Endianess>
		<Extents>360</Extents>
		<Extents>275</Extents>
		<Extents>2</Extents>
		<Resolutions>
			<Resolution>1.0</Resolution>
			<Resolution>2.0</Resolution>
			<Resolution>3.0</Resolution>
		</Resolutions>
		<Slice-spacing>1.0</Slice-spacing>
		<Slice-thickness>0.0</Slice-thickness>
		<Units>Millimeters</Units>
		<Units>Inches</Units>
		<Units>Feet</Units>
		<Compression>none</Compression>
		<Orientation>Unknown</Orientation>
		<Subject-axis-orientation>Axial</Subject-axis-orientation>
		<Subject-axis-orientation>Coronal</Subject-axis-orientation>
		<Subject-axis-orientation>Sagittal</Subject-axis-orientation>
		<Origin>0.1</Origin>
		<Origin>2.3</Origin>
		<Origin>5.7</Origin>
		<Modality>Unknown Modality</Modality>
	</Dataset-attributes>
</image>)";
	std::string f = MakeString() << GetDesktopDirectory() << ALY_PATH_SEPARATOR<<"header_read_test.xml";
	WriteTextFile(f, txt);
	MipavHeader header;
	ReadMipavHeaderFromFile(f, header);
	std::cout << header << std::endl;
	f = MakeString() << GetDesktopDirectory() << ALY_PATH_SEPARATOR<<"header_write_test.xml";
	WriteMipavHeaderToFile(f,header);
	return true;
}
void WriteMipavHeaderToFile(const std::string& file,
		const MipavHeader& header) {
	std::stringstream ss;
	ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	ss << "<!-- MIPAV header file -->\n";
	ss
			<< "<image xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" nDimensions=\""
			<< header.dimensions << "\">\n";
	ss << "	<Dataset-attributes>\n";
	ss << "		<Image-offset>" << header.dimensions << "</Image-offset>\n";
	ss << "		<Data-type>" << header.dataType << "</Data-type>\n";
	ss << "		<Endianess>" << header.endianess << "</Endianess>\n";
	for (int i : header.extents) {
		ss << "		<Extents>" << i << "</Extents>\n";
	}
	ss << "		<Resolutions>\n";
	for (float v : header.resolutions) {
		ss << "			<Resolution>" << v << "</Resolution>\n";
	}
	ss << "		</Resolutions>\n";
	ss << "		<Slice-spacing>"<<header.sliceSpacing<<"</Slice-spacing>\n";
	ss << "		<Slice-thickness>"<<header.sliceThickness<<"</Slice-thickness>\n";
	for(std::string s:header.units){
		ss << "		<Units>"<<s<<"</Units>\n";
	}
	ss << "		<Compression>"<<header.compression<<"</Compression>\n";
	ss << "		<Orientation>"<<header.orientation<<"</Orientation>\n";
	for (std::string s : header.subjectAxisOrientation) {
		ss << "		<Subject-axis-orientation>" << s<< "</Subject-axis-orientation>\n";
	}
	for (float v : header.origin) {
		ss << "		<Origin>" << v << "</Origin>\n";
	}
	ss << "		<Modality>" << header.modality << "</Modality>\n";
	ss << "	</Dataset-attributes>\n";
	ss << "</image>\n";
	WriteTextFile(GetFileWithoutExtension(file)+".xml", ss.str());
}
bool ReadMipavHeaderFromFile(const std::string& file, MipavHeader& header) {
	XMLDocument doc;
	if (doc.LoadFile(file.c_str()) == XML_ERROR_FILE_NOT_FOUND) {
		return false;
	}
	XMLElement* elem = doc.FirstChildElement("image");
	header = MipavHeader();
	elem->QueryIntAttribute("nDimensions", &header.dimensions);
	if (elem) {
		elem = elem->FirstChildElement("Dataset-attributes");
		if (elem) {
			for (XMLElement* child = elem->FirstChildElement();
					child != nullptr && child <= elem->LastChildElement();
					child++) {
				std::string name = ToLower(child->Name());
				int i;
				float f;
				if (name == "Image-offset") {
					child->QueryInt64Text(&header.imageOffset);
				} else if (name == "extents") {
					child->QueryIntText(&i);
					header.extents.push_back(i);
				} else if (name == "resolution") {
					child->QueryFloatText(&f);
					header.resolutions.push_back(f);
				} else if (name == "units") {
					header.units.push_back(child->GetText());
				} else if (name == "slice-spacing") {
					child->QueryFloatText(&header.sliceSpacing);
				} else if (name == "slice-thickness") {
					child->QueryFloatText(&header.sliceThickness);
				} else if (name == "endianess") {
					header.endianess = child->GetText();
				} else if (name == "data-type") {
					header.dataType = child->GetText();
				} else if (name == "compression") {
					header.compression = child->GetText();
				} else if (name == "orientation") {
					header.orientation = child->GetText();
				} else if (name == "subject-axis-orientation") {
					std::string s = child->GetText();
					header.subjectAxisOrientation.push_back(s);
				} else if (name == "origin") {
					child->QueryFloatText(&f);
					header.origin.push_back(f);
				} else if (name == "modality") {
					header.modality = child->GetText();
				}
			}
		}
	}
	return false;
}
} /* namespace intel */
