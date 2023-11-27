#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "compiler.h"
#include "scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

/**
 * @brief Stores the state of a parser.
 */
typedef struct
{
    Token current;
    Token previous;
    bool hadError;
    bool panic;
} Parser;

/**
 * @brief Enumerates levels of precedence.
 */
typedef enum
{
    PRECEDENCE_NONE,
    PRECEDENCE_ASSIGNMENT,
    PRECEDENCE_OR,
    PRECEDENCE_AND,
    PRECEDENCE_EQUALITY,
    PRECEDENCE_COMPARISON,
    PRECEDENCE_TERM,
    PRECEDENCE_FACTOR,
    PRECEDENCE_UNARY,
    PRECEDENCE_CALL,
    PRECEDENCE_PRIMARY,

} Precedence;

/**
 * @brief Represents a parsing function.
 */
typedef void (*ParseFn)();

/**
 * @brief Stores a parsing rule for an operation.
 */
typedef struct
{
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

static void CompileExpression();
static void CompileNumber();
static void CompileGrouping();
static void CompileUnary();
static void CompileBinary();

static void ParsePrecedence(Precedence precedence);
static ParseRule* GetParseRule(TokenType type);

static void NextToken();
static void ConsumeToken(TokenType type, const char* message);

static void EmitByte(uint8_t byte);
static void EmitTwoBytes(uint8_t byte1, uint8_t byte2);
static void EmitConstant(Value value);
static void EmitReturn();

static uint8_t MakeConstant(Value value);

static Chunk* CurrentChunk();
static void EndCompiler();

static void Error(const char* message);
static void ErrorAt(Token* token, const char* message);
static void ErrorAtCurrent(const char* message);

/**
 * @brief Contains all parsing rules.
 */
ParseRule rules[] =
{
    [TOKEN_LEFT_PARENTHESES]    = {CompileGrouping, NULL,          PRECEDENCE_NONE},
    [TOKEN_RIGHT_PARENTHESES]   = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_LEFT_BRACE]          = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_RIGHT_BRACE]         = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_COMMA]               = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_DOT]                 = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_MINUS]               = {CompileUnary,    CompileBinary, PRECEDENCE_TERM},
    [TOKEN_PLUS]                = {NULL,            CompileBinary, PRECEDENCE_TERM},
    [TOKEN_SEMICOLON]           = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_SLASH]               = {NULL,            CompileBinary, PRECEDENCE_FACTOR},
    [TOKEN_STAR]                = {NULL,            CompileBinary, PRECEDENCE_FACTOR},
    [TOKEN_BANG]                = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_BANG_EQUAL]          = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_EQUAL]               = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_EQUAL_EQUAL]         = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_GREATER]             = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_GREATER_EQUAL]       = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_LESS]                = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_LESS_EQUAL]          = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_IDENTIFIER]          = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_STRING]              = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_NUMBER]              = {CompileNumber,   NULL,          PRECEDENCE_NONE},
    [TOKEN_AND]                 = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_CLASS]               = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_ELSE]                = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_FALSE]               = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_FOR]                 = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_FUN]                 = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_IF]                  = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_NIL]                 = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_OR]                  = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_PRINT]               = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_RETURN]              = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_SUPER]               = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_THIS]                = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_TRUE]                = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_VAR]                 = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_WHILE]               = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_ERROR]               = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_EOF]                 = {NULL,            NULL,          PRECEDENCE_NONE},
};

Parser parser;
Chunk* compilingChunk;

bool Compile(const char* source, Chunk* chunk)
{
    InitScanner(source);
    compilingChunk = chunk;

    parser.hadError = false;
    parser.panic = false;

    NextToken();
    CompileExpression();
    ConsumeToken(TOKEN_EOF, "Expect end of expression.");
    EndCompiler();
    return !parser.hadError;
}

/**
 * @brief Compiles an expression into the current Chunk.
 */
static void CompileExpression()
{
    ParsePrecedence(PRECEDENCE_ASSIGNMENT);
}

/**
 * @brief Compiles a number into the current Chunk.
 */
static void CompileNumber()
{
    double value = strtod(parser.previous.start, NULL);
    EmitConstant(NUMBER_VALUE(value));
}

/**
 * @brief Compiles a grouping into the current Chunk.
 */
static void CompileGrouping()
{
    CompileExpression();
    ConsumeToken(TOKEN_LEFT_PARENTHESES, "Expect ')' after expression.");
}

/**
 * @brief Compiles an unary expression into the current Chunk.
 */
static void CompileUnary()
{
    TokenType op = parser.previous.type;

    // Compile the operand
    ParsePrecedence(PRECEDENCE_UNARY);

    // Emit the operator's instruction
    switch (op)
    {
        case TOKEN_MINUS:
            EmitByte(OP_NEGATE);
            break;
        // Unknown operator
        default:
            return;
    }
}

/**
 * @brief Compiles a binary expression into the current Chunk.
 */
