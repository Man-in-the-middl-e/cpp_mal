#include "env.h"

#include <iostream>

namespace mal {

Env::Env(const Env* outerEnv)
    : m_outerEnv(outerEnv)
{
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
        auto& [key, relatedEnv] = *env;
        return relatedEnv;
    }
    
    if (m_outerEnv){
        return m_outerEnv->find(key);
    }

    return nullptr;
}

} // namespace mal
