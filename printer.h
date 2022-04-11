#pragma once

#include "maltypes.h"

#include <sstream>

std::string escapeString(const std::string& str)
{
    std::stringstream ss;
    ss << '"';
    for (size_t i = 1; i + 1 < str.size(); ++i) {
        const char currentChar = str[i];
        if (currentChar == '\\' || currentChar == '"')
        {
            ss << "\\";
        }
        else if (currentChar == '\n')
        {
            ss << '\\' << 'n';
            continue;
        }
        ss << currentChar;
    }
    ss << '"';
    return ss.str();
}

std::string print_st(mal::MalType* malObject, bool printReadably = true)
{
    if (auto malStr = malObject->asMalString(); malStr && printReadably) {
        return malStr->isEmpty() ? malStr->asString() : escapeString(malStr->asString());
    }
    return malObject->asString();
}
