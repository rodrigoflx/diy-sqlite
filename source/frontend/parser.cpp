#include "parser.hpp"

// Constructor: Instantiate the tokenizer and generate the token stream.
parser::parser(const std::string& input)
    : m_input(input)
    , m_pos(0)
    , m_tokenizer(input)
{
  m_tokens = m_tokenizer.tokenize();
}

// Helper: Consume a token of a specific type (and optionally a specific value).
tl::expected<token, parse_error> parser::consume(token_type type,
                                                 const std::string& value)
{
  if (m_tokens[m_pos].type != type) {
    return tl::make_unexpected(parse_error::mismatching_type);
  }
  if (!value.empty() && m_tokens[m_pos].value != value) {
    return tl::make_unexpected(parse_error::mismatching_value);
  }
  return m_tokens[m_pos++];
}

// Helper: Lookahead at the current token without consuming.
token parser::peek() const
{
  return m_tokens[m_pos];
}

// --- SELECT statement ---
// Grammar: SELECT select_list FROM table_reference [WHERE condition] [JOIN
// join_clause] ";" ;
tl::expected<select_statement, parse_error> parser::parse_select()
{
  if (auto result = consume(token_type::keyword, "SELECT"); !result) {
    return tl::make_unexpected(result.error());
  }

  select_statement stmt;
  // Parse select_list: either "*" or a comma-separated list.
  if (peek().value == "*") {
    if (auto star = consume(token_type::punctuation, "*"); !star) {
      return tl::make_unexpected(star.error());
    }
    stmt.columns.push_back(std::string("*"));
  } else {
    while (true) {
      auto col = consume(token_type::identifier, "");
      if (col.has_value()) {
        stmt.columns.push_back(col.value().value);
      } else {
        return tl::make_unexpected(col.error());
      }
      if (peek().type == token_type::punctuation && peek().value == ",") {
        if (auto comma = consume(token_type::punctuation, ","); !comma) {
          return tl::make_unexpected(comma.error());
        }
      } else {
        break;
      }
    }
  }

  if (auto from = consume(token_type::keyword, "FROM"); !from) {
    return tl::make_unexpected(from.error());
  }
  auto table = consume(token_type::identifier, "");
  if (table.has_value()) {
    stmt.table = table.value().value;
  } else {
    return tl::make_unexpected(table.error());
  }

  if (peek().type == token_type::keyword && peek().value == "WHERE") {
    if (auto where_kw = consume(token_type::keyword, "WHERE"); !where_kw) {
      return tl::make_unexpected(where_kw.error());
    }
    auto cond = parse_condition();
    if (!cond) {
      return tl::make_unexpected(cond.error());
    }
    stmt.where_clause = cond.value();
  }

  if (peek().type == token_type::keyword && peek().value == "JOIN") {
    auto join = parse_join_clause();
    if (!join) {
      return tl::make_unexpected(join.error());
    }
    stmt.join_clause = join.value();
  }

  if (auto semi = consume(token_type::punctuation, ";"); !semi) {
    return tl::make_unexpected(semi.error());
  }
  return stmt;
}

// --- JOIN clause ---
// Grammar: JOIN table_name ON join_condition ;
tl::expected<join_clause, parse_error> parser::parse_join_clause()
{
  join_clause join;
  if (auto join_kw = consume(token_type::keyword, "JOIN"); !join_kw) {
    return tl::make_unexpected(join_kw.error());
  }
  const auto table = consume(token_type::identifier, "");
  if (table.has_value()) {
    join.table = table.value().value;
  } else {
    return tl::make_unexpected(table.error());
  }
  if (auto on_kw = consume(token_type::keyword, "ON"); !on_kw) {
    return tl::make_unexpected(on_kw.error());
  }

  condition cond;
  const auto left = consume(token_type::identifier, "");
  if (left.has_value()) {
    cond.column = left.value().value;
  } else {
    return tl::make_unexpected(left.error());
  }
  if (auto eq = consume(token_type::operator_, "="); !eq) {
    return tl::make_unexpected(eq.error());
  }
  const auto right = consume(token_type::identifier, "");
  if (right.has_value()) {
    cond.value = right.value().value;
  } else {
    return tl::make_unexpected(right.error());
  }
  cond.op = "=";
  join.on = cond;
  return join;
}

