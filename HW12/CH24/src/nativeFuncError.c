//in value.h add to enum
typedef enum {
  VAL_BOOL,
  VAL_NIL,
  VAL_NUMBER,
  VAL_OBJ,
  VAL_NATIVE_ERROR,  //signals a native function runtime error
} ValueType;

//add new macros
#define NATIVE_ERROR_VAL    ((Value){VAL_NATIVE_ERROR, {.number = 0}})
#define IS_NATIVE_ERROR(v)  ((v).type == VAL_NATIVE_ERROR)

//in vm.h expose runtimeError method
void runtimeError(const char* format, ...);

//edit OBJ_NATIVE case in vm.c, make sure to remove the word static from the runtimeError function call
case OBJ_NATIVE: {
      ObjNative* native = AS_NATIVE(callee);
      if (argCount != native->arity) {
        runtimeError("Expected %d arguments but got %d.",
                     native->arity, argCount);
        return false;
      }
      Value result = native->function(argCount, vm.stackTop - argCount);
      if (IS_NATIVE_ERROR(result)) return false;
      vm.stackTop -= argCount + 1;
      push(result);
      return true;
    }

//adding sqrt native method for testing
static Value sqrtNative(int argCount, Value* args) {
  if (!IS_NUMBER(args[0])) {
    runtimeError("Argument to 'sqrt' must be a number.");
    return NATIVE_ERROR_VAL;
  }
  if (AS_NUMBER(args[0]) < 0) {
    runtimeError("Argument to 'sqrt' must not be negative.");
    return NATIVE_ERROR_VAL;
  }
  return NUMBER_VAL(sqrt(AS_NUMBER(args[0])));
}
//need to add #include <math.h> at top of file

//register it in initVM method
defineNative("sqrt", sqrtNative, 1);