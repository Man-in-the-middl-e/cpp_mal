#include "maltypes.h"

#include <sstream>
#include <iostream>

namespace {
std::string unescapeString(std::string_view str)
{
    std::stringstream ss;
    // translate:
    //      '\n' -> newline
    //      '\"' -> '"\'
    //      '\\' -> '\'
    for (size_t i = 0; i < str.size();) {
        if (i + 1 >= str.size()) {
            ss << str.back();
            break;
        }
        const char currentChar = str[i];
        const char nextChar = str[i + 1];
        if (currentChar == '\\') {
            bool escaped = true;
            switch (nextChar) {
            case 'n':
                ss << "\n";
                break;
            case '\\':
                ss << "\\";
                break;
            case '"':
                ss << "\"";
                break;
            default:
                escaped = false;
                break;
            }
            if (escaped) {
                i += 2;
                continue;
            }
        }
        ss << currentChar;
        ++i;
        }
    return ss.str();
}
} 
namespace mal {

MalType::~MalType() { }

MalNumber::MalNumber(std::string_view number)
    : m_number(std::atoi(number.data()))
{
}

std::string MalNumber::asString() const
{
    return std::to_string(m_number);
}

int MalNumber::getValue() const
{
    return m_number;
}

MalContainer::MalContainer(char openBracket, char closeBracket)
    : m_openBracket(openBracket)
    , m_closeBracket(closeBracket)
{
}

std::string MalContainer::asString() const
{
    std::stringstream ss;
    ss << m_openBracket;
    for (const auto& obj : m_data) {
        ss << obj->asString() << (m_data.back() != obj ? " " : "");
    }
    ss << m_closeBracket;
    return ss.str();
}

void MalContainer::append(std::unique_ptr<MalType> element)
{
    m_data.push_back(std::move(element));
}

MalList::MalList()
    : MalContainer('(',')')
{
}

MalVector::MalVector()
    : MalContainer('[',']')
{
}

MalSymbol::MalSymbol(std::string_view symbol)
    : m_symbol(symbol.data(), symbol.size())
{
}

std::string MalSymbol::asString() const 
{
    return m_symbol;
}

MalString::MalString(std::string_view str)
    : m_malString(unescapeString(str))
{
}

std::string MalString::asString() const 
{
    if (m_malString.size() == 0)
        return "unbalanced";
    return m_malString;
}

bool MalString::isEmpty() const 
{
    return m_malString.empty();
}

std::string MalNil::asString() const 
{
    return "nil";
}

MalBoolean::MalBoolean(bool value)
    : m_boolValue(value)
{
}

std::string MalBoolean::asString() const 
{
    return m_boolValue ? "true" : "false";
}

bool MalBoolean::getValue() const
{
    return m_boolValue;
}

std::string MalHashMap::asString() const 
{
    std::stringstream ss;
    ss << "{";
    for (auto it = m_hasMap.begin(); it != m_hasMap.end(); ++it) {
        const auto& [key, value] = *it;
        ss << key << " " << value->asString() << (std::distance(it, m_hasMap.end()) > 1 ? " " : "");
    }
    ss << "}";
    return ss.str();
}

void MalHashMap::insert(const std::string& key, std::unique_ptr<MalType> value)
{
    m_hasMap.insert({key, std::move(value)});
}

} // namespace mal