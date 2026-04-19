//in compiler.c edit to make closure wrapping conditional
if (function->upvalueCount == 0) {
  emitBytes(OP_CONSTANT, makeConstant(OBJ_VAL(function)));
} else {
  emitBytes(OP_CLOSURE, makeConstant(OBJ_VAL(function)));
  for (int i = 0; i < function->upvalueCount; i++) {
    emitByte(compiler.upvalues[i].isLocal ? 1 : 0);
    emitByte(compiler.upvalues[i].index);
  }
}

//in vm.h edit CallFrame struct
typedef struct {
  ObjFunction* function;   // always valid
  ObjClosure* closure;     // NULL if not a closure
  uint8_t* ip;
  Value* slots;
} CallFrame;

//in vm.c add new methods
static bool callFunction(ObjFunction* function, int argCount) {
  if (argCount != function->arity) {
    runtimeError("Expected %d arguments but got %d.",
                 function->arity, argCount);
    return false;
  }
  if (vm.frameCount == FRAMES_MAX) {
    runtimeError("Stack overflow.");
    return false;
  }
  CallFrame* frame = &vm.frames[vm.frameCount++];
  frame->closure = NULL;   // no closure
  frame->function = function;
  frame->ip = function->chunk.code;
  frame->slots = vm.stackTop - argCount - 1;
  return true;
}

// helper to get the function from a frame regardless of type
static inline ObjFunction* frameFunction(CallFrame* frame) {
  return frame->closure != NULL ? frame->closure->function : frame->function;
}

//edit callValue method
static bool callValue(Value callee, int argCount) {
  if (IS_OBJ(callee)) {
    switch (OBJ_TYPE(callee)) {
      case OBJ_FUNCTION:
        return callFunction(AS_FUNCTION(callee), argCount);
      case OBJ_CLOSURE:
        return call(AS_CLOSURE(callee), argCount);
      case OBJ_NATIVE: {
        // ...
      }
      // ...
    }
  }
}

//edit this part of runtimeError
for (int i = vm.frameCount - 1; i >= 0; i--) {
  CallFrame* frame = &vm.frames[i];
  ObjFunction* function = frameFunction(frame); //change to frameFunction
  size_t instruction = frame->ip - function->chunk.code - 1;
  fprintf(stderr, "[line %d] in ",
          function->chunk.lines[instruction]);
  if (function->name == NULL) {
    fprintf(stderr, "script\n");
  } else {
    fprintf(stderr, "%s()\n", function->name->chars);
  }
}

//update macro
#define READ_CONSTANT() (frameFunction(frame)->chunk.constants.values[READ_BYTE()])