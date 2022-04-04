#include <iostream>
#include <string_view>

#include "maltypes.h"
#include "reader.h"

namespace mal {

Reader::Reader(const std::vector<Token>& tokens)
    : m_tokens(tokens)
{
}

Token Reader::next() const
{
    return m_tokens[m_currentIndex++];
}

Token Reader::peek() const
{
    return m_tokens[m_currentIndex];
}

std::unique_ptr<MalType> readStr(std::string_view program)
{
    Lexer lexer(program);
    Reader reader(lexer.tokenize());
    return readFrom(reader);
}

std::unique_ptr<MalType> readFrom(const Reader& reader)
{
    const auto currentTokenType = reader.peek().type;
    switch (currentTokenType) {
    case TokenType::LEFT_PAREN:
        return readList(reader);
    case TokenType::LEFT_SQUARE_BRACE:
        return readVector(reader);
    case TokenType::LEFT_CURLY_BRACE:
        return readHashMap(reader);
    default:
        return readAtom(reader);
    }
}

// NOTE: We could return raw pointer, that will be adopted by callers
std::unique_ptr<MalType> readAtom(const Reader& reader)
{
    const auto currentToken = reader.peek();
    switch (currentToken.type) {
    case TokenType::NUMBER:
        return std::make_unique<MalNumber>(currentToken.token);
    case TokenType::STRING:
        return std::make_unique<MalString>(currentToken.token);
    case TokenType::ERROR_UNTERMINATED_STRING:
        return std::make_unique<MalString>();
    default:
        return std::make_unique<MalSymbol>(currentToken.token);
    }
}

std::unique_ptr<MalList> readList(const Reader& reader)
{
    auto malList = std::make_unique<MalList>();
    // skip left paren
    reader.next();

    while (reader.peek().type != TokenType::RIGHT_PAREN
        && reader.peek().type != TokenType::LAST_TOKEN) {
        // TODO: do error checking
        malList->append(readFrom(reader));
        reader.next();
    }
    if (reader.peek().type == TokenType::LAST_TOKEN) {
        std::cout << "unbalanced" << std::endl;
        return std::make_unique<MalList>();
    }
    return malList;
}

std::unique_ptr<MalVector> readVector(const Reader& reader)
{
    auto malVector = std::make_unique<MalVector>();
    reader.next();

    while (reader.peek().type != TokenType::RIGHT_SQUARE_BACE
        && reader.peek().type != TokenType::LAST_TOKEN) {
        // TODO: do error checking
        malVector->append(readFrom(reader));
        reader.next();
    }
    if (reader.peek().type == TokenType::LAST_TOKEN) {
        std::cout << "unbalanced" << std::endl;
        return std::make_unique<MalVector>();
    }
    return malVector;
}

std::unique_ptr<MalHashMap> readHashMap(const Reader& reader)
{
    auto malHashMap = std::make_unique<MalHashMap>();
    reader.next();

    while (reader.peek().type != TokenType::RIGHT_CURLY_BRACE
        && reader.peek().type != TokenType::LAST_TOKEN) {
        // TODO: do error checking
        const std::string key = readAtom(reader)->asString();
        reader.next();
        auto value = readFrom(reader);
        malHashMap->insert(key, std::move(value));
        reader.next();
    }
    if (reader.peek().type == TokenType::LAST_TOKEN) {
        std::cout << "unbalanced" << std::endl;
        return std::make_unique<MalHashMap>();
    }
    return malHashMap;
}

} // mal