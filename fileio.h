/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
 */

#ifndef FILEIO_H__
#define FILEIO_H__

#include "intern.h"

struct File {

	static const char *_dataPath;

	FILE *_fp;

	File();
	virtual ~File();

	static void setDataPath(const char *path);
	bool open(const char *filename);
	void close();
	virtual void seekAlign(int pos);
	virtual void seek(int pos, int whence);
	virtual int read(uint8_t *ptr, int size);
	uint8_t readByte();
	uint16_t readUint16();
	uint32_t readUint32();
	int getSize();
	virtual void flush();
};

struct SectorFile : File {

	enum {
		kFioBufferSize = 2048
	};

	uint8_t _buf[kFioBufferSize];
	int _bufPos;
	int _bufLen;

	SectorFile();

	void refillBuffer();
	virtual void seekAlign(int pos);
	virtual void seek(int pos, int whence);
	virtual int read(uint8_t *ptr, int size);
	virtual void flush();
};

#endif // FILEIO_H__

