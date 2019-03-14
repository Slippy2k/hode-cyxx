/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
 */

#ifndef RESOURCE_H__
#define RESOURCE_H__

#include "fs.h"
#include "intern.h"

struct DatHdr {
	uint32_t sssOffset; // 0xC
	int yesNoQuitImage; // 0x40
	int loadingImageSize; // 0x48
	uint32_t hintsImageOffsetTable[46];
	uint32_t hintsImageSizeTable[46];
};

struct LvlHdr {
	uint8_t screensCount;
	uint8_t staticLvlObjectsCount;
	uint8_t otherLvlObjectsCount;
	uint8_t spritesCount;
};

struct LvlScreenVector {
	int32_t u;
	int32_t v;
};

struct LvlScreenState {
	uint8_t s0;
	uint8_t s1;
	uint8_t s2;
	uint8_t s3; // maskData
};

struct LvlBackgroundData {
	uint8_t backgroundCount;
	uint8_t currentBackgroundId;
	uint8_t maskCount;
	uint8_t currentMaskId;
	uint8_t dataUnk1Count; /* shadowCount */
	uint8_t currentDataUnk1Id; /* currentShadowId */
	uint8_t dataUnk2Count; /* soundCount */
	uint8_t unk7; /* currentSoundId */
	uint8_t dataUnk3Count;
	uint8_t unk9;
	uint8_t dataUnk45Count;
	uint8_t unkB;
	uint16_t backgroundPaletteId;
	uint16_t backgroundBitmapId;
	uint8_t *backgroundPaletteTable[4];
	uint8_t *backgroundBitmapTable[4];
	uint8_t *dataUnk0Table[4];
	uint8_t *backgroundMaskTable[4];
	uint8_t *backgroundSoundTable[4];
	uint8_t *backgroundAnimationTable[4];
	uint8_t *dataUnk4Table[4]; /* unused ? */
	LvlObjectData *dataUnk5Table[4];
	uint8_t *dataUnk6Table[4]; /* unused ? */
};

struct MstHdr {
	int version;
	int dataSize;
	int unk0x08;
	int unk0x0C;
	int unk0x10;
	int unk0x14;
	int screenAreaCodesCount;
	int unk0x1C;
	int unk0x20;
	int unk0x24;
	int unk0x28;
	int unk0x2C;
	int unk0x30;
	int unk0x34;
	int unk0x38;
	int unk0x3C;
	int unk0x40;
	int unk0x44;
	int unk0x48;
	int unk0x4C;
	int unk0x50;
	int unk0x54;
	int unk0x58;
	int unk0x5C;
	int unk0x60;
	int unk0x64;
	int unk0x68;
	int unk0x6C;
	int unk0x70;
	int unk0x74;
	int unk0x78;
	int codeSize; // sizeof(uint32_t)
	int pointsCount;
};

struct MstPointOffset {
	int32_t xOffset;
	int32_t yOffset;
}; // sizeof == 8

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
	uint32_t codeData; // const uint8_t *codeData; // 0x20, offset _mstCodeData
}; // SIZEOF_MstScreenAreaCode 36

struct MstUnk35 {
	uint32_t ptr;
	uint32_t count;
	uint32_t unk8;
	uint32_t size;
}; // sizeof == 16

struct MstUnk42 {
	uint32_t offset1; //
	uint32_t count1; // 4
	uint32_t offset2; // 8
	uint32_t count2; // 12
}; // SIZEOF_MstUnk42 16

struct MstUnk43 {
	uint32_t offset1; // indexes _resMstUnk48
	uint32_t count1; // 4
	uint32_t offset2; // 8
	uint32_t count2; // 12
}; // SIZEOF_MstUnk43 16

struct MstUnk44 {
	uint32_t unk0;
	uint32_t unk4;
	uint32_t unk8;
	uint32_t count;
}; // SIZEOF_MstUnk44 16

struct MstUnk46 {
	uint32_t offset;
	uint32_t count;
}; // SIZEOF_MstUnk46 8

struct MstUnk47 {
	uint32_t offset;
	uint32_t count;
}; // SIZEOF_MstUnk47 8

struct MstUnk48 {
	uint32_t unk0;
	uint8_t unk4;
	uint8_t unk5;
	uint8_t unk6;
	uint8_t unk7;
	uint32_t codeData; // 0x8, PTR_OFFS<uint32>(_mstCodeData, N)
	uint32_t offsetUnk12; // MstUnk48Unk12 *
	int countUnk12; // 0x10
	uint32_t offsets1[2]; // 0x14, 0x18
	uint32_t offsets2[2]; // 0x1C, 0x20
	uint32_t count[2]; // 36,40
}; // SIZEOF_MstUnk48 44

struct MstUnk48Unk12 {
	uint32_t unk0;
	uint32_t offset; // sizeof == 28
	uint32_t count;
}; // sizeof == 12

