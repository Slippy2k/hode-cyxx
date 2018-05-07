
#ifndef FILE_H__
#define FILE_H__

#include "intern.h"

struct File_impl;

struct File {
	File();
	~File();

	File_impl *_impl;

	bool open(const char *filename, const char *mode);
	void close();
	bool ioErr() const;
	uint32_t size();
	void seek(int32_t off, int whence = SEEK_SET);
	void read(void *ptr, uint32_t len);
	uint8_t readByte();
	uint16_t readUint16LE();
	uint32_t readUint32LE();
	uint16_t readUint16BE();
	uint32_t readUint32BE();
	void write(const void *ptr, uint32_t size);
	void writeByte(uint8_t b);
	void writeUint16LE(uint16_t n);
	void writeUint32LE(uint32_t n);
	void writeUint16BE(uint16_t n);
	void writeUint32BE(uint32_t n);
};

#endif // FILE_H__
