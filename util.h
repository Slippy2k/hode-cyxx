/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
 */

#ifndef UTIL_H__
#define UTIL_H__

#include "intern.h"

enum {
	kDebug_GAME     = 1 << 0,
	kDebug_RESOURCE = 1 << 1,
	kDebug_ANDY     = 1 << 2,
	kDebug_SOUND    = 1 << 3,
};

extern int g_debugMask;

void debug(int mask, const char *msg, ...);
void error(const char *msg, ...);
void warning(const char *msg, ...);

#endif // UTIL_H__
