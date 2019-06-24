/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
 */

#include <SDL.h>
#include <stdarg.h>
#include <math.h>
#include "scaler.h"
#include "systemstub.h"
#include "util.h"

static const char *kIconBmp = "icon.bmp";

static const int kJoystickCommitValue = 3200;

static int _scalerMultiplier = 3;
static const Scaler *_scaler = &scaler_xbr;
static const float _gamma = 1.f;

static const int _pixelFormat = SDL_PIXELFORMAT_RGB888;

static const struct {
	const char *name;
	const Scaler *scaler;
} _scalers[] = {
	{ "nearest", &scaler_nearest },
	{ "xbr", &scaler_xbr },
	{ 0, 0 }
};

struct KeyMapping {
	int keyCode;
	int mask;
};

struct SystemStub_SDL : SystemStub {
	enum {
		kCopyRectsSize = 200,
		kKeyMappingsSize = 20,
		kAudioHz = 22050,
	};

	uint8_t *_offscreenLut;
	uint32_t *_offscreenRgb;
	SDL_Window *_window;
	SDL_Renderer *_renderer;
	SDL_Texture *_texture;
	int _texW, _texH;
	SDL_PixelFormat *_fmt;
	uint32_t _pal[256];
	int _screenW, _screenH;
	int _shakeDx, _shakeDy;
	KeyMapping _keyMappings[kKeyMappingsSize];
	int _keyMappingsCount;
	AudioCallback _audioCb;
	bool _paletteGrayScale;
	uint8_t _gammaLut[256];
	SDL_GameController *_controller;
	SDL_Joystick *_joystick;

	SystemStub_SDL();
	virtual ~SystemStub_SDL() {}
	virtual void init(const char *title, int w, int h, bool fullscreen);
	virtual void destroy();
	virtual void setScaler(const char *name, int multiplier);
	virtual void setGamma(float gamma);
	virtual void setPaletteScale(bool gray);
	virtual void setPalette(const uint8_t *pal, int n, int depth);
	virtual void copyRect(int x, int y, int w, int h, const uint8_t *buf, int pitch);
	virtual void fillRect(int x, int y, int w, int h, uint8_t color);
	virtual void shakeScreen(int dx, int dy);
	virtual void updateScreen();
	virtual void processEvents();
	virtual void sleep(int duration);
	virtual uint32_t getTimeStamp();

	virtual void startAudio(AudioCallback callback);
	virtual void stopAudio();
	virtual uint32_t getOutputSampleRate();
	virtual void lockAudio();
	virtual void unlockAudio();
	virtual AudioCallback setAudioCallback(AudioCallback callback);

	void addKeyMapping(int key, uint8_t mask);
	void setupDefaultKeyMappings();
	void updateKeys(PlayerInput *inp);
	void prepareScaledGfx(const char *caption, bool fullscreen);
};

SystemStub *SystemStub_SDL_create() {
	return new SystemStub_SDL();
}

SystemStub_SDL::SystemStub_SDL():
	_paletteGrayScale(false) {
	for (int i = 0; i < 256; ++i) {
		_gammaLut[i] = i;
	}
}

void SystemStub_SDL::init(const char *title, int w, int h, bool fullscreen) {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER);
	SDL_ShowCursor(SDL_DISABLE);
	setupDefaultKeyMappings();
	memset(&inp, 0, sizeof(inp));
	memset(&pad, 0, sizeof(pad));
	_screenW = w;
	_screenH = h;
	_shakeDx = _shakeDy = 0;
	memset(_pal, 0, sizeof(_pal));
	const int offscreenSize = w * h;
	_offscreenLut = (uint8_t *)malloc(offscreenSize);
	if (!_offscreenLut) {
		error("SystemStub_SDL::init() Unable to allocate offscreen buffer");
	}
	_offscreenRgb = (uint32_t *)malloc(offscreenSize * sizeof(uint32_t));
	if (!_offscreenRgb) {
		error("SystemStub_SDL::init() Unable to allocate RGB offscreen buffer");
	}
	memset(_offscreenLut, 0, offscreenSize);
	prepareScaledGfx(title, fullscreen);
	_joystick = 0;
	_controller = 0;
	const int count = SDL_NumJoysticks();
	if (count > 0) {
		SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");
		for (int i = 0; i < count; ++i) {
			if (SDL_IsGameController(i)) {
				_controller = SDL_GameControllerOpen(i);
				if (_controller) {
					fprintf(stdout, "Using controller '%s'\n", SDL_GameControllerName(_controller));
					break;
				}
			}
			_joystick = SDL_JoystickOpen(i);
			if (_joystick) {
				fprintf(stdout, "Using joystick '%s'", SDL_JoystickName(_joystick));
				break;
			}
		}
	}
}

