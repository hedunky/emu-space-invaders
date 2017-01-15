#include "Processor8080.h"
#include <stdio.h>

void Processor8080::unimplementedInstruction(State8080 *state) {
	uint8 *opcode = &state->memory[state->pc];
    printf("Error: Unimplemented instruction %0x at %0x \n", *opcode, state->pc);
}

int Processor8080::parity(int x, int size) {
	int i;
	int p = 0;
	x = (x & ((1 << size) - 1));
	for (i = 0; i < size; i++) {
		if (x & 0x1) p++;
		x = x >> 1;
	}
	return (0 == (p & 0x1));
}

void Processor8080::operationMovValueToRegister(uint8 *to, uint8 value, State8080 *state) {
	*to = value;
	state->pc += 2;
}

void Processor8080::operationMov(uint8 *to, uint8 *from, State8080 *state) {
	*to = *from;
	state->pc++;
}

void Processor8080::operationMovFromMemory(uint8 *reg, State8080 *state) {
	uint8 value = ReadFromHL(state);
	*reg = value;
	state->pc++;
}

void Processor8080::operationPush(uint8 high, uint8 low, State8080 *state) {
	state->memory[state->sp - 1] = high;
	state->memory[state->sp - 2] = low;
	state->sp -= 2;
	state->pc++;
}

void Processor8080::operationReturn(State8080 *state) {
	state->pc = state->memory[state->sp] | (state->memory[state->sp + 1] << 8);
	state->sp += 2;
}

void Processor8080::operationINR(State8080 *state, uint8 *reg) {
	*reg += 1;
	LogicFlagsZSP(state, *reg);
	state->pc++;
}

void Processor8080::operationDCR(State8080 *state, uint8 *reg) {
	uint8 result = *reg - 1;
	LogicFlagsZSP(state, result);

	*reg = result;
	state->pc++;
}

void Processor8080::operationANA(State8080 *state, uint8 value) {
	state->a &= value;
	LogicFlagsA(state);
	state->pc++;
}

void Processor8080::operationCMP(State8080 *state, uint8 value) {
	uint16 result = (uint16)state->a - (uint16)value;
	ArithmeticFlagsA(state, result);
	state->pc++;
}

void Processor8080::operationADD(State8080 *state, uint8 value) {
	uint16 result = (uint16)state->a + (uint16)value;
	ArithmeticFlagsA(state, result);
	state->a = (result & 0xff);
	state->pc++;
}

void Processor8080::operationADC(State8080 *state, uint8 value) {
	uint16 result = (uint16)state->a + (uint16)value + state->flags.c;
	ArithmeticFlagsA(state, result);
	state->a = (result & 0xff);
	state->pc++;
}

void Processor8080::operationCall(State8080 *state) {
	uint8 *opcode = &state->memory[state->pc];
	uint16 ret = state->pc + 3;

	state->memory[state->sp - 1] = highMemoryAddress(ret);
	state->memory[state->sp - 2] = lowMemoryAddress(ret);

	state->sp -= 2;
	state->pc = memoryAddress(opcode);
}

bool Processor8080::isMSBSet(uint8 x) {
	bool result = ((0x80) == (x & 0x80));
	return result;
}

void Processor8080::printOperation(char *instruction) {
	//printf("%s\n", instruction);
}

void Processor8080::LogicFlagsA(State8080 *state) {
	state->flags.c = state->flags.ac = 0;
	state->flags.z = (state->a == 0);
	state->flags.s = isMSBSet(state->a);
	state->flags.p = parity(state->a, 8);
}

void Processor8080::ArithmeticFlagsA(State8080 *state, uint16 result) {
	state->flags.c = (result > 0xff);
	state->flags.z = ((result & 0xff) == 0);
	state->flags.s = isMSBSet((uint8)result);
	state->flags.p = parity(result & 0xff, 8);
}

void Processor8080::LogicFlagsZSP(State8080 *state, uint8 value) {
	state->flags.z = (value == 0);
	state->flags.s = isMSBSet(value);
	state->flags.p = parity(value, 8);
}

uint8 Processor8080::ReadFromHL(State8080* state) {
	uint16 offset = (state->h << 8) | state->l;
	uint8 result = state->memory[offset];
	return result;
}

