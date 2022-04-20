#pragma once

#include <memory>
#include <unordered_map>

namespace mal {
class MalType;
class MalContainer;

class EnvInterface {
public:
    virtual void set(const std::string& key, std::shared_ptr<MalType> value) = 0;
    virtual std::shared_ptr<MalType> find(const std::string& key) const = 0;
    virtual bool isGlobalEnv() const { return true; }
    virtual ~EnvInterface();
};

class GlobalEnv : public EnvInterface {
public:
    void set(const std::string& key, std::shared_ptr<MalType> value) override;
    std::shared_ptr<MalType> find(const std::string& key) const override;
    static GlobalEnv& instance()
    {
        static GlobalEnv env;
        return env;
    }

private:
    GlobalEnv();

private:
    std::unordered_map<std::string, std::shared_ptr<MalType>> m_data;
};

class FunctionEnv : public EnvInterface {
public:
    FunctionEnv() = default;
    FunctionEnv(const FunctionEnv& newEnv);

    void setBindings(const MalContainer* binds, const MalContainer* exprs);

    std::shared_ptr<MalType> find(const std::string& key) const override;
    void set(const std::string& key, std::shared_ptr<MalType> value) override;
    bool isGlobalEnv() const override;

private:
    std::unordered_map<std::string, std::shared_ptr<MalType>> m_data;
};

} // namespace mal
