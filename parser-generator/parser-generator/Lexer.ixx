module;

#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include <iterator>

/*
* The lexer accepts following regex syntax:
* 
* 1. The fundamentals: 
*   a. ab (concatenation)
*   b. a|b (branching)
*   c. a* (appears for any times, include zero)
* 2. frequently used features:
*   a. a+ (appears once or more)
*   b. a? (optional),
*   c. [abc], [^abc], [a-z] ... (range matching)
*   f. a{3}, a{3, 5}, a{, 10} (repetition)
*/

export module Lexer;
import Grammar;

export struct Token {};

export class Lexer {
public:
    Lexer(const std::vector<Terminator>&) {};
    Token getCurrentToken() { return Token(); };
    Token getNextToken() { return Token(); }
};

