#include "eval_ast.h"

#include "maltypes.h"

#include <assert.h>

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
        return MalException::throwException("not enough arguments");
    } else if (auto result = eval_ast(ls->tail(), env); result->asMalContainer()) {
        return result->asMalContainer()->back();
    } else {
        return result;
    }
}

// (if (cond) (ture branch) (optinal false branch))
std::shared_ptr<MalType> evaluateIf(const MalContainer* ls, Env& env)
{
    // if - 1, cond - 1, tureBranch - 1 = 1 + 1 + 1 = 3
    const size_t numberOfArguments = 3;
    if (ls->size() < numberOfArguments) {
        return MalException::throwException("Not enough arguments for if statement");
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
        return MalException::throwException("Not enough arguments");
    }

    const auto envName = ls->at(1)->asString();
    const auto envArguments = EVAL(ls->at(2), env);

    if (!envArguments->asMalException()) {
        env.set(envName, envArguments);
    }
    return envArguments;
}

std::shared_ptr<MalType> evaluateAtom(const MalContainer* ls, Env& env)
{
    if (ls->size() < 2) {
        return MalException::throwException("Not enough arguments");
    }

    const auto atomValue = EVAL(ls->at(1), env);
    return std::make_shared<MalAtom>(atomValue, ls->asString());
}

std::shared_ptr<MalType> evaluateReset(const MalContainer* ls, Env& env)
{
    if (ls->size() <= 2) {
        return MalException::throwException("Not enough arguments");
    }

    const auto atomName = ls->at(1)->asString();
    if (const auto maybeAtom = env.find(atomName); !maybeAtom) {
        return MalException::throwException(atomName + " is not defined");
    } else if (!maybeAtom->asMalAtom()) {
        return MalException::throwException("This operation could only be applied to atoms");
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
        return MalException::throwException("Not enough arguments");
    }

    const auto atomName = ls->at(1)->asString();
    if (const auto maybeAtom = env.find(atomName); !maybeAtom) {
        return MalException::throwException(atomName + " is not defined");
    } else if (!maybeAtom->asMalAtom()) {
        return MalException::throwException("This operation could only be applied to atoms");
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
        return MalException::throwException("Not enough arguments");
    }
    return ast->at(1);
}

std::shared_ptr<MalType> evaluateQuasiQuoteHelper(std::shared_ptr<MalType> ast, Env& env)
{
    if (auto ls = ast->asMalContainer(); ls) {
        auto resultList = std::make_shared<MalList>();
        if (ls->type() == MalContainer::ContainerType::VECTOR) {
            resultList->append(std::make_shared<MalSymbol>("vec"));
            ls->toList();
            resultList->append(evaluateQuasiQuoteHelper(ast, env));
            return resultList;
        } else if (!ls->isEmpty()) {
            auto firstElemet = ls->at(0);
            if (ls->size() > 1 && firstElemet->asString() == "unquote") {
                return ls->at(1);
            }
            if (auto firstElementAsContainer = firstElemet->asMalContainer(); firstElementAsContainer
                && !firstElementAsContainer->isEmpty()
                && firstElementAsContainer->at(0)->asString() == "splice-unquote") {
                resultList->append(std::make_shared<MalSymbol>("concat"));
                resultList->append(firstElementAsContainer->at(1));
            } else {
                resultList->append(std::make_shared<MalSymbol>("cons"));
                resultList->append(evaluateQuasiQuoteHelper(firstElemet, env));
            }
            resultList->append(evaluateQuasiQuoteHelper(MalContainer::tail(ls), env));
            return resultList;
        }
    } else if (ast->asMalSymbol() || ast->asMalHashMap()) {
        auto list = std::make_shared<MalList>();
        list->append(std::make_shared<MalSymbol>("quote"));
        list->append(ast);
        return list;
    }
    return ast;
}

std::shared_ptr<MalType> evaluateQuasiQuote(MalContainer* ast, Env& env)
{
    // buildin or env clsoure could be passed here, so we have to create a new list
    // TODO: get rid of member function call of .tail() everywhere
    auto quasiQuoteArgument = MalContainer::tail(ast);
    if (quasiQuoteArgument->isEmpty()) {
        return quasiQuoteArgument;
    }
    return evaluateQuasiQuoteHelper(quasiQuoteArgument->at(0), env);
}

std::shared_ptr<MalType> evaluateDefMacro(const MalContainer* ls, Env& env)
{
    auto macroArguments = evaluateDef(ls, env);
    if (auto closure = macroArguments->asMalClosure(); closure) {
        closure->setIsMacroFunctionCall(true);
    }
    return macroArguments;
}

