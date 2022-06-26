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
#include "scanner.h"
#include "parser.h"
#undef private
#undef protected

#include <typeinfo>       // operator typeid

using aankaa::Scanner;
using aankaa::Token;
using aankaa::Parser;

namespace test {

class ParserTest : public ::testing::Test {
private:
    virtual void SetUp() {
    }
    virtual void TearDown() {
    }
protected:
};

TEST_F(ParserTest, test_parse) {
    {
        Scanner s;
        std::string source = "1+2*3-4";
        std::cout << "\n============================= " << source << std::endl;
        s.reset(source);        
        Parser parser(&s);
        parser.advance();
        parser.expression();
        parser.current_chunk().print();
    }
    {
        Scanner s;
        std::string source = "1+2*(3-4)";
        std::cout << "\n============================= " << source << std::endl;
        s.reset(source);        
        Parser parser(&s);
        parser.current_chunk().clear();
        parser.advance();
        parser.expression();
        parser.current_chunk().print();
    }    
    
    {
        Scanner s;
        std::string source = "(-1 + 2) * 3 - -4";
        std::cout << "\n============================= " << source << std::endl;
        s.reset(source);        
        Parser parser(&s);
        parser.current_chunk().clear();
        parser.advance();
        parser.expression();
        parser.current_chunk().print();
    }
    {
        // 测试带 = 的表达式，只返回 = 前面的
        Scanner s;
        std::string source = "1 + 3 = 2 + 4";
        std::cout << "\n============================= " << source << std::endl;
        s.reset(source);        
        Parser parser(&s);
        parser.current_chunk().clear();
        parser.advance();
        parser.expression();
        parser.current_chunk().print();
    }    
    {
        Scanner s;
        std::string source = "\"hello world\"";
        std::cout << "\n============================= " << source << std::endl;
        s.reset(source);        
        Parser parser(&s);
        parser.current_chunk().clear();
        parser.advance();
        parser.expression();
        parser.current_chunk().print();
    }
}

}