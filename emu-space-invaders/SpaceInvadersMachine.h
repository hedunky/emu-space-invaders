#pragma once

#include "Processor8080.h"
#include "platform.h"

/*
Links:
http://emulator101.com/
http://patpend.net/articles/ar/aev021.txt
http://computerarcheology.com/Arcade/SpaceInvaders/Code.html
*/

enum MachineKey{
	MachineKeyCoin,
	MachineKeyP1Left,
	MachineKeyP1Right,
	MachineKeyP1Fire,
	MachineKeyP1Start,
};

class SpaceInvadersMachine {
public:
	SpaceInvadersMachine();
	~SpaceInvadersMachine();

	bool TicksPassed(uint32 currentTicks);
	void KeyChanged(MachineKey key, bool isPressed);

	uint8 *Framebuffer();
	uint32 GetScreenWidth();
	uint32 GetScreenHeight();
private:
	Processor8080 processor;
	State8080 *state;

	State8080 *InitState();
	uint16 ReadFileIntoMemory(uint8 *memory, char *filename, uint16 offset);

	void OutPort(uint8 port, uint8 value);
	uint8 InPort(uint8 port);

	uint8 inPort1;
	uint8 shiftOffset, shift0, shift1;

	uint32 lastTicks;
	uint32 nextInterruptTime;
	uint32 nextInterruptType;
};
