#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include "input_buffer.hpp"
#include "frontend/parser/parser.hpp"  // Include your parser header file
#include "frontend/tokenizer/tokenizer.hpp"

TEST_CASE("Parse SELECT statement", "[parser]") {
    tokenizer tok("SELECT name, age FROM users WHERE age > 18;");
    auto token_vec = tok.tokenize();

    parser p(token_vec);
    auto stmt = p.parse_select();

    REQUIRE(stmt.columns.size() == 2);
    REQUIRE(stmt.columns[0] == "name");
    REQUIRE(stmt.columns[1] == "age");
    REQUIRE(stmt.table == "users");

    REQUIRE(stmt.where_clause.has_value());
    REQUIRE(stmt.where_clause->column == "age");
    REQUIRE(stmt.where_clause->op == ">");
    REQUIRE(stmt.where_clause->value == "18");
}

TEST_CASE("Parse INSERT statement", "[parser]") {
    tokenizer tok("INSERT INTO users (name, age) VALUES ('Alice', 30);");
    auto token_vec = tok.tokenize();
    
    parser p(token_vec);
    auto stmt = p.parse_insert();

    REQUIRE(stmt.table == "users");
    REQUIRE(stmt.columns.size() == 2);
    REQUIRE(stmt.columns[0] == "name");
    REQUIRE(stmt.columns[1] == "age");
    REQUIRE(stmt.values.size() == 2);
    REQUIRE(stmt.values[0] == "Alice");
    REQUIRE(stmt.values[1] == "30");
}

TEST_CASE("Parse UPDATE statement", "[parser]") {
    tokenizer tok("UPDATE users SET age = 31 WHERE name = 'Alice';");
    auto token_vec = tok.tokenize();
    
    parser p(token_vec);
    auto stmt = p.parse_update();

    REQUIRE(stmt.table == "users");
    REQUIRE(stmt.assignments.size() == 1);
    REQUIRE(stmt.assignments[0].first == "age");
    REQUIRE(stmt.assignments[0].second == "31");

    REQUIRE(stmt.where_clause.has_value());
    REQUIRE(stmt.where_clause->column == "name");
    REQUIRE(stmt.where_clause->op == "=");
    REQUIRE(stmt.where_clause->value == "Alice");
}


