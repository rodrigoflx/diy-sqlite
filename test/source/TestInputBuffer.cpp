#define CATCH_CONFIG_MAIN
#include <sstream>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include "input_buffer.hpp"

// Helper function to create an input_buffer from a string
auto create_buffer_from_string(const std::string& data) -> input_buffer
{
  std::istringstream iss(data);
  return input_buffer(iss);
}

TEST_CASE("input_buffer reads lines correctly")
{
  std::istringstream input("line1\nline2\nline3\n");
  input_buffer buffer(input);

  REQUIRE(buffer.eof() == false);

  std::string line = buffer.read_line();
  REQUIRE(line == "line1");
  REQUIRE(buffer.eof() == false);

  line = buffer.read_line();
  REQUIRE(line == "line2");
  REQUIRE(buffer.eof() == false);

  line = buffer.read_line();
  REQUIRE(line == "line3");
  REQUIRE(buffer.eof() == false);

  line = buffer.read_line();  // Should trigger EOF after this
  REQUIRE(line.empty());
  REQUIRE(buffer.eof() == true);
}

TEST_CASE("input_buffer handles trailing newline")
{
  std::istringstream input("First line\nSecond line\n");
  input_buffer buffer(input);

  REQUIRE(buffer.read_line() == "First line");
  REQUIRE(buffer.read_line() == "Second line");

  std::string line = buffer.read_line();  // Should trigger EOF after this
  REQUIRE(line.empty());
  REQUIRE(buffer.eof() == true);
}

TEST_CASE("input_buffer handles input with no newlines")
{
  std::istringstream input("Single line without newline");
  input_buffer buffer(input);

  REQUIRE(buffer.read_line() == "Single line without newline");
  REQUIRE(buffer.eof() == true);
}

TEST_CASE("input_buffer handles empty input")
{
  std::istringstream input("");
  input_buffer buffer(input);

  REQUIRE(buffer.read_line().empty());
  REQUIRE(buffer.eof() == true);
}

TEST_CASE("input_buffer handles consecutive newlines")
{
  std::istringstream input("Line 1\n\nLine 3\n");
  input_buffer buffer(input);

  REQUIRE(buffer.read_line() == "Line 1");
  REQUIRE(buffer.read_line().empty());  // Empty line
  REQUIRE(buffer.read_line() == "Line 3");
  REQUIRE(buffer.read_line().empty());  // Empty line
  REQUIRE(buffer.eof() == true);
}

TEST_CASE("input_buffer handles a newline-only input")
{
  std::istringstream input("\n");
  input_buffer buffer(input);

  REQUIRE(buffer.read_line().empty());  // Empty line before newline
  REQUIRE(buffer.read_line().empty());  // Empty line before newline
  REQUIRE(buffer.eof() == true);
}

TEST_CASE("input_buffer clears buffer correctly")
{
  std::istringstream input("Some input line\nAnother line\n");
  input_buffer buffer(input);

  REQUIRE(buffer.read_line() == "Some input line");
  REQUIRE(buffer.get_buffer() == "Some input line");

  buffer.clear_buffer();
  REQUIRE(buffer.get_buffer().empty());

  REQUIRE(buffer.read_line() == "Another line");
  REQUIRE(buffer.get_buffer() == "Another line");

  REQUIRE(buffer.read_line().empty());
  REQUIRE(buffer.eof() == true);
}

TEST_CASE("input_buffer reads multiple data types correctly")
{
  std::istringstream input("256\n1.618\nC\nfalse\nOpenAI\n");
  input_buffer buffer(input);

  // Since input_buffer reads lines as strings, further parsing would be needed.
  // Here, we test the reading mechanism.
  REQUIRE(buffer.read_line() == "256");
  REQUIRE(buffer.read_line() == "1.618");
  REQUIRE(buffer.read_line() == "C");
  REQUIRE(buffer.read_line() == "false");
  REQUIRE(buffer.read_line() == "OpenAI");

  REQUIRE(buffer.read_line().empty());
  REQUIRE(buffer.eof() == true);
}
