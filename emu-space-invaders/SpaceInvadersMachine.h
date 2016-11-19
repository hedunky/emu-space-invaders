#pragma once

#include "Processor8080.h"
#include "platform.h"

/*
Links:
http://emulator101.com/
http://patpend.net/articles/ar/aev021.txt
http://computerarcheology.com/Arcade/SpaceInvaders/Code.html
*/

class SpaceInvadersMachine {
public:
	SpaceInvadersMachine();
	~SpaceInvadersMachine();

	bool TicksPassed();

	uint8 *Framebuffer();
	uint32 GetScreenWidth();
	uint32 GetScreenHeight();
private:
	Processor8080 processor;
	State8080 *state;

	State8080 *InitState();
	uint16 ReadFileIntoMemory(uint8 *memory, char *filename, uint16 offset);
};
