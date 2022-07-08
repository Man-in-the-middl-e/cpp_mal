#include "eval_ast.h"

#include "maltypes.h"

namespace mal {

std::shared_ptr<MalType> evaluateFunc(const MalContainer* ls, Env& env)
{
    const auto functionParameters = ls->at(1);
    const auto functionBody = ls->at(2);
    return std::make_shared<MalClosure>(functionParameters, functionBody, env);
}

std::shared_ptr<MalType> evaluateDo(MalContainer* ls, Env& env)
{
    if (ls->size() == 1) {
        return std::make_unique<MalError>("not enough arguments");
    } else {
        auto evaluatedList = eval_ast(ls->tail(), env);
        return evaluatedList->asMalContainer()->back();
    }
}

// (if (cond) (ture branch) (optinal false branch))
std::shared_ptr<MalType> evaluateIf(const MalContainer* ls, Env& env)
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

std::shared_ptr<MalType> evaluateLet(const MalContainer* ls, Env& env)
{
    Env letEnv(&env);
    auto letArguments = ls->at(1)->asMalContainer();

    // EXAMPLE: (let* (p (+ 2 3) q (+ 2 p)) (+ p q))
    for (size_t i = 0; i < letArguments->size(); i += 2) {
        letEnv.set(letArguments->at(i)->asString(), EVAL(letArguments->at(i + 1), letEnv));
    }

    return EVAL(ls->at(2), letEnv);
}

std::shared_ptr<MalType> evaluateDef(const MalContainer* ls, Env& env)
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

std::shared_ptr<MalType> evaluateAtom(const MalContainer* ls, Env& env)
{
    if (ls->size() < 2) {
        return std::make_unique<MalError>("Not enough arguments");
    }

    const auto atomValue = EVAL(ls->at(1), env);
    return std::make_shared<MalAtom>(atomValue, ls->asString());
}

std::shared_ptr<MalType> evaluateReset(const MalContainer* ls, Env& env)
{
    if (ls->size() <= 2) {
        return std::make_unique<MalError>("Not enough arguments");
    }

    const auto atomName = ls->at(1)->asString();
    if (const auto maybeAtom = env.find(atomName); !maybeAtom) {
        return std::make_unique<MalError>(atomName + " is not defined");
    } else if (!maybeAtom->asMalAtom()) {
        return std::make_unique<MalError>("This operation could only be applied to atoms");
    } else {
        // NOTE: can you reset new atom with new atom?
        const auto newValue = EVAL(ls->at(2), env);
        maybeAtom->asMalAtom()->reset(newValue);
        return newValue;
    }
}

std::shared_ptr<MalType> evaluateSwap(const MalContainer* ls, Env& env)
{
    if (ls->size() <= 2) {
        return std::make_unique<MalError>("Not enough arguments");
    }

    const auto atomName = ls->at(1)->asString();
    if (const auto maybeAtom = env.find(atomName); !maybeAtom) {
        return std::make_unique<MalError>(atomName + " is not defined");
    } else if (!maybeAtom->asMalAtom()) {
        return std::make_unique<MalError>("This operation could only be applied to atoms");
    } else {
        const auto function = EVAL(ls->at(2), env);
        const auto functionAst = std::make_shared<MalContainer>(ls->type());
        functionAst->append(function);
        functionAst->append(maybeAtom->asMalAtom()->deref());
        for (size_t argIndex = 3; argIndex < ls->size(); ++argIndex) {
            functionAst->append(ls->at(argIndex));
        }
        const auto res = EVAL(functionAst, env);
        maybeAtom->asMalAtom()->reset(res);
        return res;
    }
}

std::shared_ptr<MalType> evaluateQuote(const MalContainer* ast)
{
    if (ast->size() < 2) {
        return std::make_unique<MalError>("Not enough arguments");
    }
    return ast->at(1);
}

std::shared_ptr<MalType> evaluateQuasiQuote(MalContainer* ast, Env& env)
{
    // TODO: factor out this error handling piece of code
    if (ast->size() < 2) {
        return std::make_unique<MalError>("Not enough arguments");
    }
    return EVAL(ast->at(1), env, false);

}

std::shared_ptr<MalType> evaluateUnquote(const MalContainer* ls, Env& env)
{
    if (ls->size() < 2) {
        return std::make_unique<MalError>("Not enough arguments");
    }
    return EVAL(ls->at(1), env);
}

std::shared_ptr<MalType> evaluateSpliceUnQuote(const MalContainer* ls, Env& env)
{
    auto res = evaluateUnquote(ls, env);
    if (!res->asMalContainer()) {
        return res;
    }
    if (res->asMalContainer()->isEmpty()) {
        return std::make_unique<MalError>("Can't splice empyt list");
    }
    // NOTE: if this won't work out in the future then use this in evaluateQuasiQuote() 
    // and delet lookupSymobls from EVLA\eval_ast

    /*    const auto argument = ast->at(1);
    if (!argument->asMalContainer()) {
        return argument;
    }
    
    auto newContainer = std::make_shared<MalContainer>(ast->type());
    for (const auto& elem : *argument->asMalContainer()) {
        if (const auto ls = elem->asMalContainer(); ls && !ls->isEmpty()) {
            const auto evaluatedList = EVAL(elem, env);
            if (ls->at(0)->asString() == "splice-unquote") {
                for (const auto& subElement : *evaluatedList->asMalContainer()) {
                    newContainer->append(subElement);
                }
            } else {
                newContainer->append(evaluatedList);
            }
        } else {
            newContainer->append(elem);
        }
    }
    return newContainer;
    */
    std::string splicedList = res->asString().substr(1, res->asString().size() - 2);
    return std::make_shared<MalSymbol>(splicedList);
}

std::shared_ptr<MalType> EVAL(std::shared_ptr<MalType> ast, Env& env, bool lookupSymoblsInEnv)
{
    if (const auto container = ast->asMalContainer(); container) {
        if (container->isEmpty()) {
            return ast;
        }

        if (const auto symbolStr = container->at(0)->asString(); symbolStr == "def!") {
            return evaluateDef(container, env);
        } else if (symbolStr == "let*") {
            return evaluateLet(container, env);
        } else if (symbolStr == "if") {
            return evaluateIf(container, env);
        } else if (symbolStr == "do") {
            return evaluateDo(container, env);
        } else if (symbolStr == "fn*") {
            return evaluateFunc(container, env);
        } else if (symbolStr == "atom") {
            return evaluateAtom(container, env);
        } else if (symbolStr == "reset!") {
            return evaluateReset(container, env);
        } else if (symbolStr == "swap!") {
            return evaluateSwap(container, env);
        } else if (symbolStr == "quote") {
            return evaluateQuote(container);
        } else if (symbolStr == "quasiquote") {
            return evaluateQuasiQuote(container, env);
        } else if (symbolStr == "unquote") {
            return evaluateUnquote(container, env);
        } else if (symbolStr == "splice-unquote") {
            return evaluateSpliceUnQuote(container, env);
        }

        const auto evaluatedList = eval_ast(ast, env, lookupSymoblsInEnv);
        if (auto ls = evaluatedList->asMalContainer(); ls && !ls->isEmpty()) {
            const auto head = ls->head();
            if (const auto closure = head->asMalClosure(); closure) {
                return closure->evaluate(ls->tail().get(), env);
            }
            else if (const auto func = head->asMalCallable(); func) {
                const auto parameters = ls->tail();
                return func->evaluate(parameters.get(), env);
            }
        }
        return evaluatedList;
    }
    return eval_ast(ast, env, lookupSymoblsInEnv);
}

std::shared_ptr<MalType> eval_ast(std::shared_ptr<MalType> ast, Env& env, bool lookupSymoblsInEnv)
{
    if (const auto container = ast->asMalContainer(); container) {
        if (container->isEmpty()) {
            return ast;
        }
        auto newContainer = std::make_shared<MalContainer>(container->type());
        for (const auto& element : *container) {
            if (const auto evaluatedElement = EVAL(element, env, lookupSymoblsInEnv); evaluatedElement->asMalError()) {
                return evaluatedElement;
            } else {
                newContainer->append(evaluatedElement);
            }
        }
        return newContainer;
    } else if (const auto symbol = ast->asMalSymbol(); symbol) {
        const auto relatedEnv = lookupSymoblsInEnv ? env.find(symbol->asString()) : ast;
        if (!relatedEnv) {
            std::string error = symbol->asString() + " is not defined";
            return std::make_unique<MalError>(error);
        } else if(symbol->asString() == "*ARGV*") {
            // *ARGV* is the only one callable that doesn't require any arguments
            return relatedEnv->asMalCallable()->evaluate(nullptr);
        }
        return relatedEnv;
    } else if (const auto hashMap = ast->asMalHashMap(); hashMap) {
        auto newHashMap = std::make_shared<MalHashMap>();
        for (auto& [key, value] : *hashMap) {
            newHashMap->insert(key, EVAL(value, env, lookupSymoblsInEnv));
        }
        return newHashMap;
    }
    return ast;
}
} // namespace mal
