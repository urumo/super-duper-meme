//
// Created by aramh on 3/13/2021.
//

#include <stdio.h>
#include "vm.h"
#include "debug.h"
#include "compiler.h"

VM vm;

static void resetStack() {
    vm.stackTop = vm.stack;
}

void initVM() {
    resetStack();
}

void freeVM() {

}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(op)   \
    do {                \
      double b = pop(); \
      double a = pop(); \
      push(a op b);     \
    } while(false)

    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        printf("          ");
        for (Value *slot = vm.stack; slot < vm.stackTop; slot++) {
            printf("[ ");
            printValue(*slot);
            printf(" ]");
        }
        printf("\n");
        disassembleInstruction(vm.chunk, (int) (vm.ip - vm.chunk->code));
#endif
        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }
            case OP_ADD:
                BINARY_OP(+);
                break;
            case OP_SUB:
                BINARY_OP(-);
                break;
            case OP_MUL:
                BINARY_OP(*);
                break;
            case OP_DIV:
                BINARY_OP(/);
                break;
            case OP_NEGATE: {
                push(-pop());
                break;
            }
            case OP_RETURN: {
                printValue(pop());
                printf("\n");
                return OK;
            }
        }
    }
#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

InterpretResult interpret(const char *source) {
    compile(source);
    return OK;
}

void push(Value value) {
    *vm.stackTop = value;
    vm.stackTop += 1;
}

Value pop() {
    vm.stackTop -= 1;
    return *vm.stackTop;
}
