//add new opcode to chunk.h
typedef enum {
  OP_CONSTANT,
  OP_CONSTANT_LONG,   // <-- add this
  // ... rest unchanged
} OpCode;

//declare writeConstant as a method in chunk.h
void writeConstant(Chunk* chunk, Value value, int line);

//implement writeConstant in chunk.c
void writeConstant(Chunk* chunk, Value value, int line) {
  int index = addConstant(chunk, value);

  if (index < 256) {
    // Fits in one byte, use the short form
    writeChunk(chunk, OP_CONSTANT, line);
    writeChunk(chunk, (uint8_t)index, line);
  } else {
    // Needs 24 bits, use the long form
    writeChunk(chunk, OP_CONSTANT_LONG, line);
    // Write index as three bytes, little-endian
    writeChunk(chunk, (uint8_t)(index & 0xff), line);
    writeChunk(chunk, (uint8_t)((index >> 8) & 0xff), line);
    writeChunk(chunk, (uint8_t)((index >> 16) & 0xff), line);
  }
}

//add disassembly support in debug.c
// Add this helper function:
static int longConstantInstruction(const char* name, Chunk* chunk, int offset) {
  // Reconstruct the 24-bit index from 3 bytes, little-endian
  uint32_t index = chunk->code[offset + 1]
                 | (chunk->code[offset + 2] << 8)
                 | (chunk->code[offset + 3] << 16);
  printf("%-16s %4d '", name, index);
  printValue(chunk->constants.values[index]);
  printf("'\n");
  return offset + 4;  // 1 byte opcode + 3 bytes operand
}

// Then in the switch inside disassembleInstruction(), add:
case OP_CONSTANT_LONG:
  return longConstantInstruction("OP_CONSTANT_LONG", chunk, offset);

//handle case in vm.c
case OP_CONSTANT_LONG: {
  uint32_t index = READ_BYTE()
                 | (READ_BYTE() << 8)
                 | (READ_BYTE() << 16);
  Value constant = frame->closure->function->chunk.constants.values[index];
  push(constant);
  break;
}

//edit makeConstant and update emitConstant in compiler.c
static uint32_t makeConstant(Value value) {
  int constant = addConstant(currentChunk(), value);
  return constant;
}

static void emitConstant(Value value) {
  writeConstant(currentChunk(), value, previousToken().line);
}