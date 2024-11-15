#include "frontend/tokenizer/token.hpp"
#include <memory>
#include "def.hpp"

class parser {
public:
    explicit parser(const std::vector<token>& tokens) : m_tokens(tokens), m_pos(0) {}

    auto parse_select() -> select_statement;
    auto parse_insert() -> insert_statement;
    auto parse_update() -> update_statement;
    auto parse_delete() -> delete_statement;
    auto parse_statement() -> void;

private:
    const std::vector<token>& m_tokens;
    size_t m_pos = 0;

    auto consume(token_type type, const std::string& value = "") -> token;
    auto peek() const -> token;
    auto parse_expression() -> expression;
};