// --- INSERT statement ---
// Grammar: INSERT INTO table_name "(" column_list ")" VALUES "(" value_list ")"
// ";" ;
tl::expected<insert_statement, parse_error> parser::parse_insert()
{
  if (auto ins = consume(token_type::keyword, "INSERT"); !ins) {
    return tl::make_unexpected(ins.error());
  }
  if (auto into = consume(token_type::keyword, "INTO"); !into) {
    return tl::make_unexpected(into.error());
  }

  insert_statement stmt;
  const auto table = consume(token_type::identifier, "");
  if (table.has_value()) {
    stmt.table = table.value().value;
  } else {
    return tl::make_unexpected(table.error());
  }

  if (auto lparen = consume(token_type::punctuation, "("); !lparen) {
    return tl::make_unexpected(lparen.error());
  }
  while (true) {
    const auto col = consume(token_type::identifier, "");
    if (col.has_value()) {
      stmt.columns.push_back(col.value().value);
    } else {
      return tl::make_unexpected(col.error());
    }
    if (peek().type == token_type::punctuation && peek().value == ",") {
      if (auto comma = consume(token_type::punctuation, ","); !comma) {
        return tl::make_unexpected(comma.error());
      }
    } else {
      break;
    }
  }
  if (auto rparen = consume(token_type::punctuation, ")"); !rparen) {
    return tl::make_unexpected(rparen.error());
  }

  if (auto values_kw = consume(token_type::keyword, "VALUES"); !values_kw) {
    return tl::make_unexpected(values_kw.error());
  }
  if (auto lparen2 = consume(token_type::punctuation, "("); !lparen2) {
    return tl::make_unexpected(lparen2.error());
  }
  while (true) {
    const auto val = consume(token_type::literal, "");
    if (val.has_value()) {
      stmt.values.push_back(val.value().value);
    } else {
      return tl::make_unexpected(val.error());
    }
    if (peek().type == token_type::punctuation && peek().value == ",") {
      if (auto comma = consume(token_type::punctuation, ","); !comma) {
        return tl::make_unexpected(comma.error());
      }
    } else {
      break;
    }
  }
  if (auto rparen2 = consume(token_type::punctuation, ")"); !rparen2) {
    return tl::make_unexpected(rparen2.error());
  }
  if (auto semi = consume(token_type::punctuation, ";"); !semi) {
    return tl::make_unexpected(semi.error());
  }
  return stmt;
}

// --- UPDATE statement ---
// Grammar: UPDATE table_name SET assignment_list [WHERE condition] ";" ;
tl::expected<update_statement, parse_error> parser::parse_update()
{
  if (auto upd = consume(token_type::keyword, "UPDATE"); !upd) {
    return tl::make_unexpected(upd.error());
  }
  update_statement stmt;
  const auto table = consume(token_type::identifier, "");
  if (table.has_value()) {
    stmt.table = table.value().value;
  } else {
    return tl::make_unexpected(table.error());
  }

  if (auto set_kw = consume(token_type::keyword, "SET"); !set_kw) {
    return tl::make_unexpected(set_kw.error());
  }
  while (true) {
    std::string col;
    const auto col_tok = consume(token_type::identifier, "");
    if (col_tok.has_value()) {
      col = col_tok.value().value;
    } else {
      return tl::make_unexpected(col_tok.error());
    }
    if (auto eq = consume(token_type::operator_, "="); !eq) {
      return tl::make_unexpected(eq.error());
    }
    const auto val = consume(token_type::literal, "");
    if (val.has_value()) {
      stmt.assignments.emplace_back(col, val.value().value);
    } else {
      return tl::make_unexpected(val.error());
    }
    if (peek().type == token_type::punctuation && peek().value == ",") {
      if (auto comma = consume(token_type::punctuation, ","); !comma) {
        return tl::make_unexpected(comma.error());
      }
    } else {
      break;
    }
  }
  if (peek().type == token_type::keyword && peek().value == "WHERE") {
    if (auto where_kw = consume(token_type::keyword, "WHERE"); !where_kw) {
      return tl::make_unexpected(where_kw.error());
    }
    auto cond = parse_condition();
    if (!cond) {
      return tl::make_unexpected(cond.error());
    }
    stmt.where_clause = cond.value();
  }
  if (auto semi = consume(token_type::punctuation, ";"); !semi) {
    return tl::make_unexpected(semi.error());
  }
  return stmt;
}

