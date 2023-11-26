#include "common.h"
#include "chunk.h"
#include "debug.h"

/**
 * @brief Main entry point.
 */
int main(int argc, const char* argv[])
{
    Chunk chunk;

    InitChunk(&chunk);
    WriteChunk(&chunk, OP_RETURN);

    DisassembleChunk(&chunk, "test chunk");
    FreeChunk(&chunk);

    return 0;
}