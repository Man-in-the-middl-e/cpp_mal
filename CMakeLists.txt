cmake_minimum_required(VERSION 3.16.3)

project(MAL)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")
add_compile_options(-Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wunused -pedantic -std=c++2a -ggdb3)

if (${STEP} STREQUAL "step0")
    add_executable(step0_repl step0_repl.cpp)
    return()
endif()

if (${STEP} STREQUAL "step1")
    add_executable(step1_read_print step1_read_print.cpp lexer.cpp reader.cpp maltypes.cpp env.cpp eval_ast.cpp buildins.cpp)
    return()
endif()

if (${STEP} STREQUAL "step2")
    add_executable(step2_eval step2_eval.cpp lexer.cpp reader.cpp maltypes.cpp env.cpp eval_ast.cpp buildins.cpp)
    return()
endif()

if (${STEP} STREQUAL "step3")
    add_executable(step3_env step3_env.cpp lexer.cpp reader.cpp maltypes.cpp env.cpp eval_ast.cpp buildins.cpp)
    return()
endif()

if (${STEP} STREQUAL "step4")
    add_executable(step4_if_fn_do step4_if_fn_do.cpp lexer.cpp reader.cpp maltypes.cpp env.cpp eval_ast.cpp buildins.cpp)
    return()
endif()

if (${STEP} STREQUAL "step5")
    add_executable(step5_tco step5_tco.cpp lexer.cpp reader.cpp maltypes.cpp env.cpp eval_ast.cpp buildins.cpp)
    return()
endif()

if (${STEP} STREQUAL "step6")
    add_executable(step6_file step6_file.cpp lexer.cpp reader.cpp maltypes.cpp env.cpp eval_ast.cpp buildins.cpp)
    return()
endif()

if (${STEP} STREQUAL "step7")
    add_executable(step7_quote step7_quote.cpp lexer.cpp reader.cpp maltypes.cpp env.cpp eval_ast.cpp buildins.cpp)
    return()
endif()

if (${STEP} STREQUAL "step8")
    add_executable(step8_macros step8_macros.cpp lexer.cpp reader.cpp maltypes.cpp env.cpp eval_ast.cpp buildins.cpp)
    return()
endif()

