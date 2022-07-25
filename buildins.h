#pragma once
#include <memory>

namespace mal {
class MalType;
class MalContainer;
class Env;

std::shared_ptr<MalType> prn(MalContainer* args);
std::shared_ptr<MalType> printString(MalContainer* args);
std::shared_ptr<MalType> str(MalContainer* args);
std::shared_ptr<MalType> println(MalContainer* args);
std::shared_ptr<MalType> list(MalContainer* args);
std::shared_ptr<MalType> isList(MalContainer* args);
std::shared_ptr<MalType> isEmpty(MalContainer* args);
std::shared_ptr<MalType> isAtom(MalContainer *args);
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
std::shared_ptr<MalType> readString(MalType* args, Env& env);
std::shared_ptr<MalType> slurp(MalContainer* args, Env& env);
std::shared_ptr<MalType> eval(MalContainer* args, Env& env);
std::shared_ptr<MalType> loadFile(MalContainer* args, Env& env);
std::shared_ptr<MalType> deref(MalContainer* args);
std::shared_ptr<MalType> argv(MalContainer *args);
std::shared_ptr<MalType> cons(MalContainer *args);
std::shared_ptr<MalType> concat(MalContainer *args);
std::shared_ptr<MalType> vec(MalContainer* args);
std::shared_ptr<MalType> nth(MalContainer* args);
std::shared_ptr<MalType> first(MalContainer* args);
std::shared_ptr<MalType> rest(MalContainer* args);
std::shared_ptr<MalType> cond(MalContainer* args);
std::shared_ptr<MalType> malThrow(MalContainer* args);
std::shared_ptr<MalType> apply(MalContainer* args, Env& env);
std::shared_ptr<MalType> map(MalContainer* args, Env& env);
std::shared_ptr<MalType> isNil(MalContainer* args);
std::shared_ptr<MalType> isSymbol(MalContainer* args);
std::shared_ptr<MalType> isTrue(MalContainer* args);
std::shared_ptr<MalType> isFalse(MalContainer* args);
std::shared_ptr<MalType> isVector(MalContainer* args);
std::shared_ptr<MalType> isSequential(MalContainer* args);
std::shared_ptr<MalType> isMap(MalContainer* args);
std::shared_ptr<MalType> isKeyword(MalContainer* args);
std::shared_ptr<MalType> makeKeyword(MalContainer* args);
std::shared_ptr<MalType> makeSymbol(MalContainer* args);
std::shared_ptr<MalType> makeVector(MalContainer* args);
std::shared_ptr<MalType> makeHashMap(MalContainer* args);
std::shared_ptr<MalType> assoc(MalContainer* args);
std::shared_ptr<MalType> dissoc(MalContainer* args);
std::shared_ptr<MalType> malGet(MalContainer* args);
std::shared_ptr<MalType> contains(MalContainer* args);
std::shared_ptr<MalType> keys(MalContainer* args);
std::shared_ptr<MalType> vals(MalContainer* args);
std::shared_ptr<MalType> malReadline(MalContainer* args);
std::shared_ptr<MalType> hostLanguage(MalContainer* args);
std::shared_ptr<MalType> meta(MalContainer* args);
std::shared_ptr<MalType> withMeta(MalContainer* args);
} // mal
