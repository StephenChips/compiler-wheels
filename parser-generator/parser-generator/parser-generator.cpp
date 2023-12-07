// parser-generator.cpp : Defines the entry point for the application.
//
#include <memory>
#include <map>
#include <type_traits>
#include <functional>
#include <fmt/core.h>

struct ParseResult {};

template<typename ValueType>
using OnParseCallback = std::function<ValueType(std::vector<ParseResult>)>;


template <typename ValueType>
class NonTerminator {
    OnParseCallback<ValueType> whenParsedSuccessfully;
public:
    NonTerminator& produce(const char* s, OnParseCallback<ValueType>&& lambda) {
        return *this;
    }

    NonTerminator& produce(const char* s) {
        return *this;
    }
};


template <typename ValueType>
class Terminator {
    OnParseCallback<ValueType> whenParsedSuccessfully;
public:
    void matches(const char* s) {

    }

    void matches(const char* s, OnParseCallback<ValueType>&& lambda) {

    }
};

template<typename ValueType>
class Grammar {
public:
    NonTerminator<ValueType>& rule(const char* s) {

    }

    Terminator<ValueType>& token(const char* s, const char* regex) {

    }

    Terminator<ValueType>& token(const char* s, const char* regex, OnParseCallback<ValueType>&& lambda) {}
};

template <typename ValueType>
class Parser {
public:
    virtual ValueType run() = 0;
};

template <typename ValueType>
class LL1Parser : public Parser<ValueType> {
public:
    LL1Parser(const Grammar<ValueType>& grammar) {

    }
    virtual ValueType run() override {

    }
};
