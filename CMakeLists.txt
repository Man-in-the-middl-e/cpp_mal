cmake_minimum_required(VERSION 3.16.3)

project(MAL)

add_compile_options(-Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wunused -pedantic -std=c++17 -g)

if (${STEP} STREQUAL "step0")
    add_executable(step0_repl step0_repl.cpp)
    return()
endif()

if (${STEP} STREQUAL "step1")
    add_executable(step1_read_print step1_read_print.cpp)
    return()
endif()
