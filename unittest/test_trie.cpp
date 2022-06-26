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
#include "trie.h"
#undef private
#undef protected

#include <typeinfo>       // operator typeid

namespace test {

class TrieTest : public ::testing::Test {
private:
    virtual void SetUp() {
    }
    virtual void TearDown() {
    }
protected:
};

TEST_F(TrieTest, test_trie) {
    base::trie::Trie trie;
    trie.insert("hello");
    trie.insert("word");
    std::cout << "hello -> " << trie.search("hello") << std::endl;
    std::cout << "word -> " << trie.search("word") << std::endl;
}

}