//small change, edit get property case in vm.c
case OP_GET_PROPERTY: {
  if (!IS_INSTANCE(peek(0))) {
    runtimeError("Only instances have properties.");
    return INTERPRET_RUNTIME_ERROR;
  }

  ObjInstance* instance = AS_INSTANCE(peek(0));
  ObjString* name = READ_STRING();

  Value value;
  if (tableGet(&instance->fields, name, &value)) {
    pop();
    push(value);
    break;
  }

  Value method;
  if (tableGet(&instance->klass->methods, name, &method)) {
    if (!bindMethod(instance->klass, name)) {
      return INTERPRET_RUNTIME_ERROR;
    }
  } else {
    pop();
    push(NIL_VAL);
  }
  break;
}