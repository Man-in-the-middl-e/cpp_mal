#pragma once

#include <memory>
#include <unordered_map>

namespace mal {
class MalCallable;
class MalType;
class MalContainer;
class MalList;

class GlobalEnv {
public:
    static GlobalEnv& the();
    std::shared_ptr<MalType> find(const std::string& key) const;
    std::shared_ptr<MalList> getArgvs() const;
    void setUpArgv(int argc, char* argv[]);

private:
    GlobalEnv();

private:
    std::unordered_map<std::string, std::shared_ptr<MalCallable>> m_buildins;
    std::shared_ptr<MalList> m_argvs;
};

class Env {
public:
    Env() = default;
    Env(Env* parentEnv);
    Env(const Env& newEnv);

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
