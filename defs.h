/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009 Gregory Montoir
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

struct Point8 {
	int8 x;
	int8 y;
} PACKED;

struct Point16 {
	int16 x;
	int16 y;
} PACKED;

/* LvlBackgroundAnim */
struct GameRect {
	uint8 *ptr; /* currentSpriteData */
	uint8 *unk4; /* firstSpriteData */
	uint8 *unk8; /* temporarySpriteData */
	uint16 unkC; /* numFrames */
	uint16 unkE; /* currentFrame */
};

struct LvlAnimSeqHeader {
	uint16 firstFrame;
	uint16 unk2;
	int8 dx; // 4
	int8 dy; // 5
	uint8 count; // 6
	uint8 unk7; // 7
	uint16 sound;
	uint16 flags0;
	uint16 flags1;
	uint16 unkE;
	uint32 offset; // 0x10, LvlAnimSeqFrameHeader
} PACKED;

#define SIZEOF_LvlAnimSeqHeader 20

struct LvlAnimSeqFrameHeader {
	uint16 move; // 0
	uint16 anim; // 2
	uint8 frame; // 4
	uint8 unk5; // 5
	int8 unk6;
	int8 unk7;
} PACKED;

#define SIZEOF_LvlAnimSeqFrameHeader 8

struct LvlAnimHeader {
	uint16 unk0;
	uint8 seqCount;
	uint8 unk2;
	uint32 seqOffset;
} PACKED;

#define SIZEOF_LvlAnimHeader 8

struct LvlSprMoveData {
	uint8 op1;
	uint8 op2;
	uint16 op3;
	uint16 op4;
	uint16 unk0x6;
} PACKED;

#define SIZEOF_LvlSprMoveData 8

struct LvlSprHotspotData {
	Point8 pts[8];
} PACKED;

#define SIZEOF_LvlSprHotspotData 16

struct LvlObjectData {
	uint8 unk0;
	uint8 index;
	uint16 framesCount;
	uint16 hotspotsCount;
	uint16 movesCount;
	uint16 animsCount;
	uint8 refCount; // 0xA
	uint8 frame; // 0xB
	uint16 anim; // 0xC
	uint8 unkE;
	uint8 unkF;
	uint8 *animsInfoData; // 0x10, LevelSprAnimInfo
	uint8 *movesData; // 0x14, LvlSprMoveData
	uint8 *framesData; // 0x18
/*	uint32 framesDataOffset; */ // 0x1C, not needed
	uint8 *animsData; // 0x20
	uint8 *coordsData; // 0x24
	uint8 *hotspotsData; // 0x28, LvlSprHotspotData
};

struct Game;

struct LvlObject {
	int32 xPos;
	int32 yPos;
	uint8 data0x2E08;
	uint8 screenState;
	uint8 flags;
	uint8 frame;
	uint16 anim;
	uint8 type;
	uint8 data0x2988;
	uint16 flags0;
	uint16 flags1;
	uint16 flags2; /* spriteType */
	uint8 stateValue;
	uint8 stateCounter;
	LvlObject *linkObjPtr; // plasma cannon object in case of _andyObject
	uint16 width;
	uint16 height;
	uint8 actionKeyMask;
	uint8 directionKeyMask;
	uint16 unk22; /* currentSprite */
	uint16 soundToPlay;
	uint8 unk26;
	uint8 unk27;
	uint8 *bitmapBits;
	int (Game::*callbackFuncPtr)(LvlObject *ptr);
	void *dataPtr;
// ptr->data0x2988==0:AndyObjectScreenData
// ptr->type==0: GameRect * / _gameRectsListTable
// ptr->type==1: _resLvlScreenBackgroundDataTable[num].dataUnk2Table[ptr->flags & 0xFF];
// ptr->type==8(_andyObject): &_gameUnkList1Element
	uint32 unk34;
	LvlObjectData *levelData0x2988;
	Point16 posTable[8];
	LvlObject *nextPtr;
};

struct LvlBackgroundData {
	uint8 backgroundCount;
	uint8 currentBackgroundId;
	uint8 dataUnk0Count;
	uint8 unk3; /* movingMask */
	uint8 dataUnk1Count;
	uint8 currentDataUnk1Id;
	uint8 dataUnk2Count;
	uint8 unk7;
	uint8 dataUnk3Count;
	uint8 unk9;
	uint8 dataUnk45Count;
	uint8 unkB;
	uint16 backgroundPaletteId;
	uint16 backgroundBitmapId;
	uint8 *backgroundPaletteTable[4];
	uint8 *backgroundBitmapTable[4];
	uint8 *dataUnk0Table[4];
	uint8 *backgroundMaskTable[4];
	uint8 *dataUnk2Table[4];
	uint8 *backgroundAnimationTable[4];
	uint8 *dataUnk4Table[4]; /* unused ? */
	LvlObjectData *dataUnk5Table[4]; /* LevelData0x2988 */
	uint8 *dataUnk6Table[4]; /* unused ? */
};

struct MstScreenAreaCode {
	int32 x1; // 0
	int32 x2; // 4
	int32 y1; // 8
	int32 y2; // 0xC
	uint32 next; // struct MstAreaCode* next; // 0x10, offset _resMstUnk38
	uint32 prev; // 0x14, offset _resMstUnk38
	uint32 unk0x18; // 0x18, offset _resMstUnk38
	uint8 unk0x1C; // 0x1C
	uint8 unk0x1D; // 0x1D
	uint16 unk0x1E;
	uint32 codeData; // const uint8 *codeData; // 0x20, offset _resMstUnk62
} PACKED;

