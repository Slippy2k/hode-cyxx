
#include <SDL.h>
#include "systemstub.h"

static SDL_Surface *screenSurface;
static uint16 screenPalette[256];
static int screenW, screenH;

void sysInit(int w, int h, const char *title) {
	SDL_Init(SDL_INIT_VIDEO);
	SDL_ShowCursor(SDL_DISABLE);
	SDL_WM_SetCaption(title, NULL);
	screenW = w;
	screenH = h;
	screenSurface = SDL_SetVideoMode(screenW, screenH, 16, SDL_SWSURFACE);
	memset(screenPalette, 0, sizeof(screenPalette));
}

void sysDestroy() {
	SDL_FreeSurface(screenSurface);
}

void sysSetPalette(const uint8 *palData, int numColors) {
	assert(numColors <= 256);
	for (int i = 0; i < numColors; ++i) {
		const uint8 r = palData[0] << 2;
		const uint8 g = palData[1] << 2;
		const uint8 b = palData[2] << 2;
		screenPalette[i] = SDL_MapRGB(screenSurface->format, r, g, b);
		palData += 3;
	}
}

void sysCopyRect(const uint8 *src, int pitch, int x, int y, int w, int h) {
//	printf("copyRect %p %d %d %d %d %d\n", src, pitch, x, y, w, h);
	SDL_LockSurface(screenSurface);
	uint16 *dst = (uint16 *)screenSurface->pixels + (screenSurface->pitch >> 1) * y + x;
	for (int j = 0; j < h; ++j) {
		for (int i = 0; i < w; ++i) {
			uint16 color = screenPalette[src[i]];
			dst[i] = color;
/*			dst[2 * i + 1] = color;
			dst[(screenSurface->pitch >> 1) + 2 * i + 0] = color;
			dst[(screenSurface->pitch >> 1) + 2 * i + 1] = color;*/
		}
		src += pitch;
		dst += (screenSurface->pitch >> 1);
	}
	SDL_UnlockSurface(screenSurface);
	SDL_UpdateRect(screenSurface, x, y, w, h);
}

bool sysDelay(int amount) {
	const uint32 timestampEnd = SDL_GetTicks() + amount;
	SDL_Event ev;
	do {
		while (SDL_PollEvent(&ev)) {
			switch (ev.type) {
			case SDL_QUIT:
				return true;
			default:
				break;
			}
		}
		SDL_Delay(20);
	} while (SDL_GetTicks() < timestampEnd);
	return false;
}
