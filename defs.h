/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
 */

#ifndef DEFS_H__
#define DEFS_H__

enum {
	kPosTopScreen    = 0,
	kPosRightScreen  = 1,
	kPosBottomScreen = 2,
	kPosLeftScreen   = 3
};

enum {
	kNone = 0xFFFFFFFF, // (uint32_t)-1
	kLvlAnimHdrOffset = 0x2C
};

enum {
	kLvl_rock, // 0
	kLvl_fort,
	kLvl_pwr1,
	kLvl_isld,
	kLvl_lava, // 4
	kLvl_pwr2,
	kLvl_lar1,
	kLvl_lar2,
	kLvl_dark, // 8
	kLvl_test
};

struct Point8_t {
	int8_t x;
	int8_t y;
} PACKED;

struct Point16_t {
	int16_t x;
	int16_t y;
};

struct AnimBackgroundData {
	const uint8_t *currentSpriteData; // 0
	uint8_t *firstSpriteData; // 4
	uint8_t *otherSpriteData; // 8
	uint16_t framesCount; // 12
	uint16_t currentFrame; // 14
};

struct LvlAnimSeqHeader {
	uint16_t firstFrame;
	uint16_t unk2;
	int8_t dx; // 4
	int8_t dy; // 5
	uint8_t count; // 6
	uint8_t unk7; // 7
	uint16_t sound;
	uint16_t flags0;
	uint16_t flags1;
	uint16_t unkE;
	uint32_t offset; // 0x10, LvlAnimSeqFrameHeader
} PACKED;

#define SIZEOF_LvlAnimSeqHeader 20

struct LvlAnimSeqFrameHeader {
	uint16_t move; // 0
	uint16_t anim; // 2
	uint8_t frame; // 4
	uint8_t unk5; // 5
	int8_t unk6;
	int8_t unk7;
} PACKED;

#define SIZEOF_LvlAnimSeqFrameHeader 8

struct LvlAnimHeader {
	uint16_t unk0;
	uint8_t seqCount;
	uint8_t unk2;
	uint32_t seqOffset;
} PACKED;

#define SIZEOF_LvlAnimHeader 8

struct LvlSprMoveData {
	uint8_t op1;
	uint8_t op2;
	uint16_t op3;
	uint16_t op4;
	uint16_t unk0x6;
} PACKED;

#define SIZEOF_LvlSprMoveData 8

struct LvlSprHotspotData {
	Point8_t pts[8];
} PACKED;

#define SIZEOF_LvlSprHotspotData 16

struct LvlObjectData {
	uint8_t unk0;
	uint8_t index;
	uint16_t framesCount;
	uint16_t hotspotsCount;
	uint16_t movesCount;
	uint16_t animsCount;
	uint8_t refCount; // 0xA
	uint8_t frame; // 0xB
	uint16_t anim; // 0xC
	uint8_t unkE;
	uint8_t unkF;
	uint8_t *animsInfoData; // 0x10, LevelSprAnimInfo
	uint8_t *movesData; // 0x14, LvlSprMoveData
	uint8_t *framesData; // 0x18
/*	uint32_t framesDataOffset; */ // 0x1C, not needed
	uint8_t *animsData; // 0x20
	uint8_t *coordsData; // 0x24
	uint8_t *hotspotsData; // 0x28, LvlSprHotspotData
};

struct Game;
struct SssObject;

struct LvlObject {
	int32_t xPos; // 0
	int32_t yPos; // 4
	uint8_t screenNum; // 8
	uint8_t screenState;
	uint8_t flags;
	uint8_t frame;
	uint16_t anim;
	uint8_t type;
	uint8_t spriteNum;
	uint16_t flags0; // hFlags
	uint16_t flags1; // vFlags
	uint16_t flags2; // spriteType
	uint8_t objectUpdateType;
	uint8_t hitCount;
	LvlObject *linkObjPtr; // attachedLvlObject, plasma cannon object for _andyObject
	uint16_t width;
	uint16_t height;
	uint8_t actionKeyMask;
	uint8_t directionKeyMask;
	uint16_t currentSprite;
	uint16_t currentSound;
	uint8_t unk26;
	uint8_t unk27;
	const uint8_t *bitmapBits;
	int (Game::*callbackFuncPtr)(LvlObject *ptr);
	void *dataPtr;
// ptr->data0x2988==0:AndyLvlObjectData
// ptr->type==0: AnimBackgroundData * / _animBackgroundDataTable
// ptr->type==1: _resLvlScreenBackgroundDataTable[num].backgroundSoundTable[ptr->flags & 0xFF];
// ptr->type==8(_andyObject): &_gameUnkList1Element
	SssObject *sssObj; // 0x34
	LvlObjectData *levelData0x2988;
	Point16_t posTable[8];
	LvlObject *nextPtr;
};

