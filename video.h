/*
 * Heart of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir (cyx@users.sourceforge.net)
 */

#ifndef VIDEO_H__
#define VIDEO_H__

#include "intern.h"

struct System;

enum {
	kSprHorizFlip  = 1 << 0, // left-right
	kSprVertFlip   = 1 << 1, // up-down
	kSprClipTop    = 1 << 2,
	kSprClipBottom = 1 << 3,
	kSprClipLeft   = 1 << 4,
	kSprClipRight  = 1 << 5
};

struct Video {
	enum {
		W = 256,
		H = 192
	};

	static const uint8_t _fontCharactersTable[78];

	System *_system;

	uint8_t _palette[256 * 3];
	uint16_t _displayPaletteBuffer[256 * 3];
	bool _paletteNeedRefresh;
	bool _refreshPalette;
	bool _displayShadowLayer;
	uint8_t *_shadowLayer;
	uint8_t *_frontLayer;
	uint8_t *_backgroundLayer;
	uint8_t *_shadowColorLookupTable;
	uint16_t _fadePaletteBuffer[256 * 3];
	uint8_t *_shadowScreenMaskBuffer;
	uint8_t *_transformShadowBuffer;
	uint8_t _transformShadowLayerDelta;
	uint8_t _fillColor;
	uint8_t _blackColor;
	uint8_t _shadowColorLut[256];
	const uint8_t *_font;

	struct {
		int x1, y1;
		int x2, y2;
		int pitch;
		uint8_t color;
	} _drawLine;

	struct {
		int pitch;
		int x, y, w, h;
	} _spr;

	Video(System *system);
	~Video();

	void refreshGamePalette(const uint16_t *pal);
	void updateGameDisplay(uint8_t *buf);
	void updateScreen();
	void fillBackBuffer();
	void clearPalette();
	static void decodeRLE(const uint8_t *src, uint8_t *dst, int size);
	void decodeSPR(const uint8_t *src, uint8_t *dst, int x, int y, uint8_t flags, uint16_t spr_w, uint16_t spr_h);
	int computeLineOutCode(int x, int y);
	bool clipLineCoords(int &x1, int &y1, int &x2, int &y2);
	void drawLine(int x1, int y1, int x2, int y2);
	void applyShadowColors(int x, int y, int src_w, int src_h, int dst_pitch, int src_pitch, uint8_t *dst1, uint8_t *dst2, uint8_t *src1, uint8_t *src2);
	void buildShadowColorLookupTable(const uint8_t *src, uint8_t *dst);

	uint8_t findStringCharacterFontIndex(uint8_t chr) const;
	void drawStringCharacter(int x, int y, uint8_t chr, uint8_t color, uint8_t *dst);
	void drawString(const char *s, int x, int y, uint8_t color, uint8_t *dst);
	uint8_t findWhiteColor() const;
};

#endif // VIDEO_H__

