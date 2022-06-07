#pragma once

#include <string_view>
#include <vector>
#include <ostream>

namespace mal {

// TODO: delete unused tokens
#define TOKEN_TYPE_ENUM(VARIANT) \
    VARIANT(LEFT_SQUARE_BRACE)\
    VARIANT(RIGHT_SQUARE_BACE) \
    VARIANT(LEFT_CURLY_BRACE) \
    VARIANT(RIGHT_CURLY_BRACE) \
    VARIANT(LEFT_PAREN) \
    VARIANT(RIGHT_PAREN) \
    VARIANT(APOSTROPHE) \
    VARIANT(SINGLE_QUOTE) \
    VARIANT(SEMICOLON) \
    VARIANT(TILDE) \
    VARIANT(CARET) \
    VARIANT(AT) \
    VARIANT(DOUBLE_STAR) \
    VARIANT(EQUAL) \
    VARIANT(GREATER_THAN) \
    VARIANT(GREATER_THAN_EQUAL) \
    VARIANT(LESS_THAN) \
    VARIANT(LESS_THAN_EQUAL) \
    VARIANT(TILDE_AT) /* ~@ */ \
    VARIANT(STRING) \
    VARIANT(NUMBER) \
    VARIANT(ERROR_UNTERMINATED_STRING) \
    VARIANT(KEYWORD) \
    VARIANT(BOOLEAN) \
    VARIANT(NIL) \
    VARIANT(SYMBOL) /* all other tokens that are not implementd yet*/ \
    VARIANT(LAST_TOKEN) \

#define TOKEN_TYPE_VARIANT(NAME) NAME,
enum class TokenType : char {
    TOKEN_TYPE_ENUM(TOKEN_TYPE_VARIANT)
};
#undef TOKEN_TYPE_VARIANT

std::ostream& operator<<(std::ostream& stream, TokenType type);

struct Token {
    TokenType type;
    std::string_view token;
};

class Lexer {
public:
    Lexer(std::string_view program);
    std::vector<Token> tokenize();

private:
    char advance();
    bool match(char symbol) const;
    char peek() const;
    bool isEnd() const;


    Token makeToken(TokenType type, size_t startPos, size_t tokenLen) const;
    Token matchString();
    Token matchSemicolon();
    Token matchNumber();
    Token matchSymbol(bool isKeyword = false);

private:
    size_t m_currentIndex { 0 };
    std::string_view m_program;
};

}