struct SssFilter;
struct SssPcm;

struct SssObject {
	SssPcm *pcm; // 0x0
	uint16_t num; // 0x4
	uint16_t unk6; // 0x6
	int8_t unk8; // 0x8
	int8_t priority; // 0x9
	uint8_t flags; // 0xA
	uint8_t unkB;
	uint32_t flags0; // 0xC
	uint32_t flags1; // 0x10
	int32_t volume; // 0x14 panning default:64
	int32_t unk18; // 0x18 volume (db) default:128
	int panL; // 0x1C leftGain
	int panR; // 0x20 rightGain
	int panType; // 0x24
	const int16_t *currentPcmPtr; // 0x28
	int32_t unk2C; // 0x2C
	SssObject *prevPtr; // 0x30
	SssObject *nextPtr; // 0x34
	const uint8_t *codeDataStage1; // 0x38
	const uint8_t *codeDataStage2; // 0x3C
	const uint8_t *codeDataStage3; // 0x40
	const uint8_t *codeDataStage4; // 0x44
	int32_t counter; // 0x48 repeat_count
	int32_t unk4C; // 0x4C loop_count
	int32_t unk50; // 0x50 delay
	int32_t unk54; // 0x54 volume_modulate_steps
	int32_t unk58; // 0x58 panning_modulate_steps
	int32_t unk5C; // 0x5C volume_modulate_current
	int32_t unk60; // 0x60 volume_modulate_delta
	int32_t unk64; // 0x64 panning_modulate_current
	int32_t unk68; // 0x68 panning_modulate_delta
	int32_t unk6C; // 0x6C seek_pos
	int *volumePtr; // 0x70
	LvlObject *lvlObject; // 0x74
	int32_t unk78; // 0x78
	int32_t unk7C; // 0x7C
	SssFilter *filter;
};

struct Sprite {
	int16_t xPos;
	int16_t yPos;
	const uint8_t *bitmapBits;
	Sprite *nextPtr;
	uint16_t num;
	uint16_t flags;
};

struct BoundingBox {
	int32_t x1; // 0
	int32_t y1; // 4
	int32_t x2; // 8
	int32_t y2; // C
};

struct AndyLvlObjectData {
	uint8_t unk0;
	uint8_t unk1;
	uint8_t unk2;
	uint8_t unk3;
	uint8_t unk4;
	uint8_t unk5;
	uint16_t unk6;
	BoundingBox boundingBox; // 0x8
	int32_t dxPos; // 0x18
	int32_t dyPos; // 0x1C
	LvlObject *shootLvlObject; // 0x20 'cannon plasma' or 'special powers'
};

struct ShootLvlObjectData {
	uint8_t unk0;
	uint8_t unk1;
	uint8_t unk2;
	uint8_t unk3;
	int xPos; // 0x4
	int yPos; // 0x8
	int x2; // 0xC
	int y2; // 0x10
	int32_t dxPos; // 0x18
	int32_t dyPos; // 0x1C
	ShootLvlObjectData *nextPtr; // next pointer to 'free' element
};

struct ScreenMask { // ShadowScreenMask
	uint32_t dataSize;
	uint8_t *projectionDataPtr;
	uint8_t *shadowPalettePtr;
	int16_t x;
	int16_t y;
	uint16_t w;
	uint16_t h;
}; // sizeof == 0x14

struct MstTaskData;

struct MovingOpcodeState {
	int32_t xPos;
	int32_t yPos; // 4
	BoundingBox boundingBox; // 8
	int32_t unk18;
	int32_t unk1C;
	int32_t unk20;
	int32_t unk24;
	ShootLvlObjectData *unk28;
	LvlObject *o; // 0x2C
	MstTaskData *m; // 0x30
	int32_t unk3C;
	uint8_t unk40;
	uint8_t unk41;
}; // sizeof == 0x44

