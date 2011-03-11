
typedef struct {
	uint8 read_buffer[188];
	FILE *file_ptr;
	int cutscene_number;
	uint8 *fread_buf_ptr;
	int fread_buf_sz;
	uint8 *fread_hdr_ptr;
	int fread_hdr_sz;
} preload_buffer_t;

preload_buffer_t _paf_preloadBuffers[4];
uint8 _paf_offsetsTable[200];
uint8 *_paf_unkTable[4];
const uint8 _paf_bufferFilledMarker = 1;
int _paf_preloadSize = 4;
int _paf_defaultAudioFrameBlockSize = 8820;

static void paf_resetPreloadBuffer() {
	_paf_readBufferEnd = ((0x80000 / (_paf_preloadSize << 11)) * _paf_preloadSize) << 11;
	_paf_readBuffer = _paf_readBufferPos = _paf_readBufferLen = 0;
}

static void paf_updateReadOffset() {
	_paf_readBufferLen += _paf_preloadSize << 11;
	_paf_readBuffer += _paf_readBufferLen;
	if (_paf_readBuffer >= _paf_readBufferEnd) {
		_paf_readBuffer = 0;
	}
}

static bool paf_needReadData() {
	return (_paf_readBufferEnd - _paf_readBufferLen) >= (_paf_preloadSize << 11);
}

static uint8 *paf_getNextDstBuffer(_ecx) {
	uint32 a = _paf_headerDataPtr2[_ecx];
	if (a == 0xFFFFFFFF) {
		return _paf_buffer0x800;
	} else if (a & 0x80000000) { // audio buffer
		return _paf_audioFrameDstBuffer + (a & 0x7FFFFFFF);
	} else { // video buffer
		return _paf_videoFrameDstBuffer + a;
	}
}

static bool paf_needBufferRefill() {
	return _paf_readBufferLen >= _paf_hdrReadBufferSize;
}

static void paf_refillReadBuffer(_ecx) {
	memcpy(_ecx, _paf_readBufferPos + _paf_preloadOffset, ((_paf_hdrReadBufferSize + (_paf_hdrReadBufferSize & 3)) / 4) * 4);
	_paf_readBufferLen -= _paf_hdrReadBufferSize;
	_paf_readBufferPos += _paf_hdrReadBufferSize;
	if (_paf_readBufferPos >= _paf_readBufferEnd) {
		_paf_readBufferPos = 0;
	}
}

static void paf_freeSomeTables() {
	free(_paf_videoFrameDstBuffer);
	free(_paf_decodeVideoPages);
	free(_paf_preloadOffset);
	if (_res_lvlUnk2) {
		sub_4038F0();
	}
}

static void paf_readHeader() {
	_paf_hdrSoundType = READ_LE_UINT32(_paf_preloadBuffers[_paf_preloadBufferNum].read_buffer + 0x88);
	_paf_animFramesCount = READ_LE_UINT32(_paf_preloadBuffers[_paf_preloadBufferNum].read_buffer + 0x84);
	_paf_hdrPreloadFrameBlocksCount = READ_LE_UINT32(_paf_preloadBuffers[_paf_preloadBufferNum].read_buffer + 0x9C);
	_paf_hdrMaxVideoFrameBuffers = READ_LE_UINT32(_paf_preloadBuffers[_paf_preloadBufferNum].read_buffer + 0xA8);
	_paf_hdrMaxAudioFrameBuffers = READ_LE_UINT32(_paf_preloadBuffers[_paf_preloadBufferNum].read_buffer + 0xAC);
	_paf_hdrReadBufferSize = READ_LE_UINT32(_paf_preloadBuffers[_paf_preloadBufferNum].read_buffer + 0x98);
	_paf_headerUnk6 = _paf_hdrReadBufferSize * _paf_hdrMaxVideoFrameBuffers; // videoDataSize ?
	_edi = READ_LE_UINT32(_paf_preloadBuffers[_paf_preloadBufferNum].read_buffer + 0xA0);
	_ebx = READ_LE_UINT32(_paf_preloadBuffers[_paf_preloadBufferNum].read_buffer + 0xB4);
	_paf_unk5 = _paf_hdrReadBufferSize * _paf_hdrMaxAudioFrameBuffers; // audioDataSize ?
	_paf_headerUnk7 = READ_LE_UINT32(_paf_preloadBuffers[_paf_preloadBufferNum].read_buffer + 0xB8);
	_paf_unk33 = (_paf_hdrReadBufferSize + (_paf_headerUnk6 & 0x7FF)) >> 11;
	_paf_unk17 = _paf_hdrMaxAudioFrameBuffers / _paf_preloadSize;
	_paf_framesCount = _edi;
	_paf_unk35 = _ebx;
	if ((_paf_hdrMaxAudioFrameBuffers % _paf_preloadSize) != 0) {
		++_paf_unk17;
	}
}

static void paf_preloadHelper(FILE *_ecx, uint8 *_edx, int count) {
	__read(_ecx, _edx, count);
	_eax = count & 0x7FF;
	if (_eax != 0) {
		_esi = 0x800 - _eax;
		__read(_ecx, _paf_buffer0x800, _esi);
		return _esi + count;
	} else {
		return _eax + count;
	}
}

