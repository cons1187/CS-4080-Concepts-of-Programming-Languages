//add another native function in vm.c
static Value deleteFieldNative(int argCount, Value* args) {
  if (!IS_INSTANCE(args[0])) {
    runtimeError("First argument to 'deleteField' must be an instance.");
    return NATIVE_ERROR_VAL;
  }
  if (!IS_STRING(args[1])) {
    runtimeError("Second argument to 'deleteField' must be a string.");
    return NATIVE_ERROR_VAL;
  }
  ObjInstance* instance = AS_INSTANCE(args[0]);
  tableDelete(&instance->fields, AS_STRING(args[1]));
  return NIL_VAL;
}

defineNative("deleteField", deleteFieldNative, 2);