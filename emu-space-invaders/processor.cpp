#include <stdio.h>
#include <Windows.h>
#include "processor.h"

void UnimplementedInstruction(State8080 *state) {
    printf("Error: Unimplemented instruction\n");
    exit(1);
}

uint8 Emulate8080Operation(State8080 *state) {
    uint8 *opcode = &state->memory[state->pc];
    switch (*opcode) {
        case 0x00: break; // NOP
        case 0x01: // LXI B,D16
            state->b = opcode[2];
            state->c = opcode[1];
            state->pc += 2;
            break;
        case 0x41: state->b = state->c; break; // MOV B,C
        case 0x42: state->b = state->d; break; // MOV B,D
        case 0x43: state->b = state->e; break; // MOV B,E
    }
    state->pc += 1;
    return 0;
}