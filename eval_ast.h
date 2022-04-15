#pragma once

#include <cassert>

#include "env.h"
#include "maltypes.h"

namespace mal {

std::shared_ptr<MalType> applyLet(std::shared_ptr<MalType> ast, Env& env)
{
    auto malContainer = ast->asMalContainer();
    assert(malContainer->at(0)->asString() == "let*");

    Env lentEnv(env);
    auto letArguments = malContainer->at(1)->asMalContainer();

    //EXAMPLE: (let* (p (+ 2 3) q (+ 2 p)) (+ p q))
    for (size_t i = 0; i < letArguments->size(); i += 2) {
        lentEnv.set(letArguments->at(i)->asString(), EVAL(letArguments->at(i+1), lentEnv));
    }

    return EVAL(malContainer->at(2), lentEnv);
}

std::shared_ptr<MalType> applyDef(std::shared_ptr<MalType> ast, Env& env)
{
    const auto malContainer = ast->asMalContainer();
    assert(malContainer->at(0)->asString() == "def!");

    if (malContainer->size() <= 2) {
        return std::make_unique<MalError>("Not enough arguments");
    }

    const auto envName = malContainer->at(1)->asString();
    const auto envArgumanets = EVAL(malContainer->at(2), env);

    if (!envArgumanets->asMalError())
        env.set(envName, envArgumanets);
    return envArgumanets;
}

std::shared_ptr<MalType> applyOp(std::shared_ptr<MalType> ast)
{
    const auto list = ast->asMalContainer();
    if (const auto op = list->head()->asMalOp(); op) {
        return op->operator()(list->tail().get());
    }
    return ast;
}

std::shared_ptr<MalType> EVAL(std::shared_ptr<MalType> ast, Env& env)
{
    if (const auto container = ast->asMalContainer(); container) {
        if (container->isEmpty()) {
            return ast;
        }

        // `def!` and `let*` are special atoms, so we need to evluate the differently
        if (const auto symbolStr = container->at(0)->asString(); symbolStr == "def!") {
            return applyDef(ast, env);
        } else if (symbolStr == "let*") {
            return applyLet(ast, env);
        }
        if (const auto element = eval_ast(ast, env); element->asMalContainer()) {
            return applyOp(element);
        }
    }
    return eval_ast(ast, env);
}

std::shared_ptr<MalType> eval_ast(std::shared_ptr<MalType> ast, Env& env)
{
    if (const auto container = ast->asMalContainer(); container) {
        if (container->isEmpty()) {
            return ast;
        }
        auto newContainer = std::make_shared<MalContainer>(container->type());
        for (const auto& element : *container) {
            if (const auto evaluatedElement = EVAL(element, env); evaluatedElement->asMalError()) {
                return evaluatedElement;
            } else {
                newContainer->append(evaluatedElement);
            }
        }
        return newContainer;
    } else if (const auto symbol = ast->asMalSymbol(); symbol) {
        const auto relatedEnv = env.find(symbol->asString());
        if (!relatedEnv){
            std::string error = symbol->asString() + " is not defined";
            return std::make_unique<MalError>(error);
        }
        return relatedEnv;
    } else if (const auto hashMap = ast->asMalHashMap(); hashMap) {
        auto newHashMap = std::make_shared<MalHashMap>();
        for (auto& [key, value] : *hashMap) {
            newHashMap->insert(key, EVAL(value, env));
        }
        return newHashMap;
    }
    return ast;
}
} // namespace mal