void SystemStub_SDL::destroy() {
	free(_offscreenLut);
	_offscreenLut = 0;
	free(_offscreenRgb);
	_offscreenRgb = 0;

	if (_controller) {
		SDL_GameControllerClose(_controller);
		_controller = 0;
	}
	if (_joystick) {
		SDL_JoystickClose(_joystick);
		_joystick = 0;
	}
}

void SystemStub_SDL::setScaler(const char *name, int multiplier) {
	if (multiplier != 0) {
		_scalerMultiplier = multiplier;
	}
	if (name) {
		for (int i = 0; _scalers[i].name; ++i) {
			if (strcmp(name, _scalers[i].name) == 0) {
				_scaler = _scalers[i].scaler;
				break;
			}
		}
	}
}

void SystemStub_SDL::setGamma(float gamma) {
	for (int i = 0; i < 256; ++i) {
		_gammaLut[i] = (uint8_t)round(pow(i / 255., 1. / gamma) * 255);
	}
}

void SystemStub_SDL::setPaletteScale(bool gray) {
	_paletteGrayScale = gray;
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
		if (_paletteGrayScale) {
			const int gray = (r * 30 + g * 59 + b * 11) / 100;
			r = g = b = gray;
		}
		r = _gammaLut[r];
		g = _gammaLut[g];
		b = _gammaLut[b];
		_pal[i] = SDL_MapRGB(_fmt, r, g, b);
	}
}

void SystemStub_SDL::copyRect(int x, int y, int w, int h, const uint8_t *buf, int pitch) {
	assert(x >= 0 && x + w <= _screenW && y >= 0 && y + h <= _screenH);
	for (int i = 0; i < h; ++i) {
		memcpy(_offscreenLut + y * _screenW + x, buf, w);
		buf += pitch;
		++y;
	}
}

void SystemStub_SDL::fillRect(int x, int y, int w, int h, uint8_t color) {
	assert(x >= 0 && x + w <= _screenW && y >= 0 && y + h <= _screenH);
	for (int i = 0; i < h; ++i) {
		memset(_offscreenLut + y * _screenW + x, color, w);
		++y;
	}
}

void SystemStub_SDL::shakeScreen(int dx, int dy) {
	_shakeDx = dx;
	_shakeDy = dy;
}

static void clearScreen(uint32_t *dst, int dstPitch, int x, int y, int w, int h) {
	uint32_t *p = dst + (y * dstPitch + x) * _scalerMultiplier;
	for (int j = 0; j < h * _scalerMultiplier; ++j) {
		memset(p, 0, w * sizeof(uint32_t) * _scalerMultiplier);
		p += dstPitch;
	}
}

