#include "lexer.h"

#include <assert.h>
#include <ctype.h>
#include <iostream>

namespace mal {

const char* printToken(TokenType type)
{
#define TOKEN_TYPE_STR(NAME) \
    case TokenType::NAME:    \
        return #NAME;
    switch (type) {
        TOKEN_TYPE_ENUM(TOKEN_TYPE_STR)
    default:
        return "UNKONWN";
    }
#undef TOKEN_TYPE_STR
}

std::ostream& operator<<(std::ostream& stream, TokenType type)
{
    stream << printToken(type);
    return stream;
}

Lexer::Lexer(std::string_view program)
    : m_program(program)
{
}

std::vector<Token> Lexer::tokenize()
{
    std::vector<Token> tokens;
    auto makeOneCharToken = [this](TokenType type) {
        return makeToken(type, m_currentIndex - 1, 1);
    };

    while (m_currentIndex != m_program.size()) {
        const char currentSymbol = advance();

        if (isspace(currentSymbol) || currentSymbol == ',') {
            continue;
        }

        if (isdigit(currentSymbol)) {
            tokens.push_back(matchNumber());
            continue;
        }

        switch (currentSymbol) {
        case '[': {
            tokens.push_back(makeOneCharToken(TokenType::LEFT_SQUARE_BRACE));
            break;
        }
        case ']': {
            tokens.push_back(makeOneCharToken(TokenType::RIGHT_SQUARE_BACE));
            break;
        }
        case '{': {
            tokens.push_back(makeOneCharToken(TokenType::LEFT_CURLY_BRACE));
            break;
        }
        case '}': {
            tokens.push_back(makeOneCharToken(TokenType::RIGHT_CURLY_BRACE));
            break;
        }
        case '(': {
            tokens.push_back(makeOneCharToken(TokenType::LEFT_PAREN));
            break;
        }
        case ')': {
            tokens.push_back(makeOneCharToken(TokenType::RIGHT_PAREN));
            break;
        }
        case '^':
        case '\'': 
        case '`':
        case '@':
        case '~': {
            // try to match ~@
            if (match('@')) {
                advance();
                tokens.push_back(makeToken(TokenType::MACRO, m_currentIndex - 2, 2));
            } else {
                tokens.push_back(makeOneCharToken(TokenType::MACRO));
            }
            break;
        }
        case '*': {
            if (match('*')) {
                advance();
                tokens.push_back(makeToken(TokenType::DOUBLE_STAR, m_currentIndex - 2, 2));
            } else if (isalpha(peek())) {
                tokens.push_back(matchEverythingElse());
            } else {
                tokens.push_back(makeOneCharToken(TokenType::SYMBOL));
            }
            break;
        }
        case '-': {
            if (isdigit(peek())) {
                tokens.push_back(matchNumber());
            } else {
                tokens.push_back(makeOneCharToken(TokenType::SYMBOL));
            }
            break;
        }
        case '=': {
            tokens.push_back(makeOneCharToken(TokenType::SYMBOL));
            break;
        }
        case '>': {
            if (match('=')) {
                advance();
                tokens.push_back(makeToken(TokenType::SYMBOL, m_currentIndex - 2, 2));
            } else {
                tokens.push_back(makeOneCharToken(TokenType::SYMBOL));
            }
            break;
        }
        case '<': {
            if (match('=')) {
                advance();
                tokens.push_back(makeToken(TokenType::SYMBOL, m_currentIndex - 2, 2));
            } else {
                tokens.push_back(makeOneCharToken(TokenType::SYMBOL));
            }
            break;
        }
        case '"': {
            tokens.push_back(matchString());
            break;
        }
        case ';': {
            skipComment();
            break;
        }
        default: {
            tokens.push_back(matchEverythingElse(currentSymbol == ':'));
            break;
        }
        }
    }
    tokens.push_back(makeToken(TokenType::LAST_TOKEN, 0, 0));
    return tokens;
}

char Lexer::advance()
{
    return m_program[m_currentIndex++];
}

Token Lexer::makeToken(TokenType type, size_t startPos, size_t tokenLen) const
{
    return { type, m_program.substr(startPos, tokenLen) };
}

bool Lexer::match(char symbol) const
{
    if (isEnd()) {
        return false;
    }
    return m_program[m_currentIndex] == symbol;
}

char Lexer::peek() const
{
    return m_program[m_currentIndex];
}

bool Lexer::isEnd() const
{
    return m_currentIndex == m_program.size();
}

Token Lexer::matchString()
{
    const auto startPos = m_currentIndex - 1;
    int num = 0;
    // TODO: try to come up with other solution, this is not really readable
    while (!isEnd()) {
        //  Two backslashes will be transformed to one`//` -> `/`,
        //  so we don't want to treat transformed backslash as an escape char.
        num = m_program[m_currentIndex - 1] == '\\' ? num + 1 : 0;
        if (!(peek() != '"' || ((num % 2 != 0) && m_program[m_currentIndex - 1] == '\\')))
            break;
        advance();
    }

    auto tokenType = TokenType::ERROR_UNTERMINATED_STRING;
    if (match('"')) {
        advance();
        tokenType = TokenType::STRING;
    }
    return makeToken(tokenType, startPos, m_currentIndex - startPos);
}

void Lexer::skipComment()
{
    while (!isEnd() && peek() != '\n') {
        advance();
    }
}

Token Lexer::matchNumber()
{
    const auto startPos = m_currentIndex - 1;
    // TODO: handel errors
    while (!isEnd() && isdigit(peek())) {
        advance();
    }
    return makeToken(TokenType::NUMBER, startPos, m_currentIndex - startPos);
}

Token Lexer::matchEverythingElse(bool isKeyword)
{
    using namespace std::literals;
    const auto startPos = m_currentIndex - 1;
    auto isSeparator = [](char c) { return " )]},\n"sv.find(c) != std::string_view::npos; };

    while (!isEnd() && !isSeparator(peek())) {
        advance();
    }

    TokenType symbolType = isKeyword ? TokenType::KEYWORD : TokenType::SYMBOL;
    
    auto is = [this, startPos](std::string_view wordToMatch) {
        return wordToMatch == m_program.substr(startPos, m_currentIndex - startPos);
    };
    if (is("true") || is("false")) {
        symbolType = TokenType::BOOLEAN;
    } else if (is("nil")) {
        symbolType = TokenType::NIL;
    }

    return makeToken(symbolType, startPos, m_currentIndex - startPos);
}
} // namespace mal
