
#pragma once

#include <string>
#include <vector>
#include "value.h"

namespace aankaa {

enum OpCode {
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_GLOBAL,
    OP_GET_UPVALUE,
    OP_SET_UPVALUE,
    OP_GET_PROPERTY,
    OP_SET_PROPERTY,
    OP_GET_SUPER,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_NEGATE,
    OP_PRINT,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    OP_CALL,
    OP_INVOKE,
    OP_SUPER_INVOKE,
    OP_CLOSURE,
    OP_CLOSE_UPVALUE,
    OP_RETURN,
    OP_CLASS,
    OP_INHERIT,
    OP_METHOD
};

extern const char* op_name[];

// 1 + 2 * 3 - 4的解析结果：
// code -> OP_CONSTANT,1,OP_CONSTANT,2,OP_CONSTANT,3,OP_MULTIPLY,OP_ADD,OP_CONSTANT,4,OP_SUBTRACT,
// constants -> 1,2,3,4,

class Chunk {
public:
    Chunk() = default;
    ~Chunk() {}
    void write(uint8_t byte, int line) {
        code.emplace_back(byte);
        lines.emplace_back(line);
        count++;
    }

    // 添加一个常量，返回常量所在的下标
    int add_constant(Value value) {
        constants.emplace_back(value);
        return constants.size() - 1;
    }
    void print() {

                
        std::cout << "chunk:" << this << " code[" << code.size() << "] -> \n";
        for (int i = 0; i < code.size();) {
            printf("%3d    ", i);
            if (code[i] == OP_GET_LOCAL || code[i] == OP_SET_LOCAL || code[i] == OP_CALL) {
                std::cout << op_name[code[i]];
                std::cout << "(" << static_cast<int>(code[i+1]) << ")\n";
                i += 2;
            } else if (code[i] == OP_JUMP_IF_FALSE || code[i] == OP_JUMP || code[i] == OP_LOOP) {
                std::cout << op_name[code[i]];
                uint16_t offset = (static_cast<uint16_t>(code[i+1]) << 8) | static_cast<uint16_t>(code[i+2]);
                printf("(%hu)\n", offset);
                i += 3;
            } else if (code[i] == OP_CONSTANT) {
                int cons_idx = code[i+1];
                Value& v = constants.at(cons_idx);
                std::cout << "[" << v.to_string() << "]\n";
                i += 2;
            } else if ((code[i] == OP_DEFINE_GLOBAL
                        || code[i] == OP_SET_GLOBAL
                        || code[i] == OP_GET_GLOBAL)) {
                std::cout << op_name[code[i]];
                int cons_idx = code[i+1];
                Value& v = constants[cons_idx];
                std::cout << "(" << v.to_string() << ")\n";
                i += 2;
            } else {
                std::cout << op_name[code[i]] << "\n";
                i += 1;
            }
        }
        std::cout << "constants -> ";
        for (int i = 0; i < constants.size(); ++i) {
            std::cout << constants[i].to_string() << ",";
        }
        std::cout << std::endl << std::flush;
    }
    void clear() {
        count = 0;
        code.clear();
        lines.clear();
        constants.clear();
    }
public:
    int count = 0;
    int capacity = 0;
    std::vector<uint8_t> code;
    std::vector<int> lines;
    std::vector<Value> constants;
};

} //namespace