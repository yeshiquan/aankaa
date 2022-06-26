#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include "token.h"

namespace aankaa {

class Scanner {
public:
    Scanner();
    void reset(const std::string& source);
    char advance();
    std::vector<Token>& scan();
    Token scan_token();
    Token make_token(TokenType type);
    void skip_whitespace();

    TokenType check_keyword(int offset, int length, const char* rest, TokenType type);
    TokenType identifier_type();
    Token identifier();
    bool is_at_end();
    bool match_next(char expected);
    char peek();
    char peek_next();
    void comment();
    Token string();
    Token number();
    bool is_alpha(char ch);
    bool is_digit(char ch);
    void error(const std::string msg);
    Token error_token(const char* message);
private:
    std::string _source;
    const char* _current = nullptr;
    const char* _start = nullptr;
    int _line = 0;
    std::vector<Token> _tokens; 
};

} // namespace