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
#undef private
#undef protected

#include <typeinfo>       // operator typeid

using aankaa::Scanner;
using aankaa::Token;

namespace test {

class ScannerTest : public ::testing::Test {
private:
    virtual void SetUp() {
    }
    virtual void TearDown() {
    }
protected:
};

TEST_F(ScannerTest, test_scan) {
    Scanner s;
    std::string source = "if (a*2 > 3) {\n print b+4;\n}";
    s.reset(source);
    std::vector<Token> tokens = s.scan();
    int line = -1;
    for (Token& token : tokens) {
        std::string t(token.start, token.length);
        if (token.line != line) {
            std::cout << token.line+1 << " -> " << t << std::endl;
            line = token.line;
        } else {
            std::cout << "   -> " << t << std::endl;
        }
    }
}

}