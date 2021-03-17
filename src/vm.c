//
// Created by aramh on 3/13/2021.
//

#include <stdarg.h>
#include <stdio.h>
#include "vm.h"
#include "debug.h"
#include "compiler.h"

VM vm;

static void resetStack() {
    vm.stackTop = vm.stack;
}

static void runtimeError(const char *format, ...) {
    va_list args;
            va_start(args, format);
    vfprintf(stderr, format, args);
            va_end(args);
    fputs("\n", stderr);

    size_t instruction = vm.ip - vm.chunk->code - 1;
    int line = vm.chunk->lines[instruction];
    fprintf(stderr, "[line %d] in code\n", line);
    resetStack();
}

void initVM() {
    resetStack();
}

void freeVM() {

}

static Value peek(int distance);

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(valueType, op)                      \
    do {                                              \
        if(!IS_NUM(peek(0)) || !IS_NUM(peek(1))) {    \
            runtimeError("Operand must be a number"); \
            return RUNTIME_ERROR;                     \
        }                                             \
        double b = AS_NUM(pop());                     \
        double a = AS_NUM(pop());                     \
        push(valueType(a op b));                      \
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
                BINARY_OP(NUM_VAL, +);
                break;
            case OP_SUB:
                BINARY_OP(NUM_VAL, -);
                break;
            case OP_MUL:
                BINARY_OP(NUM_VAL, *);
                break;
            case OP_DIV:
                BINARY_OP(NUM_VAL, /);
                break;
            case OP_NEGATE: {
                if (!IS_NUM(peek(0))) {
                    runtimeError("Operand must be a number");
                    return RUNTIME_ERROR;
                }
                push(NUM_VAL(-AS_NUM(pop())));
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
    Chunk chunk;
    initChunk(&chunk);
    if (!compile(source, &chunk)) {
        freeChunk(&chunk);
        return COMPILE_ERROR;
    }

    vm.chunk = &chunk;
    vm.ip = vm.chunk->code;
    InterpretResult result = run();
    freeChunk(&chunk);
    return result;
}

void push(Value value) {
    *vm.stackTop = value;
    vm.stackTop++;
}

Value pop() {
    vm.stackTop -= 1;
    return *vm.stackTop;
}

static Value peek(int distance) {
    return vm.stackTop[-1 - distance];
}
