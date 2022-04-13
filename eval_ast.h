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

std::shared_ptr<MalType> apply(std::shared_ptr<MalType> ast)
{
    if (ast->asMalError()){
        return ast;
    }
    const auto malContainer = ast->asMalContainer();

    if (const auto firstElement = malContainer->at(0).get(); !firstElement->asMalOp()) {
        if (firstElement->asMalNumber()) {
            return ast;
        }
        return std::make_unique<MalError>("Operation is not supported");
    } else {
        const auto op = firstElement->asMalOp();
        MalNumber res(malContainer->at(1)->asMalNumber()->getValue());
        for (size_t i = 2; i < malContainer->size(); ++i) {
            res = op->applyOp(res, *malContainer->at(i)->asMalNumber());
        }
        return std::make_shared<MalNumber>(res.getValue());
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
        return apply(eval_ast(ast, env));
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
