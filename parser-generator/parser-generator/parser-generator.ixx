module;

#include <memory>
#include <map>
#include <type_traits>
#include <functional>

export module ParserGenerator;

export struct ParseResult;

export template<typename ValueType>
using OnParseCallback = std::function<ValueType(std::vector<ParseResult>)>;

export template <typename ValueType> class NonTerminator {
    OnParseCallback<ValueType> whenParsedSuccessfully;
public:
    NonTerminator& produce(const char* s, OnParseCallback<ValueType>&& lambda);
    NonTerminator& produce(const char* s);
};

export template <typename ValueType> class Terminator {
    OnParseCallback<ValueType> whenParsedSuccessfully;
public:
    void matches(const char* s);
    void matches(const char* s, OnParseCallback<ValueType>&& lambda);
};

export template<typename ValueType> class Grammar {
public:
    NonTerminator<ValueType>& rule(const char* s);
    Terminator<ValueType>& token(const char* s, const char* regex);
    Terminator<ValueType>& token(const char* s, const char* regex, OnParseCallback<ValueType>&& lambda);
};

export template <typename ValueType> class Parser {
public:
    virtual ValueType run() = 0;
};

export template <typename ValueType>
class LL1Parser : public Parser<ValueType> {};

export template <typename ValueType>
class LR1Parser : public Parser<ValueType> {};


