cmake_minimum_required(VERSION 3.16)
project(CVM++ VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler warnings
if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Release build optimisations (use: cmake -DCMAKE_BUILD_TYPE=Release ..)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# ─── Sources ──────────────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/compiler.cpp
    src/vm.cpp
    src/ast_printer.cpp
)

add_executable(cvm ${SOURCES})

target_include_directories(cvm PRIVATE src)

# ─── Install ──────────────────────────────────────────────────────────────────
install(TARGETS cvm DESTINATION bin)
