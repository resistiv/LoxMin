#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "compiler.h"
#include "memory.h"
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
typedef void (*ParseFn)(bool canAssign);

/**
 * @brief Stores a parsing rule for an operation.
 */
typedef struct
{
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

/**
 * @brief Stores information about a local variable. 
 */
typedef struct
{
    Token name;
    int depth;
    bool isCaptured;
} Local;

/**
 * @brief Stores information about an upvalue.
 */
typedef struct 
{
    uint8_t index;
    bool isLocal;
} Upvalue;

/**
 * @brief Enumerates all function types.
 */
typedef enum
{
    TYPE_FUNCTION,
    TYPE_INITIALIZER,
    TYPE_METHOD,
    TYPE_SCRIPT,
} FunctionType;

/**
 * @brief Stores the state of a compiler.
 */
typedef struct Compiler
{
    struct Compiler* enclosing;
    ObjectFunction* function;
    FunctionType type;

    Local locals[UINT8_COUNT];
    int localCount;
    Upvalue upvalues[UINT8_COUNT];
    int scopeDepth;
} Compiler;

/**
 * @brief Stores the state of a class compiler.
 */
typedef struct ClassCompiler
{
    struct ClassCompiler* enclosing;
} ClassCompiler;

static void CompileExpression();
static void CompileDeclaration();
static void CompileClassDeclaration();
static void CompileFunctionDeclaration();
static void CompileVariableDeclaration();
static void CompileStatement();
static void CompileExpressionStatement();
static void CompilePrint();
static void CompileBlock();
static void CompileIfStatement();
static void CompileReturnStatement();
static void CompileWhileStatement();
static void CompileForStatement();
static void CompileFunction(FunctionType type);
static void CompileMethod();
static void CompileNumber(bool canAssign);
static void CompileGrouping(bool canAssign);
static void CompileUnary(bool canAssign);
static void CompileBinary(bool canAssign);
static void CompileCall(bool canAssign);
static uint8_t CompileArgumentList();
static void CompileAnd(bool canAssign);
static void CompileOr(bool canAssign);
static void CompileLiteral(bool canAssign);
static void CompileString(bool canAssign);
static void CompileVariable(bool canAssign);
static void CompileNamedVariable(Token name, bool canAssign);
static void CompileDot(bool canAssign);
static void CompileThis(bool canAssign);

static void ParsePrecedence(Precedence precedence);
static ParseRule* GetParseRule(TokenType type);
static uint8_t ParseVariable(const char* errorMessage);
static void DeclareVariable();
static void DefineVariable(uint8_t global);
static void AddLocalVariable(Token name);
static int ResolveLocalVariable(Compiler* compiler, Token* name);
static int AddUpvalue(Compiler* compiler, uint8_t index, bool isLocal);
static int ResolveUpvalue(Compiler* compiler, Token* name);
static void MarkInitialized();

static void NextToken();
static void ConsumeToken(TokenType type, const char* message);
static bool MatchToken(TokenType type);
static bool CheckToken(TokenType type);
static void SynchronizeState();

static void EmitByte(uint8_t byte);
static void EmitTwoBytes(uint8_t byte1, uint8_t byte2);
static int EmitJump(uint8_t instruction);
static void PatchJump(int offset);
static void EmitLoop(int loopStart);
static void EmitConstant(Value value);
static void EmitReturn();

static uint8_t MakeConstant(Value value);
static uint8_t MakeIdentifierConstant(Token* name);
static bool AreIdentifiersEqual(Token* a, Token* b);

static Chunk* CurrentChunk();
static void InitCompiler(Compiler* compiler, FunctionType type);
static ObjectFunction* EndCompiler();
static void BeginScope();
static void EndScope();

static void Error(const char* message);
static void ErrorAt(Token* token, const char* message);
static void ErrorAtCurrent(const char* message);

/**
 * @brief Contains all parsing rules.
 */
ParseRule rules[] =
{
    [TOKEN_LEFT_PARENTHESES]    = {CompileGrouping, CompileCall,   PRECEDENCE_CALL},
    [TOKEN_RIGHT_PARENTHESES]   = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_LEFT_BRACE]          = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_RIGHT_BRACE]         = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_COMMA]               = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_DOT]                 = {NULL,            CompileDot,    PRECEDENCE_CALL},
    [TOKEN_MINUS]               = {CompileUnary,    CompileBinary, PRECEDENCE_TERM},
    [TOKEN_PLUS]                = {NULL,            CompileBinary, PRECEDENCE_TERM},
    [TOKEN_SEMICOLON]           = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_SLASH]               = {NULL,            CompileBinary, PRECEDENCE_FACTOR},
    [TOKEN_STAR]                = {NULL,            CompileBinary, PRECEDENCE_FACTOR},
    [TOKEN_BANG]                = {CompileUnary,    NULL,          PRECEDENCE_NONE},
    [TOKEN_BANG_EQUAL]          = {NULL,            CompileBinary, PRECEDENCE_EQUALITY},
    [TOKEN_EQUAL]               = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_EQUAL_EQUAL]         = {NULL,            CompileBinary, PRECEDENCE_EQUALITY},
    [TOKEN_GREATER]             = {NULL,            CompileBinary, PRECEDENCE_COMPARISON},
    [TOKEN_GREATER_EQUAL]       = {NULL,            CompileBinary, PRECEDENCE_COMPARISON},
    [TOKEN_LESS]                = {NULL,            CompileBinary, PRECEDENCE_COMPARISON},
    [TOKEN_LESS_EQUAL]          = {NULL,            CompileBinary, PRECEDENCE_COMPARISON},
    [TOKEN_IDENTIFIER]          = {CompileVariable, NULL,          PRECEDENCE_NONE},
    [TOKEN_STRING]              = {CompileString,   NULL,          PRECEDENCE_NONE},
    [TOKEN_NUMBER]              = {CompileNumber,   NULL,          PRECEDENCE_NONE},
    [TOKEN_AND]                 = {NULL,            CompileAnd,    PRECEDENCE_AND},
    [TOKEN_CLASS]               = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_ELSE]                = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_FALSE]               = {CompileLiteral,  NULL,          PRECEDENCE_NONE},
    [TOKEN_FOR]                 = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_FUN]                 = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_IF]                  = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_NIL]                 = {CompileLiteral,  NULL,          PRECEDENCE_NONE},
    [TOKEN_OR]                  = {NULL,            CompileOr,     PRECEDENCE_OR},
    [TOKEN_PRINT]               = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_RETURN]              = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_SUPER]               = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_THIS]                = {CompileThis,     NULL,          PRECEDENCE_NONE},
    [TOKEN_TRUE]                = {CompileLiteral,  NULL,          PRECEDENCE_NONE},
    [TOKEN_VAR]                 = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_WHILE]               = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_ERROR]               = {NULL,            NULL,          PRECEDENCE_NONE},
    [TOKEN_EOF]                 = {NULL,            NULL,          PRECEDENCE_NONE},
};