static void paf_open() {
	_paf_buffer0x800 = (uint8 *)malloc(0x800);
	_paf_preloadSize = 4;
	paf_resetPreloadBuffer();
	_paf_readBufferEnd = 0;
	_paf_headerUnk6 = 0;
	_paf_unk5 = 0;
	_paf_preloadBufferNum = -1;
	paf_initTables();
	for (i = 0; i < 4; ++i) {
		_paf_preloadBuffers[i].file_ptr = -1;
	}
	paf_freeSomeTables();
	_paf_fileHandle = fopen("HOD_DEMO.PAF", "rb");
	fread(_paf_offsetsTable, 200, 1, _paf_fileHandle);
}

static void paf_decode6() {
	const uint8 *_esi = *_paf_frameData;
	_paf_decode6Code = *_esi++;
	if (_paf_decode6Code & 0x20) {
		memset(_paf_decodeVideoPages, 0, 0x10000 * 4);
		_paf_currentDecodeVideoPage = 0;
		memset(_paf_paletteBuffer, 0, 0xC0 * 4); /* palette */
	}
	_paf_decode6Code2 = _paf_decode6Code & 0x40; // palette flag
	if (_paf_decode6Code & 0x40) {
		_al = *_esi++; /* color_index */
		_edi = _paf_paletteBuffer + _al * 3;
		_cl = *_esi++; /* color_count */
		_ecx = _cl + 1;
		memcpy(_edi, _esi, _ecx * 3);
		_edi += _ecx * 3;
		_esi += _ecx * 3;
	}
	switch (_paf_decode6Code & 0xF) {
	case 0:
		/* XXX paf_decode6_0 */
		uint8 _cl = *src++;
//			const uint8 *_edx = src;
		if (_cl != 0) {
			if (code & 0x10) {
//					_edx -= _paf_frameData;
				int _edx = src - _paf_frameData;
				int _eax = 4;
				_edx &= 3;
				if (_edx != 0) {
					_eax -= _edx;
					src += _eax;
				}
			}
//loc_42F0EE:
			do {
				int _eax = 0;
				uint8 byte_4474F9 = _cl;
				int _edx = 0;
				int _ecx = 0;
				int _ebx = 0;
				_eax = *src++;
				_edx = *src; src += 3;
				uint8 *_ebp_p = _paf_unk47[_eax]; // * 4];
				_ebx = src[-2];
//					_edi = _paf_unk50[_edx]; // * 4];
//					_dl &= 0x7F; // &= ~0x80;
				_edx &= 0x7F;
//					_edi += _ebp;
				uint8 *_edi = _ebp_p + _paf_unk50[_edx];
				_edx *= 2;
				_ecx = 0x100; // ++_ch;
				_ebx += _edx;
				int _ebp = 4;
//loc_42F12C:
				do {
					++_edx;
					*(uint32 *)_edi = *(uint32 *)src;
					*(uint32 *)(_edi + _ecx) = *(uint32 *)(src + _ebp);
					*(uint32 *)(_edi + _ecx * 2) = *(uint32 *)(src + _ebp * 2);
					*(uint32 *)(_edi + _ecx * 2 + 256) = *(uint32 *)(src + _ebp * 4 - 4);
					_edi += _ebp;
					uint8 _dl = _edx & 0xFF;
					if ((_dl & 0x3F) == 0) {
						_edi = _edi + _ecx * 2 + 256;
					}
				} while (_edx < _ebx);
				_cl = byte_4474F9 - 1;
			} while (_cl != 0);
		}
//loc_42F166:
		uint8 *_edi = _paf_decodeVideoPages + (_paf_currentDecodeVideoPage << 16);
		int _ebx = 0;
//			int _edx = 0;
//loc_42F17C:
		do {
//				int _eax = 0;
			int _dl = src[0];
			int _al = src[1];
			uint8 *_ebp = _paf_unk47[_dl]; // _edx * 4
			uint32 _ecx_i = _paf_unk50[_al]; // _eax * 4
			src += 2;
			uint8 *_ecx = _ebp + _ecx_i;
			*(uint32 *)_edi = *(uint32 *)_ecx;
			*(uint32 *)(_edi + 256) = *(uint32 *)(_ecx + 256);
			*(uint32 *)(_edi + 512) = *(uint32 *)(_ecx + 512);
			*(uint32 *)(_edi + 768) = *(uint32 *)(_ecx + 768);
			++_ebx;
			uint8 _bl = _ebx & 0xFF;
			if ((_bl & 0x3F) == 0) {
				_edi += 768;
			}
			_edi += 4;
		} while (_ebx <  256 * 192 / 16);
		_eax = READ_LE_UINT16(src); src += 4;
		dword_4474F0 = src;
		src += _eax;
		_edi = _paf_decodeVideoPages + (_paf_currentDecodeVideoPage << 16);
		_al = 0x30;
		_edi += _ebx;
loc_42F1FC:
		_bl = 0x20;
		byte_4474F8 = _al;
loc_42F203:
		_eax = 0;
		byte_4474F9 = _bl;
		_ecx = dword_4474F0;
		_ebx = _paf_unk48;
		_al = *ecx++;
		_eax *= 16;
		dword_4474F0 = _ecx;
		if (_eax == 0) { //goto loc_42F274;
			_edi += 4;
			goto paf_decode6Sub0_1;
		}
		_ebx += _eax;
		_edx = 512;
		_al = *_ebx++;
		_paf_decode6Sub0TableIndex = _ebx;

		// _paf_decode6Sub0Table jmp_table
		if (_al == 0) { // paf_decode6Sub0_0
			_al = *_paf_decode6Sub0TableIndex++;
			_edx = 512;
			jmp _paf_decode6Sub0Table[_al];
		}
		if (_al == 1) { // paf_decode6Sub0_1
			_edi += 4;
			_bl = byte_4474F9 - 1;
			if (_bl != 0) goto loc_42F203;
			_edi += 768;
			_al = byte_4474F8 - 1;
			if (_al != 0) goto loc_42F1FC;
		}
		if (_al == 2) { // paf_decode6Sub0_2
			_edx = 0;
			goto paf_decode6Sub0_3;
		}
		if (_al == 3) { // paf_decode6Sub0_3
			_eax = 0;
			_al = *src++;
			_ebp = _paf_remapPalette[_eax * 4];
			goto paf_decode6Sub0_4;
		}
		if (_al == 4) { // paf_decode6Sub0_4
			_eax = 0;
			_edx += _edi;
			_al = *src++;
			_ecx = _paf_unk16;
			_eax *= 8;
			_ecx += _eax;
			_ebx = *(uint32 *)_edx;
			_eax = *(uint32 *)_ecx;
			_ecx += 4;
			_ebx &= _eax;
			_eax = ~_eax;
			_eax &= _ebp;
			_eax |= _ebp;
			*_edx = *_ecx; // uint32
			_edx += 256;
			_eax = 0;
			_ebx = *_edx;
			_ebx &= _ecx;
			_ecx = ~_ecx;
			_ecx &= _ebp;
			_ecx |= _ebx;
			_al = *_paf_decode6Sub0TableIndex++;
			*_edx = _ecx; // uint32
			_edx = 512;
			jmp _paf_decode6Sub0Table[_al];
		}
		if (_al == 5) { // paf_decode6Sub0_5
			_edx = 0;
			goto paf_decode6Sub0_6;
		}
		if (_al == 6) { // paf_decode6Sub0_6
			_eax = 0;
			_al = *src++;
			_ebx = _paf_unk47[_eax * 4];
			_al = *src++;
			_ebx += _paf_unk50[_eax * 4];
			goto paf_decode6Sub0_7;
		}
		if (_al == 7) { // paf_decode6Sub0_7
			_eax = 0;
			_ecx = _paf_unk16;
			_al = *src;
			_ebp = *(_edx + _ebx);
			_ecx = *(_ecx + _eax * 8);
			_eax = *(_edx + _edi);
			_eax &= _ecx;
			_ecx = ~_ecx;
			_ebp &= _ecx;
			_ebp |= _eax;
			_eax = 0;
			*(_edx + _edi) = _ebp;
			_al = *src++;
			_edx += 256;
			_ecx = _paf_unk16;
			_ecx = *(_paf_unk16 + _eax * 8 + 4); // uint32
			_ebp = *(_edx + _ebx); // uint32
			_eax = *(_edx + _edi); // uint32
			_eax &= _ecx;
			_ecx = ~_ecx;
			_ebp &= _ecx;
			_ebp |= _eax;
			_eax = 0;
			_al = *_paf_decode6Sub0TableIndex++;
			*(_edx + _edi) = _ebp;
			_edx = 512;
			goto _paf_decode6Sub0Table[_al];
		}
		break;
	case 1:
		/* clear screen */
		memcpy(_paf_decodeVideoPages + (_paf_currentDecodeVideoPage << 0x10), _esi + 2, 12288 * 4);
		break;
	case 2:
		_al = *_esi;
		if (_paf_currentDecodeVideoPage != 0) {
			_edi = _paf_decodeVideoPages + (_paf_currentDecodeVideoPage << 0x10);
			_esi = _paf_decodeVideoPages + (_al << 0x10);
			memcpy(_edi, _esi, 0x3000 * 4);
		}
		break;
	case 4:
		_ecx = 0;
		_edi = _paf_decodeVideoPages + (_paf_currentDecodeVideoPage << 0x10);
		_esi += 2;
		_ebx = 0xC000;
		_eax = 0;
		_al = *_esi++;
		if (_al & 0x80) {
loc_42F058:
			_al = -_al + 1;
			_dl = *_esi++;
			_ecx = _paf_remapPalette;
			_edx = _dl << 2;
			_ebx -= _eax;
			_edx += _ecx;
			_ecx = _eax;
			_eax = READ_LE_UINT32(_edx);
			memset(_edi, _eax, _ecx);
			if (_ebx == 0) return;
			_al = *_esi++;
			if (_al & 0x80) goto loc_42F058;
		}
loc_42F090:
		++_al;
		_ecx = _eax;
		_ebx -= _eax,
		memcpy(_edi, _esi, _ecx);
		if (_ebx == 0) return;
		_al = *_esi++;
		if (_al & 0x80) goto loc_42F058;
		goto loc_42F090;
		break;
	default:
		break;
	}
}

