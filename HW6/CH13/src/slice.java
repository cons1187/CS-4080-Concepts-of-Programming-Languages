//create new file LoxSlice.java
package com.craftinginterpreters.lox;

import java.util.List;

class LoxSlice implements LoxCallable {
    @Override
    public int arity() {
        return 3;  // slice(string, start, end)
    }

    @Override
    public Object call(Interpreter interpreter, List<Object> arguments) {
        Object stringObj = arguments.get(0);
        Object startObj = arguments.get(1);
        Object endObj = arguments.get(2);

        // Validate string argument
        if (!(stringObj instanceof String)) {
            throw new RuntimeError(null,
                    "First argument to slice() must be a string.");
        }

        // Validate numeric arguments
        if (!(startObj instanceof Double) || !(endObj instanceof Double)) {
            throw new RuntimeError(null,
                    "Slice indices must be numbers.");
        }

        String str = (String)stringObj;
        int start = ((Double)startObj).intValue();
        int end = ((Double)endObj).intValue();

        // Handle negative indices (count from end)
        if (start < 0) {
            start = str.length() + start;
        }
        if (end < 0) {
            end = str.length() + end;
        }

        // Clamp to valid range
        start = Math.max(0, Math.min(start, str.length()));
        end = Math.max(0, Math.min(end, str.length()));

        // Ensure start <= end
        if (start > end) {
            return "";
        }

        return str.substring(start, end);
    }

    @Override
    public String toString() {
        return "<native fn slice>";
    }
}

//add to Interpreter.java constructor
Interpreter() {
    globals.define("clock", new LoxCallable() {
        @Override
        public int arity() { return 0; }

        @Override
        public Object call(Interpreter interpreter, List<Object> arguments) {
            return (double)System.currentTimeMillis() / 1000.0;
        }

        @Override
        public String toString() { return "<native fn>"; }
    });

    // NEW: Add slice function
    globals.define("slice", new LoxSlice());

    environment = globals;
}