/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
 */

#ifndef GAME_H__
#define GAME_H__

#include "intern.h"
#include "defs.h"
#include "fileio.h"
#include "mixer.h"
#include "random.h"
#include "resource.h"

struct Game;
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

struct Game {
	typedef int (Game::*OpStage1Proc)(LvlObject *o);
	typedef void (Game::*CallLevelProc1)(int);
	typedef void (Game::*CallLevelProc0)();

	enum {
		kObjectDataTypeAndy,
		// LvlObject.type == 0
		kObjectDataTypeAnimBackgroundData,
		// LvlObject.type == 8
		kObjectDataTypeOther,
		// LvlObject.type == 1
		kObjectDataTypeLvlBackgroundSound
	};
	enum {
		kMaxScreens = 40,
		kFrameTimeStamp = 50 // 80
	};

	static const uint8_t _gameLevelStartingCutscene[]; // _levelStartingCutsceneTable
	static const uint8_t _pointDstIndexTable[];
	static const uint8_t _pointRandomizeShiftTable[];
	static const uint8_t _pointSrcIndex2Table[];
	static const uint8_t _pointSrcIndex1Table[];
	static const uint8_t _actionDirectionKeyMaskTable[];
	static const char *_resLevelNames[];
	static const uint8_t *_levelCheckpointData[];
	static const uint8_t *_levelScreenStartData[];
	static const uint8_t _transformBufferData1[];
	static const uint8_t _transformBufferData2[];

	Mixer _mix;
	PafPlayer *_paf;
	Random _rnd;
	Resource *_res;
	Video *_video;
	SystemStub *_system;

	LvlObject *_screenLvlObjectsList[kMaxScreens];
	LvlObject *_andyObject;
	LvlObject *_plasmaExplosionObject;
	LvlObject *_plasmaCannonObject;
	LvlObject *_currentMonsterObject;
	LvlObject *_currentSoundLvlObject;
	int _currentLevel;
	int _levelCheckpoint;
	int _currentLevelCheckpoint;
	bool _quit;
	Sprite _gameSpriteListTable[128];
	Sprite *_gameSpriteListHead;
	Sprite *_gameSpriteListPtrTable[32];
	uint16_t _fadePaletteBuffer[256 * 3];
	uint8_t *_shadowScreenMaskBuffer;
	uint8_t *_transformShadowBuffer;
	uint8_t _transformShadowLayerDelta;
	uint8_t _directionKeyMask;
	uint8_t _actionKeyMask;
	uint8_t _currentRightScreen;
	uint8_t _currentLeftScreen;
	uint8_t _currentScreen;
	uint8_t _fadePaletteCounter;
	uint8_t _fadePalette;
	bool _hideAndyObjectSprite;
	OtherObjectScreenData _otherObjectScreenDataTable[32];
	OtherObjectScreenData *_otherObjectScreenDataList;
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
	uint8_t _plasmaCannonExplodeFlag;
	uint8_t _plasmaCannonPointsMask;
	uint8_t _plasmaCannonFirstIndex;
	uint8_t _plasmaCannonLastIndex2;
	uint8_t _plasmaCannonFlags;
	uint8_t _plasmaCannonKeyMaskCounter;
	bool _fallingAndyFlag;
	uint8_t _fallingAndyCounter;
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
	int _runTaskOpcodesCount;
	int32_t _mstGlobalVars[40];
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
	AnimBackgroundData _animBackgroundDataTable[64];
	int _animBackgroundDataCount;
	uint8_t _andyActionKeysFlags;
	int _executeMstLogicCounter;
	int _executeMstLogicLastCounter;
	int _gameMstScreenRefPosX, _gameMstScreenRefPosY;
	int _gameMstMovingStatePosX, _gameMstMovingStatePosY;
	int _gameMstObjectRefPointPosX, _gameMstObjectRefPointPosY;
	Task _tasksTable[128];
	Task *_tasksListTail;

	Game(SystemStub *system, const char *dataPath);
	~Game();

	// benchmark.cpp
	uint32_t benchmarkLoop(const uint8_t *p, int count);
	uint32_t benchmarkCpu();

	// game.cpp

	void mainLoop(int level, int checkpoint);
	void clearObjectScreenDataList();
	void prependObjectScreenDataList(LvlObject *ptr);
	void setShakeScreen(int type, int counter);
	void fadeScreenPalette();
	void shakeScreen();
	void transformShadowLayer(int delta);
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
	void resetScreen();
	void restartLevel();
	void playAndyFallingCutscene(int type);
	int8_t updateLvlObjectScreen(LvlObject *ptr);
	void setAndyLvlObjectPlasmaCannonKeyMask();
	int resetAndyLvlObjectPlasmaCannonKeyMask(uint8_t mask);
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
	void updateAndyMonsterObjects();
	void updateInput();
	void levelMainLoop();
	void callLevel_postScreenUpdate(int num);
	void callLevel_preScreenUpdate(int num);
	void callLevel_initialize();
	void callLevel_tick();
	void callLevel_terminate();
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
	LvlObject *findLvlObject(uint8_t type, uint8_t num, int index);
	LvlObject *findLvlObject2(uint8_t type, uint8_t flags, int index);
	void captureScreenshot();