static void paf_preload(_ecx) {
	_paf_preloadBufferNum = 0;
	for (int i = 0; i < unk_462A90; ++i) {
		if (_paf_preloadBuffers[i].file_ptr != -1 && _paf_preloadBuffers[i].cutscene_number == _ecx) {
			_paf_preloadBufferNum = i;
			return;
		}
	}
	_paf_preloadBufferNum = i;
	for (i = 0; i < unk_462A8C; ++i) {
		if (_paf_preloadBuffers[i].file_ptr == -1) {
			goto ok;
		}
	}
	i = 0;
ok:
	_paf_preloadBufferNum = i;
	_paf_preloadBuffers[i].file_ptr = _paf_fileHandle;
	_paf_preloadBuffers[i].cutscene_number = _ecx;
	_eax = READ_LE_UINT32(_paf_offsetsTable + _ecx * 4);
	if (_eax == 0) {
		error("Can't open cinematic File n°:%d", _ecx);
		return;
	}
	__lseek(_paf_fileHandle, _eax, 0);
	__read(_paf_fileHandle, _paf_preloadBuffers[_paf_preloadBufferNum].read_buffer, 0xBC);
	__read(_paf_fileHandle, _paf_buffer0x800, 0x744);
	_al = _paf_preloadBuffers[_paf_preloadBufferNum].read_buffer[0x80];
	if (_al > 1) {
		error();
	}
	_dl = _paf_preloadBuffers[_paf_preloadBufferNum].read_buffer[0x81];
	if (_dl > 0 && _al == 1) {
		error();
	}
	_edx = _paf_preloadBuffers[_paf_preloadBufferNum].cutscene_number;
	// hof_demo6.paf
	*(uint32 *)_paf_preloadBuffers.read_buffer[0xA4] += *(uint32 *)(_paf_offsetsTable + _edx * 4); // 0x2000 + offs (start offset ?)
	_paf_hdrSoundType = *(uint32 *)_paf_preloadBuffers.read_buffer[0x88]; // 125
	// 100 -> snd_resample22050 / else -> snd_resample17640
	_paf_hdrPreloadFrameBlocksCount = *(uint32 *)_paf_preloadBuffers.read_buffer[0x9C]; // 36 (frame count ?)
	_paf_hdrMaxVideoFrameBuffers = *(uint32 *)_paf_preloadBuffers.read_buffer[0xA8]; // 128
	_eax = _paf_hdrReadBufferSize = *(uint32 *)_paf_preloadBuffers.read_buffer[0x98]; // 2048
	_edx = _paf_hdrMaxVideoFrameBuffers * _paf_hdrReadBufferSize;
	_edi = _paf_hdrMaxAudioFrameBuffers = *(uint32 *)_paf_preloadBuffers.read_buffer[0xAC]; // 0
	_paf_headerUnk6 = _edx;
	_paf_headerUnk7 = *(uint32 *)_paf_preloadBuffers.read_buffer[0xB8]; // 0
	_ebx = *(uint32 *)_paf_preloadBuffers.read_buffer[0x84]; // 4
	_esi = *(uint32 *)_paf_preloadBuffers.read_buffer[0xA0]; // 36
	_ebp = *(uint32 *)_paf_preloadBuffers.read_buffer[0xB4]; // 0
	_ecx = *(uint32 *)_paf_preloadBuffers.read_buffer[0xB8]; // 0
	_edi *= _eax;
	_paf_headerUnk6 = _edx;
	_ecx = _paf_preloadSize;
	_paf_unk33 = (_eax + (_edx & 0x7FF)) >> 0xB;
	_paf_animFramesCount = _ebx;
	_paf_framesCount = _esi;
	_paf_unk35 = _ebp;
	_paf_unk5 = _edi;
	_paf_unk17 = _esi / _ecx;
	if (_esi % _ecx != 0) {
		++_paf_unk17;
	}
	_eax = _paf_headerData = paf_allocMem(_esi + _ebx * 2, "Header Data");
	_paf_preloadBuffers[_paf_preloadBufferNum].fread_hdr_sz = _esi;
	_paf_preloadBuffers[_paf_preloadBufferNum].fread_hdr_ptr = _paf_headerData;
	_ecx = _paf_animFramesCount * 4;
	_paf_headerDataPtr2 = _paf_headerData + _paf_animFramesCount * 8;
	_paf_headerDataPtr1 = _paf_headerData + _paf_animFramesCount * 4; //_ecx + _eax;
	/* read frame 1 */
	paf_preloadHelper(_paf_fileHandle, _eax, _ecx); /* frame blocks count */
	/* read frame 2 */
	paf_preloadHelper(_paf_fileHandle, _paf_headerDataPtr1, _paf_animFramesCount * 4); /* data frames offsets (video ?), relative to read buffer size ? */
	/* read frame 3 */
	paf_preloadHelper(_paf_fileHandle, _paf_headerDataPtr2, _paf_framesCount * 4); /* data frame blocks type / size ? */
	_eax = 0;
	if (_paf_animFramesCount > 0) {
		do { ++_eax; } while (_eax < _paf_animFramesCount);
	}
	_ecx = 0;
	if (_paf_framesCount > 0) {
		do {
			_eax = paf_headerDataPtr2[_ecx * 4];
			if (_eax != 0xFFFFFFFF) {
				if (_snd_unkVar31 != 0) { // _snd_isMuted ?
					if (_eax & 0x80000000) {
						_eax |= 0xFFFFFFFF;
					}
					paf_headerDataPtr2[_ecx] = _eax;
				}
			}
		} while (++_ecx < _paf_framesCount);
	}
	if (_cfg_slowCD) {
		_eax = _paf_hdrReadBufferSize * _paf_hdrPreloadFrameBlocksCount;
		_esi = &_paf_preloadBuffers[_paf_preloadBufferNum].fread_buf_sz;
	} else {
		_eax = _paf_hdrReadBufferSize * _paf_hdrPreloadFrameBlocksCount * 2;
		if (_eax > 0x20000) {
			_eax = 0x20000;
		}
		_esi = &_paf_preloadBuffers[_paf_preloadBufferNum].fread_buf_sz;
	}
	*_esi = _eax;
	_eax = ((_eax / (_paf_preloadSize << 11)) * _paf_preloadSize) << 11;
	*_esi = _eax;
	_paf_preloadBuffers[_paf_preloadBufferNum].fread_buf_ptr = paf_allocMem(_eax, "PreloadBuffer");
	__lseek(_paf_fileHandle, *(uint32 *)_paf_preloadBuffers[_paf_preloadBufferNum].read_buffer[0xA4], SEEK_SET);
	__read(_paf_fileHandle, _paf_preloadBuffers[_paf_preloadBufferNum].fread_buf_ptr, _paf_preloadBuffers[_paf_preloadBufferNum].fread_buf_sz);
	return _paf_preloadBufferNum;
}

