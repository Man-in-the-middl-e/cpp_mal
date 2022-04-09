#include "maltypes.h"

#include <iostream>
#include <sstream>
#include <cassert>

#include "lexer.h"

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

MalNumber::MalNumber(int number)
    : m_number(number)
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

MalContainer::MalContainer(ContainerType type)
    : m_type(type)
{
}

std::string MalContainer::asString() const
{
    std::stringstream ss;
    ss << (m_type == ContainerType::LIST ? '(' : '[');
    for (const auto& obj : m_data) {
        ss << obj->asString() << (m_data.back() != obj ? " " : "");
    }
    ss << (m_type == ContainerType::LIST ? ')' : ']');
    return ss.str();
}

void MalContainer::append(std::unique_ptr<MalType> element)
{
    m_data.push_back(std::move(element));
}

std::vector<std::unique_ptr<MalType>>::iterator MalContainer::begin()
{
    return m_data.begin();
}

std::vector<std::unique_ptr<MalType>>::iterator MalContainer::end()
{
    return m_data.end();
}

bool MalContainer::isEmpty() const
{
    return m_data.empty();
}

MalContainer::ContainerType MalContainer::type() const 
{
    return m_type;
}

MalType* MalContainer::first() const 
{
    assert(m_data.size());
    return m_data[0].get();
}

MalList::MalList()
    : MalContainer(MalContainer::ContainerType::LIST)
{
}

MalVector::MalVector()
    : MalContainer(MalContainer::ContainerType::VECTOR)
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
    for (auto it = m_hashMap.begin(); it != m_hashMap.end(); ++it) {
        const auto& [key, value] = *it;
        ss << key << " " << value->asString() << (std::distance(it, m_hashMap.end()) > 1 ? " " : "");
    }
    ss << "}";
    return ss.str();
}

void MalHashMap::insert(const std::string& key, std::unique_ptr<MalType> value)
{
    m_hashMap.insert({ key, std::move(value) });
}

MalHashMap::HashMapIteraotr MalHashMap::begin() 
{
    return m_hashMap.begin();
}

MalHashMap::HashMapIteraotr MalHashMap::end()
{
    return m_hashMap.end();
}

MalOp::MalOp(TokenType type)
{
    switch (type) {
    case TokenType::PLUS:
        m_op = '+';
        break;
    case TokenType::MINUS:
        m_op = '-';
        break;
    case TokenType::MULT:
        m_op = '*';
        break;
    case TokenType::DIVIDE:
        m_op = '/';
        break;
    default:
        std::cout << "No such operation: " << type;
        assert(false);
    }
}

std::string MalOp::asString() const 
{
    return std::string(1, m_op);
}

char MalOp::getOp() const
{
    return m_op;
}

MalError::MalError(const std::string& message)
    : m_message(message)
{
}

std::string MalError::asString() const
{
    return "ERROR: " + m_message;
}
} // namespace mal