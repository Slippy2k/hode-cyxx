/*
 * Heart of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir (cyx@users.sourceforge.net)
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
	uint32_t levelCheckpointsCount[8]; // 0x20..0x3C
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
	uint8_t currentShadowId;
	uint8_t soundCount;
	uint8_t currentSoundId;
	uint8_t dataUnk3Count;
	uint8_t unk9;
	uint8_t dataUnk45Count;
	uint8_t unkB;
	uint16_t backgroundPaletteId;
	uint16_t backgroundBitmapId;
	uint8_t *backgroundPaletteTable[4];
	uint8_t *backgroundBitmapTable[4];
	uint8_t *dataUnk0Table[4]; // unused
	uint8_t *backgroundMaskTable[4];
	uint8_t *backgroundSoundTable[4];
	uint8_t *backgroundAnimationTable[4];
	uint8_t *dataUnk4Table[4]; // unused
	LvlObjectData *dataUnk5Table[4];
	uint8_t *dataUnk6Table[4]; // unused
};

struct MstHdr {
	int version;
	int dataSize;
	uint32_t walkBoxDataCount; // 8
	int unk0x0C; // mstUnk35DataCount
	int unk0x10; // mstUnk36DataCount
	int levelCheckpointCodeDataCount; // 14
	int screenAreaDataCount; // 18
	int unk0x1C; // mstUnk39DataCount
	int unk0x20; // mstUnk42DataCount
	int unk0x24; // mstUnk43DataCount
	int walkPathDataCount; // 28
	int infoMonster2Count; // 2C
	int unk0x30; // mstUnk45DataCount
	int unk0x34; // mstUnk47DataCount
	int unk0x38; // mstUnk48DataCount
	int infoMonster1Count; // 3C
	int unk0x40; // mstUnk49DataCount
	int unk0x44; // mstUnk50DataCount
	int unk0x48; // mstUnk51DataCount
	int unk0x4C; // mstUnk52DataCount
	int op223DataCount; // 50
	int op226DataCount; // 54
	int op227DataCount; // 58
	int op234DataCount; // 5C
	int op2DataCount; // 60
	int op197DataCount; // 64
	int op211DataCount; // 68
	int op240DataCount; // 6C
	int unk0x70; // mstUnk60DataCount
	int unk0x74; // mstUnk61DataCount
	int op204DataCount; // 78
	int codeSize; // sizeof(uint32_t)
	int pointsCount; // 80
};

struct MstPointOffset {
	int32_t xOffset;
	int32_t yOffset;
}; // sizeof == 8

struct MstScreenArea {
	int32_t x1; // 0
	int32_t x2; // 4
	int32_t y1; // 8
	int32_t y2; // 0xC
	uint32_t nextByPos; // 0x10 _mstUnk40
	uint32_t prev; // 0x14 unused
	uint32_t nextByValue; // 0x18 indexUnk39
	uint8_t unk0x1C; // 0x1C indexUnk39
	uint8_t unk0x1D; // 0x1D value
	uint16_t unk0x1E; // 0x1E unused
	uint32_t codeData; // 0x20, offset _mstCodeData
}; // sizeof == 36

struct MstWalkBox { // MstUnk34
	int32_t right; // 0
	int32_t left; //  4
	int32_t top; // 8
	int32_t bottom; // 0xC
	uint8_t flags[4]; // 0x10
}; // sizeof == 20

struct MstUnk35 { // MstWalkCode
	uint32_t *indexCodeData; // 0, offset _mstCodeData codeData
	uint32_t count1; // 4 codeDataCount
	uint8_t *data2; // 8 data
	uint32_t count2; // C dataCount
}; // sizeof == 16

struct MstUnk36 {
	uint32_t indexUnk49; // indexes _mstUnk49
	uint32_t unk4; // indexes _mstUnk49.data1
	int32_t unk8;
}; // sizeof == 12

struct MstUnk42 {
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

struct MstWalkNode { // u44u1
	int32_t x1; // 0
	int32_t x2; // 4
	int32_t y1; // 8
	int32_t y2; // 0xC
	uint32_t indexWalkBox; // 0x10
	uint32_t indexUnk35_20; // 0x14
	uint32_t indexUnk35_0x18; // 0x18
	uint32_t indexUnk36_28; // 0x1C
	uint32_t indexUnk36_32; // 0x20
	uint32_t indexUnk35_0x24[2];
	int32_t unk2C[4][2]; // 0x2C, 0x34, 0x3C, 0x44
	uint32_t neighborNodes[4]; // per direction 0x4C
	uint32_t indexUnk44Unk1_92; // next 0x5C
	uint8_t *unk60[2]; // 0x60
}; // sizeof == 104

struct MstWalkPath { // u44
	MstWalkNode *data;
	uint32_t *indexUnk44Unk1; // indexed by screen number
	uint32_t mask; // 0x8
	uint32_t count; // 0xC
}; // sizeof == 16

struct MstInfoMonster2 {
	uint8_t type; // 0x0
	uint8_t shootMask; // 0x1
	uint16_t anim; // 0x2
	uint32_t codeData; // 4
	uint32_t unk8;
}; // sizeof == 12

struct MstUnk46Unk1 { // MstBehavior
	uint32_t indexMonsterInfo; // 0, indexes _mstMonsterInfos
	uint16_t anim; // 4
	uint16_t unk6; // 6
	uint32_t unk8; // 0x8
	uint32_t energy; // 0xC
	uint32_t unk10; // 0x10
	uint32_t count; // 0x14
	uint32_t unk18; // 0x18
	uint32_t indexUnk51; // 0x1C indexes _mstUnk51
	uint32_t indexUnk44; // 0x20 indexes _mstWalkPathData
	uint32_t indexUnk47; // 0x24 indexes _mstUnk47
	uint32_t codeData; // 0x28 indexes _mstCodeData
}; // sizeof == 44

struct MstUnk46 {
	MstUnk46Unk1 *data;
	uint32_t count;
}; // sizeof == 8

struct MstUnk47 { // MstHitBox
	uint8_t *data; // sizeof == 20
	uint32_t count;
}; // sizeof == 8

struct MstUnk48Unk12Unk4 {
	uint32_t unk0; // 0x0, indexes _mstMonsterInfos
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
	uint32_t codeData; // 0x8 indexes _mstCodeData
	MstUnk48Unk12 *unk12; // 0xC
	int countUnk12; // 0x10
	uint32_t *data1[2]; // 0x14, 0x18
	uint32_t *data2[2]; // 0x1C, 0x20
	uint32_t count[2]; // 0x24, 0x28
}; // sizeof == 44

struct MstUnk49Unk1 {
	uint32_t offsetMonsterInfo; // 0, offset _mstMonsterInfos[32]
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
	uint32_t indexMonsterInfo; // 0, indexes _mstMonsterInfos
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

struct MstUnk52 {
	uint8_t unk0;
	uint8_t unk1;
	uint8_t unk2;
	uint8_t unk3;
}; // sizeof == 4

struct MstOp240Data {
	uint32_t flags;
	uint32_t codeData;
}; // sizeof == 8

struct MstOp223Data {
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

struct MstOp227Data {
	int16_t indexVar1; // 0
	int16_t indexVar2; // 2
	uint8_t compare; // 4
	uint8_t maskVars; // 5
	uint16_t codeData; // 6
}; // sizeof == 8

struct MstOp234Data {
	int16_t indexVar1; // 0
	int16_t indexVar2; // 2
	uint8_t compare; // 4
	uint8_t maskVars; // 5
	uint32_t codeData;
}; // sizeof == 8

struct MstOp2Data {
	int32_t indexVar1; // 0
	int32_t indexVar2; // 4
	uint8_t maskVars; // 8
	uint8_t unk9;
	uint8_t unkA;
	uint8_t unkB;
}; // sizeof == 12

struct MstOp226Data {
	uint8_t unk0;
	uint8_t unk1;
	uint8_t unk2;
	uint8_t unk3;
	uint8_t unk4;
	uint8_t unk5;
	uint8_t unk6;
	uint8_t unk7;
}; // sizeof == 8

struct MstOp204Data {
	uint32_t arg0; // 0
	uint32_t arg1; // 4
	uint32_t arg2; // 8
	uint32_t arg3; // C
}; // sizeof == 16

struct MstOp197Data {
	int16_t unk0;
	int16_t unk2;
	int16_t unk4;
	int16_t unk6;
	uint32_t maskVars;
	uint16_t indexUnk49;
	int8_t unkE;
	int8_t unkF;
}; // sizeof == 16

struct MstOp211Data {
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
	int bufferSize;
	int preloadPcmCount;
	int preloadInfoCount;
	int infosDataCount;
	int filtersDataCount;
	int banksDataCount;
	int samplesDataCount;
	int codeSize;
	int preloadData1Count; // 24
	int preloadData2Count; // 28
	int preloadData3Count; // 2C
	int pcmCount; // 30
};

struct SssInfo {
	uint16_t sssBankIndex; // 0 indexes _sssBanksData
	int8_t sampleIndex; // 2
	uint8_t targetVolume; // 3
	int8_t targetPriority; // 4
	int8_t targetPanning; // 5
	uint8_t concurrencyMask; // 6
};

struct SssDefaults {
	uint8_t defaultVolume;
	int8_t defaultPriority;
	int8_t defaultPanning;
};

struct SssBank {
	uint8_t flags; // 0 flags0
	int8_t count; // 1 codeOffsetCount
	uint16_t sssFilter; // 2 index to _sssFilters
	uint32_t firstSampleIndex; // 4 offset to _sssSamplesData
};

struct SssSample {
	uint16_t pcm; // index to _sssPcmTable
	uint16_t framesCount;
	uint8_t initVolume; // 0x4
	uint8_t unk5;
	int8_t initPriority; // 0x6
	int8_t initPanning; // 0x7
	uint32_t codeOffset1; // 0x8 offset to _sssCodeData
	uint32_t codeOffset2; // 0xC offset to _sssCodeData
	uint32_t codeOffset3; // 0x10 offset to _sssCodeData
	uint32_t codeOffset4; // 0x14 offset to _sssCodeData
}; // sizeof == 24

struct SssPreloadInfo {
	uint32_t count;
	uint8_t *data; // sizeof == 32 (v10,v11) 68 (v6)
};

struct SssFilter {
	int32_t volume;
	int32_t volumeCurrent; // fp16
	int32_t volumeDelta;
	int32_t volumeSteps;
	int32_t panning;
	int32_t panningCurrent; // fp16
	int32_t panningDelta;
	int32_t panningSteps;
	int32_t priority;
	int32_t priorityCurrent;
	bool changed; // 0x30
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
	uint32_t mask; // 10
};

struct SssPreloadData {
	uint8_t count;
	uint8_t *ptr;
};

template <typename T>
struct ResStruct {
	T *ptr;
	unsigned int count;

	ResStruct()
		: ptr(0), count(0) {
	}
	~ResStruct() {
		deallocate();
	}

	void deallocate() {
		free(ptr);
		count = 0;
	}
	void allocate(unsigned int size) {
		free(ptr);
		count = size;
		ptr = (T *)malloc(size * sizeof(T));
	}

	const T& operator[](int i) const {
		assert((unsigned int)i < count);
		return ptr[i];
	}
	T& operator[](int i) {
		assert((unsigned int)i < count);
		return ptr[i];
	}
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

	uint8_t _screensGrid[kMaxScreens * 4];
	LvlScreenVector _screensBasePos[kMaxScreens];
	LvlScreenState _screensState[kMaxScreens];
	uint8_t *_resLevelData0x470CTable;
	uint8_t *_resLevelData0x470CTablePtrHdr;
	uint8_t *_resLevelData0x470CTablePtrData;
	uint32_t _resLevelData0x2B88SizeTable[kMaxScreens];
	uint32_t _resLevelData0x2988SizeTable[kMaxSpriteTypes];
	LvlObjectData _resLevelData0x2988Table[kMaxScreens];
	LvlObjectData *_resLevelData0x2988PtrTable[kMaxSpriteTypes];
	LvlBackgroundData _resLvlScreenBackgroundDataTable[kMaxScreens];
	uint8_t *_resLvlScreenBackgroundDataPtrTable[kMaxScreens];
	uint8_t *_resLvlSpriteDataPtrTable[kMaxSpriteTypes];

	LvlObject _resLvlScreenObjectDataTable[104];
	LvlObject _dummyObject; // (LvlObject *)0xFFFFFFFF

	ResStruct<SssInfo> _sssInfosData;
	ResStruct<SssDefaults> _sssDefaultsData;
	ResStruct<SssBank> _sssBanksData;
	ResStruct<SssSample> _sssSamplesData;
	ResStruct<SssPreloadInfo> _sssPreloadInfosData;
	ResStruct<SssFilter> _sssFilters;
	ResStruct<SssPcm> _sssPcmTable;
	ResStruct<SssUnk6> _sssDataUnk6;
	uint32_t *_sssGroup1[3];
	uint32_t *_sssGroup2[3];
	uint32_t *_sssGroup3[3];
	uint8_t *_sssCodeData;

	ResStruct<MstPointOffset> _mstPointOffsets;
	ResStruct<MstWalkBox> _mstWalkBoxData;
	ResStruct<MstUnk35> _mstUnk35;
	ResStruct<MstUnk36> _mstUnk36;
	uint32_t _mstTickDelay;
	uint32_t _mstTickCodeData;
	ResStruct<uint32_t> _mstLevelCheckpointCodeData;
	ResStruct<MstScreenArea> _mstScreenAreaData;
	ResStruct<uint32_t> _mstUnk39; // index to _mstScreenAreaData
	ResStruct<uint32_t> _mstUnk40; // index to _mstScreenAreaData
	ResStruct<uint32_t> _mstUnk41;
	ResStruct<MstUnk42> _mstUnk42;
	ResStruct<MstUnk43> _mstUnk43;
	ResStruct<MstWalkPath> _mstWalkPathData;
	ResStruct<MstInfoMonster2> _mstInfoMonster2Data;
	ResStruct<MstUnk46> _mstUnk46;
	ResStruct<MstUnk47> _mstUnk47;
	ResStruct<MstUnk48> _mstUnk48;
	uint8_t *_mstMonsterInfos; // sizeof == 948
	ResStruct<MstUnk49> _mstUnk49;
	ResStruct<MstUnk50> _mstUnk50;
	ResStruct<MstUnk51> _mstUnk51;
	ResStruct<MstUnk52> _mstUnk52;
	ResStruct<MstOp223Data> _mstOp223Data;
	ResStruct<MstOp227Data> _mstOp227Data;
	ResStruct<MstOp234Data> _mstOp234Data;
	ResStruct<MstOp2Data> _mstOp2Data;
	ResStruct<MstOp197Data> _mstOp197Data;
	ResStruct<MstOp211Data> _mstOp211Data;
	ResStruct<MstOp240Data> _mstOp240Data;
	ResStruct<uint32_t> _mstUnk60; // index to _mstCodeData
	ResStruct<MstOp204Data> _mstOp204Data;
	uint8_t *_mstCodeData;
	ResStruct<MstOp226Data> _mstOp226Data;

	Resource(const char *dataPath);
	~Resource();

	bool sectorAlignedGameData();

	void loadSetupDat();
	void loadDatHintImage(int num, uint8_t *dst, uint8_t *pal);
	void loadDatLoadingImage(uint8_t *dst, uint8_t *pal);
	void loadDatMenuBuffers();

	void loadLevelData(int levelNum);

	void loadLvlScreenGridData(int num);
	void loadLvlScreenVectorData(int num);
	void loadLvlScreenStateData(int num);
	void loadLvlScreenObjectData(int num);
	void loadLvlData(File *fp, const char *name);
	void unloadLvlData();
	void loadLvlSpriteData(int num);
	const uint8_t *getLvlScreenMaskDataPtr(int num) const;
	const uint8_t *getLvlScreenPosDataPtr(int num) const;
	void loadLevelData0x470C();
	void loadLvlScreenBackgroundData(int num);
	void unloadLvlScreenBackgroundData(int num);
	bool isLvlSpriteDataLoaded(int num) const;
	bool isLvlBackgroundDataLoaded(int num) const;
	void incLvlSpriteDataRefCounter(LvlObject *ptr);
	void decLvlSpriteDataRefCounter(LvlObject *ptr);
	const uint8_t *getLvlSpriteFramePtr(LvlObjectData *dat, int frame) const;
	const uint8_t *getLvlSpriteCoordPtr(LvlObjectData *dat, int num) const;

	void loadSssData(File *fp, const char *name);
	void unloadSssData();
	void checkSssCode(const uint8_t *buf, int size) const;
	void loadSssPcm(File *fp, int num);
	uint32_t getSssPcmSize(const SssPcm *pcm) const;
	void clearSssGroup3();
	void resetSssFilters();

	void loadMstData(File *fp, const char *name);
	void unloadMstData();
	const MstScreenArea *findMstCodeForPos(int num, int xPos, int yPos) const;
	void flagMstCodeForPos(int num, uint8_t value);
};

#endif // RESOURCE_H__
