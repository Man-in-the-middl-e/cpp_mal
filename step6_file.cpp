#include <iostream>
#include <string>
#include <string_view>

#include "core.h"
#include "eval_ast.h"
#include "maltypes.h"
#include "printer.h"
#include "reader.h"

using MalType = mal::MalType;

std::unordered_map<std::string, std::shared_ptr<mal::MalCallable>> setUpBuildins()
{
    const static std::unordered_map<std::string, std::shared_ptr<mal::MalCallable>> buildins {
        { "prn", std::make_shared<mal::MalCallable>(mal::prn) },
        { "list", std::make_shared<mal::MalCallable>(mal::list) },
        { "list?", std::make_shared<mal::MalCallable>(mal::isList) },
        { "empty?", std::make_shared<mal::MalCallable>(mal::isEmpty) },
        { "count", std::make_shared<mal::MalCallable>(mal::count) },
        { "=", std::make_shared<mal::MalCallable>(mal::equal) },
        { "<", std::make_shared<mal::MalCallable>(mal::less) },
        { "<=", std::make_shared<mal::MalCallable>(mal::lessEqual) },
        { ">", std::make_shared<mal::MalCallable>(mal::greater) },
        { ">=", std::make_shared<mal::MalCallable>(mal::greaterEqual) },
        { "+", std::make_shared<mal::MalCallable>(mal::plus) },
        { "-", std::make_shared<mal::MalCallable>(mal::minus) },
        { "/", std::make_shared<mal::MalCallable>(mal::divides) },
        { "*", std::make_shared<mal::MalCallable>(mal::multiplies) },
    };
    return buildins;
}

std::shared_ptr<MalType>
read(std::string_view program)
{
    return mal::readStr(program);
}

std::shared_ptr<MalType>
eval(std::shared_ptr<MalType> ast, mal::Env& env)
{
    return EVAL(ast, env);
}

std::string
print(std::shared_ptr<MalType> program)
{
    return print_st(program.get());
}

std::string
rep(std::string_view program, mal::Env& env)
{
    return print(eval(read(program), env));
}

int main()
{
    static mal::Env env;
    for (const auto& [key, value] : setUpBuildins()) {
        env.set(key, value);
    }

    std::cout << "user> ";
    for (std::string currentLine; std::getline(std::cin, currentLine);) {
        std::cout << rep(currentLine, env) << "\n";
        std::cout << "user> ";
    }
}