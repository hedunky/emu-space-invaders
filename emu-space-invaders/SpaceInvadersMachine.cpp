#define _CRT_SECURE_NO_DEPRECATE

#include "SpaceInvadersMachine.h"
#include "Processor8080.h"
#include <Windows.h>
#include <stdio.h>

static const uint32 interruptPeriod = 8;

SpaceInvadersMachine::SpaceInvadersMachine() {
	state = InitState();
}

SpaceInvadersMachine::~SpaceInvadersMachine() {
	delete state->memory;
	delete state;
}

bool SpaceInvadersMachine::TicksPassed(uint32 currentTicks) {
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

	uint8 *opcode = &state->memory[state->pc];
	if (*opcode == 0xdb) {
		// Machine specific handling for IN
		printf("2");
	} else if (*opcode == 0xd3) {
		// Machine specific handling for OUT
		processor.printOperation("OUT D8");

		OutPort(opcode[1], state->a);
		state->pc += 2;
	} else {
		bool result = processor.EmulateOperation(state);
		return result;
	}

	return 0;
}

void SpaceInvadersMachine::OutPort(uint8 port, uint8 value)
{
	switch (port) {
		case 3: {
			// TODO: Play sound
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

void SpaceInvadersMachine::KeyPressed(MachineKey) {
	printf("1");
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
