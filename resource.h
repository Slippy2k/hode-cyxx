/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
 */

#ifndef RESOURCE_H__
#define RESOURCE_H__

#include "fs.h"
#include "intern.h"

struct DatHdr {
	uint32_t version; // 0x0
	uint32_t bufferSize0; // 0x4
	uint32_t bufferSize1; // 0x8
	uint32_t sssOffset; // 0xC
	uint32_t iconsCount; // 0x10
	uint32_t menusCount; // 0x14
	uint32_t cutscenesCount; // 0x18
	uint32_t levelsCount; // 0x1C
	uint32_t checkpointsLevel1Count; // 0x20
	uint32_t checkpointsLevel2Count; // 0x24
	uint32_t checkpointsLevel3Count; // 0x28
	uint32_t checkpointsLevel4Count; // 0x2C
	uint32_t checkpointsLevel5Count; // 0x30
	uint32_t checkpointsLevel6Count; // 0x34
	uint32_t checkpointsLevel7Count; // 0x38
	uint32_t checkpointsLevel8Count; // 0x3C
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
	uint8_t shadowCount;
	uint8_t currentDataUnk1Id; /* currentShadowId */
	uint8_t dataUnk2Count; /* soundCount */
	uint8_t currentSoundId;
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
	int pointsCount; // 0x80
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
	uint32_t codeData; // 0x20, offset _mstCodeData
}; // sizeof == 36

struct MstUnk34 {
	int32_t right; // 0
	int32_t left; //  4
	int32_t top; // 8
	int32_t bottom; // 0xC
	int32_t unk10; // 0x10
}; // sizeof == 20

struct MstUnk35 {
	uint32_t *indexCodeData; // 0, offset _mstCodeData
	uint32_t count1; // 4
	uint8_t *data2; // 8
	uint32_t count2; // C
}; // sizeof == 16

struct MstUnk36 {
	uint32_t indexUnk49; // indexes _mstUnk49
	uint32_t unk4; // indexes _mstUnk49.data1
	int32_t unk8;
}; // sizeof == 12

struct MstUnk42 { // MonsterGroup
	uint32_t *indexUnk46; // 0 indexes _mstUnk46
	uint32_t count1; // 4
	uint8_t *data2; // 8
	uint32_t count2; // C
}; // sizeof == 16

struct MstUnk43 {
	uint32_t *indexUnk48; // indexes _mstUnk48
	uint32_t count1; // 4
	uint8_t *data2; // 8
	uint32_t count2; // C
}; // sizeof == 16

struct MstUnk44Unk1 {
	int32_t x1; // 0
	int32_t x2; // 4
	int32_t y1; // 8
	int32_t y2; // 0xC
	uint32_t indexUnk34_16; // 0x10
	uint32_t indexUnk35_20; // 0x14
	uint32_t indexUnk35_24; // 0x18
	uint32_t indexUnk36_28; // 0x1C
	uint32_t indexUnk36_32; // 0x20
	uint32_t indexUnk35_0x24[2];
	int32_t unk2C[2]; // 0x2C
	int32_t unk34[2]; // 0x34
	int32_t unk3C[2]; // 0x3C
	int32_t unk44[2]; // 0x44
	uint32_t indexUnk44Unk1_76[4]; // next per direction 0x4C
	uint32_t indexUnk44Unk1_92; // next 0x5C
	uint8_t *unk60[2]; // 0x60
}; // sizeof == 104

struct MstUnk44 {
	MstUnk44Unk1 *data;
	uint32_t *indexUnk44Unk1; // indexed by screen number
	uint32_t mask; // 0x8
	int count; // 0xC
}; // sizeof == 16

struct MstUnk45 {
	uint8_t unk0;
	uint8_t unk1;
	uint16_t unk2;
	uint32_t codeData; // 4
	uint32_t unk8;
}; // sizeof == 12

struct MstUnk46Unk1 { // MonsterBehaviour
	uint32_t indexHeight; // 0, indexes mstHeightMapData
	uint16_t anim; // 4
	uint16_t unk6; // 6
	uint32_t unk8; // 0x8
	uint32_t energy; // 0xC
	uint32_t unk10; // 0x10
	uint32_t count; // 0x14
	uint32_t unk18; // 0x18
	uint32_t indexUnk51; // 0x1C indexes _mstUnk51
	uint32_t indexUnk44; // 0x20 indexes _mstUnk44
	uint32_t indexUnk47; // 0x24 indexes _mstUnk47
	uint32_t codeData; // 0x28 indexes _mstCodeData
}; // sizeof == 44

