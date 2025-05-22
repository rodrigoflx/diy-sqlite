#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>

#include "frontend/parser.hpp"  // Include your parser header file
#include "frontend/tokenizer.hpp"
#include "input_buffer.hpp"

TEST_CASE("Parse SELECT statement", "[parser]")
{
  parser p("SELECT name, age FROM users WHERE age > 18;");
  auto stmt_opt = p.parse_select();
  REQUIRE(stmt_opt.has_value());
  auto& stmt = stmt_opt.value();

  REQUIRE(stmt.columns.size() == 2);
  REQUIRE(stmt.columns[0] == "name");
  REQUIRE(stmt.columns[1] == "age");
  REQUIRE(stmt.table == "users");

  REQUIRE(stmt.where_clause.has_value());
  REQUIRE(stmt.where_clause->column == "age");
  REQUIRE(stmt.where_clause->op == ">");
  REQUIRE(stmt.where_clause->value == "18");
}

TEST_CASE("Parse INSERT statement", "[parser]")
{
  parser p("INSERT INTO users (name, age) VALUES ('Alice', 30);");
  auto stmt_opt = p.parse_insert();
  REQUIRE(stmt_opt.has_value());
  auto& stmt = stmt_opt.value();

  REQUIRE(stmt.table == "users");
  REQUIRE(stmt.columns.size() == 2);
  REQUIRE(stmt.columns[0] == "name");
  REQUIRE(stmt.columns[1] == "age");
  REQUIRE(stmt.values.size() == 2);
  REQUIRE(stmt.values[0] == "Alice");
  REQUIRE(stmt.values[1] == "30");
}

TEST_CASE("Parse UPDATE statement", "[parser]")
{
  parser p("UPDATE users SET age = 31 WHERE name = 'Alice';");
  auto stmt_opt = p.parse_update();
  REQUIRE(stmt_opt.has_value());
  auto& stmt = stmt_opt.value();

  REQUIRE(stmt.table == "users");
  REQUIRE(stmt.assignments.size() == 1);
  REQUIRE(stmt.assignments[0].first == "age");
  REQUIRE(stmt.assignments[0].second == "31");

  REQUIRE(stmt.where_clause.has_value());
  REQUIRE(stmt.where_clause->column == "name");
  REQUIRE(stmt.where_clause->op == "=");
  REQUIRE(stmt.where_clause->value == "Alice");
}
