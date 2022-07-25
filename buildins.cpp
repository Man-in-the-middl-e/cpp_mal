#include "buildins.h"

#include "eval_ast.h"
#include "maltypes.h"
#include "reader.h"

#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>

namespace mal {

std::shared_ptr<MalType> compareNumbers(MalContainer* numbers, char op, bool equal = false)
{
    if (numbers->size() <= 1) {
        return MalException::throwException("Not enough arguments");
    }

    auto lhs = numbers->at(0)->asMalNumber();
    auto rhs = numbers->at(1)->asMalNumber();

    if (lhs && rhs) {
        bool res = false;
        if (op == '>') {
            res = equal ? lhs->operator>=(rhs) : lhs->operator>(rhs);
        } else if (op == '<') {
            res = equal ? lhs->operator<=(rhs) : lhs->operator<(rhs);
        }
        return std::make_shared<MalBoolean>(res);
    }
    return MalException::throwException("Could compare only numbers");
}

std::shared_ptr<MalType> applyArithmeticOperations(MalContainer* arguments, std::function<int(int, int)> op)
{
    if (arguments->isEmpty() || arguments->size() == 1) {
        return MalException::throwException("Not enough arguments");
    }
    if (const auto baseNumber = arguments->head()->asMalNumber(); !baseNumber) {
        return MalException::throwException("Couldn't apply arithmetic operation to not a number");
    } else {
        int res = baseNumber->getValue();
        for (size_t i = 1; i < arguments->size(); ++i) {
            const auto currentNumber = arguments->at(i)->asMalNumber()->getValue();
            res = op(res, currentNumber);
        }
        return std::make_shared<MalNumber>(res);
    }
}

std::string joinTypeStrings(MalContainer* args, bool withSpace = true)
{
    std::string outStr;
    if (!args->isEmpty()) {
        for (const auto& element : *args) {
            outStr += element->asString() + (args->back() != element  && withSpace ? " " : "");
        }
    }
    return outStr;
}

std::shared_ptr<MalType> prn(MalContainer* args)
{
    std::cout << joinTypeStrings(args) <<  std::endl;
    return std::make_shared<MalNil>();
}

std::shared_ptr<MalType> printString(MalContainer* args)
{
    return std::make_shared<MalString>('"' + MalString::escapeString(joinTypeStrings(args)) + '"');
}

std::shared_ptr<MalType> str(MalContainer* args)
{
    std::string outStr = joinTypeStrings(args, false);

    // remove quotes
    std::string withoutQuotes;
    if (!outStr.empty() && outStr[0] != '"') {
        withoutQuotes += outStr[0];
    }

    for (size_t i = 1; i < outStr.size(); ++i) {
        if (outStr[i - 1] != '\\' && outStr[i] == '"') {
            continue;
        }
        withoutQuotes += outStr[i];
    }
    return std::make_shared<MalString>('"' + withoutQuotes + '"');
}

std::shared_ptr<MalType> println(MalContainer* args)
{
    std::string outStr = joinTypeStrings(args);
    std::string withoutQuotes;
    for (size_t i = 0; i + 1 < outStr.size(); ++i) {
        if (outStr[i] == '"') {
            continue;
        }
        if (outStr[i] == '\\' && outStr[i + 1] == '"') {
            withoutQuotes += '"';
            continue;
        }
        withoutQuotes += outStr[i];
    }
    std::cout << MalString::unEscapeString(withoutQuotes) << std::endl;
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

std::shared_ptr<MalType> makeVector(MalContainer* args)
{
    auto newVector = std::make_shared<MalVector>();
    for (const auto& obj : *args) {
        newVector->append(obj);
    }
    return newVector;
}

std::shared_ptr<MalType> vec(MalContainer* args)
{
    auto vector = std::make_shared<MalVector>();
    if (!args->isEmpty()) {
        if (!args->at(0)->asMalContainer()) {
            return MalException::throwException("Could only be applied to list or vectors");
        }
        for (const auto& obj : *args->at(0)->asMalContainer()) {
            vector->append(obj);
        }
    }
    return vector;
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

std::shared_ptr<MalType> isAtom(MalContainer *args)
{
    return std::make_shared<MalBoolean>(args->head()->asMalAtom() != nullptr);
}

std::shared_ptr<MalType> count(MalContainer* args)
{
    if (auto first = args->head(); first->asMalContainer()) {
        return std::make_shared<MalNumber>(first->asMalContainer()->size());
    } else if (first->asMalNil()) {
        return std::make_shared<MalNumber>(0);
    }
    return std::make_shared<MalNil>();
}

std::shared_ptr<MalType> equal(MalContainer* args)
{
    if (args->size() <= 1) {
        return MalException::throwException("Not enough arguments");
    }
    auto lhs = args->at(0).get();
    auto rhs = args->at(1).get();
    return std::make_shared<MalBoolean>(lhs->operator==(rhs));
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
        return MalException::throwException("Not enough argumetns for not operator");
    }
    const auto predicate = args->at(0)->asString();
    return std::make_shared<MalBoolean>(predicate == "nil" || predicate == "false");
}

std::shared_ptr<MalType> plus(MalContainer* args)
{
    return applyArithmeticOperations(args, std::plus<int>());
}

std::shared_ptr<MalType> minus(MalContainer* args)
{
    return applyArithmeticOperations(args, std::minus<int>());
}

std::shared_ptr<MalType> divides(MalContainer* args)
{
    return applyArithmeticOperations(args, std::divides<int>());
}

std::shared_ptr<MalType> multiplies(MalContainer* args)
{
    return applyArithmeticOperations(args, std::multiplies<int>());
}

std::shared_ptr<MalType> readString(MalType* args, Env&)
{
    const auto progWithQuotes = args->asMalContainer() ? args->asMalContainer()->at(0)->asString() : args->asString();
    const auto prog = progWithQuotes.substr(1, progWithQuotes.size() - 2);
    return mal::readStr(prog);
}

std::optional<std::string> readFile(const std::string& filePath)
{
    if (filePath.empty()) {
        return std::nullopt;
    }

    const auto fileName = filePath.front() == '"' ? filePath.substr(1, filePath.size() - 2) : filePath;
    std::ifstream is(fileName, std::ios::in);
    if (is.is_open()) {
        std::stringstream buffer;
        buffer << is.rdbuf();
        return buffer.str();
    }

    return std::nullopt;
}

std::shared_ptr<MalType> slurp(MalContainer* args, Env&)
{
    if (args->isEmpty()) {
        return MalException::throwException("slurp expect file name");
    }

    auto fileContent = readFile(args->at(0)->asString());
    if (fileContent.has_value()){
        return std::make_shared<MalSymbol>('"' + MalString::escapeString(fileContent.value()) + '"');
    }

    return MalException::throwException("Couldn't open the file");
}

std::shared_ptr<MalType> eval(MalContainer* args, Env& env)
{
    if (args->isEmpty()) {
        return MalException::throwException("eval <ast>");
    }
    // TODO: make copy ctor
    // TODO: don't create new container
    auto newContaienr = std::make_shared<MalContainer>(args->type());
    for (const auto& elem : *args) {
        // TODO: how to construct shared_ptr<MalType> from first elem of the list????
        if (args->size() == 1) {
            return EVAL(elem, env);
        }
        newContaienr->append(elem);
    }
    return EVAL(newContaienr, env);
}

std::shared_ptr<MalType> loadFile(MalContainer* args, Env& env)
{
    auto fileContent = readFile(args->at(0)->asString());
    if (fileContent.has_value()) {
        auto program = std::make_shared<MalSymbol>("\"(do " + fileContent.value() + "\n)\"");
        auto ast = readString(program.get(), env);
        std::cout << eval(ast->asMalContainer(), env)->asString() << std::endl;
        return std::make_shared<MalNil>();
    }
    return std::make_shared<MalException>("Failed to load file");
}

std::shared_ptr<MalType> deref(MalContainer* args)
{
    if (args->size() < 1) {
        return MalException::throwException("Not enough arguments");
    }

    if (const auto malAtom = args->at(0); malAtom->asMalAtom()) {
        return malAtom->asMalAtom()->deref();
    }
    return MalException::throwException("Value is not an atom");;
}

std::shared_ptr<MalType> argv(MalContainer*)
{
    return GlobalEnv::the().getArgvs();
}

std::shared_ptr<MalType> cons(MalContainer *args)
{
    if (args->size() < 2) {
        return MalException::throwException("Not enough arguments");
    }
    if (auto originalContainer = args->at(1)->asMalContainer(); originalContainer) {
        auto list = std::make_shared<MalList>();
        list->append(args->at(0));
        for (const auto& elem : *originalContainer) {
            list->append(elem);
        }
        return list;
    }
    return MalException::throwException("Can append only to vectors and list");
}

std::shared_ptr<MalType> concat(MalContainer* args)
{
    auto list = std::make_shared<MalList>();
    if (args->isEmpty()) {
        return list;
    }

    //([1 2] (list 3 4) [5 6])
    for (size_t elementIndex = 0; elementIndex < args->size(); ++elementIndex) {
        if (const auto maybeContainer = args->at(elementIndex)->asMalContainer(); maybeContainer) {
            for (const auto& elem : *maybeContainer) {
                list->append(elem);
            }
        } else {
            list->append(args->at(elementIndex));
        }
    }
    return list;
}

std::shared_ptr<MalType> nth(MalContainer* args)
{
    if (args->isEmpty() || !args->at(0)->asMalContainer()) {
        return MalException::throwException("List or vector is expected");
    }

    if (args->size() == 1 || !args->at(1)->asMalNumber()) {
        return MalException::throwException("Integer index is expected");
    }

    auto container = args->at(0)->asMalContainer();
    size_t nthElemet = args->at(1)->asMalNumber()->getValue();
    return nthElemet >= container->size() ? MalException::throwException("Index out of range") : container->at(nthElemet);
}

std::shared_ptr<MalType> first(MalContainer* args)
{
    if (args->isEmpty() || !args->at(0)->asMalContainer()) {
        return std::make_shared<MalNil>();
    }

    auto container = args->at(0)->asMalContainer();
    return container->isEmpty() ? std::make_shared<MalNil>() : container->at(0);
}

std::shared_ptr<MalType> rest(MalContainer* args) 
{
    if (args->isEmpty() || !args->at(0)->asMalContainer()) {
        return std::make_shared<MalList>();
    }
    auto tail = MalContainer::tail(args->at(0)->asMalContainer());
    tail->toList();
    return tail;
}

std::shared_ptr<MalType> cond(MalContainer* args)
{
    // TODO: this should be a macro, for now there is no way to define buildin macros
    if (args->size() < 2) {
        return std::make_shared<MalNil>();
    }

    auto trueCondtion = args->at(0);
    auto trueBranch = args->at(1);
    if (trueCondtion->asString() != "false" && trueBranch->asString() != "nil") {
        return trueBranch;
    }
    
    auto rest = args->tail()->tail();
    return cond(rest.get());
}

std::shared_ptr<MalType> malThrow(MalContainer* args)
{
    if (args->isEmpty()) {
        return std::make_shared<MalNil>();
    }
    return MalException::throwException(args->at(0)->asString());
}

std::shared_ptr<MalType> apply(MalContainer* args, Env& env)
{
    if (args->size() < 2) {
        return MalException::throwException("not enough argumetns for apply");
    }

    auto function = MalCallable::builinOrCallable(args->at(0).get());
    if (!function) {
        return MalException::throwException("function is expected");
    }

    auto arguments = concat(args->tail()->asMalContainer());
    return function->evaluate(arguments->asMalContainer(), env);
}

std::shared_ptr<MalType> map(MalContainer* args, Env& env)
{
    if (args->size() < 2) {
        return MalException::throwException("not enough argumetns for map");
    }

    auto function = MalCallable::builinOrCallable(args->at(0).get());
    if (!function) {
        return MalException::throwException("function as first argument is expected");
    }

    auto lisToMapped = args->at(1)->asMalContainer();
    if (!lisToMapped) {
        return MalException::throwException("list or vector is expected");
    }

    auto mappedList = std::make_shared<MalList>();
    for (auto element : *lisToMapped) {
        // TODO: don't make stupid design decisions and assumptions)
        auto elemAsList = std::make_shared<MalList>();
        elemAsList->append(element);
        if (auto mappedElemet = function->evaluate(elemAsList.get(), env); mappedElemet->asMalException()){
            return mappedElemet;
        } else {
            mappedList->append(mappedElemet);
        }
    }
    return mappedList;
}

std::shared_ptr<MalType> isNil(MalContainer* args)
{
    return std::make_shared<MalBoolean>(args->head()->asMalNil() != nullptr);
}

std::shared_ptr<MalType> isSymbol(MalContainer* args)
{
    auto symbol = args->head()->asMalSymbol();
    return std::make_shared<MalBoolean>(symbol && symbol->getType() != MalSymbol::SymbolType::KEYWORD);
}

std::shared_ptr<MalType> isTrue(MalContainer* args)
{
    auto boolean = args->head()->asMalBoolean() ;
    return std::make_shared<MalBoolean>(boolean && boolean->asString() == "true");
}

std::shared_ptr<MalType> isFalse(MalContainer* args)
{
    auto boolean = args->head()->asMalBoolean() ;
    return std::make_shared<MalBoolean>(boolean && boolean->asString() == "false");
}

std::shared_ptr<MalType> isVector(MalContainer* args)
{
    const auto list = args->head()->asMalContainer();
    return std::make_shared<MalBoolean>(list != nullptr && list->type() == MalContainer::ContainerType::VECTOR);
}

std::shared_ptr<MalType> isSequential(MalContainer* args)
{
    return std::make_shared<MalBoolean>(!args->isEmpty() && args->at(0)->asMalContainer());
}

std::shared_ptr<MalType> isMap(MalContainer* args)
{
    return std::make_shared<MalBoolean>(!args->isEmpty() && args->at(0)->asMalHashMap());
}

std::shared_ptr<MalType> isKeyword(MalContainer* args)
{
    auto symbol = args->head()->asMalSymbol();
    return std::make_shared<MalBoolean>(symbol && symbol->getType() == MalSymbol::SymbolType::KEYWORD);
}

std::string removeQuotes(const std::string& str)
{
    if (str.size() < 2 || str.front() != '"') {
        return str;
    }
    return str.substr(1, str.size() - 2);
}

std::shared_ptr<MalType> makeKeyword(MalContainer* args)
{
    if (args->isEmpty()) {
        return MalException::throwException("Not enough arguments to make keyword");
    }

    auto toKeword = args->at(0);
    if (toKeword->asString().front() == ':') {
        return args->at(0);
    }

    if (!toKeword->asMalString()) {
        return MalException::throwException("Argument should be string or keyword");
    }

    auto keyword = ':' + removeQuotes(toKeword->asString());
    return std::make_shared<MalSymbol>(keyword, MalSymbol::SymbolType::KEYWORD);
}

std::shared_ptr<MalType> makeSymbol(MalContainer* args)
{
    if (args->isEmpty()) {
        return MalException::throwException("Not enough arguments to make symbol");
    }

    if (auto argument = args->at(0); MalCallable::builinOrCallable(argument.get())){
        return MalException::throwException("Symbol can't be callable object");
    }

    return std::make_shared<MalSymbol>(removeQuotes(args->at(0)->asString()));
}

std::shared_ptr<MalType> makeHashMap(MalContainer* args)
{
    if (args->size() % 2 != 0) {
        return MalException::throwException("Not enough arguments to make hash map");
    }

    auto hashMap = std::make_shared<MalHashMap>();
    for (size_t elementIndex = 0; elementIndex  < args->size(); elementIndex += 2) {
        hashMap->insert(args->at(elementIndex)->asString(),
                        args->at(elementIndex + 1));
    }
    return hashMap;
}

std::shared_ptr<MalType> assoc(MalContainer* args)
{
    // (assoc {} "a" 1)
    if (args->isEmpty()) {
        return MalException::throwException("Not enought arguments to assoc");
    }

    auto mapToMereIn = args->at(0)->asMalHashMap();

    if (!mapToMereIn) {
        return MalException::throwException("First argument should be hash-map");
    }

    if ((args->size() - 1) % 2 != 0) {
        return MalException::throwException("Number of keys\\values should be even");
    }

    auto newHashMap = std::make_shared<MalHashMap>();

    for (auto& [key, value] : *mapToMereIn) {
        newHashMap->insert(key, value);
    }

    for (size_t elementIndex = 1; elementIndex < args->size(); elementIndex += 2) {
        newHashMap->insert(args->at(elementIndex)->asString(),
                           args->at(elementIndex + 1));
    }
    return newHashMap;
}

std::shared_ptr<MalType> dissoc(MalContainer* args)
{
    // (dissoc {:cde 345 :fgh 456} :cde) -> {:fgh 465}
    if (args->isEmpty() || !args->at(0)->asMalHashMap()) {
        return MalException::throwException("Hash-map is expected");
    }
    auto oldHashMap = args->at(0)->asMalHashMap();
    auto newHashMap = std::make_shared<MalHashMap>();
    // TODO: make copy constructor
    for (auto& [key, value] : *oldHashMap) {
        newHashMap->insert(key, value);
    }

    for (size_t keyToRemoveIndex = 1; keyToRemoveIndex < args->size(); ++keyToRemoveIndex) {
        auto keyToRemove = args->at(keyToRemoveIndex)->asString();
        newHashMap->remove(keyToRemove);
    }
    return newHashMap;
}

std::shared_ptr<MalType> malGet(MalContainer* args)
{
    if (args->isEmpty() || (!args->at(0)->asMalHashMap() && !args->at(0)->asMalNil())) {
        return MalException::throwException("Hash-map is expected");
    }

    if (args->at(0)->asMalNil()) {
        return args->at(0);
    }

    if (args->size() < 2) {
        return MalException::throwException("Key to hash-map is expected");
    }

    auto hashMap = args->at(0)->asMalHashMap();
    auto key = args->at(1)->asString();

    if (auto relatedValue = hashMap->find(key); relatedValue != hashMap->end()) {
        return relatedValue->second;
    }
    return std::make_shared<MalNil>();
}

std::shared_ptr<MalType> contains(MalContainer* args)
{
    if (args->isEmpty() || !args->at(0)->asMalHashMap()) {
        return MalException::throwException("Hash-map is expected");
    }

    if (args->size() < 2) {
        return MalException::throwException("Key to hash-map is expected");
    }

    auto hashMap = args->at(0)->asMalHashMap();
    auto key = args->at(1)->asString();

    auto relatedValue = hashMap->find(key); 
    return std::make_shared<MalBoolean>(relatedValue != hashMap->end());
}

std::shared_ptr<MalType> keys(MalContainer* args)
{
    if (args->isEmpty() || !args->at(0)->asMalHashMap()) {
        return MalException::throwException("Hash-map is expected");
    }
    return args->at(0)->asMalHashMap()->keys();
}

std::shared_ptr<MalType> vals(MalContainer* args)
{
    if (args->isEmpty() || !args->at(0)->asMalHashMap()) {
        return MalException::throwException("Hash-map is expected");
    }
    return args->at(0)->asMalHashMap()->vals();
}

std::shared_ptr<MalType> malReadline(MalContainer* args)
{
    if (args->isEmpty()) {
        return MalException::throwException("Except a string as first argument");
    }
    std::cout << removeQuotes(args->at(0)->asString()) << " ";
    std::string currentLine;
    std::getline(std::cin, currentLine);
    
    if (!currentLine.empty())
    {
        return std::make_shared<MalSymbol>('"' + MalString::escapeString(currentLine) + '"');
    }

    return std::make_shared<MalNil>();
}

std::shared_ptr<MalType> hostLanguage(MalContainer*)
{
    return std::make_shared<MalSymbol>("C++20");
}

std::shared_ptr<MalType> meta(MalContainer* args)
{
    if (args->isEmpty()) {
        return MalException::throwException("Not arguments for meta");
    }
    return args->at(0)->getMetaInfo();
}

std::shared_ptr<MalType> withMeta(MalContainer* args)
{
    if (args->size() < 2) {
        return MalException::throwException("Not arguments for meta");
    }

    auto type = args->at(0);
    auto metaInfo = args->at(1);

    auto newType = type->clone();
    newType->setMetaInfo(metaInfo);
    
    return newType;
}

} // mal