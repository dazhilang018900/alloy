/*
 * Copyright(C) 2017, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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
#ifndef INCLUDE_CORE_MIPAVHEADERREADERWRITER_H_
#define INCLUDE_CORE_MIPAVHEADERREADERWRITER_H_
#include <string>
#include <AlloyCommon.h>

namespace aly {
bool SANITY_CHECK_XML();
struct MipavHeader{
	int dimensions=0;
	int64_t imageOffset = 0;
	std::string dataType="Byte";
	std::string endianess="Little";
	std::vector<int> extents;
	std::vector<float> resolutions;
	float sliceSpacing=1.0f;
	float sliceThickness=0.0f;
	std::vector<std::string> units;
	std::vector<std::string> subjectAxisOrientation;
	std::string compression="None";
	std::string orientation="Unknown";
	std::vector<float> origin;
	std::string modality="Unknown Modality";
};
template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const MipavHeader& header) {
	ss<<"Dimensions = "<<header.dimensions<<std::endl;
	ss<<"Image-offset = "<<header.imageOffset<<std::endl;
	ss<<"Data-type = "<<header.dataType<<std::endl;
	ss<<"Endianess = "<<header.endianess<<std::endl;
	for(int v:header.extents){
		ss<<"Extents = "<<v<<std::endl;
	}
	for(float v:header.resolutions){
		ss<<"Resolutions = "<<v<<std::endl;
	}
	ss<<"Slice-spacing = "<<header.sliceSpacing<<std::endl;
	ss<<"Slice-thickness = "<<header.sliceThickness<<std::endl;
	for(std::string s:header.units){
		ss<<"Units = "<<s<<std::endl;
	}
	ss<<"Compression = "<<header.compression<<std::endl;
	ss<<"Orientation = "<<header.orientation<<std::endl;
	for(std::string s:header.subjectAxisOrientation){
		ss<<"Subject-axis-orientation = "<<s<<std::endl;
	}
	for(float v:header.origin){
		ss<<"Origin = "<<v<<std::endl;
	}
	ss<<"Modality = "<<header.modality<<std::endl;
	return ss;
}
bool ReadMipavHeaderFromFile(const std::string& file,MipavHeader& header);
void WriteMipavHeaderToFile(const std::string& file,const MipavHeader& header);
} /* namespace intel */

#endif /* INCLUDE_CORE_MIPAVHEADERREADERWRITER_H_ */
