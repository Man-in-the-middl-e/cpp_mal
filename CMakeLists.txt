cmake_minimum_required(VERSION 3.16.3)

project(MAL)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")
add_compile_options(-Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wunused -pedantic -std=c++17 -ggdb3)

if (${STEP} STREQUAL "step0")
    add_executable(step0_repl step0_repl.cpp)
    return()
endif()

if (${STEP} STREQUAL "step1")
    add_executable(step1_read_print step1_read_print.cpp lexer.cpp reader.cpp maltypes.cpp)
    return()
endif()

if (${STEP} STREQUAL "step2")
    add_executable(step2_eval step2_eval.cpp lexer.cpp reader.cpp maltypes.cpp env.cpp)
    return()
endif()

if (${STEP} STREQUAL "step3")
    add_executable(step3_env step3_env.cpp lexer.cpp reader.cpp maltypes.cpp env.cpp)
    return()
endif()

if (${STEP} STREQUAL "step4")
    add_executable(step4_if_fn_do step4_if_fn_do.cpp lexer.cpp reader.cpp maltypes.cpp env.cpp)
    return()
endif()

