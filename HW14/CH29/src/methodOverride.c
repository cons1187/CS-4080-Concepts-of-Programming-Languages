//add TOKEN_INNER to enum in scanner.h

//add identifier to scanner.c
case 'i':
  if (scanner.current - scanner.start > 1) {
    switch (scanner.start[1]) {
      case 'f': return TOKEN_IF;
      case 'n': return checkKeyword(2, 3, "ner", TOKEN_INNER);
    }
  }
  break;

//add new OP_INNER opcode in chunk.h

//add new disassembly case in debug.c
case OP_INNER:
  return simpleInstruction("OP_INNER", offset);

//edit ObjClosure struct in object.h
typedef struct ObjClosure {
  Obj obj;
  ObjFunction* function;
  ObjUpvalue** upvalues;
  int upvalueCount;
  struct ObjClosure* inner;  // next method down chain, NULL if none
} ObjClosure;

//in object.c add to newClosure method
closure->inner = NULL;

//in memory.c add to OBJ_Closure case
markObject((Obj*)closure->inner);

//add inner to parse table in compiler.c
[TOKEN_INNER] = {inner, NULL, PREC_NONE},

//also add new parse function\
static void inner(bool canAssign) {
  if (currentClass == NULL) {
    error("Can't use 'inner' outside of a class.");
    return;
  }
  uint8_t argCount = argumentList();
  emitBytes(OP_INNER, argCount);
}

//since super is not usable replace body of method
static void super_(bool canAssign) {
  error("'super' is not supported. Use 'inner' instead.");
}

//in vm.c edit defineMethod
static void defineMethod(ObjString* name) {
  ObjClosure* newMethod = AS_CLOSURE(peek(0));
  ObjClass* klass = AS_CLASS(peek(1));

  Value existing;
  if (tableGet(&klass->methods, name, &existing)) {
    ObjClosure* cursor = AS_CLOSURE(existing);
    while (cursor->inner != NULL) {
      cursor = cursor->inner;
    }
    cursor->inner = newMethod;
  } else {
    tableSet(&klass->methods, name, OBJ_VAL(newMethod));
  }
  pop();
}

//add OP_INNER handling in dispatch
case OP_INNER: {
  int argCount = READ_BYTE();
  ObjClosure* current = frame->closure;
  if (current->inner == NULL) {
    vm.stackTop -= argCount;
    push(NIL_VAL);
    break;
  }
  if (!call(current->inner, argCount)) {
    return INTERPRET_RUNTIME_ERROR;
  }
  frame = &vm.frames[vm.frameCount - 1];
  break;
}