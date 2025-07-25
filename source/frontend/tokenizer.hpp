#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <stdexcept>
#include <string>
#include <vector>

#include "token.hpp"

class tokenizer
{
public:
  // Construct with input string
  explicit tokenizer(std::string input)
      : m_input(std::move(input))
      , m_pos(0)
  {
  }

  // Tokenize the input string into a vector of tokens
  std::vector<token> tokenize();

  // Reset the tokenizer to reuse with a new string
  void reset(std::string input)
  {
    m_input = std::move(input);
    m_pos = 0;
  }

private:
  std::string m_input;
  size_t m_pos;
  token tokenize_identifier_or_keyword();
  token tokenize_number();
  token tokenize_string_literal();
  token tokenize_operator();
};

#endif  // TOKENIZER_HPP