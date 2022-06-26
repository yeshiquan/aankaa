#include "parser.h"
#include <iostream>
#include <memory.h>
#include "defer.h"

namespace aankaa {

std::vector<ParseRule> rules(static_cast<size_t>(EEOF) + 1);

void Parser::init_rules() {
    rules[LEFT_PAREN]    = {&Parser::grouping, &Parser::call,   PREC_CALL};
    rules[RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE};
    rules[LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE}; 
    rules[RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE};
    rules[COMMA]         = {NULL,     NULL,   PREC_NONE};
    rules[DOT]           = {NULL,     NULL,   PREC_NONE};

    rules[MINUS]         = {&Parser::unary,    &Parser::binary, PREC_TERM};
    rules[PLUS]          = {NULL,     &Parser::binary, PREC_TERM};
    rules[SLASH]         = {NULL,     &Parser::binary, PREC_FACTOR};
    rules[STAR]          = {NULL,     &Parser::binary, PREC_FACTOR};

    rules[SEMICOLON]     = {NULL,     NULL,   PREC_NONE};
    rules[BANG]          = {NULL,     NULL,   PREC_NONE};
    rules[EQUAL]         = {NULL,     NULL,   PREC_NONE};

    rules[BANG_EQUAL]    = {NULL,     &Parser::binary,   PREC_EQUALITY};
    rules[EQUAL_EQUAL]   = {NULL,     &Parser::binary,   PREC_EQUALITY};
    rules[GREATER]       = {NULL,     &Parser::binary,   PREC_COMPARISON};
    rules[GREATER_EQUAL] = {NULL,     &Parser::binary,   PREC_COMPARISON};
    rules[LESS]          = {NULL,     &Parser::binary,   PREC_COMPARISON};
    rules[LESS_EQUAL]    = {NULL,     &Parser::binary,   PREC_COMPARISON};

    rules[IDENTIFIER]    = {&Parser::variable,     NULL,   PREC_NONE};
    rules[STRING]        = {&Parser::string,     NULL,   PREC_NONE};
    rules[NUMBER]        = {&Parser::number,   NULL,   PREC_NONE};

    rules[AND]           = {NULL,     &Parser::and_,   PREC_AND};
    rules[OR]            = {NULL,     &Parser::or_,   PREC_OR};

    rules[CLASS]         = {NULL,     NULL,   PREC_NONE};
    rules[ELSE]          = {NULL,     NULL,   PREC_NONE};
    rules[FALSE]         = {NULL,     NULL,   PREC_NONE};
    rules[FOR]           = {NULL,     NULL,   PREC_NONE};
    rules[FUN]           = {NULL,     NULL,   PREC_NONE};
    rules[IF]            = {NULL,     NULL,   PREC_NONE};
    rules[NIL]           = {NULL,     NULL,   PREC_NONE};
    rules[PRINT]         = {NULL,     NULL,   PREC_NONE};
    rules[RETURN]        = {NULL,     NULL,   PREC_NONE};
    rules[SUPER]         = {NULL,     NULL,   PREC_NONE};
    rules[THIS]          = {NULL,     NULL,   PREC_NONE};
    rules[TRUE]          = {NULL,     NULL,   PREC_NONE};
    rules[VAR]           = {NULL,     NULL,   PREC_NONE};
    rules[WHILE]         = {NULL,     NULL,   PREC_NONE};
    rules[ERROR]         = {NULL,     NULL,   PREC_NONE};
    rules[EEOF]           = {NULL,     NULL,   PREC_NONE};
}

Parser::Parser(Scanner* scanner_) : scanner(scanner_) {
    init_rules();
    // 创建1个compiler用来编译main函数
    compiler = new Compiler(nullptr, TYPE_SCRIPT);
    compiler->function = fun_pool.get();
    // main函数变成一个名字是空的字符串，这样就无法通过变量名来访问main函数了
    compiler->function->name = string_pool.get(EMPTY_NAME); 
}

Parser::~Parser() {
    delete compiler;
}

void Parser::error_at(Token* token, const char* message) {
    fprintf(stderr, "line %d Error", token->line);
    if (token->type == EEOF) {
        fprintf(stderr, " at end");
    } else if (token->type == ERROR) {
        // nothing
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    had_error = true;
}

void Parser::error_at_current(const char* message) {
    error_at(&current, message);
}

void Parser::error(const char* message) {
    error_at(&previous, message);
}

void Parser::must_and_consume(TokenType type, const char* message) {
    if (current.type == type) {
        advance();
    } else {
        error_at_current(message);
    }
}

void Parser::advance() {
    previous = current;
    for (;;) {
        current = scanner->scan_token();
        if (current.type != ERROR) {
            break;
        }
        error_at_current(current.start);
    }
}

bool Parser::check(TokenType type) {
    return current.type == type;
}

bool Parser::match(TokenType type) {
    if (!check(type)) {
        return false;
    }
    advance();
    return true;
}

Chunk& Parser::current_chunk() {
    return *compiler->function->chunk;
}   

void Parser::emit_byte(uint8_t byte) {
    //std::cout << "chunk:" << &current_chunk() << " chunk_code_size:" << current_chunk().code.size();
    //printf(" emit_byte() --> %hhu\n", byte);
    current_chunk().write(byte, previous.line);
}

void Parser::emit_byte(uint8_t byte1, uint8_t byte2) {
    emit_byte(byte1);
    emit_byte(byte2);
}

void Parser::emit_return() {
    emit_byte(OP_NIL);
    emit_byte(OP_RETURN);
}

void Parser::emit_constant(Value value) {
    emit_byte(OP_CONSTANT, make_constant(value));
}

// 返回常量所在的下标
uint8_t Parser::make_constant(Value value) {
    //std::cout << ">>>>>> make_constant " << value.to_string() << " to chunk:" << &current_chunk() << std::endl;
    int constant_idx = current_chunk().add_constant(value);
    if (constant_idx > UINT8_MAX) {
        fprintf(stderr, "Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t)constant_idx;
}

ParseRule* Parser::get_rule(TokenType type) {
    return &rules[type];
}

// 从剩下的token中找到所有优先级比ctx_precedence更高的子表达式
// 1+    2*3*4-5
//       |
//      current
void Parser::parse_expr(Precedence ctx_precedence) {
    const Token prefix_token = current;
    advance();
    std::string token_str;
    token_str.assign(prefix_token.start, prefix_token.length);
    //std::cout << "parse_expr() prefix_token:[" << token_str << "] token_type:"  << prefix_token.type << std::endl;
    ParseRule* rule = get_rule(prefix_token.type);
    ParseFn prefix_fn = rule->prefix;
    if (prefix_fn == nullptr) {
        error("Expect expression");
        return;
    }

    bool can_assign = (ctx_precedence <= PREC_ASSIGNMENT);
    (this->*prefix_fn)(can_assign);

    //    *3*4-5
    //    |
    //  current
    while (true) {
        const Token& infix_token = current;
        ParseRule* rule = get_rule(infix_token.type);
        if (rule->precedence <= ctx_precedence) {
            //std::cout << "while stop token:" << infix_token.type << " precedence:" << rule->precedence << " ctx:" << ctx_precedence << std::endl;
            break;
        }
        ParseFn infix_fn = rule->infix;
        advance();
        (this->*infix_fn)(can_assign);
    }    

    if (can_assign && match(EQUAL)) {
        error("Invalid assignment target.");
    }
}


// We assume the token for the number literal has already been consumed and is stored in previous. 
void Parser::number(bool can_assign) {
    double value = strtod(previous.start, NULL);
    emit_constant(value);
    std::cout << "number() -> " << value << std::endl;
}

void Parser::grouping(bool can_assign) {
    expression();
    must_and_consume(RIGHT_PAREN, "Expect ')' after expression");
}

void Parser::expression() {
    std::cout << "expression()" << std::endl;
    // a * b = 3 + 4
    // 要求返回优先级大于'='的子表达式
    // 所以解析结果为 a * b
    parse_expr(PREC_ASSIGNMENT);
}

void Parser::unary(bool can_assign) {
    std::cout << "unary()" << std::endl;

    TokenType operator_type = previous.type;

    parse_expr(PREC_UNARY);

    switch(operator_type) {
    case MINUS: 
        emit_byte(OP_NEGATE);
        break;
    default:
        return;
    }
}

// 3    +        4
// |    |        |
//    previous current
void Parser::binary(bool can_assign) {
    TokenType operator_type = previous.type;
    ParseRule* rule = get_rule(operator_type);

    auto type_to_str = [](TokenType type) -> std::string {
        if (type == PLUS) return "+";
        if (type == MINUS) return "-";
        if (type == STAR) return "*";
        if (type == SLASH) return "/";

        if (type == BANG_EQUAL) return "!=";
        if (type == EQUAL_EQUAL) return "==";
        if (type == GREATER) return ">";
        if (type == GREATER_EQUAL) return ">=";
        if (type == LESS) return "<";
        if (type == LESS_EQUAL) return "<=";
        return "?";
    };

    std::cout << "binary() operator:'" << type_to_str(operator_type) << "'" << std::endl;

    parse_expr(rule->precedence);

    switch(operator_type) {
    case MINUS:         emit_byte(OP_SUBTRACT); break;
    case PLUS:          emit_byte(OP_ADD); break;
    case STAR:          emit_byte(OP_MULTIPLY); break;
    case SLASH:         emit_byte(OP_DIVIDE); break;

    case BANG_EQUAL:    emit_byte(OP_EQUAL, OP_NOT); break;
    case EQUAL_EQUAL:   emit_byte(OP_EQUAL); break;
    case GREATER:       emit_byte(OP_GREATER); break;
    case GREATER_EQUAL: emit_byte(OP_LESS, OP_NOT); break;
    case LESS:          emit_byte(OP_LESS); break;
    case LESS_EQUAL:    emit_byte(OP_GREATER, OP_NOT); break;    
    default:
        return;
    }
}

ObjFunction* Parser::compile() {
    had_error = false;
    while (!match(EEOF)) {
        declaration();
    }

    // main函数
    ObjFunction* function = end_compiler();

    return had_error ? nullptr : function;
}


void Parser::statement() {
    if (match(PRINT)) {
        print_statement();
    } else if (match(LEFT_BRACE)) {
        begin_scope();
        block();
        end_scope();
    } else if (match(IF)) {
        if_statement();
    } else if (match(WHILE)) {
        while_statement();
    } else if (match(FOR)) {
        for_statement(); 
    } else if (match(RETURN)) {
        return_statement();
    } else {
        expression_statement();
    } 
}

void Parser::begin_scope() {
    compiler->current_depth++;
}

void Parser::block() {
    while (!check(RIGHT_BRACE) && !check(EEOF)) {
        declaration();
    }

    must_and_consume(RIGHT_BRACE, "Expect '}' after block.");
}

void Parser::end_scope() {
    std::cout << "end_scope()" << std::endl;
    compiler->current_depth--;

    while (compiler->local_count > 0 && compiler->is_top_local_expired()) {
        emit_byte(OP_POP);
        compiler->pop_local();
    }
}

void Parser::declaration() {
    if (match(FUN)) {
        fun_declaration();
    } else if (match(VAR)) {
        var_declaration();
    } else {
        statement();
    }
}

void Parser::fun_declaration() {
    std::cout << "fun_declaration()" << std::endl;
    uint8_t fun_idx = parse_variable_name("Expect function name.");

    // 函数体内可以自己调用自己，只要解析完函数名，就把函数变量标记为已经初始化，这样在含树体内可以自己调用自己，实现递归。
    mark_initialized();

    function(TYPE_FUNCTION);
    define_global_variable(fun_idx);
}

void Parser::function(FunctionType type) {
    std::cout << "\n---- function() start" << std::endl;

    Compiler *new_compiler = new Compiler(compiler, type);
    DEFER({
        delete new_compiler;
    });
    
    ObjFunction* fun = fun_pool.get();
    if (type != TYPE_SCRIPT) {
        fun->name = string_pool.get(previous.start, previous.length);
    }
    new_compiler->function = fun;

    // 设置新的compiler
    compiler = new_compiler;
    std::cout << "function() change compiler [" << compiler << " -> " << new_compiler << "]" 
                << " new_compiler_depth:" << new_compiler->current_depth << std::endl;

    begin_scope();

    must_and_consume(LEFT_PAREN, "Expect '(' after function name.");

    if (!check(RIGHT_PAREN)) {
        // 编译函数形参
        do {
            compiler->function->arity++;
            if (compiler->function->arity > 255) {
                error_at_current("Can't have more than 255 parameters");
            }
            uint8_t cons_idx = parse_variable_name("Expect parameter name");
            // 形参不需要初始化，所以解析完变量名，就可以标记为初始化完成。
            mark_initialized();
        } while (match(COMMA));
    }

    must_and_consume(RIGHT_PAREN, "Expect ')' after parameters.");
    must_and_consume(LEFT_BRACE, "Expect '{' before function body");

    // 编译函数body
    block();

    ObjFunction* function = end_compiler();

    emit_byte(OP_CONSTANT, make_constant(Value(function)));
    std::cout << "---- function() finish\n" << std::endl;
}

void Parser::call(bool can_assign) {
    uint8_t arg_count = argument_list();
    emit_byte(OP_CALL, arg_count);
}

uint8_t Parser::argument_list() {
    uint8_t arg_count = 0;
    if (!check(RIGHT_PAREN)) {
        do {
            expression();
            arg_count++;
            if (arg_count == 255) {
                error("Can't have more than 255 arguments.");
            }
        } while (match(COMMA));
    }

    must_and_consume(RIGHT_PAREN, "Expect ')' after arguments");

    return arg_count;
}

// var a = 3 * 4;
void Parser::var_declaration() {
    std::cout << "var_declaration()" << std::endl;
    uint8_t var_name_idx = parse_variable_name("Expect variable name.");
    
    if (match(EQUAL)) {
        // var a = 3*2+1;
        std::cout << "var_declaration() with initializer expression" << std::endl;
        expression();
    } else {
        // var a;
        emit_byte(OP_NIL);
    }
    must_and_consume(SEMICOLON, "Expect ';' after variable declaration.");

    if (compiler->current_depth == 0) {
        define_global_variable(var_name_idx);
    }
}

// var a = 5; 如何处理a(a在常量表中的idx已经确定了)
void Parser::define_global_variable(uint8_t var_name_idx) {
    printf("define_global_variable() var_name_idx:%hhu current_depth:%d\n", var_name_idx, compiler->current_depth);
    emit_byte(OP_DEFINE_GLOBAL, var_name_idx);
}

void Parser::mark_initialized() {
    std::cout << "mark_initialized() top_local:[" << compiler->top_local().name.to_string() 
                                << "] depth:" << compiler->current_depth << std::endl;
    if (compiler->current_depth == 0) {
        return;
    }
    // var a = a; 编译之前depth设置为-1，用来表示未初始化；
    // 现在恢复到正常，表示初始化完成
    compiler->top_local().depth = compiler->current_depth;
}

// var a = 5; 如何处理a，需要先把a添加到locals或者constants区域
uint8_t Parser::parse_variable_name(const char* error_message) {
    std::cout << "parse_variable_name() " << current.to_string() << " current_depth:" << compiler->current_depth << std::endl;
    must_and_consume(IDENTIFIER, error_message);

    if (compiler->current_depth > 0) {
        // 局部变量，不用添加到constants表，直接返回0，这个0不会使用
        declare_local_variable();
        return 0;
    }
    // 全局变量，添加到constants表
    return identifier_constant(previous);
}

// var a = 5; 遇到a怎么处理：把a添加到local区域
void Parser::declare_local_variable() {
    const Token& name = previous;
    std::cout << "declare_local_variable() add local " << name.to_string() << std::endl;

    if (compiler->is_local_exist(name)) {
        error("Already a variable with this name in this scope.");
    }

    if (compiler->is_local_full()) {
        error("Too many local variables in block.");
    } else {
        compiler->add_local(name);
    }
}

// 返回变量名在constants里面的下标
uint8_t Parser::identifier_constant(const Token& name) {
    std::cout << "identifier_constant() add global constants" << std::endl;
    ObjString* obj_str = string_pool.get(name.start, name.length);
    Value v(obj_str);
    return make_constant(v);
}

void Parser::print_statement() {
    std::cout << "print_statement()" << std::endl;
    expression();
    must_and_consume(SEMICOLON, "Expect ';' after value.");
    emit_byte(OP_PRINT);
}

void Parser::expression_statement() {
    expression();
    must_and_consume(SEMICOLON, "Expect ';' after expression");
    emit_byte(OP_POP);
}

void Parser::if_statement() {
    must_and_consume(LEFT_PAREN, "Expect '(' after 'if'.");
    expression();
    must_and_consume(RIGHT_PAREN, "Expect ')' after 'condition'.");

    int if_jump_pos = emit_jump(OP_JUMP_IF_FALSE);
    emit_byte(OP_POP);
    statement();

    int else_jump_pos = emit_jump(OP_JUMP);

    patch_jump(if_jump_pos);

    emit_byte(OP_POP);

    if (match(ELSE)) {
        statement();
    }
    patch_jump(else_jump_pos);
}

void Parser::while_statement() {
    int loop_start = current_chunk().count;
    must_and_consume(LEFT_PAREN, "Expect '(' after 'if'.");
    expression();
    must_and_consume(RIGHT_PAREN, "Expect ')' after 'condition'.");

    int exit_jump_pos = emit_jump(OP_JUMP_IF_FALSE);
    emit_byte(OP_POP);
    statement();
    emit_loop(loop_start);

    patch_jump(exit_jump_pos);
    emit_byte(OP_POP);
}

void Parser::return_statement() {
    std::cout << "return_statement() compiler->type:" << compiler->type << std::endl;
    if (compiler->type == TYPE_SCRIPT) {
        error("Can't return from top-level code.");
    }
    if (match(SEMICOLON)) {
        std::cout << "emit return nil........" << std::endl;
        emit_return();
    } else {
        expression();
        must_and_consume(SEMICOLON, "Expect ';' after return value");
        std::cout << "emit return expression........" << std::endl;
        emit_byte(OP_RETURN);
    }
}

void Parser::emit_loop(int loop_start) {
    emit_byte(OP_LOOP);

    int offset = current_chunk().count - loop_start + 2;
    if (offset > UINT16_MAX) {
        error("Loop body too large");
    }
    emit_byte((offset >> 8) & 0xff);
    emit_byte(offset & 0xff);
}

// 返回jump指令的绝对位置, 开始跳转，但是跳转到哪里还不确定
int Parser::emit_jump(uint8_t op) {
    emit_byte(op);
    emit_byte(0xff);
    emit_byte(0xff);
    return current_chunk().count - 3;
}

// 结束跳转，确定跳转的位置。
// 1:
// ......
// k:   emit_jump
// .......
// m:   patch_jump
// m+1: 
// 在k处调用emit_jump，在m处调用patch_jump，会导致k和m之间生成的指令全部被跳过
// 也就是说：从k直接跳转到m+1生成的指令
void Parser::patch_jump(int jump_pos) {
    int offset = current_chunk().count - jump_pos - 3;
    if (offset > UINT64_MAX) {
        error("Too much code to jump over.");
    }
    // 分别存储offset的高8位和低8位
    current_chunk().code[jump_pos+1] = (offset >> 8) & 0xff;
    current_chunk().code[jump_pos+2] = (offset & 0xff);
}

void Parser::for_statement() {
    begin_scope(); // for创建的临时变量是局部的，退出后要销毁
    must_and_consume(LEFT_PAREN, "Expect '(' after 'for'.");
    if (match(SEMICOLON)) {
    } else if (match(VAR)) {
        var_declaration();
    } else {
        expression_statement();
    }

    int loop_start = current_chunk().count;

    int exit_jump_pos = -1;

    if (!match(SEMICOLON)) {
        expression();
        must_and_consume(SEMICOLON, "Expect ';' after loop condition");

        exit_jump_pos = emit_jump(OP_JUMP_IF_FALSE);
        emit_byte(OP_POP);
    }

    if (!match(RIGHT_PAREN)) {
        int body_jump_pos = emit_jump(OP_JUMP);
        int increment_start = current_chunk().count;
        expression();
        emit_byte(OP_POP);
        must_and_consume(RIGHT_PAREN, "Expect ')' after for clauses.");

        emit_loop(loop_start);
        loop_start = increment_start;
        patch_jump(body_jump_pos);
    }

    statement();
    emit_loop(loop_start);

    if (exit_jump_pos != -1) {
        patch_jump(exit_jump_pos);
        emit_byte(OP_POP);
    }

    end_scope();
}

// " h e l l o " _
// |             |
// previous     current
void Parser::string(bool can_assign) {
    ObjString* obj_str = string_pool.get(previous.start + 1, previous.length - 2);
    Value v(obj_str);
    emit_constant(v);
}

void Parser::and_(bool can_assign) {
    std::cout << "logical_and" << std::endl;
    int end_jump_pos = emit_jump(OP_JUMP_IF_FALSE);
    emit_byte(OP_POP);

    parse_expr(PREC_AND);

    patch_jump(end_jump_pos);
}

void Parser::or_(bool can_assign) {
    std::cout << "logical_or" << std::endl;
    int else_jump_pos = emit_jump(OP_JUMP_IF_FALSE);
    int end_jump_pos = emit_jump(OP_JUMP);
    patch_jump(else_jump_pos);
    emit_byte(OP_POP);

    parse_expr(PREC_OR);

    patch_jump(end_jump_pos);
}

//     a         = 3 * 4;
//     |         |
//    previous   current
// 到这里的时候，变量a已经被吃掉了
// 这是Primary表达式的一个分支，例如 a = 3; a * 3; 遇到IDENTIFIER的token如何解析。
// 注意变量声明不会走到这里，例如 var a = 9;
void Parser::variable(bool can_assign) {
    std::cout << "variable()" << std::endl;
    named_variable(previous, can_assign);
}

// var a = 34*2; 注意这是declaration, 不会走到这里。
// a = 3; 这是statement，变量赋值操作, a在前面的语句肯定被添加到constants或者locals了，emit OP_SET_XXX
// a*2+2; 这是statement, 获取变量的值, a在前面的语句肯定被添加到constants或者locals了，emit OP_GET_XXX
void Parser::named_variable(const Token& name, bool can_assign) {
    uint8_t get_op, set_op;
    int var_idx = compiler->find_local(name);
    if (compiler->get_local(var_idx).depth == UNINITIALIZED_FLAG) {
        error("Can't read local variable in its own initializer");
    }
    std::cout << "named_variable() var_idx:" << var_idx << std::endl;
    if (var_idx != -1) {
        // locals找到了，肯定是局部变量, var_idx为locals区域的下标
        get_op = OP_GET_LOCAL;
        set_op = OP_SET_LOCAL;
    } else {
        // locals没找到，当做全局变量处理, var_idx为constants区域的下标
        var_idx = identifier_constant(name);
        get_op = OP_GET_GLOBAL;
        set_op = OP_SET_GLOBAL;
    }

    // a * b = c + d  can_assign = false
    // a = b = c + d  can_assign = true
    if (can_assign && match(EQUAL)) {
        expression();
        emit_byte(set_op, static_cast<uint8_t>(var_idx));
    } else {
        emit_byte(get_op, static_cast<uint8_t>(var_idx));
    }
}

ObjFunction* Parser::end_compiler() {
    emit_return();
    ObjFunction* function = compiler->function;

    std::cout << "end_compiler() enclosing:" << compiler->enclosing << " chunk:" << function->chunk << std::endl;
    std::cout << "----------- print func(" << function->name->to_string() << ") chunk in end_compiler() -> " << std::endl;
    function->chunk->print();

    std::cout << "end_compiler() change compiler [" << compiler << " -> " << compiler->enclosing << "]" << std::endl;
    compiler = compiler->enclosing;

    return function;
}

}; // namespace




