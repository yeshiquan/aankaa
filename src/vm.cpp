
#include <string>
#include <vector>
#include <stdarg.h>

#include "vm.h"
#include "value.h"
#include "token.h"
#include "object.h"

namespace aankaa {

void VM::runtime_error(const char* format, ...) {
    fprintf(stderr, "\n-----------------  Runtime Error -----------------\n");
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    int idx = 0;
    for (int i = frames.frame_count() - 1; i >= 0; i--) {
        CallFrame* frame = frames.at(i);
        ObjFunction* function = frame->function;
        size_t instruction = frame->ip - &function->chunk->code[0] - 1;
        if (function->name == nullptr) {
            fprintf(stderr, "#%d    script\n", idx);
        } else if (function->name->buffer == "") {
            fprintf(stderr, "#%d    script\n", idx);
        } else {
            fprintf(stderr, "#%d    %s()\n", idx, function->name->buffer.c_str());
        }
        idx++;
    }

    reset_stack();
}    

void VM::concatenate() {
    ObjString* b = peek(0).as_string();
    ObjString* a = peek(1).as_string();
    ObjString* result = string_pool.get(a->buffer + b->buffer);
    pop();
    pop();
    push(Value(result));
}

InterpretResult VM::interpret(ObjFunction* function) {
    if (function == nullptr) {
        return INTERPRET_COMPILE_ERROR;
    }

    //function->chunk->print();

    // 把main函数push进去，作用是？
    push(Value(function));

    CallFrame* frame = frames.new_frame();
    frame->function = function;
    frame->ip = &function->chunk->code[0];
    frame->slots = stack_bottom;

    return run();
}

bool VM::call_value(Value callee, int arg_count) {
    if (callee.is_obj_type(OBJ_FUNCTION)) {
        return call(callee.as_function(), arg_count);
    }
    if (callee.is_obj_type(OBJ_NATIVE)) {
        //std::cout << "obj_native ============= arg_count:" << arg_count << std::endl;
        NativeFn native = callee.as_native();
        Value result = native(arg_count, stack_top - arg_count);
        stack_top -= arg_count + 1;
        push(result);
        return true;
    }
    runtime_error("Can only call function");
    return false;
}

bool VM::call(ObjFunction* function, int arg_count) {
    //function->chunk->print();
    if (arg_count != function->arity) {
        runtime_error("Expected %d arguments but got %d.", function->arity, arg_count);
        return false;
    }
    if (frames.frame_count() == FRAMES_MAX) {
        runtime_error("Stack overflow");
        return false;
    }
    CallFrame* frame = frames.new_frame();
    frame->function = function;
    frame->ip = &function->chunk->code[0];
    frame->slots = stack_top - arg_count - 1;
    return true;
}

InterpretResult VM::run() {
    CallFrame* frame = frames.current_frame();
    //std::cout << "    change frame to -> " << frame << std::endl;

    for (;;) {
        std::cout << "    stack -> [";
        for (Value* slot = stack_bottom; slot < stack_top; slot++) {
            std::cout << slot->to_string() << ",";
        }
        printf("]\n");

        uint8_t instruction = READ_BYTE();

        std::cout << ">>>>>>>>>> " << op_name[instruction] << std::endl;

        switch(instruction) {
        case OP_CONSTANT: {
            Value constant = READ_CONSTANT();
            push(constant);
            break;
        }
        case OP_NIL:  push(Value(nullptr)); break;
        case OP_TRUE:  push(Value(true)); break;
        case OP_FALSE:  push(Value(false)); break;   
        case OP_POP:  pop();  break;
        case OP_PRINT: 
            std::cout << pop().to_string() << std::endl;
            break;        
        case OP_ADD: {
            if (peek(0).is_string() && peek(1).is_string()) {
                concatenate();
            } else if (peek(0).is_number() && peek(1).is_number()) {
                double b = pop().as_number();
                double a = pop().as_number();
                push(Value(a + b));
            } else if (peek(0).is_integer() && peek(1).is_integer()) {
                int b = pop().as_integer();
                int a = pop().as_integer();
                push(Value(a + b));
            } else {
                runtime_error(
                    "Operands must be two numbers or two strings.");
                return INTERPRET_RUNTIME_ERROR;
            }
            break;
        }
        case OP_GREATER:  BINARY_OP(>); break;
        case OP_LESS:     BINARY_OP(<); break;            
        case OP_SUBTRACT: BINARY_OP(-); break;        
        case OP_DIVIDE:   BINARY_OP(/); break;
        case OP_MULTIPLY: BINARY_OP(*); break;
        case OP_EQUAL: {
            Value b = pop();
            Value a = pop();
            push(Value(a == b));
            break;
        }
        case OP_NEGATE:
            if (!peek(0).is_number()) {
                runtime_error("Operand must be a number.");
                return INTERPRET_RUNTIME_ERROR;
            }
            push(Value(-pop().as_number()));
            break;        
        case OP_GET_LOCAL: {
            uint8_t slot = READ_BYTE();
            push(frame->slots[slot]);
            break;
        }
        case OP_SET_LOCAL: {
            uint8_t slot = READ_BYTE();
            frame->slots[slot] = peek(0);
            break;
        }
        case OP_GET_GLOBAL: {
            ObjString* name = READ_CONSTANT().as_string();
            auto iter = globals.find(name->buffer);
            if (iter == globals.end()) {
                runtime_error("Undefined variable '%s'.", name->buffer.c_str());
                return INTERPRET_RUNTIME_ERROR;
            }
            push(iter->second);
            break;
        }
        case OP_DEFINE_GLOBAL: {
            ObjString* name = READ_CONSTANT().as_string();
            globals[name->buffer] = peek(0);
            pop();
            break;
        }
        case OP_SET_GLOBAL: {
            ObjString* name = READ_CONSTANT().as_string();
            auto iter = globals.find(name->buffer);
            if (iter == globals.end()) {
                runtime_error("Undefined variable '%s'.", name->buffer.c_str());
                return INTERPRET_RUNTIME_ERROR;
            }
            iter->second = peek(0);

            break;
        }
        case OP_JUMP_IF_FALSE: {
            uint16_t offset = READ_SHORT();
            if (peek(0).is_falsey()) {
                frame->ip += offset;
            }
            break;
        }
        case OP_JUMP: {
            uint16_t offset = READ_SHORT();
            frame->ip += offset;
            break;
        }
        case OP_LOOP: {
            uint16_t offset = READ_SHORT();
            frame->ip -= offset;
            break;
        }
        case OP_CALL: {
            int arg_count = READ_BYTE();
            if (!call_value(peek(arg_count), arg_count)) {
                return INTERPRET_RUNTIME_ERROR;
            }
            // call_value成功，增加了一个新的frame，当前的frame需要更新一下
            frame = frames.current_frame();
            //std::cout << "    change frame to -> " << frame << std::endl;
            break;
        }
        case OP_RETURN: {
            Value result = pop();
            frames.destroy_frame();
            if (frames.frame_count() == 0) {
                // main函数
                pop();
                return INTERPRET_OK;
            }
            // 非main函数，上一个frame的栈底变成栈顶了，相当于作废了上一个frame
            stack_top = frame->slots;
            push(result);
            frame = frames.current_frame();
            //std::cout << "    change frame to -> " << frame << std::endl;
            break;
        }
        default:
            break;         
        }
    }
}

void VM::define_native(const char* name, NativeFn function) {
    ObjString* obj_str = string_pool.get(name);
    push(Value(obj_str));
    push(Value(native_pool.get(function)));
    globals.emplace(std::string(name), stack_bottom[1]);
    pop();
    pop();
}


} //namespace