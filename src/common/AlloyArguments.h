/*
 Copyright (c) 2017, Hilton Bristow
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef ALLOYARGPARSE_H_
#define ALLOYARGPARSE_H_
#include "common/AlloyAny.h"
#include <string>
#include <vector>
#include <typeinfo>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <cassert>
#include <algorithm>
#if __cplusplus >= 201103L
#include <unordered_map>
typedef std::unordered_map<std::string, size_t> IndexMap;
#else
#include <map>
typedef std::map<std::string, size_t> IndexMap;
#endif
namespace aly {

/*! @class ArgumentParser
 *  @brief A simple command-line argument parser based on the design of
 *  python's parser of the same name.
 *
 *  ArgumentParser is a simple C++ class that can parse arguments from
 *  the command-line or any array of strings. The syntax is familiar to
 *  anyone who has used python's ArgumentParser:
 *  \code
 *    // create a parser and add the options
 *    ArgumentParser parser;
 *    parser.addArgument("-n", "--name");
 *    parser.addArgument("--inputs", '+');
 *
 *    // parse the command-line arguments
 *    parser.parse(argc, argv);
 *
 *    // get the inputs and iterate over them
 *    string name = parser.retrieve("name");
 *    vector<string> inputs = parser.retrieve<vector<string>>("inputs");
 *  \endcode
 *
 */
struct Argument {
	Argument();
	Argument(const std::string& _short_name, const std::string& _name, bool _optional,
			char nargs);
	std::string short_name;
	std::string name;
	bool optional;
	union {
		size_t fixed_nargs;
		char variable_nargs;
	};
	bool fixed;
	std::string canonicalName() const ;
	std::string toString(bool named = true) const ;
};
class ArgumentParser {
public:
	static std::string delimit(const std::string& name);
	static std::string strip(const std::string& name) ;
	static std::string upper(const std::string& in);
	static std::string escape(const std::string& in) ;

private:
	class PlaceHolder;
	class Holder;
	typedef std::vector<aly::Any> AnyVector;
	typedef std::vector<std::string> StringVector;
	typedef std::vector<Argument> ArgumentVector;
	// --------------------------------------------------------------------------
	// Argument
	// --------------------------------------------------------------------------


	void insertArgument(const Argument& arg);

	// --------------------------------------------------------------------------
	// Error handling
	// --------------------------------------------------------------------------
	void argumentError(const std::string& msg, bool show_usage = false);
	// --------------------------------------------------------------------------
	// Member variables
	// --------------------------------------------------------------------------
	IndexMap index_;
	bool ignore_first_;
	bool use_exceptions_;
	size_t required_;
	std::string app_name_;
	std::string final_name_;
	ArgumentVector arguments_;
	AnyVector variables_;
	template<typename T> T get(const std::string& name) {
		if (index_.count(delimit(name)) == 0)
			throw std::out_of_range("Key not found");
		size_t N = index_[delimit(name)];
		return AnyCast<T>(variables_[N]);
	}
public:
	ArgumentParser();
	// --------------------------------------------------------------------------
	// addArgument
	// --------------------------------------------------------------------------
	void setAppName(const std::string& name) ;
	void addArgument(const std::string& name, char nargs = 0, bool optional = true);
	void addArgument(const std::string& short_name, const std::string& name, char nargs =0, bool optional = true);
	void addFinalArgument(const std::string& name, char nargs = 1, bool optional =false) ;
	void ignoreFirstArgument(bool ignore_first) ;
	std::string verify(const std::string& name);
	// --------------------------------------------------------------------------
	// Parse
	// --------------------------------------------------------------------------
	void parse(size_t argc, const char** argv);
	void parse(const StringVector& argv) ;
	int getInt(const std::string& name);
	float getFloat(const std::string& name);
	std::string getString(const std::string& name);

	std::vector<int> getIntVector(const std::string& name);
	std::vector<float> getFloatVector(const std::string& name);
	std::vector<std::string> getStringVector(const std::string& name);
	// --------------------------------------------------------------------------
	// Properties
	// --------------------------------------------------------------------------
	std::string usage();
	void useExceptions(bool state);
	bool empty() const;
	void clear();
	bool has(const std::string& name);
	bool exists(const std::string& name) const;
	size_t count(const std::string& name);
};
}
#endif
