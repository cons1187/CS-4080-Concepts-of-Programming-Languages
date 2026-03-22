//in compiler.c add PREC_TERNARY to enum
typedef enum {
  PREC_NONE,
  PREC_ASSIGNMENT,  // =
  PREC_TERNARY,     // ?:   <-- add this
  PREC_OR,          // or
  PREC_AND,         // and
  PREC_EQUALITY,    // == !=
  PREC_COMPARISON,  // < > <= >=
  PREC_TERM,        // + -
  PREC_FACTOR,      // * /
  PREC_UNARY,       // ! -
  PREC_CALL,        // . ()
  PREC_PRIMARY
} Precedence;

//implement ternary() method skeleton
static void ternary(bool canAssign) {
  parsePrecedence(PREC_TERNARY);
  consume(TOKEN_COLON, "Expected ':' after '?' expression.");
  parsePrecedence(PREC_TERNARY);
}

//add question mark token to ParseRule table
ParseRule rules[] = {
    //other tokens
    [TOKEN_QUESTION] = { NULL, ternary, PREC_TERNARY },
    //other tokens
};

//in scanner.h add the question and colon tokens to enum
typedef enum {
    //other tokens
    TOKEN_QUESTION,
    TOKEN_COLON,
    //other tokens
} TokenType;

//in scanner.c add the cases of colon and question to the scanToken switch
Token scanToken() {
    skipWhitespace();
    scanner.start = scanner.current;

    if(isAtEnd()) return makeToken(TOKEN_EOF);
    char c = advance();
    if (isAlpha(c)) return identifier();
    if (isDigit(c)) return number();
    switch (c) {
        //other cases
        case '?': return makeToken(TOKEN_QUESTION);
        case ':': return makeToken(TOKEN_COLON);
        //other cases
    }
    //rest of code
}