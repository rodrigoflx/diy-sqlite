#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include "frontend/tokenizer/tokenizer.hpp"  

TEST_CASE("Tokenize SQL keywords", "[tokenizer]") {
    tokenizer tokenizer("SELECT FROM WHERE");
    auto tokens = tokenizer.tokenize();

    REQUIRE(tokens.size() == 4);  
    REQUIRE(tokens[0].type == token_type::keyword);
    REQUIRE(tokens[0].value == "SELECT");
    REQUIRE(tokens[1].type == token_type::keyword);
    REQUIRE(tokens[1].value == "FROM");
    REQUIRE(tokens[2].type == token_type::keyword);
    REQUIRE(tokens[2].value == "WHERE");
    REQUIRE(tokens[3].type == token_type::eof);
}

TEST_CASE("Tokenize identifiers and literals", "[tokenizer]") {
    tokenizer tokenizer("name age 123 'hello'");
    auto tokens = tokenizer.tokenize();

    REQUIRE(tokens.size() == 5); 
    REQUIRE(tokens[0].type == token_type::identifier);
    REQUIRE(tokens[0].value == "name");
    REQUIRE(tokens[1].type == token_type::identifier);
    REQUIRE(tokens[1].value == "age");
    REQUIRE(tokens[2].type == token_type::literal);
    REQUIRE(tokens[2].value == "123");
    REQUIRE(tokens[3].type == token_type::literal);
    REQUIRE(tokens[3].value == "hello");
    REQUIRE(tokens[4].type == token_type::eof);
}

TEST_CASE("Tokenize operators", "[tokenizer]") {
    tokenizer tokenizer("= > < <= >= != +");
    auto tokens = tokenizer.tokenize();

    REQUIRE(tokens.size() == 8);  
    REQUIRE(tokens[0].type == token_type::operator_);
    REQUIRE(tokens[0].value == "=");
    REQUIRE(tokens[1].type == token_type::operator_);
    REQUIRE(tokens[1].value == ">");
    REQUIRE(tokens[2].type == token_type::operator_);
    REQUIRE(tokens[2].value == "<");
    REQUIRE(tokens[3].type == token_type::operator_);
    REQUIRE(tokens[3].value == "<=");
    REQUIRE(tokens[4].type == token_type::operator_);
    REQUIRE(tokens[4].value == ">=");
    REQUIRE(tokens[5].type == token_type::operator_);
    REQUIRE(tokens[5].value == "!=");
    REQUIRE(tokens[6].type == token_type::operator_);
    REQUIRE(tokens[6].value == "+");
    REQUIRE(tokens[7].type == token_type::eof);
}

TEST_CASE("Tokenize punctuation", "[tokenizer]") {
    tokenizer tokenizer(", ; ( )");
    auto tokens = tokenizer.tokenize();

    REQUIRE(tokens.size() == 5);  
    REQUIRE(tokens[0].type == token_type::punctuation);
    REQUIRE(tokens[0].value == ",");
    REQUIRE(tokens[1].type == token_type::punctuation);
    REQUIRE(tokens[1].value == ";");
    REQUIRE(tokens[2].type == token_type::punctuation);
    REQUIRE(tokens[2].value == "(");
    REQUIRE(tokens[3].type == token_type::punctuation);
    REQUIRE(tokens[3].value == ")");
    REQUIRE(tokens[4].type == token_type::eof);
}

TEST_CASE("Tokenize mixed SQL statement", "[tokenizer]") {
    tokenizer tokenizer("SELECT name, age FROM users WHERE age > 18;");
    auto tokens = tokenizer.tokenize();

    REQUIRE(tokens.size() == 12);  
    REQUIRE(tokens[0].type == token_type::keyword);
    REQUIRE(tokens[0].value == "SELECT");
    REQUIRE(tokens[1].type == token_type::identifier);
    REQUIRE(tokens[1].value == "name");
    REQUIRE(tokens[2].type == token_type::punctuation);
    REQUIRE(tokens[2].value == ",");
    REQUIRE(tokens[3].type == token_type::identifier);
    REQUIRE(tokens[3].value == "age");
    REQUIRE(tokens[4].type == token_type::keyword);
    REQUIRE(tokens[4].value == "FROM");
    REQUIRE(tokens[5].type == token_type::identifier);
    REQUIRE(tokens[5].value == "users");
    REQUIRE(tokens[6].type == token_type::keyword);
    REQUIRE(tokens[6].value == "WHERE");
    REQUIRE(tokens[7].type == token_type::identifier);
    REQUIRE(tokens[7].value == "age");
    REQUIRE(tokens[8].type == token_type::operator_);
    REQUIRE(tokens[8].value == ">");
    REQUIRE(tokens[9].type == token_type::literal);
    REQUIRE(tokens[9].value == "18");
    REQUIRE(tokens[10].type == token_type::punctuation);
    REQUIRE(tokens[10].value == ";");
    REQUIRE(tokens[11].type == token_type::eof);
}

TEST_CASE("Handle unterminated string literal", "[tokenizer]") {
    tokenizer tokenizer("SELECT 'hello");
    REQUIRE_THROWS_WITH(tokenizer.tokenize(), "Unterminated string literal");
}

TEST_CASE("Handle unexpected characters", "[tokenizer]") {
    tokenizer tokenizer("SELECT #");
    REQUIRE_THROWS_WITH(tokenizer.tokenize(), "Unexpected character: #");
}