#define SIZEOF_MstScreenAreaCode 36

struct MstUnk42 {
	uint32 offset1; //
	uint32 count1; // 4
	uint32 offset2; // 8
	uint32 count2; // 12
} PACKED;

#define SIZEOF_MstUnk42 16

struct MstUnk43 {
	uint32 offset1; // indexes _resMstUnk48
	uint32 count1; // 4
	uint32 offset2; // 8
	uint32 count2; // 12
} PACKED;

#define SIZEOF_MstUnk43 16

struct MstUnk46 {
	uint32 offset;
	uint32 count;
} PACKED;

#define SIZEOF_MstUnk46 8

struct MstUnk47 {
	uint32 offset;
	uint32 count;
} PACKED;

#define SIZEOF_MstUnk47 8

struct MstUnk48 {
	uint32 unk0;
	uint8 unk4;
	uint8 unk5;
	uint8 unk6;
	uint8 unk7;
	uint32 codeData; // 0x8, PTR_OFFS<uint32>(_resMstUnk62, N)
	uint32 offsetUnk12; // MstUnk48Unk12 *
	uint32 countUnk12; // 0x10
	uint32 offsets1[2]; // 0x14, 0x18
	uint32 offsets2[2]; // 0x1C, 0x20
	uint32 count[2]; // 36,40
} PACKED;

#define SIZEOF_MstUnk48 44

struct MstUnk48Unk12 {
	uint32 unk0;
	uint32 offset; // sizeof == 28
	uint32 count;
} PACKED;

struct MstUnk49 {
	uint8 pad0[8];
	uint32 count1; // 8
	uint32 offset1;
	uint32 count2; // 16
	uint32 unk0x14;
} PACKED;

#define SIZEOF_MstUnk49 24

struct MstUnk50 {
	uint32 offset;
	uint32 count;
} PACKED;

#define SIZEOF_MstUnk50 8

struct MstUnk51 {
	uint32 unk0;
	uint32 offset;
	uint32 count;
} PACKED;

#define SIZEOF_MstUnk51 12

struct MstScreenInitCode {
	int32 delay;
	uint32 codeData;
} PACKED;

#define SIZEOF_MstScreenInitCode 8

struct MstUnk59 {
	uint32 unk0;
	uint32 codeIndex;
} PACKED;

#define SIZEOF_MstUnk59 8

struct MstUnk53 {
	int16 index1;
	int16 index2; // 2
	int16 index3; // 4
	int16 index4; // 6
	uint8 unk8; // 8
	uint8 unk9;
	int8 index5; // 10
	int8 unk11; // 11
	uint16 unk12; // 12
	uint16 unk14;
	uint32 unk16; // 16
} PACKED;

#define SIZEOF_MstUnk53 20

struct MstUnk54 {
	int16 unk0;
	int16 unk2;
	uint8 unk4;
	uint8 unk5;
	uint16 codeIndex;
} PACKED;

#define SIZEOF_MstUnk54 8

struct Sprite {
	int16 xPos;
	int16 yPos;
	uint8 *bitmapBits;
	Sprite *nextPtr;
	uint16 num;
	uint16 flags;
};

struct LvlScreenVector {
	int32 u;
	int32 v;
};

struct LvlScreenState {
	uint8 s0;
	uint8 s1;
	uint8 s2;
	uint8 s3;
};

struct BoundingBox {
	int32 x1; // 0
	int32 y1; // 4
	int32 x2; // 8
	int32 y2; // C
};

struct AndyObjectScreenData {
	uint8 unk0;
	uint8 unk1;
	uint8 unk2;
	uint8 unk3;
	uint8 unk4;
	uint8 unk5;
	uint16 unk6;
	BoundingBox boundingBox; // 0x8
	int32 dxPos; // 0x18
	int32 dyPos; // 0x1C
	LvlObject *nextPtr; // 0x20
};

struct GameUnkList1 { // ObjectScreenData
	uint8 unk0;
	uint8 unk1;
	uint8 unk2;
	uint8 unk3;
	uint8 unk4;
	uint8 unk5;
	uint8 unk6;
	uint8 unk7;
	BoundingBox boundingBox; // 8
	int32 dxPos;
	int32 dyPos;
	GameUnkList1 *nextPtr;
};

struct ScreenMask { // ShadowScreenMask
	uint32 dataSize;
	uint8 *projectionDataPtr;
	uint8 *shadowPalettePtr;
	int16 x;
	int16 y;
	uint16 w;
	uint16 h;
}; // sizeof == 0x14

struct MovingOpcodeState {
	int32 xPos;
	int32 yPos; // 4
	BoundingBox boundingBox; // 8
	int32 unk0x18;
	int32 unk0x1C;
	int32 unk0x20;
	int32 unk0x24;
	GameUnkList1 *unk0x28;
	LvlObject *unk0x2C;
	uint32 unk0x30;
	uint32 unk0x3C;
	uint8 unk0x40;
}; // sizeof == 0x44

struct AndyMoveData {
	int32 xPos;
	int32 yPos;
	uint16 anim; // 8
	uint16 unkA;
	uint16 unkC;
	uint16 unkE;
	uint8 frame; // 0x10
	uint8 unk11;
	uint16 flags0;
	uint16 flags1;
	uint16 unk16;
	uint16 unk18;
	uint16 unk1A;
	const uint8 *unk1C;
	const uint8 *framesData;
	const uint8 *unk24;
	const uint8 *unk28;
}; // sizeof == 0x2C

#endif // DEFS_H__
