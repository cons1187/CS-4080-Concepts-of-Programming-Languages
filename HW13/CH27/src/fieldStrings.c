//add 2 new native functions in vm.c
static Value getFieldNative(int argCount, Value* args) {
  if (!IS_INSTANCE(args[0])) {
    runtimeError("First argument to 'getField' must be an instance.");
    return NATIVE_ERROR_VAL;
  }
  if (!IS_STRING(args[1])) {
    runtimeError("Second argument to 'getField' must be a string.");
    return NATIVE_ERROR_VAL;
  }
  ObjInstance* instance = AS_INSTANCE(args[0]);
  Value value;
  if (tableGet(&instance->fields, AS_STRING(args[1]), &value)) {
    return value;
  }
  return NIL_VAL;
}

static Value setFieldNative(int argCount, Value* args) {
  if (!IS_INSTANCE(args[0])) {
    runtimeError("First argument to 'setField' must be an instance.");
    return NATIVE_ERROR_VAL;
  }
  if (!IS_STRING(args[1])) {
    runtimeError("Second argument to 'setField' must be a string.");
    return NATIVE_ERROR_VAL;
  }
  ObjInstance* instance = AS_INSTANCE(args[0]);
  tableSet(&instance->fields, AS_STRING(args[1]), args[2]);
  return args[2];
}

//define them in initVM method
defineNative("getField", getFieldNative, 2);
defineNative("setField", setFieldNative, 3);