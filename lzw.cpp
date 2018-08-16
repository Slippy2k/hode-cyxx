/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
 */

#include "intern.h"

enum {
	kCodeWidth = 9,
	kClearCode = 1 << (kCodeWidth - 1),
	kEndCode = kClearCode + 1,
	kNewCodes = kEndCode + 1
};

struct LzwDecoder {

	uint16_t _prefix[4096];
	uint8_t _stack[8192];
	const uint8_t *_buf;
	uint8_t _currentByte;
	uint32_t _currentCode;
	uint8_t _codeSize;
	uint8_t _bitsLeft;

	void nextCode();
	int decode(uint8_t *dst);
};

static struct LzwDecoder _lzw;

void LzwDecoder::nextCode() {
	if (_bitsLeft == 0) {
		_bitsLeft = 8;
		_currentByte = *_buf++;
	}
	_currentCode = _currentByte >> (8 - _bitsLeft);
	while (_bitsLeft < _codeSize) {
		_currentByte = *_buf++;
		_currentCode |= _currentByte << _bitsLeft;
		_bitsLeft += 8;
	}
	_currentCode &= (1 << _codeSize) - 1;
	_bitsLeft -= _codeSize;
}

int LzwDecoder::decode(uint8_t *dst) {
	uint8_t *p = dst;
	_codeSize = kCodeWidth;
	uint32_t topSlot = 1 << kCodeWidth;
	uint8_t *stackPtr = &_stack[8191];
	uint32_t currentSlot = kNewCodes;
	uint32_t previousCode = 0;
	uint32_t lastCode = 0;
	_currentByte = _currentCode = *_buf++;
	_bitsLeft = 8;
	nextCode();
	while (_currentCode != kEndCode) {
		if (_currentCode == kClearCode) {
			currentSlot = kNewCodes;
			_codeSize = kCodeWidth;
			topSlot = 1 << kCodeWidth;
			nextCode();
			while (_currentCode == kClearCode) {
				nextCode();
			}
			if (_currentCode == kEndCode) {
				break;
			}
			if (_currentCode >= kNewCodes) {
				_currentCode = 0;
			}
			previousCode = lastCode = _currentCode;
			*p++ = (_currentCode & 255);
		} else {
			uint8_t *currentStackPtr = stackPtr;
			uint32_t slot = currentSlot;
			uint32_t code = _currentCode;
			if (_currentCode >= slot) {
				code = lastCode;
				currentStackPtr = &_stack[8190];
				_stack[8190] = previousCode & 255;
			}
			while (code >= kNewCodes) {
				--currentStackPtr;
				assert(currentStackPtr >= &_stack[0]);
				*currentStackPtr = _stack[code];
				code = _prefix[code];
			}
			--currentStackPtr;
			*currentStackPtr = (code & 255);
			if (slot < topSlot) {
				_stack[slot] = code & 255;
				previousCode = code;
				_prefix[slot] = lastCode & 0xFFFF;
				lastCode = _currentCode;
				++slot;
				currentSlot = slot;
			}
			if (slot >= topSlot && _codeSize < 12) {
				topSlot <<= 1;
				++_codeSize;
			}
			const int count = stackPtr - currentStackPtr;
			memcpy(p, currentStackPtr, count);
			p += count;
			currentStackPtr = stackPtr;
		}
		nextCode();
	}
	return p - dst;
}

int decodeLZW(const uint8_t *src, uint8_t *dst) {
	memset(&_lzw, 0, sizeof(_lzw));
	_lzw._buf = src;
	return _lzw.decode(dst);
}

