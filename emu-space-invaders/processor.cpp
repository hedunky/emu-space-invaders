#include <stdio.h>
#include <Windows.h>
#include "processor.h"

void UnimplementedInstruction(State8080 *state) {
	uint8 *opcode = &state->memory[state->pc];
    printf("Error: Unimplemented instruction %0x \n", *opcode);
}

uint8 Emulate8080Operation(State8080 *state) {
    uint8 *opcode = &state->memory[state->pc];
    switch (*opcode) {
		case 0x00: { // NOP
			state->pc += 1;
		} break;

		case 0x01: { // LXI B,D16
			state->b = opcode[2];
			state->c = opcode[1];
			state->pc += 3;
		} break;

		case 0x31: { // LXI SP,D16
			state->sp = (opcode[2] << 8) | opcode[1];
			state->pc += 3;
		} break;

		case 0x06: { // MVI B, D8
			state->b = opcode[1];
			state->pc += 2;
		} break;

		case 0xc3: { // JMP addr
			state->pc = (opcode[2] << 8) | opcode[1];
		} break;

		case 0xcd: { // CALL addr
			uint16 ret = state->pc + 2;
			state->memory[state->sp - 1] = (ret >> 8) & (0xff);
			state->memory[state->sp - 2] = ret & 0xff;
			state->sp = state->sp - 2;
			state->pc = (opcode[2] << 8) | opcode[1];
		} break;

		default: {
			UnimplementedInstruction(state);
			return 1;
		} break;
    }
    return 0;
}