#include "chunk.h"

namespace aankaa {

const char* op_name[] = {
    [OP_CONSTANT] = "constant",
    [OP_NIL] = "nil",
    [OP_TRUE] = "true",
    [OP_FALSE] = "false",
    [OP_POP] = "pop",
    [OP_GET_LOCAL] = "get_local",
    [OP_SET_LOCAL] = "set_local",
    [OP_GET_GLOBAL] = "get_global",
    [OP_DEFINE_GLOBAL] = "def_global",
    [OP_SET_GLOBAL] = "set_global",
    [OP_GET_UPVALUE] = "OP_GET_UPVALUE",
    [OP_SET_UPVALUE] = "OP_SET_UPVALUE",
    [OP_GET_PROPERTY] = "OP_GET_PROPERTY",
    [OP_SET_PROPERTY] = "OP_SET_PROPERTY",
    [OP_GET_SUPER] = "OP_GET_SUPER",
    [OP_EQUAL] = "==",
    [OP_GREATER] = ">",
    [OP_LESS] = "<",
    [OP_ADD] = "+",
    [OP_SUBTRACT] = "-",
    [OP_MULTIPLY] = "*",
    [OP_DIVIDE] = "/",
    [OP_NOT] = "!",
    [OP_NEGATE] = "-",
    [OP_PRINT] = "print",
    [OP_JUMP] = "jmp",
    [OP_JUMP_IF_FALSE] = "jmp_if_false",
    [OP_LOOP] = "loop",
    [OP_CALL] = "call",
    [OP_INVOKE] = "OP_INVOKE",
    [OP_SUPER_INVOKE] = "OP_SUPER_INVOKE",
    [OP_CLOSURE] = "OP_CLOSURE",
    [OP_CLOSE_UPVALUE] = "OP_CLOSE_UPVALUE",
    [OP_RETURN] = "return",
    [OP_CLASS] = "OP_CLASS",
    [OP_INHERIT] = "OP_INHERIT",
    [OP_METHOD] = "OP_METHOD"
};

} // namespace