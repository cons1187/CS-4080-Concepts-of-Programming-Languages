//Add these statements in Interpreter.java
private boolean replMode = false;

public void setReplMode(boolean mode) {
    this.replMode = mode;
}

//Modify visitExpressionStmt() method in Interpreter.java
@Override
public Void visitExpressionStmt(Stmt.Expression stmt) {
    Object value = evaluate(stmt.expression);  //Store the value

    if (replMode) {
        System.out.println(stringify(value));
    }

    return null;
}

//In Lox.java, edit the runPrompt() method to enable REPL mode
private static void runPrompt() throws IOException {
    InputStreamReader input = new InputStreamReader(System.in);
    BufferedReader reader = new BufferedReader(input);

    interpreter.setReplMode(true);

    for (;;) {
        System.out.print("> ");
        String line = reader.readLine();
        if (line == null) break;
        run(line);
        hadError = false;
    }

    interpreter.setReplMode(false);
}