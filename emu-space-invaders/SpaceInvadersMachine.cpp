#define _CRT_SECURE_NO_DEPRECATE

#include "SpaceInvadersMachine.h"
#include "Processor8080.h"
#include <Windows.h>
#include <stdio.h>

SpaceInvadersMachine::SpaceInvadersMachine() {
	state = InitState();
}

SpaceInvadersMachine::~SpaceInvadersMachine() {
	delete state->memory;
	delete state;
}

bool SpaceInvadersMachine::TicksPassed() {
	bool result = processor.EmulateOperation(state);
	return result;
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
