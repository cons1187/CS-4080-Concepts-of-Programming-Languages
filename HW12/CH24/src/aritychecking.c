//in object.h add arity to the struct and update native method declaration
typedef Value (*NativeFn)(int argCount, Value* args);

typedef struct {
  Obj obj;
  int arity;
  NativeFn function;
} ObjNative;

ObjNative* newNative(NativeFn function, int arity);

//also update AS_NATIVE macro
#define AS_NATIVE(value) ((ObjNative*)AS_OBJ(value))

//in object.c update newNative method
ObjNative* newNative(NativeFn function, int arity) {
  ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
  native->arity = arity;
  native->function = function;
  return native;
}

//in vm.c update defineNative function
static void defineNative(const char* name, NativeFn function, int arity) {
  push(OBJ_VAL(copyString(name, (int)strlen(name))));
  push(OBJ_VAL(newNative(function, arity)));
  tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
  pop();
  pop();
}

//add number of arguments to built in clock method
defineNative("clock", clockNative, 0);

//add arity in callValue method
case OBJ_NATIVE: {
      ObjNative* native = AS_NATIVE(callee);
      if (argCount != native->arity) {
        runtimeError("Expected %d arguments but got %d.",
                     native->arity, argCount);
        return false;
      }
      Value result = native->function(argCount, vm.stackTop - argCount);
      vm.stackTop -= argCount + 1;
      push(result);
      return true;
    }