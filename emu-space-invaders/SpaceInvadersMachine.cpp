#define _CRT_SECURE_NO_DEPRECATE

#include "SpaceInvadersMachine.h"
#include "Processor8080.h"
#include <Windows.h>
#include <stdio.h>

static const uint32 interruptPeriod = 8;

SpaceInvadersMachine::SpaceInvadersMachine() {
	state = InitState();
	inPort1 = 0;
}

SpaceInvadersMachine::~SpaceInvadersMachine() {
	delete state->memory;
	delete state;
}

void SpaceInvadersMachine::TicksPassed(uint32 currentTicks) {
	if (lastTicks == 0) {
		lastTicks = currentTicks;
		nextInterruptTime = currentTicks + (interruptPeriod * 2);
		nextInterruptType = 1;
	}

	bool shouldInterrupt = (currentTicks > nextInterruptTime);
	if (state->interrupt_enable && shouldInterrupt) {
		if (nextInterruptType == 1) {
			processor.EmulateInterrupt(state, nextInterruptType);
			nextInterruptType = 2;
		} else {
			processor.EmulateInterrupt(state, nextInterruptType);
			nextInterruptType = 1;
		}
		nextInterruptTime = currentTicks + interruptPeriod;
	}

	//CPU is 2 MHz => 2 million cycles/sec => 2000 cycles/ms

	double timePassed = currentTicks - lastTicks;
	int cyclesPassed = 2000 * timePassed;
	int cycles = 0;

	while (cycles < cyclesPassed) {
		uint8 *opcode = &state->memory[state->pc];
		if (*opcode == 0xdb) {
			// Machine specific handling for IN
			processor.printOperation("IN D8");

			state->a = InPort(opcode[1]);
			state->pc += 2;
			cycles += 3;
		} else if (*opcode == 0xd3) {
			// Machine specific handling for OUT
			processor.printOperation("OUT D8");

			OutPort(opcode[1], state->a);
			state->pc += 2;
			cycles += 3;
		} else {
			cycles += processor.EmulateOperation(state);
		}
	}

	lastTicks = currentTicks;
}

uint8 SpaceInvadersMachine::InPort(uint8 port) {
	switch (port) {
		case 1: {
			return inPort1;
		} break;

		case 2: {
			return 0;
		} break;

		case 3: {
			uint16 value = (shift1 << 8) | shift0;
			return ((value >> (8 - shiftOffset)) & 0xff);
		} break;

		default: {
			printf("NOT IMPLEMENTED IN %d\n", port);
			return 0;
		} break;
	}
}

void SpaceInvadersMachine::OutPort(uint8 port, uint8 value) {
	switch (port) {
		case 2: {
			shiftOffset = value & 0x7;
		} break;

		case 3: {
			// TODO: Play sound
		} break;

		case 4: {
			shift0 = shift1;
			shift1 = value;
		} break;

		case 5: {
			// TODO: Play sound
		} break;

		case 6: {
			// Watchdog port (probably used for debugging)
		} break;

		default: {
			printf("NOT IMPLEMENTED OUT %d %d\n", port, value);
		} break;
	}
}

void SpaceInvadersMachine::KeyChanged(MachineKey key, bool isPressed) {
	uint8 bit = 0;
	switch (key) {
		case MachineKeyCoin: bit = 0x01; break;
		case MachineKeyP1Fire: bit = 0x10; break;
		case MachineKeyP1Start: bit = 0x04; break;
		case MachineKeyP1Left: bit = 0x20; break;
		case MachineKeyP1Right: bit = 0x40; break;
	}

	if (bit != 0) {
		if (isPressed) {
			inPort1 |= bit;
		} else {
			inPort1 &= ~bit;
		}
	}
}

uint8 *SpaceInvadersMachine::Framebuffer() {
	uint8 *result = state->memory + 0x2400;
	return result;
}

uint32 SpaceInvadersMachine::GetScreenWidth() {
	return 224;
}

uint32 SpaceInvadersMachine::GetScreenHeight() {
	return 256;
}

uint16 SpaceInvadersMachine::ReadFileIntoMemory(uint8 *memory, char *filename, uint16 offset) {
	FILE *f = fopen(filename, "rb");
	if (!f) {
		printf("Couldn't open %s\n", filename);
		exit(1);
	}

	fseek(f, 0, SEEK_END);
	uint16 fileSize = (uint16)ftell(f);
	fseek(f, 0, SEEK_SET);

	uint8 *buffer = memory + sizeof(uint8) * offset;
	fread(buffer, fileSize, 1, f);
	fclose(f);
	return fileSize;
}

State8080 *SpaceInvadersMachine::InitState() {
	State8080 *state = new State8080();
	state->memory = new uint8[0x10000]; // 65536B

	uint16 offset = 0;
	char *files[4] = { "game/invaders.h", "game/invaders.g", "game/invaders.f", "game/invaders.e" };
	for (int i = 0; i < 4; i++) {
		offset += ReadFileIntoMemory(state->memory, files[i], offset);
	}
	return state;
}