static void paf_prepareNextFrame() { // paf_readNextBuffer
	if (_paf_numFrames != 0 && _paf_readBufferLen >= _paf_hdrReadBufferSize) {
		uint32 _ecx = _paf_headerDataPtr2[_paf_unk13++];
		if (_ecx == 0xFFFFFFFF) {
			_edi = _paf_buffer0x800; // sizeof 0x800
		} else {
			if (_ecx & 0x80000000) { // snd frame ?
				_edi = _paf_audioFrameDstBuffer + (_ecx & 0x7FFFFFFF); // sizeof _paf_audioFrameDstBuffer ?
			} else { // video frame ?
				_edi = _paf_videoFrameDstBuffer + _ecx; // sizeof _paf_videoFrameDstBuffer == _paf_headerUnk6
			}
		}
		_ecx = _paf_hdrReadBufferSize + (_paf_hdrReadBufferSize & 3);
		_esi = _paf_readBufferPos + _paf_preloadOffset; // _paf_readBufferPos=_paf_preloadOffset _paf_preloadOffset=_paf_preloadBuffer
		memcpy(_edi, _esi, (_ecx >> 2) << 2); // (_ecx + 3) & ~3
		_paf_readBufferLen -= _paf_hdrReadBufferSize;
		_paf_readBufferPos += _paf_hdrReadBufferSize;
		if (_paf_readBufferPos >= _paf_readBufferEnd) {
			_paf_readBufferPos = 0;
		}
		--_paf_numFrames;
	}
}

