//in chunk.h add new opcodes
typedef enum {
 // ... existing opcodes
   OP_GET_LOCAL_LONG,
   OP_SET_LOCAL_LONG,
} Opcode

//in debug.c add new cases in disassembleInstruction
case OP_GET_LOCAL_LONG:
  return simpleInstruction("OP_GET_LOCAL_LONG", offset);

case OP_SET_LOCAL_LONG:
  return simpleInstruction("OP_SET_LOCAL_LONG", offset);

//in vm.c add new cases after switch(instruction)
case OP_GET_LOCAL_LONG: {
  uint16_t slot = READ_SHORT();
  push(vm.stack[slot]);
  break;
}

case OP_SET_LOCAL_LONG: {
  uint16_t slot = READ_SHORT();
  vm.stack[slot] = peek(0);
  break;
}

//in compiler.c make many changes
//change localCount to an int instead of uint8_t
//edit locals array to have a fixed number instead of UINT8_COUNT
//update overflow check to have a fixed number instead of UINT8_COUNT
int localCount;
Local locals[65536];
if (current->localCount == 65536)

//add emitShort method after emitByte
static void emitShort(uint16_t value) {
  emitByte((value >> 8) & 0xff);
  emitByte(value & 0xff);
}

//edit namedVariable method
static void namedVariable(Token name, bool canAssign) {
  uint8_t getOp, setOp;
  int arg = resolveLocal(current, &name);

  if (arg != -1) {
    if (arg < 256) {
      getOp = OP_GET_LOCAL;
      setOp = OP_SET_LOCAL;
    } else {
      getOp = OP_GET_LOCAL_LONG;
      setOp = OP_SET_LOCAL_LONG;
    }
  } else if ((arg = resolveUpvalue(current, &name)) != -1) {
    getOp = OP_GET_UPVALUE;
    setOp = OP_SET_UPVALUE;
  } else {
    arg = identifierConstant(&name);
    getOp = OP_GET_GLOBAL;
    setOp = OP_SET_GLOBAL;
  }

  if (canAssign && match(TOKEN_EQUAL)) {
    expression();

    if (arg < 256 || getOp == OP_GET_GLOBAL || getOp == OP_GET_UPVALUE) {
      emitBytes(setOp, (uint8_t)arg);
    } else {
      emitByte(setOp);
      emitShort((uint16_t)arg);
    }
  } else {
    if (arg < 256 || getOp == OP_GET_GLOBAL || getOp == OP_GET_UPVALUE) {
      emitBytes(getOp, (uint8_t)arg);
    } else {
      emitByte(getOp);
      emitShort((uint16_t)arg);
    }
  }
}

//edit STACK_MAX in vm.h
#define STACK_MAX 100000