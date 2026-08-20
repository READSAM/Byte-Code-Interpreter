#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

#include "chunk.hpp"
#include "common.hpp"
#include "debug.hpp"
#include "vm.hpp"

static std::string readFile(std::string_view path) {
    std::ifstream file(path.data(), std::ios::in | std::ios::binary);
    if (!file) {
        std::cerr << "Could not open file \"" << path << "\".\n";
        std::exit(74);
    }

    std::ostringstream contents;
    contents << file.rdbuf();

    if (file.bad()) {
        std::cerr << "Could not read file \"" << path << "\".\n";
        std::exit(74);
    }

    return contents.str();
}

static void repl(VM& vm) {
    std::string line;

    for (;;) {
        std::cout << "> ";

        if (!std::getline(std::cin, line)) {
            std::cout << "\n";
            break;
        }

        vm.interpret(line);
    }
}

static void runFile(VM& vm, std::string_view path) {
    std::string source = readFile(path);
    InterpretResult result = vm.interpret(source);

    if (result == InterpretResult::COMPILE_ERROR) {
        std::exit(65);
    }
    if (result == InterpretResult::RUNTIME_ERROR) {
        std::exit(70);
    }
}

int main(int argc, const char* argv[]) {
    VM vm; // Construction replaces initVM(), destruction handles freeVM()

    if (argc == 1) {
        repl(vm);
    } else if (argc == 2) {
        runFile(vm, argv[1]);
    } else {
        std::cerr << "Usage: clox [path]\n";
        std::exit(64);
    }

    return 0;
}