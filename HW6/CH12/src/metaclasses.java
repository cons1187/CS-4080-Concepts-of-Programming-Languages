//Add static flag to Stmt.java
static class Function extends Stmt {
    Function(Token name, List<Token> params, List<Stmt> body, boolean isStatic) {
        this.name = name;
        this.params = params;
        this.body = body;
        this.isStatic = isStatic;
    }

    final Token name;
    final List<Token> params;
    final List<Stmt> body;
    final boolean isStatic;  // NEW
}

//update class to hold static methods
static class Class extends Stmt {
    Class(Token name, Expr.Variable superclass,
          List<Stmt.Function> methods,
          List<Stmt.Function> classMethods) {  // NEW parameter
        this.name = name;
        this.superclass = superclass;
        this.methods = methods;
        this.classMethods = classMethods;  // NEW
    }

    final Token name;
    final Expr.Variable superclass;
    final List<Stmt.Function> methods;
    final List<Stmt.Function> classMethods;  // NEW
}

//in Parser.java edit classDeclaration() method and function() method
private Stmt.Function function(String kind) {
    //start of method...
    return new Stmt.Function(name, parameters, body, false);  // NEW: Pass false
}

private Stmt classDeclaration() {
    //start of method...
    while (!check(RIGHT_BRACE) && !isAtEnd()) {
        boolean isStatic = match(CLASS);  // NEW: Check for 'class' keyword

        Token methodName = consume(IDENTIFIER, "Expect method name.");

        consume(LEFT_PAREN, "Expect '(' after method name.");
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

        consume(LEFT_BRACE, "Expect '{' before method body.");
        List<Stmt> body = block();

        Stmt.Function method = new Stmt.Function(methodName, parameters, body, isStatic);

        if (isStatic) {
            classMethods.add(method);  // NEW: Add to static methods
        } else {
            methods.add(method);
        }
    }

    consume(RIGHT_BRACE, "Expect '}' after class body.");

    return new Stmt.Class(name, superclass, methods, classMethods);  // NEW: Pass static methods
}

//make LoxClass extend LoxInstance in LoxClass.java
class LoxClass extends LoxInstance implements LoxCallable {
    final String name;
    final LoxClass superclass;
    private final Map<String, LoxFunction> methods;

    LoxClass(LoxClass metaclass, String name, LoxClass superclass,
             Map<String, LoxFunction> methods) {
        super(metaclass);  // NEW: Class is an instance of its metaclass
        this.name = name;
        this.superclass = superclass;
        this.methods = methods;
    }

    //rest of code
}

//edit visitClassStmt() method in Interpreter.java
@Override
public Void visitClassStmt(Stmt.Class stmt) {
    //start of code

    // NEW: Create metaclass for static methods
    Map<String, LoxFunction> classMethods = new HashMap<>();
    for (Stmt.Function method : stmt.classMethods) {
        LoxFunction function = new LoxFunction(method, environment, false);
        classMethods.put(method.name.lexeme, function);
    }

    // Create the metaclass (a class for the class)
    LoxClass metaclass = new LoxClass(null, stmt.name.lexeme + " metaclass",
            null, classMethods);

    //...

    // Create the class (as an instance of the metaclass)
    LoxClass klass = new LoxClass(metaclass, stmt.name.lexeme,
            (LoxClass)superclass, methods);

    //end of code
}

//update visitClassStmt() in Resolver.java
@Override
public Void visitClassStmt(Stmt.Class stmt) {
    //body of method

    // NEW: Resolve static methods (no 'this' in scope)
    for (Stmt.Function method : stmt.classMethods) {
        resolveFunction(method.params, method.body, FunctionType.FUNCTION);
    }

    if (stmt.superclass != null) endScope();

    currentClass = enclosingClass;
    return null;
}

//if any errors are encountered, look for lines that create new Stmt.Function objects and pass false as a fourth parameter