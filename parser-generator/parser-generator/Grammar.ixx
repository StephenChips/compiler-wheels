module;

#include <functional>
#include <initializer_list>
#include <variant>
#include <string>

export module Grammar;

export struct ParseResult {
    int result;
};

/*
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
