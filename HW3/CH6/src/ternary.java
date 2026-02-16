//Add conditional class to Expr.java, and add visitConditionalExpr to interface
static class Conditional extends Expr {
    Conditional(Expr condition, Expr thenBranch, Expr elseBranch) {
        this.condition = condition;
        this.thenBranch = thenBranch;
        this.elseBranch = elseBranch;
    }

    @Override
    <R> R accept(Visitor<R> visitor) {
        return visitor.visitConditionalExpr(this);
    }

    final Expr condition;
    final Expr thenBranch;
    final Expr elseBranch;
}

//Update Assignment method and add ternary method in Parser.java
private Expr assignment() {
    Expr expr = ternary();  // CHANGED: was or()

    if (match(EQUAL)) {
        Token equals = previous();
        Expr value = assignment();

        if (expr instanceof Expr.Variable) {
            Token name = ((Expr.Variable)expr).name;
            return new Expr.Assign(name, value);
        }
        // ... error handling
    }

    return expr;
}

private Expr ternary() {
    Expr expr = logic_or();

    if (match(QUESTION)) {
        Expr thenBranch = expression();  // Allow ANY expression between ? and :
        consume(COLON, "Expect ':' after then branch of conditional expression.");
        Expr elseBranch = ternary();  // Right-associative: recurse on right side
        expr = new Expr.Conditional(expr, thenBranch, elseBranch);
    }

    return expr;
}

//Add Question and Colon tokens to TokenType.java, and add ? and : cases to Scanner.java

//Add visitor methods to Interpreter.java, AstPrinter.java, RpnPrinter.java, and Resolver.java

//Interpreter
@Override
public Object visitConditionalExpr(Expr.Conditional expr) {
    Object condition = evaluate(expr.condition);

    if (isTruthy(condition)) {
        return evaluate(expr.thenBranch);
    } else {
        return evaluate(expr.elseBranch);
    }
}

//AstPrinter
@Override
public String visitConditionalExpr(Expr.Conditional expr) {
    return parenthesize("?:",
            expr.condition, expr.thenBranch, expr.elseBranch);
}

//RpnPrinter/rpnconverter
@Override
public String visitConditionalExpr(Expr.Conditional expr) {
    return expr.condition.accept(this)
            + " " + expr.thenBranch.accept(this)
            + " " + expr.elseBranch.accept(this)
            + " ?:";
}

//Resolver
@Override
public Void visitConditionalExpr(Expr.Conditional expr) {
    resolve(expr.condition);
    resolve(expr.thenBranch);
    resolve(expr.elseBranch);
    return null;
}