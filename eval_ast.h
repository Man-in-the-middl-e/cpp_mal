#pragma once

#include <cassert>

#include "env.h"
#include "maltypes.h"

namespace mal {
std::shared_ptr<MalType> apply(std::shared_ptr<MalType> ast)
{
    auto malContainer = ast->asMalContainer();
    if (malContainer->size() <= 2) {
        return std::make_unique<MalError>("Not enough arguments");
    }

    if (auto firstElement = malContainer->at(0).get(); !firstElement->asMalOp()) {
        if (firstElement->asMalNumber()) {
            return ast;
        }
        return std::make_unique<MalError>("Operation is not supported");
    }else {
        auto op = firstElement->asMalOp();
        MalNumber res(malContainer->at(1)->asMalNumber()->getValue());
        for (size_t i = 2; i < malContainer->size(); ++i) {
            res = op->applyOp(res, *malContainer->at(i)->asMalNumber());
        }
        return std::make_shared<MalNumber>(res.getValue());
    }
    return ast;
}

std::shared_ptr<MalType> eval_ast(std::shared_ptr<MalType> ast, const Env& env);

std::shared_ptr<MalType> EVAL(std::shared_ptr<MalType> ast, const Env& env)
{
    if (auto container = ast->asMalContainer(); container) {
        if (container->isEmpty()) {
            return ast;
        }
        return apply(eval_ast(ast, env));
    }
    return eval_ast(ast, env);
}

std::shared_ptr<MalType> eval_ast(std::shared_ptr<MalType> ast, const Env& env)
{
    if (auto container = ast->asMalContainer(); container) {
        if (container->isEmpty()) {
            return ast;
        }
        auto newContainer = std::make_shared<MalContainer>(container->type());
        for (auto& element : *container) {
            newContainer->append(EVAL(element, env));
        }
        return newContainer;
    } else if (auto symbol = ast->asMalSymbol(); symbol) {
        return env.find(*symbol);
    } else if (auto hashMap = ast->asMalHashMap(); hashMap) {
        auto newHashMap = std::make_shared<MalHashMap>();
        for (auto& [key, value] : *hashMap) {
            newHashMap->insert(key, EVAL(value, env));
        }
        return newHashMap;
    }
    return ast;
}
} // namespace mal
