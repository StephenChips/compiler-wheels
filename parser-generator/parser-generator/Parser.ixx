module;

#include <string>
#include <memory>

export module Parser;
import Grammar;
import Lexer;

export class Parser {
protected:
public:
    virtual bool test(const std::string& s) const = 0;
};

struct LL1Table {};
struct LR1Table {};

export class LL1Parser : public Parser {
    std::unique_ptr<LL1Table> table;
    std::unique_ptr<Lexer> lexer; // late init
public:
    LL1Parser(const LL1Table& table, std::unique_ptr<Lexer>& lexer) : lexer(std::move(lexer)) {}

    static std::unique_ptr<LL1Parser> create(const Grammar& grammar);
};

export class LR1Parser : public Parser {
    std::unique_ptr<LR1Table> table;
    std::unique_ptr<Lexer> lexer;
public:
    LR1Parser(const Grammar& grammar, std::unique_ptr<Lexer>& lexer) : lexer(std::move(lexer)) {}

    static std::unique_ptr<LR1Parser> create(const Grammar& grammar);
};
