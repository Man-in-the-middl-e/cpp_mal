#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace mal {
enum class TokenType : char;

class MalNumber;
class MalContainer;
class MalSymbol;
class MalString;
class MalHashMap;
class MalOp;
class MalError;
class MalBoolean;
class MalNil;
class MalFunction;

class MalType {
public:
    virtual std::string asString() const = 0;

    virtual MalNumber* asMalNumber() { return nullptr; }
    virtual MalContainer* asMalContainer() { return nullptr; }
    virtual MalSymbol* asMalSymbol() { return nullptr; }
    virtual MalString* asMalString() { return nullptr; }
    virtual MalHashMap* asMalHashMap() { return nullptr; }
    virtual MalOp* asMalOp() { return nullptr; }
    virtual MalError* asMalError() { return nullptr; }
    virtual MalBoolean* asMalBoolean() { return nullptr; }
    virtual MalNil* asMalNil() { return nullptr; }
    virtual MalFunction* asMalFunction() { return nullptr; }

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

    bool isEmpty() const;

private:
    std::string m_malString;
};

class MalNil final : public MalType {
public:
    std::string asString() const override;
};

class MalBoolean final : public MalType {
public:
    MalBoolean(bool value);
    std::string asString() const override;
    bool getValue() const;

private:
    const bool m_boolValue;
};

class MalHashMap final : public MalType {
public:
    using HashMapIteraotr = std::unordered_map<std::string, std::shared_ptr<MalType>>::iterator;

public:
    std::string asString() const override;
    MalHashMap* asMalHashMap() override;

    void insert(const std::string& key, std::shared_ptr<MalType> value);

    HashMapIteraotr begin();
    HashMapIteraotr end();

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

class MalOp final : public MalType {
public:
    MalOp(char op);

    std::string asString() const override;
    MalOp* asMalOp() override;

    std::shared_ptr<MalType> operator()(const MalContainer* arguments);

private:
    char m_opType;
    std::function<int(int, int)> m_op;
};

class MalFunction : public MalType {
public:
    MalFunction();

    std::string asString() const override;
    MalFunction* asMalFunction() override;
};

} // namespace mal