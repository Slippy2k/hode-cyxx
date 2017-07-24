
#ifndef __FILE_H__
#define __FILE_H__

#include "intern.h"

struct File_impl;

struct File {
	File();
	~File();

	File_impl *_impl;

	bool open(const char *filename, const char *mode);
	void close();
	bool ioErr() const;
	uint32 size();
	void seek(int32 off, int whence = SEEK_SET);
	void read(void *ptr, uint32 len);
	uint8 readByte();
	uint16 readUint16LE();
	uint32 readUint32LE();
	uint16 readUint16BE();
	uint32 readUint32BE();
	void write(const void *ptr, uint32 size);
	void writeByte(uint8 b);
	void writeUint16LE(uint16 n);
	void writeUint32LE(uint32 n);
	void writeUint16BE(uint16 n);
	void writeUint32BE(uint32 n);
};

#endif // __FILE_H__
