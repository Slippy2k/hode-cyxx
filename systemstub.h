/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
 */

#ifndef SYSTEMSTUB_H__
#define SYSTEMSTUB_H__

#include "intern.h"

#define SYS_INP_UP    (1 << 0)
#define SYS_INP_RIGHT (1 << 1)
#define SYS_INP_DOWN  (1 << 2)
#define SYS_INP_LEFT  (1 << 3)
#define SYS_INP_RUN   (1 << 4) /* (1 << 0) */
#define SYS_INP_JUMP  (1 << 5) /* (1 << 1) */
#define SYS_INP_SHOOT (1 << 6) /* (1 << 2) */
#define SYS_INP_ESC   (1 << 7)

struct PlayerInput {
	uint8_t prevMask, mask;
	bool quit;
	bool screenshot;

	bool keyPressed(int keyMask) const {
		return (prevMask & keyMask) == 0 && (mask & keyMask) == keyMask;
	}
	bool keyReleased(int keyMask) const {
		return (prevMask & keyMask) == keyMask && (mask & keyMask) == 0;
	}
};

struct SystemStub {
	typedef void (*AudioCallback)(void *param, int16_t *stream, int len);

	PlayerInput inp;

	virtual ~SystemStub() {}

	virtual void init(const char *title, int w, int h) = 0;
	virtual void destroy() = 0;

	virtual void setPalette(const uint8_t *pal, int n, int depth = 8) = 0;
	virtual void copyRect(int x, int y, int w, int h, const uint8_t *buf, int pitch) = 0;
	virtual void fillRect(int x, int y, int w, int h, uint8_t color) = 0;
	virtual void shakeScreen(int dx, int dy) = 0;
	virtual void updateScreen() = 0;

	virtual void processEvents() = 0;
	virtual void sleep(int duration) = 0;
	virtual uint32_t getTimeStamp() = 0;

	virtual void startAudio(AudioCallback callback, void *param) = 0;
	virtual void stopAudio() = 0;
	virtual uint32_t getOutputSampleRate() = 0;
	virtual void lockAudio() = 0;
	virtual void unlockAudio() = 0;
};

extern SystemStub *SystemStub_SDL_create();

#endif // SYSTEMSTUB_H__
