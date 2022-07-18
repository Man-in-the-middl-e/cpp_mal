#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "env.h"

namespace mal {
enum class TokenType : char;

class MalAtom;
class MalNumber;
class MalContainer;
class MalSymbol;
class MalString;
class MalHashMap;
class MalException;
class MalBoolean;
class MalNil;
class MalClosure;
class MalBuildin;

class MalType {
public:
    virtual std::string asString() const = 0;

    virtual MalNumber* asMalNumber() { return nullptr; }
    virtual MalContainer* asMalContainer() { return nullptr; }
    virtual MalSymbol* asMalSymbol() { return nullptr; }
    virtual MalString* asMalString() { return nullptr; }
    virtual MalHashMap* asMalHashMap() { return nullptr; }
    virtual MalException* asMalException() { return nullptr; }
    virtual MalBoolean* asMalBoolean() { return nullptr; }
    virtual MalNil* asMalNil() { return nullptr; }
    virtual MalClosure* asMalClosure() { return nullptr; }
    virtual MalBuildin* asMalBuildin() { return nullptr; }
    virtual MalAtom* asMalAtom() { return nullptr; }

    virtual bool operator==(MalType*) const { return false; }

    virtual ~MalType()
    {
    }
};

class MalAtom : public MalType {
public:
    MalAtom(std::shared_ptr<MalType> malType, const std::string& atomDesripton);

    std::string asString() const override;
    MalAtom* asMalAtom() override;

    std::shared_ptr<MalType> reset(std::shared_ptr<MalType> newType);
    std::shared_ptr<MalType> deref() const;

private:
    std::shared_ptr<MalType> m_underlyingType;
    std::string m_atomDescripton;
};

class MalNumber final : public MalType {
public:
    MalNumber(std::string_view number);
    MalNumber(int number);

    std::string asString() const override;
    MalNumber* asMalNumber() override;

    virtual bool operator==(MalType* type) const override
    {
        return type->asMalNumber() && type->asMalNumber()->getValue() == m_number;
    }

    bool operator>(MalNumber* malNumber) const
    {
        return m_number > malNumber->getValue();
    }

    bool operator>=(MalNumber* malNumber) const
    {
        return m_number >= malNumber->getValue();
    }

    bool operator<(MalNumber* malNumber) const
    {
        return m_number < malNumber->getValue();
    }

    bool operator<=(MalNumber* malNumber) const
    {
        return m_number <= malNumber->getValue();
    }

    int getValue() const;

private:
    int m_number;
};

class MalContainer : public MalType {
public:
    enum class ContainerType {
        LIST,
        VECTOR
    };

    MalContainer(const std::vector<std::shared_ptr<MalType>>& data, MalContainer::ContainerType type);
    MalContainer(ContainerType containerType);

    std::string asString() const override;
    MalContainer* asMalContainer() override;

