//in scanner.h add tokens
typedef enum {
  // ...existing tokens
  TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE,

  // add continue keyword
  TOKEN_CONTINUE,

  TOKEN_ERROR, TOKEN_EOF
} TokenType;

//in scanner.c edit identifierType method
static TokenType identifierType() {
  switch (scanner.start[0]) {
    case 'a': return checkKeyword(1, 2, "nd", TOKEN_AND);

    case 'c':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'l': return checkKeyword(2, 3, "ass", TOKEN_CLASS);
          case 'o': return checkKeyword(2, 6, "ntinue",
              TOKEN_CONTINUE);
        }
      }
      break;

    case 'e': return checkKeyword(1, 3, "lse", TOKEN_ELSE);

    // ...rest the same

    return TOKEN_IDENTIFIER;
}

//in compiler.c add continue statement support
// track current loop for continue
int innermostLoopStart = -1;
int innermostLoopScopeDepth = 0;

// forward declaration
static void continueStatement();

// add to parse rules
[TOKEN_CONTINUE] = {NULL, NULL, PREC_NONE};

// continue implementation
static void continueStatement() {
  // error if not in loop
  if (innermostLoopStart == -1) {
    error("Can't use 'continue' outside of a loop.");
  }

  consume(TOKEN_SEMICOLON, "Expect ';' after 'continue'.");

  // pop locals inside loop body
  for (int i = current->localCount - 1;
       i >= 0 && current->locals[i].depth > innermostLoopScopeDepth;
       i--) {
    emitByte(OP_POP);
  }

  // jump to loop start
  emitLoop(innermostLoopStart);
}

//in statement method
} else if (match(TOKEN_RETURN)) {
    returnStatement();

  } else if (match(TOKEN_CONTINUE)) {
    continueStatement();

  } else if (match(TOKEN_WHILE)) {
    whileStatement();

//in forStatement
static void forStatement() {
  beginScope();

  consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");
  if (match(TOKEN_VAR)) {
    varDeclaration();
  } else if (!match(TOKEN_SEMICOLON)) {
    // no initializer
  } else {
    expressionStatement();
  }

  // save outer loop
  int surroundingLoopStart = innermostLoopStart;
  int surroundingLoopScopeDepth = innermostLoopScopeDepth;

  // set current loop
  innermostLoopStart = currentChunk()->count;
  innermostLoopScopeDepth = current->scopeDepth;

  int exitJump = -1;
  if (!match(TOKEN_SEMICOLON)) {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");

    exitJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
  }

  if (!match(TOKEN_RIGHT_PAREN)) {
    int bodyJump = emitJump(OP_JUMP);

    int incrementStart = currentChunk()->count;
    expression();
    emitByte(OP_POP);
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

    // jump before increment
    emitLoop(innermostLoopStart);

    // continue goes to increment
    innermostLoopStart = incrementStart;

    patchJump(bodyJump);
  }

  statement();

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

//in whileStatement
static void whileStatement() {
  int loopStart = currentChunk()->count;

  // save outer loop
  int surroundingLoopStart = innermostLoopStart;
  int surroundingLoopScopeDepth = innermostLoopScopeDepth;

  // set current loop
  innermostLoopStart = loopStart;
  innermostLoopScopeDepth = current->scopeDepth;

  consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

  int exitJump = emitJump(OP_JUMP_IF_FALSE);
  emitByte(OP_POP);
  statement();
  emitLoop(loopStart);

  patchJump(exitJump);
  emitByte(OP_POP);

  // restore outer loop
  innermostLoopStart = surroundingLoopStart;
  innermostLoopScopeDepth = surroundingLoopScopeDepth;
}

//in synchronize
// ...everything same, add:
case TOKEN_RETURN:
case TOKEN_CONTINUE:
  return;