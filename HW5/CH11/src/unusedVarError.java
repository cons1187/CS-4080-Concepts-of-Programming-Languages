//Add helper class to track variable states
private static class Variable {
    final Token name;       // Store the token for error reporting
    boolean isDefined;      // Has the variable been initialized?
    boolean isUsed;         // Has the variable been referenced?

    Variable(Token name, boolean isDefined, boolean isUsed) {
        this.name = name;
        this.isDefined = isDefined;
        this.isUsed = isUsed;
    }
}

//Update scope
private final Stack<Map<String, Variable>> scopes = new Stack<>(); //changed to variable instead of boolean

//Update declare() method
private void declare(Token name) {
    if (scopes.isEmpty()) return;

    Map<String, Variable> scope = scopes.peek();

    if (scope.containsKey(name.lexeme)) {
        Lox.error(name,
                "Already a variable with this name in this scope.");
    }

    // Create new variable, marked as declared but not defined
    scope.put(name.lexeme, new Variable(name, false, false));
}

//Update define() method
private void define(Token name) {
    if (scopes.isEmpty()) return;

    Map<String, Variable> scope = scopes.peek();
    Variable variable = scope.get(name.lexeme);

    if (variable != null) {
        variable.isDefined = true;
    }
}

//Update resolveLocal() method
private void resolveLocal(Expr expr, Token name) {
    for (int i = scopes.size() - 1; i >= 0; i--) {
        if (scopes.get(i).containsKey(name.lexeme)) {
            // Mark variable as used
            scopes.get(i).get(name.lexeme).isUsed = true;

            interpreter.resolve(expr, scopes.size() - 1 - i);
            return;
        }
    }
}

//Update endScope() method
private void endScope() {
    Map<String, Variable> scope = scopes.pop();

    // Check for unused variables
    for (Variable variable : scope.values()) {
        if (!variable.isUsed) {
            Lox.error(variable.name,
                    "Local variable is never used.");
        }
    }
}

//edit visitClassStmt() method
@Override
public Void visitClassStmt(Stmt.Class stmt) {

    //code comes before

    if (stmt.superclass != null) {
        beginScope();
        scopes.peek().put("super", new Variable(
                new Token(TokenType.SUPER, "super", null, 0), true, true));
    }

    beginScope();
    scopes.peek().put("this", new Variable(
            new Token(TokenType.THIS, "this", null, 0), true, true));

    //code comes after
}

//edit visitVariableExpr() method
@Override
public Void visitVariableExpr(Expr.Variable expr) {
    if (!scopes.isEmpty()) {
        Variable variable = scopes.peek().get(expr.name.lexeme);
        if (variable != null && !variable.isDefined) {
            Lox.error(expr.name,
                    "Can't read local variable in its own initializer.");
        }
    }

    resolveLocal(expr, expr.name);
    return null;
}

//edit beginScope() method
private void beginScope() {
    scopes.push(new HashMap<String, Variable>());
}