struct AndyMoveData {
	int32_t xPos;
	int32_t yPos;
	uint16_t anim; // 8
	uint16_t unkA;
	uint16_t unkC;
	uint16_t unkE;
	uint8_t frame; // 0x10
	uint8_t unk11;
	uint16_t flags0;
	uint16_t flags1;
	uint16_t unk16;
	uint16_t unk18;
	uint16_t unk1A;
	const uint8_t *unk1C;
	const uint8_t *framesData;
	const uint8_t *unk24;
	const uint8_t *unk28;
}; // sizeof == 0x2C

struct MstRect {
	int x1; // 0
	int y1; // 4
	int x2; // 8
	int y2; // 12
	int num; // 16
}; // sizeof == 0x20

struct MstUnk46;

struct MstCollision {
	MstUnk46 *m46; // 0x00
	int32_t unk80; // 0x80
}; // sizeof == 132

struct Task;

struct MstUnk35;
struct MstUnk46Unk1;
struct MstUnk44Unk1;
struct MstUnk48Unk12Unk4;
struct MstUnk49;

struct MstTaskData { // MonsterObject
	MstUnk46 *m46;
	MstUnk46Unk1 *unk4;
	const uint8_t *unk8;
	MstUnk44Unk1 *unkC;
	LvlObject *o16; // 0x10
	LvlObject *o20; // 0x14
	MstUnk48Unk12Unk4 *unk18;
	MovingOpcodeState *unk1C; // 0x1C
	int soundType; // 0x20
	int executeCounter; // 0x24
	int32_t localVars[8]; // 0x28
	uint8_t flags48; // 0x48 0x4:indexUnk51!=kNone, 0x10:indexUnk47!=kNone
	uint8_t flags49; // 0x49
	uint8_t flags4A; // 0x4A
	uint8_t flags4B; // 0x4B
	int xPos; // 0x4C
	int yPos; // 0x50
	int xMstPos; // 0x54
	int yMstPos; // 0x58
	int xDelta; // 0x5C
	int yDelta; // 0x60
	int unk64; // 0x64
	int unk68; // 0x68
	int unk74; // 0x74
	int unk7C; // 0x7C
	int unk84; // 0x84
	int unk88; // 0x88
	int unk8C; // 0x8C
	int unk90; // 0x90
	int x1; // 0x94
	int x2; // 0x98
	int y1; // 0x9C
	int y2; // 0xA0
	uint8_t flagsA4; // 0xA4
	uint8_t flagsA5; // 0xA5
	uint8_t flagsA6; // 0xA6
	uint8_t flagsA7; // 0xA7
	uint8_t flagsA8[4]; // 0xA8, 0xA9, 0xAA, 0xAB
	int32_t unkBC;
	int32_t unkC0;
	Task *task; // 0xC4
	uint8_t unkC8[4]; // 0xC8
	uint8_t unkCC[4]; // 0xCC
	MstUnk35 *m35; // 0xD0
	const uint8_t *unkD4; // 0xD4, sizeof=16
	MstUnk49 *m49; // 0xD8
	int unkDC; // 0xDC
	uint8_t unkE4;
	uint8_t unkE6; // 0xE6 indexes _mstLut4
	uint8_t directionKeyMask;
	int unkE8; // 0xE8
	int unkEC; // 0xEC
	int unkF0; // 0xF0
	int unkF4; // 0xF4
	uint8_t unkF8; // 0xF8
	int unkFC; // 0xFC
}; // sizeof == 256

struct MstObject { // MonsterObject2
	void *unk0;
	LvlObject *o; // 4
	MstTaskData *mstTaskData; // 8
	int unk0x10;
	int xPos; // 14
	int yPos; // 18
	int xMstPos; // 1C
	int yMstPos; // 20
	uint8_t flags24; // 24
	BoundingBox boundingBox; // 28
	uint8_t flags38;
	uint8_t flags39;
	uint8_t flags3A;
	uint8_t flags3B;
}; // sizeof == 64

struct Task {
	const uint8_t *codeData;
	Task *prevPtr, *nextPtr; // 4,8
	MstTaskData *dataPtr; // 0xC monster
	MstObject *mstObject; // 0x10 monster2
	int32_t localVars[8]; // 0x14
	uint8_t flags; // 0x34
	uint8_t runningState; // 0x35
	int16_t delay; // 0x36
	uint32_t tempVar; // 0x38
	int (Game::*run)(Task *t); // 0x3C
	Task *child; // 0x40
}; // sizeof == 0x44

#endif // DEFS_H__
