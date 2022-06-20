#pragma once

#include <memory>
#include <unordered_map>

namespace mal {
class MalCallable;
class MalType;
class MalContainer;

class GlobalEnv {
public:
    static GlobalEnv& the();
    std::shared_ptr<MalType> find(const std::string& key) const;

private:
    GlobalEnv();

private:
    std::unordered_map<std::string, std::shared_ptr<MalCallable>> m_buildins;
};

class Env {
public:
    Env() = default;
    Env(Env* parentEnv);

    void set(const std::string& key, std::shared_ptr<MalType> value);
    std::shared_ptr<MalType> find(const std::string& key) const;
    void setBindings(const MalContainer* binds, const MalContainer* exprs);
    void addToEnv(Env& newEnv);
    bool isEmpty() const;

private:
    std::unordered_map<std::string, std::shared_ptr<MalType>> m_data;
    Env* parentEnv = nullptr;
};

} // namespace mal
