#pragma once
#include <memory>

namespace mal {
class MalType;
class MalContainer;
class Env;

std::shared_ptr<MalType> prn(MalContainer* args);
std::shared_ptr<MalType> list(MalContainer* args);
std::shared_ptr<MalType> isList(MalContainer* args);
std::shared_ptr<MalType> isEmpty(MalContainer* args);
std::shared_ptr<MalType> count(MalContainer* args);
std::shared_ptr<MalType> equal(MalContainer* args);
std::shared_ptr<MalType> less(MalContainer* args);
std::shared_ptr<MalType> lessEqual(MalContainer* args);
std::shared_ptr<MalType> greater(MalContainer* args);
std::shared_ptr<MalType> greaterEqual(MalContainer* args);
std::shared_ptr<MalType> malNot(MalContainer* args);
std::shared_ptr<MalType> plus(MalContainer* args);
std::shared_ptr<MalType> minus(MalContainer* args);
std::shared_ptr<MalType> divides(MalContainer* args);
std::shared_ptr<MalType> multiplies(MalContainer* args);
std::shared_ptr<MalType> readString(MalContainer* args, Env&);
std::shared_ptr<MalType> slurp(MalContainer* args, Env&);
std::shared_ptr<MalType> eval(MalContainer* args, Env& env);
} // mal