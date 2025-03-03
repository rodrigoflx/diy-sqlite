#include <iostream>
#include <string>

#include <fmt/core.h>

#include "input_buffer.hpp"

auto main() -> int
{
  auto buffer = input_buffer(std::cin);

  /*TODO Separate user input as library instead of writing raw code in main*/

  while (true) {
    fmt::print("db > ");
    std::string line = buffer.read_line();

    if (line == ".exit") {
      break;
    }

    fmt::print("Unrecognized command '{}'.\n", line);
  }
}