// --- DELETE statement ---
// Grammar: DELETE FROM table_name [WHERE condition] ";" ;
tl::expected<delete_statement, parse_error> parser::parse_delete()
{
  if (auto del_kw = consume(token_type::keyword, "DELETE"); !del_kw) {
    return tl::make_unexpected(del_kw.error());
  }
  if (auto from_kw = consume(token_type::keyword, "FROM"); !from_kw) {
    return tl::make_unexpected(from_kw.error());
  }
  delete_statement stmt;
  const auto table = consume(token_type::identifier, "");
  if (table.has_value()) {
    stmt.table = table.value().value;
  } else {
    return tl::make_unexpected(table.error());
  }

  if (peek().type == token_type::keyword && peek().value == "WHERE") {
    if (auto where_kw = consume(token_type::keyword, "WHERE"); !where_kw) {
      return tl::make_unexpected(where_kw.error());
    }
    auto cond = parse_condition();
    if (!cond) {
      return tl::make_unexpected(cond.error());
    }
    stmt.where_clause = cond.value();
  }
  if (auto semi = consume(token_type::punctuation, ";"); !semi) {
    return tl::make_unexpected(semi.error());
  }
  return stmt;
}

// --- WHERE clause / condition ---
// Grammar: condition ::= column_name operator value ;
tl::expected<condition, parse_error> parser::parse_condition()
{
  condition cond;
  const auto col = consume(token_type::identifier, "");
  if (col.has_value()) {
    cond.column = col.value().value;
  } else {
    return tl::make_unexpected(col.error());
  }
  auto op = consume(token_type::operator_, "");
  if (op.has_value()) {
    if (op.value().value != "=" && op.value().value != "!=" && op.value().value != "<"
        && op.value().value != ">" && op.value().value != "<=" && op.value().value != ">=")
    {
      return tl::make_unexpected(parse_error::invalid_operator);
    }
    cond.op = op.value().value;
  } else {
    return tl::make_unexpected(op.error());
  }
  auto val = consume(token_type::literal, "");
  if (!val) {
    return tl::make_unexpected(val.error());
  }
  cond.value = val.value().value;
  return cond;
}

// --- Top-level statement ---
// Grammar: statement ::= select_statement | insert_statement | update_statement
// | delete_statement | ";" ;
tl::expected<statement_variant, parse_error> parser::parse_statement()
{
  if (peek().type == token_type::punctuation && peek().value == ";") {
    if (auto semi = consume(token_type::punctuation, ";"); !semi) {
      return tl::make_unexpected(semi.error());
    }
    return statement_variant(empty_statement {});
  }

  token tok = peek();
  if (tok.type != token_type::keyword) {
    return tl::make_unexpected(parse_error::unknown_statement);
  }

  if (tok.value == "SELECT") {
    auto selectStmt = parse_select();
    if (!selectStmt) {
      return tl::make_unexpected(selectStmt.error());
    }
    return statement_variant(selectStmt.value());
  } else if (tok.value == "INSERT") {
    auto insertStmt = parse_insert();
    if (!insertStmt) {
      return tl::make_unexpected(insertStmt.error());
    }
    return statement_variant(insertStmt.value());
  } else if (tok.value == "UPDATE") {
    auto updateStmt = parse_update();
    if (!updateStmt) {
      return tl::make_unexpected(updateStmt.error());
    }
    return statement_variant(updateStmt.value());
  } else if (tok.value == "DELETE") {
    auto deleteStmt = parse_delete();
    if (!deleteStmt) {
      return tl::make_unexpected(deleteStmt.error());
    }
    return statement_variant(deleteStmt.value());
  }

  return tl::make_unexpected(parse_error::unknown_statement);
}
