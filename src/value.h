#pragma once

#include <string>
#include <type_traits>
#include <iostream>
#include "obj_type.h"

namespace aankaa {

template <class T>
inline constexpr
typename std::enable_if<
  (std::is_integral<T>::value &&
   std::is_unsigned<T>::value &&
   sizeof(T) > sizeof(unsigned int) &&
   sizeof(T) <= sizeof(unsigned long)),
  unsigned int>::type
findLastSet(T x) {
    return x ? ((8 * sizeof(unsigned long) - 1) ^ __builtin_clzl(x)) + 1 : 0;
}

template <class T>
inline constexpr
typename std::enable_if<
  std::is_integral<T>::value && std::is_unsigned<T>::value,
  T>::type
nextPowTwo(T v) {
    return v ? (T(1) << findLastSet(v - 1)) : 1;
}


enum ValueType {
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER,
    VAL_INTEGER,
    VAL_OBJ
};

class Obj;
class ObjString;
class ObjFunction;
class Value;

typedef Value (*NativeFn)(int arg_count, Value* args);

class Value {
public:
    Value() = default;
    inline Value& operator=(const Value& other) {
        //std::cout << "operator=" << std::endl;
        Value tmp(other);
        swap(tmp);
        return *this;
    }
    ~Value() {
        //std::cout << "~Value()" << std::endl;
    }
    Value(const Value& other) {
        as = other.as;
        type = other.type;
        //std::cout << "Value(const Value& other)" << std::endl;
    }
    void swap(Value& other) {
        std::swap(type, other.type);
        std::swap(as, other.as);
    }
    Value(bool v) {
        //std::cout << "Value(bool)" << std::endl;
        set_bool(v);
    }
    Value(double v) {
        set_number(v);
    }      
    Value(int v) {
        set_integer(v);
    }                           
    Value(Obj* obj);

    bool is_bool() const {
        return type == VAL_BOOL;
    }
    bool is_nil() const {
        return type == VAL_NIL;
    }
    bool is_obj() const {
        return type == VAL_OBJ;
    }    
    bool is_falsey() const {
        return is_nil() || (is_bool() && as_bool() == false) || (is_integer() && as_integer() == 0);
    }
    bool is_number() const {
        return type == VAL_NUMBER;
    }
    bool is_integer() const {
        return type == VAL_INTEGER;
    }    
    bool as_bool() const {
        return as.boolean;
    }
    double as_number() const {
        return as.number;
    }
    int as_integer() const {
        return as.integer;
    }     
    void set_bool(bool v) {
        as.boolean = v;
        type = VAL_BOOL;
    }
    void set_number(double v) {
        as.number = v;
        type = VAL_NUMBER;
    }
    void set_integer(double v) {
        as.integer = v;
        type = VAL_INTEGER;
    }    
    void set_nil() {
        type = VAL_NIL;
    }

    ObjString* as_string() const;
    Obj* as_obj() const;
    const char* as_cstring() const;
    ObjFunction* as_function() const;
    NativeFn as_native() const;
    bool is_string() const;
    ObjType obj_type() const;
    bool is_obj_type(ObjType type) const;
    void set_obj(Obj* obj);
    std::string to_string() const;  

public:
    ValueType type;
    union {
        bool boolean;
        double number;
        int integer;
        Obj* obj;
    } as;     
};

bool operator==(const Value& a, const Value& b);

} // namespace