void paf_play() {
	// decode preload frames
	_paf_numFrames = _paf_hdrPreloadFrameBlocksCount; // preload
	while (_paf_numFrames != 0) {
		do {
			if (paf_needReadData()) {
				paf_readData();
			}
			paf_prepareNextFrame();
		} while (_paf_numFrames != 0);
		if (_paf_fillBufferFlagTable[_paf_fillBufferCurrentIndex] != 0 || _paf_fillBufferCurrentIndex >= 4)
			goto paf_end_header;
		_paf_numFrames = _paf_headerData[_paf_fillBufferCurrentIndex];
		_paf_frameData = _paf_videoFrameDstBuffer + _paf_headerDataPtr1[_paf_fillBufferCurrentIndex]; // sizeof _paf_videoFrameDstBuffer == _paf_headerUnk6
		paf_decode6();
		_paf_buffersTable[_paf_fillBufferCurrentIndex] = (_paf_currentDecodeVideoPage << 16) | _paf_decodeVideoPages;
		_pal_paletteRefreshFlag[_paf_fillBufferCurrentIndex] = _paf_decode6Code2;
		if (_pal_paletteRefreshFlag[_paf_fillBufferCurrentIndex] != 0) {
			memcpy(&_paf_palettePtr[_paf_fillBufferCurrentIndex * 3], _paf_paletteBuffer, 768);
		}
		++_paf_currentDecodeVideoPage;
		if (_paf_currentDecodeVideoPage >= 4) {
			_paf_currentDecodeVideoPage = 0;
		}
		++_paf_currentFrame;
		_paf_fillBufferFlagTable[_paf_fillBufferCurrentIndex] = _paf_bufferFilledMarker;
		++_paf_fillBufferCurrentIndex;
	}
paf_end_header:
	if (_snd_unkVar31 == 0) {
		_edx = &_snd_unkVar44;
		_ecx = &_paf_sndUnk1;
		paf_soundUnk1(0, _null_var5);
		if (_menu_currentState != 1) {
			EnterCriticalSection(_snd_mutex);
			if (_paf_hdrSoundType == 100) {
				_snd_playbackFreq = 2205;
				_snd_resampleFuncPtr = snd_resample22050;
			} else {
				_snd_playbackFreq = 1764;
				_snd_resampleFuncPtr = snd_resample17640;
			}
			LeaveCriticalSection(_snd_mutex);
		}
	}
	_time_counter1 += _time_counter2 + GetTickCount();
	if (_menu_currentState == 0) {
		InitializeCriticalSection(_util_threadMutex);
	}
	ResumeThread(_paf_readDataThreadHandle);
	ResumeThread(_paf_ioThreadHandle);
	paf_mainLoop(); // XXX
	SetEvent(_paf_threadEvent);
	WaitForSingleObject(_paf_ioThreadHandle, -1);
	WaitForSingleObject(_paf_readDataThreadHandle, -1);
	CloseHandle(_paf_readDataThreadHandle);
	CloseHandle(_paf_ioThreadHandle);
	_paf_threadEvent = 0;
	if (_menu_currentState == 0) {
		DeleteCriticalSection(_util_threadMutex);
	}
}


