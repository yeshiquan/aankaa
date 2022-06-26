#include <iostream>
#include <chrono>
#include <future>
#include <atomic>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <functional>
#include <thread>
#include <vector>


#include <fstream>
#include <memory>
#include "gtest/gtest.h"

#define private public
#define protected public
#include "value.h"
#undef private
#undef protected

#include <typeinfo>       // operator typeid

using aankaa::Value;

namespace test {

class ValueTest : public ::testing::Test {
private:
    virtual void SetUp() {
    }
    virtual void TearDown() {
    }
protected:
};

TEST_F(ValueTest, test_value) {
    Value value;
    value.set_number(3.2);
    std::cout << "value -> " << value.as_number() << std::endl;
    std::cout << "value -> " << value.as_bool() << std::endl;
    std::cout << "is_number -> " << value.is_number() << std::endl;
    std::cout << "is_bool -> " << value.is_bool() << std::endl;

    value.set_bool(true);
    std::cout << "value -> " << value.as_bool() << std::endl;
    std::cout << "value -> " << value.as_number() << std::endl;
    std::cout << "is_number -> " << value.is_number() << std::endl;
    std::cout << "is_bool -> " << value.is_bool() << std::endl;

    Value v2 = false;
    Value v3(false);
    std::cout << "is_bool -> " << v2.is_bool() << std::endl;
    std::cout << "value -> " << v2.as_bool() << std::endl;

}

}