Parser parser;
Compiler* current = NULL;
ClassCompiler* currentClass = NULL;

ObjectFunction* Compile(const char* source)
{
    InitScanner(source);
    Compiler compiler;
    InitCompiler(&compiler, TYPE_SCRIPT);

    parser.hadError = false;
    parser.panic = false;

    NextToken();
    
    while (!MatchToken(TOKEN_EOF))
    {
        CompileDeclaration();
    }

    ObjectFunction* function = EndCompiler();
    return parser.hadError ? NULL : function;
}

void MarkCompilerRoots()
{
    Compiler* compiler = current;
    while (compiler != NULL)
    {
        MarkObject((Object*)compiler->function);
        compiler = compiler->enclosing;
    }
}

/**
 * @brief Compiles an expression into the current Chunk.
 */
static void CompileExpression()
{
    ParsePrecedence(PRECEDENCE_ASSIGNMENT);
}

/**
 * @brief Compiles a declaration into the current Chunk.
 */
static void CompileDeclaration()
{
    if (MatchToken(TOKEN_CLASS))
    {
        CompileClassDeclaration();
    }
    else if (MatchToken(TOKEN_FUN))
    {
        CompileFunctionDeclaration();
    }
    else if (MatchToken(TOKEN_VAR))
    {
        CompileVariableDeclaration();
    }
    else
    {
        CompileStatement();
    }

    if (parser.panic)
    {
        SynchronizeState();
    }
}

