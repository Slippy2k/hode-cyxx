/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
 */

#ifndef GAME_H__
#define GAME_H__

#include "intern.h"
#include "defs.h"
#include "fileio.h"
#include "random.h"
#include "resource.h"

struct Game;
struct Task;
struct PafPlayer;
struct Video;
struct SystemStub;

template <typename T>
inline T *PTR(void *base, const uint8_t *offsets, int index) {
	const uint32_t offset = READ_LE_UINT32((const uint8_t *)offsets + index * 4);
	return offset == 0xFFFFFFFF ? 0 : (T *)(((const uint8_t *)base) + offset * sizeof(T));
}

template <typename T>
inline const uint8_t *PTR_OFFS(const uint8_t *base, uint32_t index) {
	return index == 0xFFFFFFFF ? 0 : (const uint8_t *)base + index * sizeof(T);
}

struct MixerChannel {
	const int16_t *pcm;
	uint32_t size;
	uint32_t offset;
};

struct Game {
	typedef void (Game::*OpStage0_2Proc)();
	typedef int (Game::*OpStage1Proc)(LvlObject *o);
	typedef void (Game::*OpStage3_4_5Proc)();

	enum {
		kObjectDataTypeAndy,
		kObjectDataTypeRect,
		kObjectDataTypeUnk1
	};
	enum {
		kMixerChannelsCount = 16,
		kFrameTimeStamp = 50 // 80
	};

	static const uint8_t _gameLevelStartingCutscene[]; // _levelStartingCutsceneTable
	static const uint8_t _pointDstIndexTable[];
	static const uint8_t _pointRandomizeShiftTable[];
	static const uint8_t _pointSrcIndex2Table[];
	static const uint8_t _pointSrcIndex1Table[];
	static const uint8_t _actionDirectionKeyMaskTable[];
	static const char *_resLevelNames[];
	static const uint8_t _levelOpStage3ImageData1[];
	static const uint8_t _levelOpStage3ImageData2[];
	static const uint8_t *_levelCheckpointData[];
	static const uint8_t *_levelUpdateData2[];
	static const OpStage1Proc _level1OpStage1[];
	static const OpStage1Proc _level2OpStage1[];
	static const OpStage1Proc _level3OpStage1[];
	static const uint8_t _dbVolumeTable[129];
	static const uint8_t _benchmarkData1[];
	static const uint8_t _benchmarkData2[];

	PafPlayer *_paf;
	Random _rnd;
	Resource *_res;
	Video *_video;
	SystemStub *_system;
	MixerChannel _mixerChannels[kMixerChannelsCount];

	LvlObject *_andyObject;
	LvlObject *_plasmaExplosionObject;
	LvlObject *_plasmaCannonObject;
	LvlObject *_currentMonsterObject;
	LvlObject *_currentSoundLvlObject;
	int _currentLevel;
	int _levelCheckpoint;
	bool _quit;
	Sprite _gameSpriteListTable[128];
	Sprite *_gameSpriteListHead;
	Sprite *_gameSpriteListPtrTable[32];
	uint16_t _fadePaletteBuffer[256 * 3];
	uint8_t *_shadowScreenMaskBuffer;
	uint8_t _directionKeyMask;
	uint8_t _actionKeyMask;
	uint8_t _currentRightScreen;
	uint8_t _currentLeftScreen;
	uint8_t _currentScreen;
	uint8_t _fadePaletteCounter;
	uint8_t _fadePalette;
	bool _hideAndyObjectSprite;
	GameUnkList1 _gameUnkList1Table[32];
	GameUnkList1 *_gameUnkList1Head;
	int32_t _globalVars[40];
	LvlObject *_lvlObjectsList0;
	LvlObject *_lvlObjectsList1;
	LvlObject *_lvlObjectsList2;
	LvlObject *_lvlObjectsList3;
	uint8_t _screenCounterTable[40];
	uint8_t _screenPosTable[5][24 * 32];
	uint8_t _screenTempMaskBuffer[24 * 32];
	uint8_t _screenMaskBuffer[96 * 24 * 32];
	int _gameCurrentLevelScreenNum;
	uint8_t _shakeScreenDuration;
	const uint8_t *_shakeScreenTable;
	uint8_t _plasmaCannonDirection;
	uint8_t _plasmaCannonPrevDirection;
	uint8_t _plasmaCannonPointsSetupCounter;
	uint8_t _plasmaCannonLastIndex1;
	uint8_t _gameMainLoopFlag5;
	uint8_t _gameMainLoopFlag6;
	uint8_t _plasmaCannonFirstIndex;
	uint8_t _plasmaCannonLastIndex2;
	uint8_t _plasmaCannonFlags;
	uint8_t _plasmaCannonKeyMaskCounter;
	uint8_t _gameResType0CallbackFlag1;
	uint8_t _gameResType0CallbackFlag2;
	uint8_t _gameKeyPressedMaskIndex;
	uint8_t _andyActionKeyMaskAnd, _andyActionKeyMaskOr;
	uint8_t _andyDirectionKeyMaskAnd, _andyDirectionKeyMaskOr;
	int32_t _gameXPosTable[129];
	int32_t _gameYPosTable[129];
	int32_t _plasmaCannonXPointsTable1[129];
	int32_t _plasmaCannonYPointsTable1[129];
	int32_t _plasmaCannonXPointsTable2[127];
	int32_t _plasmaCannonYPointsTable2[127];
	uint16_t _gameMstAnim;
	uint16_t _gameMstAnimFlags1;
	int16_t _screen_dx;
	int16_t _screen_dy;
	ScreenMask _shadowScreenMasksTable[8];

