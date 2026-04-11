//edit identifierConstant in compiler.c
static uint8_t identifierConstant(Token* name) {
  ObjString* string = copyString(name->start, name->length);

  Chunk* chunk = currentChunk();
  for (int i = 0; i < chunk->constants.count; i++) {
    Value val = chunk->constants.values[i];
    if (IS_OBJ(val) && IS_STRING(val)) {
      ObjString* existing = AS_STRING(val);
      if (existing == string) return i;
    }
  }
  return makeConstant(OBJ_VAL(string));
}