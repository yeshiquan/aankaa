#pragma once
#include <vector>
#include <type_traits>
#include <iostream>

#ifndef DEFAULT_POOL_NUM
#define DEFAULT_POOL_NUM 4
#endif

// 一种简单的对象池，适用于多次分配，最后统一回收的场景
// 不支持单个对象的回收，不支持线程安全

namespace aankaa {

using Allocator = std::allocator<uint8_t>;

template<typename NODE>
class DoubleLinkedList {
public:
    void push_back(NODE* node) {
        node->next = nullptr;
        if (tail) {
            tail->next = node;    
        } else {
            head = node;
        }
        tail = node;
        count++;
    }
    NODE* pop_front() {
        if (head) {
            NODE* ret = head;
            head = reinterpret_cast<NODE*>(head->next);
            count--;
            return ret;
        } 
        tail = nullptr;
        return nullptr;
    }
    void clear() {
        head = nullptr;
        tail = nullptr;
        count = 0;
    }
public:
    NODE* head = {nullptr};
    NODE* tail = {nullptr};
    int count = {0};
};

template<typename T>
class ObjectPool {
public:
    ObjectPool() {
        init(DEFAULT_POOL_NUM);
    }

    ObjectPool(size_t num) {
        init(num);
    }

    ObjectPool(ObjectPool const&) = delete;             // Copy construct
    ObjectPool(ObjectPool&&) = delete;                  // Move construct
    ObjectPool& operator=(ObjectPool const&) = delete;  // Copy assign
    ObjectPool& operator=(ObjectPool &&) = delete;      // Move assign    

    void init(size_t num) {
        _elements = (T*)Allocator().allocate(sizeof(T) * num);
        _capacity = num;
    }

    template<class... Args>
    T* get(Args&&... args) {
        // 有先从数据缓存分配
        if (_size < _capacity) {
            new(_elements + _size) T{std::forward<Args>(args)...};
            return (_elements + _size++);
        }
        // 缓存不够用，动态创建一个对象，并挂载到链表中
        T* new_obj = (T*)Allocator().allocate(sizeof(T));
        new(new_obj) T{std::forward<Args>(args)...};
        list.push_back(new_obj);
        return new_obj;
    }

    void clear() {
        if (_size > 0) {
            std::cout << "delete " << _size + list.count << " object<" << typeid(_elements[0]).name() << "> from pool" << std::endl;
        }        
        if constexpr (!std::is_trivial_v<T>) {
            // 非平凡数据需要调用析构函数
            // 析构数组缓存
            for (int k = 0; k < _size; ++k) {
                reinterpret_cast<T*>(&_elements[k])->~T();
            }
        }
        _size = 0;
        // if (list.count > 0) {
        //     std::cout << "delete " << list.count << " object<" << typeid(*list.head).name() << "> from list" << std::endl;
        // }
        for (T* obj = list.pop_front(); obj != nullptr; obj = list.pop_front()) {
            if constexpr (!std::is_trivial_v<T>) {
                reinterpret_cast<T*>(obj)->~T();
            }
            Allocator().deallocate((uint8_t*)obj, sizeof(T));
        }
    }
    ~ObjectPool() {
        clear();
        if (_elements != nullptr ) { 
            Allocator().deallocate((uint8_t*)_elements, sizeof(T) * _capacity);
        }        
        _elements = nullptr;
        _capacity = 0;
    }
private:
    T* _elements = nullptr;  //数组缓存，预先创建好，容量有限
    DoubleLinkedList<T> list; // 动态创建的对象，缓存不够用的时候启用
    int _size = 0;
    size_t _capacity = 0;
};

} // namespace