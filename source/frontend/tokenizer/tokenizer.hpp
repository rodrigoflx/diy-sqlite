#include <vector>
#include <string>
#include <stdexcept>
#include "token.hpp"

class Tokenizer {
public:
    explicit Tokenizer(const std::string& input) : input_(input), pos_(0) {}
    std::vector<Token> tokenize();

private:
    std::string input_;
    size_t pos_;
    Token tokenizeIdentifierOrKeyword();
    Token tokenizeNumber();
    Token tokenizeStringLiteral();
    Token tokenizeOperator();
};