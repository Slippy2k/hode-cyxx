/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009 Gregory Montoir
 */

#ifndef VIDEO_H__
#define VIDEO_H__

#include "intern.h"

struct SystemStub;

enum {
	kSprHorizFlip  = 1 << 0,
	kSprClipTop    = 1 << 2,
	kSprClipBottom = 1 << 3,
	kSprClipLeft   = 1 << 4,
	kSprClipRight  = 1 << 5
};

struct Video {
	enum {
		kScreenWidth = 256,
		kScreenHeight = 192
	};

	SystemStub *_system;

	uint8 _palette[768];
	uint16 _displayPaletteBuffer[256 * 3];
	bool _paletteNeedRefresh;
	uint8 _refreshPalette;
	bool _displayShadowLayer;
	uint8 *_shadowLayer;
	uint8 *_frontLayer;
	uint8 *_backgroundLayer;
	uint8 *_shadowColorLookupTable;
	uint8 _fillColor;
	uint8 _blackColor;
	bool _findBlackColor;

	struct {
		int x1, y1;
		int x2, y2;
		int pitch;
		uint8 color;
	} _drawLine;

	struct {
		int pitch;
		int x, y, w, h;
	} _spr;

	Video(SystemStub *system);
	~Video();

	void refreshGamePalette(const uint16 *pal);
	void updateGameDisplay(uint8 *buf);
	void updateScreen();
	void fillBackBuffer();
	void clearPalette();
	static void decodeRLE(const uint8 *src, uint8 *dst, int size);
	void decodeSPR(const uint8 *src, uint8 *dst, int x, int y, uint8 flags);
	int computeLineOutCode(int x, int y);
	bool clipLineCoords(int &x1, int &y1, int &x2, int &y2);
	void drawLine(int x1, int y1, int x2, int y2);
	void applyShadowColors(int x, int y, int src_w, int src_h, int dst_pitch, int src_pitch, uint8 *dst1, uint8 *dst2, uint8 *src1, uint8 *src2);
	void buildShadowColorLookupTable(const uint8 *src, uint8 *dst);
};

#endif // VIDEO_H__

