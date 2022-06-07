#include "eval_ast.h"

#include "maltypes.h"

namespace mal {

std::shared_ptr<MalType> applyFunc(const MalContainer* ls, Env& env)
{
    const auto functionParameters = ls->at(1);
    const auto functionBody = ls->at(2);
    return std::make_shared<MalClosure>(functionParameters, functionBody, env);
}

std::shared_ptr<MalType> applyDo(MalContainer* ls, Env& env)
{
    if (ls->size() == 1) {
        return std::make_unique<MalError>("not enough arguments");
    } else {
        auto evaluatedList = eval_ast(ls->tail(), env);
        return evaluatedList->asMalContainer()->back();
    }
}

// (if (cond) (ture branch) (optinal false branch))
std::shared_ptr<MalType> applyIf(const MalContainer* ls, Env& env)
{
    // if - 1, cond - 1, tureBranch - 1 = 1 + 1 + 1 = 3
    const size_t numberOfArguments = 3;
    if (ls->size() < numberOfArguments) {
        return std::make_unique<MalError>("Not enough arguments for if statement");
    }

    const auto ifCondition = ls->at(1);
    const auto res = EVAL(ifCondition, env);

    if (auto resStr = res->asString(); resStr != "nil" && resStr != "false") {
        const auto trueBranch = ls->at(2);
        return EVAL(trueBranch, env);
    } else if (ls->size() > numberOfArguments) {
        const auto falseBranch = ls->at(3);
        return EVAL(falseBranch, env);
    }
    return std::make_shared<MalNil>();
}

std::shared_ptr<MalType> applyLet(const MalContainer* ls, const Env& env)
{
    Env letEnv(env);
    auto letArguments = ls->at(1)->asMalContainer();

    // EXAMPLE: (let* (p (+ 2 3) q (+ 2 p)) (+ p q))
    for (size_t i = 0; i < letArguments->size(); i += 2) {
        letEnv.set(letArguments->at(i)->asString(), EVAL(letArguments->at(i + 1), letEnv));
    }

    return EVAL(ls->at(2), letEnv);
}

std::shared_ptr<MalType> applyDef(const MalContainer* ls, Env& env)
{
    if (ls->size() <= 2) {
        return std::make_unique<MalError>("Not enough arguments");
    }

    const auto envName = ls->at(1)->asString();
    const auto envArguments = EVAL(ls->at(2), env);

    if (!envArguments->asMalError()) {
        env.set(envName, envArguments);
    }
    return envArguments;
}

std::shared_ptr<MalType> EVAL(std::shared_ptr<MalType> ast, Env& env)
{
    if (const auto container = ast->asMalContainer(); container) {
        if (container->isEmpty()) {
            return ast;
        }

        if (const auto symbolStr = container->at(0)->asString(); symbolStr == "def!") {
            return applyDef(container, env);
        } else if (symbolStr == "let*") {
            return applyLet(container, env);
        } else if (symbolStr == "if") {
            return applyIf(container, env);
        } else if (symbolStr == "do") {
            return applyDo(container, env);
        } else if (symbolStr == "fn*") {
            return applyFunc(container, env);
        }

        const auto evaluatedList = eval_ast(ast, env);
        if (auto ls = evaluatedList->asMalContainer(); ls && !ls->isEmpty()) {
            const auto head = ls->head();
            if (const auto closure = head->asMalClosure(); closure) {
                return closure->apply(ls->tail().get());
            }
            else if (const auto func = head->asMalCallable(); func) {
                const auto parameters = evaluatedList->asMalContainer()->tail();
                return func->apply(parameters.get());
            }
        }
        return evaluatedList;
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
