//Add index to the variable class in Resolver.java
private static class Variable {
    final Token name;
    boolean isDefined;
    boolean isUsed;
    final int index;  // NEW: Position in scope (0, 1, 2, ...)

    Variable(Token name, boolean isDefined, boolean isUsed, int index) {
        this.name = name;
        this.isDefined = isDefined;
        this.isUsed = isUsed;
        this.index = index;
    }
}

//Add global field to Resolver.java that keeps track of number of variables
private final Stack<Integer> scopeSizes = new Stack<>();

//update beginScope() method
private void beginScope() {
    scopes.push(new HashMap<>());
    scopeSizes.push(0);  // Start counting from 0
}

//update endScope() method
private void endScope() {
    Map<String, Variable> scope = scopes.pop();
    scopeSizes.pop();  // Remove the counter

    for (Variable variable : scope.values()) {
        if (!variable.isUsed && !variable.name.lexeme.startsWith("_")) {
            Lox.error(variable.name, "Local variable is never used.");
        }
    }
}

//update declare() method
private void declare(Token name) {
    if (scopes.isEmpty()) return;

    Map<String, Variable> scope = scopes.peek();
    if (scope.containsKey(name.lexeme)) {
        Lox.error(name, "Already a variable with this name in this scope.");
    }

    // Get next available index and assign it
    int index = scopeSizes.peek();

    // Create variable with index
    scope.put(name.lexeme, new Variable(name, false, false, index));

    // Increment counter for next variable
    int newSize = scopeSizes.pop() + 1;
    scopeSizes.push(newSize);
}

//update resolveLocal() method
private void resolveLocal(Expr expr, Token name) {
    for (int i = scopes.size() - 1; i >= 0; i--) {
        if (scopes.get(i).containsKey(name.lexeme)) {
            Variable variable = scopes.get(i).get(name.lexeme);
            variable.isUsed = true;

            int depth = scopes.size() - 1 - i;

            // NEW: Pass both depth AND index
            interpreter.resolve(expr, depth, variable.index);
            return;
        }
    }
}

//edit super and this cases in visitClassStmt() method
// In visitClassStmt, when adding "this":
beginScope();
Token thisToken = new Token(TokenType.THIS, "this", null, 0);
scopes.peek().put("this", new Variable(thisToken, true, true, 0));
// Increment counter since we used index 0
scopeSizes.push(scopeSizes.pop() + 1);

// Similarly for "super":
Token superToken = new Token(TokenType.SUPER, "super", null, 0);
scopes.peek().put("super", new Variable(superToken, true, true, 0));
scopeSizes.push(scopeSizes.pop() + 1);

//create location helper class in Interpreter.java
// In Interpreter.java
private static class Location {
    final int depth;
    final int index;

    Location(int depth, int index) {
        this.depth = depth;
        this.index = index;
    }
}

//update the locals map
// OLD:
private final Map<Expr, Integer> locals = new HashMap<>();

// NEW:
private final Map<Expr, Location> locals = new HashMap<>();

//update resolve() method
// OLD:
void resolve(Expr expr, int depth) {
    locals.put(expr, depth);
}

// NEW:
void resolve(Expr expr, int depth, int index) {
    locals.put(expr, new Location(depth, index));
}

//create new method for variable lookup
private Object lookupVariable(Token name, Expr expr) {
    Location location = locals.get(expr);

    if (location != null) {
        // Local variable - use indexed lookup
        return environment.getAt(location.depth, location.index);
    } else {
        // Global variable - use name lookup
        return globals.get(name);
    }
}

//update visitVariableExpr() method
@Override
public Object visitVariableExpr(Expr.Variable expr) {
    return lookupVariable(expr.name, expr);
}

//update visitAssignExpr() method
@Override
public Object visitAssignExpr(Expr.Assign expr) {
    Object value = evaluate(expr.value);

    Location location = locals.get(expr);

    if (location != null) {
        // Local variable - use indexed assignment
        environment.assignAt(location.depth, location.index, value);
    } else {
        // Global variable - use name assignment
        globals.assign(expr.name, value);
    }

    return value;
}

//update visitSuperExpr method
@Override
public Object visitSuperExpr(Expr.Super expr) {
    Location location = locals.get(expr);

    // 'super' and 'this' use name-based lookup (special case)
    LoxClass superclass = (LoxClass)environment.ancestor(location.depth).get(
            new Token(TokenType.SUPER, "super", null, 0));

    LoxInstance object = (LoxInstance)environment.ancestor(location.depth - 1).get(
            new Token(TokenType.THIS, "this", null, 0));

    LoxFunction method = superclass.findMethod(expr.method.lexeme);

    if (method == null) {
        throw new RuntimeError(expr.method,
                "Undefined property '" + expr.method.lexeme + "'.");
    }

    return method.bind(object);
}

//in Environment.java add indexed storage list and update imports
package com.craftinginterpreters.lox;

import java.util.HashMap;
import java.util.Map;
import java.util.List;      // ADD THIS
import java.util.ArrayList;  // ADD THIS
class Environment {
    final Environment enclosing;
    private final Map<String, Object> values = new HashMap<>();
    private final List<Object> slots = new ArrayList<>();  // NEW: Indexed storage

    private static final Object UNINITIALIZED = new Object();

    Environment() {
        enclosing = null;
    }

    Environment(Environment enclosing) {
        this.enclosing = enclosing;
    }

    // ... existing methods ...
}

//update definition methods
void define(String name, Object value) {
    // Add to indexed array
    slots.add(value);

    // Also keep in map (for name-based access like globals)
    values.put(name, value);
}

void defineUninitialized(String name) {
    slots.add(UNINITIALIZED);
    values.put(name, UNINITIALIZED);
}

//add indexed access methods
Object getAt(int depth, int index) {
    Environment target = ancestor(depth);

    if (index >= target.slots.size()) {
        throw new RuntimeError(null, "Variable index out of bounds.");
    }

    Object value = target.slots.get(index);

    if (value == UNINITIALIZED) {
        throw new RuntimeError(null, "Variable is not initialized.");
    }

    return value;
}

void assignAt(int depth, int index, Object value) {
    Environment target = ancestor(depth);

    if (index >= target.slots.size()) {
        throw new RuntimeError(null, "Variable index out of bounds.");
    }

    target.slots.set(index, value);
}

Environment ancestor(int distance) {
    Environment environment = this;
    for (int i = 0; i < distance; i++) {
        environment = environment.enclosing;
    }
    return environment;
}

//edit function calls in LoxFunction.java
//replace old:
if (isInitializer) return closure.getAt(0, "this");
//with new:
if (isInitializer) return closure.get(new Token(TokenType.THIS, "this", null, 0));

