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

//declare reference counting functions in memory.h
void incRef(Obj* value);
void decRef(Obj* value);

//implement functions in memory.c
void incRef(Obj* value) {
    value->refCount++;
}

void decRef(Obj* value) {
    if (value->refCount > 1) {
        value->refCount--;
    } else {
        // Decrement refVal of objects referenced by the current object
        switch (value->type) {
            case OBJ_FUNCTION: {
                ObjFunction* function = (ObjFunction*)value;
                if (function->name != NULL) decRef((Obj*)function->name);
                decrementArray(&function->chunk.constants);
                break;
            }
            case OBJ_UPVALUE: {
                ObjUpvalue* upvalue = (ObjUpvalue*)value;
                decrementValue(upvalue->closed);
                break;
            }
            case OBJ_CLOSURE: {
                ObjClosure* closure = (ObjClosure*)value;
                decRef((Obj*)closure->function);
                for (int i = 0; i < closure->upvalueCount; i++) {
                    decRef((Obj*)closure->upvalues[i]);
                }
                break;
            }
            case OBJ_NATIVE:
            case OBJ_STRING:
                break;
        }
    }
}

//helper functions for decrementing
static void decrementValue(Value value) {
    if (IS_OBJ(value)) decRef(AS_OBJ(value));
}

static void decrementArray(ValueArray* value_array) {
    for (int i = 0; i < value_array->count; i++) {
        decrementValue(value_array->values[i]);
    }
}

//edit sweep method
static void sweep() {
    Obj* previous = NULL;
    Obj* object = vm.objects;
    while (object != NULL) {
        // Changing sweep to free refCounts of 0
        if (object->refCount > 0) {
            previous = object;
            object = object->next;

//in compiler.c add function referencing
static void initCompiler(Compiler* compiler, FunctionType type) {
    compiler->function = newFunction();
    // This function is referenced by the compiler
    incRef((Obj*) compiler->function);

static ObjFunction* endCompiler() {
    current = current->enclosing;
    // Function no longer referenced by the compiler
    decRef((Obj*)function);
    return function;
}

//in table.c manage references for global variables
bool tableSet(Table* table, ObjString* key, Value value) {
    // Increase reference when assigning to global variable
    if (IS_OBJ(value)) incRef(AS_OBJ(value));

    // If entry exists, decrement the reference value of the object stored
    if (!isNewKey && IS_OBJ(entry->value)) decRef(AS_OBJ(entry->value));

//in vm.c edit push, pop, and upvalue captures with references
// Pushes trigger a reference count increase
void push(Value value) {
    if (IS_OBJ(value)) incRef(AS_OBJ(value));
    *vm.stackTop = value;
    vm.stackTop++;
}

// Pops trigger a reference count decrease
Value pop() {
    if (IS_OBJ(*vm.stackTop)) decRef(AS_OBJ(*vm.stackTop));
    vm.stackTop--;
    return *vm.stackTop;
}

static ObjUpvalue* captureUpvalue(Value* local) {
    // Upvalue is referenced immediately, so refCount is updated
    ObjUpvalue* createdUpvalue = newUpvalue(local);
    incRef((Obj*)createdUpvalue);
    // Increment the reference count of the local if it is an object
    if (IS_OBJ(*local)) incRef(AS_OBJ(*local));
    createdUpvalue->next = upvalue;