void Processor8080::WriteToHL(State8080* state, uint8 value) {
	uint16 offset = (state->h << 8) | state->l;
	state->memory[offset] = value;
}

uint16 Processor8080::memoryAddress(uint8 *opcode) {
	uint16 result = (opcode[2] << 8) | opcode[1];
	return result;
}

uint8 Processor8080::highMemoryAddress(uint16 fullAddress) {
	uint8 result = (fullAddress & 0xFF00) >> 8;
	return result;
}

uint8 Processor8080::lowMemoryAddress(uint16 fullAddress) {
	uint8 result = (fullAddress & 0xff);
	return result;
}

bool Processor8080::EmulateOperation(State8080 *state) {
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

		case 0x03: {
			printOperation("INX B");
			state->c++;
			if (state->c == 0) {
				state->b++;
			}
			state->pc++;
		} break;

		case 0x04: {
			printOperation("INR B");
			operationINR(state, &state->b);
		} break;

		case 0x05: {
			printOperation("DCR B");
			operationDCR(state, &state->b);
		} break;

		case 0x06: {
			printOperation("MVI B, D8");
			operationMovValueToRegister(&state->b, opcode[1], state);
		} break;

		case 0x07: {
			printOperation("RLC");
			uint8 value = state->a;
			state->a = ((value & 0x80) >> 7) | (value << 1);
			state->flags.c = isMSBSet(value);

			state->pc++;
		} break;

		case 0x09: {
			printOperation("DAD B");
			uint32 hl = (state->h << 8) | state->l;
			uint32 bc = (state->b << 8) | state->c;
			uint32 res = hl + bc;
			state->h = highMemoryAddress(res);
			state->l = lowMemoryAddress(res);
			state->flags.c = ((res & 0xffff0000) != 0);
			state->pc++;
		} break;

		case 0x0a: {
			printOperation("LDAX B");
			uint16 offset = (state->b << 8) | (state->c);
			state->a = state->memory[offset];
			state->pc++;
		} break;

		case 0x0c: {
			printOperation("INR C");
			operationINR(state, &state->c);
		} break;

		case 0x0d: {
			printOperation("DCR C");
			operationDCR(state, &state->c);
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

		case 0x12: {
			printOperation("STAX D");
			uint16 offset = (state->d << 8) | state->e;
			state->memory[offset] = state->a;
			state->pc++;
		} break;

		case 0x13: {
			printOperation("INX D");
			state->e++;
			if (state->e == 0) {
				state->d++;
			}
			state->pc++;
		} break;

		case 0x14: {
			printOperation("INR D");
			operationINR(state, &state->d);
		} break;

		case 0x15: {
			printOperation("DCR D");
			operationDCR(state, &state->d);
		} break;

		case 0x16: {
			printOperation("MVI D, B8");
			operationMovValueToRegister(&state->d, opcode[1], state);
		} break;

		case 0x19: {
			printOperation("DAD D");
			uint32 hl = (state->h << 8) | state->l;
			uint32 de = (state->d << 8) | state->e;
			uint32 res = hl + de;
			state->h = highMemoryAddress(res);
			state->l = lowMemoryAddress(res);
			state->flags.c = ((res & 0xffff0000) != 0);
			state->pc++;
		} break;

		case 0x1a: {
			printOperation("LDAX D");
			uint16 offset = (state->d << 8) | (state->e);
			state->a = state->memory[offset];
			state->pc++;
		} break;

		case 0x1b: {
			printOperation("DCX D");
			state->e -= 1;
			if (state->e == 0xff) {
				state->d -= 1;
			}
			state->pc++;
		} break;

		case 0x1f: {
			printOperation("RRA");

			uint8 value = state->a;
			state->a = (state->flags.c << 7) | (value >> 1);
			state->flags.c = (1 == (value & 1));

			state->pc++;
		} break;

		case 0x21: {
			printOperation("LXI H, D16");
			state->h = opcode[2];
			state->l = opcode[1];
			state->pc += 3;
		} break;

		case 0x22: {
			printOperation("SHLD");
			uint16 offset = memoryAddress(opcode);

			state->memory[offset] = state->l;
			state->memory[offset + 1] = state->h;
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

		case 0x24: {
			printOperation("INR H");
			operationINR(state, &state->h);
		} break;
				   
		case 0x26: {
			printOperation("MVI H, D8");
			operationMovValueToRegister(&state->h, opcode[1], state);
		} break;

		case 0x27: {
			printOperation("DAA");
			if ((state->a & 0xf) > 9) {
				state->a += 6;
			}
			if ((state->a & 0xf0) > 0x90) {
				uint16 result = (uint16)state->a + 0x60;
				state->a = result & 0xff;
				ArithmeticFlagsA(state, result);
			}
			state->pc++;
		} break;

		case 0x29: {
			printOperation("DAD H");
			uint32 hl = (state->h << 8) | state->l;
			uint32 res = hl + hl;
			state->h = highMemoryAddress(res);
			state->l = lowMemoryAddress(res);
			state->flags.c = ((res & 0xffff0000) != 0);
			state->pc++;
		} break;

		case 0x2a: {
			printOperation("LHLD");

			uint16 offset = memoryAddress(opcode);
			state->l = state->memory[offset];
			state->h = state->memory[offset + 1];
			state->pc += 3;
		} break;

		case 0x2b: {
			printOperation("DCX H");
			state->l -= 1;
			if (state->l == 0xff) {
				state->h -= 1;
			}
			state->pc++;
		} break;

		case 0x2c: {
			printOperation("INR L");
			operationINR(state, &state->l);
		} break;

		case 0x2e: {
			printOperation("MVI L, D8");
			operationMovValueToRegister(&state->l, opcode[1], state);
		} break;

		case 0x2f: {
			printOperation("CMA");
			state->a = ~state->a;
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

		case 0x34: {
			printOperation("INR M");
			uint8 result = ReadFromHL(state) + 1;
			LogicFlagsZSP(state, result);
			WriteToHL(state, result);
			state->pc++;
		} break;

		case 0x35: {
			printOperation("DCR M");
			uint8 value = ReadFromHL(state) - 1;
			LogicFlagsZSP(state, value);

			WriteToHL(state, value);
			state->pc++;
		} break;

		case 0x36: {
			printOperation("MVI M, D8");
			WriteToHL(state, opcode[1]);
			state->pc += 2;
		} break;

		case 0x37: {
			printOperation("SCF");
			state->flags.c = 1;
			state->pc++;
		} break;

		case 0x3a: {
			printOperation("LDA addr");
			uint16 offset = memoryAddress(opcode);
			state->a = state->memory[offset];
			state->pc += 3;
		} break;

		case 0x3c: {
			printOperation("INR A");
			operationINR(state, &state->a);
		} break;

		case 0x3d: {
			printOperation("DCR A");
			operationDCR(state, &state->a);
		} break;

		case 0x3e: {
			printOperation("MVI A, D8");
			operationMovValueToRegister(&state->a, opcode[1], state);
		} break;

		case 0x41: {
			printOperation("MOV B, C");
			operationMov(&state->b, &state->c, state);
		} break;

		case 0x46: {
			printOperation("MOV B, M");
			operationMovFromMemory(&state->b, state);
		} break;

		case 0x47: {
			printOperation("MOV B, A");
			operationMov(&state->b, &state->a, state);
		} break;

		case 0x48: {
			printOperation("MOV C, B");
			operationMov(&state->c, &state->b, state);
		} break;

		case 0x4e: {
			printOperation("MOV C, M");
			operationMovFromMemory(&state->c, state);
		} break;

		case 0x4f: {
			printOperation("MOV C, A");
			operationMov(&state->c, &state->a, state);
		} break;

		case 0x56: {
			printOperation("MOV D, M");
			operationMovFromMemory(&state->d, state);
		} break;

		case 0x57: {
			printOperation("MOV D, A");
			operationMov(&state->d, &state->a, state);
		} break;

		case 0x5e: {
			printOperation("MOV E, M");
			operationMovFromMemory(&state->e, state);
		} break;

		case 0x5f: {
			printOperation("MOV E, A");
			operationMov(&state->e, &state->a, state);
		} break;

		case 0x61: {
			printOperation("MOV H, C");
			operationMov(&state->h, &state->c, state);
		} break;

		case 0x65: {
			printOperation("MOV H, L");
			operationMov(&state->h, &state->l, state);
		} break;

		case 0x66: {
			printOperation("MOV H, M");
			operationMovFromMemory(&state->h, state);
		} break;

		case 0x67: {
			printOperation("MOV H, A");
			operationMov(&state->h, &state->a, state);
		} break;

		case 0x68: {
			printOperation("MOV L, B");
			operationMov(&state->l, &state->b, state);
		} break;

		case 0x69: {
			printOperation("MOV L, C");
			operationMov(&state->l, &state->c, state);
		} break;

		case 0x6f: {
			printOperation("MOV L, A");
			operationMov(&state->l, &state->a, state);
		} break;

		case 0x70: {
			printOperation("MOV M, B");
			WriteToHL(state, state->b);
			state->pc++;
		} break;

		case 0x71: {
			printOperation("MOV M, C");
			WriteToHL(state, state->c);
			state->pc++;
		} break;

		case 0x77: {
			printOperation("MOV M, A");
			WriteToHL(state, state->a);
			state->pc++;
		} break;

		case 0x79: {
			printOperation("MOV A, C");
			operationMov(&state->a, &state->c, state);
		} break;

		case 0x78: {
			printOperation("MOV A, B");
			operationMov(&state->a, &state->b, state);
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

		case 0x7d: {
			printOperation("MOV A, L");
			operationMov(&state->a, &state->l, state);
		} break;

		case 0x7e: {
			printOperation("MOV A, M");
			operationMovFromMemory(&state->a, state);
		} break;

		case 0x80: {
			printOperation("ADD B");
			operationADD(state, state->b);
		} break;

		case 0x81: {
			printOperation("ADD C");
			operationADD(state, state->c);
		} break;

		case 0x83: {
			printOperation("ADD E");
			operationADD(state, state->e);
		} break;

		case 0x85: {
			printOperation("ADD L");
			operationADD(state, state->l);
		} break;

		case 0x86: {
			printOperation("ADD M");
			operationADD(state, ReadFromHL(state));
		} break;

		case 0x8a: {
			printOperation("ADC D");
			operationADC(state, state->d);
		} break;

		case 0x97: {
			printOperation("SUB A");

			uint16 result = (uint16)state->a - (uint16)state->a; 
			ArithmeticFlagsA(state, result); 
			state->a = (result & 0xff);
			state->pc++;
		} break;

		case 0xa0: {
			printOperation("ANA B");
			operationANA(state, state->b);
		} break;

		case 0xa6: {
			printOperation("ANA M");
			operationANA(state, ReadFromHL(state)); 
		} break;

		case 0xa7: {
			printOperation("ANA A");
			operationANA(state, state->a);
		} break;

		case 0xa8: {
			printOperation("XRA B");
			state->a ^= state->b;
			LogicFlagsA(state);
			state->pc++;
		} break;

		case 0xaf: {
			printOperation("XRA A");
			state->a ^= state->a;
			LogicFlagsA(state);
			state->pc++;
		} break;

		case 0xb0: {
			printOperation("ORA B");

			state->a = state->a | state->b;
			LogicFlagsA(state);

			state->pc++;
		} break;

		case 0xb4: {
			printOperation("ORA H");

			state->a = state->a | state->h;
			LogicFlagsA(state);

			state->pc++;
		} break;

		case 0xb6: {
			printOperation("ORA M");

			uint8 value = ReadFromHL(state);
			state->a = state->a | value;
			LogicFlagsA(state);

			state->pc++;
		} break;

		case 0xb8: {
			printOperation("CMP B");
			operationCMP(state, state->b);
		} break;

		case 0xbc: {
			printOperation("CMP H");
			operationCMP(state, state->h);
		} break;

		case 0xbe: {
			printOperation("CMP M");
			operationCMP(state, ReadFromHL(state));
		} break;

		case 0xc0: {
			printOperation("RET NZ");
			if (state->flags.z == 0) {
				operationReturn(state);
			} else {
				state->pc++;
			}
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

		case 0xc4: {
			printOperation("CNZ addr");
			if (state->flags.z == 0) {
				operationCall(state);
			} else {
				state->pc += 3;
			}
		} break;

		case 0xc5: {
			printOperation("PUSH B");
			operationPush(state->b, state->c, state);
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

		case 0xc8: {
			printOperation("RET Z");
			if (state->flags.z) {
				operationReturn(state);
			} else {
				state->pc++;
			}
		} break;

		case 0xc9: {
			printOperation("RET");
			operationReturn(state);
		} break;

		case 0xca: {
			printOperation("JZ addr");
			if (state->flags.z) {
				state->pc = memoryAddress(opcode);
			} else {
				state->pc += 3;
			}
		} break;

		case 0xcc: {
			printOperation("CZ addr");
			if (state->flags.z == 1) {
				operationCall(state);
			} else {
				state->pc += 3;
			}
		} break;

		case 0xcd: {
			printOperation("CALL addr");
			operationCall(state);
		} break;

		case 0xd0: {
			printOperation("RET NC");
			if (state->flags.c == 0) {
				operationReturn(state);
			} else {
				state->pc++;
			}
		} break;

		case 0xd1: {
			printOperation("POP D");
			state->e = state->memory[state->sp];
			state->d = state->memory[state->sp + 1];
			state->sp += 2;
			state->pc++;
		} break;

		case 0xd2: {
			printOperation("JNC");
			if (state->flags.c == 0) {
				state->pc = memoryAddress(opcode);
			} else {
				state->pc += 3;
			}
		} break;

		case 0xd4: {
			printOperation("CNC addr");
			if (state->flags.c == 0) {
				operationCall(state);
			} else {
				state->pc += 3;
			}
		} break;

		case 0xd5: {
			printOperation("PUSH D");
			operationPush(state->d, state->e, state);
		} break;

		case 0xd6: {
			printOperation("SUB D8");

			uint8 value = state->a - opcode[1];
			LogicFlagsZSP(state, value & 0xff);
			state->flags.c = (state->a < opcode[1]);
			state->a = value;
			state->pc += 2;
		} break;

		case 0xd8: {
			printOperation("RET C");
			if (state->flags.c != 0) {
				operationReturn(state);
			} else {
				state->pc++;
			}
		} break;

		case 0xda: {
			printOperation("JP C");
			if (state->flags.c != 0) {
				state->pc = memoryAddress(opcode);
			} else {
				state->pc += 3;
			}
		} break;

		case 0xde: {
			printOperation("SBI");
			uint16 value = state->a - opcode[1] - state->flags.c;
			LogicFlagsZSP(state, value & 0xff);
			state->flags.c = (value > 0xff);
			state->a = value & 0xff;
			state->pc += 2;
		} break;

		case 0xe1: {
			printOperation("POP H");
			state->l = state->memory[state->sp];
			state->h = state->memory[state->sp + 1];
			state->sp += 2;
			state->pc++;
		} break;

		case 0xe3: {
			printOperation("XTHL");
			uint8 h = state->h;
			uint8 l = state->l;
			state->l = state->memory[state->sp];
			state->h = state->memory[state->sp + 1];
			state->memory[state->sp] = l;
			state->memory[state->sp + 1] = h;
			state->pc++;
		} break;

		case 0xe5: {
			printOperation("PUSH H");
			operationPush(state->h, state->l, state);
		} break;

		case 0xe6: {
			printOperation("ANI D8");
			state->a &= opcode[1];
			LogicFlagsA(state);
			state->pc += 2;
		} break;

		case 0xe9: {
			printOperation("PCHL");
			state->pc = (state->h << 8) | state->l;
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

		case 0xf6: {
			printOperation("ORI D8");

			uint8 value = state->a | opcode[1];
			LogicFlagsZSP(state, value);
			state->flags.c = 0;
			state->a = value;
			state->pc += 2;
		} break;

		case 0xfa: {
			printOperation("JM");
			if (state->flags.s != 0) {
				state->pc = memoryAddress(opcode);
			} else {
				state->pc += 3;
			}
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
			unimplementedInstruction(state);
			return 1;
		} break;
    }

    return 0;
}

void Processor8080::EmulateInterrupt(State8080 *state, int interruptType) {
	uint8 high = highMemoryAddress(state->pc);
	uint8 low = lowMemoryAddress(state->pc);
	operationPush(high, low, state);

	//Set the PC to the low memory vector
	state->pc = bitsInByte * interruptType;

	printOperation("DI");
	state->interrupt_enable = 0;
}
