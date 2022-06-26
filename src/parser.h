
#pragma once

#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include "value.h"
#include "object.h"
#include "token.h"
#include "scanner.h"
#include "chunk.h"
#include "pool.h"

namespace aankaa {

constexpr int UNINITIALIZED_FLAG = -1;
static const char* EMPTY_NAME = "";

struct Local {
    Token name;
    int depth = 0;
};

enum FunctionType {
    TYPE_FUNCTION, // 函数内的body
    TYPE_SCRIPT // 主body
};

struct Compiler {
    struct Compiler* enclosing = nullptr;
    // 函数是一等公民，Compiler存在的目的就是编译出function对象
    // 一旦function对象编译成功，compiler就没有作用了。
    // 到了runtime阶段，只需要function的chunk就可以执行程序了
    ObjFunction* function = nullptr;
    FunctionType type = TYPE_SCRIPT;
    Local locals[UINT8_MAX];
    int local_count = 0;
    int current_depth = 0;

    Compiler(Compiler* enclosing_, FunctionType type_) 
                    : type(type_)
                    , enclosing(enclosing_) {
        // 关键点：函数开始之前在locals创建一个占位符，表示function自身
        // 有了这个占位符，编译期和运行时就可以完美对齐了，
        // 例如 add(a,b) 都是，| add |  a  |  b  | 这种形式
        // a和b在local的idx为1和2
        Local& local = locals[local_count++];
        local.name.start = EMPTY_NAME;
        local.name.length  = 0;
    }

    void print() {
        std::cout << "locals -> ";
        for (int i = 0; i < local_count; ++i) {
            std::cout << locals[i].name.to_string() << ":" << current_depth << ",";
        }
        std::cout << std::endl;
    }

    void add_local(const Token& name) {
        Local& local = locals[local_count++];
        local.name = name;
        // -1 表示这个local变量没有完成初始化，不能使用
        local.depth = UNINITIALIZED_FLAG;
    }

    Local& top_local() {
        return locals[local_count - 1];
    }

    Local& get_local(int i) {
        return locals[i];
    }    

    bool is_local_full() { return local_count == UINT8_MAX; }

    bool is_local_exist(const Token& name) {
        for (int i = local_count - 1; i >= 0; --i) {
            Local& local = locals[i];
            if (local.depth < current_depth) {
                break;
            }
            if (local.name.identifiers_equal(name)) {
                return true;
            }
        }
        return false;
    }

    // 最近的local变量是否因为退出作用域导致过期失效了
    bool is_top_local_expired() {
        return locals[local_count-1].depth > current_depth;
    }

    void pop_local() {
        local_count--;
    }

    int find_local(const Token& name) {
        for (int i = local_count - 1; i >= 0; --i) {
            Local& local = locals[i];
            if (local.name.identifiers_equal(name)) {
                return i;
            }
        }
        return -1;
    }
};    

// 优先级越来越高
enum Precedence {
    PREC_NONE,
    PREC_ASSIGNMENT, // =
    PREC_OR,         // ||
    PREC_AND,        // &&
    PREC_EQUALITY,   // == !=
    PREC_COMPARISON, // > < <= >=
    PREC_TERM,       // + -
    PREC_FACTOR,     // * /
    PREC_UNARY,      // ! -
    PREC_CALL,       // . ()
    PREC_PRIMARY
};

struct ParseRule;

class Parser {
public:
    Parser(Scanner* scanner_);
    ~Parser();
    void init_rules();

    void error_at(Token* token, const char* message);
    void error_at_current(const char* message);
    void error(const char* message);
    void must_and_consume(TokenType type, const char* message);
    void advance();
    bool check(TokenType type);
    bool match(TokenType type);

    ObjFunction* compile();

    void number(bool can_assign);
    void grouping(bool can_assign);
    void expression();
    void unary(bool can_assign);
    void binary(bool can_assign);
    void string(bool can_assign);
    void and_(bool can_assign);
    void or_(bool can_assign);

    void statement();
    void begin_scope();
    void block();
    void end_scope();
    void declaration();
    void function(FunctionType type);
    void print_statement();
    void expression_statement();
    void var_declaration();
    void fun_declaration();
    void call(bool can_assign);
    uint8_t argument_list();
    void if_statement();
    void while_statement();
    void return_statement();
    int emit_jump(uint8_t op);
    void patch_jump(int offset);
    void emit_loop(int loop_start);
    void for_statement();

    ObjFunction* end_compiler();


    uint8_t parse_variable_name(const char* error_message);
    void declare_local_variable();
    void define_global_variable(uint8_t global);
    void mark_initialized();
    uint8_t identifier_constant(const Token& name);
    void named_variable(const Token& name, bool can_assign);
    void variable(bool can_assign);

    Chunk& current_chunk();
    void emit_byte(uint8_t byte);
    void emit_byte(uint8_t byte1, uint8_t byte2);

    void emit_return();
    void emit_constant(Value value);
    uint8_t make_constant(Value value);

    void parse_expr(Precedence ctx_precedence);
    ParseRule* get_rule(TokenType type);

public:
    Token current;
    Token previous;
    bool had_error = false;
    Scanner* scanner = nullptr;
    ObjectPool<ObjString> string_pool;
    ObjectPool<ObjFunction> fun_pool;
    Compiler* compiler = nullptr;
};

typedef void (Parser::*ParseFn)(bool can_assign);

struct ParseRule {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
};



} //namespace

