/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009 Gregory Montoir
 */

#ifndef PAF_PLAYER_H__
#define PAF_PLAYER_H__

#include "intern.h"
#include "fileio.h"

struct PafHeader {
	uint32 preloadFrameBlocksCount;
	uint32 *frameBlocksCountTable;
	uint32 *framesOffsetTable;
	uint32 *frameBlocksOffsetTable;
	uint32 framesCount;
	uint32 frameBlocksCount;
	uint32 startOffset;
	uint32 readBufferSize;
	uint32 maxVideoFrameBlocksCount;
	uint32 maxAudioFrameBlocksCount;
};

struct SystemStub;

struct PafPlayer {

	enum {
		kMaxVideosCount = 50,
		kBufferBlockSize = 2048,
		kVideoWidth = 256,
		kVideoHeight = 192
	};

	bool _skipCutscenes;
	SystemStub *_system;
	File _file;
	int _videoNum;
	uint32 _videoOffset;
	PafHeader _pafHdr;
	int _currentPageBuffer;
	uint8 *_pageBuffers[4];
	uint8 _paletteBuffer[256 * 3];
	uint8 _bufferBlock[kBufferBlockSize];
	uint8 *_demuxVideoFrameBlocks;

	PafPlayer(SystemStub *system);
	~PafPlayer();

	void preload(int num);
	void play(int num);
	void unload(int num);
	void readPafHeader();
	void readPafHeaderTable(uint32 *dst, int count);
	void decodeVideoFrame(const uint8 *src);
	uint8 *getVideoPageOffset(uint8 a, uint8 b);
	void decodeVideoFrameOp0(const uint8 *base, const uint8 *src, uint8 code);
	void decodeVideoFrameOp1(const uint8 *src);
	void decodeVideoFrameOp2(const uint8 *src);
	void decodeVideoFrameOp4(const uint8 *src);
	void mainLoop();
};

#endif // PAF_PLAYER_H__
