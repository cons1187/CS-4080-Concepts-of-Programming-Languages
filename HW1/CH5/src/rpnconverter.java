package com.craftinginterpreters.lox;

class RpnPrinter implements Expr.Visitor<String> {

    String print(Expr expr) {
        return expr.accept(this);
    }

    @Override
    public String visitBinaryExpr(Expr.Binary expr) {
        return expr.left.accept(this)
                + " " + expr.right.accept(this)
                + " " + expr.operator.lexeme;
    }

    @Override
    public String visitUnaryExpr(Expr.Unary expr) {
        return expr.right.accept(this)
                + " " + expr.operator.lexeme;
    }

    @Override
    public String visitGroupingExpr(Expr.Grouping expr) {
        return expr.expression.accept(this);
    }

    @Override
    public String visitLiteralExpr(Expr.Literal expr) {
        if (expr.value == null) return "nil";
        return expr.value.toString();
    }

    @Override
    public String visitVariableExpr(Expr.Variable expr) {
        return expr.name.lexeme;
    }

    @Override
    public String visitAssignExpr(Expr.Assign expr) {
        return expr.value.accept(this) + " " + expr.name.lexeme + " =";
    }

    @Override
    public String visitLogicalExpr(Expr.Logical expr) {
        return expr.left.accept(this)
                + " " + expr.right.accept(this)
                + " " + expr.operator.lexeme;
    }

    @Override
    public String visitCallExpr(Expr.Call expr) {
        StringBuilder sb = new StringBuilder();
        for (Expr arg : expr.arguments) {
            sb.append(arg.accept(this)).append(" ");
        }
        sb.append(expr.callee.accept(this)).append(" call");
        return sb.toString();
    }

    @Override
    public String visitGetExpr(Expr.Get expr) {
        return expr.object.accept(this) + " ." + expr.name.lexeme;
    }

    @Override
    public String visitSetExpr(Expr.Set expr) {
        return expr.object.accept(this)
                + " " + expr.value.accept(this)
                + " ." + expr.name.lexeme + " =";
    }

    @Override
    public String visitThisExpr(Expr.This expr) {
        return "this";
    }

    @Override
    public String visitSuperExpr(Expr.Super expr) {
        return "super." + expr.method.lexeme;
    }

    public static void main(String[] args) {
        // (1 + 2) * (4 - 3)  ->  "1 2 + 4 3 - *"
        Expr expression = new Expr.Binary(
                new Expr.Grouping(
                        new Expr.Binary(
                                new Expr.Literal(1),
                                new Token(TokenType.PLUS, "+", null, 1),
                                new Expr.Literal(2)
                        )
                ),
                new Token(TokenType.STAR, "*", null, 1),
                new Expr.Grouping(
                        new Expr.Binary(
                                new Expr.Literal(4),
                                new Token(TokenType.MINUS, "-", null, 1),
                                new Expr.Literal(3)
                        )
                )
        );

        RpnPrinter printer = new RpnPrinter();
        System.out.println("(1 + 2) * (4 - 3)");
        System.out.println("Expected: 1 2 + 4 3 - *");
        System.out.println("Got:      " + printer.print(expression));
    }
}