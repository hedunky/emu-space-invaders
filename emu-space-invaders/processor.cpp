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

void operationMovValueToRegister(uint8 *to, uint8 value, State8080 *state) {
	*to = value;
	state->pc += 2;
}

void operationMov(uint8 *to, uint8 *from, State8080 *state) {
	*to = *from;
	state->pc++;
}

void operationMovFromMemory(uint8 *reg, State8080 *state) {
	uint16_t offset = (state->h << 8) | (state->l);
	*reg = state->memory[offset];
	state->pc++;
}

bool isMSBSet(uint8 x) {
	bool result = ((0x80) == (x & 0x80));
	return result;
}

void printOperation(char *instruction) {
	//printf("%s\n", instruction);
}

void LogicFlagsA(State8080 *state) {
	state->flags.c = state->flags.ac = 0;
	state->flags.z = (state->a == 0);
	state->flags.s = isMSBSet(state->a);
	state->flags.p = parity(state->a, 8);
}

uint16 memoryAddress(uint8 *opcode) {
	uint16 result = (opcode[2] << 8) | opcode[1];
	return result;
}

bool Emulate8080Operation(State8080 *state) {
    uint8 *opcode = &state->memory[state->pc];
    switch (*opcode) {
		case 0x00: {
			printOperation("NOP");
			state->pc++;
		} break;

		case 0x01: {
			printOperation("LXI B, D16");
			state->b = opcode[2];
			state->c = opcode[1];
			state->pc += 3;
		} break;

		case 0x05: {
			printOperation("DCR B");
			uint8 res = state->b - 1;
			state->flags.z = (res == 0);
			state->flags.s = isMSBSet(res);
			state->flags.p = parity(res, 8);

			state->b = res;
			state->pc++;
		} break;

		case 0x06: {
			printOperation("MVI B, D8");
			operationMovValueToRegister(&state->b, opcode[1], state);
		} break;

		case 0x09: {
			printOperation("DAD B");
			uint32 hl = (state->h << 8) | state->l;
			uint32 bc = (state->b << 8) | state->c;
			uint32 res = hl + bc;
			state->h = (res & 0xff00) >> 8;
			state->l = res & 0xff;
			state->flags.c = ((res & 0xffff0000) != 0);
			state->pc++;
		} break;

		case 0x0d: {
			printOperation("DCR C");
			uint8 res = state->c - 1;
			state->flags.z = (res == 0);
			state->flags.s = isMSBSet(res);
			state->flags.p = parity(res, 8);

			state->c = res;
			state->pc++;
		} break;

		case 0x0e: {
			printOperation("MVI C, D8");
			operationMovValueToRegister(&state->c, opcode[1], state);
		} break;

		case 0x0f: {
			printOperation("RRC");
			uint8 x = state->a;
			bool firstBit = x & 1;
			state->a = (firstBit << 7) | (x >> 1);
			state->flags.c = firstBit;
			state->pc++;
		} break;

		case 0x11: {
			printOperation("LXI D, D16");
			state->d = opcode[2];
			state->e = opcode[1];
			state->pc += 3;
		} break;

		case 0x13: {
			printOperation("INX D");
			state->e++;
			if (state->e == 0) {
				state->d++;
			}
			state->pc++;
		} break;

		case 0x19: {
			printOperation("DAD D");
			uint32 hl = (state->h << 8) | state->l;
			uint32 de = (state->d << 8) | state->e;
			uint32 res = hl + de;
			state->h = (res & 0xff00) >> 8;
			state->l = res & 0xff;
			state->flags.c = ((res & 0xffff0000) != 0);
			state->pc++;
		} break;

		case 0x1a: {
			printOperation("LDAX D");
			uint16 offset = (state->d << 8) | (state->e);
			state->a = state->memory[offset];
			state->pc++;
		} break;

		case 0x21: {
			printOperation("LXI H, D16");
			state->h = opcode[2];
			state->l = opcode[1];
			state->pc += 3;
		} break;

		case 0x23: {
			printOperation("INX H");
			state->l++;
			if (state->l == 0) {
				state->h++;
			}
			state->pc++;
		} break;

		case 0x26: {
			printOperation("MVI H, D8");
			operationMovValueToRegister(&state->h, opcode[1], state);
		} break;

		case 0x29: {
			printOperation("DAD H");
			uint32 hl = (state->h << 8) | state->l;
			uint32 res = hl + hl;
			state->h = (res & 0xff00) >> 8;
			state->l = res & 0xff;
			state->flags.c = ((res & 0xffff0000) != 0);
			state->pc++;
		} break;

		case 0x31: {
			printOperation("LXI SP, D16");
			state->sp = memoryAddress(opcode);
			state->pc += 3;
		} break;

		case 0x32: {
			printOperation("STA addr");
			uint16 offset = memoryAddress(opcode);
			state->memory[offset] = state->a;
			state->pc += 3;
		} break;

		case 0x36: {
			printOperation("MVI M, D8");
			uint16 offset = (state->h << 8) | state->l;
			state->memory[offset] = opcode[1];
			state->pc += 2;
		} break;

		case 0x3a: {
			printOperation("LDA addr");
			uint16 offset = memoryAddress(opcode);
			state->a = state->memory[offset];
			state->pc += 3;
		} break;

		case 0x3e: {
			printOperation("MVI A, D8");
			operationMovValueToRegister(&state->a, opcode[1], state);
		} break;

		case 0x56: {
			printOperation("MOV D, M");
			operationMovFromMemory(&state->d, state);
		} break;

		case 0x5e: {
			printOperation("MOV E, M");
			operationMovFromMemory(&state->e, state);
		} break;

		case 0x66: {
			printOperation("MOV H, M");
			operationMovFromMemory(&state->h, state);
		} break;

		case 0x6f: {
			printOperation("MOV L, A");
			operationMov(&state->l, &state->a, state);
		} break;

		case 0x77: {
			printOperation("MOV M, A");
			uint16 offset = (state->h << 8) | state->l;
			state->memory[offset] = state->a;
			state->pc++;
		} break;

		case 0x7a: {
			printOperation("MOV A, D");
			operationMov(&state->a, &state->d, state);
		} break;

		case 0x7b: {
			printOperation("MOV A, E");
			operationMov(&state->a, &state->e, state);
		} break;

		case 0x7c: {
			printOperation("MOV A, H");
			operationMov(&state->a, &state->h, state);
		} break;

		case 0x7e: {
			printOperation("MOV A, M");
			operationMovFromMemory(&state->a, state);
		} break;

		case 0xa7: {
			printOperation("ANA A");
			state->a &= state->a;
			LogicFlagsA(state);
			state->pc++;
		} break;

		case 0xaf: {
			printOperation("XRA A");
			state->a ^= state->a;
			LogicFlagsA(state);
			state->pc++;
		} break;

		case 0xc1: {
			printOperation("POP B");
			state->c = state->memory[state->sp];
			state->b = state->memory[state->sp + 1];
			state->sp += 2;
			state->pc++;
		} break;

		case 0xc2: {
			printOperation("JNZ addr");
			if (state->flags.z == 0) {
				state->pc = memoryAddress(opcode);
			} else {
				state->pc += 3;
			}
		} break;

		case 0xc3: {
			printOperation("JMP addr");
			state->pc = memoryAddress(opcode);
		} break;

		case 0xc5: {
			printOperation("PUSH B");
			state->memory[state->sp - 1] = state->b;
			state->memory[state->sp - 2] = state->c;
			state->sp -= 2;
			state->pc++;
		} break;

		case 0xc6: {
			printOperation("ADI D8");
			uint16 result = (uint16)state->a + (uint16)opcode[1];
			uint8 result8Bit = result & 0xff;
			state->flags.z = (result8Bit == 0);
			state->flags.c = isMSBSet(result8Bit);
			state->flags.p = parity(result8Bit, 8);
			state->flags.c = result > 0xff;
			state->a = result8Bit;
			state->pc += 2;
		} break;

		case 0xc9: {
			printOperation("RET");
			state->pc = state->memory[state->sp] | (state->memory[state->sp + 1] << 8);
			state->sp += 2;
		} break;

		case 0xcd: {
			printOperation("CALL addr");
			uint16 ret = state->pc + 3;
			state->memory[state->sp - 1] = (ret >> 8) & (0xff);
			state->memory[state->sp - 2] = ret & 0xff;
			state->sp = state->sp - 2;
			state->pc = memoryAddress(opcode);
		} break;

		case 0xd1: {
			printOperation("POP D");
			state->e = state->memory[state->sp];
			state->d = state->memory[state->sp + 1];
			state->sp += 2;
			state->pc++;
		} break;

		case 0xd3: {
			printOperation("OUT D8");
			// TODO: OUT     (WATCHDOG),A        ; Feed watchdog
			state->pc += 2;
		} break;

		case 0xd5: {
			printOperation("PUSH D");
			state->memory[state->sp - 1] = state->d;
			state->memory[state->sp - 2] = state->e;
			state->sp -= 2;
			state->pc++;
		} break;

		case 0xe1: {
			printOperation("POP H");
			state->l = state->memory[state->sp];
			state->h = state->memory[state->sp + 1];
			state->sp += 2;
			state->pc++;
		} break;

		case 0xe5: {
			printOperation("PUSH H");
			state->memory[state->sp - 1] = state->h;
			state->memory[state->sp - 2] = state->l;
			state->sp -= 2;
			state->pc++;
		} break;

		case 0xe6: {
			printOperation("ANI D8");
			state->a &= opcode[1];
			LogicFlagsA(state);
			state->pc += 2;
		} break;

		case 0xeb: {
			printOperation("XCHG");
			uint8 origD = state->d;
			uint8 origE = state->e;
			state->d = state->h;
			state->e = state->l;
			state->h = origD;
			state->l = origE;
			state->pc++;
		} break;

		case 0xf1: {
			printOperation("POP PSW");
			state->a = state->memory[state->sp + 1];
			uint8 psw = state->memory[state->sp];

			state->flags.z = (0x01 == (psw & 0x01));
			state->flags.s = (0x02 == (psw & 0x02));
			state->flags.p = (0x04 == (psw & 0x04));
			state->flags.c = (0x08 == (psw & 0x08));
			state->flags.ac = (0x10 == (psw & 0x10));
			state->sp += 2;

			state->pc++;
		} break;

		case 0xf5: {
			printOperation("PUSH PSW");
			state->memory[state->sp - 1] = state->a;
			uint8 psw = (state->flags.z |
				state->flags.s << 1 |
				state->flags.p << 2 |
				state->flags.c << 3 |
				state->flags.ac << 4);
			state->memory[state->sp - 2] = psw;
			state->sp = state->sp - 2;
			state->pc++;
		} break;

		case 0xfb: {
			printOperation("EI");
			state->interrupt_enable = 1;
			state->pc++;
		} break;

		case 0xfe: {
			printOperation("CPI D8");
			uint8 x = state->a - opcode[1];
			state->flags.z = (x == 0);
			state->flags.s = isMSBSet(x);
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