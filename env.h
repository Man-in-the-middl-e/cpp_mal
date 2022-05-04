#pragma once

#include <memory>
#include <unordered_map>

namespace mal {
class MalType;
class MalContainer;

class Env {
public:
    Env(const Env* parentEnv = nullptr);
    Env(const Env& parentEnv);

    void set(const std::string& key, std::shared_ptr<MalType> value);
    std::shared_ptr<MalType> find(const std::string& key) const;
    void setBindings(const MalContainer* binds, const MalContainer* exprs);

private:
    std::unordered_map<std::string, std::shared_ptr<MalType>> m_data;
    const Env* m_parentEnv;
};

} // namespace mal