/**
 * @brief Compiles a class declaration into the current Chunk.
 */
static void CompileClassDeclaration()
{
    ConsumeToken(TOKEN_IDENTIFIER, "Expect class name.");
    Token className = parser.previous;
    uint8_t nameConstant = MakeIdentifierConstant(&parser.previous);
    DeclareVariable();

    EmitTwoBytes(OP_CLASS, nameConstant);
    DefineVariable(nameConstant);

    ClassCompiler classCompiler;
    classCompiler.enclosing = currentClass;
    currentClass = &classCompiler;

    CompileNamedVariable(className, false);
    ConsumeToken(TOKEN_LEFT_BRACE, "Expect '{' before class body.");
    while (!CheckToken(TOKEN_RIGHT_BRACE) && !CheckToken(TOKEN_EOF))
    {
        CompileMethod();
    }
    ConsumeToken(TOKEN_RIGHT_BRACE, "Expect '}' after class body.");
    EmitByte(OP_POP);

    currentClass = currentClass->enclosing;
}

/**
 * @brief Compiles a function declaration into the current Chunk.
 */
static void CompileFunctionDeclaration()
{
    uint8_t global = ParseVariable("Expect function name.");
    MarkInitialized();
    CompileFunction(TYPE_FUNCTION);
    DefineVariable(global);
}

/**
 * @brief Compiles a variable declaration into the current Chunk.
 */
