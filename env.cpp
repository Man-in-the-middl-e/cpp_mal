#include "env.h"
#include "maltypes.h"

#include <iostream>

namespace mal {

Env::Env(const Env& parentEvn)
{
    for (const auto& [key, value] : parentEvn.m_data) {
        if (auto itElem = m_data.find(key); itElem == m_data.end()) {
            m_data.insert({key, value});
        }
    }
    m_parentEnv = &parentEvn;
}

Env::Env(const Env*  parentEnv) : m_parentEnv(parentEnv) {
    // TODO: move it to setUpBuildins()
    m_data.insert({ "+", std::make_shared<MalOp>('+') });
    m_data.insert({ "-", std::make_shared<MalOp>('-') });
    m_data.insert({ "*", std::make_shared<MalOp>('*') });
    m_data.insert({ "/", std::make_shared<MalOp>('/') });
}

void Env::set(const std::string& key, std::shared_ptr<MalType> value)
{
    m_data[key] = value;
}

std::shared_ptr<MalType> Env::find(const std::string& key) const
{
    if (auto env = m_data.find(key); env != m_data.end()) {
        auto& [k, relatedEnv] = *env;
        return relatedEnv;
    }

    if (m_parentEnv){
        return m_parentEnv->find(key);
    }

    return nullptr;
}

void Env::setBindings(const MalContainer* parameters, const MalContainer* arguments)
{
    for (size_t parameterIndex = 0; parameterIndex < parameters->size(); ++parameterIndex) {
        const auto currentParameter = parameters->at(parameterIndex)->asString();
    
        // (& paramName) bound name to all arguments that left
        // (fn* (a & paramName) (+ a count paramName))(1 2 3) -> a = 1 paramName = (2, 3)
        if (currentParameter == "&") {
            auto allOtherArgs = std::make_shared<MalList>();
            for (size_t vaArgs = parameterIndex; vaArgs < arguments->size(); ++vaArgs){
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

} // namespace mal