	// level1_rock.cpp
	static const OpStage1Proc _callLevel_objectUpdate_rock[];

	void postScreenUpdate_rock_screen0();
	void postScreenUpdate_rock_screen4();
	void postScreenUpdate_rock_screen8();
	void postScreenUpdate_rock_helper1(int num);
	void postScreenUpdate_rock_screen9();
	void postScreenUpdate_rock_helper2(BoundingBox *box, int num);
	void postScreenUpdate_rock_screen10();
	void postScreenUpdate_rock_screen11();
	void postScreenUpdate_rock_screen13();
	void postScreenUpdate_rock_screen15();
	void postScreenUpdate_rock_screen16();
	void postScreenUpdate_rock_screen18();
	void postScreenUpdate_rock_screen19();
	void callLevel_postScreenUpdate_rock(int num);

	int objectUpdate_rock_case0(LvlObject *o);
	void level1OpHelper1(LvlObject *ptr, uint8_t *p);
	int level1OpHelper2(LvlObject *ptr);
	int objectUpdate_rock_case1(LvlObject *o);
	int objectUpdate_rock_case2(LvlObject *o);
	int objectUpdate_rock_case3(LvlObject *o);
	int objectUpdate_rock_case4(LvlObject *o);

	void preScreenUpdate_rock_screen0();
	void preScreenUpdate_rock_screen1();
	void preScreenUpdate_rock_screen2();
	void preScreenUpdate_rock_screen3();
	void preScreenUpdate_rock_screen4();
	void preScreenUpdate_rock_screen5();
	void preScreenUpdate_rock_screen7();
	void preScreenUpdate_rock_screen9();
	void preScreenUpdate_rock_screen10();
	void preScreenUpdate_rock_screen13();
	void preScreenUpdate_rock_screen14();
	void preScreenUpdate_rock_screen15();
	void preScreenUpdate_rock_screen16();
	void preScreenUpdate_rock_screen17();
	void preScreenUpdate_rock_screen18();
	void preScreenUpdate_rock_screen19();
	void callLevel_preScreenUpdate_rock(int num);

	void callLevel_initialize_rock();
	void callLevel_tick_rock();
	void callLevel_terminate_rock();

	void setupLvlObjects_rock_screen2();
	void setupLvlObjects_rock_screen3();
	void setupLvlObjects_rock_screen18();
	void callLevel_setupLvlObjects_rock(int hum);

	// level2_fort.cpp
	void postScreenUpdate_fort_screen1();
	void postScreenUpdate_fort_screen6();
	void postScreenUpdate_fort_screen7();
	void postScreenUpdate_fort_screen8();
	void postScreenUpdate_fort_screen16();
	void postScreenUpdate_fort_screen17();
	void postScreenUpdate_fort_screen21();
	void callLevel_postScreenUpdate_fort(int num);

	void preScreenUpdate_fort_screen1();
	void preScreenUpdate_fort_screen2();
	void preScreenUpdate_fort_screen6();
	void preScreenUpdate_fort_screen9();
	void preScreenUpdate_fort_screen14();
	void preScreenUpdate_fort_screen16();
	void preScreenUpdate_fort_screen17();
	void preScreenUpdate_fort_screen21();
	void callLevel_preScreenUpdate_fort(int num);

	void callLevel_tick_fort();

	void setupLvlObjects_fort_screen1();
	void callLevel_setupLvlObjects_fort(int num);

	// level3_pwr1.cpp
	void postScreenUpdate_pwr1_screen35();
	void callLevel_postScreenUpdate_pwr1(int num);

	void preScreenUpdate_pwr1_screen4();
	void preScreenUpdate_pwr1_screen6();
	void preScreenUpdate_pwr1_screen9();
	void preScreenUpdate_pwr1_screen15();
	void preScreenUpdate_pwr1_screen21();
	void preScreenUpdate_pwr1_screen23();
	void preScreenUpdate_pwr1_screen24();
	void preScreenUpdate_pwr1_screen26();
	void callLevel_preScreenUpdate_pwr1(int num);

	void callLevel_initialize_pwr1();
	void callLevel_tick_pwr1();

	// level4_isld.cpp
	void postScreenUpdate_isld_screen0();
	void postScreenUpdate_isld_screen1();
	void postScreenUpdate_isld_screen3();
	void postScreenUpdate_isld_screen4();
	void postScreenUpdate_isld_screen8();
	void postScreenUpdate_isld_screen9();
	void callLevel_postScreenUpdate_isld(int num);

	void preScreenUpdate_isld_screen20();
	void callLevel_preScreenUpdate_isld(int num);

	void callLevel_initialize_isld();
	void callLevel_tick_isld();
	void callLevel_terminate_isld();

