//edit chunk.h
typedef struct {
    int line;
    int count;
} LineRun;

typedef struct {
    int count;
    int capacity;
    uint8_t* code;
    ValueArray constants;

    // RLE line info
    int lineCount;
    int lineCapacity;
    LineRun* lines;
} Chunk;

//add getLine declaration to chunk.h as well
int getLine(Chunk* chunk, int instructionIndex);

//edit chunk.c freeChunk and writeChunk methods, and add getLine method
void freeChunk(Chunk* chunk) {
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    FREE_ARRAY(LineRun, chunk->lines, chunk->lineCapacity); //change to LineRun
    freeValueArray(&chunk->constants);
    initChunk(chunk);
}

void writeChunk(Chunk* chunk, uint8_t byte, int line) {
    // Grow code array as before
    if (chunk->capacity < chunk->count + 1) {
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
    }
    chunk->code[chunk->count] = byte;
    chunk->count++;

    // RLE: if same line as last run, just increment the count
    if (chunk->lineCount > 0 &&
        chunk->lines[chunk->lineCount - 1].line == line) {
        chunk->lines[chunk->lineCount - 1].count++;
        return;
    }

    // Otherwise start a new run
    if (chunk->lineCapacity < chunk->lineCount + 1) {
        int oldCapacity = chunk->lineCapacity;
        chunk->lineCapacity = GROW_CAPACITY(oldCapacity);
        chunk->lines = GROW_ARRAY(LineRun, chunk->lines, oldCapacity, chunk->lineCapacity);
    }
    chunk->lines[chunk->lineCount].line = line;
    chunk->lines[chunk->lineCount].count = 1;
    chunk->lineCount++;
}

int getLine(Chunk* chunk, int instructionIndex) {
    int offset = 0;
    for (int i = 0; i < chunk->lineCount; i++) {
        offset += chunk->lines[i].count;
        if (offset > instructionIndex) {
            return chunk->lines[i].line;
        }
    }
    // Should never reach here for a valid instruction index
    return -1;
}

//edit disassembleInstruction method in debug.c
int disassembleInstruction(Chunk* chunk, int offset) {
  printf("%04d ", offset);

  if (offset > 0 &&
      getLine(chunk, offset) == getLine(chunk, offset - 1)) {
    printf("   | ");
  } else {
    printf("%4d ", getLine(chunk, offset));
  }
}

//edit vm.c runtimeError method around line 76
// Change:
function->chunk.lines[instruction]
// To:
getLine(&function->chunk, instruction)
