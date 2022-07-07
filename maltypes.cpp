#include "maltypes.h"

#include "eval_ast.h"
#include "lexer.h"

#include <cassert>
#include <iostream>
#include <sstream>

namespace mal {

MalAtom::MalAtom(std::shared_ptr<MalType> malType, const std::string& atomDesripton)
    : m_underlyingType(malType)
    , m_atomDescripton(atomDesripton)
{
}

std::string MalAtom::asString() const
{
    return m_atomDescripton;
}

MalAtom* MalAtom::asMalAtom()
{
    return this;
}

std::shared_ptr<MalType> MalAtom::reset(std::shared_ptr<MalType> newType)
{
    m_underlyingType = newType;
    return m_underlyingType;
}

std::shared_ptr<MalType> MalAtom::deref() const
{
    return m_underlyingType;
}

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
    if (m_data.size() == 0) {
        // TODO: maybe thorw error here
        return std::make_shared<MalContainer>(m_type);
    }
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
    : m_malString(str)
{
}

std::string MalString::asString() const
{
    return m_malString;
}

MalString* MalString::asMalString()
{
    return this;
}

std::string MalString::escapeString(const std::string& str)
{
    std::stringstream ss;
    for (size_t i = 0; i < str.size(); ++i) {
        const char currentChar = str[i];
        if (currentChar == '\\') {
            ss << "\\";
        } else if (currentChar == '"') {
            ss << "\\";
        } else if (currentChar == '\n') {
            ss << '\\' << 'n';
            continue;
        }
        ss << currentChar;
    }
    return ss.str();
}

std::string MalString::unEscapeString(const std::string& str)
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

bool MalString::isEmpty() const
{
    return m_malString.empty();
}

std::string MalNil::asString() const
{
    return "nil";
}

MalNil* MalNil::asMalNil()
{
    return this;
}

MalBoolean::MalBoolean(bool value)
    : m_boolValue(value)
{
}

MalBoolean::MalBoolean(std::string_view value)
    : m_boolValue(value == "true")
{
}

std::string MalBoolean::asString() const
{
    return m_boolValue ? "true" : "false";
}

MalBoolean* MalBoolean::asMalBoolean()
{
    return this;
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

size_t MalHashMap::size() const
{
    return m_hashMap.size();
}

MalHashMap::HashMapIteraotr MalHashMap::begin()
{
    return m_hashMap.begin();
}

MalHashMap::HashMapIteraotr MalHashMap::end()
{
    return m_hashMap.end();
}

MalHashMap::HashMapIteraotr MalHashMap::find(const std::string& key)
{
    return m_hashMap.find(key);
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

MalClosure::MalClosure(const std::shared_ptr<MalType> parameters, const std::shared_ptr<MalType> body, const Env& env)
    : m_functionParameters(parameters)
    , m_functionBody(body)
    , m_relatedEnv(env)
{
}

std::string MalClosure::asString() const
{
    return "#<function>";
}

MalClosure* MalClosure::asMalClosure()
{
    return this;
}

std::shared_ptr<MalType> MalClosure::apply(const MalContainer* arguments, Env& env)
{
    // NOTE: we can't just make env parent of m_relatedEnv, 
    // because at some point env could become referene to deallocated memory,
    // so we copy it, probably there is a better way to do this
    m_relatedEnv.addToEnv(env);
    Env newEnv(&m_relatedEnv);
    newEnv.setBindings(m_functionParameters->asMalContainer(), arguments);
    auto res = EVAL(m_functionBody, newEnv);
    return res;
}

MalCallable::MalCallable(Callable callable)
    : m_callable(std::move(callable))
{
}

MalCallable::MalCallable(CallableWithEnv callableWithEnv)
    : m_callableWithEnv(std::move(callableWithEnv))
{
}

std::string MalCallable::asString() const
{
    return "callable";
}

MalCallable* MalCallable::asMalCallable()
{
    return this;
}

std::shared_ptr<MalType> MalCallable::apply(MalContainer* args, Env& env) const
{
    if (m_callable) {
        return m_callable(args);
    }
    return m_callableWithEnv(args, env);
}

std::shared_ptr<MalType> MalCallable::apply(MalContainer* args) const
{
    if (m_callable) {
        return m_callable(args);
    }
    return std::make_unique<MalError>("Callable function is not defined");
}

} // namespace mal