	uint32_t _gameMstLogicHelper1TestValue;
	uint32_t _gameMstLogicHelper1TestMask;
	Task *_tasksListTail;
	int _runTaskOpcodesCount;
	uint32_t _mstGlobalFlags;
	int _clipBoxOffsetX, _clipBoxOffsetY;
	Task *_gameMstResToLoad1Pri, *_gameMstResToLoad2Pri;
	Task *_currentTask;
	int _mstOp54Counter;
	uint8_t _mstOp54Table[32];
	bool _mstLogicDisabled;
	LvlObject _declaredLvlObjectsList[160];
	LvlObject *_declaredLvlObjectsListHead;
	int _declaredLvlObjectsListCount;
	AndyObjectScreenData _andyObjectScreenData;
	GameRect _gameRectsListTable[64];
	int _gameRectsListCount;
	uint8_t _resUpdateLevelDataType0Flag;
	int _executeMstLogicCounter;
	int _executeMstLogicLastCounter;
	int _gameMstScreenRefPosX, _gameMstScreenRefPosY;
	int _gameMstMovingStatePosX, _gameMstMovingStatePosY;
	int _gameMstObjectRefPointPosX, _gameMstObjectRefPointPosY;

	Game(SystemStub *system, const char *dataPath);
	~Game();

	// benchmark.cpp
	uint32_t benchmarkLoop(const uint8_t *p, int count);
	uint32_t benchmarkCpu();

	// game.cpp

