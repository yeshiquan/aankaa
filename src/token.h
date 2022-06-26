
#pragma once

#include <string>
#include <memory.h>

namespace aankaa {

enum TokenType {
    // Single-character tokens.
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR, QUESTION, COLON,

    // One or two character tokens.
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,
    AND_AND, OR_OR,

    // Literals.
    IDENTIFIER, STRING, NUMBER,

    // Keywords.
    AND, CLASS, ELSE, FALSE, FUN, FOR, IF, NIL, OR,
    PRINT, RETURN, SUPER, THIS, TRUE, VAR, WHILE,

    ERROR, EEOF
};

// struct Token {
//     TokenType type;
//     std::string lexeme;
//     std::any literal;
//     int line = 0;
// };

struct Token {
    TokenType type;
    const char* start;
    int length;
    int line;

    bool identifiers_equal(const Token& rhs) {
        if (type != IDENTIFIER || rhs.type != IDENTIFIER) {
            return false;
        }
        if (length != rhs.length) {
            return false;
        }
        return memcmp(start, rhs.start, length) == 0;
    }

    std::string to_string() const {
        std::string tmp;
        tmp.assign(start, length);
        return tmp;
    }
};

} //namespace