#ifndef loxmin_scanner_h
#define loxmin_scanner_h

/**
 * @brief Enumerates all Token types.
 */
typedef enum
{
    // Single-character tokens
    TOKEN_LEFT_PARENTHESES,
    TOKEN_RIGHT_PARENTHESES,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_MINUS,
    TOKEN_PLUS,
    TOKEN_SEMICOLON,
    TOKEN_SLASH,
    TOKEN_STAR,

    // Comparison, variable-length tokens
    TOKEN_BANG,
    TOKEN_BANG_EQUAL,
    TOKEN_EQUAL,
    TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,

    // Literal tokens
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_NUMBER,

    // Keyword tokens
    TOKEN_AND,
    TOKEN_CLASS,
    TOKEN_ELSE,
    TOKEN_FALSE,
    TOKEN_FOR,
    TOKEN_FUN,
    TOKEN_IF,
    TOKEN_NIL,
    TOKEN_OR,
    TOKEN_PRINT,
    TOKEN_RETURN,
    TOKEN_SUPER,
    TOKEN_THIS,
    TOKEN_TRUE,
    TOKEN_VAR,
    TOKEN_WHILE,

    // Special tokens
    TOKEN_ERROR,
    TOKEN_EOF,
} TokenType;

/**
 * @brief Stores a scanned piece of code and information about it.
 */
typedef struct
{
    TokenType type;
    const char* start;
    int length;
    int line;
} Token;

/**
 * @brief Initializes a Scanner on a piece of source code.
 * 
 * @param source A source code string.
 */
void InitScanner(const char* source);

/**
 * @brief Scans in a Token from source code.
 * 
 * @return Token The scanned Token.
 */
Token ScanToken();

#endif