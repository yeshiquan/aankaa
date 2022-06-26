
#pragma once

#include <string>
#include <sys/time.h>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <thread>

#include "value.h"
#include "chunk.h"
#include "object.h"
#include "pool.h"

namespace aankaa {

#define UINT8_COUNT (UINT8_MAX + 1)
constexpr uint32_t FRAMES_MAX = 64;
constexpr uint32_t STACK_MAX = FRAMES_MAX * UINT8_COUNT;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

struct CallFrame {
    ObjFunction* function = nullptr;
    uint8_t* ip = nullptr;
    Value* slots = nullptr;
};

class FrameList {
public:
    CallFrame* current_frame() {
        return &frames[_frame_count - 1];
    }
    CallFrame* new_frame() {
        return &frames[_frame_count++];
    }
    CallFrame* at(int i) {
        return &frames[i];
    }    
    void destroy_frame() {
        _frame_count--;
    }
    void reset() {
        _frame_count = 0;
    }
    int frame_count() const {
        return _frame_count;
    }
private:
    CallFrame frames[FRAMES_MAX];
    int _frame_count = 0;
};

#define READ_BYTE() (*frame->ip++)

#define READ_CONSTANT() (frame->function->chunk->constants[READ_BYTE()])

#define READ_SHORT()  \
    (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))

#define BINARY_OP(op)  \
    do { \
        if (!peek(0).is_number() || !peek(1).is_number()) { \
            runtime_error( \
                "Operands must be two numbers or two strings."); \
            return INTERPRET_RUNTIME_ERROR; \
        } \
        double b = pop().as_number(); \
        double a = pop().as_number(); \
        push(Value(a op b)); \
    } while(false)



inline Value clock_native(int arg_count, Value* args) {
    printf("clock_native()...\n");
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    double res = tv.tv_usec + (uint64_t)tv.tv_sec * 1000000;

    return Value(res);
}
inline Value sleep_native(int arg_count, Value* args) {
    printf("sleep_native()...\n");
    int sleep_ms = args[0].as_number();
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    return Value(nullptr);
}

class VM {
public:
    VM() {
        reset_stack();
        define_native("clock", clock_native);
        define_native("sleep", sleep_native);
    }
    InterpretResult interpret(ObjFunction* function);

    void reset_stack() {
        stack_top = stack_bottom;
        frames.reset();
    }
    void push(const Value& v) {
        *stack_top = v;
        stack_top++;
    }

    Value pop() {
        stack_top--;
        return *stack_top;
    }
    Value peek(int offset) {
        return stack_top[-1 - offset];
    }
    bool call_value(Value callee, int arg_count);
    bool call(ObjFunction* function, int arg_count);
    InterpretResult run();
    InterpretResult interpret();
    void runtime_error(const char* format, ...);
    void concatenate();
    void define_native(const char* name, NativeFn function);
public:
    FrameList frames;
    Value stack_bottom[STACK_MAX];
    Value* stack_top = nullptr;
    ObjectPool<ObjString> string_pool;
    ObjectPool<ObjNative> native_pool;
    std::unordered_map<std::string, Value> globals;
};

} //namespace