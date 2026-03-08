//first steps are to add inner as a token in TokenType.java and Scanner.java
public enum TokenType {
    // ... existing tokens ...
    AND, CLASS, ELSE, FALSE, FUN, FOR, IF, NIL, OR,
    PRINT, RETURN, SUPER, THIS, TRUE, VAR, WHILE,
    BREAK, MIXIN, WITH, INNER,  // NEW

    EOF
}

static {
    keywords = new HashMap<>();
    // ... existing keywords ...
    keywords.put("break",  BREAK);
    keywords.put("INNER", INNER);
}

//add to Expr.java
static class Inner extends Expr {
    Inner(Token keyword) {
        this.keyword = keyword;
    }

    @Override
    <R> R accept(Visitor<R> visitor) {
        return visitor.visitInnerExpr(this);
    }

    final Token keyword;
}

//parse inner
if (match(INNER)) {
Token keyword = previous();
consume(LEFT_PAREN, "Expect '(' after 'inner'.");
consume(RIGHT_PAREN, "Expect ')' after arguments.");
  return new Expr.Inner(keyword);
}

//edit findmethod in LoxClass.java
LoxFunction findMethod(String name) {
    // NEW: Top-down - superclass first
    if (superclass != null) {
        LoxFunction inherited = superclass.findMethod(name);
        if (inherited != null) {
            return inherited;
        }
    }

    // Then our own method
    if (methods.containsKey(name)) {
        return methods.get(name);
    }

    return null;
}

//add definingClass field to LoxFunction.java
class LoxFunction implements LoxCallable {
    //other fields
    private final LoxClass definingClass;  // NEW

    LoxFunction(Stmt.Function declaration, Environment closure,
                boolean isInitializer, LoxClass definingClass) {
        //other fields
        this.definingClass = definingClass;  // NEW
    }

    // Update anonymous function constructor
    LoxFunction(Expr.Function expression, Environment closure) {
        //other fields
        this.definingClass = null;  // NEW
    }

    LoxClass getDefiningClass() {  // NEW
        return definingClass;
    }
}

//edit call method
@Override
public Object call(Interpreter interpreter, List<Object> arguments) {
    Environment environment = new Environment(closure);

    // NEW: Store execution context
    if (definingClass != null) {
        environment.define("__definingClass__", definingClass);
    }
    environment.define("__currentMethod__", name != null ? name.lexeme : "");

    for (int i = 0; i < params.size(); i++) {
        environment.define(params.get(i).lexeme, arguments.get(i));
    }

    try {
        interpreter.executeBlock(body, environment);
    } catch (Return returnValue) {
        if (isInitializer) return closure.getByName(0, "this");
        return returnValue.value;
    }

    if (isInitializer) return closure.getByName(0, "this");
    return null;
}

//implement inner into interpreter.java
@Override
public Object visitInnerExpr(Expr.Inner expr) {
    // Get execution context from environment
    Object definingClassObj = null;
    Object methodNameObj = null;
    Object thisObj = null;

    try {
        definingClassObj = environment.get(new Token(TokenType.IDENTIFIER, "__definingClass__", null, 0));
        methodNameObj = environment.get(new Token(TokenType.IDENTIFIER, "__currentMethod__", null, 0));
        thisObj = environment.get(new Token(TokenType.THIS, "this", null, 0));
    } catch (RuntimeError e) {
        throw new RuntimeError(expr.keyword,
                "Can only use 'inner' inside a method.");
    }

    if (!(thisObj instanceof LoxInstance)) {
        throw new RuntimeError(expr.keyword,
                "Can only use 'inner' on an instance.");
    }

    LoxInstance instance = (LoxInstance)thisObj;
    LoxClass definingClass = (LoxClass)definingClassObj;
    String methodName = (String)methodNameObj;

    // Find the inner method (in subclass direction)
    LoxFunction innerMethod = findInnerMethod(instance.getClass_(),
            definingClass,
            methodName);

    if (innerMethod != null) {
        return innerMethod.bind(instance).call(this, new ArrayList<>());
    }

    // No inner method - return nil
    return null;
}

private LoxFunction findInnerMethod(LoxClass instanceClass,
                                    LoxClass definingClass,
                                    String methodName) {
    // Build hierarchy from instance class up to root
    List<LoxClass> hierarchy = new ArrayList<>();
    LoxClass c = instanceClass;
    while (c != null) {
        hierarchy.add(c);
        c = c.superclass;
    }

    // Find defining class in hierarchy
    int definingIndex = -1;
    for (int i = 0; i < hierarchy.size(); i++) {
        if (hierarchy.get(i) == definingClass) {
            definingIndex = i;
            break;
        }
    }

    if (definingIndex == -1 || definingIndex == 0) {
        return null;  // No subclass to call
    }

    // Look for method in classes below defining class
    for (int i = definingIndex - 1; i >= 0; i--) {
        if (hierarchy.get(i).methods.containsKey(methodName)) {
            return hierarchy.get(i).methods.get(methodName);
        }
    }

    return null;
}

//edit visitclassStmt
// When creating instance methods:
for (Stmt.Function method : stmt.methods) {
LoxFunction function = new LoxFunction(method, environment,
        method.name.lexeme.equals("init"),
        klass);  // NEW: Pass the class being defined
  methods.put(method.name.lexeme, function);
}

//add to LoxInstance.java
LoxClass getClass_() {
    return klass;
}

//add setMethods method to LoxClass.java
void setMethods(Map<String, LoxFunction> newMethods) {
    methods.putAll(newMethods);
}

//update bind method in LoxFunction.java
LoxFunction bind(LoxInstance instance) {
    Environment environment = new Environment(closure);
    environment.define("this", instance);
    Stmt.Function temp = new Stmt.Function(name, params, body, false, isGetter);
    return new LoxFunction(temp, environment, isInitializer, definingClass);  // NEW: pass definingClass
}

//if any errors add null as a fourth parameter to any new LoxFunction constructor calls