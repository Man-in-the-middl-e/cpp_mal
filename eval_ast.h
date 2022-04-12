#pragma once

#include <cassert>

#include "env.h"
#include "maltypes.h"

namespace mal {
std::shared_ptr<MalType> apply(std::shared_ptr<MalType> ast)
{
    auto malContainer = ast->asMalContainer();
    assert(malContainer->size() != 0);

    if (auto firstElem = malContainer->first(); !firstElem->asMalOp()) {
        if (firstElem->asMalNumber()) {
            return ast;
        }
        return std::make_shared<MalError>("Operation is not supported");
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
        return std::make_shared<MalNumber>(res);
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
