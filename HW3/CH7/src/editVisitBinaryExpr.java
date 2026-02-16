//Question 2
case PLUS:
        if (left instanceof Double && right instanceof Double) {
        return (double)left + (double)right;
  }
          if (left instanceof String || right instanceof String) {  //change and (&&) to or (||)
        return stringify(left) + stringify(right); //use stringify method
  }
          throw new RuntimeError(expr.operator,
      "Operands must be two numbers or two strings.");

//Question 3
case SLASH:
checkNumberOperands(expr.operator, left, right);
double divisor = (double)right;
  if (divisor == 0.0) { //check if the divisor is zero and throw a runtime error if it is
        throw new RuntimeError(expr.operator, "Division by zero.");
  }

          return (double)left / divisor;