	void mainLoop(int level, int checkpoint);
	void GameClearUnkList1();
	void GameRemoveGameUnkList1ElementFromLevelScreenData(LvlObject *ptr);
	void setShakeScreen(int type, int counter);
	void fadeScreenPalette();
	void shakeScreen();
	void decodeShadowScreenMask(LvlBackgroundData *lvl);
	void playSound(int num, LvlObject *ptr, int a, int b);
	void removeSound(LvlObject *ptr);
	void setupBackgroundBitmap();
	void addToSpriteList(LvlObject *ptr);
	int16_t fixScreenPos(int16_t xPos, int16_t yPos, int num);
	void setupScreenPosTable(uint8_t num);
	void setupScreenMask(uint8_t num);
	void resetScreenMask();
	void setupLvlObjectBitmap(LvlObject *ptr);
	void randomizeInterpolatePoints(int32_t *pts, int count);
	int fixPlasmaCannonPointsScreenMask(int num);
	void setupPlasmaCannonPointsHelper();
	void destroyLvlObjectUnk(LvlObject *o);
	void shuffleArray(uint8_t *p, int count);
	void destroyLvlObject(LvlObject *o);
	void setupPlasmaCannonPoints(LvlObject *ptr);
	int testPlasmaCannonPointsDirection(int x1, int y1, int x2, int y2);
	void preloadLevelScreenData(int num, int prev);
	void loadLevelScreenSounds(int num);
	void setLvlObjectPosRelativeToObject(LvlObject *ptr1, int num1, LvlObject *ptr2, int num2);
	void setLvlObjectPosRelativeToPoint(LvlObject *ptr, int num, int x, int y);
	void level1SetupLvlObjects_screen2();
	void level1SetupLvlObjects_screen3();
	void level1SetupLvlObjects_screen18();
	void clearLvlObjectsList1();
	void clearLvlObjectsList2();
	void clearLvlObjectsList3();
	LvlObject *addLvlObjectToList1(int type, int num);
	int addLvlObjectToList3(int num);
	LvlObject *findLvlObjectNoDataPtr(int num, int index);
	void removeLvlObject(LvlObject *ptr);
	void removeLvlObjectNotType2List1(LvlObject *o);
	void setupCurrentScreen();
	void updateScreenHelper(int num);
	void resetDisplay();
	void updateScreen(uint8_t num);
	void resetLevel();
	void restartLevel();
	void playAndyFallingCutscene(int type);
	int8_t updateLvlObjectScreen(LvlObject *ptr);
	void setAndyLvlObjectPlasmaCannonKeyMask();
	int game_unk27(uint8_t mask);
	int clipBoundingBox(BoundingBox *coords, BoundingBox *box);
	int updateBoundingBoxClippingOffset(BoundingBox *_ecx, BoundingBox *_ebp, const uint8_t *coords, int direction);
	int game_unk16(LvlObject *o1, BoundingBox *box1, LvlObject *o2, BoundingBox *box2);
	int clipLvlObjectsBoundingBox(LvlObject *o, LvlObject *ptr, int count);
	int updateAndyLvlObject();
	void drawPlasmaCannon();
	void redrawObjects();
	void updateLvlObjectList(LvlObject *list);
	void updateLvlObjectLists();
	LvlObject *updateAnimatedLvlObjectType0(LvlObject *ptr);
	LvlObject *updateAnimatedLvlObjectType1(LvlObject *ptr);
	LvlObject *updateAnimatedLvlObjectType2(LvlObject *ptr);
	LvlObject *updateAnimatedLvlObjectTypeDefault(LvlObject *ptr);
	LvlObject *updateAnimatedLvlObject(LvlObject *o);
	void updateAnimatedLvlObjectsLeftRightCurrentScreens();
	void updatePlasmaCannonExplosionLvlObject(LvlObject *ptr);
	void resetPlasmaCannonState();
	void GameLevelMainLoopHelper3();
	void updateInput();
	void initMstCode();
	void levelMainLoop();
	void callLevelOpStage0(int num); // callLevel_preScreenUpdate
	int callLevelOpStage1(int num, LvlObject *o); // callLevel_objectUpdate
	void callLevelOpStage2(int num); // callLevel_postScreenUpdate
	void callLevelOpStage3(); // callLevel_preTick
	void callLevelOpStage4(); // callLevel_postTick
	void callLevelOpStage5(); // callLevel_terminate
	int displayHintScreen(int num, int pause);
	void prependLvlObjectToList(LvlObject **list, LvlObject *ptr);
	void removeLvlObjectFromList(LvlObject **list, LvlObject *ptr);
	void *getLvlObjectDataPtr(LvlObject *o, int type);
	void lvlObjectType0Init(LvlObject *ptr);
	void lvlObjectType1Init(LvlObject *ptr);
	void lvlObjectTypeInit(LvlObject *ptr);
	void lvlObjectType0CallbackHelper1();
	int lvlObjectType0CallbackHelper2(int x, int y, int num);
	void lvlObjectType0CallbackHelper3(LvlObject *ptr);
	void lvlObjectType0CallbackHelper4(LvlObject *ptr);
	int lvlObjectType0Callback(LvlObject *ptr);
	int lvlObjectType1Callback(LvlObject *ptr);
	int lvlObjectList3Callback(LvlObject *o);
	void lvlObjectTypeCallback(LvlObject *o);
	LvlObject *game_unk115(int type, int y, int x, int screen, int num, int o_anim, int o_flags1, int o_flags2, int actionKeyMask, int directionKeyMask);
	int setLvlObjectPosInScreenGrid(LvlObject *o, int num);
	LvlObject *declareLvlObject(uint8_t type, uint8_t num);
	void clearDeclaredLvlObjectsList();
	void initLvlObjects();
	void setLvlObjectType8Resource(LvlObject *ptr, uint8_t _dl, uint8_t num);
	void captureScreenshot();

	// level1.cpp

