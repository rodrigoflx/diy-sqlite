#pragma once

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "tl/expected.hpp"  // Provides tl::expected

#include "token.hpp"  // Provides token, token_type, etc.
#include "tokenizer.hpp"  // Provides your tokenizer class.

// --- Parse Error Definitions ---
enum class parse_error
{
  mismatching_type,
  mismatching_value,
  invalid_operator,
  unknown_statement
};

// --- AST Node Definitions ---
struct condition
{
  std::string column;
  std::string op;
  std::string value;
};

struct join_clause
{
  std::string table;
  condition on;
};

struct select_statement
{
  std::vector<std::string> columns;  // Either "*" or a list of columns.
  std::string table;
  std::optional<condition> where_clause;
  std::optional<join_clause> join_clause;
};

struct insert_statement
{
  std::string table;
  std::vector<std::string> columns;
  std::vector<std::string> values;
};

struct update_statement
{
  std::string table;
  std::vector<std::pair<std::string, std::string>> assignments;
  std::optional<condition> where_clause;
};

struct delete_statement
{
  std::string table;
  std::optional<condition> where_clause;
};

struct empty_statement
{
};

using statement_variant = std::variant<empty_statement,
                                       select_statement,
                                       insert_statement,
                                       update_statement,
                                       delete_statement>;

// --- Parser Class Declaration ---
class parser
{
public:
  // Construct the parser from an input string.
  explicit parser(const std::string& input);

  // Top-level production for a complete statement.
  tl::expected<statement_variant, parse_error> parse_statement();

  // Productions for each statement type.
  tl::expected<select_statement, parse_error> parse_select();
  tl::expected<insert_statement, parse_error> parse_insert();
  tl::expected<update_statement, parse_error> parse_update();
  tl::expected<delete_statement, parse_error> parse_delete();

  // Additional productions.
  tl::expected<condition, parse_error> parse_condition();
  tl::expected<join_clause, parse_error> parse_join_clause();

private:
  // Helper function to consume a token of a specific type and (optionally) a
  // specific value.
  tl::expected<token, parse_error> consume(token_type type,
                                           const std::string& value = "");

  // Lookahead without consuming.
  token peek() const;

  // Token stream produced by the tokenizer.
  std::vector<token> m_tokens;
  size_t m_pos;

  // The original input and tokenizer instance.
  std::string m_input;
  tokenizer m_tokenizer;
};
