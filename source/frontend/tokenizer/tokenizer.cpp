#include "tokenizer.hpp"
#include "utils.hpp"

std::vector<Token> Tokenizer::tokenize() {
    std::vector<Token> tokens;

    while (pos_ < input_.size()) {
        if (std::isspace(input_[pos_])) {
            pos_++;
        } else if (std::isalpha(input_[pos_])) {
            tokens.push_back(tokenizeIdentifierOrKeyword());
        } else if (std::isdigit(input_[pos_])) {
            tokens.push_back(tokenizeNumber());
        } else if (input_[pos_] == '\'') {
            tokens.push_back(tokenizeStringLiteral());
        } else if (isOperator(input_[pos_])) {
            tokens.push_back(tokenizeOperator());
        } else if (isPunctuation(input_[pos_])) {
            tokens.push_back(Token(TokenType::Punctuation, std::string(1, input_[pos_++])));
        } else {
            throw std::runtime_error("Unexpected character: " + std::string(1, input_[pos_]));
        }
    }

    tokens.push_back(Token(TokenType::Eof, ""));
    return tokens;
};

Token Tokenizer::tokenizeIdentifierOrKeyword() {
        size_t start = pos_;
        while (pos_ < input_.size() && (std::isalnum(input_[pos_]) || input_[pos_] == '_')) pos_++;
        std::string word = input_.substr(start, pos_ - start);
        TokenType type = isKeyword(word) ? TokenType::Keyword : TokenType::Identifier;
        return Token(type, word);
};

Token Tokenizer::tokenizeNumber() {
        size_t start = pos_;
        while (pos_ < input_.size() && (std::isdigit(input_[pos_]) || input_[pos_] == '.')) pos_++;
        return Token(TokenType::Literal, input_.substr(start, pos_ - start));
    }

Token Tokenizer::tokenizeStringLiteral() {
    pos_++;  // Skip opening quote
    size_t start = pos_;
    while (pos_ < input_.size() && input_[pos_] != '\'') pos_++;
    if (pos_ >= input_.size()) throw std::runtime_error("Unterminated string literal");
    std::string str_literal = input_.substr(start, pos_ - start);
    pos_++;  // Skip closing quote
    return Token(TokenType::Literal, str_literal);
}

Token Tokenizer::tokenizeOperator() {
    std::string op(1, input_[pos_]);
    pos_++;
    if ((op == "=" || op == "<" || op == ">" || op == "!") && pos_ < input_.size() && input_[pos_] == '=') {
        op += input_[pos_++];
    }
    return Token(TokenType::Operator, op);
}