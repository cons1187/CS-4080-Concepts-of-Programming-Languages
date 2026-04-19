//all changes are in vm.c

//first add new macros to run method
static InterpretResult run() {
  CallFrame* frame = &vm.frames[vm.frameCount - 1];
  register uint8_t* ip = frame->ip;

#define STORE_FRAME() (frame->ip = ip)
#define LOAD_FRAME()  (frame = &vm.frames[vm.frameCount - 1], ip = frame->ip)

#define READ_BYTE()     (*ip++)
#define READ_SHORT()    (ip += 2, (uint16_t)((ip[-2] << 8) | ip[-1]))
#define READ_CONSTANT() \
    (frame->closure->function->chunk.constants.values[READ_BYTE()])
#define READ_STRING() AS_STRING(READ_CONSTANT())

//edit OP_CALL case
case OP_CALL: {
        int argCount = READ_BYTE();
        STORE_FRAME();
        if (!callValue(peek(argCount), argCount)) {
          return INTERPRET_RUNTIME_ERROR;
        }
        LOAD_FRAME();
        break;
      }

//edit OP_RETURN case
case OP_RETURN: {
        Value result = pop();
        closeUpvalues(frame->slots);
        vm.frameCount--;
        if (vm.frameCount == 0) {
          pop();
          return INTERPRET_OK;
        }
        vm.stackTop = frame->slots;
        push(result);
        LOAD_FRAME();
        break;
      }

//edit OP_INVOKE AND OP_SUPER_INVOKE cases
case OP_INVOKE: {
        ObjString* method = READ_STRING();
        int argCount = READ_BYTE();
        STORE_FRAME();
        if (!invoke(method, argCount)) {
          return INTERPRET_RUNTIME_ERROR;
        }
        LOAD_FRAME();
        break;
      }

case OP_SUPER_INVOKE: {
        ObjString* method = READ_STRING();
        int argCount = READ_BYTE();
        ObjClass* superclass = AS_CLASS(pop());
        STORE_FRAME();
        if (!invokeFromClass(superclass, method, argCount)) {
          return INTERPRET_RUNTIME_ERROR;
        }
        LOAD_FRAME();
        break;
      }

//add one more macro at the top of the method
#define RUNTIME_ERROR(...) \
    do { \
      STORE_FRAME(); \
      runtimeError(__VA_ARGS__); \
      return INTERPRET_RUNTIME_ERROR; \
    } while (false)

//then replace all patterns in this format:
runtimeError("*error text*");
return INTERPRET_RUNTIME_ERROR;
//with
RUNTIME_ERROR("*error text*");

//clean up new macros at the end of the method
#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
#undef STORE_FRAME
#undef LOAD_FRAME
#undef RUNTIME_ERROR