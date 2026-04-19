//native for checking string length
static Value strLenNative(int argCount, Value* args) {
  if (!IS_OBJ(args[0]) || !IS_STRING(args[0])) {
    runtimeError("Argument to 'strLen' must be a string.");
    return NATIVE_ERROR_VAL;
  }
  return NUMBER_VAL(AS_STRING(args[0])->length);
}

//native for checking if a string contains a substring
static Value containsNative(int argCount, Value* args) {
  if (!IS_STRING(args[0]) || !IS_STRING(args[1])) {
    runtimeError("Arguments to 'contains' must be strings.");
    return NATIVE_ERROR_VAL;
  }
  char* haystack = AS_CSTRING(args[0]);
  char* needle   = AS_CSTRING(args[1]);
  return BOOL_VAL(strstr(haystack, needle) != NULL);
}

//type conversion natives
static Value isNumberNative(int argCount, Value* args) {
  return BOOL_VAL(IS_NUMBER(args[0]));
}

static Value isStringNative(int argCount, Value* args) {
  return BOOL_VAL(IS_STRING(args[0]));
}

static Value isBoolNative(int argCount, Value* args) {
  return BOOL_VAL(IS_BOOL(args[0]));
}

static Value isNilNative(int argCount, Value* args) {
  return BOOL_VAL(IS_NIL(args[0]));
}

// number to string - allocates a new Lox string
static Value numToStrNative(int argCount, Value* args) {
  if (!IS_NUMBER(args[0])) {
    runtimeError("Argument to 'numToStr' must be a number.");
    return NATIVE_ERROR_VAL;
  }
  char buf[64];
  snprintf(buf, sizeof(buf), "%g", AS_NUMBER(args[0]));
  return OBJ_VAL(copyString(buf, strlen(buf)));
}

//register in initVM
defineNative("strLen",    strLenNative,    1);
defineNative("contains",  containsNative,  2);
defineNative("isNumber",  isNumberNative,  1);
defineNative("isString",  isStringNative,  1);
defineNative("isBool",    isBoolNative,    1);
defineNative("isNil",     isNilNative,     1);
defineNative("numToStr",  numToStrNative,  1);