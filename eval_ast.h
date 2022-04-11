#pragma once

#include <cassert>

#include "env.h"
#include "maltypes.h"

namespace mal {
std::unique_ptr<MalType> apply(std::unique_ptr<MalType> ast)
{
    auto malContainer = ast->asMalContainer();
    assert(malContainer->size() != 0);

    if (auto firstElem = malContainer->first(); !firstElem->asMalOp()) {
        if (firstElem->asMalNumber()) {
            return ast;
        }
        return std::make_unique<MalError>("Operation is not supported");
    } else {
        int res = 0;
        for (auto it = malContainer->begin(); it != malContainer->end(); ++it) {
            // assume that first element is operation
            if (std::distance(malContainer->begin(), it) == 0) {
                continue;
            }

            const auto currentValue = (*it)->asMalNumber()->getValue();
            if (std::distance(malContainer->begin(), it) == 1) {
                res = currentValue;
                continue;
            }
            switch (firstElem->asMalOp()->getOp()) {
            case '+':
                res += currentValue;
                break;
            case '-':
                res -= currentValue;
                break;
            case '*':
                res *= currentValue;
                break;
            case '/':
                res /= currentValue;
                break;
            }
        }
        return std::make_unique<MalNumber>(res);
    }
    return ast;
}

std::unique_ptr<MalType> eval_ast(std::unique_ptr<MalType> ast, const Env& env);

std::unique_ptr<MalType> EVAL(std::unique_ptr<MalType> ast, const Env& env)
{
    if (auto container = ast->asMalContainer(); container) {
        if (container->isEmpty()) {
            return ast;
        }
        return apply(eval_ast(std::move(ast), env));
    }
    return eval_ast(std::move(ast), env);
}

std::unique_ptr<MalType> eval_ast(std::unique_ptr<MalType> ast, const Env& env)
{
    if (auto container = ast->asMalContainer(); container) {
        if (container->isEmpty()) {
            return ast;
        }
        auto newContainer = std::make_unique<MalContainer>(container->type());
        for (auto& element : *container) {
            newContainer->append(EVAL(std::move(element), env));
        }
        return newContainer;
    } else if (auto symbol = ast->asMalSymbol(); symbol) {
        return env.find(*symbol);
    } else if (auto hashMap = ast->asMalHashMap(); hashMap) {
        auto newHashMap = std::make_unique<MalHashMap>();
        for (auto& [key, value] : *hashMap) {
            newHashMap->insert(key, EVAL(std::move(value), env));
        }
        return newHashMap;
    }
    return ast;
}
} // namespace mal
