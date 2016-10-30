#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include "processor.h"
#include <Windows.h>
#include <SDL.h>
#include <stdio.h>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

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

int main(int argc, char *args[]) {
	SDL_Window *window = NULL;
	SDL_Surface *screenSurface = NULL;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
	} else {
		window = SDL_CreateWindow("SDL Tutorial",
			SDL_WINDOWPOS_UNDEFINED, 
			SDL_WINDOWPOS_UNDEFINED,
			SCREEN_WIDTH, 
			SCREEN_HEIGHT, 
			SDL_WINDOW_SHOWN);
		if (window == NULL) {
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		} else {
			screenSurface = SDL_GetWindowSurface(window);
			SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
			SDL_UpdateWindowSurface(window);
			SDL_Delay(2000);
			SDL_DestroyWindow(window);
			SDL_Quit();
		}
	}

    State8080 state = {};
    state.memory = (uint8 *)malloc(0x10000); // 65KB

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