	void level1OpStage0_screen0();
	void level1OpStage0_screen4();
	void level1OpStage0_screen8();
	void level1OpStage0_screen9Helper(int num);
	void level1OpStage0_screen9();
	void level1OpHelper3(BoundingBox *box, int num);
	void level1OpStage0_screen10();
	void level1OpStage0_screen11();
	void level1OpStage0_screen13();
	void level1OpStage0_screen15();
	void level1OpStage0_screen16();
	void level1OpStage0_screen18();
	void level1OpStage0_screen19();
	void callLevelOpStage0_level1(int num);
	int level1OpStage1_case0(LvlObject *o);
	void level1OpHelper1(LvlObject *ptr, uint8_t *p);
	int level1OpHelper2(LvlObject *ptr);
	int level1OpStage1_case1(LvlObject *o);
	int level1OpStage1_case2(LvlObject *o);
	int level1OpStage1_case3(LvlObject *o);
	int level1OpStage1_case4(LvlObject *o);
	int callLevelOpStage1_level1(int num, LvlObject *o);
	void level1OpStage2_screen0();
	void level1OpStage2_screen1();
	void level1OpStage2_screen2();
	void level1OpStage2_screen3();
	void level1OpStage2_screen4();
	void level1OpStage2_screen5();
	void level1OpStage2_screen7();
	void level1OpStage2_screen9();
	void level1OpStage2_screen10();
	void level1OpStage2_screen13();
	void level1OpStage2_screen14();
	void level1OpStage2_screen15();
	void level1OpStage2_screen16();
	void level1OpStage2_screen17();
	void level1OpStage2_screen18();
	void level1OpStage2_screen19();
	void callLevelOpStage2_level1(int num);
	void level1OpStage3();
	void level1OpStage4();
	void level1OpStage5();
	void level1SetupLvlObjects(int num);

	// level2.cpp

	void level2OpStage0_screen1();
	void level2OpStage0_screen6();
	void level2OpStage0_screen7();
	void level2OpStage0_screen8();
	void level2OpStage0_screen16();
	void level2OpStage0_screen17();
	void level2OpStage0_screen21();
	void callLevelOpStage0_level2(int num);
	int level2OpStage1_case0(LvlObject *o);
	int level2OpStage1_case1(LvlObject *o);
	int callLevelOpStage1_level2(int num, LvlObject *o);
	void level2OpStage2_screen1();
	void level2OpStage2_screen2();
	void level2OpStage2_screen6();
	void level2OpStage2_screen9();
	void level2OpStage2_screen14();
	void level2OpStage2_screen16();
	void level2OpStage2_screen17();
	void level2OpStage2_screen21();
	void callLevelOpStage2_level2(int num);

	// level3.cpp

	void callLevelOpStage0_level3(int num);
	int level3OpStage1_case0(LvlObject *o);
	int level3OpStage1_case1(LvlObject *o);
	int callLevelOpStage1_level3(int num, LvlObject *o);
	void callLevelOpStage2_level3(int num);
	void level3OpStage3();

	// level4.cpp

	void level4OpStage3();
	void level4OpStage4();
	void level4OpStage5();

	// mst.cpp

	void resetMstCode();
	void startMstCode();
	void executeMstCode();

	// sound.cpp
	SssObject _sssObjectsTable[32];
	uint8_t _sssObjectsChanged;
	int _sssObjectsCount;
	SssObject *_sssObjectsList1;
	SssObject *_sssObjectsList2;
	SssObject *_sssObjectsList3;
	uint8_t _channelMixingTable[32];
	int _snd_volumeMin;
	int _snd_fadeVolumeCounter;
	int _snd_masterVolume;

	void resetSound();
	void removeSoundObject(SssObject *so);
	void updateSoundObject(SssObject *so);
	const uint8_t *executeSssCode(SssObject *so, const uint8_t *code);
	void prepareSoundObject(int num, int b, int c);
	SssObject *startSoundObject(int num, int b, int flags);
	void setupSoundObject(SssUnk1 *s, int a, int b);
	void clearSoundObjects();
	void fadeSoundObject(SssObject *so);
	int getSoundObjectVolumeByPos(SssObject *so) const;
	void setSoundObjectVolume(SssObject *so);
	void expireSoundObjects(int flags);
	void mixSoundObjects17640(bool flag);
	void mixSoundsCb(int16_t *buf, int len);

	// andy.cpp

	AndyMoveData _andyMoveData;
	int32_t _andyPosX;
	int32_t _andyPosY;
	uint8_t _andyMoveMask;
	uint8_t _andyUpdatePositionFlag;
	const Point16_t *_andyLevelData0x288PosTablePtr;
	uint8_t _andyMoveState[32];
	int _andyMaskBufferPos1;
	int _andyMaskBufferPos2;
	int _andyMaskBufferPos4;
	int _andyMaskBufferPos5;
	int _andyMaskBufferPos3;
	int _andyMaskBufferPos0;
	int _andyMaskBufferPos7;
	int _andyMaskBufferPos6;

	int moveAndyObjectOp1(int op);
	int moveAndyObjectOp2(int op);
	int moveAndyObjectOp3(int op);
	int moveAndyObjectOp4(int op);
	void setupAndyObjectMoveData(LvlObject *ptr);
	void setupAndyObjectMoveState();
	void updateAndyObject(LvlObject *ptr);
};

#endif // GAME_H__
