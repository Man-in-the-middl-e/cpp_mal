#include "lexer.h"

#include <assert.h>
#include <ctype.h>
#include <iostream>

namespace mal {

const char* printToken(TokenType type){
    #define TOKEN_TYPE_STR(NAME) case TokenType::NAME : return #NAME;
    switch(type) {
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

        if (isalpha(currentSymbol)) {
            tokens.push_back(matchIdentifier());
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
        case '\'': {
            tokens.push_back(makeOneCharToken(TokenType::SINGLE_QUOTE));
            break;
        }
        case '`': {
            tokens.push_back(makeOneCharToken(TokenType::APOSTROPHE));
            break;
        }
        case '~': {
            if (match('@')) {
                advance();
                tokens.push_back(makeToken(TokenType::TILDE_AT, m_currentIndex - 2, 2));
                break;
            }
            tokens.push_back(makeOneCharToken(TokenType::TILDE));
            break;
        }
        case '*': {
            if (match('*')) {
                advance();
                tokens.push_back(makeToken(TokenType::DOUBLE_STAR, m_currentIndex - 2, 2));
                break;
            }
            tokens.push_back(makeOneCharToken(TokenType::MULT));
            break;
        }
        case '^': {
            tokens.push_back(makeOneCharToken(TokenType::CARET));
            break;
        }
        case '@': {
            tokens.push_back(makeOneCharToken(TokenType::AT));
            break;
        }
        case '-': {
            if (isdigit(peek())) {
                tokens.push_back(matchNumber());
            } else {
                tokens.push_back(makeOneCharToken(TokenType::MINUS));
            }
            break;
        }
        case '+': {
            tokens.push_back(makeOneCharToken(TokenType::PLUS));
            break;
        }
        case '=': {
            tokens.push_back(makeOneCharToken(TokenType::EQUAL));
            break;
        }
        case '>': {
            tokens.push_back(makeOneCharToken(TokenType::GREATER_THAN));
            break;
        }
        case '<': {
            tokens.push_back(makeOneCharToken(TokenType::LESS_THAN));
            break;
        }
        case ':': {
            tokens.push_back(matchIdentifier(true));
            break;
        }
        case '"': {
            tokens.push_back(matchString());
            break;
        }
        case ';': {
            tokens.push_back(matchSemicolon());
            break;
        }
        default: {
            std::cout << "Unimplmented token: `" << currentSymbol << "`" << std::endl;
            assert(false);
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
        num = m_program[m_currentIndex -1] == '\\' ? num + 1 : 0;
        if (!(peek() !='"' || ((num % 2 != 0) && m_program[m_currentIndex - 1] == '\\')))
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

Token Lexer::matchSemicolon()
{
    const auto startPos = m_currentIndex - 1;
    while (!isEnd()) {
        advance();
    }
    return makeToken(TokenType::SEMICOLON, startPos, m_currentIndex - startPos);
}

Token Lexer::matchIdentifier(bool isKeyword)
{
    const auto startPos = m_currentIndex - 1;
    auto isIdentifierSymbol = [](char c) {
        return c == '-' || c == '_' || isalpha(c) || isdigit(c);
    };
    while (!isEnd() && isIdentifierSymbol(peek())) {
        advance();
    }
    return makeToken(isKeyword ? TokenType::KEYWORD : TokenType::IDENTIFIER,
                    startPos, m_currentIndex - startPos);
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

Token Lexer::mathcSymbol()
{
    const auto startPos = m_currentIndex - 1;

    while (!isEnd() && peek() != ' ' && peek() != ')' && peek() != ',') {
        advance();
    }
    return makeToken(TokenType::SYMBOL, startPos, m_currentIndex - startPos);
}
} // namespace mal