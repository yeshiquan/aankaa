#include <iostream>
#include <fstream>

// #include "scanner.h"
// #include "token.h"
// #include "expr.h"
// #include "stmt.h"
// #include "parser.h"
// #include "interpret.h"

#include "scanner.h"
#include "parser.h"
#include "vm.h"
#include "object.h"

using aankaa::Scanner;
using aankaa::Token;
using aankaa::Parser;

std::string read_file(std::string file_path) {
    std::ifstream t(file_path.c_str());
    std::string content;
    t.seekg(0, std::ios::end);
    content.reserve(t.tellg());
    t.seekg(0, std::ios::beg);
    content.assign((std::istreambuf_iterator<char>(t)),
                std::istreambuf_iterator<char>());
    return content;
}


int main(int argc, char* argv[]) {
    int i = 0;
    std::string file_path;
    if (argc > 1) {
        std::string arg1(argv[1]);
        file_path = arg1;
    } else {
        std::cout << "example: ./aankaa prog.js" << std::endl;
        return -1;
    }

    std::string source = read_file(file_path);

    Scanner s;
    s.reset(source);        
    Parser parser(&s);

    std::cout << "\n=================== compiler =========================" << std::endl;
    parser.current_chunk().clear();
    parser.advance();
    aankaa::ObjFunction* function = parser.compile();

    std::cout << "\n=================== vm run =========================" << std::endl;
    aankaa::VM vm;
    vm.interpret(function);

    std::cout << "\n=================== gc =========================" << std::endl;
    return 0;
}
