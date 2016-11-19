#pragma once

#include "platform.h"

typedef struct Flags8080 {
	uint8 s : 1; // Sign (set if bit 7 of the math instruction is set)
	uint8 z : 1; // Zero (set if the result is zero)
	uint8 p : 1; // Parity (set if the number of 1 bits in the result is even)
	uint8 c : 1; /* Carry (set if the last addition operation resulted in a carry,
				 or if the last substraction operation required a borrow) */
	uint8 ac : 1; // Auxiliary carry (used for binary-coded decimal arithemtics)

	uint8 padding : 3;
} Flags8080;

typedef struct State8080 {
	// Registers
	uint8 a;
	uint8 b;
	uint8 c;
	uint8 d;
	uint8 e;
	uint8 h;
	uint8 l;

	uint16 sp; // Stack pointer
	uint16 pc; // Program counter

	uint8 *memory;
	struct Flags8080 flags;
	uint8 interrupt_enable;
} State8080;

class Processor8080 {
public:
	bool EmulateOperation(State8080 *state);
private:
	void unimplementedInstruction(State8080 *state);
	int parity(int x, int size);
	void operationMovValueToRegister(uint8 *to, uint8 value, State8080 *state);
	void operationMov(uint8 *to, uint8 *from, State8080 *state);
	void operationMovFromMemory(uint8 *reg, State8080 *state);
	bool isMSBSet(uint8 x);
	void printOperation(char *instruction);
	void LogicFlagsA(State8080 *state);
	uint16 memoryAddress(uint8 *opcode);
};
