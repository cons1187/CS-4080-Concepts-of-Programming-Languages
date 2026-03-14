//edit stack definition in vm.h
// Remove this:
#define STACK_MAX 256  // or (FRAMES_MAX * UINT8_COUNT)
// And this field in VM struct:
Value stack[STACK_MAX];

// Replace with these fields in VM struct:
Value* stack;
int stackCapacity;

//in vm.c add these fields to the top of the initVM method
void initVM() {
  vm.stackCapacity = 256;  // initial size
  vm.stack = (Value*)reallocate(NULL, 0, sizeof(Value) * vm.stackCapacity);
  vm.stackTop = vm.stack;
  // ... rest of initVM unchanged
}

//edit freeVM in vm.c
void freeVM() {
  reallocate(vm.stack, sizeof(Value) * vm.stackCapacity, 0);
  // ... rest of freeVM unchanged
}

//edit push method in vm.c
void push(Value value) {
  if (vm.stackTop >= vm.stack + vm.stackCapacity) {
    int oldCapacity = vm.stackCapacity;
    int stackTopOffset = (int)(vm.stackTop - vm.stack);

    // Save all frame slot offsets
    int frameOffsets[FRAMES_MAX];
    for (int i = 0; i < vm.frameCount; i++) {
      frameOffsets[i] = (int)(vm.frames[i].slots - vm.stack);
    }

    vm.stackCapacity = GROW_CAPACITY(oldCapacity);
    vm.stack = (Value*)reallocate(vm.stack,
                                  sizeof(Value) * oldCapacity,
                                  sizeof(Value) * vm.stackCapacity);

    // Restore stackTop and all frame slots
    vm.stackTop = vm.stack + stackTopOffset;
    for (int i = 0; i < vm.frameCount; i++) {
      vm.frames[i].slots = vm.stack + frameOffsets[i];
    }
  }
  *vm.stackTop = value;
  vm.stackTop++;
}