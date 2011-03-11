/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009 Gregory Montoir
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
inline T *PTR(void *base, const uint8 *offsets, int index) {
	const uint32 offset = READ_LE_UINT32((const uint8 *)offsets + index * 4);
	return offset == 0xFFFFFFFF ? 0 : (T *)(((const uint8 *)base) + offset * sizeof(T));
}

template <typename T>
inline const uint8 *PTR_OFFS(const uint8 *base, uint32 index) {
	return index == 0xFFFFFFFF ? 0 : (const uint8 *)base + index * sizeof(T);
}

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
		kFrameTimeStamp = 50 // 80
	};

	static const uint8 _gameLevelStartingCutscene[]; // _levelStartingCutsceneTable
	static const uint8 _pointDstIndexTable[];
	static const uint8 _pointRandomizeShiftTable[];
	static const uint8 _pointSrcIndex2Table[];
	static const uint8 _pointSrcIndex1Table[];
	static const uint8 _actionDirectionKeyMaskTable[];
	static const char *_resLevelNames[];
	static const uint8 _levelOpStage3ImageData1[];
	static const uint8 _levelOpStage3ImageData2[];
	static const uint8 *_levelUpdateData1[];
	static const uint8 *_levelUpdateData2[];
	static const OpStage1Proc _level1OpStage1[];
	static const OpStage1Proc _level2OpStage1[];
	static const OpStage1Proc _level3OpStage1[];

	PafPlayer *_paf;
	Random _rnd;
	Resource *_res;
	Video *_video;
	SystemStub *_system;

	LvlObject *_andyObject;
	LvlObject *_plasmaExplosionObject;
	LvlObject *_plasmaCannonObject;
	LvlObject *_currentMonsterObject;
	int _currentLevel;
	int _currentScreenResourceState; // _res->_currentScreenResourceState
	bool _quit;
	Sprite _gameSpriteListTable[128];
	Sprite *_gameSpriteListHead;
	Sprite *_gameSpriteListPtrTable[32];
	uint16 _fadePaletteBuffer[256 * 3];
	uint8 *_shadowScreenMaskBuffer;
	uint8 _directionKeyMask;
	uint8 _actionKeyMask;
	uint8 _currentRightScreen; // _res->rightScreenResourceNum
	uint8 _currentLeftScreen; // _res->leftScreenResourceNum
	uint8 _gameResData0x2E08; // _res->nextScreenResourceNum
	uint8 _fadePaletteCounter;
	uint8 _fadePalette;
	bool _hideAndyObjectSprite;
	GameUnkList1 _gameUnkList1Table[32];
	GameUnkList1 *_gameUnkList1Head;
	int32 _globalVars[40];
	LvlObject *_lvlObjectsList0;
	LvlObject *_lvlObjectsList1;
	LvlObject *_lvlObjectsList2;
	LvlObject *_lvlObjectsList3;
	uint8 _screenCounterTable[40];
	uint8 _screenPosTable[5][24 * 32];
	uint8 _screenTempMaskBuffer[24 * 32];
	uint8 _screenMaskBuffer[96 * 24 * 32];
	int _gameCurrentData0x2E08Num;
	uint8 _shakeScreenDuration;
	const uint8 *_shakeScreenTable;
	uint8 _plasmaCannonDirection;
	uint8 _plasmaCannonPrevDirection;
	uint8 _plasmaCannonPointsSetupCounter;
	uint8 _plasmaCannonLastIndex1;
	uint8 _gameMainLoopFlag5;
	uint8 _gameMainLoopFlag6;
	uint8 _plasmaCannonFirstIndex;
	uint8 _plasmaCannonLastIndex2;
	uint8 _plasmaCannonFlags;
	uint8 _plasmaCannonKeyMaskCounter;
	uint8 _gameResType0CallbackFlag1;
	uint8 _gameResType0CallbackFlag2;
	uint8 _gameKeyPressedMaskIndex;
	uint8 _andyActionKeyMaskAnd, _andyActionKeyMaskOr;
	uint8 _andyDirectionKeyMaskAnd, _andyDirectionKeyMaskOr;
	int32 _gameXPosTable[129];
	int32 _gameYPosTable[129];
	int32 _plasmaCannonXPointsTable1[129];
	int32 _plasmaCannonYPointsTable1[129];
	int32 _plasmaCannonXPointsTable2[127];
	int32 _plasmaCannonYPointsTable2[127];
	uint16 _gameMstAnim;
	uint16 _gameMstAnimFlags1;
	int16 _screen_dx;
	int16 _screen_dy;
	ScreenMask _shadowScreenMasksTable[8];

	uint32 _gameMstLogicHelper1TestValue;
	uint32 _gameMstLogicHelper1TestMask;
	Task *_tasksListTail;
	int _runTaskOpcodesCount;
	uint32 _mstGlobalFlags;
	int _clipBoxOffsetX, _clipBoxOffsetY;
	Task *_gameMstResToLoad1Pri, *_gameMstResToLoad2Pri;
	Task *_currentTask;
	int _mstOp54Counter;
	uint8 _mstOp54Table[32];
	bool _mstLogicDisabled;
	LvlObject _declaredLvlObjectsList[160];
	LvlObject *_declaredLvlObjectsListHead;
	int _declaredLvlObjectsListCount;
	AndyObjectScreenData _andyObjectScreenData;
	GameRect _gameRectsListTable[64];
	int _gameRectsListCount;
	uint8 _resUpdateLevelDataType0Flag;
	int _executeMstLogicCounter;
	int _executeMstLogicLastCounter;
	int _gameMstScreenRefPosX, _gameMstScreenRefPosY;
	int _gameMstMovingStatePosX, _gameMstMovingStatePosY;
	int _gameMstObjectRefPointPosX, _gameMstObjectRefPointPosY;

	Game(SystemStub *system, const char *dataPath);
	~Game();

	void mainLoop(int level = 0);
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
	int16 fixScreenPos(int16 xPos, int16 yPos, int num);
	void setupScreenPosTable(uint8 num);
	void setupScreenMask(uint8 num);
	void resetScreenMask();
	void setupLvlObjectBitmap(LvlObject *ptr);
	void randomizeInterpolatePoints(int32 *pts, int count);
	int fixPlasmaCannonPointsScreenMask(int num);
	void setupPlasmaCannonPointsHelper();
	void destroyLvlObjectUnk(LvlObject *o);
	void shuffleArray(uint8 *p, int count);
	void destroyLvlObject(LvlObject *o);
	void setupPlasmaCannonPoints(LvlObject *ptr);
	int testPlasmaCannonPointsDirection(int x1, int y1, int x2, int y2);
	void ResPreloadLevelData(int num, int prev); // RENAME: Resource::preloadLevelData
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
	void resetScreenHelper(int num);
	void resetDisplay();
	void resetScreen(uint8 num);
	void resetLevel();
	void restartLevel();
	void playAndyFallingCutscene(int type);
	int8 updateLvlObjectScreen(LvlObject *ptr);
	void setAndyLvlObjectPlasmaCannonKeyMask();
	int game_unk27(uint8 mask);
	int clipBoundingBox(BoundingBox *coords, BoundingBox *box);
	int updateBoundingBoxClippingOffset(BoundingBox *_ecx, BoundingBox *_ebp, const uint8 *coords, int direction);
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
	void callLevelOpStage0(int num);
	int callLevelOpStage1(int num, LvlObject *o);
	void callLevelOpStage2(int num);
	void callLevelOpStage3();
	void callLevelOpStage4();
	void callLevelOpStage5();
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
	LvlObject *declareLvlObject(uint8 type, uint8 num);
	void clearDeclaredLvlObjectsList();
	void initLvlObjects();
	void setLvlObjectType8Resource(LvlObject *ptr, uint8 _dl, uint8 num);

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
	void level1OpHelper1(LvlObject *ptr, uint8 *p);
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

	// andy.cpp

	AndyMoveData _andyMoveData;
	int32 _andyPosX;
	int32 _andyPosY;
	uint8 _andyMoveMask;
	uint8 _andyUpdatePositionFlag;
	const Point16 *_andyLevelData0x288PosTablePtr;
	uint8 _andyMoveState[32];
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
