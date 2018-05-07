/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
 */

#ifndef PAF_PLAYER_H__
#define PAF_PLAYER_H__

#include "intern.h"
#include "fileio.h"

struct PafHeader {
	uint32_t preloadFrameBlocksCount;
	uint32_t *frameBlocksCountTable;
	uint32_t *framesOffsetTable;
	uint32_t *frameBlocksOffsetTable;
	uint32_t framesCount;
	uint32_t frameBlocksCount;
	uint32_t startOffset;
	uint32_t readBufferSize;
	uint32_t maxVideoFrameBlocksCount;
	uint32_t maxAudioFrameBlocksCount;
};

struct SystemStub;

struct PafPlayer {

	enum {
		kMaxVideosCount = 50,
		kBufferBlockSize = 2048,
		kVideoWidth = 256,
		kVideoHeight = 192,
		kPageBufferSize = 256 * 256,
	};

	bool _skipCutscenes;
	SystemStub *_system;
	File _file;
	int _videoNum;
	uint32_t _videoOffset;
	PafHeader _pafHdr;
	int _currentPageBuffer;
	uint8_t *_pageBuffers[4];
	uint8_t _paletteBuffer[256 * 3];
	uint8_t _bufferBlock[kBufferBlockSize];
	uint8_t *_demuxVideoFrameBlocks;

	PafPlayer(SystemStub *system);
	~PafPlayer();

	void preload(int num);
	void play(int num);
	void unload(int num);
	void readPafHeader();
	void readPafHeaderTable(uint32_t *dst, int count);
	void decodeVideoFrame(const uint8_t *src);
	uint8_t *getVideoPageOffset(uint8_t a, uint8_t b);
	void decodeVideoFrameOp0(const uint8_t *base, const uint8_t *src, uint8_t code);
	void decodeVideoFrameOp1(const uint8_t *src);
	void decodeVideoFrameOp2(const uint8_t *src);
	void decodeVideoFrameOp4(const uint8_t *src);
	void mainLoop();
};

#endif // PAF_PLAYER_H__
