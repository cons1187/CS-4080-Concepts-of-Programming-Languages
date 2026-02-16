//Edit expression in Parser.java
private Expr expression() {
    return comma();  // Was: return assignment();
}

//Add comma() method
private Expr comma() {
    Expr expr = assignment();
    while (match(COMMA)) {
        Token operator = previous();
        Expr right = assignment();
        expr = new Expr.Binary(expr, operator, right);
    }
    return expr;
}

//Edit finishCall()
    arguments.add(assignment());  // Was: expression()

//Add comma case in interpreter.java
    case COMMA:
    return right;
