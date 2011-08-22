/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009 Gregory Montoir
 */

#include <SDL.h>
#include <stdarg.h>
#include "scaler.h"
#include "systemstub.h"
#include "util.h"

struct Scaler {
	const char *name;
	ScaleProc scaleProc;
	ClearProc clearProc;
	int multiplier;
};

static Scaler _scalers[] = {
	{ "point1x", &point1x, &clear1x, 1 },
	{ "point2x", &point2x, &clear2x, 2 },
	{ "scale2x", &scale2x, &clear2x, 2 },
	{ "point3x", &point3x, &clear3x, 3 },
	{ "scale3x", &scale3x, &clear3x, 3 }
};

static int _scalersCount = ARRAYSIZE(_scalers);

struct KeyMapping {
	int keyCode;
	int mask;
};

struct SystemStub_SDL : SystemStub {
	enum {
		kCopyRectsSize = 200,
		kKeyMappingsSize = 20,
		kDefaultScaler = 1
	};

	uint8_t *_offscreenBase;
	uint8_t *_offscreen;
	SDL_Surface *_screen;
	uint16_t _pal[256];
	int _screenW, _screenH;
	int _shakeDx, _shakeDy;
	KeyMapping _keyMappings[kKeyMappingsSize];
	int _keyMappingsCount;
	int _scaler;

	virtual ~SystemStub_SDL() {}
	virtual void init(const char *title, int w, int h);
	virtual void destroy();
	virtual void setPalette(const uint8_t *pal, int n, int depth);
	virtual void copyRect(int x, int y, int w, int h, const uint8_t *buf, int pitch);
	virtual void fillRect(int x, int y, int w, int h, uint8_t color);
	virtual void shakeScreen(int dx, int dy);
	virtual void updateScreen();
	virtual void processEvents();
	virtual void sleep(int duration);
	virtual uint32_t getTimeStamp();

	void addKeyMapping(int key, uint8_t mask);
	void setupDefaultKeyMappings();
	void updateKeys(PlayerInput *inp);
	void prepareScaledGfx(int scaler);
	void switchScaledGfx(int scaler);
};

SystemStub *SystemStub_SDL_create() {
	return new SystemStub_SDL();
}

void SystemStub_SDL::init(const char *title, int w, int h) {
	SDL_Init(SDL_INIT_VIDEO);
	SDL_ShowCursor(SDL_DISABLE);
	SDL_WM_SetCaption(title, NULL);
	setupDefaultKeyMappings();
	memset(&inp, 0, sizeof(inp));
	_screenW = w;
	_screenH = h;
	_shakeDx = _shakeDy = 0;
	memset(_pal, 0, sizeof(_pal));
	const int offscreenSize = (w + 2) * (h + 2); // extra bytes for scalers accessing border pixels
	_offscreenBase = (uint8_t *)malloc(offscreenSize);
	if (!_offscreenBase) {
		error("SystemStub_SDL::init() Unable to allocate offscreen buffer");
	}
	memset(_offscreenBase, 0, offscreenSize);
	_offscreen = _offscreenBase + (w + 2) + 1;
	prepareScaledGfx(kDefaultScaler);
}

void SystemStub_SDL::destroy() {
	if (_screen) {
		// free()'ed in SDL_Quit()
		_screen = 0;
	}
	free(_offscreenBase);
	_offscreenBase = 0;
}

void SystemStub_SDL::setPalette(const uint8_t *pal, int n, int depth) {
	assert(n <= 256);
	assert(depth <= 8);
	const int shift = 8 - depth;
	for (int i = 0; i < n; ++i) {
		int r = pal[i * 3 + 0];
		int g = pal[i * 3 + 1];
		int b = pal[i * 3 + 2];
		if (shift != 0) {
			r = (r << shift) | (r >> depth);
			g = (g << shift) | (g >> depth);
			b = (b << shift) | (b >> depth);
		}
		_pal[i] = SDL_MapRGB(_screen->format, r, g, b);
	}
}

void SystemStub_SDL::copyRect(int x, int y, int w, int h, const uint8_t *buf, int pitch) {
	assert(x >= 0 && x + w <= _screenW && y >= 0 && y + h <= _screenH);
	for (int i = 0; i < h; ++i) {
		memcpy(_offscreen + y * _screenW + x, buf, w);
		buf += pitch;
		++y;
	}
}

void SystemStub_SDL::fillRect(int x, int y, int w, int h, uint8_t color) {
	assert(x >= 0 && x + w <= _screenW && y >= 0 && y + h <= _screenH);
	for (int i = 0; i < h; ++i) {
		memset(_offscreen + y * _screenW + x, color, w);
		++y;
	}
}

void SystemStub_SDL::shakeScreen(int dx, int dy) {
	_shakeDx = dx;
	_shakeDy = dy;
}

