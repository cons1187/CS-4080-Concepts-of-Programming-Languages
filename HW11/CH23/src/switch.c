//add OP_DUP to enum in chunk.h
typedef enum {
  OP_CONSTANT,
  OP_NIL,
  OP_TRUE,
  OP_FALSE,
  OP_POP,
  OP_DUP,          // <-- ADD THIS
  OP_GET_LOCAL,
  // ... rest unchanged
}

//add OP_DUP case in vm.c
case OP_DUP:
        push(peek(0));
        break;

//add OP_DUP case in disassembly.c
case OP_DUP:
        return simpleInstruction("OP_DUP", offset);

//add new token types to scanner.h
TOKEN_COLON, TOKEN_CASE, TOKEN_DEFAULT, TOKEN_SWITCH

//in scanner.c add new cases to identifierType method
case 'c': return checkKeyword(1, 3, "ase", TOKEN_CASE);

case 'd': return checkKeyword(1, 6, "efault", TOKEN_DEFAULT);

case 's':
    if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'u': return checkKeyword(2, 3, "per", TOKEN_SUPER);
          case 'w': return checkKeyword(2, 4, "itch", TOKEN_SWITCH);
        }
    }
    return TOKEN_IDENTIFIER;

//add new case under switch (c)
case ':': return makeToken(TOKEN_COLON);

//add switchStatement method in compiler.c
static void switchStatement() {
  consume(TOKEN_LEFT_PAREN, "Expect '(' after 'switch'.");
  expression();  // Switch value stays on stack.
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after value.");
  consume(TOKEN_LEFT_BRACE, "Expect '{' before switch cases.");

#define MAX_CASES 256
  int exitJumps[MAX_CASES];
  int exitJumpCount = 0;

  int nextCaseJump = -1;  // Jump to patch when a case doesn't match.

  while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
    if (match(TOKEN_CASE)) {
      // If there was a previous false-jump, patch it to here.
      if (nextCaseJump != -1) {
        patchJump(nextCaseJump);
        emitByte(OP_POP);  // Pop the false comparison result.
      }

      // Duplicate switch value so OP_EQUAL doesn't consume it.
      emitByte(OP_DUP);

      // Evaluate the case expression and compare.
      expression();
      consume(TOKEN_COLON, "Expect ':' after case value.");
      emitByte(OP_EQUAL);

      // If not equal, jump past body to the next case.
      nextCaseJump = emitJump(OP_JUMP_IF_FALSE);

      // Pop the true comparison result, leaving switch value.
      emitByte(OP_POP);

      // Pop the switch value itself — we're about to run the body.
      emitByte(OP_POP);

      // Compile the case body statements.
      while (!check(TOKEN_CASE) && !check(TOKEN_DEFAULT) &&
             !check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
        statement();
      }

      // Jump to end of switch after body executes.
      if (exitJumpCount == MAX_CASES) {
        error("Too many cases in switch statement.");
        return;
      }
      exitJumps[exitJumpCount++] = emitJump(OP_JUMP);

    } else if (match(TOKEN_DEFAULT)) {
      consume(TOKEN_COLON, "Expect ':' after 'default'.");

      if (nextCaseJump != -1) {
        patchJump(nextCaseJump);
        emitByte(OP_POP);  // Pop false comparison result.
        nextCaseJump = -1;
      }

      // Pop the switch value — we're running the default body.
      emitByte(OP_POP);

      while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
        statement();
      }
    } else {
      error("Expect 'case' or 'default' in switch body.");
      return;
    }
  }

  // If no case matched and there's no default,
  // patch the last false-jump and pop the switch value.
  if (nextCaseJump != -1) {
    patchJump(nextCaseJump);
    emitByte(OP_POP);  // Pop false comparison.
    emitByte(OP_POP);  // Pop switch value.
  }

  // Patch all the exit jumps to here (end of switch).
  for (int i = 0; i < exitJumpCount; i++) {
    patchJump(exitJumps[i]);
  }

  consume(TOKEN_RIGHT_BRACE, "Expect '}' after switch cases.");
#undef MAX_CASES
}

//in statement method add this
} else if (match(TOKEN_SWITCH)) {
    switchStatement();
  } else {