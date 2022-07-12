#include <iostream>
#include <string>
#include <string_view>
#include <sstream>

#include "eval_ast.h"
#include "maltypes.h"
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
    return program->asString();
}

std::string
rep(std::string_view program, mal::Env& env)
{
    return print(eval(read(program), env));
}

bool startWithComment(const std::string& line)
{
    for (auto ch : line) {
        if (ch != ' ') {
            return ch == ';';
        }
    }
    return false;
}

int main(int argc, char* argv[])
{
    mal::GlobalEnv::the().setUpArgv(argc, argv);
    static mal::Env env;

    if (argc > 1) {
        std::ostringstream malProgramToLoadFile;
        malProgramToLoadFile << "(load-file " << '"' << argv[1] << "\")";
        std::cout << rep(malProgramToLoadFile.str(), env) << '\n';
    } else {
        std::cout << "user> ";
        for (std::string currentLine; std::getline(std::cin, currentLine);) {
            if (!currentLine.empty() && !startWithComment(currentLine)) {
                std::cout << rep(currentLine, env) << '\n';
            }
            std::cout << "user> ";
        }
    }
}