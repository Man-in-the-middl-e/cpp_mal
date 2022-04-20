#include "env.h"
#include "maltypes.h"

#include <iostream>

namespace mal {

EnvInterface::~EnvInterface()
{
}

GlobalEnv::GlobalEnv()
{
    m_data.insert({ "+", std::make_shared<MalOp>('+') });
    m_data.insert({ "-", std::make_shared<MalOp>('-') });
    m_data.insert({ "*", std::make_shared<MalOp>('*') });
    m_data.insert({ "/", std::make_shared<MalOp>('/') });
}

void GlobalEnv::set(const std::string& key, std::shared_ptr<MalType> value)
{
    m_data[key] = value;
}

std::shared_ptr<MalType> GlobalEnv::find(const std::string& key) const
{
    if (auto env = m_data.find(key); env != m_data.end()) {
        auto& [k, relatedEnv] = *env;
        return relatedEnv;
    }
    return nullptr;
}

FunctionEnv::FunctionEnv(const FunctionEnv& newEnv)
{
    m_data = static_cast<const FunctionEnv&>(newEnv).m_data;
}

void FunctionEnv::setBindings(const MalContainer* binds, const MalContainer* exprs)
{
    if (binds && exprs && binds->size() == exprs->size()) {
        for (size_t argCount = 0; argCount < binds->size(); ++argCount) {
            const auto currentArgument = binds->at(argCount)->asString();
            const auto boundedValue = exprs->at(argCount);
            set(currentArgument, boundedValue);
        }
    }
}

std::shared_ptr<MalType> FunctionEnv::find(const std::string& key) const
{
    if (auto env = m_data.find(key); env != m_data.end()) {
        auto& [k, relatedEnv] = *env;
        return relatedEnv;
    }

    if (auto env = GlobalEnv::instance().find(key); env != nullptr) {
        return env;
    }

    return nullptr;
}

void FunctionEnv::set(const std::string& key, std::shared_ptr<MalType> value)
{
    m_data[key] = value;
}

bool FunctionEnv::isGlobalEnv() const
{
    return false;
}
} // namespace mal
