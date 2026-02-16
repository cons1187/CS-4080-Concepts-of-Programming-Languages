//Editing all methods that handle parsing to add error handling

//Equality
private Expr equality() {
    // ERROR PRODUCTION: Binary operator without left operand
    if (match(BANG_EQUAL, EQUAL_EQUAL)) {
        Token operator = previous();
        error(operator, "Missing left-hand operand for '" + operator.lexeme + "'.");
        equality();  // Parse and discard right operand (recursive!)
        return new Expr.Literal(null);  // Return dummy value to continue
    }

    // NORMAL PRODUCTION
    Expr expr = comparison();

    while (match(BANG_EQUAL, EQUAL_EQUAL)) {
        Token operator = previous();
        Expr right = comparison();
        expr = new Expr.Binary(expr, operator, right);
    }

    return expr;
}

//Comparison
private Expr comparison() {
    // ERROR PRODUCTION
    if (match(GREATER, GREATER_EQUAL, LESS, LESS_EQUAL)) {
        Token operator = previous();
        error(operator, "Missing left-hand operand for '" + operator.lexeme + "'.");
        comparison();  // Parse and discard right operand
        return new Expr.Literal(null);
    }

    // NORMAL PRODUCTION
    Expr expr = term();

    while (match(GREATER, GREATER_EQUAL, LESS, LESS_EQUAL)) {
        Token operator = previous();
        Expr right = term();
        expr = new Expr.Binary(expr, operator, right);
    }

    return expr;
}

//Term
private Expr term() {
    // ERROR PRODUCTION
    // Note: We DON'T include MINUS because "-5" is valid (unary minus)
    if (match(PLUS)) {
        Token operator = previous();
        error(operator, "Missing left-hand operand for '+'.");
        term();  // Parse and discard right operand
        return new Expr.Literal(null);
    }

    // NORMAL PRODUCTION
    Expr expr = factor();

    while (match(MINUS, PLUS)) {
        Token operator = previous();
        Expr right = factor();
        expr = new Expr.Binary(expr, operator, right);
    }

    return expr;
}

//Factor
private Expr factor() {
    // ERROR PRODUCTION
    if (match(SLASH, STAR)) {
        Token operator = previous();
        error(operator, "Missing left-hand operand for '" + operator.lexeme + "'.");
        factor();  // Parse and discard right operand
        return new Expr.Literal(null);
    }

    // NORMAL PRODUCTION
    Expr expr = unary();

    while (match(SLASH, STAR)) {
        Token operator = previous();
        Expr right = unary();
        expr = new Expr.Binary(expr, operator, right);
    }

    return expr;
}

//Or
private Expr or() {
    // ERROR PRODUCTION (less common, but possible)
    if (match(OR)) {
        Token operator = previous();
        error(operator, "Missing left-hand operand for 'or'.");
        or();
        return new Expr.Literal(null);
    }

    // NORMAL PRODUCTION
    Expr expr = and();

    while (match(OR)) {
        Token operator = previous();
        Expr right = and();
        expr = new Expr.Logical(expr, operator, right);
    }

    return expr;
}

//And
private Expr and() {
    // ERROR PRODUCTION
    if (match(AND)) {
        Token operator = previous();
        error(operator, "Missing left-hand operand for 'and'.");
        and();
        return new Expr.Literal(null);
    }

    // NORMAL PRODUCTION
    Expr expr = equality();

    while (match(AND)) {
        Token operator = previous();
        Expr right = equality();
        expr = new Expr.Logical(expr, operator, right);
    }

    return expr;
}

//Unary
private Expr unary() {
    if (match(BANG, MINUS)) {
        Token operator = previous();
        Expr right = unary();
        return new Expr.Unary(operator, right);
    }

    if (match(PLUS, STAR, SLASH,
            BANG_EQUAL, EQUAL_EQUAL,
            GREATER, GREATER_EQUAL, LESS, LESS_EQUAL)) {
        Token operator = previous();
        error(operator, "Missing left-hand operand for '" + operator.lexeme + "'.");
        unary();  // Parse and discard the right side
        return new Expr.Literal(null);
    }

    return call();
}