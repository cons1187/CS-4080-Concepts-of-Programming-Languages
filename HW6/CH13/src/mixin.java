/* mixin syntax design
Use with keyword to show which methods to mix with the class
Mixin method declarations use the mixin keyword
Mixins take priority over superclass methods but have less priority than own-class methods
Mixins are not instantiable
 */

//Add to TokenType.java
public enum TokenType {
    // ... existing tokens ...
    AND, CLASS, ELSE, FALSE, FUN, FOR, IF, NIL, OR,
    PRINT, RETURN, SUPER, THIS, TRUE, VAR, WHILE,
    BREAK, MIXIN, WITH,  // NEW

    EOF
}

//Add to Scanner.java
static {
    keywords = new HashMap<>();
    // ... existing keywords ...
    keywords.put("break",  BREAK);
    keywords.put("mixin",  MIXIN);  // NEW
    keywords.put("with",   WITH);   // NEW
}

//Add to Stmt.java and visitor interface
static class Mixin extends Stmt {
    Mixin(Token name, List<Stmt.Function> methods) {
        this.name = name;
        this.methods = methods;
    }

    @Override
    <R> R accept(Visitor<R> visitor) {
        return visitor.visitMixinStmt(this);
    }

    final Token name;
    final List<Stmt.Function> methods;
}

interface Visitor<R> {
    // ... existing methods ...
    R visitMixinStmt(Mixin stmt);  // NEW
}

//also update the class in Stmt.java
static class Class extends Stmt {
    Class(Token name, Expr.Variable superclass,
          List<Expr.Variable> mixins,  // NEW
          List<Stmt.Function> methods,
          List<Stmt.Function> classMethods) {
        this.name = name;
        this.superclass = superclass;
        this.mixins = mixins;  // NEW
        this.methods = methods;
        this.classMethods = classMethods;
    }

    final Token name;
    final Expr.Variable superclass;
    final List<Expr.Variable> mixins;  // NEW
    final List<Stmt.Function> methods;
    final List<Stmt.Function> classMethods;
}

//add to Parser.java
private Stmt declaration() {
    try {
        if (match(CLASS)) return classDeclaration();
        if (match(FUN)) return funDeclaration();
        if (match(MIXIN)) return mixinDeclaration();  // NEW
        if (match(VAR)) return varDeclaration();
        return statement();
    } catch (ParseError error) {
        synchronize();
        return null;
    }
}

private Stmt mixinDeclaration() {
    Token name = consume(IDENTIFIER, "Expect mixin name.");

    consume(LEFT_BRACE, "Expect '{' before mixin body.");

    List<Stmt.Function> methods = new ArrayList<>();

    while (!check(RIGHT_BRACE) && !isAtEnd()) {
        // Parse methods (no static methods in mixins)
        Token methodName = consume(IDENTIFIER, "Expect method name.");

        boolean isGetter = check(LEFT_BRACE);
        List<Token> parameters = new ArrayList<>();

        if (!isGetter) {
            consume(LEFT_PAREN, "Expect '(' after method name.");
            if (!check(RIGHT_PAREN)) {
                do {
                    if (parameters.size() >= 255) {
                        error(peek(), "Can't have more than 255 parameters.");
                    }
                    parameters.add(consume(IDENTIFIER, "Expect parameter name."));
                } while (match(COMMA));
            }
            consume(RIGHT_PAREN, "Expect ')' after parameters.");
        }

        consume(LEFT_BRACE, "Expect '{' before method body.");
        List<Stmt> body = block();

        methods.add(new Stmt.Function(methodName, parameters, body, false, isGetter));
    }

    consume(RIGHT_BRACE, "Expect '}' after mixin body.");

    return new Stmt.Mixin(name, methods);
}

//also update class declaration in parser
private Stmt classDeclaration() {
    Token name = consume(IDENTIFIER, "Expect class name.");

    Expr.Variable superclass = null;
    if (match(LESS)) {
        consume(IDENTIFIER, "Expect superclass name.");
        superclass = new Expr.Variable(previous());
    }

    // NEW: Parse mixins
    List<Expr.Variable> mixins = new ArrayList<>();
    if (match(WITH)) {
        do {
            consume(IDENTIFIER, "Expect mixin name.");
            mixins.add(new Expr.Variable(previous()));
        } while (match(COMMA));
    }

    consume(LEFT_BRACE, "Expect '{' before class body.");

    // ... parse methods ...

    consume(RIGHT_BRACE, "Expect '}' after class body.");

    return new Stmt.Class(name, superclass, mixins, methods, classMethods);
}

//create new file of LoxMixin class
package com.craftinginterpreters.lox;

import java.util.Map;

class LoxMixin {
    final String name;
    private final Map<String, LoxFunction> methods;

    LoxMixin(String name, Map<String, LoxFunction> methods) {
        this.name = name;
        this.methods = methods;
    }