	// level5_lava.cpp
	void postScreenUpdate_lava_screen0();
	void postScreenUpdate_lava_screen15();
	void callLevel_postScreenUpdate_lava(int num);

	void preScreenUpdate_lava_screen0();
	void preScreenUpdate_lava_screen3();
	void preScreenUpdate_lava_screen6();
	void preScreenUpdate_lava_screen10();
	void preScreenUpdate_lava_screen13();
	void preScreenUpdate_lava_screen15();
	void callLevel_preScreenUpdate_lava(int num);

	void callLevel_initialize_lava();
	void callLevel_tick_lava();

	void setupLvlObjects_lava_screen3();
	void callLevel_setupLvlObjects_lava(int num);

	// level6_pwr2.cpp
	void postScreenUpdate_pwr2_screen2();
	void postScreenUpdate_pwr2_screen5();
	void postScreenUpdate_pwr2_screen8();
	void callLevel_postScreenUpdate_pwr2(int num);

	void preScreenUpdate_pwr2_screen2();
	void preScreenUpdate_pwr2_screen3();
	void preScreenUpdate_pwr2_screen5();
	void preScreenUpdate_pwr2_screen7();
	void callLevel_preScreenUpdate_pwr2(int num);

	void callLevel_initialize_pwr2();

	// level7_lar1.cpp
	void preScreenUpdate_lar1_screen0();
	void preScreenUpdate_lar1_screen2();
	void preScreenUpdate_lar1_screen6();
	void preScreenUpdate_lar1_screen11();
	void preScreenUpdate_lar1_screen13();
	void preScreenUpdate_lar1_screen14();
	void preScreenUpdate_lar1_screen16();
	void preScreenUpdate_lar1_screen20();
	void preScreenUpdate_lar1_screen24();
	void callLevel_preScreenUpdate_lar1(int num);

	void callLevel_initialize_lar1();
	void callLevel_tick_lar1();

	void callLevel_setupLvlObjects_lar1(int num);

	// level8_lar2.cpp
	void postScreenUpdate_lar2_screen3();
	void postScreenUpdate_lar2_screen4();
	void postScreenUpdate_lar2_screen5();
	void postScreenUpdate_lar2_screen7();
	void postScreenUpdate_lar2_screen10();
	void callLevel_postScreenUpdate_lar2(int num);

	void preScreenUpdate_lar2_screen2();
	void preScreenUpdate_lar2_screen4();
	void preScreenUpdate_lar2_screen5();
	void preScreenUpdate_lar2_screen6();
	void preScreenUpdate_lar2_screen15();
	void preScreenUpdate_lar2_screen19();
	void callLevel_preScreenUpdate_lar2(int num);

	void callLevel_setupLvlObjects_lar2(int num);

	// level9_dark.cpp
	void postScreenUpdate_dark_screen0();
	void callLevel_postScreenUpdate_dark(int num);

	void preScreenUpdate_dark_screen0();
	void callLevel_preScreenUpdate_dark(int num);

	// monsters.cpp

	void initMstCode();
	void resetMstCode();
	void startMstCode();
	void executeMstCode();

	Task *findFreeTask();
	Task *createTask(const uint8_t *codeData);
	void removeTask(Task **tasksList, Task *t);

	// sound.cpp
	SssObject _sssObjectsTable[32];
	bool _sssObjectsChanged;
	int _sssObjectsCount;
	SssObject *_sssObjectsList1; // playing
	SssObject *_sssObjectsList2; // paused
	SssObject *_lowPrioritySssObject; // point to the object in _sssObjectsList1 with the highest 'priority'
	uint8_t _channelMixingTable[32];
	int _playingSssObjectsMax;
	int _snd_volumeMax;
	int _playingSssObjectsCount;
	int _snd_masterVolume;

	void resetSound();
	void removeSoundObjectFromList(SssObject *so);
	void updateSoundObject(SssObject *so);
	void executeSssCodeOp4(uint32_t flags);
	void executeSssCodeOp12(int num, uint8_t lut, uint8_t c);
	void executeSssCodeOp17(SssObject *so);
	const uint8_t *executeSssCode(SssObject *so, const uint8_t *code);
	SssObject *addSoundObject(SssPcm *pcm, int priority, uint32_t flags_a, uint32_t flags_b);
	void addSoundObjectToList(SssObject *so);
	void updateSoundObjectLut2(uint32_t flags);
	SssObject *createSoundObject(int num, int b, int c);
	SssObject *startSoundObject(int num, int b, int flags);
	void playSoundObject(SssUnk1 *s, int a, int b);
	void clearSoundObjects();
	void setLowPrioritySoundObject(SssObject *so);
	int getSoundObjectVolumeByPos(SssObject *so) const;
	void setSoundObjectVolume(SssObject *so);
	void expireSoundObjects(int flags);
	void mixSoundObjects17640(bool flag);
	void mixSoundObjects();
	void stopSoundObjects(SssObject **sssObjectsList, int num);

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
