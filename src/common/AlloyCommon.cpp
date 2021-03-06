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
#include <math.h>
namespace aly {
void SANITY_CHECK_STRINGS(){
	std::cout<<"["<<LongestCommonPrefix(std::vector<std::string>{"abra"})<<"]"<<std::endl;
	std::cout<<"["<<LongestCommonPrefix(std::vector<std::string>{"abra","cadabra"})<<"]"<<std::endl;
	std::cout<<"["<<LongestCommonPrefix(std::vector<std::string>{"abra","abracadabra"})<<"]"<<std::endl;
	std::cout<<"["<<LongestCommonPrefix(std::vector<std::string>{"abracadabra","abra","abradoodle"})<<"]"<<std::endl;
	std::cout<<"["<<LongestCommonPrefix(std::vector<std::string>{"abracadabra","abra","abradoodle","dabra"})<<"]"<<std::endl;
}
//no need to reinvent the wheel
//https://www.geeksforgeeks.org/wildcard-character-matching/
bool MatchWildCardInternal(const char *first,const char * second){
    // If we reach at the end of both strings, we are done
    if (*first == '\0' && *second == '\0')
        return true;

    // Make sure that the characters after '*' are present
    // in second string. This function assumes that the first
    // string will not contain two consecutive '*'
    if (*first == '*' && *(first+1) != '\0' && *second == '\0')
        return false;

    // If the first string contains '?', or current characters
    // of both strings match
    if (*first == '?' || *first == *second)
        return MatchWildCardInternal(first+1, second+1);

    // If there is *, then there are two possibilities
    // a) We consider current character of second string
    // b) We ignore current character of second string.
    if (*first == '*')
        return MatchWildCardInternal(first+1, second) || MatchWildCardInternal(first, second+1);
    return false;
}
bool MatchWildCard(const std::string& str, const std::string& pattern){
	return MatchWildCardInternal(str.c_str(),pattern.c_str());
}
bool MatchWildCardIgnoreCase(const std::string& str, const std::string& pattern){
	std::string strTmp=ToUpper(str);
	std::string patTmp=ToUpper(pattern);
	return MatchWildCardInternal(strTmp.c_str(),patTmp.c_str());
}
int ReplaceAll(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    int count=0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
        count++;
    }
    return count;
}
int ReplaceFirst(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    if((start_pos = str.find(from)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        return 1;
    }
    return 0;
}
std::string FormatTimeDuration(double elapsedTime) {
	std::stringstream ss;
	if (elapsedTime == 0) {
		return "0 sec";
	} else {
		int sec = (int) std::floor(elapsedTime);
		if (elapsedTime < 1.0f) {
			ss << (int) (elapsedTime * 1E3) << " ms";
		} else if (elapsedTime < 60.0f) {
			int ms = ((int) (elapsedTime * 1E3)) % 1000;
			if (ms != 0) {
				ss << sec << " sec " << ms << " ms";
			} else {
				ss << sec << " sec";
			}
		} else if (elapsedTime < 3600.0f) {
			if (sec % 60 != 0) {
				ss << sec / 60 << " min " << sec % 60 << " sec";
			} else {
				ss << sec / 60 << " min";
			}
		} else {
			int hrs = (sec / 3600);
			int mins = (sec - hrs * 3600) / 60;
			if (sec % 60 != 0) {
				ss << hrs << " hrs " << mins << " min " << sec % 60 << " sec";
			} else {
				ss << hrs << " hrs " << mins << " min";
			}
		}
		return ss.str();
	}
}
int ReplaceLast(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    if((start_pos = str.rfind(from)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        return 1;
    }
    return 0;
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
std::string& Trim(std::string &s) {
	s.erase(
			std::find_if(s.rbegin(), s.rend(),
					std::not1(std::ptr_fun<int, int>(std::isspace))).base(),
			s.end());
	s.erase(s.begin(),
			std::find_if(s.begin(), s.end(),
					std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;

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
std::string ZeroPad(int num,int width){
	return MakeString()<<std::setw(width)<<std::setfill('0')<<num;
}
std::string ZeroPad(int64_t num,int width){
	return MakeString()<<std::setw(width)<<std::setfill('0')<<num;
}
std::string ZeroPad(uint64_t num,int width){
	return MakeString()<<std::setw(width)<<std::setfill('0')<<num;
}
std::string ZeroPad(uint32_t num,int width){
	return MakeString()<<std::setw(width)<<std::setfill('0')<<num;
}
std::vector<int> ExtractIntegerRange(const std::string& str){
	//List should be comma separated
	std::vector<std::string> tokens=Split(str,',',false);
	std::vector<int> frames;
	for(std::string str:tokens){
		//first try split on '-', should be no negative frames
		std::vector<std::string> split=Split(Trim(str),':',false);
		if(split.size()<=1){//try split again on '-'. Colon is preferred to correctly handle negative numbers
			split=Split(Trim(str),'-',false);
		}
		if(split.size()==2){
			//works, 17-20, 30, 1, 40:47
			int st=std::atoi(Trim(split[0]).c_str());
			int ed=std::atoi(Trim(split[1]).c_str());
			if(st>ed)std::swap(st,ed);//force range to be increasing
			for(int n=st;n<=ed;n++){
				frames.push_back(n);
			}
		} else if(split.size()==1){
			//works, -2,-1,0,1,2
			frames.push_back(atoi(Trim(str).c_str()));
		}
	}
	//Sort increasing and remove duplicated numbers
	std::sort(frames.begin(),frames.end());
	frames.erase(std::unique(frames.begin(),frames.end()),frames.end());
	return frames;
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
Timer::Timer(const std::string& name):name(name){
	lastTime=std::chrono::steady_clock::now();
}
double Timer::getElapsed(bool reset){
	std::chrono::steady_clock::time_point currentTime=std::chrono::steady_clock::now();
	if(reset)lastTime=currentTime;
	return std::chrono::duration<double>(currentTime - lastTime).count();
}
bool Timer::resetAfterElapsed(double timeOut){
	double t=getElapsed(false);
	if(t>=timeOut){
		lastTime=std::chrono::steady_clock::now();
		if(t-timeOut<0.5f*timeOut){
			lastTime-=std::chrono::microseconds((int64_t)(1E6*(t-timeOut)));
		}
		return true;
	}
	return false;
}
void Timer::reset(){
	lastTime=std::chrono::steady_clock::now();
}
void Timer::tic(){
	reset();
}
double Timer::toc() {
	double e=getElapsed(true);
	if(name.size()>0){
		std::cout<<"["<<name<<"] elapsed time = "<<FormatTimeDuration(e)<<std::endl;
	} else {
		std::cout<<"Elapsed time = "<<FormatTimeDuration(e)<<std::endl;
	}
	return e;
}
}
