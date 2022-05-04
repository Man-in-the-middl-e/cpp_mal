#pragma once

#include "maltypes.h"
#include "printer.h"

#include <functional>

namespace mal {

std::shared_ptr<MalType> prn(MalContainer* args)
{
    std::cout << print_st(args->head().get()) << std::endl;
    return std::make_shared<MalNil>();
}

std::shared_ptr<MalType> list(MalContainer* args)
{
    auto newList = std::make_shared<MalList>();
    for (const auto& obj : *args) {
        newList->append(obj);
    }
    return newList;
}

std::shared_ptr<MalType> isList(MalContainer* args)
{
    const auto list = args->head()->asMalContainer();
    return std::make_shared<MalBoolean>(list != nullptr && list->type() == MalContainer::ContainerType::LIST);
}

std::shared_ptr<MalType> isEmpty(MalContainer* args)
{
    auto ls = args->head()->asMalContainer();
    return std::make_shared<MalBoolean>(ls && ls->size() == 0);
}

std::shared_ptr<MalType> count(MalContainer* args)
{
    if (auto first = args->head(); first->asMalContainer()) {
        return std::make_shared<MalNumber>(first->asMalContainer()->size());
    } else if (first->asMalNil()){
        return std::make_shared<MalNumber>(0);
    }
    return std::make_shared<MalNil>();
}

std::shared_ptr<MalType> equal(MalContainer* args)
{
    if (args->size() <= 1) {
        return std::make_unique<MalError>("Not enough arguments");
    }
    auto lhs = args->at(0).get();
    auto rhs = args->at(1).get();
    return std::make_shared<MalBoolean>(lhs->operator==(rhs));
}

std::shared_ptr<MalType> compareNumbers(MalContainer* numbers, char op, bool equal = false)
{
    if (numbers->size() <= 1) {
        return std::make_unique<MalError>("Not enough arguments");
    }
    
    auto lhs = numbers->at(0)->asMalNumber();
    auto rhs = numbers->at(1)->asMalNumber();

    if (lhs && rhs) {
        bool res = false;
        if (op == '>'){
            res = equal ? lhs->operator>=(rhs) : lhs->operator>(rhs);
        } else if (op == '<'){
            res = equal ? lhs->operator<=(rhs) : lhs->operator<(rhs);
        }
        return std::make_shared<MalBoolean>(res);
    }
    return std::make_unique<MalError>("Could compare only numbers");
}
std::shared_ptr<MalType> less(MalContainer* args)
{
    return compareNumbers(args, '<');
}

std::shared_ptr<MalType> lessEqual(MalContainer* args)
{
    return compareNumbers(args, '<', true);
}

std::shared_ptr<MalType> greater(MalContainer* args)
{
    return compareNumbers(args, '>');
}

std::shared_ptr<MalType> greaterEqual(MalContainer* args)
{
    return compareNumbers(args, '>', true);
}

std::shared_ptr<MalType> malNot(MalContainer* args)
{
    if (args->isEmpty()) {
        return std::make_unique<MalError>("Not enough argumetns for not operator");
    }
    const auto predicate = args->at(0)->asString();
    return std::make_shared<MalBoolean>(predicate == "nil" || predicate == "false");
}

} // mal