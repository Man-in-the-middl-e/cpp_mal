
#include <iostream>
#include <string>
#include <string_view>

std::string_view
read(std::string_view program)
{
    return program;
}

std::string_view
eval(std::string_view program)
{
    return program;
}

std::string_view
print(std::string_view program)
{
    return program;
}

std::string_view
rep(std::string_view program)
{
    return print(eval(read(program)));
}

int main()
{
    std::cout << "user> ";
    for (std::string currentLine; std::getline(std::cin, currentLine);) {
        std::cout << currentLine << "\n";
        std::cout << "user> ";
    }
}