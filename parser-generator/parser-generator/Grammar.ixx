module;

#include <functional>
#include <initializer_list>
#include <variant>
#include <string>
#include <set>

export module Grammar;

export struct ParseResult {
    int result;
};

/*
* An identifier means any terminator and non-terminators here.
* An identifier means any terminator and non-terminators here.
*/
export struct Identifier {
    std::string name;
};

export Identifier operator ""_id(const char* s, size_t len) {
	return Identifier{ std::string(s, len) };
};

export using OnParseCallback = std::function<void(std::vector<ParseResult>)>;

export struct NonTerminator {
    using ProductionListItem = std::variant<std::string, Identifier>;
    using ProductionList = std::vector<ProductionListItem>;

    std::string name;
    std::vector<ProductionList> productions;
    OnParseCallback whenParsedSuccessfully = nullptr;
};

export struct Terminator {
    std::string name;
    std::string regexString;
    OnParseCallback whenParsedSuccessfully = nullptr;
};

export struct Grammar {
    std::vector<Terminator> terminators;
    std::vector<NonTerminator> nonTerminators;
};

export bool isGrammarValid(const Grammar& grammar) {
	std::set<std::string> definedIdentifiers;

	for (const auto& terminator : grammar.terminators) {
		if (!definedIdentifiers.insert(terminator.name).second) {
			// Found a re-defined identifier.
			return false;
		}
	}

	for (const auto& nonTerminator : grammar.nonTerminators) {
		if (!definedIdentifiers.insert(nonTerminator.name).second) {
			// Found a re-defined identifier
			return false;
		}
	}

	for (const auto& nonTerminator : grammar.nonTerminators) {
		for (const auto& productionList : nonTerminator.productions) {
			for (const auto& listItem : productionList) {
				auto symbol = std::get_if<Identifier>(&listItem);
				if (symbol != nullptr && !definedIdentifiers.contains(symbol->name)) {
					return false;
				}
			}
		}
	}

	return true;
}
