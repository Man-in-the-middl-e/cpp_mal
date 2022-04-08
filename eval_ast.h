#pragma once

#include <cassert>
#include <functional>
#include <iostream>
#include <numeric>
#include <stack>

#include "maltypes.h"

int applyOp(const std::vector<int>& numbers, const mal::MalOp* malOp)
{
    switch (malOp->getOp()) {
    case '+':
        return std::accumulate(numbers.rbegin(), numbers.rend(), 0);
    case '-':
        return std::accumulate(numbers.rbegin(), numbers.rend(), numbers.back() * 2, std::minus<int>());
    case '*':
        return std::accumulate(numbers.rbegin(), numbers.rend(), 1, std::multiplies<int>());
    case '/':
        return std::accumulate(numbers.rbegin(), numbers.rend(), numbers.back() * numbers.back(), std::divides<int>());
    default:
        std::cout << "op: " << malOp->getOp() << "is not supported\n";
        assert(false);
    }
}

namespace mal {
// TODO: replace ad hoc implementation with pure recusive solution, without using a stack!
std::unique_ptr<MalType> eval_ast(std::unique_ptr<MalType> ast)
{
    if (auto container = dynamic_cast<MalContainer*>(ast.get()); container) {

        if (container->isEmpty()) {
            return ast;
        }

        if (!dynamic_cast<MalOp*>(container->first()) && !dynamic_cast<MalNumber*>(container->first())) {
            auto errorMsg = "operation " + container->first()->asString() + " is not supported";
            return std::make_unique<MalError>(errorMsg);
        }

        std::stack<std::unique_ptr<MalType>> malStack;
        for (auto& obj : *container) {
            if (dynamic_cast<MalContainer*>(obj.get())) {
                malStack.push((eval_ast(std::move(obj))));
                continue;
            }
            malStack.push(std::move(obj));
        }

        while (malStack.size() != 1) {
            std::vector<int> tempBuffer;

            while (!malStack.empty() && !dynamic_cast<MalOp*>(malStack.top().get())) {
                tempBuffer.push_back(dynamic_cast<MalNumber*>(malStack.top().get())->getValue());
                malStack.pop();
            }

            if (malStack.empty()) {
                auto newContainer = std::make_unique<MalContainer>(container->type());
                for (auto it = tempBuffer.rbegin(); it != tempBuffer.rend(); ++it) {
                    newContainer->append(std::make_unique<MalNumber>(*it));
                }
                return newContainer;
            } 

            int res = applyOp(tempBuffer, dynamic_cast<const MalOp*>(malStack.top().get()));
            malStack.pop();
            malStack.push(std::make_unique<MalNumber>(res));
        }
        return std::make_unique<MalNumber>(malStack.top()->asString());
    }

    if (auto hashMap = dynamic_cast<MalHashMap*>(ast.get()); hashMap) {
        auto newHashMap = std::make_unique<MalHashMap>();
        for (auto& [key, value] : *hashMap) {
            newHashMap->insert(key, eval_ast(std::move(value)));
        }
        return newHashMap;
    }
    return ast;
}

} // namespace mal
