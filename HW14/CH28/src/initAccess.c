//update ObjClass struct in object.h
typedef struct {
  Obj obj;
  ObjString* name;
  Table methods;
  ObjClosure* init;
} ObjClass;

//initialize to Null in object.c newClass method
ObjClass* newClass(ObjString* name) {
  ObjClass* klass = ALLOCATE_OBJ(ObjClass, OBJ_CLASS);
  klass->name = name;
  klass->init = NULL;   // add this
  initTable(&klass->methods);
  return klass;
}

//update defineMethod in vm.c
static void defineMethod(ObjString* name) {
  Value method = peek(0);
  ObjClass* klass = AS_CLASS(peek(1));
  tableSet(&klass->methods, name, method);
  if (name == vm.initString) {
    klass->init = AS_CLOSURE(method);
  }
  pop();
}

//update OBJ_CLASS case in vm.c inside callValue method
case OBJ_CLASS: {
  ObjClass* klass = AS_CLASS(callee);
  vm.stackTop[-argCount - 1] = OBJ_VAL(newInstance(klass));
  if (klass->init != NULL) {
    return call(klass->init, argCount);
  } else if (argCount != 0) {
    runtimeError("Expected 0 arguments but got %d.", argCount);
    return false;
  }
  return true;
}

//update OBJ_CLASS case in memory.c
case OBJ_CLASS: {
  ObjClass* klass = (ObjClass*)object;
  markObject((Obj*)klass->name);
  markObject((Obj*)klass->init);  // add this
  markTable(&klass->methods);
  break;
}