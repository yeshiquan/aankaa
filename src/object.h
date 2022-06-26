#pragma once

#include <string>
#include <string.h>
#include <vector>
#include "value.h"
#include "obj_type.h"

namespace aankaa {

struct Obj {
    ObjType type;
    struct Obj* next = nullptr;
};

struct ObjString : public Obj {
    ObjString(const char* src, int length) {
        buffer.assign(src, length);
        type = OBJ_STRING;
    }
    ObjString(std::string src) {
        buffer = std::move(src);
        type = OBJ_STRING;
    }    
    ~ObjString() {
    }
    std::string to_string() {
        return "\"" + buffer + "\"";
    }
    std::string buffer;
};

class Chunk;

struct ObjFunction : public Obj {
    ObjFunction();
    ~ObjFunction();

    Obj obj;
    int arity = 0;
    Chunk* chunk = nullptr;
    ObjString* name = nullptr;
};

struct ObjNative : public Obj {
    ObjNative(NativeFn function_) : function(function_) {
        type = OBJ_NATIVE;
    }
    Obj obj;
    NativeFn function;
};

} //namespace