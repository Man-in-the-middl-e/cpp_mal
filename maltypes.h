#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <unordered_map>

namespace mal {

class MalType {
public:
    virtual std::string asString() const = 0;
    virtual ~MalType();
};

class MalNumber final : public MalType {
public:
    MalNumber(std::string_view number);
    std::string asString() const override;
    int getValue() const;

private:
    int m_number;
};

class MalContainer : public MalType {
public:
    MalContainer(char openBracket, char closeBracket);
    std::string asString() const override;
    void append(std::unique_ptr<MalType>);

protected:
    std::vector<std::unique_ptr<MalType>> m_data;

private:
    char m_openBracket;
    char m_closeBracket;
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
    std::string asString() const override;

    void insert(const std::string& key, std::unique_ptr<MalType> value);

private:
    std::unordered_map<std::string, std::unique_ptr<MalType>> m_hasMap;
};

} // namespace mal