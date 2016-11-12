#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include "processor.h"
#include <Windows.h>
#include <SDL.h>
#include <stdio.h>

//Screen dimension constants
const int SCREEN_WIDTH = 256;
const int SCREEN_HEIGHT = 224;

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

void setPixel(SDL_Surface *surface, int x, int y, uint32 pixel) {
	uint32 *target_pixel = (uint32 *)((uint8 *)surface->pixels + y * surface->pitch +
		x * sizeof *target_pixel);
	*target_pixel = pixel;
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
			for (int height = 0; height < SCREEN_HEIGHT; height++) {
				for (int width = 0; width < SCREEN_WIDTH; width+= sizeof(uint8)) {
					uint8 pixel = *(framebuffer + sizeof(uint8) * width + sizeof(uint8) * height);
					for (int i = 0; i < 8; i++) {
						uint8 value = (((pixel & (1 << i)) >> i) == 1) ? 0xFF : 0;
						uint32 result = value | (value << 8) | (value << 16);
						setPixel(surface, width, height, result);
					}
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