void paf_ioThread() {
loop:
	while (_paf_numFrames != 0) { // _paf_currentFrameBlocksCount
		if (!paf_needBufferRefill())
			break;
		_ecx = getFrameDataPtr(_paf_unk13++); // _paf_currentBlock
		paf_refillReadBuffer();
		--_paf_numFrames;
	}
//	_game_keysPressedMask = 0;
//	_game_keysToCheckMask = 0;
//	inp_updateKeyboardGetDirection();
//	if (_game_keysToCheckMask == 0) {
//		byte_462510 = 1;
//	} else {
//		if (byte_462510 != 0) {
//			if (_paf_cutscenesAlreadyPlayed != 0 || (_inp_keysBuffer[0x1F] && _inp_keysBuffer[0x2E])) { // DIK_S, DIK_C
//				_inp_S_C_pressed = 1;
//			}
//		}
//	}
	if (_paf_numFrames == 0 && _paf_currentFrame - _paf_unk2 < 4 && _paf_currentFrame < _paf_animFramesCount && _paf_unkTable[_paf_currentFrame & 3] == 0) {
		_paf_numFrames = _paf_headerData[_paf_currentFrame];
		_paf_unkTable[_paf_currentFrame & 3] = _paf_videoFrameDstBuffer + _paf_headerDataPtr1[_paf_currentFrame];
		++_paf_currentFrame;
		if (_snd_unkVar31 == 0 && _menu_currentState != 1) {
			if (_paf_currentFrame - _snd_unkVar12 > _eax && _snd_unkVar12 < 0) {
				EnterCriticalSection(_snd_mutex);
				dword_447658 = _paf_currentFrame - _snd_unkVar12;
				LeaveCriticalSection(_snd_mutex);
				SetEvent(_snd_threadEvent1);
				_eax = _paf_animFramesCount;
				if (dword_447658 + _snd_unkVar12 >= _eax && _paf_unk1 == 0) {
					if (_paf_audioFrameDstBuffer) {
						memset(_paf_audioFrameDstBuffer, 0, _paf_unk5);
					}
					if (dword_447658 != 0) {
						SetEvent(_snd_threadEvent1);
						while (dword_447658 != 0) {
							Sleep(1);
						}
					}
					EnterCriticalSection(_snd_mutex);
					if (_menu_currentState != 0) {
						dword_447658 = 4;
					} else {
						dword_447658 = _paf_defaultAudioFrameBlockSize / _snd_playbackFreq;
					}
					LeaveCriticalSection(_snd_mutex);
					SetEvent(_snd_threadEvent1);
					_paf_unk1 = 1;
				}
			}
		}
	}
	LeaveCriticalSection(_util_threadMutex);
	EnterCriticalSection(_util_threadMutex);
	while (_paf_numFrames != 0) {
		if (!paf_needBufferRefill())
			break;
		_ecx = paf_getNextDstBuffer(_paf_unk13++);
		paf_refillReadBuffer(_ecx);
		--_paf_numFrames;
	}
	LeaveCriticalSection(_util_threadMutex);
//	if (_inp_joystickEnabled) {
//		_game_keysPressedMask = inp_updateJoystick3();
//		_game_keysToCheckMask = inp_updateJoystickGetButtons();
//	} else {
//		_game_keysPressedMask = 0;
//		_game_keysToCheckMask = 0;
//	}
//	inp_updateKeyboardGetDirection();
//	if (_game_keysToCheckMask == 0) {
//		byte_462510 = 1;
//	} else {
//		if (byte_462510 != 0 && (_paf_cutscenesAlreadyPlayed != 0 || ((_keysBuffer[0x1F] & 0x80) != 0 && (_keysBuffer[0x2E] & 0x80) != 0)) {
//			_inp_S_C_pressed = 1;
//		}
//	}
	if (_paf_unkTable[_paf_fillBufferCurrentIndex & 3] != 0 && _paf_fillBufferFlagTable[_paf_fillBufferCurrentIndex & 3] == 0) {
		_paf_frameData = _paf_unkTable[_paf_fillBufferCurrentIndex & 3];
		if ((_paf_frameData[0] & 0x20) == 0 || READ_LE_UINT32(_paf_fillBufferFlagTable) == 0) { // _paf_fillBufferFlagTable[0] == 0 && _paf_fillBufferFlagTable[1] == 0 && _paf_fillBufferFlagTable[2] == 0 && _paf_fillBufferFlagTable[3] == 0
			paf_decode6();
			_paf_buffersTable[_paf_fillBufferCurrentIndex & 3] = (_paf_currentDecodeVideoPage << 16) + _paf_decodeVideoPages;
			_pal_paletteRefreshFlag[_paf_fillBufferCurrentIndex & 3] = _paf_decode6Code2;
			if (_paf_decode6Code2 != 0) {
				memcpy(_paf_palettePtr + _paf_fillBufferCurrentIndex * 768, _paf_paletteBuffer, 768);
			}
			++_paf_currentDecodeVideoPage;
			if (_paf_currentDecodeVideoPage >= 4) {
				_paf_currentDecodeVideoPage = 0;
			}
			_paf_fillBufferFlagTable[_paf_fillBufferCurrentIndex & 3] = _paf_bufferFilledMarker;
			_paf_unkTable[_paf_fillBufferCurrentIndex & 3] = 0;
			++_paf_fillBufferCurrentIndex;
		}
	}
	while (_paf_numFrames != 0) {
		if (!paf_needBufferRefill())
			break;
		_ecx = paf_getNextDstBuffer(_paf_unk13++);
		paf_refillReadBuffer(_ecx);
		--_paf_numFrames;
	}
//	_game_keysPressedMask = 0;
//	_game_keysToCheckMask = 0;
//	inp_updateKeyboardGetDirection();
//	if (_game_keysToCheckMask == 0) {
//		byte_462510 = 1;
//	} else {
//		if (byte_462510 != 0) {
//			if (_paf_cutscenesAlreadyPlayed != 0 || (_inp_keysBuffer[0x1F] && _inp_keysBuffer[0x2E])) { // DIK_S, DIK_C
//				_inp_S_C_pressed = 1;
//			}
//		}
//	}
//loc_42018E:
	Sleep(10);
	goto loop;
}


