#pragma once
#include <string>

enum class token_type
{
  keyword,
  identifier,
  literal,
  operator_,  // Use `operator_` to avoid conflicts with the `operator` keyword
  punctuation,
  eof
};

struct token
{
  token_type type;
  std::string value;

  token(token_type token_type, std::string val)
      : type(token_type)
      , value(std::move(val))
  {
  }
};
