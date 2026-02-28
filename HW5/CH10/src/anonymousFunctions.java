//Add the function class to Expr.java
static class Function extends Expr {
    Function(List<Token> params, List<Stmt> body) {
        this.params = params;
        this.body = body;
    }

    @Override
    <R> R accept(Visitor<R> visitor) {
        return visitor.visitFunctionExpr(this);
    }

    final List<Token> params;
    final List<Stmt> body;
}

//make sure to add to the visitor interface
interface Visitor<R> {
    R visitFunctionExpr(Function expr);
}

//change function declaration in Parser.java
private Stmt declaration() {
    try {
        if (match(CLASS)) return classDeclaration();
        if (match(FUN)) return funDeclaration();  // NEW
        if (match(VAR)) return varDeclaration();
        return statement();
    } catch (ParseError error) {
        synchronize();
        return null;
    }
}

//add funDeclaration() method
private Stmt funDeclaration() {
    //If next token is identifier, it's a declaration
    if (check(IDENTIFIER)) {
        Token name = consume(IDENTIFIER, "Expect function name.");
        return functionDeclaration("function", name);
    }

    // Otherwise, it's an expression statement with anonymous function
    return expressionStatement();
}

//renamed and modified
private Stmt.Function functionDeclaration(String kind, Token name) {
    consume(LEFT_PAREN, "Expect '(' after " + kind + " name.");

    List<Token> parameters = new ArrayList<>();
    if (!check(RIGHT_PAREN)) {
        do {
            if (parameters.size() >= 255) {
                error(peek(), "Can't have more than 255 parameters.");
            }
            parameters.add(consume(IDENTIFIER, "Expect parameter name."));
        } while (match(COMMA));
    }
    consume(RIGHT_PAREN, "Expect ')' after parameters.");

    consume(LEFT_BRACE, "Expect '{' before " + kind + " body.");
    List<Stmt> body = block();

    return new Stmt.Function(name, parameters, body);
}

//edit primary() method
private Expr primary() {
    //other cases
    if (match(FUN)) {
        return functionExpression();
    }
    throw error(peek(), "Expect expression.");
}

//add method to parse anonymous function
private Expr functionExpression() {
    consume(LEFT_PAREN, "Expect '(' after 'fun'.");

    List<Token> parameters = new ArrayList<>();
    if (!check(RIGHT_PAREN)) {
        do {
            if (parameters.size() >= 255) {
                error(peek(), "Can't have more than 255 parameters.");
            }
            parameters.add(consume(IDENTIFIER, "Expect parameter name."));
        } while (match(COMMA));
    }
    consume(RIGHT_PAREN, "Expect ')' after parameters.");

    consume(LEFT_BRACE, "Expect '{' before function body.");
    List<Stmt> body = block();

    return new Expr.Function(parameters, body);
}

//add new method to Interpreter.java, AstPrinter.java, and RpnPrinter.java
@Override //Interpreter method
public Object visitFunctionExpr(Expr.Function expr) {
    return new LoxFunction(expr, environment);
}

@Override //AstPrinter method
public String visitFunctionExpr(Expr.Function expr) {
    StringBuilder builder = new StringBuilder();
    builder.append("(fun (");
    for (int i = 0; i < expr.params.size(); i++) {
        if (i > 0) builder.append(" ");
        builder.append(expr.params.get(i).lexeme);
    }
    builder.append(") ...)");
    return builder.toString();
}

@Override //RpnPrinter method
public String visitFunctionExpr(Expr.Function expr) {
    return "<function>";
}

//edit Resolver.java to add methods and update based on parsing
@Override
public Void visitFunctionExpr(Expr.Function expr) {
    resolveFunction(expr.params, expr.body, FunctionType.FUNCTION);
    return null;
}

// Modify resolveFunction to work with either type
private void resolveFunction(List<Token> params, List<Stmt> body,
                             FunctionType type) {
    FunctionType enclosingFunction = currentFunction;
    currentFunction = type;

    beginScope();
    for (Token param : params) {
        declare(param);
        define(param);
    }
    resolve(body);
    endScope();

    currentFunction = enclosingFunction;
}

//Update visitFunctionStmt to use it
@Override
public Void visitFunctionStmt(Stmt.Function stmt) {
    declare(stmt.name);
    define(stmt.name);

    resolveFunction(stmt.params, stmt.body, FunctionType.FUNCTION);
    return null;
}

//also see updated LoxFunction.java file since it was reformatted