#include "tokenizer.hpp"

#include "utils.hpp"

std::vector<token> tokenizer::tokenize()
{
  std::vector<token> tokens;

  while (m_pos < m_input.size()) {
    if (std::isspace(static_cast<unsigned char>(m_input[m_pos]))) {
      m_pos++;
    } else if (std::isalpha(static_cast<unsigned char>(m_input[m_pos]))) {
      tokens.push_back(tokenize_identifier_or_keyword());
    } else if (std::isdigit(static_cast<unsigned char>(m_input[m_pos]))) {
      tokens.push_back(tokenize_number());
    } else if (m_input[m_pos] == '\'') {
      tokens.push_back(tokenize_string_literal());
    } else if (isoperator(m_input[m_pos])) {
      tokens.push_back(tokenize_operator());
    } else if (ispunctuation(m_input[m_pos])) {
      tokens.push_back(
          token(token_type::punctuation, std::string(1, m_input[m_pos++])));
    } else {
      throw std::runtime_error("Unexpected character: "
                               + std::string(1, m_input[m_pos]));
    }
  }

  tokens.push_back(token(token_type::eof, ""));
  return tokens;
};

token tokenizer::tokenize_identifier_or_keyword()
{
  size_t start = m_pos;
  while (m_pos < m_input.size()
         && (std::isalnum(static_cast<unsigned char>(m_input[m_pos])) || m_input[m_pos] == '_'))
    m_pos++;
  std::string word = m_input.substr(start, m_pos - start);
  token_type type =
      iskeyword(word) ? token_type::keyword : token_type::identifier;
  return token(type, word);
};

token tokenizer::tokenize_number()
{
  size_t start = m_pos;
  while (m_pos < m_input.size()
         && (std::isdigit(static_cast<unsigned char>(m_input[m_pos])) || m_input[m_pos] == '.'))
    m_pos++;
  return token(token_type::literal, m_input.substr(start, m_pos - start));
}

token tokenizer::tokenize_string_literal()
{
  m_pos++;  // Skip opening quote
  size_t start = m_pos;
  while (m_pos < m_input.size() && m_input[m_pos] != '\'')
    m_pos++;
  if (m_pos >= m_input.size())
    throw std::runtime_error("Unterminated string literal");
  std::string str_literal = m_input.substr(start, m_pos - start);
  m_pos++;  // Skip closing quote
  return token(token_type::literal, str_literal);
}

token tokenizer::tokenize_operator()
{
  std::string op(1, m_input[m_pos]);
  m_pos++;
  if ((op == "=" || op == "<" || op == ">" || op == "!")
      && m_pos < m_input.size() && m_input[m_pos] == '=')
  {
    op += m_input[m_pos++];
  }
  return token(token_type::operator_, op);
}