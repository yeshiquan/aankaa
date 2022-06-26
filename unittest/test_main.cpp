#include <stdio.h>
#include <memory>

#include "gtest/gtest.h"
#include <string>
#include <fstream>
#include <streambuf>

#define private public
#define protected public
#undef private
#undef protected

int main(int argc,char *argv[]) {
    fprintf(stderr, ">>> Test start...\n");
    testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}