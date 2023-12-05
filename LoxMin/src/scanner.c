#include <stdio.h>
#include <string.h>
#include "common.h"
#include "scanner.h"

static bool IsAtEnd();
static Token MakeToken(TokenType type);
static Token ErrorToken(const char* message);
static char NextCharacter();
static bool MatchCharacter(char expected);
static char PeekCharacter();
static char PeekNextCharacter();
static void SkipWhitespace();
static Token ReadString();
static Token ReadNumber();
static Token ReadIdentifier();
static bool IsAlpha(char c);
static bool IsDigit(char c);
static TokenType IdentifierType();
static TokenType CheckKeyword(int start, int length, const char* rest, TokenType type);

/**
 * @brief Stores the state of a scanner.
 */
typedef struct
{
    const char* start;
    const char* current;
    int line;
} Scanner;

Scanner scanner;

void InitScanner(const char* source)
{
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

Token ScanToken()
{
    SkipWhitespace();
    scanner.start = scanner.current;

    if (IsAtEnd())
    {
        return MakeToken(TOKEN_EOF);
    }

    char c = NextCharacter();
    if (IsAlpha(c))
    {
        return ReadIdentifier();
    }
    else if (IsDigit(c))
    {
        return ReadNumber();
    }

    switch (c)
    {
        case '(':
            return MakeToken(TOKEN_LEFT_PARENTHESES);
        case ')':
            return MakeToken(TOKEN_RIGHT_PARENTHESES);
        case '{':
            return MakeToken(TOKEN_LEFT_BRACE);
        case '}':
            return MakeToken(TOKEN_RIGHT_BRACE);
        case ';':
            return MakeToken(TOKEN_SEMICOLON);
        case ',':
            return MakeToken(TOKEN_COMMA);
        case '.':
            return MakeToken(TOKEN_DOT);
        case '-':
            return MakeToken(TOKEN_MINUS);
        case '+':
            return MakeToken(TOKEN_PLUS);
        case '/':
            return MakeToken(TOKEN_SLASH);
        case '*':
            return MakeToken(TOKEN_STAR);
        case '!':
            return MakeToken(MatchCharacter('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=':
            return MakeToken(MatchCharacter('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<':
            return MakeToken(MatchCharacter('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>':
            return MakeToken(MatchCharacter('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
        case '"':
            return ReadString();
    }

    return ErrorToken("Unexpected character.");
}

/**
 * @brief Checks if the Scanner has reached the end of the source code.
 * 
 * @return true The Scanner has reached the end of the source code.
 * @return false The Scanner has not yet reached the end of the source code.
 */
static bool IsAtEnd()
{
    return *scanner.current == '\0';
}

/**
 * @brief Creates a Token from a given TokenType.
 * 
 * @param type The TokenType of the Token to make.
 * @return Token A resulting Token.
 */
static Token MakeToken(TokenType type)
{
    Token token;

    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;

    return token;
}

/**
 * @brief Creates an error Token with a given message.
 * 
 * @param message A message to describe the error.
 * @return Token A resulting Token.
 */
static Token ErrorToken(const char* message)
{
    Token token;

    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner.line;

    return token;
}

/**
 * @brief Fetches the next character from the source string.
 * 
 * @return char A fetched character.
 */
static char NextCharacter()
{
    return *(scanner.current++);
}

/**
 * @brief Checks if the next character matches an expected character, and advances the stream if so.
 * 
 * @param expected A character to expect.
 * @return true The next and expected characters match.
 * @return false The next and expected characters do not match.
 */
static bool MatchCharacter(char expected)
{
    if (IsAtEnd())
    {
        return false;
    }
    else if (*scanner.current != expected)
    {
        return false;
    }
    else
    {
        scanner.current++;
        return true;
    }
}

/**
 * @brief Looks ahead at the next character without advancing the stream.
 * 
 * @return char The next character in the stream.
 */
static char PeekCharacter()
{
    return *scanner.current;
}

/**
 * @brief Looks ahead at the character after the next without advancing the stream.
 * 
 * @return char The character after the next in the stream.
 */
static char PeekNextCharacter()
{
    if (IsAtEnd())
    {
        return '\0';
    }
    else
    {
        return *(scanner.current + 1);
    }
}

/**
 * @brief Skips the next pieces of whitespace in the source string.
 */
static void SkipWhitespace()
{
    while (1)
    {
        char c = PeekCharacter();
        switch (c)
        {
            // Generic whitespace
            case ' ':
            case '\r':
            case '\t':
                NextCharacter();
                break;
            // Newline
            case '\n':
                scanner.line++;
                NextCharacter();
                break;
            // Comments
            case '/':
                if (PeekNextCharacter() == '/')
                {
                    while (PeekCharacter() != '\n' && !IsAtEnd())
                    {
                        NextCharacter();
                    }
                }
                else
                {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

/**
 * @brief Reads a string into a Token.
 * 
 * @return Token A string Token.
 */
static Token ReadString()
{
    while (PeekCharacter() != '"' && !IsAtEnd())
    {
        if (PeekCharacter() == '\n')
        {
            scanner.line++;
        }
        NextCharacter();
    }

    if (IsAtEnd())
    {
        return ErrorToken("Unterminated string.");
    }

    NextCharacter();
    return MakeToken(TOKEN_STRING);
}

/**
 * @brief Reads a number into a Token.
 * 
 * @return Token A number Token.
 */
static Token ReadNumber()
{
    while (IsDigit(PeekCharacter()))
    {
        NextCharacter();
    }

    // Check for decimal point
    if (PeekCharacter() == '.' && IsDigit(PeekNextCharacter()))
    {
        // Consume decimal point
        NextCharacter();
        while (IsDigit(PeekCharacter()))
        {
            NextCharacter();
        }
    }

    return MakeToken(TOKEN_NUMBER);
}

/**
 * @brief Reads an identifier into a Token.
 * 
 * @return Token An identifier Token.
 */
static Token ReadIdentifier()
{
    while (IsAlpha(PeekCharacter()) || IsDigit(PeekCharacter()))
    {
        NextCharacter();
    }
    return MakeToken(IdentifierType());
}

/**
 * @brief Checks if a character is alphabetic.
 * 
 * @param c A character to check.
 * @return true The given character is alphabetic.
 * @return false The given character is not alphabetic.
 */
static bool IsAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

/**
 * @brief Checks if a character is a numeric digit.
 * 
 * @param c A character to check.
 * @return true The given character is a numeric digit.
 * @return false The given character is not a numeric digit.
 */
static bool IsDigit(char c)
{
    return c >= '0' && c <= '9';
}

/**
 * @brief Determines the type of an identifier.
 * 
 * @return TokenType The type of the identifier.
 */
static TokenType IdentifierType()
{
    switch (scanner.start[0])
    {
        case 'a':
            return CheckKeyword(1, 2, "nd", TOKEN_AND);
        case 'c':
            return CheckKeyword(1, 4, "lass", TOKEN_CLASS);
        case 'e':
            return CheckKeyword(1, 3, "lse", TOKEN_ELSE);
        case 'f':
            if (scanner.current - scanner.start > 1)
            {
                switch (scanner.start[1])
                {
                    case 'a':
                        return CheckKeyword(2, 3, "lse", TOKEN_FALSE);
                    case 'o':
                        return CheckKeyword(2, 1, "r", TOKEN_FOR);
                    case 'u':
                        return CheckKeyword(2, 1, "n", TOKEN_FUN);
                }
            }
            break;
        case 'i':
            return CheckKeyword(1, 1, "f", TOKEN_IF);
        case 'n':
            return CheckKeyword(1, 2, "il", TOKEN_NIL);
        case 'o':
            return CheckKeyword(1, 1, "r", TOKEN_OR);
        case 'p':
            return CheckKeyword(1, 4, "rint", TOKEN_PRINT);
        case 'r':
            return CheckKeyword(1, 5, "eturn", TOKEN_RETURN);
        case 's':
            return CheckKeyword(1, 4, "uper", TOKEN_SUPER);
        case 't':
            if (scanner.current - scanner.start > 1)
            {
                switch (scanner.start[1])
                {
                    case 'h':
                        return CheckKeyword(2, 2, "is", TOKEN_THIS);
                    case 'r':
                        return CheckKeyword(2, 2, "ue", TOKEN_TRUE);
                }
            }
            break;
        case 'v':
            return CheckKeyword(1, 2, "ar", TOKEN_VAR);
        case 'w':
            return CheckKeyword(1, 4, "hile", TOKEN_WHILE);
    }

    return TOKEN_IDENTIFIER;
}

/**
 * @brief Checks if an identifier matches a keyword.
 * 
 * @param start The number of characters to start into the source string.
 * @param length The length of the search string.
 * @param rest The search string, being the rest of a keyword.
 * @param type If the identifier matches the keyword, this type is returned.
 * @return TokenType The type of the read Token.
 */
static TokenType CheckKeyword(int start, int length, const char* rest, TokenType type)
{
    if ((scanner.current - scanner.start == start + length) && (memcmp(scanner.start + start, rest, length) == 0))
    {
        return type;
    }
    else
    {
        return TOKEN_IDENTIFIER;
    }
}
