//edit ObjString struct in object.h
struct ObjString {
  Obj obj;
  int length;
  uint32_t hash;
  char chars[];    // flexible array member — must be last
};

//edit allocateString, copyString, and takeString methods in object.c
static ObjString* allocateString(const char* chars, int length, uint32_t hash) {
  // Allocate ObjString + room for chars + null terminator in one block
  ObjString* string = (ObjString*)reallocate(NULL, 0, sizeof(ObjString) + length + 1);
  string->obj.type = OBJ_STRING;
  string->length = length;
  string->hash = hash;
  memcpy(string->chars, chars, length);
  string->chars[length] = '\0';
  push(OBJ_VAL(string));
  tableSet(&vm.strings, string, NIL_VAL);
  pop();
  return string;
}

ObjString* copyString(const char* chars, int length) {
  uint32_t hash = hashString(chars, length);
  ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
  if (interned != NULL) return interned;
  // No separate heap allocation needed — allocateString handles it
  return allocateString(chars, length, hash);
}

ObjString* takeString(char* chars, int length) {
  uint32_t hash = hashString(chars, length);
  ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
  if (interned != NULL) {
    FREE_ARRAY(char, chars, length + 1);
    return interned;
  }
  // Copy into the new combined allocation, then free the old buffer
  ObjString* string = allocateString(chars, length, hash);
  FREE_ARRAY(char, chars, length + 1);
  return string;
}

//edit string object case in freeObject method inside of memory.c
case OBJ_STRING: {
  ObjString* string = (ObjString*)object;
  // chars are part of the same allocation — just free the whole block
  reallocate(object, sizeof(ObjString) + string->length + 1, 0);
  break;
}