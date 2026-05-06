//add new fields in object.h structs
struct ObjClass;
typedef struct {
    Obj obj;
    ObjFunction* function;
    ObjUpvalue** upvalues;
    int upvalueCount;
    struct ObjClass* owner;  //new
} ObjClosure;

typedef struct ObjClass {
    Obj obj;
    ObjString* name;
    Table methods;
    Table ownMethods;        //new
    struct ObjClass* superclass;
} ObjClass;

//in vm.c edit defineMethod
static void defineMethod(ObjString* name) {
    Value method = peek(0);
    ObjClass* klass = AS_CLASS(peek(1));
    ObjClosure* closure = AS_CLOSURE(method);
    closure->owner = klass;

    tableSet(&klass->ownMethods, name, method);

    Value existing;
    if (!tableGet(&klass->methods, name, &existing)) {
        tableSet(&klass->methods, name, method);
    }
    pop();
}

//add op_inner case in vm.c
case OP_INNER: {
    ObjString* name = AS_STRING(READ_STRING());
    int argCount = READ_BYTE();
    ObjClass* own = frame->closure->owner;
    ObjInstance* instance = AS_INSTANCE(peek(argCount));
    ObjClass* path[64];
    int pathLen = 0;
    for (ObjClass* k = instance->klass; k != NULL && k != own && pathLen < 64;
         k = k->superclass) {
        path[pathLen++] = k;
    }
    Value method;
    bool found = false;
    bool fnd = false;
    for (int i = pathLen - 1; i >= 0; i--) {
        if (tableGet(&path[i]->ownMethods, name, &method)) { found = true; break; }
    }
    if (!found) {
        vm.stackTop -= argCount;
        *(vm.stackTop - 1) = NIL_VAL;
    }
    STORE_FRAME();
    if (!callAS_CLOSURE(method), argCount)) return INTERPRET_RUNTIME_ERROR;
    LOAD_FRAME();
}

//in compiler.c add inner_ handler method
static void inner_(bool canAssign) {
    (void)canAssign;
    if (currentClass == NULL ||
        (current->type != TYPE_METHOD && current->type != TYPE_INITIALIZER)) {
        error("Can't use 'inner' outside of a method.");
        return;
    }

    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'inner'.");
    emitBytes(OP_GET_LOCAL, 0);
    uint8_t argCount = argumentList();

    ObjString* name = current->function->name;
    uint8_t nameConst = makeConstant(OBJ_VAL(name));
    emitBytes(OP_INNER, nameConst);
    emitByte(argCount);
}