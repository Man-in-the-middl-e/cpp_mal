#include "maltypes.h"

#include <cassert>
#include <iostream>
#include <sstream>

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

MalNumber::MalNumber(std::string_view number)
    : m_number(std::atoi(number.data()))
{
}

MalNumber::MalNumber(int number)
    : m_number(number)
{
}

MalNumber* MalNumber::asMalNumber()
{
    return this;
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

MalContainer::MalContainer(const std::vector<std::shared_ptr<MalType>>& data, MalContainer::ContainerType type)
    : m_data(data)
    , m_type(type)
{
}

MalContainer* MalContainer::asMalContainer()
{
    return this;
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

void MalContainer::append(std::shared_ptr<MalType> element)
{
    m_data.push_back(element);
}

std::vector<std::shared_ptr<MalType>>::iterator MalContainer::begin()
{
    return m_data.begin();
}

std::vector<std::shared_ptr<MalType>>::iterator MalContainer::end()
{
    return m_data.end();
}

bool MalContainer::isEmpty() const
{
    return m_data.empty();
}

size_t MalContainer::size() const
{
    return m_data.size();
}

MalContainer::ContainerType MalContainer::type() const
{
    return m_type;
}

std::shared_ptr<MalType> MalContainer::at(size_t index) const
{
    return m_data[index];
}

std::shared_ptr<MalType> MalContainer::back() const 
{
    return m_data.back();
}


std::shared_ptr<MalType> MalContainer::head() const
{
    return m_data.at(0);
}

std::shared_ptr<MalContainer> MalContainer::tail() 
{
    if (m_data.begin() + 1 != m_data.end()) {
        const std::vector<std::shared_ptr<MalType>> newData(m_data.begin() + 1, m_data.end());
        m_data = newData;
    } else {
        m_data.clear();
    }
    return std::make_shared<MalContainer>(m_data, m_type);
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

MalSymbol* MalSymbol::asMalSymbol()
{
    return this;
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

MalString* MalString::asMalString()
{
    return this;
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

MalHashMap* MalHashMap::asMalHashMap()
{
    return this;
}

void MalHashMap::insert(const std::string& key, std::shared_ptr<MalType> value)
{
    m_hashMap.insert({ key, value });
}

MalHashMap::HashMapIteraotr MalHashMap::begin()
{
    return m_hashMap.begin();
}

MalHashMap::HashMapIteraotr MalHashMap::end()
{
    return m_hashMap.end();
}

MalOp::MalOp(char op)
    : m_opType(op)
{
    switch (m_opType) {
    case '+':
        m_op = std::plus<int>();
        break;
    case '-':
        m_op = std::minus<int>();
        break;
    case '*':
        m_op = std::multiplies<int>();
        break;
    case '/':
        m_op = std::divides<int>();
        break;
    default:
        std::cout << "No such operation: " << m_opType;
        assert(false);
    }
}

std::string MalOp::asString() const
{
    return std::string(1, m_opType);
}

MalOp* MalOp::asMalOp()
{
    return this;
}

std::shared_ptr<MalType> MalOp::operator()(const MalContainer* arguments)
{
    if (arguments->isEmpty() || arguments->size() == 1) {
        return std::make_unique<MalError>("Not enough arguments");
    }
    if (const auto baseNumber = arguments->head()->asMalNumber(); !baseNumber){
        return std::make_unique<MalError>("Couldn't apply arithmetic operation to not a number");
    } else {
        int res = baseNumber->getValue();
        for (size_t i = 1; i < arguments->size(); ++i) {
            const auto currentNumber = arguments->at(i)->asMalNumber()->getValue();
            res = m_op(res, currentNumber);
        }
        return std::make_shared<MalNumber>(res);
    }
}

MalError::MalError(const std::string& message)
    : m_message(message)
{
}

std::string MalError::asString() const
{
    return "ERROR: " + m_message;
}

MalError* MalError::asMalError() 
{
    return this;
}

MalFunction::MalFunction() { }

std::string MalFunction::asString() const
{
    return "#<function>";
}

MalFunction* MalFunction::asMalFunction() 
{
    return this;
}

} // namespace mal