void SystemStub_SDL::updateScreen() {
	SDL_LockSurface(_screen);
	uint16_t *dst = (uint16_t *)_screen->pixels;
	const int dstPitch = _screen->pitch / 2;
	const uint8_t *src = _offscreen;
	const int srcPitch = _screenW;
	int w = _screenW;
	int h = _screenH;
	if (_shakeDy > 0) {
		_scalers[_scaler].clearProc(dst, dstPitch, 0, 0, w, _shakeDy);
		h -= _shakeDy;
		dst += _shakeDy * dstPitch;
	} else if (_shakeDy < 0) {
		_scalers[_scaler].clearProc(dst, dstPitch, 0, h + _shakeDy, w, -_shakeDy);
		h += _shakeDy;
		src -= _shakeDy * srcPitch;
	}
	if (_shakeDx > 0) {
		_scalers[_scaler].clearProc(dst, dstPitch, 0, 0, _shakeDx, h);
		w -= _shakeDx;
		dst += _shakeDx;
	} else if (_shakeDx < 0) {
		_scalers[_scaler].clearProc(dst, dstPitch, w + _shakeDx, 0, -_shakeDx, h);
		w += _shakeDx;
		src -= _shakeDx;
	}
	_scalers[_scaler].scaleProc(dst, dstPitch, _pal, src, srcPitch, w, h);
	SDL_UnlockSurface(_screen);
	const int multiplier = _scalers[_scaler].multiplier;
	SDL_UpdateRect(_screen, 0, 0, _screenW * multiplier, _screenH * multiplier);
	_shakeDx = _shakeDy = 0;
}

void SystemStub_SDL::processEvents() {
	SDL_Event ev;
	while (SDL_PollEvent(&ev)) {
		switch (ev.type) {
		case SDL_KEYUP:
			if (ev.key.keysym.mod & KMOD_ALT) {
				switch (ev.key.keysym.sym) {
				case SDLK_KP_PLUS:
					if (_scaler < _scalersCount - 1) {
						switchScaledGfx(_scaler + 1);
					}
					break;
				case SDLK_KP_MINUS:
					if (_scaler > 0) {
						switchScaledGfx(_scaler - 1);
					}
					break;
				default:
					break;
				}
			}
			break;
		case SDL_QUIT:
			inp.quit = true;
			break;

		}
	}
	updateKeys(&inp);
}

void SystemStub_SDL::sleep(int duration) {
	SDL_Delay(duration);
}

uint32_t SystemStub_SDL::getTimeStamp() {
	return SDL_GetTicks();
}

void SystemStub_SDL::addKeyMapping(int key, uint8_t mask) {
	if (_keyMappingsCount < kKeyMappingsSize) {
		for (int i = 0; i < _keyMappingsCount; ++i) {
			if (_keyMappings[i].keyCode == key) {
				_keyMappings[i].mask = mask;
				return;
			}
		}
		if (_keyMappingsCount < kKeyMappingsSize) {
			_keyMappings[_keyMappingsCount].keyCode = key;
			_keyMappings[_keyMappingsCount].mask = mask;
			++_keyMappingsCount;
		}
	}
}

void SystemStub_SDL::setupDefaultKeyMappings() {
	_keyMappingsCount = 0;
	memset(_keyMappings, 0, sizeof(_keyMappings));

	/* original key mappings of the PC version */

	addKeyMapping(SDLK_LEFT,     SYS_INP_LEFT);
	addKeyMapping(SDLK_UP,       SYS_INP_UP);
	addKeyMapping(SDLK_RIGHT,    SYS_INP_RIGHT);
	addKeyMapping(SDLK_DOWN,     SYS_INP_DOWN);
//	addKeyMapping(SDLK_PAGEUP,   SYS_INP_UP | SYS_INP_RIGHT);
//	addKeyMapping(SDLK_HOME,     SYS_INP_UP | SYS_INP_LEFT);
//	addKeyMapping(SDLK_END,      SYS_INP_DOWN | SYS_INP_LEFT);
//	addKeyMapping(SDLK_PAGEDOWN, SYS_INP_DOWN | SYS_INP_RIGHT);

	addKeyMapping(SDLK_RETURN,   SYS_INP_JUMP);
	addKeyMapping(SDLK_LCTRL,    SYS_INP_RUN);
//	addKeyMapping(SDLK_f,        SYS_INP_RUN);
//	addKeyMapping(SDLK_LALT,     SYS_INP_JUMP);
//	addKeyMapping(SDLK_g,        SYS_INP_JUMP);
	addKeyMapping(SDLK_LSHIFT,   SYS_INP_SHOOT);
//	addKeyMapping(SDLK_h,        SYS_INP_SHOOT);
//	addKeyMapping(SDLK_d,        SYS_INP_SHOOT | SYS_INP_RUN);
//	addKeyMapping(SDLK_SPACE,    SYS_INP_SHOOT | SYS_INP_RUN);
	addKeyMapping(SDLK_ESCAPE,   SYS_INP_ESC);
}

void SystemStub_SDL::updateKeys(PlayerInput *inp) {
	inp->prevMask = inp->mask;
	uint8_t *keyState = SDL_GetKeyState(NULL);
	for (int i = 0; i < _keyMappingsCount; ++i) {
		KeyMapping *keyMap = &_keyMappings[i];
		if (keyState[keyMap->keyCode]) {
			inp->mask |= keyMap->mask;
		} else {
			inp->mask &= ~keyMap->mask;
		}
	}
}

void SystemStub_SDL::prepareScaledGfx(int scaler) {
        const int multiplier = _scalers[scaler].multiplier;
        _screen = SDL_SetVideoMode(_screenW * multiplier, _screenH * multiplier, 16, SDL_SWSURFACE);
	if (!_screen) {
		error("SystemStub_SDL::prepareScaledGfx() Unable to allocate _screen buffer, scaler %d", scaler);
	}
	_scaler = scaler;
}

void SystemStub_SDL::switchScaledGfx(int scaler) {
	if (_scaler != scaler) {
		SDL_FreeSurface(_screen);
		_screen = 0;
		prepareScaledGfx(scaler);
	}
}

