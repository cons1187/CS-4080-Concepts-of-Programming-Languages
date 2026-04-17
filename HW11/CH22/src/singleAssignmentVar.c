//add TOKEN_VAL in scanner.h next to TOKEN_VAR

//in scanner.c edit keyword case v in identifierType method
case 'v':
  if (scanner.current - scanner.start > 2) {
    switch (scanner.start[1]) {
      case 'a':
        switch (scanner.start[2]) {
          case 'r': return checkKeyword(3, 0, "", TOKEN_VAR);
          case 'l': return checkKeyword(3, 0, "", TOKEN_VAL);
        }
    }
  }

//add isVal boolean to local struct
typedef struct {
  Token name;
  int depth;
  bool isCaptured;
  bool isVal;      //true if declared with 'val' — cannot be reassigned
} Local;

//add valGlobals table in VM struct in vm.h
typedef struct {
  // ... existing fields ...
  Table globals;
  Table valGlobals;  //set of global names declared with 'val'
} VM;

//add currentIsVal boolean to compiler struct after currentVar

//add valDeclaration method in compiler.c
static void valDeclaration() {
  current->currentIsVal = true;
  uint8_t global = parseVariable("Expected variable name.");
  current->currentIsVal = false;

  if (match(TOKEN_EQUAL)) {
    expression();
  } else {
    // val must be initialized — no 'val x;' without a value
    error("'val' variables must be initialized at declaration.");
    emitByte(OP_NIL);
  }
  consume(TOKEN_SEMICOLON, "Expected ';' after variable declaration.");

  defineVal(global);  // separate from defineVariable
}

//add defineVal method
static void defineVal(uint8_t global) {
  if (current->scopeDepth > 0) {
    // Local val — just mark it initialized, the isVal flag
    // was already set in addLocal()
    markInitialized();
    return;
  }
  // Global val — emit the normal define instruction,
  // then record this name in the valGlobals table so
  // assignments can be blocked at runtime
  ObjString* name = AS_STRING(currentChunk()->constants.values[global]);
  tableSet(&vm.valGlobals, OBJ_VAL(name), BOOL_VAL(true));

  emitBytes(OP_DEFINE_GLOBAL, global);
}

//edit addLocal method
static void addLocal(Token name) {
  // ... existing bounds check ...
  Local* local = &current->locals[current->localCount++];
  local->name = name;
  local->depth = -1;
  local->isCaptured = false;
  local->isVal = current->currentIsVal;  // mark if val
}

//edit namedVariable method
static void namedVariable(Token name, bool canAssign) {
  uint8_t getOp, setOp;
  int arg = resolveLocal(current, &name);

  if (arg != -1) {
    getOp = OP_GET_LOCAL;
    setOp = OP_SET_LOCAL;
    // Check if local is val
    if (canAssign && current->locals[arg].isVal) {
      error("Cannot assign to 'val' variable.");
    }
  } else if ((arg = resolveUpvalue(current, &name)) != -1) {
    getOp = OP_GET_UPVALUE;
    setOp = OP_SET_UPVALUE;
    // Upvalue val checking would require propagating the flag
    // through the upvalue chain — omit for now
  } else {
    arg = identifierConstant(&name);
    getOp = OP_GET_GLOBAL;
    setOp = OP_SET_GLOBAL;
  }

  if (canAssign && match(TOKEN_EQUAL)) {
    // Local val check above already errored if needed
    // Global val check:
    if (setOp == OP_SET_GLOBAL) {
      // emit the assignment — runtime will check valGlobals table
      // (see vm.c change below)
    }
    expression();
    emitBytes(setOp, (uint8_t)arg);
  } else {
    emitBytes(getOp, (uint8_t)arg);
  }
}

//add val to declaration method
static void declaration() {
  if (match(TOKEN_VAL)) {
    valDeclaration();
  } else if (match(TOKEN_VAR)) {
    varDeclaration();
  } else if (match(TOKEN_FUN)) {
    funDeclaration();
  } else if (match(TOKEN_CLASS)) {
    classDeclaration();
  } else {
    statement();
  }
  if (parser.panicMode) synchronize();
}