struct MstUnk46 {
	MstUnk46Unk1 *data;
	uint32_t count;
}; // sizeof == 8

struct MstUnk47 {
	uint8_t *data; // sizeof == 20
	uint32_t count;
}; // sizeof == 8

struct MstUnk48Unk12Unk4 {
	uint32_t unk0; // 0x0, indexes _mstHeightMapData
	uint32_t indexUnk51; // 0x4
	int32_t unk8; // 0x8 xPos
	int32_t unkC; // 0xC yPos
	uint32_t codeData; // 0x10
	uint32_t codeData2; // 0x14
	uint8_t unk18; // 0x18
	uint8_t unk19; // 0x19
	int8_t screenNum; // 0x1A
	uint8_t monster1Index; // 0x1B
}; // sizeof == 28

struct MstUnk48Unk12 {
	uint8_t unk0;
	MstUnk48Unk12Unk4 *data; // 0x4 sizeof == 28
	uint32_t count;
}; // sizeof == 12

struct MstUnk48 {
	uint16_t unk0;
	uint16_t unk2;
	uint8_t unk4;
	uint8_t unk5;
	uint8_t unk6;
	uint8_t unk7;
	uint32_t codeData; // 0x8, PTR_OFFS<uint32>(_mstCodeData, N)
	MstUnk48Unk12 *unk12; // 0xC
	int countUnk12; // 0x10
	uint32_t *data1[2]; // 0x14, 0x18
	uint32_t *data2[2]; // 0x1C, 0x20
	uint32_t count[2]; // 0x24, 0x28
}; // sizeof == 44

struct MstUnk49Unk1 {
	uint32_t offsetHeight; // 0, offset _mstHeightMapData[32]
	uint32_t unk4;
	uint8_t unk8;
	uint8_t unk9;
	uint8_t unkA;
	uint8_t unkB;
	uint8_t unkC;
	uint8_t unkD;
	uint8_t unkE;
	uint8_t unkF;
}; // sizeof == 16

struct MstUnk49 {
	uint32_t indexHeight; // 0, indexes _mstHeightMapData
	MstUnk49Unk1 *data1; // 0x4, sizeof == 16
	uint32_t count1; // 0x8
	uint8_t *data2; // 0xC
	uint32_t count2; // 0x10
	uint8_t unk14; // 0x14
	uint8_t unk15; // 0x15
	uint8_t unk16; // 0x16
	uint8_t unk17; // 0x17
}; // sizeof == 24

struct MstUnk50Unk1 {
	uint32_t codeData;
	uint32_t unk4;
	uint32_t unk8;
	uint32_t unkC;
	uint32_t unk10;
	uint32_t unk14;
	uint32_t unk18;
	uint32_t unk1C;
	uint32_t unk20;
	int32_t unk24;
}; // sizeof == 40

struct MstUnk50 {
	MstUnk50Unk1 *data;
	uint32_t count;
}; // sizeof == 8

struct MstUnk51 {
	uint32_t indexUnk50;
	uint32_t *indexUnk50Unk1; // sizeof == 36
	uint32_t count;
}; // sizeof == 12

struct MstScreenInitCode {
	int32_t delay;
	uint32_t codeData;
}; // sizeof == 8

struct MstUnk59 { // MstOp240Data
	uint32_t flags;
	uint32_t codeData;
}; // sizeof == 8

struct MstUnk53 { // MstOp223Data
	int16_t indexVar1;
	int16_t indexVar2; // 2
	int16_t indexVar3; // 4
	int16_t indexVar4; // 6
	uint8_t unk8; // 8
	uint8_t unk9;
	int8_t indexVar5; // A
	int8_t unkB; // B
	uint16_t unkC; // C
	uint16_t unkE; // E
	uint32_t maskVars; // 0x10
}; // sizeof == 20

