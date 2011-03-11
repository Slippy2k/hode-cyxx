/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009 Gregory Montoir
 */

#ifndef FILEIO_H__
#define FILEIO_H__

#include "intern.h"

struct File {

	enum {
		kMaxFilePathLen = 512
	};

	static const char *_dataPath;

	FILE *_fp;

	File();
	virtual ~File();

	static void setDataPath(const char *path);
	bool open(const char *filename);
	void close();
	virtual void seekAlign(int pos);
	virtual void seek(int pos, int whence);
	virtual int read(uint8 *ptr, int size);
	uint8 readByte();
	uint16 readUint16();
	uint32 readUint32();
	int getSize();
	virtual void flush();
};

struct SectorFile : File {

	enum {
		kFioBufferSize = 2048
	};

	uint8 _buf[kFioBufferSize];
	int _bufPos;
	int _bufLen;

	SectorFile();

	void refillBuffer();
	virtual void seekAlign(int pos);
	virtual void seek(int pos, int whence);
	virtual int read(uint8 *ptr, int size);
	virtual void flush();
};

#endif // FILEIO_H__

