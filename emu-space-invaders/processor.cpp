#include <stdio.h>
#include <Windows.h>
#include "processor.h"

void UnimplementedInstruction(State8080 *state) {
	uint8 *opcode = &state->memory[state->pc];
    printf("Error: Unimplemented instruction %0x at %0x \n", *opcode, state->pc);
}

int parity(int x, int size) {
	int i;
	int p = 0;
	x = (x & ((1 << size) - 1));
	for (i = 0; i < size; i++) {
		if (x & 0x1) p++;
		x = x >> 1;
	}
	return (0 == (p & 0x1));
}

bool isNegative(uint8 x) {
	bool result = ((0x80) == (x & 0x80));
	return result;
}

void printInstruction(char *instruction) {
	//printf("%s\n", instruction);
}

uint8 Emulate8080Operation(State8080 *state) {
    uint8 *opcode = &state->memory[state->pc];
    switch (*opcode) {
		case 0x00: {
			printInstruction("NOP");
			state->pc++;
		} break;

		case 0x01: {
			printInstruction("LXI B, D16");
			state->b = opcode[2];
			state->c = opcode[1];
			state->pc += 3;
		} break;

		case 0x05: {
			printInstruction("DCR B");
			uint8 res = state->b - 1;
			state->flags.z = (res == 0);
			state->flags.s = isNegative(res);
			state->flags.p = parity(res, 8);

			state->b = res;
			state->pc++;
		} break;

		case 0x06: {
			printInstruction("MVI B, D8");
			state->b = opcode[1];
			state->pc += 2;
		} break;

		case 0x11: {
			printInstruction("LXI D, D16");
			state->d = opcode[2];
			state->e = opcode[1];
			state->pc += 3;
		} break;

		case 0x13: {
			printInstruction("INX D");
			state->e++;
			if (state->e == 0) {
				state->d++;
			}
			state->pc++;
		} break;

		case 0x1a: {
			printInstruction("LDAX D");
			uint16 offset = (state->d << 8) | (state->e);
			state->a = state->memory[offset];
			state->pc++;
		} break;

		case 0x21: {
			printInstruction("LXI H, D16");
			state->h = opcode[2];
			state->l = opcode[1];
			state->pc += 3;
		} break;

		case 0x23: {
			printInstruction("INX H");
			state->l++;
			if (state->l == 0) {
				state->h++;
			}
			state->pc++;
		} break;

		case 0x31: {
			printInstruction("LXI SP, D16");
			state->sp = (opcode[2] << 8) | opcode[1];
			state->pc += 3;
		} break;

		case 0x36: {
			printInstruction("MVI M, D8");
			uint16 offset = (state->h << 8) | state->l;
			state->memory[offset] = opcode[1];
			state->pc += 2;
		} break;

		case 0x77: {
			printInstruction("MOV M, A");
			uint16 offset = (state->h << 8) | state->l;
			state->memory[offset] = state->a;
			state->pc++;
		} break;

		case 0x7c: {
			printInstruction("MOV A, H");
			state->a = state->h;
			state->pc++;
		} break;

		case 0xc2: {
			printInstruction("JNZ addr");
			if (state->flags.z == 0) {
				state->pc = (opcode[2] << 8) | opcode[1];
			} else {
				state->pc += 3;
			}
		} break;

		case 0xc3: {
			printInstruction("JMP addr");
			state->pc = (opcode[2] << 8) | opcode[1];
		} break;

		case 0xc9: {
			printInstruction("RET");
			state->pc = state->memory[state->sp] | (state->memory[state->sp + 1] << 8);
			state->sp += 2;
		} break;

		case 0xcd: {
			printInstruction("CALL addr");
			uint16 ret = state->pc + 3;
			state->memory[state->sp - 1] = (ret >> 8) & (0xff);
			state->memory[state->sp - 2] = ret & 0xff;
			state->sp = state->sp - 2;
			state->pc = (opcode[2] << 8) | opcode[1];
		} break;

		case 0xfe: {
			printInstruction("CPI D8");
			uint8 x = state->a - opcode[1];
			state->flags.z = (x == 0);
			state->flags.s = isNegative(x);
			state->flags.p = parity(x, 8);
			state->flags.c = (state->a < opcode[1]);
			state->pc += 2;
		} break;

		default: {
			UnimplementedInstruction(state);
			return 1;
		} break;
    }
    return 0;
}