    LoxFunction findMethod(String name) {
        if (methods.containsKey(name)) {
            return methods.get(name);
        }
        return null;
    }

    Map<String, LoxFunction> getMethods() {
        return methods;
    }

    @Override
    public String toString() {
        return "<mixin " + name + ">";
    }
}

//store mixins in Interpreter.java
@Override
public Void visitMixinStmt(Stmt.Mixin stmt) {
    // Create methods map
    Map<String, LoxFunction> methods = new HashMap<>();
    for (Stmt.Function method : stmt.methods) {
        LoxFunction function = new LoxFunction(method, environment, false);
        methods.put(method.name.lexeme, function);
    }

    // Create and define mixin
    LoxMixin mixin = new LoxMixin(stmt.name.lexeme, methods);
    environment.define(stmt.name.lexeme, mixin);

    return null;
}

//apply when creating class
@Override
public Void visitClassStmt(Stmt.Class stmt) {
    Object superclass = null;
    if (stmt.superclass != null) {
        superclass = evaluate(stmt.superclass);
        if (!(superclass instanceof LoxClass)) {
            throw new RuntimeError(stmt.superclass.name,
                    "Superclass must be a class.");
        }
    }

    environment.define(stmt.name.lexeme, null);

    // NEW: Resolve mixins
    List<LoxMixin> mixins = new ArrayList<>();
    for (Expr.Variable mixinExpr : stmt.mixins) {
        Object mixinObj = evaluate(mixinExpr);
        if (!(mixinObj instanceof LoxMixin)) {
            throw new RuntimeError(mixinExpr.name,
                    "Can only mix in mixins.");
        }
        mixins.add((LoxMixin)mixinObj);
    }

    // Create metaclass with static methods
    Map<String, LoxFunction> classMethods = new HashMap<>();
    for (Stmt.Function method : stmt.classMethods) {
        LoxFunction function = new LoxFunction(method, environment, false);
        classMethods.put(method.name.lexeme, function);
    }

    LoxClass metaclass = new LoxClass(null, stmt.name.lexeme + " metaclass",
            null, classMethods);

    if (stmt.superclass != null) {
        environment = new Environment(environment);
        environment.define("super", superclass);
    }

    // Create instance methods
    Map<String, LoxFunction> methods = new HashMap<>();

    // First, add mixin methods (lower priority)
    for (LoxMixin mixin : mixins) {
        for (Map.Entry<String, LoxFunction> entry : mixin.getMethods().entrySet()) {
            methods.put(entry.getKey(), entry.getValue());
        }
    }

    // Then, add class methods (override mixin methods)
    for (Stmt.Function method : stmt.methods) {
        LoxFunction function = new LoxFunction(method, environment,
                method.name.lexeme.equals("init"));
        methods.put(method.name.lexeme, function);
    }

    LoxClass klass = new LoxClass(metaclass, stmt.name.lexeme,
            (LoxClass)superclass, methods);

    if (superclass != null) {
        environment = environment.enclosing;
    }

    environment.assign(stmt.name, klass);
    return null;
}

//update Resolver.java
@Override
public Void visitMixinStmt(Stmt.Mixin stmt) {
    declare(stmt.name);
    define(stmt.name);

    beginScope();
    Token thisToken = new Token(TokenType.THIS, "this", null, 0);
    scopes.peek().put("this", new Variable(thisToken, true, true, 0));
    scopeSizes.push(scopeSizes.pop() + 1);

    for (Stmt.Function method : stmt.methods) {
        FunctionType declaration = FunctionType.METHOD;
        resolveFunction(method.params, method.body, declaration);
    }

    endScope();

    return null;
}

@Override
public Void visitClassStmt(Stmt.Class stmt) {
    ClassType enclosingClass = currentClass;
    currentClass = ClassType.CLASS;

    declare(stmt.name);
    define(stmt.name);

    if (stmt.superclass != null &&
            stmt.name.lexeme.equals(stmt.superclass.name.lexeme)) {
        Lox.error(stmt.superclass.name,
                "A class can't inherit from itself.");
    }

    if (stmt.superclass != null) {
        currentClass = ClassType.SUBCLASS;
        resolve(stmt.superclass);
    }

    // NEW: Resolve mixins
    for (Expr.Variable mixin : stmt.mixins) {
        resolve(mixin);
    }

    // ... rest of class resolution ...

    return null;
}

//update AstPrinter.java
@Override
public String visitMixinStmt(Stmt.Mixin stmt) {
    StringBuilder builder = new StringBuilder();
    builder.append("(mixin ").append(stmt.name.lexeme);
    for (Stmt.Function method : stmt.methods) {
        builder.append(" ");
        builder.append(method.name.lexeme);
    }
    builder.append(")");
    return builder.toString();
}
