#include "parser.hpp"
#include <stdexcept>

auto parser::consume(token_type type, const std::string& value) -> token {
    if (m_tokens[m_pos].type != type || (!value.empty() && m_tokens[m_pos].value != value)) {
        throw std::runtime_error("Unexpected token: " + m_tokens[m_pos].value);
    }
    return m_tokens[m_pos++];
}

auto parser::peek() const -> token {
    return m_tokens[m_pos];
}

select_statement parser::parse_select() {
    consume(token_type::keyword, "SELECT");

    select_statement select_stmt;

    // Parse column list
    do {
        select_stmt.columns.push_back(consume(token_type::identifier).value);
        if (peek().type == token_type::punctuation && peek().value == ",") {
            consume(token_type::punctuation, ",");
        } else {
            break;
        }
    } while (true);

    consume(token_type::keyword, "FROM");
    select_stmt.table = consume(token_type::identifier).value;

    // Optional WHERE clause
    if (peek().type == token_type::keyword && peek().value == "WHERE") {
        consume(token_type::keyword, "WHERE");
        select_stmt.where_clause = parse_expression();
    }

    return select_stmt;
}

expression parser::parse_expression() {
    expression expr;
    expr.column = consume(token_type::identifier).value;
    expr.op = consume(token_type::operator_).value;
    expr.value = consume(token_type::literal).value;
    return expr;
}

insert_statement parser::parse_insert() {
/* INSERT INTO <table> (column1, column2, ...) VALUE (value1, value2, ...)
*/
    consume(token_type::keyword, "INSERT");
    consume(token_type::keyword, "INTO");

    insert_statement insert_stmt;
    insert_stmt.table = consume(token_type::identifier).value;

    consume(token_type::punctuation, "(");
    do {
        insert_stmt.columns.push_back(consume(token_type::identifier).value);
        if (peek().type == token_type::punctuation && peek().value == ",") {
            consume(token_type::punctuation, ",");
        } else {
            break;
        }
    } while (true);
    consume(token_type::punctuation, ")");

    consume(token_type::keyword, "VALUES");
    
    consume(token_type::punctuation, "(");
    do {
        insert_stmt.values.push_back(consume(token_type::literal).value);
        if (peek().type == token_type::punctuation && peek().value == ",") {
            consume(token_type::punctuation, ",");
        } else {
            break;
        }
    } while (true);
    consume(token_type::punctuation, ")");

    return insert_stmt;
}

update_statement parser::parse_update() {
    consume(token_type::keyword, "UPDATE");

    update_statement update_stmt;
    update_stmt.table = consume(token_type::identifier).value;

    consume(token_type::keyword, "SET");
    do {
        std::string column = consume(token_type::identifier).value;
        consume(token_type::operator_, "=");
        std::string value = consume(token_type::literal).value;
        update_stmt.assignments.emplace_back(column, value);

        if (peek().type == token_type::punctuation && peek().value == ",") {
            consume(token_type::punctuation, ",");
        } else {
            break;
        }
    } while (true);

    if (peek().type == token_type::keyword && peek().value == "WHERE") {
        consume(token_type::keyword, "WHERE");
        update_stmt.where_clause = parse_expression();
    }

    return update_stmt;
}

delete_statement parser::parse_delete() {
    consume(token_type::keyword, "DELETE");
    consume(token_type::keyword, "FROM");

    delete_statement delete_stmt;
    delete_stmt.table = consume(token_type::identifier).value;

    if (peek().type == token_type::keyword && peek().value == "WHERE") {
        consume(token_type::keyword, "WHERE");
        delete_stmt.where_clause = parse_expression();
    }

    return delete_stmt;
}

// Pensar de trocar essa gambiarra para uma struct de "resposta"
// Vide: https://github.com/jaypipes/sqltoast
void parser::parse_statement() {
    token first_token = peek();
    if (first_token.type == token_type::keyword) {
        if (first_token.value == "SELECT") {
        } else if (first_token.value == "INSERT") {
        } else if (first_token.value == "UPDATE") {
        } else if (first_token.value == "DELETE") {
        }
    }
    throw std::runtime_error("Unknown SQL statement: " + first_token.value);
}
