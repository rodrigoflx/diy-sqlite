#ifndef INPUTBUFFER_HPP
#define INPUTBUFFER_HPP

#include <istream>
#include <string>

class input_buffer
{
  /*
   * Do pay attention that the EOF is only reached AFTER
   * read_line() is invoked when the buffer is empty
   */
public:
  /**
   * @brief Constructs an input_buffer with the given input stream.
   *
   * @param input_stream A reference to an input stream (e.g., std::cin,
   * std::ifstream).
   */
  explicit input_buffer(std::istream& input_stream);

  /**
   * @brief Reads the next line from the input stream.
   *
   * @return auto A string representing the next line. Returns an empty string
   * if EOF is reached.
   */
  auto read_line() -> std::string;

  /**
   * @brief Checks if the end of the input stream has been reached.
   *
   * @return auto Boolean indicating EOF status.
   */
  auto eof() const -> bool;

  /**
   * @brief Retrieves the last line read from the input stream.
   *
   * @return auto The last read line as a string.
   */
  auto get_buffer() const -> std::string;

  /**
   * @brief Clears the internal buffer.
   */
  void clear_buffer();

private:
  std::istream& m_input;  // Reference to the input stream
  std::string m_buffer;  // Stores the last read line
};

#endif  // INPUTBUFFER_HPP
