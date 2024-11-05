#include <string>

enum class TokenType {
    Keyword,
    Identifier,
    Literal,
    Operator,
    Punctuation,
    Eof
};

struct Token {
    TokenType type;
    std::string value;

    Token(TokenType t, const std::string& val) : type(t), value(val) {}
};