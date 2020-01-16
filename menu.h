
#ifndef MENU_H__
#define MENU_H__

#include "intern.h"

struct Game;
struct PafPlayer;
struct Resource;
struct Video;

struct DatSpritesGroup {
	uint32_t currentFrame; // 0
	uint32_t firstFrame; // 4
	uint32_t size; // 8 following this header
	uint16_t count; // 12
	uint16_t num; // 14
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
	uint32_t offset; // 4
	uint32_t palette; // 8
} PACKED; // sizeof == 12

struct Menu {
	enum {
		kCheckpointLevelsCount = 8,
		kOptionsCount = 19
	};
	enum {
		kTitleScreen_AssignPlayer,
		kTitleScreen_Play,
		kTitleScreen_Options,
		kTitleScreen_Quit
	};

	Game *_g;
	PafPlayer *_paf;
	Resource *_res;
	Video *_video;

	SetupConfig *_config;
	uint8_t _checkpointNum;
	uint8_t _levelNum;

	DatSpritesGroup *_titleSprites;
	DatSpritesGroup *_playerSprites;
	const uint8_t *_titleBitmapData;
	uint32_t _titleBitmapSize;
	const uint8_t *_playerBitmapData;
	uint32_t _playerBitmapSize;
	uint32_t _optionsBitmapSize[kOptionsCount];
	const uint8_t *_optionsBitmapData[kOptionsCount];
	DatBitmapsGroup *_cutscenesBitmaps;
	const uint8_t *_cutscenesBitmapsData;
	DatBitmapsGroup *_checkpointsBitmaps[kCheckpointLevelsCount];
	const uint8_t *_checkpointsBitmapsData[kCheckpointLevelsCount];
	DatBitmapsGroup *_levelsBitmaps;
	const uint8_t *_levelsBitmapsData;
	DatSpritesGroup *_iconsSprites;

	const uint8_t *_digitsData;
	const uint8_t *_optionData;
	const uint8_t *_soundData;

	uint8_t _paletteBuffer[256 * 3];

	uint8_t _optionNum;

	Menu(Game *g, PafPlayer *paf, Resource *res, Video *video);

	void loadData();

	int getSoundNum(int num) const;
	void playSound(int num);

	void drawSprite(const DatSpritesGroup *spriteGroup, uint32_t num);
	void drawSpritePos(const DatSpritesGroup *spriteGroup, int x, int y, uint32_t num);
	void drawSpriteNextFrame(DatSpritesGroup *spriteGroup, int x, int y);
	void refreshScreen(bool updatePalette);

	bool mainLoop();

	void drawTitleScreen(int option);
	int handleTitleScreen();
	void drawDigit(int x, int y, int num);
	void drawBitmap(const DatBitmapsGroup *bitmapsGroup, const uint8_t *bitmapData, int x, int y, int w, int h, uint8_t baseColor = 0);
	void setCurrentPlayer(int num);
	void drawPlayerProgress(int state, int cursor);
	void handleAssignPlayer();
	void drawLevelScreen();
	void changeToOption(int num);
	void handleOptions();
};

#endif // MENU_H__
