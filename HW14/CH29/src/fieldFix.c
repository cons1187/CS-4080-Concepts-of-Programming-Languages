//add new token to scanner.h
TOKEN_FIELD

//add as keyword in scanner.c
case 'f':
  if (scanner.current - scanner.start > 1) {
    switch (scanner.start[1]) {
      case 'a': return checkKeyword(2, 2, "ls", TOKEN_FALSE);
      case 'i': return checkKeyword(2, 3, "eld", TOKEN_FIELD);  // NEW
      case 'o': return checkKeyword(2, 1, "r",   TOKEN_FOR);
      case 'u': return checkKeyword(2, 1, "n",   TOKEN_FUN);
    }
  }
  break;

//in compiler.c edit struct
typedef struct ClassCompiler {
  struct ClassCompiler* enclosing;
  bool hasSuperclass;
  ObjString* declaredFields[256]; //new
  int declaredFieldCount; //new
} ClassCompiler;

//also add new method for declaring fields in compiler.c
static void fieldDeclaration() {
  consume(TOKEN_IDENTIFIER, "Expect field name.");
  ObjString* fieldName = copyString(parser.previous.start,
                                    parser.previous.length);

  for (int i = 0; i < currentClass->declaredFieldCount; i++) {
    if (currentClass->declaredFields[i] == fieldName) {
      error("Field already declared in this class.");
      return;
    }
  }

  if (currentClass->declaredFieldCount == 256) {
    error("Too many field declarations in one class.");
    return;
  }
  currentClass->declaredFields[currentClass->declaredFieldCount++] = fieldName;

  uint8_t nameIdx = makeConstant(OBJ_VAL(fieldName));
  emitBytes(OP_FIELD, nameIdx);

  consume(TOKEN_SEMICOLON, "Expect ';' after field declaration.");
}

//edit class declaration method to include fields as well in compiler.c
static void classDeclaration() {
  consume(TOKEN_IDENTIFIER, "Expect class name.");
  Token className = parser.previous;
  uint8_t nameConstant = identifierConstant(&parser.previous);
  declareVariable();

  emitBytes(OP_CLASS, nameConstant);
  defineVariable(nameConstant);

  ClassCompiler classCompiler;
  classCompiler.enclosing = currentClass;
  classCompiler.hasSuperclass = false;
  classCompiler.declaredFieldCount = 0;
  currentClass = &classCompiler;

  // Handle inheritance
  if (match(TOKEN_LESS)) {
    consume(TOKEN_IDENTIFIER, "Expect superclass name.");
    variable(false);  // load superclass onto stack

    if (identifiersEqual(&className, &parser.previous)) {
      error("A class can't inherit from itself.");
    }

    beginScope();
    addLocal(syntheticToken("super"));
    defineVariable(0);

    namedVariable(className, false);
    emitByte(OP_INHERIT);
    classCompiler.hasSuperclass = true;
  }

  namedVariable(className, false);
  consume(TOKEN_LEFT_BRACE, "Expect '{' before class body.");

  while (check(TOKEN_FIELD)) {
    advance();
    fieldDeclaration();
  }
  emitByte(OP_CHECK_FIELDS);

  while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
    method();
  }

  consume(TOKEN_RIGHT_BRACE, "Expect '}' after class body.");
  emitByte(OP_POP);

  if (classCompiler.hasSuperclass) {
    endScope();
  }

  currentClass = currentClass->enclosing;
}

//add new opcodes in chunk.h
OP_FIELD,
OP_CHECK_FIELDS,

//edit struct in object.h
typedef struct ObjClass {   // <-- named tag
  Obj obj;
  ObjString* name;
  Table methods;
  ObjClosure* initializer;
  ObjString** fields;
  int fieldCount;
  int fieldCapacity;
  struct ObjClass* superclass;
} ObjClass;

//update newClass method in object.c
ObjClass* newClass(ObjString* name) {
  ObjClass* klass = ALLOCATE_OBJ(ObjClass, OBJ_CLASS);
  klass->name = name;
  initTable(&klass->methods);
  klass->initializer = NULL;
  klass->fields = NULL;
  klass->fieldCount = 0;
  klass->superclass  = NULL;
  klass->fieldCapacity = 0;
  return klass;
}

//in memory.c edit freeObject and blackenObject methods
case OBJ_CLASS: {
  ObjClass* klass = (ObjClass*)object;
  freeTable(&klass->methods);
  FREE_ARRAY(ObjString*, klass->fields, klass->fieldCapacity);
  FREE(ObjClass, object);
  break;
}

case OBJ_CLASS: {
  ObjClass* klass = (ObjClass*)object;
  markObject((Obj*)klass->name);
  markTable(&klass->methods);
  markObject((Obj*)klass->initializer);
  for (int i = 0; i < klass->fieldCount; i++) {
    markObject((Obj*)klass->fields[i]);
  }
  break;
}

//implement new opcodes in vm.c
case OP_FIELD: {
  ObjClass* klass = AS_CLASS(peek(0));
  ObjString* fieldName = AS_STRING(READ_CONSTANT());

  if (klass->fieldCount == klass->fieldCapacity) {
    int oldCap = klass->fieldCapacity;
    klass->fieldCapacity = GROW_CAPACITY(oldCap);
    klass->fields = GROW_ARRAY(ObjString*, klass->fields,
                               oldCap, klass->fieldCapacity);
  }
  klass->fields[klass->fieldCount++] = fieldName;
  break;
}

case OP_CHECK_FIELDS: {
  ObjClass* klass = AS_CLASS(peek(0));
  ObjClass* super = klass->superclass;
  while (super != NULL) {
    for (int i = 0; i < klass->fieldCount; i++) {
      for (int j = 0; j < super->fieldCount; j++) {
        if (klass->fields[i] == super->fields[j]) {
          runtimeError(
            "Field '%s' in class '%s' conflicts with field '%s' in '%s'.",
            klass->fields[i]->chars, klass->name->chars,
            super->fields[j]->chars, super->name->chars);
          return INTERPRET_RUNTIME_ERROR;
        }
      }
    }
    super = super->superclass;
  }
  break;
}

//edit inherit opcode
case OP_INHERIT: {
  Value superclassVal = peek(1);
  ObjClass* subclass = AS_CLASS(peek(0));
  tableAddAll(&AS_CLASS(superclassVal)->methods, &subclass->methods);
  subclass->superclass = AS_CLASS(superclassVal);  // NEW
  if (subclass->initializer == NULL) {
    subclass->initializer = AS_CLASS(superclassVal)->initializer;
  }
  pop();
  break;
}

//edit set property opcode
case OP_SET_PROPERTY: {
  if (!IS_INSTANCE(peek(1))) {
    runtimeError("Only instances have fields.");
    return INTERPRET_RUNTIME_ERROR;
  }
  ObjInstance* instance = AS_INSTANCE(peek(1));
  ObjString* fieldName = READ_STRING();
  bool declared = false;
  for (int i = 0; i < instance->klass->fieldCount; i++) {
    if (instance->klass->fields[i] == fieldName) {
      declared = true;
      break;
    }
  }
  if (!declared) {
    runtimeError("Undeclared field '%s' on class '%s'.",
                 fieldName->chars, instance->klass->name->chars);
    return INTERPRET_RUNTIME_ERROR;
  }

  tableSet(&instance->fields, fieldName, peek(0));
  Value value = pop();
  pop();
  push(value);
  break;
}