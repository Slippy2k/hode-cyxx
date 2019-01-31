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
	kLvlAnimHdrOffset = 0x2C
};

struct Point8_t {
	int8_t x;
	int8_t y;
} PACKED;

struct Point16_t {
	int16_t x;
	int16_t y;
};

/* LvlBackgroundAnim */
struct GameRect {
	uint8_t *ptr; /* currentSpriteData */
	uint8_t *unk4; /* firstSpriteData */
	uint8_t *unk8; /* temporarySpriteData */
	uint16_t unkC; /* numFrames */
	uint16_t unkE; /* currentFrame */
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

struct LvlObject {
	int32_t xPos;
	int32_t yPos;
	uint8_t screenNum;
	uint8_t screenState;
	uint8_t flags;
	uint8_t frame;
	uint16_t anim;
	uint8_t type;
	uint8_t data0x2988; /* shootType */
	uint16_t flags0;
	uint16_t flags1;
	uint16_t flags2; /* spriteType */
	uint8_t stateValue;
	uint8_t stateCounter;
	LvlObject *linkObjPtr; // plasma cannon object in case of _andyObject
	uint16_t width;
	uint16_t height;
	uint8_t actionKeyMask;
	uint8_t directionKeyMask;
	uint16_t unk22; /* currentSprite */
	uint16_t soundToPlay;
	uint8_t unk26;
	uint8_t unk27;
	uint8_t *bitmapBits;
	int (Game::*callbackFuncPtr)(LvlObject *ptr);
	void *dataPtr;
// ptr->data0x2988==0:AndyObjectScreenData
// ptr->type==0: GameRect * / _gameRectsListTable
// ptr->type==1: _resLvlScreenBackgroundDataTable[num].dataUnk2Table[ptr->flags & 0xFF];
// ptr->type==8(_andyObject): &_gameUnkList1Element
	uint32_t unk34;
	LvlObjectData *levelData0x2988;
	Point16_t posTable[8];
	LvlObject *nextPtr;
};

struct SssFilter;

struct SssObject {
	uint8_t *soundBits; // 0x0
	uint16_t num; // 0x4
	uint16_t unk6; // 0x6
	int8_t unk8; // 0x8
	int8_t unk9; // 0x9
	uint8_t flags; // 0xA
	uint32_t flags0; // 0xC
	uint32_t flags1; // 0x10
	int32_t volume; // 0x14
	int32_t unk18; // 0x18
	int panL; // 0x1C
	int panR; // 0x20
	int panType; // 0x24
	int32_t unk28; // 0x28
	int32_t unk2C; // 0x2C
	SssObject *prevPtr; // 0x30
	SssObject *nextPtr; // 0x34
	const uint8_t *codeDataStage1; // 0x38
	const uint8_t *codeDataStage2; // 0x3C
	const uint8_t *codeDataStage3; // 0x40
	const uint8_t *codeDataStage4; // 0x44
	int32_t counter; // 0x48 counter0, counters[8]
	int32_t unk4C; // 0x4C counter1
	int32_t unk50; // 0x50 counter2
	int32_t unk54; // 0x54 counter3
	int32_t unk58; // 0x58 counter4
	int32_t unk64; // 0x64 counter5
	int32_t unk68; // 0x68 counter6
	int32_t unk6C; // 0x6C counter7
	int *volumePtr; // 0x70
	LvlObject *lvlObject; // 0x74
	int32_t unk78; // 0x78
	int32_t unk7C; // 0x7C
	SssFilter *filter;
};

struct MstScreenAreaCode {
	int32_t x1; // 0
	int32_t x2; // 4
	int32_t y1; // 8
	int32_t y2; // 0xC
	uint32_t next; // struct MstAreaCode* next; // 0x10, offset _resMstUnk38
	uint32_t prev; // 0x14, offset _resMstUnk38
	uint32_t unk0x18; // 0x18, offset _resMstUnk38
	uint8_t unk0x1C; // 0x1C
	uint8_t unk0x1D; // 0x1D
	uint16_t unk0x1E;
	uint32_t codeData; // const uint8_t *codeData; // 0x20, offset _resMstUnk62
} PACKED;

#define SIZEOF_MstScreenAreaCode 36

struct MstUnk42 {
	uint32_t offset1; //
	uint32_t count1; // 4
	uint32_t offset2; // 8
	uint32_t count2; // 12
} PACKED;

#define SIZEOF_MstUnk42 16

struct MstUnk43 {
	uint32_t offset1; // indexes _resMstUnk48
	uint32_t count1; // 4
	uint32_t offset2; // 8
	uint32_t count2; // 12
} PACKED;

#define SIZEOF_MstUnk43 16

struct MstUnk46 {
	uint32_t offset;
	uint32_t count;
} PACKED;

#define SIZEOF_MstUnk46 8

struct MstUnk47 {
	uint32_t offset;
	uint32_t count;
} PACKED;

#define SIZEOF_MstUnk47 8

struct MstUnk48 {
	uint32_t unk0;
	uint8_t unk4;
	uint8_t unk5;
	uint8_t unk6;
	uint8_t unk7;
	uint32_t codeData; // 0x8, PTR_OFFS<uint32>(_resMstUnk62, N)
	uint32_t offsetUnk12; // MstUnk48Unk12 *
	uint32_t countUnk12; // 0x10
	uint32_t offsets1[2]; // 0x14, 0x18
	uint32_t offsets2[2]; // 0x1C, 0x20
	uint32_t count[2]; // 36,40
} PACKED;

#define SIZEOF_MstUnk48 44

struct MstUnk48Unk12 {
	uint32_t unk0;
	uint32_t offset; // sizeof == 28
	uint32_t count;
} PACKED;

struct MstUnk49 {
	uint8_t pad0[8];
	uint32_t count1; // 8
	uint32_t offset1;
	uint32_t count2; // 16
	uint32_t unk0x14;
} PACKED;

#define SIZEOF_MstUnk49 24

struct MstUnk50 {
	uint32_t offset;
	uint32_t count;
} PACKED;

#define SIZEOF_MstUnk50 8

struct MstUnk51 {
	uint32_t unk0;
	uint32_t offset;
	uint32_t count;
} PACKED;

#define SIZEOF_MstUnk51 12

struct MstScreenInitCode {
	int32_t delay;
	uint32_t codeData;
} PACKED;

#define SIZEOF_MstScreenInitCode 8

struct MstUnk59 {
	uint32_t unk0;
	uint32_t codeIndex;
} PACKED;

#define SIZEOF_MstUnk59 8

struct MstUnk53 {
	int16_t index1;
	int16_t index2; // 2
	int16_t index3; // 4
	int16_t index4; // 6
	uint8_t unk8; // 8
	uint8_t unk9;
	int8_t index5; // 10
	int8_t unk11; // 11
	uint16_t unk12; // 12
	uint16_t unk14;
	uint32_t unk16; // 16
} PACKED;

#define SIZEOF_MstUnk53 20

struct MstUnk54 {
	int16_t unk0;
	int16_t unk2;
	uint8_t unk4;
	uint8_t unk5;
	uint16_t codeIndex;
} PACKED;

#define SIZEOF_MstUnk54 8

struct Sprite {
	int16_t xPos;
	int16_t yPos;
	uint8_t *bitmapBits;
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

struct AndyObjectScreenData {
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
	LvlObject *nextPtr; // 0x20
};

struct GameUnkList1 { // ObjectScreenData
	uint8_t unk0;
	uint8_t unk1;
	uint8_t unk2;
	uint8_t unk3;
	uint8_t unk4;
	uint8_t unk5;
	uint8_t unk6;
	uint8_t unk7;
	BoundingBox boundingBox; // 8
	int32_t dxPos;
	int32_t dyPos;
	GameUnkList1 *nextPtr;
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

struct MovingOpcodeState {
	int32_t xPos;
	int32_t yPos; // 4
	BoundingBox boundingBox; // 8
	int32_t unk0x18;
	int32_t unk0x1C;
	int32_t unk0x20;
	int32_t unk0x24;
	GameUnkList1 *unk0x28;
	LvlObject *unk0x2C;
	uint32_t unk0x30;
	uint32_t unk0x3C;
	uint8_t unk0x40;
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

#endif // DEFS_H__
