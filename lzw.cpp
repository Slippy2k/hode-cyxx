/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
 */

#include "intern.h"

static const uint32_t codeMask[] = { 0, 1, 3, 7, 0xF, 0x1F, 0x3F, 0x7F, 0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF, 0xFFFFFFFF };

enum {
	kCodeWidth = 9,
	kClearCode = 1 << (kCodeWidth - 1),
	kEndCode = kClearCode + 1,
	kNewCodes = kEndCode + 1
};

struct LzwDecoder {

	uint16_t _prefix[4096];
	uint32_t _currentCode;
	uint8_t _stack[8192];
	const uint8_t *_buf;
	uint32_t _topSlot;
	uint32_t _currentByte;
	uint32_t _codeSize;
	uint32_t _currentSlot;
	uint32_t _bitsLeft;

	void nextCode();
	int decode(uint8_t *dst);
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

int LzwDecoder::decode(uint8_t *dst) {
	uint8_t *dst_ = dst;
	_codeSize = kCodeWidth;
	_topSlot = 1 << kCodeWidth;
	uint8_t *stackPtr = &_stack[8191];
	_currentSlot = kNewCodes;
	uint32_t previousCode = 0;
	uint32_t lastCode = 0;
	_currentByte = _currentCode = *_buf++;
	_bitsLeft = 8;
	nextCode();
	while (_currentCode != kEndCode) {
		if (_currentCode == kClearCode) {
			_currentSlot = kNewCodes;
			_codeSize = kCodeWidth;
			_topSlot = 1 << kCodeWidth;
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
			previousCode = _currentCode;
			lastCode = _currentCode;
			*dst_++ = (_currentCode & 255);
		} else {
			uint8_t *currentStackPtr = stackPtr;
			uint32_t slot = _currentSlot;
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

int decodeLZW(const uint8_t *src, uint8_t *dst) {
	memset(&_lzw, 0, sizeof(_lzw));
	_lzw._buf = src;
	return _lzw.decode(dst);
}

