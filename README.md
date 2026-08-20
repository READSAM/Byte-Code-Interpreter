# Byte-Code-Interpreter

A bytecode interpreter implemented in C++, following the design and architecture of **clox** illustrated in the book [*Crafting Interpreters*](https://craftinginterpreters.com/) by Robert Nystorm.

## About the Project

This repository contains a stack-based bytecode virtual machine and compiler. It bridges high-level source code execution down to low-level byte instructions, featuring custom memory management, a chunk-based bytecode representation, and a fast evaluation loop.

## Project Structure

Here is an overview of the core files in the codebase:

*   **`main.cpp`**: Entry point for the interpreter application. Handles starting the REPL (Read-Eval-Print Loop) or running script files.
*   **`chunk.hpp` / `chunk.cpp`**: Manages dynamic arrays of bytecode instructions and constant pools.
*   **`compiler.hpp` / `compiler.cpp`**: Translates source code tokens into low-level executable bytecode chunks.
*   **`debug.hpp` / `debug.cpp`**: Disassembly utilities used to print instructions for debugging and tracing execution.
*   **`common.hpp`**: Common configuration macros, type definitions, and standard includes used across the project.

## Getting Started

### Prerequisites

To build and run this project, you will need:
*   A modern C++ compiler supporting C++17 or later (e.g., GCC, Clang, or MSVC)
*   Make, CMake, or a build tool configured for your environment

### Building the Project

Clone the repository and compile the source files using your preferred C++ compiler:

```bash
git clone [https://github.com/READSAM/Byte-Code-Interpreter.git](https://github.com/READSAM/Byte-Code-Interpreter.git)
cd Byte-Code-Interpreter
g++ -std=c++17 main.cpp chunk.cpp compiler.cpp debug.cpp -o clox
```

### Running the Interpreter
* Start the REPL:

```bash
./clox
```
* Run a script file:
```bash
./clox path/to/script.lox
```
### Reference
This implementation follows the second part of Crafting Interpreters by Robert Nystorm, focusing on building a bytecode virtual machine from scratch in a systems programming language.
