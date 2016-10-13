#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include "processor.h"
#include <Windows.h>

/*
Links: 
http://emulator101.com/
http://patpend.net/articles/ar/aev021.txt
http://computerarcheology.com/Arcade/SpaceInvaders/Code.html
*/

uint16 ReadFileIntoMemory(uint8 *memory, char *filename, uint16 offset) {
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

int main() {
    State8080 state = {};
    state.memory = (uint8 *)malloc(0x2000); // 8192 B

    uint16 offset = 0;
    char *files[4] = {"game/invaders.h", "game/invaders.g", "game/invaders.f", "game/invaders.e"};
    for (int i = 0; i < 4; i++) {
        offset += ReadFileIntoMemory(state.memory, files[i], offset);
    }

    int done = 0;
    while (!done) {
        done = Emulate8080Operation(&state);
    }
	system("pause");
    return 0;
}
