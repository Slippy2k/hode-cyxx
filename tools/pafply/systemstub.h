
#ifndef __SYSTEMSTUB_H__
#define __SYSTEMSTUB_H__

#include "intern.h"

extern void sysInit(int w, int h, const char *title);
extern void sysDestroy();
extern void sysSetPalette(const uint8 *palData, int numColors);
extern void sysCopyRect(const uint8 *src, int pitch, int x, int y, int w, int h);
extern bool sysDelay(int amount);

#endif // __SYSTEMSTUB_H__
