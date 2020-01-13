
#ifndef MENU_H__
#define MENU_H__

#include "intern.h"

struct Game;
struct PafPlayer;
struct Resource;
struct Video;

struct DatSpritesGroup {
	uint32_t unk0; // 0
	uint32_t unk4; // 4
	uint32_t size; // 8 following this header
	uint32_t count; // 12
} PACKED; // sizeof == 16

struct DatBitmap {
	uint32_t size; // 0
	uint32_t unk4; // 4
	// 8 lzw + 768 palette
} PACKED; // sizeof == 8

struct DatBitmapsGroup {
	uint8_t w;
	uint8_t h;
	uint8_t colors;
	uint8_t unk3; // padding to 4 bytes
	uint32_t unk4; // 4
	uint32_t unk8; // 8
} PACKED; // sizeof == 12

struct Menu {

	Game *_g;
	PafPlayer *_paf;
	Resource *_res;
	Video *_video;

	DatSpritesGroup *_titleSprites;
	DatSpritesGroup *_playerSprites;
	const uint8_t *_titleBitmapData;
	uint32_t _titleBitmapSize;
	const uint8_t *_playerBitmapData;
	uint32_t _playerBitmapSize;
	const uint8_t *_digitTiles;
	const uint8_t *_soundData;

	uint8_t _paletteBuffer[256 * 3];

	int _currentOption;

	Menu(Game *g, PafPlayer *paf, Resource *res, Video *video);

	void loadData();

	int getSoundNum(int num) const;
	void playSound(int num);

	void drawSprite(const DatSpritesGroup *spriteGroup, uint32_t num);
	void drawSpritePos(const DatSpritesGroup *spriteGroup, int x, int y, uint32_t num);
	void drawBitmap(const uint8_t *bitmapData, uint32_t bitmapSize, const DatSpritesGroup *spritesGroup);
	void refreshScreen(bool updatePalette);

	bool mainLoop();
	bool handleTitleScreen();
	void drawDigit(int x, int y, int num);
	void drawPlayerProgress(int num, int b);
	void handleAssignPlayer();
};

#endif // MENU_H__
