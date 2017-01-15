#include <stdio.h>
#include <SDL.h>
#include "SpaceInvadersMachine.h"

SDL_Window *window = NULL;
SDL_Surface *screenSurface = NULL;

// TODO: Next up - interrupts!

void initWindow(SpaceInvadersMachine *machine) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
	} else {
		window = SDL_CreateWindow("SDL Tutorial",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			machine->GetScreenWidth(),
			machine->GetScreenHeight(),
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

int rotatedOffset(int originalOffset, SpaceInvadersMachine *machine) {
	int width = machine->GetScreenWidth();
	int height = machine->GetScreenHeight();
	int y1 = originalOffset / height;
	int x1 = originalOffset % height;
	int y2 = height - 1 - x1;
	int x2 = y1;
	int result = y2 * width + x2;
	return result;
}

void mainLoop(SpaceInvadersMachine *machine) {
	bool quit = false;
	SDL_Event event;

	uint32 lastTick = SDL_GetTicks();

	while (!quit) {
		while (SDL_PollEvent(&event) != 0) {
			switch (event.type) {
				case SDL_QUIT: {
					quit = true;
				}	break;

				case SDL_KEYDOWN: {
					switch (event.key.keysym.sym) {
						case SDLK_TAB: {
							machine->KeyPressed(MachineKeyCoin);
						} break;
					}
				} break;
			}
		}
		if (!quit) {
			uint32 currentTick = SDL_GetTicks();
			quit = machine->TicksPassed(currentTick);

			uint32 ticksPassed = currentTick - lastTick;
			if (ticksPassed < 60) {
				continue;
			}
			lastTick = currentTick;

			uint8 *framebuffer = machine->Framebuffer();
			SDL_Surface *surface = SDL_CreateRGBSurface(0, machine->GetScreenWidth(), machine->GetScreenHeight(), 32, 0, 0, 0, 0);

			SDL_LockSurface(surface);
			int totalBytes = (machine->GetScreenWidth() * machine->GetScreenHeight()) / bitsInByte;
			for (int byte = 0; byte < totalBytes; byte++) {
				uint8 pixels = *(framebuffer + byte);
				for (int i = 0; i < bitsInByte; i++) {
					uint8 value = (((pixels & (1 << i)) >> i) == 1) ? 0xFF : 0;
					uint32 result = value | (value << 8) | (value << 16);
					int offset = rotatedOffset((byte * bitsInByte) + i, machine);
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
	SpaceInvadersMachine machine = SpaceInvadersMachine();

	initWindow(&machine);
	mainLoop(&machine);
	closeWindow();

	system("pause");
    return 0;
}
