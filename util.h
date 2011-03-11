/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009 Gregory Montoir
 */

#ifndef UTIL_H__
#define UTIL_H__

#include "intern.h"

enum {
	DEBUG_GAME = 1 << 0,
	DEBUG_RESOURCE = 1 << 1
};

extern int g_debugMask;

void debug(int mask, const char *msg, ...);
void error(const char *msg, ...);
void warning(const char *msg, ...);

#endif // UTIL_H__
