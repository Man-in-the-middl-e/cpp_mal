#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace mal {
enum class TokenType : char;

class MalType {
public:
    virtual std::string asString() const = 0;
    virtual ~MalType();
};

class MalNumber final : public MalType {
public:
    MalNumber(std::string_view number);
    MalNumber(int number);
    std::string asString() const override;
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

    MalContainer(ContainerType containerType);

    std::string asString() const override;

    void append(std::unique_ptr<MalType>);
    bool isEmpty() const;  
    ContainerType type() const;

    MalType* first() const;

    std::vector<std::unique_ptr<MalType>>::iterator begin();
    std::vector<std::unique_ptr<MalType>>::iterator end();

protected:
    std::vector<std::unique_ptr<MalType>> m_data;

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

private:
    std::string m_symbol;
};

class MalString final : public MalType {
public:
    MalString() = default; // for invalid strings
    MalString(std::string_view str);
    std::string asString() const override;
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
    using HashMapIteraotr = std::unordered_map<std::string, std::unique_ptr<MalType>>::iterator;

public:
    std::string asString() const override;

    void insert(const std::string& key, std::unique_ptr<MalType> value);

    HashMapIteraotr begin();
    HashMapIteraotr end();

private:
    std::unordered_map<std::string, std::unique_ptr<MalType>> m_hashMap;
};

class MalOp final : public MalType {
public:
    MalOp(TokenType);
    std::string asString() const override;
    char getOp() const;

private:
    char m_op;
};

class MalError : public MalType {
public:
    MalError(const std::string& message);
    std::string asString() const;

private:
    std::string m_message;
};

} // namespace mal