void SystemStub_SDL::updateScreen() {
	void *texturePtr = 0;
	int texturePitch = 0;
	if (SDL_LockTexture(_texture, 0, &texturePtr, &texturePitch) != 0) {
		return;
	}
	uint32_t *dst = (uint32_t *)texturePtr;
	assert((texturePitch & 3) == 0);
	const int dstPitch = texturePitch / sizeof(uint32_t);
	const uint8_t *src = _offscreenLut;
	const int srcPitch = _screenW;
	int w = _screenW;
	int h = _screenH;
	if (_shakeDy > 0) {
		clearScreen(dst, dstPitch, 0, 0, w, _shakeDy);
		h -= _shakeDy;
		dst += _shakeDy * dstPitch;
	} else if (_shakeDy < 0) {
		h += _shakeDy;
		clearScreen(dst, dstPitch, 0, h, w, -_shakeDy);
		src -= _shakeDy * srcPitch;
	}
	if (_shakeDx > 0) {
		clearScreen(dst, dstPitch, 0, 0, _shakeDx, h);
		w -= _shakeDx;
		dst += _shakeDx;
	} else if (_shakeDx < 0) {
		w += _shakeDx;
		clearScreen(dst, dstPitch, w, 0, -_shakeDx, h);
		src -= _shakeDx;
	}
	uint32_t *p = _offscreenRgb;
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			p[x] = _pal[src[y * w + x]];
		}
		p += w;
	}
	_scaler->scale(_scalerMultiplier, dst, dstPitch, _offscreenRgb, srcPitch, w, h);
	SDL_UnlockTexture(_texture);

	SDL_RenderClear(_renderer);
	SDL_Rect r;
	r.x = 0;
	r.y = 0;
	r.w = _screenW * _scalerMultiplier;
	r.h = _screenH * _scalerMultiplier;
	SDL_RenderCopy(_renderer, _texture, 0, &r);
        SDL_RenderPresent(_renderer);
	_shakeDx = _shakeDy = 0;
}