static void CompileBinary()
{
    TokenType op = parser.previous.type;
    ParseRule* rule = GetParseRule(op);
    ParsePrecedence((Precedence)(rule->precedence + 1));

    switch (op)
    {
        case TOKEN_PLUS:
            EmitByte(OP_ADD);
            break;
        case TOKEN_MINUS:
            EmitByte(OP_SUBTRACT);
            break;
        case TOKEN_STAR:
            EmitByte(OP_MULTIPLY);
            break;
        case TOKEN_SLASH:
            EmitByte(OP_DIVIDE);
            break;
        // Unknown operator
        default:
            return;
    }
}

/**
 * @brief Parses a Precedence enum to determine what type of expression to parse next.
 * 
 * @param precedence A Precedence enum.
 */
static void ParsePrecedence(Precedence precedence)
{
    NextToken();

    // Attempt to parse out a prefix
    ParseFn prefixRule = GetParseRule(parser.previous.type)->prefix;
    if (prefixRule == NULL)
    {
        Error("Expect expression.");
        return;
    }
    else
    {
        prefixRule();
    }

    // Attempt to parse out an infix
    while (precedence <= GetParseRule(parser.current.type)->precedence)
    {
        NextToken();
        ParseFn infixRule = GetParseRule(parser.previous.type)->infix;
        infixRule();
    }
}

/**
 * @brief Get the ParseRule for a specified TokenType.
 * 
 * @param type A TokenType to get a parsing rule for.
 * @return ParseRule The corresponding ParseRule.
 */
static ParseRule* GetParseRule(TokenType type)
{
    return &rules[type];
}

/**
 * @brief Consumes the next Token in the stream.
 */
static void NextToken()
{
    parser.previous = parser.current;

    while (1)
    {
        parser.current = ScanToken();
        if (parser.current.type != TOKEN_ERROR)
        {
            break;
        }

        ErrorAtCurrent(parser.current.start);
    }
}

/**
 * @brief Attempts to consume a Token and throws an error if the received Token does not match the expected TokenType.
 * 
 * @param type An expected TokenType.
 * @param message An error message to report if the expected TokenType does not match the received TokenType.
 */
static void ConsumeToken(TokenType type, const char* message)
{
    if (parser.current.type == type)
    {
        NextToken();
        return;
    }
    else
    {
        ErrorAtCurrent(message);
    }
}

/**
 * @brief Appends a single byte to the current Chunk.
 * 
 * @param byte A byte to append.
 */
static void EmitByte(uint8_t byte)
{
    WriteChunk(CurrentChunk(), byte, parser.previous.line);
}

/**
 * @brief Appends two bytes to the current Chunk.
 * 
 * @param byte1 The first byte to append.
 * @param byte2 The second byte to append.
 */
static void EmitTwoBytes(uint8_t byte1, uint8_t byte2)
{
    EmitByte(byte1);
    EmitByte(byte2);
}

/**
 * @brief Appends a constant to the current Chunk.
 * 
 * @param value A Value to add as a constant.
 */
static void EmitConstant(Value value)
{
    EmitTwoBytes(OP_CONSTANT, MakeConstant(value));
}

/**
 * @brief Emits a return operation byte.
 */
static void EmitReturn()
{
    EmitByte(OP_RETURN);
}

/**
 * @brief Converts a Value to a constant and adds it to the current Chunk.
 * 
 * @param value A Value to convert.
 * @return uint8_t The resulting constant's index within the current Chunk.
 */
static uint8_t MakeConstant(Value value)
{
    int constant = AddConstant(CurrentChunk(), value);
    if (constant > UINT8_MAX)
    {
        Error("Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t)constant;
}

/**
 * @brief Returns the Chunk that is currently being compiled.
 * 
 * @return Chunk* A Chunk that is currently being compiled.
 */
static Chunk* CurrentChunk()
{
    return compilingChunk;
}

/**
 * @brief Ends the current compilation.
 */
static void EndCompiler()
{
    EmitReturn();
#ifdef DEBUG_PRINT_CODE
    if (!parser.hadError)
    {
        DisassembleChunk(CurrentChunk(), "code");
    }
#endif
}

/**
 * @brief Reports an error at the previous Token.
 * 
 * @param message An error message.
 */
static void Error(const char* message)
{
    ErrorAt(&parser.previous, message);
}

/**
 * @brief Reports an error at a Token.
 * 
 * @param token The Token where the error occurred.
 * @param message An error message.
 */
static void ErrorAt(Token* token, const char* message)
{
    // If we're already panicking, don't report another error
    if (parser.panic)
    {
        return;
    }
    else
    {
        parser.panic = true;
    }

    // Report line
    fprintf(stderr, "[line %d] Error", token->line);

    // Add positional indicator
    switch (token->type)
    {
        case TOKEN_EOF:
            fprintf(stderr, " at end");
            break;
        case TOKEN_ERROR:
            break;
        default:
            fprintf(stderr, " at '%.*s'", token->length, token->start);
            break;
    }

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

/**
 * @brief Reports an error at the current Token.
 * 
 * @param message An error message.
 */
static void ErrorAtCurrent(const char* message)
{
    ErrorAt(&parser.current, message);
}
