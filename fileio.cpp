/*
 * Heart of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir (cyx@users.sourceforge.net)
 */

#include <sys/param.h>
#include "fileio.h"
#include "util.h"

File::File()
	: _fp(0) {
}

File::~File() {
}

void File::setFp(FILE *fp) {
	_fp = fp;
}

void File::seekAlign(int pos) {
	fseek(_fp, pos, SEEK_SET);
}

void File::seek(int pos, int whence) {
	fseek(_fp, pos, whence);
}

int File::read(uint8_t *ptr, int size) {
	return fread(ptr, 1, size, _fp);
}

uint8_t File::readByte() {
	uint8_t buf;
	read(&buf, 1);
	return buf;
}

uint16_t File::readUint16() {
	uint8_t buf[2];
	read(buf, 2);
	return READ_LE_UINT16(buf);
}

uint32_t File::readUint32() {
	uint8_t buf[4];
	read(buf, 4);
	return READ_LE_UINT32(buf);
}

void File::flush() {
}

SectorFile::SectorFile() {
	memset(_buf, 0, sizeof(_buf));
	_bufPos = 2044;
}

int fioAlignSizeTo2048(int size) {
	return ((size + 2043) / 2044) * 2048;
}

uint32_t fioUpdateCRC(uint32_t sum, const uint8_t *buf, uint32_t size) {
	assert((size & 3) == 0);
	size >>= 2;
	while (size--) {
		sum ^= READ_LE_UINT32(buf); buf += 4;
	}
	return sum;
}

void SectorFile::refillBuffer() {
	const int size = fread(_buf, 1, 2048, _fp);
	assert(size == 2048);
	const uint32_t crc = fioUpdateCRC(0, _buf, 2048);
	assert(crc == 0);
	_bufPos = 0;
}

void SectorFile::seekAlign(int pos) {
	pos += (pos / 2048) * 4;
	const int alignPos = (pos / 2048) * 2048;
	fseek(_fp, alignPos, SEEK_SET);
	refillBuffer();
	const int skipCount = pos - alignPos;
	_bufPos += skipCount;
}

void SectorFile::seek(int pos, int whence) {
	if (whence == SEEK_SET) {
		assert((pos & 2047) == 0);
		_bufPos = 2044;
		File::seek(pos, SEEK_SET);
	} else {
		assert(whence == SEEK_CUR && pos >= 0);
		const int bufLen = 2044 - _bufPos;
		if (pos < bufLen) {
			_bufPos += pos;
		} else {
			pos -= bufLen;
			const int count = (fioAlignSizeTo2048(pos) / 2048) - 1;
			if (count > 0) {
				const int alignPos = count * 2048;
				fseek(_fp, alignPos, SEEK_CUR);
			}
			refillBuffer();
			_bufPos = pos % 2044;
		}
	}
}

int SectorFile::read(uint8_t *ptr, int size) {
	const int bufLen = 2044 - _bufPos;
	if (size >= bufLen) {
		if (bufLen) {
			memcpy(ptr, _buf + _bufPos, bufLen);
			ptr += bufLen;
			size -= bufLen;
		}
		const int count = (fioAlignSizeTo2048(size) / 2048) - 1;
		for (int i = 0; i < count; ++i) {
			refillBuffer();
			memcpy(ptr, _buf, 2044);
			ptr += 2044;
			size -= 2044;
		}
		refillBuffer();
	}
	if (size != 0) {
		assert(size <= 2044 - _bufPos);
		memcpy(ptr, _buf + _bufPos, size);
		_bufPos += size;
	}
	return 0;
}

void SectorFile::flush() {
	const int currentPos = ftell(_fp);
	assert((currentPos & 2047) == 0);
	_bufPos = 2044;
}