struct MstUnk54 { // MstOpCompareData
	int16_t indexVar1; // 0
	int16_t indexVar2; // 2
	uint8_t compare; // 4
	uint8_t maskVars; // 5
	uint16_t codeData; // 6
}; // sizeof == 8

struct MstUnk55 { // MstOp234Data
	int16_t indexVar1; // 0
	int16_t indexVar2; // 2
	uint8_t compare; // 4
	uint8_t maskVars; // 5
	uint32_t codeData;
}; // sizeof == 8

struct MstUnk56 {
	int32_t indexVar1; // 0
	int32_t indexVar2; // 4
	uint8_t maskVars; // 8
	uint8_t unk9;
	uint8_t unkA;
	uint8_t unkB;
}; // sizeof == 12

struct MstUnk63 {
	uint8_t unk0;
	uint8_t unk1;
	uint8_t unk2;
	uint8_t unk3;
	uint8_t unk4;
	uint8_t unk5;
	uint8_t unk6;
	uint8_t unk7;
}; // sizeof == 8

struct MstOp56Data {
	uint32_t unk0; // arg0 // 0
	uint32_t unk4; // arg1 // 4
	uint32_t unk8; // arg2 // 8
	uint32_t unkC; // arg3 // C
}; // sizeof == 16

struct MstOp49Data {
	uint16_t unk0;
	uint16_t unk2;
	uint16_t unk4;
	uint16_t unk6;
	uint32_t unk8; // maskVars
	uint16_t unkC; // indexes _mstUnk49
	int8_t unkE;
	int8_t unkF;
}; // sizeof == 16

struct MstOp58Data {
	uint16_t indexVar1; // 0
	uint16_t indexVar2; // 2
	uint16_t unk4; // 4
	int16_t unk6; // 6
	uint8_t unk8; // 8
	uint8_t unk9; // 9
	uint8_t unkA; // A
	uint8_t unkB; // B
	uint8_t unkC; // C
	uint8_t unkD; // D
	uint16_t unkE; // E
}; // sizeof == 16

struct SssHdr {
	int version;
	int unk4;
	int preloadPcmCount;
	int preloadInfoCount;
	int infosDataCount;
	int dataUnk2Count;
	int dataUnk3Count;
	int samplesDataCount;
	int codeSize;
	int preloadData1Count; // 24
	int preloadData2Count; // 28
	int preloadData3Count; // 2C
	int pcmCount; // 30
};

struct SssInfo {
	uint16_t sssBankIndex; // 0 indexes _sssBanksData
	int8_t unk2; // 2 codeOffsetNum
	int8_t unk3; // 3 filterDelta1
	int8_t unk4; // 4 filterDelta2
	int8_t unk5; // 5 filterDelta3
	uint8_t unk6; // 6 sound source mask
	uint8_t unk7;
};

struct SssUnk2 {
	uint8_t unk0; // defaultPriority
	int8_t unk1; // defaultVolume
	int8_t unk2;
};

struct SssBank {
	uint8_t flags; // 0 flags0
	int8_t count; // 1 codeOffsetCount
	uint16_t sssFilter; // 2 index to _sssFilters
	uint32_t firstSampleIndex; // 4 offset to _sssSamplesData
};

struct SssSample {
	uint16_t pcm; // index to _sssPcmTable
	uint16_t unk2; // framesCount
	uint8_t unk4; // initVolume
	uint8_t unk5;
	int8_t unk6; // priority
	uint8_t unk7; // initPanning
	uint32_t codeOffset1; // 0x8 offset to _sssCodeData
	uint32_t codeOffset2; // 0xC offset to _sssCodeData
	uint32_t codeOffset3; // 0x10 offset to _sssCodeData
	uint32_t codeOffset4; // 0x14 offset to _sssCodeData
}; // sizeof == 24

struct SssUnk4 { // SssPreloadInfo
	uint32_t count;
	uint8_t *data; // sizeof 32
};

struct SssFilter {
	int32_t unk0;
	int32_t unk4; // volume (0,127)
	int32_t unk8; // volumeDelta
	int32_t unkC; // volumeSteps
	int32_t unk10;
	int32_t unk14; // panning (0,127)
	int32_t unk18; // panningDelta
	int32_t unk1C; // panningSteps
	int32_t unk20;
	int32_t unk24; // priority (0,7)
	int32_t unk30; // flag
}; // sizeof == 52