    virtual bool operator==(MalType* type) const override
    {
        // TODO: Compare only lists and not containers
        if (auto ls = type->asMalContainer(); ls && ls->size() == m_data.size()) {
            for (size_t lsIndex = 0; lsIndex < m_data.size(); ++lsIndex) {
                const auto lhs = m_data[lsIndex];
                const auto rhs = ls->at(lsIndex);
                if (!(lhs->operator==(rhs.get()))) {
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    void append(std::shared_ptr<MalType>);
    bool isEmpty() const;
    size_t size() const;
    ContainerType type() const;
    void toList() {
        m_type = ContainerType::LIST;
    }

    std::shared_ptr<MalType> at(size_t index) const;
    std::shared_ptr<MalType> back() const;

    std::shared_ptr<MalType> head() const;
    std::shared_ptr<MalContainer> tail();

    static std::shared_ptr<MalContainer> tail(MalContainer* container);

    std::vector<std::shared_ptr<MalType>>::iterator begin();
    std::vector<std::shared_ptr<MalType>>::iterator end();

protected:
    std::vector<std::shared_ptr<MalType>> m_data;

private:
    ContainerType m_type;
};

// TODO: do we need MalList and MalVector???
class MalList final : public MalContainer {
public:
    MalList();
};

class MalVector final : public MalContainer {
public:
    MalVector();
};

class MalSymbol final : public MalType {
public:
    enum class SymbolType {
        REGULAR_SYMBOL,
        KEYWORD
    };
public:
    MalSymbol(std::string_view symbol, SymbolType type = SymbolType::REGULAR_SYMBOL);

    std::string asString() const override;
    MalSymbol* asMalSymbol() override;

    virtual bool operator==(MalType* type) const override
    {
        return type->asMalSymbol() && type->asString() == m_symbol;
    }

    SymbolType getType() const;
    
private:
    std::string m_symbol;
    SymbolType m_symbolType;
};

class MalString final : public MalType {
public:
    MalString(std::string_view str);

    std::string asString() const override;
    MalString* asMalString() override;

    virtual bool operator==(MalType* type) const override
    {
        return type->asMalString() && type->asString() == m_malString;
    }

    bool isEmpty() const;

public:
    static std::string escapeString(const std::string& str);
    static std::string unEscapeString(const std::string& str);

private:
    std::string m_malString;
};

class MalNil final : public MalType {
public:
    std::string asString() const override;
    MalNil* asMalNil() override;

    virtual bool operator==(MalType* type) const override
    {
        return type->asMalNil();
    }
};

class MalBoolean final : public MalType {
public:
    MalBoolean(bool value);
    MalBoolean(std::string_view strValue);

    std::string asString() const override;
    MalBoolean* asMalBoolean() override;

    bool getValue() const;

    virtual bool operator==(MalType* type) const override
    {
        return type->asMalBoolean() && type->asMalBoolean()->getValue() == m_boolValue;
    }

private:
    const bool m_boolValue;
};

class MalHashMap final : public MalType {
public:
    using HashMapIteraotr = std::unordered_map<std::string, std::shared_ptr<MalType>>::iterator;

public:
    std::string asString() const override;
    MalHashMap* asMalHashMap() override;

    virtual bool operator==(MalType* type) const override
    {
        if (auto hashMap = type->asMalHashMap(); hashMap && hashMap->size() == m_hashMap.size()) {
            for (const auto& [key, value]: m_hashMap) {
                auto otherElement = hashMap->find(key);
                if (otherElement != hashMap->end()) {
                    auto& [otherKey, otherValue] = *otherElement;
                    if (key != otherKey || !(value->operator==(otherValue.get()))){
                        return false;
                    }
                }
            }
            return true;
        }
        return false;
    }

    void insert(const std::string& key, std::shared_ptr<MalType> value);
    size_t size() const;

    HashMapIteraotr begin();
    HashMapIteraotr end();
    HashMapIteraotr find(const std::string& key);

private:
    std::unordered_map<std::string, std::shared_ptr<MalType>> m_hashMap;
};

class MalException : public MalType {
public:
    MalException(const std::string& message);

    std::string asString() const override;
    MalException* asMalException() override;

    static std::shared_ptr<MalException> throwException(const std::string& message);

private:
    std::string m_message;
};


class MalCallable : public MalType {
public:
    virtual std::shared_ptr<MalType> evaluate(MalContainer* arguments, Env& env) = 0;
    static MalCallable* builinOrCallable(MalType* callable);
};

class MalClosure : public MalCallable {
public:
    MalClosure(const std::shared_ptr<MalType> parameters, const std::shared_ptr<MalType> body, const Env& env);

    std::string asString() const override;
    MalClosure* asMalClosure() override;

    std::shared_ptr<MalType> evaluate(MalContainer* arguments, Env& env) override;

    bool getIsMacroFucntionCall() const;
    void setIsMacroFunctionCall(bool isMacro);

private:
    const std::shared_ptr<MalType> m_functionParameters;
    const std::shared_ptr<MalType> m_functionBody;
    Env m_relatedEnv;
    bool m_isMacroFunctionCall { false };
};

class MalBuildin : public MalCallable {
public:
    using Buildin = std::function<std::shared_ptr<MalType>(MalContainer*)>;
    using BuildinWithEnv = std::function<std::shared_ptr<MalType>(MalContainer*, Env&)>;

    MalBuildin(Buildin buildinFunc);
    MalBuildin(BuildinWithEnv buildinFuncWithEnv);

    std::string asString() const override;
    MalBuildin* asMalBuildin() override;

    std::shared_ptr<MalType> evaluate(MalContainer* args, Env& env) override;
    std::shared_ptr<MalType> evaluate(MalContainer* args) const;

private:
    Buildin m_buildin;
    BuildinWithEnv m_buildinWithEnv;
};

} // namespace mal