struct MstUnk49 {
	uint8_t pad0[8];
	uint32_t count1; // 8
	uint32_t offset1;
	uint32_t count2; // 16
	uint32_t unk0x14;
}; // SIZEOF_MstUnk49 24

struct MstUnk50 {
	uint32_t offset;
	uint32_t count;
}; // SIZEOF_MstUnk50 8

struct MstUnk51 {
	uint32_t unk0;
	uint32_t offset;
	uint32_t count;
}; // SIZEOF_MstUnk51 12

struct MstScreenInitCode {
	int32_t delay;
	uint32_t codeData;
}; // SIZEOF_MstScreenInitCode 8

struct MstUnk59 {
	uint32_t taskId;
	uint32_t codeData;
}; // SIZEOF_MstUnk59 8

struct MstUnk53 { // MstMonsterInfo
	int16_t indexVar1;
	int16_t indexVar2; // 2
	int16_t indexVar3; // 4
	int16_t indexVar4; // 6
	uint8_t unk8; // 8
	uint8_t unk9;
	int8_t indexVar5; // 10
	int8_t unk11; // 11
	uint16_t unk12; // 12
	uint16_t unk14;
	uint32_t unk16; // 16
}; // SIZEOF_MstUnk53 20

struct MstUnk54 {
	int16_t unk0;
	int16_t unk2;
	uint8_t unk4;
	uint8_t unk5;
	uint16_t codeData;
}; // SIZEOF_MstUnk54 8

struct MstOp56Data {
	uint32_t unk0;
	uint32_t unk4;
	uint32_t unk8;
	uint32_t unkC;
}; // sizeof == 16

struct SssHdr {
	int version;
	int unk4;
	int preloadPcmCount;
	int preloadInfoCount;
	int dataUnk1Count;
	int dataUnk2Count;
	int dataUnk3Count;
	int codeOffsetsCount;
	int codeSize;
	int preloadData1Count; // 24
	int preloadData2Count; // 28
	int preloadData3Count; // 2C
	int pcmCount; // 30
};

struct SssUnk1 {
	uint16_t sssUnk3; // 0 index to _sssDataUnk3
	uint8_t unk2; // 2
	uint8_t unk3;
	uint8_t unk4; // 4
	uint8_t unk5;
	uint8_t unk6;
	uint8_t unk7;
};

struct SssUnk2 { // SssProperty
	uint8_t unk0; // defaultPriority
	int8_t unk1; // defaultVolume
	int8_t unk2;
};

struct SssUnk3 { // SssInfo
	uint8_t flags; // 0 flags0
	int8_t count; // 1 codeOffsetCount
	uint16_t sssFilter; // 2 index to _sssFilters
	uint32_t firstCodeOffset; // 4 offset to _sssCodeOffsets
};

struct SssCodeOffset {
	uint16_t pcm; // index to _sssPcmTable
	uint16_t unk2;
	uint8_t unk4;
	uint8_t unk5;
	int8_t unk6;
	uint8_t unk7;
	uint32_t codeOffset1; // 0x8 offset to _sssCodeData
	uint32_t codeOffset2; // 0xC offset to _sssCodeData
	uint32_t codeOffset3; // 0x10 offset to _sssCodeData
	uint32_t codeOffset4; // 0x14 offset to _sssCodeData
};

struct SssUnk4 { // SssPreloadInfo
	uint32_t count;
	uint8_t *data;
};

struct SssFilter {
	int32_t unk0;
	int32_t unk4; // priority (0,7)
	int32_t unk8; // priorityDelta
	int32_t unkC; // priorityCounter
	int32_t unk10;
	int32_t unk14; // volume (0,127)
	int32_t unk18; // volumeDelta
	int32_t unk1C; // volumeCounter
	int32_t unk20;
	int32_t unk24;
	int32_t unk30; // flag
};

#define SIZEOF_SssFilter 52

struct SssPcm {
	int16_t *ptr;    // 0 PCM data
	uint32_t offset; // 4 offset in .sss
	uint32_t totalSize;   // 8 size in .sss (256 int16_t words + followed by indexes)
	uint32_t strideSize;  // 12
	uint16_t strideCount; // 16
	uint16_t flag;        // 18
};

struct SssUnk6 {
	uint32_t unk0[4]; // 0
	uint32_t unk10; // 10
};

struct SssPreloadData {
	uint8_t count;
	uint8_t *ptr;
};

struct Resource {

	FileSystem _fs;

	DatHdr _datHdr;
	File *_datFile;
	LvlHdr _lvlHdr;
	File *_lvlFile;
	MstHdr _mstHdr;
	File *_mstFile;
	SssHdr _sssHdr;
	File *_sssFile;

	uint8_t *_loadingImageBuffer;

	uint8_t _currentScreenResourceNum;

