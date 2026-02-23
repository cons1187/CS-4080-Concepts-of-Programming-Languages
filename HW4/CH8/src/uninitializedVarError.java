//Add new class variable to Environment.java
private static final Object UNINITIALIZED = new Object();

//Edit get() method in Environment.java
Object get(Token name) {
    if (values.containsKey(name.lexeme)) {
        Object value = values.get(name.lexeme);

        //check if the variable is uninitialized
        if (value == UNINITIALIZED) {
            throw new RuntimeError(name,
                    "Variable '" + name.lexeme + "' is not initialized.");
        }

        return value;
    }

    if (enclosing != null) return enclosing.get(name);

    throw new RuntimeError(name,
            "Undefined variable '" + name.lexeme + "'.");
}

//Add new method in Environment.java
void defineUninitialized(String name) {
    values.put(name, UNINITIALIZED);
}

//Edit visitVarStmt() method in Interpreter.java
@Override
public Void visitVarStmt(Stmt.Var stmt) {
    Object value = null;
    if (stmt.initializer != null) {
        value = evaluate(stmt.initializer);
        environment.define(stmt.name.lexeme, value);
    } else {
        environment.defineUninitialized(stmt.name.lexeme);
    }

    return null;
}