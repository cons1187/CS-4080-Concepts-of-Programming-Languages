//update Stmt.java
static class Function extends Stmt {
    Function(Token name, List<Token> params, List<Stmt> body,
             boolean isStatic, boolean isGetter) {
        this.name = name;
        this.params = params;
        this.body = body;
        this.isStatic = isStatic;
        this.isGetter = isGetter;  // NEW
    }

    final Token name;
    final List<Token> params;
    final List<Stmt> body;
    final boolean isStatic;
    final boolean isGetter;  // NEW
}

//update classDeclaration() method in Parser.java
private Stmt classDeclaration() {
    //start of method...

    while (!check(RIGHT_BRACE) && !isAtEnd()) {
        boolean isStatic = match(CLASS);

        Token methodName = consume(IDENTIFIER, "Expect method name.");

        // NEW: Check if it's a getter (no parentheses)
        boolean isGetter = check(LEFT_BRACE);

        List<Token> parameters = new ArrayList<>();

        if (!isGetter) {
            // Normal method - parse parameters
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
        // Getters have no parameters

        consume(LEFT_BRACE, "Expect '{' before method body.");
        List<Stmt> body = block();

        Stmt.Function method = new Stmt.Function(methodName, parameters, body,
                isStatic, isGetter);

        if (isStatic) {
            classMethods.add(method);
        } else {
            methods.add(method);
        }
    }

    //rest of method
}

//edit function() method
return new Stmt.Function(name, parameters, body, false, false); //add 5th parameter

//update bind() function in LoxFunction.java
LoxFunction bind(LoxInstance instance) {
    Environment environment = new Environment(closure);
    environment.define("this", instance);

    Stmt.Function temp = new Stmt.Function(name, params, body, false, false);
    return new LoxFunction(temp, environment, isInitializer);
}

//update constructor in LoxFunction.java and add getter check
class LoxFunction implements LoxCallable {
    private final Token name;
    private final List<Token> params;
    private final List<Stmt> body;
    private final Environment closure;
    private final boolean isInitializer;
    private final boolean isGetter;  // NEW

    LoxFunction(Stmt.Function declaration, Environment closure,
                boolean isInitializer) {
        this.name = declaration.name;
        this.params = declaration.params;
        this.body = declaration.body;
        this.closure = closure;
        this.isInitializer = isInitializer;
        this.isGetter = declaration.isGetter;  // NEW
    }

    // Constructor for anonymous functions
    LoxFunction(Expr.Function expression, Environment closure) {
        this.name = null;
        this.params = expression.params;
        this.body = expression.body;
        this.closure = closure;
        this.isInitializer = false;
        this.isGetter = false;  // NEW
    }

    // ... rest of class ...
}

boolean isGetter() {
    return isGetter;
}

//update get() method in LoxInstance.java
Object get(Token name, Interpreter interpreter) {
    if (fields.containsKey(name.lexeme)) {
        return fields.get(name.lexeme);
    }

    LoxFunction method = klass.findMethod(name.lexeme);
    if (method != null) {
        if (method.isGetter()) {
            // Call the getter immediately
            return method.bind(this).call(interpreter, new ArrayList<>());
        }
        return method.bind(this);
    }

    throw new RuntimeError(name,
            "Undefined property '" + name.lexeme + "'.");
}

//update visitGetExpr() in Interpreter.java
@Override
public Object visitGetExpr(Expr.Get expr) {
    Object object = evaluate(expr.object);
    if (object instanceof LoxInstance) {
        return ((LoxInstance) object).get(expr.name, this);  // Pass 'this' (interpreter)
    }

    throw new RuntimeError(expr.name,
            "Only instances have properties.");
}

//make sure to also import ArrayList package in LoxInstance.java