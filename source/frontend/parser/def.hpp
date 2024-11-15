#include <string>
#include <vector>
#include <memory>
#include <optional>

struct expression {
    std::string column;
    std::string op;
    std::string value;
};

struct select_statement {
    std::vector<std::string> columns {};
    std::string table {};
    std::optional<expression> where_clause {};
};

struct insert_statement {
    std::string table {};
    std::vector<std::string> columns {};
    std::vector<std::string> values {};
};

struct update_statement {
    std::string table {};
    std::vector<std::pair<std::string, std::string>> assignments {};
    std::optional<expression> where_clause {};
};

struct delete_statement {
    std::string table {};
    std::optional<expression> where_clause {};
};