#include "input_buffer.hpp"
#include <stdexcept>

input_buffer::input_buffer(std::istream& input_stream)
    : m_input(input_stream) {}

auto input_buffer::read_line() -> std::string {
    if (eof()) {
        m_buffer.clear();
        return m_buffer;
    }

    if (std::getline(m_input, m_buffer)) {
        return m_buffer;
    }
    // If getline fails (e.g., EOF), clear the buffer
    m_buffer.clear();
    return m_buffer;
}

auto input_buffer::eof() const -> bool {
    return m_input.eof();
}

auto input_buffer::get_buffer() const -> std::string {
    return m_buffer;
}

void input_buffer::clear_buffer() {
    m_buffer.clear();
}

