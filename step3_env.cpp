#include <iostream>
#include <string>
#include <string_view>

#include "maltypes.h"
#include "printer.h"
#include "reader.h"
#include "eval_ast.h"

using MalType = mal::MalType;

std::shared_ptr<MalType>
read(std::string_view program)
{
    return mal::readStr(program);
}

std::shared_ptr<MalType>
eval(std::shared_ptr<MalType> ast)
{
    static mal::Env env;
    return EVAL(ast, env);
}

std::string
print(std::shared_ptr<MalType> program)
{
    return print_st(program.get());
}

std::string
rep(std::string_view program)
{
    return print(eval(read(program)));
}

int main()
{
    std::cout << "user> ";
    for (std::string currentLine; std::getline(std::cin, currentLine);) {
        std::cout << rep(currentLine) << "\n";
        std::cout << "user> ";
    }
}