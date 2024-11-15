#include <vector>
#include <string>
#include <stdexcept>
#include "token.hpp"

class tokenizer {
public:
    explicit tokenizer(const std::string& input) : m_input(input), m_pos(0) {}
    std::vector<token> tokenize();

private:
    std::string m_input;
    size_t m_pos;
    token tokenize_identifier_or_keyword();
    token tokenize_number();
    token tokenize_string_literal();
    token tokenize_operator();
};