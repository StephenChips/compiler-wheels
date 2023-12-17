module;

#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include <set>
#include <variant>

export module ValidateGrammar;
import Grammar;

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
