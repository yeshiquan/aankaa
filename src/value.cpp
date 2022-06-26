#include <string>
#include <vector>
#include <stdarg.h>

#include "value.h"
#include "object.h"

namespace aankaa {
Value::Value(Obj* obj) {
    set_obj(obj);
}

ObjString* Value::as_string() const {
    return (ObjString*)(as.obj);
}        
Obj* Value::as_obj() const {
    return as.obj;
}
NativeFn Value::as_native() const {
    return ((ObjNative*)as_obj())->function;
}
ObjFunction* Value::as_function() const {
    return (ObjFunction*)(as.obj);
}    
ObjType Value::obj_type() const {
    return as_obj()->type;
}
bool Value::is_obj_type(ObjType type) const {
    return is_obj() && obj_type() == type;
}    
const char* Value::as_cstring() const {
    ObjString* objs = (ObjString*)(as_obj());
    return objs->buffer.c_str();
}
void Value::set_obj(Obj* obj) {
    as.obj = obj;
    type = (obj != nullptr ? VAL_OBJ : VAL_NIL);
}    
bool Value::is_string() const {
    return is_obj_type(OBJ_STRING);
}        
std::string Value::to_string() const {
    if (is_nil()) {
        return "nil";
    } else if (is_bool()) {
        return as_bool() ? "true" : "false";
    } else if (is_number()) {
        return std::to_string(as_number());
    } else if (is_integer()) {
        return std::to_string(as_integer());
    } else if (is_obj_type(OBJ_STRING)) {
        return as_string()->to_string();
    } else if (is_obj_type(OBJ_FUNCTION)){
        return "fun(" + as_function()->name->buffer + ")";
    } else if (is_obj_type(OBJ_NATIVE)){
        return "native()";
    }
    return "unknown_value";
}    

bool operator==(const Value& a, const Value& b) { 
    if (a.type != b.type) {
        return false;
    }

    switch (a.type) {
    case VAL_BOOL:   return a.as_bool() == b.as_bool();
    case VAL_NIL:    return true;
    case VAL_NUMBER: return a.as_number() == b.as_number();
    case VAL_OBJ:    return a.as_obj() == b.as_obj();
    default:         return false; // Unreachable.
    }

    return false;
}

} // namespace