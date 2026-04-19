//only edit is to forStatement in compiler.c
static void forStatement() {
  beginScope();

  // initializer
  int loopVarSlot = -1;
  Token loopVarName;

  consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");
  if (match(TOKEN_VAR)) {
    loopVarName = parser.current;
    varDeclaration();
    loopVarSlot = current->localCount - 1;
  } else if (match(TOKEN_SEMICOLON)) {
    // no initializer
  } else {
    expressionStatement();
  }

  // save outer loop
  int surroundingLoopStart = innermostLoopStart;
  int surroundingLoopScopeDepth = innermostLoopScopeDepth;

  // condition
  int loopStart = currentChunk()->count;
  innermostLoopStart = loopStart;
  innermostLoopScopeDepth = current->scopeDepth;

  int exitJump = -1;
  if (!match(TOKEN_SEMICOLON)) {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");
    exitJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
  }

  // increment
  if (!match(TOKEN_RIGHT_PAREN)) {
    int bodyJump = emitJump(OP_JUMP);
    int incrementStart = currentChunk()->count;

    expression();
    emitByte(OP_POP);
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

    emitLoop(innermostLoopStart);
    innermostLoopStart = incrementStart;
    patchJump(bodyJump);
  }

  // per-iteration scope for loop variable copy
  if (loopVarSlot != -1) {
    beginScope();
    emitBytes(OP_GET_LOCAL, (uint8_t)loopVarSlot);
    addLocal(loopVarName);
    markInitialized();
  }

  // body
  statement();

  // close per-iteration scope
  if (loopVarSlot != -1) {
    endScope();
  }

  // loop back
  emitLoop(innermostLoopStart);

  if (exitJump != -1) {
    patchJump(exitJump);
    emitByte(OP_POP);
  }

  // restore outer loop
  innermostLoopStart = surroundingLoopStart;
  innermostLoopScopeDepth = surroundingLoopScopeDepth;

  endScope();
}