MalClosure* getMacroFunction(std::shared_ptr<MalType> ast, Env& env)
{
    auto ls = ast->asMalContainer();
    if (!ls || ls->isEmpty()) {
        return nullptr;
    }

    auto firstElemt = ls->at(0);
    if (auto symobl = firstElemt->asMalSymbol(); symobl) {
        if (auto callable = env.find(symobl->asString()); callable) {
            auto macroFunction = callable->asMalClosure();
            if (macroFunction && macroFunction->getIsMacroFucntionCall()) {
                return macroFunction;
            }
        }
    }
    return nullptr;
}

std::shared_ptr<MalType> tryToExpandMacro(std::shared_ptr<MalType> ast, Env& env)
{
    auto macroFunction = getMacroFunction(ast, env);
    if (auto ls = ast->asMalContainer(); ls && macroFunction) {
        auto args = ls->tail();
        auto res = macroFunction->evaluate(args.get(), env);
        return tryToExpandMacro(res, env);
    }
    return ast;
}

std::shared_ptr<MalType> evaluateMacroExpansion(const MalContainer* ls, Env& env)
{
    if (ls->size() < 2) {
        return MalException::throwException("Not engough arguments for macro expansion");
    }
    return tryToExpandMacro(ls->at(1), env);
}

std::string extractExcetpionMessage(const MalException* exception)
{
    auto exceptionString = exception->asString();
    assert(exceptionString.starts_with("Exception: "));

    auto msgBegins = exceptionString.find_first_of(':');
    return msgBegins + 2 < exceptionString.size() ? exceptionString.substr(msgBegins + 2) : "";
}

std::shared_ptr<MalType> evaluateTry(const MalContainer* ls, Env& env)
{
    if (ls->size() == 1) {
        return std::make_shared<MalNil>();
    }

    auto tryBlock = EVAL(ls->at(1), env);
    if (ls->size() > 2 && tryBlock->asMalException()) {
        if (auto catchBlock = ls->at(2)->asMalContainer(); catchBlock && !catchBlock->isEmpty() && catchBlock->at(0)->asString() == "catch*") {
            if (catchBlock->size() <= 2) {
                return std::make_shared<MalNil>();
            }
            auto exceptionName = catchBlock->at(1)->asString();
            auto exceptioinAction = catchBlock->at(2);
            // TODO: come up with other exception handling logic
            auto strException = std::make_shared<MalString>(extractExcetpionMessage(tryBlock->asMalException()));
            Env exceptionEnv(&env);
            env.set(exceptionName, strException);
            return EVAL(exceptioinAction, exceptionEnv);
        }
    }
    return tryBlock;
}

std::shared_ptr<MalType> EVAL(std::shared_ptr<MalType> ast, Env& env)
{
    if (const auto container = ast->asMalContainer(); container) {
        if (container->isEmpty()) {
            return ast;
        }

        // TODO: move some of this fucntions to global env
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
            return EVAL(evaluateQuasiQuote(container, env), env);
        } else if (symbolStr == "quasiquoteexpand") {
            return evaluateQuasiQuote(container, env);
        } else if (symbolStr == "defmacro!") {
            return evaluateDefMacro(container, env);
        } else if (symbolStr == "macroexpand") {
            return evaluateMacroExpansion(container, env);
        } else if (symbolStr == "try*") {
            return evaluateTry(container, env);
        }

        const auto evaluatedList = eval_ast(ast, env);
        if (auto ls = evaluatedList->asMalContainer(); ls && !ls->isEmpty()) {
            const auto head = ls->head();
            if (auto closure = head->asMalClosure(); closure) {
                auto returnValue = closure->evaluate(ls->tail().get(), env);
                return closure->getIsMacroFucntionCall() ? EVAL(returnValue, env) : returnValue;
            } else if (auto buildin = head->asMalBuildin(); buildin) {
                return buildin->evaluate(ls->tail().get(), env);
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
            if (const auto evaluatedElement = EVAL(element, env); evaluatedElement->asMalException()) {
                return evaluatedElement;
            } else {
                newContainer->append(evaluatedElement);
            }
        }
        return newContainer;
    } else if (const auto symbol = ast->asMalSymbol(); symbol) {
        if (symbol->getType() == MalSymbol::SymbolType::KEYWORD) {
            return ast;
        }
        const auto relatedEnv = env.find(symbol->asString());
        if (!relatedEnv) {
            return MalException::throwException("\"'" + symbol->asString() + "'" + " not found\"");
        } else if(symbol->asString() == "*ARGV*") {
            // *ARGV* is the only one callable that doesn't require any arguments
            return relatedEnv->asMalBuildin()->evaluate(nullptr);
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
