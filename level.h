
#ifndef LEVEL_H__
#define LEVEL_H__

struct CheckpointData;
struct Game;
struct LvlObject;
struct Resource;
struct PafPlayer;
struct Video;

struct Level {
	virtual ~Level() {
	}

	void setPointers(Game *g, LvlObject *andyObject, PafPlayer *paf, Resource *r, Video *video) {
		_g = g;
		_andyObject = andyObject;
		_paf = paf;
		_res = r;
		_video = video;
	}

	//const CheckpointData *getCheckpointData() = 0;

	virtual void initialize() {}
	virtual void terminate() {}
	virtual void tick() {}
	virtual void preScreenUpdate(int screenNum) = 0;
	virtual void postScreenUpdate(int screenNum) = 0;
	virtual void setupLvlObjects(int screenNum) {}

	Game *_g;
	LvlObject *_andyObject;
	PafPlayer *_paf;
	Resource *_res;
	Video *_video;
};

Level *Level_rock_create();
Level *Level_fort_create();

#endif
