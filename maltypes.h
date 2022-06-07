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

class MalNumber;
class MalContainer;
class MalSymbol;
class MalString;
class MalHashMap;
class MalError;
class MalBoolean;
class MalNil;
class MalClosure;
class MalCallable;

class MalType {
public:
    virtual std::string asString() const = 0;

    virtual MalNumber* asMalNumber() { return nullptr; }
    virtual MalContainer* asMalContainer() { return nullptr; }
    virtual MalSymbol* asMalSymbol() { return nullptr; }
    virtual MalString* asMalString() { return nullptr; }
    virtual MalHashMap* asMalHashMap() { return nullptr; }
    virtual MalError* asMalError() { return nullptr; }
    virtual MalBoolean* asMalBoolean() { return nullptr; }
    virtual MalNil* asMalNil() { return nullptr; }
    virtual MalClosure* asMalClosure() { return nullptr; }
    virtual MalCallable* asMalCallable() { return nullptr; }

    virtual bool operator==(MalType*) const { return false; }

    virtual ~MalType()
    {
    }
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

    std::shared_ptr<MalType> at(size_t index) const;
    std::shared_ptr<MalType> back() const;

    std::shared_ptr<MalType> head() const;
    std::shared_ptr<MalContainer> tail();

    std::vector<std::shared_ptr<MalType>>::iterator begin();
    std::vector<std::shared_ptr<MalType>>::iterator end();

protected:
    std::vector<std::shared_ptr<MalType>> m_data;

private:
    ContainerType m_type;
};

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
    MalSymbol(std::string_view symbol);

    std::string asString() const override;
    MalSymbol* asMalSymbol() override;

private:
    std::string m_symbol;
};

class MalString final : public MalType {
public:
    MalString() = default; // for invalid strings
    MalString(std::string_view str);

    std::string asString() const override;
    MalString* asMalString() override;

    virtual bool operator==(MalType* type) const override
    {
        return type->asMalString() && type->asString() == m_malString;
    }

    bool isEmpty() const;

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

class MalError : public MalType {
public:
    MalError(const std::string& message);

    std::string asString() const override;
    MalError* asMalError() override;

private:
    std::string m_message;
};

class MalClosure : public MalType {
public:
    MalClosure(const std::shared_ptr<MalType> parameters, const std::shared_ptr<MalType> body, const Env& parentEnv);

    std::string asString() const override;
    MalClosure* asMalClosure() override;

    std::shared_ptr<MalType> apply(const MalContainer* arguments);
    
private:
    const std::shared_ptr<MalType> m_functionParameters;
    const std::shared_ptr<MalType> m_functionBody;
    Env m_relatedEnv;
};

class MalCallable : public MalType {
public:
    using Callable = std::function<std::shared_ptr<MalType>(MalContainer*)>;

    MalCallable(Callable callable);

    std::string asString() const override;
    MalCallable* asMalCallable() override;

    std::shared_ptr<MalType> apply(MalContainer* args) const;

private:
    Callable m_callableObj;
};


} // namespace mal