#pragma once

#include <unordered_map>

#include "maltypes.h"

namespace mal {

class Env {
public:
    Env(const Env* outerEnv = nullptr);
    void set(const std::string& key, std::shared_ptr<MalType> value);
    std::shared_ptr<MalType> find(const std::string& key) const;

private:
    const Env* m_outerEnv;
    std::unordered_map<std::string, std::shared_ptr<MalType>> m_data;
};

std::shared_ptr<MalType> eval_ast(std::shared_ptr<MalType> ast,Env& env);
std::shared_ptr<MalType> apply(std::shared_ptr<MalType> ast);
std::shared_ptr<MalType> EVAL(std::shared_ptr<MalType> ast, Env& env);

} // namespace mal
