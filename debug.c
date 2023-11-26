#include <stdio.h>

#include "debug.h"

void DisassembleChunk(Chunk* chunk, const char* name)
{
    printf("== %s ==\n", name);

    // Iterate through instructions
    for (int offset = 0; offset < chunk->count;)
    {
        offset = DisassembleInstruction(chunk, offset);
    }
}

static int SimpleInstruction(const char* name, int offset)
{
    printf("%s\n", name);
    return offset + 1;
}

int DisassembleInstruction(Chunk* chunk, int offset)
{
    printf("%04d ", offset);

    uint8_t instruction = chunk->code[offset];
    switch (instruction)
    {
        case OP_RETURN:
            return SimpleInstruction("OP_RETURN", offset);
        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}