	uint8_t _screensGrid[40 * 4];
	LvlScreenVector _screensBasePos[40];
	LvlScreenState _screensState[40];
	uint8_t *_resLevelData0x470CTable;
	uint8_t *_resLevelData0x470CTablePtrHdr;
	uint8_t *_resLevelData0x470CTablePtrData;
	uint8_t *_resLvlScreenSpriteDataPtrTable[32];
	uint32_t _resLevelData0x2B88SizeTable[40];
	uint32_t _resLevelData0x2988SizeTable[32];
	LvlObjectData _resLevelData0x2988Table[40];
	LvlObjectData *_resLevelData0x2988PtrTable[32];
	LvlBackgroundData _resLvlScreenBackgroundDataTable[40];
	uint8_t *_resLvlScreenBackgroundDataPtrTable[40];

	LvlObject _resLvlScreenObjectDataTable[104];
	LvlObject _dummyObject; // (LvlObject *)0xFFFFFFFF

	SssUnk1 *_sssDataUnk1;
	SssUnk2 *_sssDataUnk2;
	SssUnk3 *_sssDataUnk3;
	SssCodeOffset *_sssCodeOffsets;
	SssUnk4 *_sssDataUnk4;
	SssFilter *_sssFilters;
	SssPcm *_sssPcmTable;
	SssUnk6 *_sssDataUnk6;
	SssPreloadData *_sssPreloadData1;
	SssPreloadData *_sssPreloadData2;
	SssPreloadData *_sssPreloadData3;
	uint32_t *_sssLookupTable1[3];
	uint32_t *_sssLookupTable2[3];
	uint32_t *_sssLookupTable3[3];
	uint8_t *_sssCodeData;
	uint32_t _sssPreloadedPcmTotalSize;

	MstPointOffset *_mstPointOffsets;
	MstUnk35 *_mstUnk35;
	uint32_t _mstTickDelay;
	uint32_t _mstTickCodeData;
	uint32_t *_mstScreenInitCodeData;
	MstScreenAreaCode *_mstScreenAreaCodes;
	uint32_t *_mstUnk39; // index to _mstScreenAreaCodes
	uint32_t *_mstUnk40; // index to _mstScreenAreaCodes
	uint32_t *_mstUnk41;
	MstUnk42 *_mstUnk42;
	MstUnk43 *_mstUnk43;
	MstUnk44 *_mstUnk44;
	MstUnk46 *_mstUnk46;
	MstUnk47 *_mstUnk47;
	MstUnk48 *_mstUnk48;
	uint8_t *_mstHeightMapData;
	MstUnk49 *_mstUnk49;
	MstUnk50 *_mstUnk50;
	MstUnk51 *_mstUnk51;
	MstUnk53 *_mstUnk53;
	MstUnk54 *_mstUnk54;
	MstUnk59 *_mstUnk59;
	uint32_t *_mstUnk60; // index to _mstCodeData
	MstOp56Data *_mstOp56Data;
	uint8_t *_mstCodeData;

	bool _isDemoData;

	Resource(const char *dataPath);

	bool detectGameData();

	void loadLevelData(const char *levelName);

	void loadSetupDat();
	void loadLvlScreenMoveData(int num);
	void loadLvlScreenVectorData(int num);
	void loadLvlScreenStateData(int num);
	void loadLvlScreenObjectData(int num);
	void loadLvlData(File *fp);
	void loadLvlSpriteData(int num);

	uint8_t *getLevelData0x470CPtr0(int num); // getLvlScreenMaskDataPtr
	uint8_t *getLevelData0x470CPtr4(int num); // getLvlScreenPosDataPtr
	void loadLevelData0x470C();

	void loadLvlScreenBackgroundData(int num);
	void unloadLvlScreenBackgroundData(int num);

	bool isLevelData0x2988Loaded(int num);
	bool isLevelData0x2B88Loaded(int num);

	void incLevelData0x2988RefCounter(LvlObject *ptr);
	void decLevelData0x2988RefCounter(LvlObject *ptr);

	void loadHintImage(int num, uint8_t *dst, uint8_t *pal);
	void loadLoadingImage(uint8_t *dst, uint8_t *pal);

	const uint8_t *getLvlSpriteFramePtr(LvlObjectData *dat, int frame);
	const uint8_t *getLvlSpriteCoordPtr(LvlObjectData *dat, int num);

	void loadSssData(File *fp);
	void checkSssCode(const uint8_t *buf, int size);
	void loadSssPcm(File *fp, int num);
	uint32_t getSssPcmSize(SssPcm *pcm) const;

	uint32_t *getSssLutPtr(int lut, uint32_t flags) {
		const uint32_t a = (flags >> 20) & 0xF;
		assert(a < 3);
		const uint32_t b = flags & 0xFFF;
		switch (lut) {
		case 1:
			return _sssLookupTable1[a] + b;
		case 2:
			return _sssLookupTable2[a] + b;
		case 3:
			return _sssLookupTable3[a] + b;
		}
		assert(0);
		return 0;
	}

	void clearSssLookupTable3();

	void loadMstData(File *fp);

	MstScreenAreaCode *findMstCodeForPos(int num, int xPos, int yPos);
	void flagMstCodeForPos(int num, uint8_t value);
};

#endif // RESOURCE_H__

