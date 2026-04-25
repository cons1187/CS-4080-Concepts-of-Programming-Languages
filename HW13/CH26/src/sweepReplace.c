//in object.h add new field to Obj struct
struct Obj {
  ObjType type;
  bool isMarked;
  int refCount; //new
  struct Obj* next;
} Obj;

//in object.c initialize reference count
static Obj* allocateObject(size_t size, ObjType type) {
  Obj* object = (Obj*)reallocate(NULL, 0, size);
  object->type = type;
  object->isMarked = false;
  object->refCount = 0; //new
  object->next = vm.objects;
  vm.objects = object;
  return object;
}

//in vm.h add new method declarations
void incrementRef(Obj* object);
void decrementRef(Obj* object);

//implement them in vm.c
void incrementRef(Obj* object) {
  if (object == NULL) return;
  object->refCount++;
}

void decrementRef(Obj* object) {
  if (object == NULL) return;
  object->refCount--;
  if (object->refCount <= 0) {
    freeObject(object);
  }
}

//add reference calls to various places in vm.c
void push(Value value) {
  if (IS_OBJ(value)) incrementRef(AS_OBJ(value));
  *vm.stackTop = value;
  vm.stackTop++;
}

Value pop() {
  vm.stackTop--;
  Value value = *vm.stackTop;
  if (IS_OBJ(value)) decrementRef(AS_OBJ(value));
  return value;
}

//inside run function
case OP_DEFINE_GLOBAL: {
  ObjString* name = READ_STRING();
  Value value = peek(0);
  //decrement old value if redefining
  Value oldValue;
  if (tableGet(&vm.globals, name, &oldValue)) {
    if (IS_OBJ(oldValue)) decrementRef(AS_OBJ(oldValue));
  }
  //increment new value
  if (IS_OBJ(value)) incrementRef(AS_OBJ(value));
  tableSet(&vm.globals, name, value);
  pop();
  break;
}

case OP_SET_GLOBAL: {
  ObjString* name = READ_STRING();
  Value value = peek(0);
  //decrement old value
  Value oldValue;
  if (tableGet(&vm.globals, name, &oldValue)) {
    if (IS_OBJ(oldValue)) decrementRef(AS_OBJ(oldValue));
  } else {
    runtimeError("Undefined variable '%s'.", name->chars);
    return INTERPRET_RUNTIME_ERROR;
  }
  //increment new value
  if (IS_OBJ(value)) incrementRef(AS_OBJ(value));
  tableSet(&vm.globals, name, value);
  break;
}