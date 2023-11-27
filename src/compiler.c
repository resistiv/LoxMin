#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "compiler.h"
#include "scanner.h"

static void NextToken();
static void ConsumeToken(TokenType type, const char* message);

static void EmitByte(uint8_t byte);
static void EmitTwoBytes(uint8_t byte1, uint8_t byte2);
static void EmitReturn();

static Chunk* CurrentChunk();
static void EndCompiler();

static void Error(const char* message);
static void ErrorAt(Token* token, const char* message);
static void ErrorAtCurrent(const char* message);

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

Parser parser;
Chunk* compilingChunk;

bool Compile(const char* source, Chunk* chunk)
{
    InitScanner(source);
    compilingChunk = chunk;

    parser.hadError = false;
    parser.panic = false;

    NextToken();
    Expression();
    ConsumeToken(TOKEN_EOF, "Expect end of expression.");
    EndCompiler();
    return !parser.hadError;
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
 * @brief Emits a return operation byte.
 */
static void EmitReturn()
{
    EmitByte(OP_RETURN);
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
