#include <iostream>
#include <string>
#include <string_view>

#include "eval_ast.h"
#include "maltypes.h"
#include "printer.h"
#include "reader.h"

using MalType = mal::MalType;

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

    std::cout << "user> ";
    for (std::string currentLine; std::getline(std::cin, currentLine);) {
        if (!currentLine.empty()) {
            std::cout << rep(currentLine, env) << "\n";
        }
        std::cout << "user> ";
    }
}