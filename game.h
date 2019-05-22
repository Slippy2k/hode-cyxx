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
		kObjectDataTypeShoot,
		// LvlObject.type == 1
		kObjectDataTypeLvlBackgroundSound,
		kObjectDataTypeMonster
	};
	enum {
		kMaxScreens = 40,
		kMaxTasks = 128,
		kMaxVars = 40,
		kMaxLocals = 8,
		kFrameTimeStamp = 50 // 80
	};

	static const uint8_t _pointDstIndexTable[];
	static const uint8_t _pointRandomizeShiftTable[];
	static const uint8_t _pointSrcIndex2Table[];
	static const uint8_t _pointSrcIndex1Table[];
	static const uint8_t _actionDirectionKeyMaskTable[];
	static const uint8_t *_levelCheckpointData[];
	static const uint8_t *_levelScreenStartData[];
	static const uint8_t _pwr1_screenTransformData[];
	static const uint8_t _pwr2_screenTransformData[];
	static const uint8_t _pwr1_screenTransformLut[];
	static const uint8_t _lava_screenTransformLut[];
	static const uint8_t _pwr2_screenTransformLut[];

	Mixer _mix;
	PafPlayer *_paf;
	Random _rnd;
	Resource *_res;
	Video *_video;
	SystemStub *_system;

	int _difficulty;
	LvlObject *_screenLvlObjectsList[kMaxScreens]; // LvlObject linked list for each screen
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
	int8_t _fadePaletteCounter;
	bool _fadePalette;
	bool _hideAndyObjectSprite;
	ShootLvlObjectData _otherObjectScreenDataTable[32];
	ShootLvlObjectData *_otherObjectScreenDataList; // pointer to the first 'free' element
	LvlObject *_lvlObjectsList0;
	LvlObject *_lvlObjectsList1;
	LvlObject *_lvlObjectsList2;
	LvlObject *_lvlObjectsList3;
	uint8_t _screenCounterTable[kMaxScreens];
	uint8_t _screenPosTable[5][24 * 32];
	uint8_t _screenTempMaskBuffer[24 * 32];
	uint8_t _screenMaskBuffer[96 * 24 * 32];
	int _andyCurrentLevelScreenNum;
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
	uint8_t _actionDirectionKeyMaskCounter;
	bool _fallingAndyFlag;
	uint8_t _fallingAndyCounter;
	uint8_t _actionDirectionKeyMaskIndex;
	uint8_t _andyActionKeyMaskAnd, _andyActionKeyMaskOr;
	uint8_t _andyDirectionKeyMaskAnd, _andyDirectionKeyMaskOr;
	int32_t _gameXPosTable[129];
	int32_t _gameYPosTable[129];
	int32_t _plasmaCannonXPointsTable1[129];
	int32_t _plasmaCannonYPointsTable1[129];
	int32_t _plasmaCannonXPointsTable2[127];
	int32_t _plasmaCannonYPointsTable2[127];
	ScreenMask _shadowScreenMasksTable[8];

	uint16_t _mstCurrentAnim;
	uint16_t _mstCurrentFlags1;
	uint8_t _mstCurrentScreenNum;
	int16_t _mstOriginPosX;
	int16_t _mstOriginPosY;
	bool _mstCurrentUnkFlag;
	int _mstUnk10;
	MovingOpcodeState _mstMovingState[8];
	int _mstMovingStateCount;
	uint8_t _mstOp68_arg8, _mstOp68_arg9, _mstOp67_type;
	uint8_t _mstOp67_flags1;
	uint16_t _mstOp67_unk;
	int _mstOp67_x1, _mstOp67_x2, _mstOp67_y1, _mstOp67_y2;
	uint8_t _mstOp67_screenNum;
	uint16_t _mstOp68_flags1;
	int _mstOp68_x1, _mstOp68_x2, _mstOp68_y1, _mstOp68_y2;
	uint8_t _mstOp68_screenNum;
	uint32_t _mstLogicHelper1TestValue;
	uint32_t _mstLogicHelper1TestMask;
	int _runTaskOpcodesCount;
	int32_t _mstVars[kMaxVars];
	uint32_t _mstFlags;
	int _clipBoxOffsetX, _clipBoxOffsetY;
	Task *_currentTask;
	int _mstOp54Counter;
	int _mstOp56Counter;
	uint8_t _mstOp54Table[32];
	bool _mstLogicDisabled;
	LvlObject _declaredLvlObjectsList[160];
	LvlObject *_declaredLvlObjectsListHead;
	int _declaredLvlObjectsListCount;
	AndyLvlObjectData _andyObjectScreenData;
	AnimBackgroundData _animBackgroundDataTable[64];
	int _animBackgroundDataCount;
	uint8_t _andyActionKeysFlags;
	int _executeMstLogicCounter;
	int _executeMstLogicPrevCounter;
	Task _tasksTable[kMaxTasks];
	Task *_tasksList;
	Task *_mstTasksList1;
	Task *_mstTasksList2;
	Task *_mstTasksList3;
	Task *_mstTasksList4;
	int _mstPrevPosX;
	int _mstPrevPosY;
	int _mstPosX;
	int _mstPosY;
	int _mstRefPosX;
	int _mstRefPosY;
	int _mstOp54Unk1;
	int _mstOp54Unk2;
	int _mstOp54Unk3;
	int _mstUnk6;
	int _mstTaskDataCount;
	int _mstPosXmin, _mstPosXmax;
	int _mstPosYmin, _mstPosYmax;
	MstTaskData _mstUnkDataTable[32];
	MstObject _mstObjectsTable[64];
	int _mstTickDelay;
	uint8_t _mstCurrentActionKeyMask;
	uint8_t _mstRandomLookupTable[8][32];
	int _xMstPos1, _xMstPos2;
	int _mstCurrentPosX, _mstCurrentPosY;
	int _mstRectsCount;
	MstRect _mstRectsTable[64];
	Task *_mstCurrentTask;
	MstCollision _mstCollisionTable[2][32];

	Game(SystemStub *system, const char *dataPath);
	~Game();

	// benchmark.cpp
	uint32_t benchmarkLoop(const uint8_t *p, int count);
	uint32_t benchmarkCpu();

	// game.cpp

	void mainLoop(int level, int checkpoint);
	void mixAudio(int16_t *buf, int len);
	void resetObjectScreenDataList();
	void clearObjectScreenData(LvlObject *ptr);
	void setShakeScreen(int type, int counter);
	void fadeScreenPalette();
	void shakeScreen();
	void transformShadowLayer(int delta);
	void decodeShadowScreenMask(LvlBackgroundData *lvl);
	void playSound(int num, LvlObject *ptr, int a, int b);
	void removeSound(LvlObject *ptr);
	void setupBackgroundBitmap();
	void addToSpriteList(LvlObject *ptr);
	int16_t calcScreenMaskDy(int16_t xPos, int16_t yPos, int num);
	void setupScreenPosTable(uint8_t num);
	void setupScreenMask(uint8_t num);
	void resetScreenMask();
	void setScreenMaskRectHelper(int x1, int y1, int x2, int y2, int screenNum);
	void setScreenMaskRect(int x1, int y1, int x2, int y2, int pos);
	void updateScreenMaskBuffer(int x, int y, int type);
	void setupLvlObjectBitmap(LvlObject *ptr);
	void randomizeInterpolatePoints(int32_t *pts, int count);
	int fixPlasmaCannonPointsScreenMask(int num);
	void setupPlasmaCannonPointsHelper();
	void destroyLvlObjectPlasmaExplosion(LvlObject *o);
	void shuffleArray(uint8_t *p, int count);
	void shuffleDword(uint8_t *p);
	uint8_t shuffleFlags(uint8_t *p);
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
	void removeLvlObject(LvlObject *ptr);
	void removeLvlObject2(LvlObject *o);
	void setupCurrentScreen(); // setAndyFromCheckpoint
	void updateScreenHelper(int num);
	void resetDisplay();
	void updateScreen(uint8_t num);
	void resetScreen();
	void restartLevel();
	void playAndyFallingCutscene(int type);
	int8_t updateLvlObjectScreen(LvlObject *ptr);
	void setAndyLvlObjectPlasmaCannonKeyMask();
	int setAndySpecialAnimation(uint8_t mask);
	int clipBoundingBox(BoundingBox *coords, BoundingBox *box);
	int updateBoundingBoxClippingOffset(BoundingBox *_ecx, BoundingBox *_ebp, const uint8_t *coords, int direction);
	int clipLvlObjectsBoundingBoxHelper(LvlObject *o1, BoundingBox *box1, LvlObject *o2, BoundingBox *box2);
	int clipLvlObjectsBoundingBox(LvlObject *o, LvlObject *ptr, int count);
	int clipLvlObjectsSmall(LvlObject *o, LvlObject *ptr, int count);
	int updateAndyLvlObject();
	void drawPlasmaCannon();
	void drawScreen();
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
	void *getLvlObjectDataPtr(LvlObject *o, int type) const;
	void lvlObjectType0Init(LvlObject *ptr);
	void lvlObjectType1Init(LvlObject *ptr);
	void lvlObjectTypeInit(LvlObject *ptr);
	void lvlObjectType0CallbackHelper1();
	int calcScreenMaskDx(int x, int y, int num);
	void lvlObjectType0CallbackHelper3(LvlObject *ptr);
	void setupSpecialPowers(LvlObject *ptr);
	int lvlObjectType0Callback(LvlObject *ptr);
	int lvlObjectType1Callback(LvlObject *ptr);
	int lvlObjectType8Callback(LvlObject *o);
	int lvlObjectList3Callback(LvlObject *o);
	void lvlObjectSpecialPowersCallbackHelper(LvlObject *o);
	int lvlObjectSpecialPowersCallback(LvlObject *o);
	void lvlObjectTypeCallback(LvlObject *o);
	LvlObject *addLvlObject(int type, int x, int y, int screen, int num, int o_anim, int o_flags1, int o_flags2, int actionKeyMask, int directionKeyMask);
	int setLvlObjectPosInScreenGrid(LvlObject *o, int num);
	LvlObject *declareLvlObject(uint8_t type, uint8_t num);
	void clearDeclaredLvlObjectsList();
	void initLvlObjects();
	void setLvlObjectType8Resource(LvlObject *ptr, uint8_t _dl, uint8_t num);
	LvlObject *findLvlObject(uint8_t type, uint8_t num, int index);
	LvlObject *findLvlObject2(uint8_t type, uint8_t flags, int index);
	LvlObject *findLvlObjectType2(int num, int index);
	LvlObject *findLvlObjectBoundingBox(BoundingBox *box);
	void resetLevelTickHelperData();
	void updateLevelTickHelper();
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
	void postScreenUpdate_pwr1_helper(BoundingBox *b, int dx, int dy);
	void postScreenUpdate_pwr1_screen6();
	void postScreenUpdate_pwr1_screen10();
	void postScreenUpdate_pwr1_screen12();
	void postScreenUpdate_pwr1_screen14();
	void postScreenUpdate_pwr1_screen16();
	void postScreenUpdate_pwr1_screen18();
	void postScreenUpdate_pwr1_screen23();
	void postScreenUpdate_pwr1_screen27();
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
	void preScreenUpdate_pwr1_screen27();
	void preScreenUpdate_pwr1_screen29();
	void preScreenUpdate_pwr1_screen31();
	void preScreenUpdate_pwr1_screen35();
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

	void preScreenUpdate_isld_screen1();
	void preScreenUpdate_isld_screen2();
	void preScreenUpdate_isld_screen3();
	void preScreenUpdate_isld_screen9();
	void preScreenUpdate_isld_screen14();
	void preScreenUpdate_isld_screen15();
	void preScreenUpdate_isld_screen16();
	void preScreenUpdate_isld_screen21();
	void callLevel_preScreenUpdate_isld(int num);

	void callLevel_initialize_isld();
	void callLevel_tick_isld();
	void callLevel_terminate_isld();

	// level5_lava.cpp
	void postScreenUpdate_lava_helper(int yPos);
	void postScreenUpdate_lava_screen0();
	void postScreenUpdate_lava_screen4();
	void postScreenUpdate_lava_screen5();
	void postScreenUpdate_lava_screen6();
	void postScreenUpdate_lava_screen7();
	void postScreenUpdate_lava_screen8();
	void postScreenUpdate_lava_screen10();
	void postScreenUpdate_lava_screen11();
	void postScreenUpdate_lava_screen12();
	void postScreenUpdate_lava_screen13();
	void postScreenUpdate_lava_screen14();
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
	void callLevel_tick_pwr2();

	// level7_lar1.cpp
	void postScreenUpdate_lar1_helper(LvlObject *o, uint8_t *p, int num);

	void postScreenUpdate_lar1_screen0();
	void postScreenUpdate_lar1_screen3();
	void postScreenUpdate_lar1_screen4();
	void postScreenUpdate_lar1_screen5();
	void postScreenUpdate_lar1_screen8();
	void postScreenUpdate_lar1_screen9();
	void postScreenUpdate_lar1_screen13();
	void postScreenUpdate_lar1_screen15();
	void postScreenUpdate_lar1_screen16();
	void postScreenUpdate_lar1_screen19();
	void postScreenUpdate_lar1_screen20();
	void postScreenUpdate_lar1_screen22();
	void postScreenUpdate_lar1_screen24();
	void callLevel_postScreenUpdate_lar1(int num);

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
	bool postScreenUpdate_lar2_screen2_helper(BoundingBox *b);
	void postScreenUpdate_lar2_screen2();
	void postScreenUpdate_lar2_screen3();
	void postScreenUpdate_lar2_screen4();
	void postScreenUpdate_lar2_screen5();
	void postScreenUpdate_lar2_screen7();
	void postScreenUpdate_lar2_screen10();
	void postScreenUpdate_lar2_screen19();
	void callLevel_postScreenUpdate_lar2(int num);

	void preScreenUpdate_lar2_screen2();
	void preScreenUpdate_lar2_screen4();
	void preScreenUpdate_lar2_screen5();
	void preScreenUpdate_lar2_screen6();
	void preScreenUpdate_lar2_screen7();
	void preScreenUpdate_lar2_screen15();
	void preScreenUpdate_lar2_screen19();
	void callLevel_preScreenUpdate_lar2(int num);

	void callLevel_tick_lar2();

	void callLevel_setupLvlObjects_lar2(int num);

	// level9_dark.cpp
	void postScreenUpdate_dark_screen0();
	void callLevel_postScreenUpdate_dark(int num);

	void preScreenUpdate_dark_screen0();
	void callLevel_preScreenUpdate_dark(int num);

	// monsters.cpp

	void resetMstTaskData(MstTaskData *m);
	void initMstTaskData(MstTaskData *m);
	int addMstTaskData(MstUnk48 *m48, uint8_t flag);
	void disableMstTaskData(MstTaskData *m);
	void initMstTaskDataType2(Task *t);
	bool updateMstTaskDataPositionHelper(MstTaskData *m);
	bool updateMstTaskDataPosition(MstTaskData *m);
	void resetMstObject(MstObject *m);
	int prepareMstTask(Task *t);
	void clearMstRectsTable(MstTaskData *m, int num);
	int resetMstRectsTable(int num, int x1, int y1, int x2, int y2);
	int updateMstRectsTable(int num, int a, int x1, int y1, int x2, int y2);
	int checkMstRectsTable(int num, int x1, int y1, int x2, int y2);
	void setMstObjectDefaultPos(Task *t);
	void setMstTaskDataDefaultPos(Task *t);
	void shuffleMstUnk43(MstUnk43 *p);

	void initMstCode();
	void resetMstCode();
	void startMstCode();
	void executeMstCode();
	void executeMstCodeHelper2();
	void executeMstUnk10(LvlObject *o, const uint8_t *ptr, uint8_t mask1, uint8_t mask2);
	bool executeMstUnk17(MstTaskData *m, int num);
	bool executeMstUnk19(LvlObject *o, int type);
	bool executeMstUnk21(LvlObject *o, int type);
	bool executeMstUnk28(LvlObject *o, int type) const;
	bool executeMstUnk22(LvlObject *o, int type);
	bool executeMstUnk20(MstTaskData *m, uint32_t flags);
	bool executeMstUnk27(MstTaskData *m, const uint8_t *p);
	int executeMstCodeHelper3(Task *t);
	int executeMstCodeHelper4(Task *t);
	void updateMstMoveData();
	void updateMstHeightMapData();

	void removeMstObjectTask(Task *t, Task **tasksList);
	void resetMstTask(Task *t, uint32_t codeData, uint8_t flags);
	void stopMstTaskData(Task *t, Task **tasksList);
	Task *findFreeTask();
	Task *createTask(const uint8_t *codeData);
	int changeTask(Task *t, int num, int value);
	void updateTask(Task *t, int num, const uint8_t *codeData);
	void resetTask(Task *t, const uint8_t *codeData);
	void removeTask(Task **tasksList, Task *t);
	void appendTask(Task **tasksList, Task *t);
	int getTaskVar(Task *t, int index, int type) const;
	void setTaskVar(Task *t, int index, int type, int value);
	int getTaskAndyVar(int index, Task *t) const;
	int getTaskOtherVar(int index, Task *t) const;
	int getTaskFlag(Task *t, int index, int type) const;
	int runTask_default(Task *t);
	void executeMstOp26(Task **tasksList, int screenNum);
	void executeMstOp27(Task **tasksList, int num, int arg);
	int executeMstOp49(int a, int b, int c, int d, int screen, Task *t, int num);
	void executeMstOp52();
	int checkMstOp54Helper(MstUnk48 *m, uint8_t flag);
	void executeMstOp54();
	int executeMstOp56(Task *t, int code, int num);
	void executeMstOp58(Task *t, int num);
	void executeMstUnk1(Task *t);
	int executeMstUnk2(MstTaskData *m, int x, int y);
	void executeMstUnk7(MstTaskData *m);
	void executeMstUnk12();
	void executeMstUnk13(Task *t);
	int executeMstOp67Type1(Task *t);
	int executeMstOp67Type2(Task *t, int flag);
	void executeMstOp67(Task *t, int y1, int y2, int x1, int x2, int screen, int type, int o_flags1, int o_flags2, int arg1C, int arg20, int arg24);
	int runTask_waitResetInput(Task *t);
	int runTask_wait(Task *t);
	int runTask_waitFlags(Task *t);
	int runTask_idle(Task *t);
	int runTask_mstOp231(Task *t);
	int runTask_mstOp232(Task *t);
	int runTask_mstOp233(Task *t);
	int runTask_mstOp234(Task *t);
	int runTask_unk1(Task *t);
	int runTask_unk2(Task *t);
	int runTask_unk3(Task *t);
	int runTask_unk4(Task *t);

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
	void executeSssCodeOp16(SssObject *so);
	void executeSssCodeOp17(SssObject *so);
	const uint8_t *executeSssCode(SssObject *so, const uint8_t *code, bool tempSssObject = false);
	SssObject *addSoundObject(SssPcm *pcm, int priority, uint32_t flags_a, uint32_t flags_b);
	void prependSoundObjectToList(SssObject *so);
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
