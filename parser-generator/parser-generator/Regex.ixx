module;

#include <string>


export module Regex;

import Automata;

constexpr bool isTheFirstCharOfQualifier(const char ch);

export struct MatchResult {
	bool succeeded;
	std::string::const_iterator endIter;
};

export class Regex {
	Automata automata;
public:
	Regex(const std::string& pattern) {}
	MatchResult match(std::string::const_iterator str);
};

MatchResult Regex::match(std::string::const_iterator str) {
	return MatchResult();
}
