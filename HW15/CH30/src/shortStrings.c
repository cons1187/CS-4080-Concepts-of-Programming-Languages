//in value.h add these items
#define SHORT_STRING_MAX 7
#define IS_SHORT_STRING(value)  ((value).type == VAL_SHORT_STRING)
#define IS_ANY_STRING(value)    (IS_STRING(value) || IS_SHORT_STRING(value))
#define AS_SHORT_STRING(value)  ((value).as.shortStr)

//edit ValueType enum in value.h
typedef enum {
  VAL_BOOL,
  VAL_NIL,
  VAL_NUMBER,
  VAL_OBJ,
  VAL_SHORT_STRING, //added
} ValueType;

//edit Value struct in value.h
typedef struct {
  ValueType type;
  union {
    bool boolean;
    double number;
    Obj* obj;
    struct {    //added
      uint8_t length;
      char chars[SHORT_STRING_MAX];
    } shortStr;
  } as;
} Value;

//declare new methods in value.h
Value makeShortString(const char* chars, int length);
bool stringsEqual(Value a, Value b);

//in value.c add makeShortString method
Value makeShortString(const char* chars, int length) {
  Value value;
  value.type = VAL_SHORT_STRING;
  value.as.shortStr.length = (uint8_t)length;
  memcpy(value.as.shortStr.chars, chars, length);
  if (length < SHORT_STRING_MAX) {
    value.as.shortStr.chars[length] = '\0';
  }
  return value;
}


//update printValue and valuesEqual methods in value.c
void printValue(Value value) {
  switch (value.type) {
    case VAL_BOOL:
      printf(AS_BOOL(value) ? "true" : "false"); break;
    case VAL_NIL:
      printf("nil"); break;
    case VAL_NUMBER:
      printf("%g", AS_NUMBER(value)); break;
    case VAL_OBJ:
      printObject(value); break;
    case VAL_SHORT_STRING:        //new case
      printf("%.*s", value.as.shortStr.length,
                     value.as.shortStr.chars); break;
  }
}

bool valuesEqual(Value a, Value b) {
  if (IS_ANY_STRING(a) && IS_ANY_STRING(b)) {
    return stringsEqual(a, b);
  }
  if (a.type != b.type) return false;
  switch (a.type) {
    case VAL_BOOL:         return AS_BOOL(a) == AS_BOOL(b);
    case VAL_NIL:          return true;
    case VAL_NUMBER:       return AS_NUMBER(a) == AS_NUMBER(b);
    case VAL_OBJ:          return AS_OBJ(a) == AS_OBJ(b);
    case VAL_SHORT_STRING: return stringsEqual(a, b); //new case
    default:               return false;
  }
}

//declare new method in object.h
Value makeStringValue(const char* chars, int length);

//implement it in object.c
Value makeStringValue(const char* chars, int length) {
  if (length <= SHORT_STRING_MAX) {
    return makeShortString(chars, length);
  }
  return OBJ_VAL(copyString(chars, length));
}

//in vm.c add stringsEqual and concatenateStrings methods
bool stringsEqual(Value a, Value b) {
  const char* aChars;
  int aLen;
  const char* bChars;
  int bLen;

  if (IS_SHORT_STRING(a)) {
    aChars = a.as.shortStr.chars;
    aLen   = a.as.shortStr.length;
  } else {
    aChars = AS_CSTRING(a);
    aLen   = AS_STRING(a)->length;
  }

  if (IS_SHORT_STRING(b)) {
    bChars = b.as.shortStr.chars;
    bLen   = b.as.shortStr.length;
  } else {
    bChars = AS_CSTRING(b);
    bLen   = AS_STRING(b)->length;
  }

  if (aLen != bLen) return false;
  return memcmp(aChars, bChars, aLen) == 0;
}

static Value concatenateStrings(Value a, Value b) {
  const char* aChars;
  int aLen;
  const char* bChars;
  int bLen;

  if (IS_SHORT_STRING(a)) {
    aChars = a.as.shortStr.chars;
    aLen   = a.as.shortStr.length;
  } else {
    aChars = AS_CSTRING(a);
    aLen   = AS_STRING(a)->length;
  }

  if (IS_SHORT_STRING(b)) {
    bChars = b.as.shortStr.chars;
    bLen   = b.as.shortStr.length;
  } else {
    bChars = AS_CSTRING(b);
    bLen   = AS_STRING(b)->length;
  }

  int totalLen = aLen + bLen;
  if (totalLen <= SHORT_STRING_MAX) {
    char buf[SHORT_STRING_MAX];
    memcpy(buf, aChars, aLen);
    memcpy(buf + aLen, bChars, bLen);
    return makeShortString(buf, totalLen);
  }

  char* heapChars = ALLOCATE(char, totalLen + 1);
  memcpy(heapChars, aChars, aLen);
  memcpy(heapChars + aLen, bChars, bLen);
  heapChars[totalLen] = '\0';
  return OBJ_VAL(takeString(heapChars, totalLen));
}

//update OP_EQUAL and OP_ADD cases in vm.c
case OP_EQUAL: {
  Value b = pop();
  Value a = pop();
  if (IS_ANY_STRING(a) && IS_ANY_STRING(b)) {
    push(BOOL_VAL(stringsEqual(a, b)));
  } else {
    push(BOOL_VAL(valuesEqual(a, b)));
  }
  break;
}

//replace concatenate in OP_ADD
if (IS_ANY_STRING(peek(0)) && IS_ANY_STRING(peek(1))) {
  Value b = pop();
  Value a = pop();
  push(concatenateStrings(a, b));
}