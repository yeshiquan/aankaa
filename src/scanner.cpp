#include "scanner.h"
#include <iostream>
#include <memory.h>

namespace aankaa {

Scanner::Scanner() {
}

void Scanner::reset(const std::string& source) {
    _source = source;
    _start = nullptr;
    _current = _source.c_str();
    _line = 0;
    _tokens.clear();
}

char Scanner::advance() {
    return *_current++;
}

std::vector<Token>& Scanner::scan() {
    while (!is_at_end()) {
        scan_token();
    }
    return _tokens;
}


// X X X X X X X X X X X X _
// |           |           |
// start     offset      current
TokenType Scanner::check_keyword(int offset, int length, const char* rest, TokenType type) {
    if ((_current - _start == offset + length) && memcmp(_start + offset, rest, length) == 0) {
        return type;
    }
    return IDENTIFIER;
}

TokenType Scanner::identifier_type() {
    switch (_start[0]) {
        case 'a': 
            return check_keyword(1, 2, "nd", AND);
        case 'c' :
            return check_keyword(1, 4, "lass", CLASS);
        case 'e': return check_keyword(1, 3, "lse", ELSE);
        case 'f':
        if (_current - _start > 1) {
            switch (_start[1]) {
            case 'a': return check_keyword(2, 3, "lse", FALSE);
            case 'o': return check_keyword(2, 1, "r", FOR);
            case 'u': return check_keyword(2, 1, "n", FUN);
            }
        }
        break;
        case 'i': return check_keyword(1, 1, "f", IF);
        case 'n': return check_keyword(1, 2, "il", NIL);
        case 'o': return check_keyword(1, 1, "r", OR);
        case 'p': return check_keyword(1, 4, "rint", PRINT);
        case 'r': return check_keyword(1, 5, "eturn", RETURN);
        case 's': return check_keyword(1, 4, "uper", SUPER);
        case 't':
        if (_current - _start > 1) {
            switch (_start[1]) {
            case 'h': return check_keyword(2, 2, "is", THIS);
            case 'r': return check_keyword(2, 2, "ue", TRUE);
            }
        }
        break;
        case 'v': return check_keyword(1, 2, "ar", VAR);
        case 'w': return check_keyword(1, 4, "hile", WHILE);
    }
    return IDENTIFIER;
}

Token Scanner::identifier() {
    while (is_alpha(peek()) || is_digit(peek())) {
        advance();
    }
    TokenType t = identifier_type();
    return make_token(t);
}

Token Scanner::scan_token() {
    skip_whitespace();
    _start = _current;
    if (is_at_end()) {
        return make_token(EEOF);
    }
    char ch = advance();
    switch(ch) {
    case '(': 
        return make_token(LEFT_PAREN); 
    case ')': 
        return make_token(RIGHT_PAREN); 
    case '{': 
        return make_token(LEFT_BRACE); 
    case '}': 
        return make_token(RIGHT_BRACE); 
    case ',': 
        return make_token(COMMA); 
    case '.': 
        return make_token(DOT); 
    case '-': 
        return make_token(MINUS); 
    case '+': 
        return make_token(PLUS); 
    case ';': 
        return make_token(SEMICOLON); 
    case '*': 
        return make_token(STAR); 
    case '/':
        return make_token(SLASH);        
    case '?': 
        return make_token(QUESTION); 
        break;
    case ':': 
        return make_token(COLON); 
    case '!':
        return make_token(match_next('=') ? BANG_EQUAL : BANG); 
    case '=':
        return make_token(match_next('=') ? EQUAL_EQUAL : EQUAL); 
    case '<':
        return make_token(match_next('=') ? LESS_EQUAL : LESS); 
    case '>':
        return make_token(match_next('=') ? GREATER_EQUAL : GREATER); 
    case '&':
        return make_token(match_next('&') ? AND_AND : AND); 
    case '|':
        return make_token(match_next('|') ? OR_OR : OR); 
    case '"':
        return string(); 
    default: break;
    }

    if (is_digit(ch)) {
        return number();
    } else if (is_alpha(ch)) {
        return identifier();
    }

    return error_token("Unexpected character.");
}

void Scanner::skip_whitespace() {
    for (;;) {
        char c = peek();
        switch(c) {
        case ' ':
        case '\r':
        case '\t':
            advance();
            break;
        case '\n':
            _line++;
            advance();
            break;
        case '/':
            if (peek_next() == '/') {
                while (peek() != '\n' && !is_at_end()) {
                    advance();
                }
            } else {
                return;
            }
            break;
        default:
            return;
        }
    }
}

void Scanner::comment() {
    while (!is_at_end() && peek() != '\n') {
        advance();
    }
}
//  current(执行前)
//     |
// _ " X X X X X X X X " _
//   |                   |
//  start             current(执行后)
Token Scanner::string() {
    while (!is_at_end() && peek() != '"') {
        char ch = advance();
        if (ch == '\n') {
            _line++;
        }
    }
    if (is_at_end()) {
        return error_token("expected quote");
    }
    advance();
    return make_token(STRING);
}

Token Scanner::error_token(const char* message) {
    Token token;
    token.type = ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = _line;
    return token;
}

//  current(执行前)
//     |
// _ X X X X X X X X X X _
//   |                   |
//  start             current(执行后)
Token Scanner::number() {
    bool is_int = true;
    while (is_digit(peek())) {
        advance();
    }
    if (peek() == '.' && is_digit(peek_next())) {
        is_int = false;
        advance();
        while (is_digit(peek())) {
            advance();
        } 
    }
    return make_token(NUMBER);
}

bool Scanner::is_alpha(char ch) {
    return (ch >= 'a' && ch <= 'z') ||
            (ch >= 'A' && ch <= 'Z') ||
            ch == '_'; 
}

bool Scanner::is_digit(char ch) {
    return ch >= '0' && ch <= '9';
}

void Scanner::error(const std::string msg) {
    std::cout << "line:" << _line << " error:" << msg << std::endl;;
}

Token Scanner::make_token(TokenType type) {
    Token token = {
        .type = type,
        .start = _start,
        .length = _current - _start,
        .line = _line
    };    
    return token;
}

bool Scanner::is_at_end() {
    return *_current == '\0';
}

// 如果下一个字符match，吃掉它，同时返回true
// 不match就不要吃掉，返回false
bool Scanner::match_next(char expected) {
    bool is_match = false;
    if (is_at_end()) {
        return false;
    }
    if (*_current == expected) {
        is_match = true;
        _current++;
    } else {
        is_match = false;
    }
    return is_match;
}

char Scanner::peek() {
    if (is_at_end()) {
        return '\0';
    }
    return *_current;
}

char Scanner::peek_next() {
    if (is_at_end()) {
        return '\0';
    }
    return *(_current+1);
}



}; // namespace