struct SssPcm {
	int16_t *ptr;    // 0 PCM data
	uint32_t offset; // 4 offset in .sss
	uint32_t totalSize;   // 8 size in .sss (256 int16_t words + followed by indexes)
	uint32_t strideSize;  // 12
	uint16_t strideCount; // 16
	uint16_t flags;       // 18 1:stereo
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
	uint8_t *_fontBuffer;
	uint8_t *_menuBuffer0;
	uint8_t *_menuBuffer1;
	uint32_t _menuBuffersOffset;

	uint8_t _currentScreenResourceNum;

	uint8_t _screensGrid[40 * 4];
	LvlScreenVector _screensBasePos[40];
	LvlScreenState _screensState[40];
	uint8_t *_resLevelData0x470CTable;
	uint8_t *_resLevelData0x470CTablePtrHdr;
	uint8_t *_resLevelData0x470CTablePtrData;
	uint32_t _resLevelData0x2B88SizeTable[40];
	uint32_t _resLevelData0x2988SizeTable[32];
	LvlObjectData _resLevelData0x2988Table[40];
	LvlObjectData *_resLevelData0x2988PtrTable[32];
	LvlBackgroundData _resLvlScreenBackgroundDataTable[40];
	uint8_t *_resLvlScreenBackgroundDataPtrTable[40];

	LvlObject _resLvlScreenObjectDataTable[104];
	LvlObject _dummyObject; // (LvlObject *)0xFFFFFFFF

	SssInfo *_sssInfosData;
	SssUnk2 *_sssDataUnk2;
	SssBank *_sssBanksData;
	SssSample *_sssSamplesData;
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
	MstUnk34 *_mstUnk34;
	MstUnk35 *_mstUnk35;
	MstUnk36 *_mstUnk36;
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
	MstUnk45 *_mstUnk45;
	MstUnk46 *_mstUnk46;
	MstUnk47 *_mstUnk47;
	MstUnk48 *_mstUnk48;
	uint8_t *_mstHeightMapData; // _mstMonsterInfos sizeof == 948 == (32 * 28 + 52)
	MstUnk49 *_mstUnk49;
	MstUnk50 *_mstUnk50;
	MstUnk51 *_mstUnk51;
	uint8_t *_mstUnk52; // sizeof == 4
	MstUnk53 *_mstUnk53;
	MstUnk54 *_mstUnk54;
	MstUnk55 *_mstUnk55;
	MstUnk56 *_mstUnk56;
	MstOp49Data *_mstOp49Data;
	MstOp58Data *_mstOp58Data;
	MstUnk59 *_mstUnk59;
	uint32_t *_mstUnk60; // index to _mstCodeData
	MstOp56Data *_mstOp56Data;
	uint8_t *_mstCodeData;
	MstUnk63 *_mstUnk63;

	Resource(const char *dataPath);
	~Resource();

	bool sectorAlignedGameData();

	void loadLevelData(const char *levelName);

	void loadSetupDat();
	void loadLvlScreenMoveData(int num);
	void loadLvlScreenVectorData(int num);
	void loadLvlScreenStateData(int num);
	void loadLvlScreenObjectData(int num);
	void loadLvlData(File *fp, const char *name);
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

	void loadDatHintImage(int num, uint8_t *dst, uint8_t *pal);
	void loadDatLoadingImage(uint8_t *dst, uint8_t *pal);
	void loadDatMenuBuffers();

	const uint8_t *getLvlSpriteFramePtr(LvlObjectData *dat, int frame);
	const uint8_t *getLvlSpriteCoordPtr(LvlObjectData *dat, int num);

	void loadSssData(File *fp, const char *name);
	void checkSssCode(const uint8_t *buf, int size);
	void loadSssPcm(File *fp, int num);
	uint32_t getSssPcmSize(const SssPcm *pcm) const;

	void clearSssLookupTable3();

	void loadMstData(File *fp, const char *name);

	MstScreenAreaCode *findMstCodeForPos(int num, int xPos, int yPos);
	void flagMstCodeForPos(int num, uint8_t value);
};

#endif // RESOURCE_H__

