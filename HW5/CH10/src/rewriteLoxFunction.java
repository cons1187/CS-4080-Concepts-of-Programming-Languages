package com.craftinginterpreters.lox;

import java.util.List;

class LoxFunction implements LoxCallable {
    private final Token name;  // Can be null for anonymous functions
    private final List<Token> params;
    private final List<Stmt> body;
    private final Environment closure;
    private final boolean isInitializer;

    // Constructor for named functions (from Stmt.Function)
    LoxFunction(Stmt.Function declaration, Environment closure,
                boolean isInitializer) {
        this.name = declaration.name;
        this.params = declaration.params;
        this.body = declaration.body;
        this.closure = closure;
        this.isInitializer = isInitializer;
    }

    // Constructor for anonymous functions (from Expr.Function)
    LoxFunction(Expr.Function expression, Environment closure) {
        this.name = null;
        this.params = expression.params;
        this.body = expression.body;
        this.closure = closure;
        this.isInitializer = false;
    }

    LoxFunction bind(LoxInstance instance) {
        Environment environment = new Environment(closure);
        environment.define("this", instance);

        // Need to create a temporary Stmt.Function for bind
        // or modify bind to work differently
        Stmt.Function temp = new Stmt.Function(name, params, body);
        return new LoxFunction(temp, environment, isInitializer);
    }

    @Override
    public String toString() {
        if (name == null) return "<fn>";
        return "<fn " + name.lexeme + ">";
    }

    @Override
    public int arity() {
        return params.size();
    }

    @Override
    public Object call(Interpreter interpreter,
                       List<Object> arguments) {
        Environment environment = new Environment(closure);
        for (int i = 0; i < params.size(); i++) {
            environment.define(params.get(i).lexeme,
                    arguments.get(i));
        }

        try {
            interpreter.executeBlock(body, environment);
        } catch (Return returnValue) {
            if (isInitializer) return closure.getAt(0, "this");
            return returnValue.value;
        }

        if (isInitializer) return closure.getAt(0, "this");
        return null;
    }
}