//
// Sound Decoding Routines
//

uint32 _snd_unkVar40[0x100]; // size -> 0x200, left/right channels
uint32 _snd_unkVar42[0x1F00];

static void snd_resample22050() { // snd_decode2205
	_snd_unkVar10 += _snd_playbackFreq; // _snd_dataBlockSize
	_ecx = ((_snd_unkVar37 * 128) * _snd_unkVar38 + 64) / 128; // _snd_volume
	_esi = _snd_unkVar39;
	for (int i = 0; i < 256; ++i) {
		_eax = READ_LE_INT16(_esi); _esi += 2;
		_eax *= _ecx;
		_snd_unkVar40[i] = _snd_unkVar40[256 + i] = _eax;
	}

	_edx = &_snd_unkVar41[_snd_unkVar11 * 2]; // uint32
	_edi = _snd_playbackFreq;
	_snd_unkVar11 += _snd_playbackFreq;
	while (_edi--) {
		uint8 code = *_esi++;
		_eax = _snd_unkVar40[code];
		code = *_esi++;
		_ecx = _snd_unkVar40[256 + code];
		if (_eax < -0x20000000) {
			_eax = -0x20000000;
		} else if (_eax > 0x1FFFFFFF) {
			_eax = 0x1FFFFFFF;
		}
		if (_ecx < -0x20000000) {
			_ecx = -0x20000000;
		} else if (_ecx > 0x1FFFFFFF) {
			_ecx = 0x1FFFFFFF;
		}
		*_edx++ = _eax;
		*_edx++ = _ecx;
		--_edi;
	}
	--_snd_unkVar43;
	_snd_unkVar39 = _esi;
	if (_snd_unkVar43 != 0) {
		_snd_unkVar39 = READ_LE_UINT32(_snd_unkVar44);
		_snd_unkVar43 = READ_LE_UINT16(_snd_unkVar44 + 16);
	}
}

