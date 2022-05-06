#pragma once

#include <memory>

namespace mal {
class MalType;
class Env;
std::shared_ptr<MalType> EVAL(std::shared_ptr<MalType> ast, Env& env);
std::shared_ptr<MalType> eval_ast(std::shared_ptr<MalType> ast, Env& env);
} // mal
