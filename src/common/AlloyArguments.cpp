/*
 * AlloyArguments.cpp
 *
 *  Created on: Sep 18, 2018
 *      Author: blake
 */
#include "common/AlloyArguments.h"
#include "system/AlloyFileUtil.h"
namespace aly {

std::string ArgumentParser::delimit(const std::string& name) {
	return std::string(std::min(name.size(), (size_t) 2), '-').append(name);
}
std::string ArgumentParser::strip(const std::string& name) {
	size_t begin = 0;
	begin += name.size() > 0 ? name[0] == '-' : 0;
	begin += name.size() > 3 ? name[1] == '-' : 0;
	return name.substr(begin);
}
std::string ArgumentParser::upper(const std::string& in) {
	std::string out(in);
	std::transform(out.begin(), out.end(), out.begin(), ::toupper);
	return out;
}
std::string ArgumentParser::escape(const std::string& in) {
	std::string out(in);
	if (in.find(' ') != std::string::npos)
		out = std::string("\"").append(out).append("\"");
	return out;
}

Argument::Argument() :
		short_name(""), name(""), optional(true), fixed_nargs(0), fixed(true) {
}
Argument::Argument(const std::string& _short_name, const std::string& _name,
		bool _optional, char nargs) :
		short_name(_short_name), name(_name), optional(_optional) {
	if (nargs == '+' || nargs == '*') {
		variable_nargs = nargs;
		fixed = false;
	} else {
		fixed_nargs = nargs;
		fixed = true;
	}
}
std::string Argument::canonicalName() const {
	return (name.empty()) ? short_name : name;
}
std::string Argument::toString(bool named) const {
	std::ostringstream s;
	std::string uname =
			name.empty() ?
					ArgumentParser::upper(ArgumentParser::strip(short_name)) :
					ArgumentParser::upper(ArgumentParser::strip(name));
	if (named && optional)
		s << "[";
	if (named)
		s << canonicalName();
	if (fixed) {
		size_t N = std::min((size_t) 3, fixed_nargs);
		for (size_t n = 0; n < N; ++n)
			s << " " << uname;
		if (N < fixed_nargs)
			s << " ...";
	}
	if (!fixed) {
		s << " ";
		if (variable_nargs == '*')
			s << "[";
		s << uname << " ";
		if (variable_nargs == '+')
			s << "[";
		s << uname << "...]";
	}
	if (named && optional)
		s << "]";
	return s.str();
}
void ArgumentParser::argumentError(const std::string& msg, bool show_usage) {
	if (use_exceptions_)
		throw std::invalid_argument(msg);
	std::cerr << "ArgumentParser error: " << msg << std::endl;
	if (show_usage)
		std::cerr << usage() << std::endl;
	exit(-5);
}

void ArgumentParser::insertArgument(const Argument& arg) {
	size_t N = arguments_.size();
	arguments_.push_back(arg);
	if (arg.fixed && arg.fixed_nargs <= 1) {
		variables_.push_back(std::string());
	} else {
		variables_.push_back(StringVector());
	}
	if (!arg.short_name.empty())
		index_[arg.short_name] = N;
	if (!arg.name.empty())
		index_[arg.name] = N;
	if (!arg.optional)
		required_++;
}
ArgumentParser::ArgumentParser() :
		ignore_first_(true), use_exceptions_(false), required_(0) {
#ifdef ALY_WINDOWS
	parser.ignoreFirstArgument(false);
#endif
}

void ArgumentParser::setAppName(const std::string& name) {
	app_name_ = name;
}
void ArgumentParser::addArgument(const std::string& name, char nargs,
		bool optional) {
	if (name.size() > 2) {
		Argument arg("", verify(name), optional, nargs);
		insertArgument(arg);
	} else {
		Argument arg(verify(name), "", optional, nargs);
		insertArgument(arg);
	}
}
void ArgumentParser::addArgument(const std::string& short_name,
		const std::string& name, char nargs, bool optional) {
	Argument arg(verify(short_name), verify(name), optional, nargs);
	insertArgument(arg);
}
void ArgumentParser::addFinalArgument(const std::string& name, char nargs,
		bool optional) {
	final_name_ = delimit(name);
	Argument arg("", final_name_, optional, nargs);
	insertArgument(arg);
}
std::vector<int> ArgumentParser::getIntVector(const std::string& name) {
	std::vector<int> vals;
	std::vector<std::string> strs=get<StringVector>(name);
	vals.reserve(strs.size());
	for(std::string str:strs){
		vals.push_back(std::atoi(str.c_str()));
	}
	return vals;
}
std::vector<float> ArgumentParser::getFloatVector(const std::string& name) {
	std::vector<float> vals;
	std::vector<std::string> strs=get<StringVector>(name);
	vals.reserve(strs.size());
	for(std::string str:strs){
		vals.push_back(std::atof(str.c_str()));
	}
	return vals;
}
std::vector<std::string> ArgumentParser::getStringVector(const std::string& name) {
	return get<StringVector>(name);
}
int ArgumentParser::getInt(const std::string& name) {
	return std::atoi(get<std::string>(name).c_str());
}
float ArgumentParser::getFloat(const std::string& name) {
	return std::atof(get<std::string>(name).c_str());
}
std::string ArgumentParser::getString(const std::string& name) {
	return get<std::string>(name);
}
void ArgumentParser::ignoreFirstArgument(bool ignore_first) {
	ignore_first_ = ignore_first;
}
std::string ArgumentParser::verify(const std::string& name) {
	if (name.empty())
		argumentError("argument names must be non-empty");
	if ((name.size() == 2 && name[0] != '-') || name.size() == 3)
		argumentError(
				std::string("invalid argument '").append(name).append(
						"'. Short names must begin with '-'"));
	if (name.size() > 3 && (name[0] != '-' || name[1] != '-'))
		argumentError(
				std::string("invalid argument '").append(name).append(
						"'. Multi-character names must begin with '--'"));
	return name;
}
// --------------------------------------------------------------------------
// Parse
// --------------------------------------------------------------------------
void ArgumentParser::parse(size_t argc, const char** argv) {
	parse(StringVector(argv, argv + argc));
}

void ArgumentParser::parse(const StringVector& argv) {
	// check if the app is named
	if (app_name_.empty() && ignore_first_ && !argv.empty())
		app_name_ = argv[0];

	// set up the working set
	Argument active;
	Argument final =
			final_name_.empty() ? Argument() : arguments_[index_[final_name_]];
	size_t consumed = 0;
	size_t nrequired = final.optional ? required_ : required_ - 1;
	size_t nfinal =
			final.optional ?
					0 :
					(final.fixed ?
							final.fixed_nargs :
							(final.variable_nargs == '+' ? 1 : 0));

	// iterate over each element of the array
	for (StringVector::const_iterator in = argv.begin() + ignore_first_;
			in < argv.end() - nfinal; ++in) {
		std::string active_name = active.canonicalName();
		std::string el = *in;
		//  check if the element is a key
		if (index_.count(el) == 0) {
			// input
			// is the current active argument expecting more inputs?
			if (active.fixed && active.fixed_nargs <= consumed)
				argumentError(
						std::string("attempt to pass too many inputs to ").append(
								active_name), true);
			if (active.fixed && active.fixed_nargs == 1) {
				variables_[index_[active_name]] = el;
			} else {
				auto vec = AnyCast<StringVector>(
						variables_[index_[active_name]]);
				vec.push_back(el);
				variables_[index_[active_name]] = vec;
			}
			consumed++;
		} else {
			// new key!
			// has the active argument consumed enough elements?
			if ((active.fixed && active.fixed_nargs != consumed)
					|| (!active.fixed && active.variable_nargs == '+'
							&& consumed < 1))
				argumentError(
						std::string("encountered argument ").append(el).append(
								" when expecting more inputs to ").append(
								active_name), true);
			active = arguments_[index_[el]];
			// check if we've satisfied the required arguments
			if (active.optional && nrequired > 0)
				argumentError(
						std::string("encountered optional argument ").append(el).append(
								" when expecting more required arguments"),
						true);
			// are there enough arguments for the new argument to consume?
			if ((active.fixed
					&& active.fixed_nargs > (argv.end() - in - nfinal - 1))
					|| (!active.fixed && active.variable_nargs == '+'
							&& !(argv.end() - in - nfinal - 1)))
				argumentError(
						std::string("too few inputs passed to argument ").append(
								el), true);
			if (!active.optional)
				nrequired--;
			consumed = 0;
		}
	}

	for (StringVector::const_iterator in = std::max(
			argv.begin() + ignore_first_, argv.end() - nfinal);
			in != argv.end(); ++in) {
		std::string el = *in;
		// check if we accidentally find an argument specifier
		if (index_.count(el))
			argumentError(
					std::string("encountered argument specifier ").append(el).append(
							" while parsing final required inputs"), true);
		if (final.fixed && final.fixed_nargs == 1) {
			variables_[index_[final_name_]] = el;
		} else {
			auto vec = AnyCast<StringVector>(variables_[index_[final_name_]]);
			vec.push_back(el);
			variables_[index_[final_name_]] = vec;
		}
		nfinal--;
	}

	// check that all of the required arguments have been encountered
	if (nrequired > 0 || nfinal > 0)
		argumentError(
				std::string("too few required arguments passed to ").append(
						app_name_), true);
}

std::string ArgumentParser::usage() {
	// premable app name
	std::ostringstream help;
	help << "Usage: " << escape(app_name_);
	size_t indent = help.str().size();
	size_t linelength = 0;

	// get the required arguments
	for (ArgumentVector::const_iterator it = arguments_.begin();
			it != arguments_.end(); ++it) {
		Argument arg = *it;
		if (arg.optional)
			continue;
		if (arg.name.compare(final_name_) == 0)
			continue;
		help << " ";
		std::string argstr = arg.toString();
		if (argstr.size() + linelength > 80) {
			help << "\n" << std::string(indent, ' ');
			linelength = 0;
		} else {
			linelength += argstr.size();
		}
		help << argstr;
	}

	// get the optional arguments
	for (ArgumentVector::const_iterator it = arguments_.begin();
			it != arguments_.end(); ++it) {
		Argument arg = *it;
		if (!arg.optional)
			continue;
		if (arg.name.compare(final_name_) == 0)
			continue;
		help << " ";
		std::string argstr = arg.toString();
		if (argstr.size() + linelength > 80) {
			help << "\n" << std::string(indent, ' ');
			linelength = 0;
		} else {
			linelength += argstr.size();
		}
		help << argstr;
	}

	// get the final argument
	if (!final_name_.empty()) {
		Argument arg = arguments_[index_[final_name_]];
		std::string argstr = arg.toString(false);
		if (argstr.size() + linelength > 80) {
			help << "\n" << std::string(indent, ' ');
			linelength = 0;
		} else {
			linelength += argstr.size();
		}
		help << argstr;
	}

	return help.str();
}
void ArgumentParser::useExceptions(bool state) {
	use_exceptions_ = state;
}
bool ArgumentParser::empty() const {
	return index_.empty();
}
void ArgumentParser::clear() {
	ignore_first_ = true;
	required_ = 0;
	index_.clear();
	arguments_.clear();
	variables_.clear();
}
bool ArgumentParser::exists(const std::string& name) const {
	return index_.count(delimit(name)) > 0;
}
bool ArgumentParser::has(const std::string& name) {
	return (count(name) > 0);
}
size_t ArgumentParser::count(const std::string& name) {
	// check if the name is an argument
	if (index_.count(delimit(name)) == 0)
		return 0;
	size_t N = index_[delimit(name)];
	Argument arg = arguments_[N];
	Any var = variables_[N];
	if (!var.isEmpty()) {
		// check if the argument is a vector
		if (arg.fixed) {
			return !AnyCast<std::string>(var).empty();
		} else {
			return AnyCast<StringVector>(var).size();
		}
	}
	return 0;
}
}