static void snd_resample17640() { // _snd_decode1764
	while (byte_4470FD != 0);
	_snd_unkVar47 = 1;
	_ebp = _snd_playbackFreq;
	_snd_unkVar10 += _snd_playbackFreq; // _eax += _ebp
	if (_snd_unkVar45 == 0) {
		_snd_unkVar11 += _snd_playbackFreq;
		memset(&_snd_unkVar41[_snd_unkVar11 * 2], 0, _snd_playbackFreq * 8);
		_snd_unkVar47 = 0;
		return;
	}
	var8 = 0;
	var4 = _snd_unkVar46; // _ebx
//	_edx = _snd_unkVar40;
loc_42A890:
	do {
		*_ebx = *_ebx + 512;
		switch (_ebx[12]) {
		case 0:
			memset(_edx, 0, 512 * 4);
			_edx += 2048 / 4;
			break;
		case 1:
			memset(_edx, 0, 256 * 4);
			_edx += 1024 / 4;
			_edi = READ_LE_UINT16(_ebx + 8);
			for (int i = 0; i < 256; ++i) {
				_eax = READ_LE_INT16(_esi); _esi += 2;
				*_edx++ = _eax * _edi;
			}
			break;
		case 2:
			_edi = READ_LE_UINT16(_ebx + 4);
			for (int i = 0; i < 256; ++i) {
				_eax = READ_LE_INT16(_esi); _esi += 2;
				*_edx++ = _eax * _edi;
			}
			memset(_edx, 0, 256 * 4);
			_edx += 1024 / 4;
		case 3:
			_edi = READ_LE_UINT16(_ebx + 4);
			for (int i = 0; i < 256; ++i) {
				_eax = READ_LE_INT16(_esi); _esi += 2;
				_eax *= _edi;
				_edx[256] = _eax;
				*_edx++ = _eax;
			}
			_edx += 1024 / 4;
			break;
		default:
			_edi = READ_LE_UINT16(_ebx + 8);
			_ebp = READ_LE_UINT16(_ebx + 4);
			for (int i = 0; i < 256; ++i) {
				_eax = READ_LE_INT16(_esi); _esi += 2;
				_edx[256] = _edi * _eax;
				*_edx++ = _ebp * _eax;
			}
			_ebx = var4 + 1024;
			break;
		}
loc_42A980:
		var4 = _ebx + 16;
		++var8;
	} while (var8 < _snd_unkVar45);
	var4 = &_snd_unkVar41[_snd_unkVar11 * 2];
	var8 = _ebp;
	_snd_unkVar11 += _ebp;
	do {
loc_42A9BF:
		_esi = 0;
		_ecx = 0;
		_edi = &_snd_unkVar40;
		_ebx = 0;
		_eax = &_snd_unkVar46;
		do {
			uint8 code = _eax[13];
			_ebp = READ_LE_UINT32(_eax);
			if (code != 0) {
				_edx = 0;
				_dl = _ebp[0];
				_ecx += READ_LE_UINT32(_edi + _edx * 4);
				++_ebp;
				_edx = 0;
				*(uint32 *)_eax = _ebp,
				_dl = _ebp[0];
			} else {
				_edx = 0;
				_dl = _ebp[0];
				_ecx += READ_LE_UINT32(_edi + _edx * 4);
			}
			_esi += READ_LE_UINT32(_edi + _edx * 4 + 1024);
			_edx = _snd_unkVar45;
			++_ebp;
			_edi += 2048;
			*(uint32 *)_eax = _ebp;
			++_ebx;
			_eax += 16;
		} while (_ebx < _edx);
//		_eax = _ecx >> 3;
//		_edi = _ecx >> 4;
//		_eax += _ecx,
//		_ecx = _eax + _edi;
		_ecx = _ecx + (_ecx >> 3) + (_ecx >> 4);
		if (_ecx < -0x20000000) {
			_ecx = -0x20000000;
		} else if (_ecx > 0x1FFFFFFF) {
			_ecx = 0x1FFFFFFF;
		}
//		_eax = _esi;
//		_edi = _eax;
//		_eax = _esi;
//		_eax >>= 3;
//		_edi >>= 4;
//		_eax += _esi;
//		_edx = _eax + _edi;
		_edx = _esi + (_esi >> 3) + (_esi >> 4);
		if (_edx < -0x20000000) {
			_edx = -0x20000000;
		} else if (_edx > 0x1FFFFFFF) {
			_edx = 0x1FFFFFFF;
		}
		*(uint32 *)var4 = _ecx; var4 += 4;
		*(uint32 *)var4 = _edx; var4 += 4;
		--var8;
	} while (var8 != 0);
	_snd_unkVar47 = 0;
	_snd_unkVar45 = 0;
}


//
// --- Video Drawing Routines
//

// screen_dim 256x192 -> 512x384
void gfx_blit(const uint8 *_esi, uint8 *_edi, int pitch) { // screen_scale2x
	_gfx_blitPitch = (pitch - 256) * 2;
	_ebp = _edi + pitch;
	_ecx = 0;
loop:
	(_edi + 0) = *(_edi + 4) = _esi[1] << 24 | _esi[1] << 16 | _esi[0] << 8 | _esi[0];
	(_ebp + 0) = *(_ebp + 4) = _esi[3] << 24 | _esi[3] << 16 | _esi[2] << 8 | _esi[2];
	(_edi + 8) = *(_edi + 0xC) = _esi[5] << 24 | _esi[5] << 16 | _esi[4] << 8 | _esi[4];
	(_ebp + 8) = *(_ebp + 0xC) = _esi[7] << 24 | _esi[7] << 16 | _esi[6] << 8 | _esi[6];
	(_edi + 0x10) = *(_edi + 0x14) = _esi[9] << 24 | _esi[9] << 16 | _esi[8] << 8 | _esi[8];
	(_ebp + 0x10) = *(_ebp + 0x14) = _esi[11] << 24 | _esi[11] << 16 | _esi[10] << 8 | _esi[10];
	(_edi + 0x18) = *(_edi + 0x1C) = _esi[13] << 24 | _esi[13] << 16 | _esi[12] << 8 | _esi[12];
	(_ebp + 0x18) = *(_ebp + 0x1C) = _esi[15] << 24 | _esi[15] << 16 | _esi[14] << 8 | _esi[14];
	_ecx += 0x10;
	if (_cl != 0) goto loop;
	_edi += _gfx_blitPitch;
	_ebp = _edx + _edi;
	if (_ch != 192) goto loop;
}