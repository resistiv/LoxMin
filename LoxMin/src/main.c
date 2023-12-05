#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

static void Repl();
static void RunFile(const char* path);
static char* ReadFile(const char* path);

/**
 * @brief Main entry point.
 */
int main(int argc, const char* argv[])
{
    printf("LoxMin v1.0.0 - Kai NeSmith 2023\n");
    InitVM();

    // No path given
    if (argc == 1)
    {
        Repl();
    }
    // Path provided
    else if (argc == 2)
    {
        RunFile(argv[1]);
    }
    // Does our user know where they are?
    else
    {
        fprintf(stderr, "Usage: LoxMin [path]\n");
        exit(64);
    }

    FreeVM();
    return 0;
}

/**
 * @brief Runs the interpreter as a REPL.
 */
static void Repl()
{
    char line[1024];

    while (1)
    {
        // Prompt and read input
        printf("> ");
        if (!fgets(line, sizeof(line), stdin))
        {
            printf("\n");
            break;
        }

        // Interpret the input
        Interpret(line);
    }
}

/**
 * @brief Runs the interpreter from a file.
 * 
 * @param path A path to the source file.
 */
static void RunFile(const char* path)
{
    char* source = ReadFile(path);
    InterpretResult result = Interpret(source);
    free(source);

    if (result == INTERPRET_COMPILE_ERROR)
    {
        exit(65);
    }
    else if (result == INTERPRET_RUNTIME_ERROR)
    {
        exit(70);
    }
}

/**
 * @brief Reads a file into memory.
 * 
 * @param path A path to the file.
 * @return char* A pointer to the read bytes.
 */
static char* ReadFile(const char* path)
{
    FILE* file = fopen(path, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    // Get size of file
    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    // Allocate memory
    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL)
    {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }

    // Read in file
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize)
    {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }
    buffer[bytesRead] = '\0';

    // Release file handle
    fclose(file);

    return buffer;
}
