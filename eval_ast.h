#pragma once

#include <cassert>

#include "env.h"
#include "maltypes.h"

namespace mal {

std::shared_ptr<MalType> eval_ast(std::shared_ptr<MalType> ast,EnvInterface& env);
std::shared_ptr<MalType> EVAL(std::shared_ptr<MalType> ast, EnvInterface& env);

std::shared_ptr<MalType> applyFunc(std::shared_ptr<MalType> ast, EnvInterface& env)
{
    const auto ls = ast->asMalContainer();
    const auto functionParameters = ls->at(1);
    const auto functionBody = ls->at(2);
    if (env.isGlobalEnv()){
        return std::make_shared<MalClosure>(functionParameters, functionBody);
    }
    return std::make_shared<MalClosure>(functionParameters, functionBody, static_cast<FunctionEnv&>(env));
}

std::shared_ptr<MalType> applyDo(std::shared_ptr<MalType> ast, EnvInterface& env)
{
    if (auto ls = ast->asMalContainer(); ls->size() == 1) {
        return std::make_unique<MalError>("not enough arguments");
    } else {
        auto evaluatedList = eval_ast(ls->tail(), env);
        return evaluatedList->asMalContainer()->back();
    }
}

// (if (cond) (ture branch) (optinal false branch))
std::shared_ptr<MalType> applyIf(std::shared_ptr<MalType> ast, EnvInterface& env)
{
    auto ls = ast->asMalContainer();

    // if - 1, cond - 1, tureBranch - 1 = 1 + 1 + 1 = 3
    const size_t numberOfArguments = 3;
    if (ls->size() < numberOfArguments) {
        return std::make_unique<MalError>("Not enough arguments for if statement");
    }

    const auto ifCondition = ls->at(1);
    const auto res = EVAL(ifCondition, env);

    // NOTE: probaly doing type conversion is faster than creating and comparing strings
    if (auto resStr = res->asString(); resStr != "nil" && resStr != "false") {
        const auto trueBranch = ls->at(2);
        return EVAL(trueBranch, env);
    } else if (ls->size() > numberOfArguments) {
        const auto falseBranch = ls->at(3);
        return EVAL(falseBranch, env);
    }
    return std::make_shared<MalNil>();
}

std::shared_ptr<MalType> applyLet(std::shared_ptr<MalType> ast)
{
    auto malContainer = ast->asMalContainer();
    assert(malContainer->at(0)->asString() == "let*");

    FunctionEnv letEnv;
    auto letArguments = malContainer->at(1)->asMalContainer();

    // EXAMPLE: (let* (p (+ 2 3) q (+ 2 p)) (+ p q))
    for (size_t i = 0; i < letArguments->size(); i += 2) {
        letEnv.set(letArguments->at(i)->asString(), EVAL(letArguments->at(i + 1), letEnv));
    }

    return EVAL(malContainer->at(2), letEnv);
}

std::shared_ptr<MalType> applyDef(std::shared_ptr<MalType> ast, EnvInterface& env)
{
    const auto malContainer = ast->asMalContainer();
    assert(malContainer->at(0)->asString() == "def!");

    if (malContainer->size() <= 2) {
        return std::make_unique<MalError>("Not enough arguments");
    }

    const auto envName = malContainer->at(1)->asString();
    const auto envArguments = EVAL(malContainer->at(2), env);

    if (!envArguments->asMalError()) {
        env.set(envName, envArguments);
    }
    return envArguments;
}

std::shared_ptr<MalType> applyOp(std::shared_ptr<MalType> ast)
{
    const auto list = ast->asMalContainer();
    if (const auto op = list->head()->asMalOp(); op) {
        return op->operator()(list->tail().get());
    }
    return ast;
}

std::shared_ptr<MalType> EVAL(std::shared_ptr<MalType> ast, EnvInterface& env)
{
    if (const auto container = ast->asMalContainer(); container) {
        if (container->isEmpty()) {
            return ast;
        }

        if (const auto symbolStr = container->at(0)->asString(); symbolStr == "def!") {
            return applyDef(ast, env);
        } else if (symbolStr == "let*") {
            return applyLet(ast);
        } else if (symbolStr == "if") {
            return applyIf(ast, env);
        } else if (symbolStr == "do"){
            return applyDo(ast, env);
        } else if (symbolStr == "fn*"){
            return applyFunc(ast, env);
        }

        const auto evaluatedList = eval_ast(ast, env);
        if (auto ls = evaluatedList->asMalContainer(); ls && !ls->isEmpty()) {
            const auto head = ls->head();
            if (const auto closure = head->asMalClosure(); closure){
                return closure->operator()(ls->tail().get());
            }
            if (const auto func = head->asMalCallable(); func) {
                const auto parameters = evaluatedList->asMalContainer()->tail();
                return func->operator()(parameters.get());
            } else if (head->asMalOp()) {
                return applyOp(evaluatedList);
            }
        }
        return evaluatedList;
    }
    return eval_ast(ast, env);
}

std::shared_ptr<MalType> eval_ast(std::shared_ptr<MalType> ast, EnvInterface& env)
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
        if (!relatedEnv) {
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
