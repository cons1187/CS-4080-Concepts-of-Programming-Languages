//need to revert the changes made in arrayString.c if this implementation is to work, the two are mutually exclusive
//edit ObjString struct in object.h
struct ObjString {
  Obj obj;
  int length;
  uint32_t hash;
  bool ownsChars;   // true = we allocated chars, we must free them
  char* chars;      // pointer to either owned heap memory or source string
};

//edit allocateString, copyString, and takeString methods in object.c
static ObjString* allocateString(char* chars, int length,
                                  uint32_t hash, bool ownsChars) {
  ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
  string->length = length;
  string->hash = hash;
  string->ownsChars = ownsChars;
  string->chars = chars;

  push(OBJ_VAL(string));
  tableSet(&vm.strings, string, NIL_VAL);
  pop();
  return string;
}

ObjString* copyString(const char* chars, int length) {
  uint32_t hash = hashString(chars, length);
  ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
  if (interned != NULL) return interned;

  // Point directly into the source — no copy, no ownership
  // Safe because source strings (string literals) live for
  // the lifetime of the program
  return allocateString((char*)chars, length, hash, false);
}

ObjString* takeString(char* chars, int length) {
  uint32_t hash = hashString(chars, length);
  ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
  if (interned != NULL) {
    // Already interned — free the buffer we were given
    FREE_ARRAY(char, chars, length + 1);
    return interned;
  }
  // Takes ownership of the heap buffer
  return allocateString(chars, length, hash, true);
}

//edit string object case in freeObject method inside of memory.c
case OBJ_STRING: {
  ObjString* string = (ObjString*)object;
  if (string->ownsChars) {
    FREE_ARRAY(char, string->chars, string->length + 1);
  }
  // Always free the ObjString struct itself
  FREE(ObjString, object);
  break;
}