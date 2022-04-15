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

std::shared_ptr<MalType> readStr(std::string_view program)
{
    Lexer lexer(program);
    Reader reader(lexer.tokenize());
    return readFrom(reader);
}

std::shared_ptr<MalType> readFrom(const Reader& reader)
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
std::shared_ptr<MalType> readAtom(const Reader& reader)
{
    const auto currentToken = reader.peek();
    switch (currentToken.type) {
    case TokenType::NUMBER:
        return std::make_shared<MalNumber>(currentToken.token);
    case TokenType::STRING:
        return std::make_shared<MalString>(currentToken.token);
    case TokenType::ERROR_UNTERMINATED_STRING:
        return std::make_shared<MalString>();
    case TokenType::PLUS:
    case TokenType::MINUS:
    case TokenType::MULT:
    case TokenType::DIVIDE:
        return std::make_shared<MalOp>(currentToken.token.at(0));
    default:
        return std::make_shared<MalSymbol>(currentToken.token);
    }
}

std::shared_ptr<MalList> readList(const Reader& reader)
{
    auto malList = std::make_shared<MalList>();
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
        return std::make_shared<MalList>();
    }
    return malList;
}

std::shared_ptr<MalVector> readVector(const Reader& reader)
{
    auto malVector = std::make_shared<MalVector>();
    reader.next();

    while (reader.peek().type != TokenType::RIGHT_SQUARE_BACE
        && reader.peek().type != TokenType::LAST_TOKEN) {
        // TODO: do error checking
        malVector->append(readFrom(reader));
        reader.next();
    }
    if (reader.peek().type == TokenType::LAST_TOKEN) {
        std::cout << "unbalanced" << std::endl;
        return std::make_shared<MalVector>();
    }
    return malVector;
}

std::shared_ptr<MalHashMap> readHashMap(const Reader& reader)
{
    auto malHashMap = std::make_shared<MalHashMap>();
    reader.next();

    while (reader.peek().type != TokenType::RIGHT_CURLY_BRACE
        && reader.peek().type != TokenType::LAST_TOKEN) {
        // TODO: do error checking
        const std::string key = readAtom(reader)->asString();
        reader.next();
        auto value = readFrom(reader);
        malHashMap->insert(key, value);
        reader.next();
    }
    if (reader.peek().type == TokenType::LAST_TOKEN) {
        std::cout << "unbalanced" << std::endl;
        return std::make_shared<MalHashMap>();
    }
    return malHashMap;
}

} // mal