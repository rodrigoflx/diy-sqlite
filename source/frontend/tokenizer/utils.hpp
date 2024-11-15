#include <unordered_set>
#include <cctype>
#include <string>

// Keywords and operators
const std::unordered_set<std::string> keywords = {
    "SELECT", "FROM", "WHERE", "INSERT", "UPDATE", "DELETE", "INTO", "SET", "VALUES"
};

const std::unordered_set<char> operators = { '=', '<', '>', '!', '+' };
const std::unordered_set<char> punctuation = { ',', ';', '(', ')' };

// Helper function to check if a word is a keyword
bool iskeyword(const std::string& word) {
    return keywords.find(word) != keywords.end();
}

// Helper function to check if a character is an operator
bool isoperator(char ch) {
    return operators.find(ch) != operators.end();
}

// Helper function to check if a character is punctuation
bool ispunctuation(char ch) {
    return punctuation.find(ch) != punctuation.end();
}
