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
#include "common/AlloyCommon.h"
#include "system/AlloyFileUtil.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cctype>
#include <functional>
namespace aly {
void SANITY_CHECK_STRINGS(){
	std::cout<<"["<<LongestCommonPrefix(std::vector<std::string>{"abra"})<<"]"<<std::endl;
	std::cout<<"["<<LongestCommonPrefix(std::vector<std::string>{"abra","cadabra"})<<"]"<<std::endl;
	std::cout<<"["<<LongestCommonPrefix(std::vector<std::string>{"abra","abracadabra"})<<"]"<<std::endl;
	std::cout<<"["<<LongestCommonPrefix(std::vector<std::string>{"abracadabra","abra","abradoodle"})<<"]"<<std::endl;
	std::cout<<"["<<LongestCommonPrefix(std::vector<std::string>{"abracadabra","abra","abradoodle","dabra"})<<"]"<<std::endl;
}
std::vector<std::string> Tokenize(const std::string& str) {
	std::stringstream ss;
	std::vector<std::string> tokens;
	std::string delimString = "";
	std::string comp;
	for (char c : str) {
		if (std::isspace(c) || c == ',' || c == ']' || c == '['
				|| c == ')' || c == '(' || c == '>' || c == '<' || c == '{'
				|| c == '}') {
			comp = ss.str();
			if (comp.size() > 0) {
				tokens.push_back(comp);
				ss.str("");
			}
		} else {
			ss << c;
		}
	}
	comp = ss.str();
	if (comp.size() > 0) {
		tokens.push_back(comp);
	}
	return tokens;
	/*
	 std::regex re("[\\s,]+");
	 std::sregex_token_iterator it(ss.begin(), ss.end(), re, -1);
	 std::sregex_token_iterator reg_end;
	 std::vector<std::string> tokens;
	 for (; it != reg_end; ++it) {
	 tokens.push_back(it->str());
	 }
	 return tokens;
	 */
}
std::vector<std::string> Split(const std::string &str, char delim,bool keepDelim) {
	std::stringstream ss;
	std::vector<std::string> elems;
	std::string delimString = "";
	delimString += delim;
	std::string comp;
	for (char c : str) {
		if (c == delim) {
			comp = ss.str();
			if (comp.size() > 0) {
				elems.push_back(comp);
				ss.str("");
			}
			if(keepDelim)elems.push_back(delimString);
		} else {
			ss << c;
		}
	}
	comp = ss.str();
	if (comp.size() > 0) {
		elems.push_back(comp);
	}
	return elems;

}
std::string LongestCommonPrefix(const std::vector<std::string>& strs){
    std::string result;
    int l=0;
    int N=strs.size();
    if(N==0)return "";
    std::string ref=strs[0];
    if(N==1)return ref;
    for(int l=0;l<ref.length();l++){
    	char pivot=ref[l];
    	for(int n=1;n<N;n++){
    		if(l>=strs[n].size()||strs[n][l]!=pivot){
    			return result;
    		}
    	}
    	result=ref.substr(0,l+1);
    }
    return result;
}
std::vector<std::string> Split(const std::string &str) {
	std::stringstream ss;
	std::vector<std::string> elems;
	std::string comp;
	for (char c : str) {
		if (std::isspace(c)) {
			comp = ss.str();
			if (comp.size() > 0) {
				elems.push_back(comp);
				ss.str("");
			}
		} else {
			ss << c;
		}
	}
	comp = ss.str();
	if (comp.size() > 0) {
		elems.push_back(comp);
	}
	return elems;

}
void Trim(std::string &s) {
	s.erase(
			std::find_if(s.rbegin(), s.rend(),
					std::not1(std::ptr_fun<int, int>(std::isspace))).base(),
			s.end());
	s.erase(s.begin(),
			std::find_if(s.begin(), s.end(),
					std::not1(std::ptr_fun<int, int>(std::isspace))));

}
bool Contains(const std::string& str, const std::string& pattern) {
	return (str.find(pattern) != std::string::npos);
}
std::string MakeDesktopFile(const std::string& fileName){
	return MakeString()<<GetDesktopDirectory()<<ALY_PATH_SEPARATOR<<fileName;
}
std::string MakeWorkingDirectoryFile(const std::string& fileName){
	return MakeString()<<GetCurrentWorkingDirectory()<<ALY_PATH_SEPARATOR<<fileName;
}
std::string MakeExecutableDirectoryFile(const std::string& fileName){
	return MakeString()<<GetExecutableDirectory()<<ALY_PATH_SEPARATOR<<fileName;
}

int Contains(std::string& str, std::vector<std::string> tokens) {
	int count = 0;
	for (std::string token : tokens) {
		if (Contains(str, token)) {
			count++;
		}
	}
	return count;
}
bool BeginsWith(const std::string& str, const std::string& pattern) {
	if (str.size() > pattern.size()) {
		return (str.substr(0, pattern.size()) == pattern);
	} else {
		return (str == pattern);
	}
}
bool EndsWith(const std::string& str, const std::string& pattern) {
	if (str.size() > pattern.size()) {
		return (str.substr(str.size() - pattern.size(), pattern.size())
				== pattern);
	} else {
		return (str == pattern);
	}
}
std::vector<int> ExtractIntegers(const std::string& str) {
	std::vector<int> buffer;
	std::string current;
	for (char c : str) {
		if (c >= '0' && c <= '9') {
			current += c;
		} else {
			if (current.length() > 0) {
				buffer.push_back(std::atoi(current.c_str()));
				current.clear();
			}
		}
	}
	if (current.length() > 0) {
		buffer.push_back(std::atoi(current.c_str()));
	}
	return buffer;
}
int ExtractInteger(const std::string& str) {
	std::vector<int> values = ExtractIntegers(str);
	if (values.size() > 0) {
		return values[0];
	} else {
		return std::numeric_limits<int>::infinity();
	}
}
std::string ToLower(const std::string& str) {
	std::stringstream ss;
	static const std::locale local;
	for (char c : str) {
		ss << std::tolower(c, local);
	}
	return ss.str();
}
std::string ToUpper(const std::string& str) {
	std::stringstream ss;
	static const std::locale local;
	for (char c : str) {
		ss << std::toupper(c, local);
	}
	return ss.str();
}
bool ContainsIgnoreCase(const std::string& str, const std::string& pattern) {
	std::string strl = ToLower(str);
	std::string patternl = ToLower(pattern);
	return (strl.find(patternl) != std::string::npos);
}
}