void SystemStub_SDL::processEvents() {
	SDL_Event ev;
	pad.prevMask = pad.mask;
	while (SDL_PollEvent(&ev)) {
		switch (ev.type) {
		case SDL_KEYUP:
			if (ev.key.keysym.sym == SDLK_s) {
				inp.screenshot = true;
			}
			break;
		case SDL_JOYHATMOTION:
			if (_joystick) {
				pad.mask = 0;
				if (ev.jhat.value & SDL_HAT_UP) {
					pad.mask |= SYS_INP_UP;
				}
				if (ev.jhat.value & SDL_HAT_DOWN) {
					pad.mask |= SYS_INP_DOWN;
				}
				if (ev.jhat.value & SDL_HAT_LEFT) {
					pad.mask |= SYS_INP_LEFT;
				}
				if (ev.jhat.value & SDL_HAT_RIGHT) {
					pad.mask |= SYS_INP_RIGHT;
				}
			}
			break;
		case SDL_JOYAXISMOTION:
			if (_joystick) {
				switch (ev.jaxis.axis) {
				case 0:
					pad.mask &= ~(SYS_INP_RIGHT | SYS_INP_LEFT);
					if (ev.jaxis.value > kJoystickCommitValue) {
						pad.mask |= SYS_INP_RIGHT;
					} else if (ev.jaxis.value < -kJoystickCommitValue) {
						pad.mask |= SYS_INP_LEFT;
					}
					break;
				case 1:
					pad.mask &= ~(SYS_INP_UP | SYS_INP_DOWN);
					if (ev.jaxis.value > kJoystickCommitValue) {
						pad.mask |= SYS_INP_DOWN;
					} else if (ev.jaxis.value < -kJoystickCommitValue) {
						pad.mask |= SYS_INP_UP;
					}
					break;
				}
			}
			break;
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			if (_joystick) {
				const bool pressed = (ev.jbutton.state == SDL_PRESSED);
				switch (ev.jbutton.button) {
				case 0:
					if (pressed) {
						pad.mask |= SYS_INP_RUN;
					} else {
						pad.mask &= ~SYS_INP_RUN;
					}
					break;
				case 1:
					if (pressed) {
						pad.mask |= SYS_INP_JUMP;
					} else {
						pad.mask &= ~SYS_INP_JUMP;
					}
					break;
				case 2:
					if (pressed) {
						pad.mask |= SYS_INP_SHOOT;
					} else {
						pad.mask &= ~SYS_INP_SHOOT;
					}
					break;
				}
			}
			break;
		case SDL_CONTROLLERAXISMOTION:
			if (_controller) {
				switch (ev.caxis.axis) {
				case SDL_CONTROLLER_AXIS_LEFTX:
				case SDL_CONTROLLER_AXIS_RIGHTX:
					if (ev.caxis.value < -kJoystickCommitValue) {
						pad.mask |= SYS_INP_LEFT;
					} else {
						pad.mask &= ~SYS_INP_LEFT;
					}
					if (ev.caxis.value > kJoystickCommitValue) {
						pad.mask |= SYS_INP_RIGHT;
					} else {
						pad.mask &= ~SYS_INP_RIGHT;
					}
					break;
				case SDL_CONTROLLER_AXIS_LEFTY:
				case SDL_CONTROLLER_AXIS_RIGHTY:
					if (ev.caxis.value < -kJoystickCommitValue) {
						pad.mask |= SYS_INP_UP;
					} else {
						pad.mask &= ~SYS_INP_UP;
					}
					if (ev.caxis.value > kJoystickCommitValue) {
						pad.mask |= SYS_INP_DOWN;
					} else {
						pad.mask &= ~SYS_INP_DOWN;
					}
					break;
				}
			}
			break;
		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
			if (_controller) {
				const bool pressed = (ev.cbutton.state == SDL_PRESSED);
				switch (ev.cbutton.button) {
				case SDL_CONTROLLER_BUTTON_A:
					if (pressed) {
						pad.mask |= SYS_INP_RUN;
					} else {
						pad.mask &= ~SYS_INP_RUN;
					}
					break;
				case SDL_CONTROLLER_BUTTON_B:
					if (pressed) {
						pad.mask |= SYS_INP_JUMP;
					} else {
						pad.mask &= ~SYS_INP_JUMP;
					}
					break;
				case SDL_CONTROLLER_BUTTON_X:
					if (pressed) {
						pad.mask |= SYS_INP_SHOOT;
					} else {
						pad.mask &= ~SYS_INP_SHOOT;
					}
					break;
				case SDL_CONTROLLER_BUTTON_Y:
					break;
				case SDL_CONTROLLER_BUTTON_BACK:
				case SDL_CONTROLLER_BUTTON_START:
					inp.quit = pressed;
					break;
				case SDL_CONTROLLER_BUTTON_DPAD_UP:
					if (pressed) {
						pad.mask |= SYS_INP_UP;
					} else {
						pad.mask &= ~SYS_INP_UP;
					}
					break;
				case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
					if (pressed) {
						pad.mask |= SYS_INP_DOWN;
					} else {
						pad.mask &= ~SYS_INP_DOWN;
					}
					break;
				case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
					if (pressed) {
						pad.mask |= SYS_INP_LEFT;
					} else {
						pad.mask &= ~SYS_INP_LEFT;
					}
					break;
				case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
					if (pressed) {
						pad.mask |= SYS_INP_RIGHT;
					} else {
						pad.mask &= ~SYS_INP_RIGHT;
					}
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

static void mixAudioS16(void *param, uint8_t *buf, int len) {
	SystemStub_SDL *stub = (SystemStub_SDL *)param;
	memset(buf, 0, len);
	stub->_audioCb.proc(stub->_audioCb.userdata, (int16_t *)buf, len / 2);
}

void SystemStub_SDL::startAudio(AudioCallback callback) {
	SDL_AudioSpec desired;
	memset(&desired, 0, sizeof(desired));
	desired.freq = kAudioHz;
	desired.format = AUDIO_S16SYS;
	desired.channels = 2;
	desired.samples = 2048;
	desired.callback = mixAudioS16;
	desired.userdata = this;
	if (SDL_OpenAudio(&desired, 0) == 0) {
		_audioCb = callback;
		SDL_PauseAudio(0);
	} else {
		error("SystemStub_SDL::startAudio() Unable to open sound device");
	}
}

void SystemStub_SDL::stopAudio() {
	SDL_CloseAudio();
}

uint32_t SystemStub_SDL::getOutputSampleRate() {
	return kAudioHz;
}

void SystemStub_SDL::lockAudio() {
	SDL_LockAudio();
}

void SystemStub_SDL::unlockAudio() {
	SDL_UnlockAudio();
}

AudioCallback SystemStub_SDL::setAudioCallback(AudioCallback callback) {
	SDL_LockAudio();
	AudioCallback cb = _audioCb;
	_audioCb = callback;
	SDL_UnlockAudio();
	return cb;
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

	addKeyMapping(SDL_SCANCODE_LEFT,     SYS_INP_LEFT);
	addKeyMapping(SDL_SCANCODE_UP,       SYS_INP_UP);
	addKeyMapping(SDL_SCANCODE_RIGHT,    SYS_INP_RIGHT);
	addKeyMapping(SDL_SCANCODE_DOWN,     SYS_INP_DOWN);
//	addKeyMapping(SDL_SCANCODE_PAGEUP,   SYS_INP_UP | SYS_INP_RIGHT);
//	addKeyMapping(SDL_SCANCODE_HOME,     SYS_INP_UP | SYS_INP_LEFT);
//	addKeyMapping(SDL_SCANCODE_END,      SYS_INP_DOWN | SYS_INP_LEFT);
//	addKeyMapping(SDL_SCANCODE_PAGEDOWN, SYS_INP_DOWN | SYS_INP_RIGHT);

	addKeyMapping(SDL_SCANCODE_RETURN,   SYS_INP_JUMP);
	addKeyMapping(SDL_SCANCODE_LCTRL,    SYS_INP_RUN);
//	addKeyMapping(SDL_SCANCODE_f,        SYS_INP_RUN);
//	addKeyMapping(SDL_SCANCODE_LALT,     SYS_INP_JUMP);
//	addKeyMapping(SDL_SCANCODE_g,        SYS_INP_JUMP);
	addKeyMapping(SDL_SCANCODE_LSHIFT,   SYS_INP_SHOOT);
//	addKeyMapping(SDL_SCANCODE_h,        SYS_INP_SHOOT);
//	addKeyMapping(SDL_SCANCODE_d,        SYS_INP_SHOOT | SYS_INP_RUN);
//	addKeyMapping(SDL_SCANCODE_SPACE,    SYS_INP_SHOOT | SYS_INP_RUN);
	addKeyMapping(SDL_SCANCODE_ESCAPE,   SYS_INP_ESC);
}

void SystemStub_SDL::updateKeys(PlayerInput *inp) {
	inp->prevMask = inp->mask;
	const uint8_t *keyState = SDL_GetKeyboardState(NULL);
	for (int i = 0; i < _keyMappingsCount; ++i) {
		KeyMapping *keyMap = &_keyMappings[i];
		if (keyState[keyMap->keyCode]) {
			inp->mask |= keyMap->mask;
		} else {
			inp->mask &= ~keyMap->mask;
		}
	}
	inp->mask |= pad.mask;
}

void SystemStub_SDL::prepareScaledGfx(const char *caption, bool fullscreen) {
	int flags = 0;
	if (fullscreen) {
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	} else {
		flags |= SDL_WINDOW_RESIZABLE;
	}
	_texW = _screenW * _scalerMultiplier;
	_texH = _screenH * _scalerMultiplier;
	_window = SDL_CreateWindow(caption, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _texW, _texH, flags);
	SDL_Surface *icon = SDL_LoadBMP(kIconBmp);
	if (icon) {
		SDL_SetWindowIcon(_window, icon);
		SDL_FreeSurface(icon);
	}
	_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
	SDL_RenderSetLogicalSize(_renderer, _texW, _texH);
	_texture = SDL_CreateTexture(_renderer, _pixelFormat, SDL_TEXTUREACCESS_STREAMING, _texW, _texH);
	_fmt = SDL_AllocFormat(_pixelFormat);
}
