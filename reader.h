#pragma once

#include <vector>
#include <memory>

#include "lexer.h"

namespace mal {
class MalType;
class MalNumber;
class MalList;
class MalVector;
class MalHashMap;

class Reader {
public:
    Reader(const std::vector<Token>& tokens);

    Token next() const;
    Token peek() const;

private:
    const std::vector<Token> m_tokens;
    mutable size_t m_currentIndex { 0 };
};

std::shared_ptr<MalType> readStr(std::string_view program);

std::shared_ptr<MalType> readFrom(const Reader& reader);
std::shared_ptr<MalType> readAtom(const Reader& reader);
std::shared_ptr<MalList> readSymobl(const Reader& reader);

// TODO: make one function for that
std::shared_ptr<MalList> readList(const Reader& reader);
std::shared_ptr<MalVector> readVector(const Reader& reader);
std::shared_ptr<MalHashMap> readHashMap(const Reader& reader);
} // namespace mal
