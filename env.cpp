#include "env.h"
#include "maltypes.h"

namespace mal {

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

void Env::setBindings(const MalContainer* binds, const MalContainer* exprs)
{
    if (binds && exprs && binds->size() == exprs->size()) {
        for (size_t argCount = 0; argCount < binds->size(); ++argCount) {
            const auto currentArgument = binds->at(argCount)->asString();
            const auto boundedValue = exprs->at(argCount);
            set(currentArgument, boundedValue);
        }
    }
}

} // namespace mal
