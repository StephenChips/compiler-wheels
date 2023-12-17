module;

#include <memory>
#include <map>
#include <type_traits>
#include <functional>
#include <variant>
#include <initializer_list>
#include <vector>
#include <string>
#include <fmt/core.h>

export module ParserGenerator;

import Lexer;
import Parser;
import ValidateGrammar;
import Grammar;

export import Grammar;

std::unique_ptr<Lexer> findGrammarErrorsAndCreateLexer(const Grammar& grammar) {
    if (!isGrammarValid(grammar)) {
        fmt::print("The grammar is not valid!");
        return nullptr;
    }

    return std::make_unique<Lexer>(grammar.terminators);
}

export std::unique_ptr<Parser> createLL1Parser(const Grammar& grammar) {
    auto lexer = findGrammarErrorsAndCreateLexer(grammar);
    if (!lexer) return nullptr;

    return LL1Parser::create(grammar);
}

export std::unique_ptr<Parser> createLR1Parser(const Grammar& grammar) {
    auto lexer = findGrammarErrorsAndCreateLexer(grammar);
    if (!lexer) return nullptr;

    return LR1Parser::create(grammar);
}
