//Add BREAK in TokenType.java
public enum TokenType {
    //single character tokens and literals...

    // Keywords
    AND, CLASS, ELSE, FALSE, FUN, FOR, IF, NIL, OR,
    PRINT, RETURN, SUPER, THIS, TRUE, VAR, WHILE,
    BREAK,  //Add break keyword

    EOF
}

//Add break into Scanner.java hash map
private static final Map<String, TokenType> keywords;

static {
    keywords = new HashMap<>();
    keywords.put("and",    AND);
    keywords.put("class",  CLASS);
    keywords.put("else",   ELSE);
    keywords.put("false",  FALSE);
    keywords.put("fun",    FUN);
    keywords.put("for",    FOR);
    keywords.put("if",     IF);
    keywords.put("nil",    NIL);
    keywords.put("or",     OR);
    keywords.put("print",  PRINT);
    keywords.put("return", RETURN);
    keywords.put("super",  SUPER);
    keywords.put("this",   THIS);
    keywords.put("true",   TRUE);
    keywords.put("var",    VAR);
    keywords.put("while",  WHILE);
    keywords.put("break",  BREAK);  //Add break
}

//Add break to Stmt.java
static class Break extends Stmt {
    Break(Token keyword) {
        this.keyword = keyword;
    }

    @Override
    <R> R accept(Visitor<R> visitor) {
        return visitor.visitBreakStmt(this);
    }

    final Token keyword;
}

//Add to visitor interface
interface Visitor<R> {
    R visitBlockStmt(Block stmt);
    R visitBreakStmt(Break stmt);  //Add break to visitor interface
    R visitClassStmt(Class stmt);
    R visitExpressionStmt(Expression stmt);
    R visitFunctionStmt(Function stmt);
    R visitIfStmt(If stmt);
    R visitPrintStmt(Print stmt);
    R visitReturnStmt(Return stmt);
    R visitVarStmt(Var stmt);
    R visitWhileStmt(While stmt);
}

//Add break statement parsing in Parser.java
private Stmt statement() {
    if (match(FOR)) return forStatement();
    if (match(IF)) return ifStatement();
    if (match(PRINT)) return printStatement();
    if (match(RETURN)) return returnStatement();
    if (match(WHILE)) return whileStatement();
    if (match(LEFT_BRACE)) return new Stmt.Block(block());
    if (match(BREAK)) return breakStatement();  //Add break statement

    return expressionStatement();
}

private Stmt breakStatement() {
    Token keyword = previous();
    consume(SEMICOLON, "Expect ';' after 'break'.");
    return new Stmt.Break(keyword); //Parse break statement
}

//Add tracking in Resolver.java
private LoopType currentLoop = LoopType.NONE; //Track loop nesting

private enum LoopType {
    NONE,
    LOOP
}

//Edit visitWhileStmt() method in Resolver.java
@Override
public Void visitWhileStmt(Stmt.While stmt) {
    LoopType enclosingLoop = currentLoop;  //Save current state
    currentLoop = LoopType.LOOP;           //Mark in a loop

    resolve(stmt.condition);
    resolve(stmt.body);

    currentLoop = enclosingLoop;           //Restore state
    return null;
}

//Add visitBreakStmt() method in Resolver.java
@Override
public Void visitBreakStmt(Stmt.Break stmt) {
    if (currentLoop == LoopType.NONE) {
        Lox.error(stmt.keyword, "Can't use 'break' outside of a loop.");
    }
    return null;
}

//Add visitBreakStmt() method in AstPrinter.java
@Override
public String visitBreakStmt(Stmt.Break stmt) {
    return "break";
}

//Add error handling in Interpreter.java
private static class BreakException extends RuntimeException {}

@Override
public Void visitBreakStmt(Stmt.Break stmt) { //Add visitBreakStmt method
    throw new BreakException();
}

//Edit visitWhileStmt() method in Interpreter.java to catch errors
@Override
public Void visitWhileStmt(Stmt.While stmt) {
    try {
        while (isTruthy(evaluate(stmt.condition))) {
            execute(stmt.body);
        }
    } catch (BreakException ex) {
        //Break out of loop
    }
    return null;
}
