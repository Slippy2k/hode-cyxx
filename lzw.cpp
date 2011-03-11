/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009 Gregory Montoir
 */

#include "intern.h"

static const uint32 codeMask[] = { 0, 1, 3, 7, 0xF, 0x1F, 0x3F, 0x7F, 0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF, 0xFFFFFFFF };

enum {
	kCodeWidth = 9
};

struct LzwDecoder {

	uint16 _prefix[4096];
	uint32 _currentCode;
	uint8 _stack[8192];
	const uint8 *_buf;
	uint32 _topSlot;
	uint32 _currentByte;
	uint32 _codeSize;
	uint32 _currentSlot;
	uint32 _bitsLeft;

	void nextCode();
	int decode(uint8 *dst);
};

static struct LzwDecoder _lzw;

void LzwDecoder::nextCode() {
	if (_bitsLeft == 0) {
		_bitsLeft = 8;
		_currentByte = *_buf++;
	}
	_currentCode = _currentByte >> ((8 - _bitsLeft) & 255);
	while (_bitsLeft < _codeSize) {
		_currentByte = *_buf++;
		_currentCode |= _currentByte << (_bitsLeft & 255);
		_bitsLeft += 8;
	}
	assert(_codeSize <= 12);
	_currentCode &= codeMask[_codeSize];
	_bitsLeft -= _codeSize;
}

int LzwDecoder::decode(uint8 *dst) {
	uint8 *dst_ = dst;
	_codeSize = kCodeWidth;
	_topSlot = 1 << kCodeWidth;
	uint8 *stackPtr = &_stack[8191];
	static const uint32 clearCode = 1 << (kCodeWidth - 1);
	static const uint32 endCode = clearCode + 1;
	static const uint32 newCodes = endCode + 1;
	_currentSlot = newCodes;
	uint32 previousCode = 0;
	uint32 lastCode = 0;
	_currentByte = _currentCode = *_buf++;
	_bitsLeft = 8;
	nextCode();
	while (_currentCode != endCode) {
		if (_currentCode == clearCode) {
			_currentSlot = newCodes;
			_codeSize = kCodeWidth;
			_topSlot = 1 << kCodeWidth;
			nextCode();
			while (_currentCode == clearCode) {
				nextCode();
			}
			if (_currentCode == endCode) {
				break;
			}
			if (_currentCode >= newCodes) {
				_currentCode = 0;
			}
			previousCode = _currentCode;
			lastCode = _currentCode;
			*dst_++ = (_currentCode & 255);
		} else {
			uint8 *currentStackPtr = stackPtr;
			uint32 slot = _currentSlot;
			uint32 code = _currentCode;
			if (_currentCode >= slot) {
				code = lastCode;
				currentStackPtr = &_stack[8190];
				_stack[8190] = previousCode & 255;
			}
			while (code >= newCodes) {
				--currentStackPtr;
				assert(currentStackPtr >= &_stack[0]);
				*currentStackPtr = _stack[code];
				code = _prefix[code];
			}
			--currentStackPtr;
			*currentStackPtr = (code & 255);
			if (slot < _topSlot) {
				_stack[slot] = code & 255;
				previousCode = code;
				_prefix[slot] = lastCode & 0xFFFF;
				lastCode = _currentCode;
				++slot;
				_currentSlot = slot;
			}
			if (slot >= _topSlot && _codeSize < 12) {
				_topSlot <<= 1;
				++_codeSize;
			}
			const int count = stackPtr - currentStackPtr;
			memcpy(dst_, currentStackPtr, count);
			dst_ += count;
			currentStackPtr += count;
		}
		nextCode();
	}
	return dst_ - dst;
}

int decodeLZW(const uint8 *src, uint8 *dst) {
	memset(&_lzw, 0, sizeof(_lzw));
	_lzw._buf = src;
	return _lzw.decode(dst);
}

