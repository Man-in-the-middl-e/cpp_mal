#include "env.h"
#include "buildins.h"
#include "maltypes.h"

#include <iostream>

namespace mal {

GlobalEnv::GlobalEnv()
{
    m_buildins = {
        { "prn", std::make_shared<MalBuildin>(prn) },
        { "pr-str", std::make_shared<MalBuildin>(printString) },
        { "str", std::make_shared<MalBuildin>(str) },
        { "println", std::make_shared<MalBuildin>(println) },
        { "list", std::make_shared<MalBuildin>(list) },
        { "vec", std::make_shared<MalBuildin>(vec) },
        { "list?", std::make_shared<MalBuildin>(isList) },
        { "empty?", std::make_shared<MalBuildin>(isEmpty) },
        { "atom?", std::make_shared<MalBuildin>(isAtom) },
        { "nil?", std::make_shared<MalBuildin>(isNil) },
        { "symbol?", std::make_shared<MalBuildin>(isSymbol) },
        { "true?", std::make_shared<MalBuildin>(isTrue) },
        { "false?", std::make_shared<MalBuildin>(isFalse) },
        { "vector?", std::make_shared<MalBuildin>(isVector) },
        { "sequential?", std::make_shared<MalBuildin>(isSequential) },
        { "map?", std::make_shared<MalBuildin>(isMap) },
        { "keyword?", std::make_shared<MalBuildin>(isKeyword) },
        { "count", std::make_shared<MalBuildin>(count) },
        { "eval", std::make_shared<MalBuildin>(eval) },
        { "read-string", std::make_shared<MalBuildin>(readString) },
        { "slurp", std::make_shared<MalBuildin>(slurp) },
        { "load-file", std::make_shared<MalBuildin>(loadFile) },
        { "not", std::make_shared<MalBuildin>(malNot) },
        { "deref", std::make_shared<MalBuildin>(deref) },
        { "*ARGV*", std::make_shared<MalBuildin>(argv) },
        { "cons", std::make_shared<MalBuildin>(cons) },
        { "concat", std::make_shared<MalBuildin>(concat) },
        { "nth", std::make_shared<MalBuildin>(nth) },
        { "first", std::make_shared<MalBuildin>(first) },
        { "rest", std::make_shared<MalBuildin>(rest) },
        { "cond", std::make_shared<MalBuildin>(cond) },
        { "throw", std::make_shared<MalBuildin>(malThrow) },
        { "apply", std::make_shared<MalBuildin>(apply) },
        { "map", std::make_shared<MalBuildin>(map) },
        { "keyword", std::make_shared<MalBuildin>(makeKeyword) },

        { "=", std::make_shared<MalBuildin>(equal) },
        { "<", std::make_shared<MalBuildin>(less) },
        { "<=", std::make_shared<MalBuildin>(lessEqual) },
        { ">", std::make_shared<MalBuildin>(greater) },
        { ">=", std::make_shared<MalBuildin>(greaterEqual) },
        { "+", std::make_shared<MalBuildin>(plus) },
        { "-", std::make_shared<MalBuildin>(minus) },
        { "/", std::make_shared<MalBuildin>(divides) },
        { "*", std::make_shared<MalBuildin>(multiplies) }
    };
}

GlobalEnv& GlobalEnv::the()
{
    static GlobalEnv env;
    return env;
}

std::shared_ptr<MalType> GlobalEnv::find(const std::string& key) const
{
    if (auto env = m_buildins.find(key); env != m_buildins.end()) {
        auto& [k, relatedEnv] = *env;
        return relatedEnv;
    }
    return nullptr;
}

std::shared_ptr<MalList> GlobalEnv::getArgvs() const
{
    return m_argvs;
}

void GlobalEnv::setUpArgv(int argc, char* argv[])
{
    if (!m_argvs) {
        m_argvs = std::make_shared<MalList>();
        for (int argIndex = 1; argIndex < argc; ++argIndex) {
            m_argvs->append(std::make_shared<MalSymbol>(argv[argIndex]));
        }
    }
}

Env::Env(Env* parentEvn)
{
    parentEnv = parentEvn;
}

Env::Env(const Env& oldEnv)
{
    m_data = oldEnv.m_data;
    parentEnv = nullptr;
}

void Env::set(const std::string& key, std::shared_ptr<MalType> value)
{
    // TODO: check if key is already in  GlobalEnv
    m_data[key] = value;
}

std::shared_ptr<MalType> Env::find(const std::string& key) const
{
    if (auto env = m_data.find(key); env != m_data.end()) {
        auto& [k, relatedEnv] = *env;
        return relatedEnv;
    }
    if (parentEnv) {
        auto res = parentEnv->find(key);
        if (res != nullptr) {
            return res;
        }
    }
    return GlobalEnv::the().find(key);
}

void Env::setBindings(const MalContainer* parameters, const MalContainer* arguments)
{
    for (size_t parameterIndex = 0; parameterIndex < parameters->size(); ++parameterIndex) {
        const auto currentParameter = parameters->at(parameterIndex)->asString();

        // (& paramName) bound name to all arguments that left
        // (fn* (a & paramName) (+ a count paramName))(1 2 3) -> a = 1 paramName = (2, 3)
        if (currentParameter == "&") {
            auto allOtherArgs = std::make_shared<MalList>();
            for (size_t vaArgs = parameterIndex; vaArgs < arguments->size(); ++vaArgs) {
                allOtherArgs->append(arguments->at(vaArgs));
            }
            if (parameterIndex + 1 == parameters->size()) {
                std::cout << "Expected parameter pack name after `&`\n";
                return;
            }
            const auto allOtherArgsName = parameters->at(parameterIndex + 1)->asString();
            set(allOtherArgsName, allOtherArgs);
            return;
        }
        if (parameterIndex < arguments->size()) {
            const auto currentArgument = arguments->at(parameterIndex);
            set(currentParameter, currentArgument);
        }
    }
}

void Env::addToEnv(Env& newEnv)
{
    for (const auto& [key, value] : newEnv.m_data) {
        if (auto itElem = m_data.find(key); itElem == m_data.end()) {
            m_data.insert({ key, value });
        }
    }
}

bool Env::isEmpty() const
{
    return m_data.empty();
}

} // namespace mal
