#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include "processor.h"
#include <Windows.h>
#include <SDL.h>
#include <stdio.h>

//Screen dimension constants
const int SCREEN_WIDTH = 224;
const int SCREEN_HEIGHT = 256;

SDL_Window *window = NULL;
SDL_Surface *screenSurface = NULL;

/*
Links: 
http://emulator101.com/
http://patpend.net/articles/ar/aev021.txt
http://computerarcheology.com/Arcade/SpaceInvaders/Code.html
*/

uint16 readFileIntoMemory(uint8 *memory, char *filename, uint16 offset) {
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

State8080 * initState() {
	State8080 *state = (State8080 *)calloc(1, sizeof(State8080));
	state->memory = (uint8 *)malloc(0x10000); // 65KB

	uint16 offset = 0;
	char *files[4] = { "game/invaders.h", "game/invaders.g", "game/invaders.f", "game/invaders.e" };
	for (int i = 0; i < 4; i++) {
		offset += readFileIntoMemory(state->memory, files[i], offset);
	}
	return state;
}

void initWindow() {
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
		}
	}
}

void setPixel(SDL_Surface *surface, int offset, uint32 pixel) {
	uint32 *target_pixel = (uint32 *)((uint32 *)surface->pixels + offset);
	*target_pixel = pixel;
}

int rotatedOffset(int originalOffset) {
	int width = SCREEN_WIDTH;
	int height = SCREEN_HEIGHT;
	int y1 = originalOffset / height;
	int x1 = originalOffset % height;
	int y2 = height - 1 - x1;
	int x2 = y1;
	int result = y2 * width + x2;
	return result;
}

void mainLoop() {
	bool quit = false;
	SDL_Event e;

	State8080 *state = initState();
	uint32 lastTick = SDL_GetTicks();

	while (!quit) {
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				quit = true;
				break;
			}
		}
		if (!quit) {
			quit = Emulate8080Operation(state);

			uint32 currentTick = SDL_GetTicks();
			if (currentTick - lastTick < 1000) {
				continue;
			}
			lastTick = currentTick;

			uint8 *framebuffer = state->memory + 0x2400;
			SDL_Surface *surface = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, 0, 0, 0);

			SDL_LockSurface(surface);
			int totalBytes = (SCREEN_WIDTH * SCREEN_HEIGHT) / bitsInByte;
			for (int byte = 0; byte < totalBytes; byte++) {
				uint8 pixels = *(framebuffer + byte);
				for (int i = 0; i < bitsInByte; i++) {
					uint8 value = (((pixels & (1 << i)) >> i) == 1) ? 0xFF : 0;
					uint32 result = value | (value << 8) | (value << 16);
					int offset = rotatedOffset((byte * bitsInByte) + i);
					setPixel(surface, offset, result);
				}
			}
			SDL_UnlockSurface(surface);

			SDL_BlitSurface(surface, NULL, screenSurface, NULL);
			SDL_UpdateWindowSurface(window);
			SDL_FreeSurface(surface);
		}
	}
}

void closeWindow() {
	SDL_DestroyWindow(window);
	window = NULL;

	SDL_Quit();
}

int main(int argc, char *args[]) {
	initWindow();
	mainLoop();
	closeWindow();

	system("pause");
    return 0;
}