static void CompileVariableDeclaration()
{
    uint8_t global = ParseVariable("Expect variable name.");

    if (MatchToken(TOKEN_EQUAL))
    {
        CompileExpression();
    }
    else
    {
        EmitByte(OP_NIL);
    }
    ConsumeToken(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");

    DefineVariable(global);
}

/**
 * @brief Compiles a statement into the current Chunk.
 */
static void CompileStatement()
{
    if (MatchToken(TOKEN_PRINT))
    {
        CompilePrint();
    }
    else if (MatchToken(TOKEN_FOR))
    {
        CompileForStatement();   
    }
    else if (MatchToken(TOKEN_IF))
    {
        CompileIfStatement();
    }
    else if (MatchToken(TOKEN_RETURN))
    {
        CompileReturnStatement();
    }
    else if (MatchToken(TOKEN_WHILE))
    {
        CompileWhileStatement();
    }
    else if (MatchToken(TOKEN_LEFT_BRACE))
    {
        BeginScope();
        CompileBlock();
        EndScope();
    }
    else
    {
        CompileExpressionStatement();
    }
}

/**
 * @brief Compiles an expression into the current Chunk.
 */
static void CompileExpressionStatement()
{
    CompileExpression();
    ConsumeToken(TOKEN_SEMICOLON, "Expect ';' after expression.");
    EmitByte(OP_POP);
}

/**
 * @brief Compiles a print statement into the current Chunk.
 */
static void CompilePrint()
{
    CompileExpression();
    ConsumeToken(TOKEN_SEMICOLON, "Expect ';' after value.");
    EmitByte(OP_PRINT);
}

/**
 * @brief Compiles a block statement into the current Chunk.
 */
static void CompileBlock()
{
    while (!CheckToken(TOKEN_RIGHT_BRACE) && !CheckToken(TOKEN_EOF))
    {
        CompileDeclaration();
    }

    ConsumeToken(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

/**
 * @brief Compiles an if statement into the current Chunk.
 */
static void CompileIfStatement()
{
    ConsumeToken(TOKEN_LEFT_PARENTHESES, "Expect '(' after 'if'.");
    CompileExpression();
    ConsumeToken(TOKEN_RIGHT_PARENTHESES, "Expect ')' after condition.");

    int thenJump = EmitJump(OP_JUMP_IF_FALSE);
    EmitByte(OP_POP);
    CompileStatement();

    int elseJump = EmitJump(OP_JUMP);

    PatchJump(thenJump);
    EmitByte(OP_POP);

    if (MatchToken(TOKEN_ELSE))
    {
        CompileStatement();
    }
    PatchJump(elseJump);
}

/**
 * @brief Compiles a return statement.
 */
static void CompileReturnStatement()
{
    if (current->type == TYPE_SCRIPT)
    {
        Error("Can't return from top-level code.");
    }

    if (MatchToken(TOKEN_SEMICOLON))
    {
        EmitReturn();
    }
    else
    {
        if (current->type == TYPE_INITIALIZER)
        {
            Error("Can't return a value from an initializer.");
        }

        CompileExpression();
        ConsumeToken(TOKEN_SEMICOLON, "Expect ';' after return value.");
        EmitByte(OP_RETURN);
    }
}

/**
 * @brief Compiles a while loop into the current Chunk.
 */
static void CompileWhileStatement()
{
    int loopStart = CurrentChunk()->count;
    ConsumeToken(TOKEN_LEFT_PARENTHESES, "Expect '(' after 'while'.");
    CompileExpression();
    ConsumeToken(TOKEN_RIGHT_PARENTHESES, "Expect ')' after condition.");

    int exitJump = EmitJump(OP_JUMP_IF_FALSE);
    EmitByte(OP_POP);
    CompileStatement();
    EmitLoop(loopStart);

    PatchJump(exitJump);
    EmitByte(OP_POP);
}

/**
 * @brief Compiles a for loop into the current Chunk.
 */
static void CompileForStatement()
{
    BeginScope();
    ConsumeToken(TOKEN_LEFT_PARENTHESES, "Expect '(' after 'for'.");
    if (MatchToken(TOKEN_SEMICOLON))
    {
        // No initializer.
    }
    else if (MatchToken(TOKEN_VAR))
    {
        CompileVariableDeclaration();
    }
    else
    {
        CompileExpressionStatement();
    }

    int loopStart = CurrentChunk()->count;
    int exitJump = -1;
    if (!MatchToken(TOKEN_SEMICOLON))
    {
        CompileExpression();
        ConsumeToken(TOKEN_SEMICOLON, "Expect ';' after loop condition.");

        exitJump = EmitJump(OP_JUMP_IF_FALSE);
        EmitByte(OP_POP);
    }

    if (!MatchToken(TOKEN_RIGHT_PARENTHESES))
    {
        int bodyJump = EmitJump(OP_JUMP);
        int incrementStart = CurrentChunk()->count;
        CompileExpression();
        EmitByte(OP_POP);
        ConsumeToken(TOKEN_RIGHT_PARENTHESES, "Expect ')' after for clauses.");

        EmitLoop(loopStart);
        loopStart = incrementStart;
        PatchJump(bodyJump);
    }

    CompileStatement();
    EmitLoop(loopStart);

    if (exitJump != -1)
    {
        PatchJump(exitJump);
        EmitByte(OP_POP);
    }

    EndScope();
}

/**
 * @brief Compiles a function.
 * 
 * @param type The type of the function.
 */
static void CompileFunction(FunctionType type)
{
    Compiler compiler;
    InitCompiler(&compiler, type);
    BeginScope();

    ConsumeToken(TOKEN_LEFT_PARENTHESES, "Expect '(' after function name.");
    if (!CheckToken(TOKEN_RIGHT_PARENTHESES))
    {
        do
        {
            current->function->arity++;
            if (current->function->arity > 255)
            {
                ErrorAtCurrent("Can't have more than 255 parameters.");
            }
            uint8_t constant = ParseVariable("Expect parameter name.");
            DefineVariable(constant);
        } while (MatchToken(TOKEN_COMMA));
    }

    ConsumeToken(TOKEN_RIGHT_PARENTHESES, "Expect ')' after parameters.");
    ConsumeToken(TOKEN_LEFT_BRACE, "Expect '{' before function body.");
    CompileBlock();

    ObjectFunction* function = EndCompiler();
    EmitTwoBytes(OP_CLOSURE, MakeConstant(OBJECT_VALUE(function)));

    for (int i = 0; i < function->upvalueCount; i++)
    {
        EmitTwoBytes(compiler.upvalues[i].isLocal ? 1 : 0, compiler.upvalues[i].index);
    }
}

/**
 * @brief Compiles a method into the current Chunk.
 */
static void CompileMethod()
{
    ConsumeToken(TOKEN_IDENTIFIER, "Expect method name.");
    uint8_t constant = MakeIdentifierConstant(&parser.previous);

    FunctionType type = TYPE_METHOD;
    if (parser.previous.length == 4 && memcmp(parser.previous.start, "init", 4) == 0)
    {
        type = TYPE_INITIALIZER;
    }

    CompileFunction(type);
    EmitTwoBytes(OP_METHOD, constant);
}

/**
 * @brief Compiles a number into the current Chunk.
 */
static void CompileNumber(bool canAssign)
{
    double value = strtod(parser.previous.start, NULL);
    EmitConstant(NUMBER_VALUE(value));
}

/**
 * @brief Compiles a grouping into the current Chunk.
 */
static void CompileGrouping(bool canAssign)
{
    CompileExpression();
    ConsumeToken(TOKEN_RIGHT_PARENTHESES, "Expect ')' after expression.");
}

/**
 * @brief Compiles an unary expression into the current Chunk.
 */
static void CompileUnary(bool canAssign)
{
    TokenType op = parser.previous.type;

    // Compile the operand
    ParsePrecedence(PRECEDENCE_UNARY);

    // Emit the operator's instruction
    switch (op)
    {
        case TOKEN_BANG:
            EmitByte(OP_NOT);
            break;
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
static void CompileBinary(bool canAssign)
{
    TokenType op = parser.previous.type;
    ParseRule* rule = GetParseRule(op);
    ParsePrecedence((Precedence)(rule->precedence + 1));

    switch (op)
    {
        case TOKEN_BANG_EQUAL:
            EmitTwoBytes(OP_EQUAL, OP_NOT);
            break;
        case TOKEN_EQUAL_EQUAL:
            EmitByte(OP_EQUAL);
            break;
        case TOKEN_GREATER:
            EmitByte(OP_GREATER);
            break;
        case TOKEN_GREATER_EQUAL:
            EmitTwoBytes(OP_LESS, OP_NOT);
            break;
        case TOKEN_LESS:
            EmitByte(OP_LESS);
            break;
        case TOKEN_LESS_EQUAL:
            EmitTwoBytes(OP_GREATER, OP_NOT);
            break;
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
 * @brief Compiles a function call.
 */
static void CompileCall(bool canAssign)
{
    uint8_t argCount = CompileArgumentList();
    EmitTwoBytes(OP_CALL, argCount);
}

/**
 * @brief Compiles a call's argument list.
 */
static uint8_t CompileArgumentList()
{
    uint8_t argCount = 0;
    if (!CheckToken(TOKEN_RIGHT_PARENTHESES))
    {
        do
        {
            CompileExpression();
            if (argCount == 255)
            {
                Error("Can't have more than 255 arguments.");
            }
            argCount++;
        } while (MatchToken(TOKEN_COMMA));
    }
    ConsumeToken(TOKEN_RIGHT_PARENTHESES, "Expect ')' after arguments.");
    return argCount;
}

/**
 * @brief Compiles an "and" logical operator into the current Chunk.
 */
static void CompileAnd(bool canAssign)
{
    int endJump = EmitJump(OP_JUMP_IF_FALSE);

    EmitByte(OP_POP);
    ParsePrecedence(PRECEDENCE_AND);

    PatchJump(endJump);
}

/**
 * @brief Compiles an "or" logical operator into the current Chunk.
 */
static void CompileOr(bool canAssign)
{
    int elseJump = EmitJump(OP_JUMP_IF_FALSE);
    int endJump = EmitJump(OP_JUMP);

    PatchJump(elseJump);
    EmitByte(OP_POP);

    ParsePrecedence(PRECEDENCE_OR);
    PatchJump(endJump);
}

/**
 * @brief Compiles a literal into the current Chunk.
 */
static void CompileLiteral(bool canAssign)
{
    switch (parser.previous.type)
    {
        case TOKEN_FALSE:
            EmitByte(OP_FALSE);
            break;
        case TOKEN_NIL:
            EmitByte(OP_NIL);
            break;
        case TOKEN_TRUE:
            EmitByte(OP_TRUE);
            break;
        // Unknown literal
        default:
            return;
    }
}

/**
 * @brief Compiles a string into the current Chunk.
 */
static void CompileString(bool canAssign)
{
    EmitConstant(OBJECT_VALUE(CopyString(parser.previous.start + 1, parser.previous.length - 2)));
}

/**
 * @brief Compiles a variable into the current Chunk.
 */
static void CompileVariable(bool canAssign)
{
    CompileNamedVariable(parser.previous, canAssign);
}

/**
 * @brief Compiles a named variable.
 * 
 * @param name The name Token of the variable.
 */
static void CompileNamedVariable(Token name, bool canAssign)
{
    uint8_t getOp, setOp;
    int arg = ResolveLocalVariable(current, &name);

    if (arg != -1)
    {
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    }
    else if ((arg = ResolveUpvalue(current, &name)) != -1)
    {
        getOp = OP_GET_UPVALUE;
        setOp = OP_SET_UPVALUE;
    }
    else
    {
        arg = MakeIdentifierConstant(&name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }

    if (canAssign && MatchToken(TOKEN_EQUAL))
    {
        CompileExpression();
        EmitTwoBytes(setOp, (uint8_t)arg);
    }
    else
    {
        EmitTwoBytes(getOp, (uint8_t)arg);
    }
}

/**
 * @brief Compiles a dot field accessor.
 */
static void CompileDot(bool canAssign)
{
    ConsumeToken(TOKEN_IDENTIFIER, "Expect property name after '.'.");
    uint8_t name = MakeIdentifierConstant(&parser.previous);

    if (canAssign && MatchToken(TOKEN_EQUAL))
    {
        CompileExpression();
        EmitTwoBytes(OP_SET_PROPERTY, name);
    }
    else if (MatchToken(TOKEN_LEFT_PARENTHESES))
    {
        uint8_t argCount = CompileArgumentList();
        EmitTwoBytes(OP_INVOKE, name);
        EmitByte(argCount);
    }
    else
    {
        EmitTwoBytes(OP_GET_PROPERTY, name);
    }
}

/**
 * @brief Compiles a this expression.
 */
static void CompileThis(bool canAssign)
{
    if (currentClass == NULL)
    {
        Error("Can't use 'this' outside of a class.");
        return;
    }

    CompileVariable(false);
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

    bool canAssign = precedence <= PRECEDENCE_ASSIGNMENT;
    prefixRule(canAssign);

    // Attempt to parse out an infix
    while (precedence <= GetParseRule(parser.current.type)->precedence)
    {
        NextToken();
        ParseFn infixRule = GetParseRule(parser.previous.type)->infix;
        infixRule(canAssign);
    }

    if (canAssign && MatchToken(TOKEN_EQUAL))
    {
        Error("Invalid assignment target.");
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
 * @brief Parses out a variable.
 * 
 * @param errorMessage An error message if the parsing fails.
 * @return uint8_t An identifier constant.
 */
static uint8_t ParseVariable(const char* errorMessage)
{
    ConsumeToken(TOKEN_IDENTIFIER, errorMessage);

    DeclareVariable();
    if (current->scopeDepth > 0)
    {
        return 0;
    }

    return MakeIdentifierConstant(&parser.previous);
}

/**
 * @brief Declares a variable within the current scope.
 */
static void DeclareVariable()
{
    if (current->scopeDepth == 0)
    {
        return;
    }

    Token* name = &parser.previous;
    for (int i = current->localCount - 1; i >= 0; i--)
    {
        Local* local = &current->locals[i];
        if (local->depth != -1 && local->depth < current->scopeDepth)
        {
            break;
        }

        if (AreIdentifiersEqual(name, &local->name))
        {
            Error("Already a variable with this name in this scope.");
        }
    }

    AddLocalVariable(*name);
}

/**
 * @brief Defines a global variable.
 * 
 * @param global A variable index.
 */
static void DefineVariable(uint8_t global)
{
    if (current->scopeDepth > 0)
    {
        MarkInitialized();
        return;
    }

    EmitTwoBytes(OP_DEFINE_GLOBAL, global);
}

/**
 * @brief Adds a Local to the current compiler Local list.
 * 
 * @param name The name Token of the Local.
 */
static void AddLocalVariable(Token name)
{
    if (current->localCount == UINT8_COUNT)
    {
        Error("Too many local variables in function.");
        return;
    }

    Local* local = &current->locals[current->localCount++];
    local->name = name;
    local->depth = -1;
    local->isCaptured = false;
}

/**
 * @brief Attempts to resolve a local variable.
 * 
 * @param compiler A Compiler to use to resolve the variable.
 * @param name The name Token of the local.
 * @return int The index of the local, if found.
 */
static int ResolveLocalVariable(Compiler* compiler, Token* name)
{
    for (int i = compiler->localCount - 1; i >= 0; i--)
    {
        Local* local = &compiler->locals[i];
        if (AreIdentifiersEqual(name, &local->name))
        {
            if (local->depth == -1)
            {
                Error("Can't read local variable in its own initializer.");
            }
            return i;
        }
    }

    return -1;
}

/**
 * @brief Attempts to add an upvalue to the current compiler Upvalue list.
 * 
 * @param compiler A Compiler to add an Upvalue to.
 * @param index The index of the upvalue.
 * @param isLocal Whether or not the upvalue is a local.
 * @return int The upvalue's index in the Upvalue list.
 */
static int AddUpvalue(Compiler* compiler, uint8_t index, bool isLocal)
{
    int upvalueCount = compiler->function->upvalueCount;

    for (int i = 0; i < upvalueCount; i++)
    {
        Upvalue* upvalue = &compiler->upvalues[i];
        if (upvalue->index == index && upvalue->isLocal == isLocal)
        {
            return i;
        }
    }

    if (upvalueCount == UINT8_COUNT)
    {
        Error("Too many closure variables in function.");
        return 0;
    }

    compiler->upvalues[upvalueCount].isLocal = isLocal;
    compiler->upvalues[upvalueCount].index = index;
    return compiler->function->upvalueCount++;
}

/**
 * @brief Attempts to resolve an upvalue (closure variable).
 * 
 * @param compiler A Compiler to use to resolve the variable.
 * @param name The name Token of the upvalue.
 * @return int The index of the upvalue, if found.
 */
static int ResolveUpvalue(Compiler* compiler, Token* name)
{
    // No enclosing, reached globals so nothing to do
    if (compiler->enclosing == NULL)
    {
        return -1;
    }

    // First, search in the local scope
    int local = ResolveLocalVariable(compiler->enclosing, name);
    if (local != -1)
    {
        compiler->enclosing->locals[local].isCaptured = true;
        return AddUpvalue(compiler, (uint8_t)local, true);
    }

    // If not local, traverse up to enclosing scope
    int upvalue = ResolveUpvalue(compiler->enclosing, name);
    if (upvalue != -1)
    {
        return AddUpvalue(compiler, (uint8_t)upvalue, false);
    }

    return -1;
}

/**
 * @brief Marks the most recent Local as initialized.
 */
static void MarkInitialized()
{
    if (current->scopeDepth == 0)
    {
        return;
    }
    current->locals[current->localCount - 1].depth = current->scopeDepth;
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
 * @brief Consumes the next Token if it is of a matching TokenType.
 * 
 * @param type A TokenType to match.
 * @return true The Token's type matched the specified type and was consumed.
 * @return false The Token's type did not match.
 */
static bool MatchToken(TokenType type)
{
    if (!CheckToken(type))
    {
        return false;
    }
    else
    {
        NextToken();
        return true;
    }
}

/**
 * @brief Checks if the next Token is of a matching TokenType.
 * 
 * @param type A TokenType to check.
 * @return true The Token's type matched the specified type.
 * @return false The Token's type did not match.
 */
static bool CheckToken(TokenType type)
{
    return parser.current.type == type;
}

/**
 * @brief Attempts to synchronize Parser state after an error.
 */
static void SynchronizeState()
{
    parser.panic = false;

    while (parser.current.type != TOKEN_EOF)
    {
        if (parser.previous.type == TOKEN_SEMICOLON)
        {
            return;
        }

        switch (parser.current.type)
        {
            case TOKEN_CLASS:
            case TOKEN_FUN:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            case TOKEN_RETURN:
                return;

            default:
                ; // Do nothing
        }

        NextToken();
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
 * @brief Appends a jump instruction to the current Chunk.
 * 
 * @param instruction A jump instruction to append.
 * @return int The index of the instruction.
 */
static int EmitJump(uint8_t instruction)
{
    EmitByte(instruction);
    EmitTwoBytes(0xff, 0xff);
    return CurrentChunk()->count - 2;
}

/**
 * @brief Patches a previously written jump instruction with the proper offset to jump to.
 * 
 * @param offset An offset to jump to. 
 */
static void PatchJump(int offset)
{
    // -2 adjusts for IP overshoot of current instruction
    int jump = CurrentChunk()->count - offset - 2;

    if (jump > UINT16_MAX)
    {
        Error("Too much code to jump over.");
    }

    CurrentChunk()->code[offset] = (jump >> 8) & 0xff;
    CurrentChunk()->code[offset + 1] = jump & 0xff;
}

/**
 * @brief Appends a loop to the current Chunk.
 * 
 * @param loopStart The starting offset of the loop.
 */
static void EmitLoop(int loopStart)
{
    EmitByte(OP_LOOP);

    int offset = CurrentChunk()->count - loopStart + 2;
    if (offset > UINT16_MAX)
    {
        Error("Loop body too large.");
    }

    EmitTwoBytes((offset >> 8) & 0xff, offset & 0xff);
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
    if (current->type == TYPE_INITIALIZER)
    {
        EmitTwoBytes(OP_GET_LOCAL, 0);
    }
    else
    {
        EmitByte(OP_NIL);
    }

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
 * @brief Creates an identifier constant.
 * 
 * @param name The name Token of the identifier.
 * @return uint8_t An identifier constant.
 */
static uint8_t MakeIdentifierConstant(Token* name)
{
    return MakeConstant(OBJECT_VALUE(CopyString(name->start, name->length)));
}

/**
 * @brief Compares two identifiers and returns if they are equal.
 * 
 * @param a The name Token of the first identifier.
 * @param b The name Token of the second identifier.
 * @return true The identifiers are equal.
 * @return false The identifiers are not equal.
 */
static bool AreIdentifiersEqual(Token* a, Token* b)
{
    if (a->length != b->length)
    {
        return false;
    }
    else
    {
        return memcmp(a->start, b->start, a->length) == 0;
    }
}

/**
 * @brief Returns the Chunk that is currently being compiled.
 * 
 * @return Chunk* A Chunk that is currently being compiled.
 */
static Chunk* CurrentChunk()
{
    return &current->function->chunk;
}

/**
 * @brief Initializes a Compiler.
 * 
 * @param compiler A Compiler to initialize.
 */
static void InitCompiler(Compiler* compiler, FunctionType type)
{
    compiler->enclosing = current;
    compiler->function = NULL;
    compiler->type = type;
    compiler->localCount = 0;
    compiler->scopeDepth = 0;
    compiler->function = NewFunction();
    current = compiler;
    if (type != TYPE_SCRIPT)
    {
        current->function->name = CopyString(parser.previous.start, parser.previous.length);
    }

    Local* local = &current->locals[current->localCount++];
    local->depth = 0;
    local->isCaptured = false;
    if (type != TYPE_FUNCTION)
    {
        local->name.start = "this";
        local->name.length = 4;
    }
    else
    {
        local->name.start = "";
        local->name.length = 0;
    }
}

/**
 * @brief Ends the current compilation.
 */
static ObjectFunction* EndCompiler()
{
    EmitReturn();
    ObjectFunction* function = current->function;

#ifdef DEBUG_PRINT_CODE
    if (!parser.hadError)
    {
        DisassembleChunk(CurrentChunk(), function->name != NULL ? function->name->chars : "<script>");
    }
#endif

    current = current->enclosing;
    return function;
}

/**
 * @brief Begins a new scope.
 */
static void BeginScope()
{
    current->scopeDepth++;
}

/**
 * @brief Ends the last scope.
 */
static void EndScope()
{
    current->scopeDepth--;

    // Pop all locals off stack
    while (current->localCount > 0 && current->locals[current->localCount - 1].depth > current->scopeDepth)
    {
        if (current->locals[current->localCount - 1].isCaptured)
        {
            EmitByte(OP_CLOSE_UPVALUE);
        }
        else
        {
            EmitByte(OP_POP);
        }
        current->localCount--;
    }
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
