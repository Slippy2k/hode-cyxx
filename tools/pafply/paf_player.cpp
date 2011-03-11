
#include "paf_player.h"
#include "systemstub.h"
#include "writer.h"

#define HEADER_DATA_SIZE 100000

static uint32 paf_headerData[HEADER_DATA_SIZE];
static uint32 paf_headerDataPtr1[HEADER_DATA_SIZE];
static uint32 paf_headerDataPtr2[HEADER_DATA_SIZE];
static int _paf_hdrSoundType;
static int _paf_animFramesCount;
static int _paf_hdrPreloadFrameBlocksCount;
static int _paf_unkA0;
//static uint8 _paf_unk48[4096]; // video_update_opcodes_sequence
//static uint8 _paf_unk16[256 * 8]; // _bit_to_bytes_mask_table
//static uint32 _paf_remapPalette[256];
static uint8 _paf_videoBuffer[1024 * 1024]; // XXX dim from _paf_hdrVideo
static uint8 _paf_audioBuffer[1024 * 1024]; // XXX dim from _paf_hdrAudio
// frame sound size 2205 ou 1764, uncompressed size 8820 (2205*4, 1764*5)
//static int decodeCount = 0;
static int _paf_startOffset = 0;
SoundWriter *soundWriter = 0;
static int _paf_hdrReadBufferSize;
static int _paf_hdrVideoFrameBlocksCount, _paf_hdrAudioFrameBlocksCount;

uint8 *PAF_Player::getDecodePagePtr(uint8 a, uint8 b) { // _paf_unk47, _paf_unk50
/*	uint8 *ptr = _paf_decodeVideoPages;
	int page_num = (a & 0xC0) >> 6;
	int page_start_line = (a & 0x3F) * 4;
	int block_2x2_y = (b & 0x80) >> 7;
	int block_2x2_x = b & 0x7F;
	return ptr + page_num * 256 * 256 + (page_start_line + block_2x2_y * 2) * 256 + block_2x2_x * 2;*/
	int x = b & 0x7F;
	int y = ((a & 0x3F) << 1) | ((b >> 7) & 1);
	int page = (a & 0xC0) >> 6;
	return _paf_decodeVideoPages + page * 256 * 256 + y * 2 * 256 + x * 2;
}

/*static void getMask(uint8 code, uint32 &m1, uint32 &m2) {
	uint32 _eax = (code >> 4) & 15;
	m1 = 0;
	if ((_eax & 1) == 0) m1 |= 0xFF000000;
	if ((_eax & 2) == 0) m1 |= 0x00FF0000;
	if ((_eax & 4) == 0) m1 |= 0x0000FF00;
	if ((_eax & 8) == 0) m1 |= 0x000000FF;

	_eax = code & 15;
	m2 = 0;
	if ((_eax & 1) == 0) m2 |= 0xFF000000;
	if ((_eax & 2) == 0) m2 |= 0x00FF0000;
	if ((_eax & 4) == 0) m2 |= 0x0000FF00;
	if ((_eax & 8) == 0) m2 |= 0x000000FF;
}*/

/*static void movsd(uint8 *dst, const uint8 *src) {
	memcpy(dst, src, 4);
}

static void movsd_m(uint8 mask, uint8 *dst, const uint8 *src) {
	for (int i = 0; i < 4; ++i)
		if (mask & (1 << (3 - i)))
			dst[i] = src[i];
//	if ((mask & 8) != 0) dst[0] = src[0];
//	if ((mask & 4) != 0) dst[1] = src[1];
//	if ((mask & 2) != 0) dst[2] = src[2];
//	if ((mask & 1) != 0) dst[3] = src[3];
}

static void movsd_m(uint8 mask, uint8 *dst, uint8 color) {
	for (int i = 0; i < 4; ++i)
		if (mask & (1 << (3 - i)))
			dst[i] = color;
//	if ((mask & 8) != 0) dst[0] = color;
//	if ((mask & 4) != 0) dst[1] = color;
//	if ((mask & 2) != 0) dst[2] = color;
//	if ((mask & 1) != 0) dst[3] = color;
}*/

static const uint8 byte_43E984[] = {
	// 0
	0, 0, 0, 0, 0, 0, 0, 0,
	2, 0, 0, 0, 0, 0, 0, 0,
	5, 7, 0, 0, 0, 0, 0, 0,
	5, 0, 0, 0, 0, 0, 0, 0,
	// 4
	6, 0, 0, 0, 0, 0, 0, 0,
	5, 7, 5, 7, 0, 0, 0, 0,
	5, 7, 5, 0, 0, 0, 0, 0,
	5, 7, 6, 0, 0, 0, 0, 0,
	// 8
	5, 5, 0, 0, 0, 0, 0, 0,
	3, 0, 0, 0, 0, 0, 0, 0,
	6, 6, 0, 0, 0, 0, 0, 0,
	2, 4, 0, 0, 0, 0, 0, 0,
	// 12
	2, 4, 5, 7, 0, 0, 0, 0,
	2, 4, 5, 0, 0, 0, 0, 0,
	2, 4, 6, 0, 0, 0, 0, 0,
	2, 4, 5, 7, 5, 7, 0, 0
};

#if 0
static uint8 updateSequence[16];

static uint8 *getVideoUpdateSequenceOpcodes(uint8 code) {
	int i = 0;
	const uint8 *src = &byte_43E984[(code >> 4) * 8];
	while (*src != 0) {
		updateSequence[i++] = *src++;
	}
	updateSequence[i++] = 0;
	src = &byte_43E984[(code & 15) * 8];
	while (*src != 0) {
		updateSequence[i++] = *src++;
	}
	updateSequence[i] = 1;
	assert(i < 16);
	return updateSequence;

/*	static uint8 temp[16];
	const uint8 *_ecx = &byte_43E984[(code >> 4) * 8];
	uint8 *_eax = temp;
	uint8 _dl = *_ecx++;
	*_eax++ = _dl;
	while (_dl != 0) {
		_dl = *_ecx++;
		*_eax++ = _dl;
	}
	const uint8 *_edx = &byte_43E984[(code & 15) * 8];
	uint8 _cl = *_edx;
	while (_cl != 0) {
		*_eax++ = _cl;
		_cl = _edx[1];
		++_edx;
	}
	*_eax = 1;
	assert(_eax <= temp + 16);
	return temp;*/
}
#endif

static void paf_copy_4x4_h(uint8 *dst, const uint8 *src) {
	for (int i = 0; i < 4; ++i) {
		memcpy(dst, src, 4);
		src += 4;
		dst += 256;
	}
}

static void paf_copy_4x4_v(uint8 *dst, const uint8 *src) {
	for (int i = 0; i < 4; ++i) {
		memcpy(dst, src, 4);
		src += 256;
		dst += 256;
	}
}

static void paf_copy_src_mask(uint8 mask, uint8 *dst, const uint8 *src) {
	for (int i = 0; i < 4; ++i) {
		if (mask & (1 << (3 - i))) {
			dst[i] = src[i];
		}
	}
}

static void paf_copy_color_mask(uint8 mask, uint8 *dst, uint8 color) {
	for (int i = 0; i < 4; ++i) {
		if (mask & (1 << (3 - i))) {
			dst[i] = color;
		}
	}
}

void PAF_Player::decode0(const uint8 *src, uint8 code, const uint8 *_paf_frameData) {
	uint8 _cl = *src++;
	if (_cl != 0) {
		if ((code & 0x10) != 0) {
			int _edx = src - _paf_frameData;
			int _eax = 4;
			_edx &= 3;
			if (_edx != 0) {
				_eax -= _edx;
				src += _eax;
			}
		}
		do {
			uint8 *_edi = getDecodePagePtr(src[0], src[1]);
			int _edx = (src[1] & 0x7F) * 2;
			int _ebx = READ_LE_UINT16(src + 2); src += 4;
			_ebx += _edx;
			do {
				++_edx;
				paf_copy_4x4_h(_edi, src);
				src += 16;
				if ((_edx & 0x3F) == 0) {
					_edi += 256 * 3;
				}
				_edi += 4;
			} while (_edx < _ebx);
		} while (--_cl != 0);
	}
	uint8 *_edi = _paf_decodeVideoPages + (_paf_currentDecodeVideoPage << 16);
	uint32 _ebx_i = 0;
	do {
		uint8 *_ecx = getDecodePagePtr(src[0], src[1]); src += 2;
		paf_copy_4x4_v(_edi, _ecx);
		++_ebx_i;
		if ((_ebx_i & 0x3F) == 0) {
			_edi += 256 * 3;
		}
		_edi += 4;
	} while (_ebx_i < 256 * 192 / 16);

	uint32 _eax = READ_LE_UINT16(src); src += 4;

	const uint8 *dword_4474F0 = src; // block update sequence
	src += _eax;
	// init_get_bits(dword_4474F0, _eax); // ALT_LE_BITSTREAM

	uint32 _edx = 0;
	uint8 *_ebx = 0;
	uint8 color = 0;

	_edi = _paf_decodeVideoPages + (_paf_currentDecodeVideoPage << 16);

	uint8 byte_4474F8 = 192 / 4; // h
	do {
		uint8 byte_4474F9 = 256 / 4; // w
		do {
			if ((byte_4474F9 & 1) == 0) {
				_eax = *dword_4474F0 >> 4;
			} else {
				_eax = *dword_4474F0 & 15;
				++dword_4474F0;
			}
//			_eax = get_bits(&gb, 4);
			const uint8 *subopcodes = &byte_43E984[_eax * 8];
			while (*subopcodes) {
				_edx = 256 * 2;
				uint8 code = *subopcodes++;
				switch (code) {
				case 2:
					_edx = 0;
				case 3:
					color = *src++;
				case 4:
					_eax = *src++;
					paf_copy_color_mask(_eax >> 4, _edx + _edi, color);
					_edx += 256;
					paf_copy_color_mask(_eax & 15, _edx + _edi, color);
					break;
				case 5:
					_edx = 0;
				case 6:
					_ebx = getDecodePagePtr(src[0], src[1]); src += 2;
				case 7:
					_eax = *src++;
					paf_copy_src_mask(_eax >> 4, _edx + _edi, _edx + _ebx);
					_edx += 256;
					paf_copy_src_mask(_eax & 15, _edx + _edi, _edx + _ebx);
					break;
				}
			}
			_edi += 4;
		} while (--byte_4474F9 != 0);
		_edi += 256 * 3;
	} while (--byte_4474F8 != 0);
}

#if 0
void PAF_Player::decode0(const uint8 *src, uint8 code, const uint8 *_paf_frameData) {
	uint8 _cl = *src++;
	if (_cl != 0) {
		if ((code & 0x10) != 0) {
			int _edx = src - _paf_frameData;
			int _eax = 4;
			_edx &= 3;
			if (_edx != 0) {
				_eax -= _edx;
				src += _eax;
			}
		}
		do {
			uint8 *_edi = getDecodePagePtr(src[0], src[1]);
			int _edx = (src[1] & 0x7F) * 2;
			int _ebx = READ_LE_UINT16(src + 2); src += 4;
			_ebx += _edx;
			do {
				++_edx;
				paf_copy_4x4_h(_edi, src);
				src += 16;
				if ((_edx & 0x3F) == 0) {
					_edi += 256 * 3;
				}
				_edi += 4;
			} while (_edx < _ebx);
		} while (--_cl != 0);
/*		do { // fill 4x4 blocks
			uint8 byte_4474F9 = _cl;
			int _eax = *src++;
			int _edx = *src; src += 3;
			int _ebx = READ_LE_UINT16(src - 2);
			uint8 *_edi = getDecodePagePtr(_eax, _edx);
			_edx &= 0x7F;
			_edx *= 2;
			_ebx += _edx;
			do {
				++_edx;
				movsd(_edi, src); src += 4;
				movsd(_edi + 256, src); src += 4;
				movsd(_edi + 512, src); src += 4;
				movsd(_edi + 768, src); src += 4;
				if ((_edx & 0x3F) == 0) {
					_edi += 768;
				}
				_edi += 4;
			} while (_edx < _ebx);
			_cl = byte_4474F9 - 1;
		} while (_cl != 0);*/
	}
	uint8 *_edi = _paf_decodeVideoPages + (_paf_currentDecodeVideoPage << 16);
	uint32 _ebx_i = 0;
	do { // copy 4x4 blocks
		uint8 *_ecx = getDecodePagePtr(src[0], src[1]); src += 2;
		paf_copy_4x4_v(_edi, _ecx);
/*		movsd(_edi, _ecx);
		movsd(_edi + 256, _ecx + 256);
		movsd(_edi + 512, _ecx + 512);
		movsd(_edi + 768, _ecx + 768);*/
		++_ebx_i;
		if ((_ebx_i & 0x3F) == 0) {
			_edi += 768;
		}
		_edi += 4;
	} while (_ebx_i < 256 * 192 / 16);
	uint32 _eax = READ_LE_UINT16(src); src += 4;

	const uint8 *dword_4474F0 = src; // block update sequence
	src += _eax;

	uint32 _edx = 0;
	uint8 *_ebx = 0, *_paf_decode6Sub0TableIndex;
	uint8 color = 0;

	_edi = _paf_decodeVideoPages + (_paf_currentDecodeVideoPage << 16);

	uint8 byte_4474F8 = 192 / 4; //48; // 192 / 48 == 4
	do {
		uint8 byte_4474F9 = 256 / 4; //32; // 256 / 32 == 8
		do {
//			_eax = *dword_4474F0++;
//			if (_eax == 0) {
//				_edi += 4;
//			} else {
				if ((byte_4474F9 & 1) == 0) {
					_eax = *dword_4474F0 >> 4;
				} else {
					_eax = *dword_4474F0 & 15;
					++dword_4474F0;
				}
	//			_eax = get_bits(&gb, 4);
				const uint8 *subopcodes = &byte_43E984[_eax * 8];
				while (*subopcodes) {	//			_paf_decode6Sub0TableIndex = getVideoUpdateSequenceOpcodes(_eax);
//				while (*_paf_decode6Sub0TableIndex != 1) {
					_edx = 256 * 2;
					uint8 _al = *subopcodes++; //*_paf_decode6Sub0TableIndex++;
					switch (_al) {
					case 0: // skip
						_edi += 4;
						break;

					case 2: // fill 1
						_edx = 0;
					case 3: // fill 2
						color = *src++;
					case 4: // fill 3
						_eax = *src++;
						movsd_m(_eax >> 4, _edx + _edi, color);
						_edx += 256;
						movsd_m(_eax & 15, _edx + _edi, color);
						break;

					case 5: // copy 1
						_edx = 0;
					case 6: // copy 2
						_ebx = getDecodePagePtr(src[0], src[1]); src += 2;
					case 7: // copy 3
						_eax = *src++;
						movsd_m(_eax >> 4, _edx + _edi, _edx + _ebx);
						_edx += 256;
						movsd_m(_eax & 15, _edx + _edi, _edx + _ebx);
						break;

					default:
						printf("Invalid decode subopcode 0 %d\n", _al);
						exit(0);
					} // switch
				}
//			}
			_edi += 4;
		} while (--byte_4474F9 != 0);
		_edi += 256 * 3;
	} while (--byte_4474F8 != 0);
}
#endif

int PAF_Player::decode(const uint8 *src) {
	const uint8 *_paf_frameData = src;
	uint8 code = *src++; // _paf_decode6Code
//	decodeCount++;
	if (code & 0x20) {
		memset(_paf_decodeVideoPages, 0, 256 * 256 * 4); // _paf_pagesBuffers
		_paf_currentDecodeVideoPage = 0; // _paf_currentDrawingPage
		memset(_paf_paletteBuffer, 0, 768);
	}
	if (code & 0x40) { // _paf_decode6Code2 = code & 0x40;
		int index = *src++;
		int count = 1 + *src++;
		if (index + count > 256) { printf("invalid palette index %d count %d\n", index, count); exit(0); }
		if (count > 256) { printf("invalid palette count %d\n", count); exit(0); }
		memcpy(&_paf_paletteBuffer[index * 3], src, count * 3);
		src += count * 3;
	}
//	printf("code %d (&0x10=%d)\n", code & 15, (code & 16) != 0 ? 1 : 0);
	switch (code & 0xF) {
	case 0:
		decode0(src, code, _paf_frameData);
		break;
#if 0
	case 0: {
			uint8 _cl = *src++;
//			const uint8 *_edx = src;
			if (_cl != 0) {
				if ((code & 0x10) != 0) {
					int _edx = src - _paf_frameData;
					int _eax = 4;
					_edx &= 3;
					if (_edx != 0) {
						_eax -= _edx;
						src += _eax;
					}
				}
//loc_42F0EE:
				do { // fill 4x4 blocks
					int _eax = 0;
					uint8 byte_4474F9 = _cl;
					int _edx = 0;
//					int _ecx = 0;
					int _ebx = 0;
					_eax = *src++;
					_edx = *src; src += 3;
//					uint8 *_ebp_p = _paf_unk47[_eax]; // * 4];
					_ebx = READ_LE_UINT16(src - 2);
//					_edi = _paf_unk50[_edx]; // * 4];
//					_dl &= 0x7F; // &= ~0x80;
//					_edi += _ebp;
//					uint8 *_edi = _ebp_p + _paf_unk50[_edx];

					uint8 *_edi = getDecodePagePtr(_eax, _edx);

					_edx &= 0x7F;
					_edx *= 2;
//					_ecx = 256; // ++_ch;
					_ebx += _edx;
//					int _ebp = 4;
//loc_42F12C:
					do {
						++_edx;
//						*(uint32 *)_edi = *(uint32 *)src;
//						*(uint32 *)(_edi + _ecx) = *(uint32 *)(src + _ebp);
//						*(uint32 *)(_edi + _ecx * 2) = *(uint32 *)(src + _ebp * 2);
//						*(uint32 *)(_edi + _ecx * 2 + 256) = *(uint32 *)(src + _ebp * 3);
//						src = src + _ebp * 4;
//						_edi += _ebp;
						movsd(_edi, src); src += 4;
						movsd(_edi + 256, src); src += 4;
						movsd(_edi + 512, src); src += 4;
						movsd(_edi + 768, src); src += 4;
//						uint8 _dl = _edx & 0xFF;
						if ((_edx & 0x3F) == 0) {
							_edi += 768; //_edi = _edi + _ecx * 2 + 256;
						}
						_edi += 4;
					} while (_edx < _ebx);
					_cl = byte_4474F9 - 1;
				} while (_cl != 0);
			}
//loc_42F166:
//			printf("code 0, page %d\n", _paf_currentDecodeVideoPage);
			uint8 *_edi = _paf_decodeVideoPages + (_paf_currentDecodeVideoPage << 16);
			uint32 _ebx_i = 0;
//			int _edx = 0;
//loc_42F17C:
			do { // copy 4x4 blocks
//				int _eax = 0;
//				uint8 _dl = src[0];
//				uint8 _al = src[1];
//				uint8 *_ebp = _paf_unk47[_dl]; // _edx * 4
//				uint32 _ecx_i = _paf_unk50[_al]; // _eax * 4
//				uint8 *_ecx = _ebp + _ecx_i;
//				*(uint32 *)_edi = *(uint32 *)_ecx;
//				*(uint32 *)(_edi + 256) = *(uint32 *)(_ecx + 256);
//				*(uint32 *)(_edi + 512) = *(uint32 *)(_ecx + 512);
//				*(uint32 *)(_edi + 768) = *(uint32 *)(_ecx + 768);
				uint8 *_ecx = getDecodePagePtr(src[0], src[1]); src += 2;
				movsd(_edi, _ecx);
				movsd(_edi + 256, _ecx + 256);
				movsd(_edi + 512, _ecx + 512);
				movsd(_edi + 768, _ecx + 768);
				++_ebx_i;
//				uint8 _bl = _ebx_i & 0xFF;
				if ((_ebx_i & 0x3F) == 0) {
					_edi += 768;
				}
				_edi += 4;
			} while (_ebx_i < 256 * 192 / 16);
			uint32 _eax = READ_LE_UINT16(src); src += 4;
			const uint8 *dword_4474F0 = src; // block update sequence
			src += _eax;
//			printf("code 0, size %d page %d\n", _eax, _paf_currentDecodeVideoPage);
			uint8 byte_4474F8, byte_4474F9;
			uint32 _edx = 0; //, _ebp = 0, _ecx_i = 0, _null;
			uint8 *_ebx, *_paf_decode6Sub0TableIndex;
			const uint8 *_ecx;
			uint8 *_edx_p;
			uint8 _bl, color = 0;
			uint8 _al = 48; // 192 / 48 == 4
			_edi = _paf_decodeVideoPages + (_paf_currentDecodeVideoPage << 16);
loc_42F1FC:
			_bl = 32; // 256 / 32 == 8
			byte_4474F8 = _al;
loc_42F203:
			_eax = 0;
			byte_4474F9 = _bl;

			_ecx = dword_4474F0;
//			_ebx = _paf_unk48;
//			_eax = *_ecx++;
//			_eax *= 16;

			_eax = *_ecx++;
			_ebx = getVideoUpdateSequenceOpcodes(_eax);

			dword_4474F0 = _ecx;
			if (_eax == 0) { //goto loc_42F274;
				_edi += 4;
				goto paf_decode6Sub0_1;
			}
			//_ebx += _eax;
//			printf("opcodes offset %d\n", _eax );
//			_edx = 512;
//			_al = *_ebx++;
			_paf_decode6Sub0TableIndex = _ebx;
			while (1) {
				_edx = 512; // XXX
				_al = *_paf_decode6Sub0TableIndex++; // XXX
//				printf("code 0 subcode %d offset %d\n", _al, _paf_decode6Sub0TableIndex - _paf_unk48);
				switch (_al) {
				case 0: // skip
//paf_decode6Sub0_0:
					_edi += 4;
					break;
				case 1: // next block
paf_decode6Sub0_1:
					_edi += 4;
					_bl = byte_4474F9 - 1;
					if (_bl != 0) goto loc_42F203;

					_edi += 768;
					_al = byte_4474F8 - 1;
					if (_al != 0) goto loc_42F1FC;
					return src - _paf_frameData;
				case 2: // fill 1
//paf_decode6Sub0_2:
					_edx = 0;
//					goto paf_decode6Sub0_3;
				case 3: // fill 2
//paf_decode6Sub0_3:
//					_eax = 0;
					_al = *src++;
					//_ebp = _paf_remapPalette[_al];
//					_ebp = (_al << 24) | (_al << 16) | (_al << 8) | _al;
					color = _al;
//					goto paf_decode6Sub0_4;
				case 4: // fill 3
//paf_decode6Sub0_4:
//					_eax = 0;
					_edx_p = _edx + _edi;
					_eax = *src++;
//						getMask(_eax, _eax, _ecx_i);

//					_ecx = _paf_unk16 + _eax * 8;
//					_eax *= 8;
//					_ecx += _eax;
//						_ebx_i = *(uint32 *)_edx_p;
//					_eax = *(uint32 *)_ecx; // mask
//					_ecx += 4;

//					_ebx_i &= _eax;  // dst & mask
//					_eax = ~_eax;
//					_eax &= _ebp; // src & ~mask
//					_eax |= _ebx_i;

//						*(uint32 *)_edx_p = (_ebp & ~_eax) | (_ebx_i & _eax);
					movsd_m(_eax >> 4, _edx_p, color);

//					_ecx_i = *(uint32 *)_ecx;
					_edx_p += 256;
//					_eax = 0;
//						_ebx_i = *(uint32 *)_edx_p;
//					_ebx_i &= _ecx_i; // dst & mask
//					_ecx_i = ~_ecx_i;
//					_ecx_i &= _ebp; // src & ~mask
//					_ecx_i |= _ebx_i;
//						*(uint32 *)_edx_p = (_ebp & ~_ecx_i) | (_ebx_i & _ecx_i);
					movsd_m(_eax & 15, _edx_p, color);
					break;
				case 5: // copy 1
//paf_decode6Sub0_5:
					_edx = 0;
//					goto paf_decode6Sub0_6;
				case 6: // copy 2
//paf_decode6Sub0_6:
//					_eax = 0;
//					_al = *src++;
//					_ebx = _paf_unk47[_al];
//					_al = *src++;
//					_ebx += _paf_unk50[_al];
					_ebx = getDecodePagePtr(src[0], src[1]); src += 2;
//					goto paf_decode6Sub0_7;
				case 7: // copy 3
//paf_decode6Sub0_7:
//					_eax = 0;
//					_ecx = _paf_unk16;
//					_eax = *src;
//					_ecx_i = *(uint32 *)(_paf_unk16 + _eax * 8 + 0); // mask
//						getMask(_eax, _ecx_i, _null);
//						_ebp = *(uint32 *)(_edx + _ebx); // src
//						_eax = *(uint32 *)(_edx + _edi); // dst
//					_eax &= _ecx_i; // dst & mask
//					_ecx_i = ~_ecx_i;
//					_ebp &= _ecx_i; // src & ~mask
//					_ebp |= _eax;
//					_eax = 0;
//						*(uint32 *)(_edx + _edi) = (_ebp & ~_ecx_i) | (_eax & _ecx_i);

					_eax = *src++;
					movsd_m(_eax >> 4, _edx + _edi, _edx + _ebx);

//					_eax = *src++;
					_edx += 256;
//					_ecx = _paf_unk16;
//					_ecx_i = *(uint32 *)(_paf_unk16 + _eax * 8 + 4); // mask
//						getMask(_eax, _null, _ecx_i);
//						_ebp = *(uint32 *)(_edx + _ebx); // src
//						_eax = *(uint32 *)(_edx + _edi); // dst
//					_eax &= _ecx_i; // dst & mask
//					_ecx_i = ~_ecx_i;
//					_ebp &= _ecx_i; // src & ~mask
//					_ebp |= _eax;
//						*(uint32 *)(_edx + _edi) = (_ebp & ~_ecx_i) | (_eax & _ecx_i);
					movsd_m(_eax & 15, _edx + _edi, _edx + _ebx);
					break;
				default:
					printf("Invalid decode subopcode 0 %d\n", _al);
					exit(0);
				} // switch
			}
		}
		break;
#endif
	case 1: {
			uint8 *_edi = _paf_decodeVideoPages + (_paf_currentDecodeVideoPage << 16);
			src += 2;
			memcpy(_edi, src, 256 * 192);
			src += 256 * 192;
		}
		break;
	case 2: {
			uint8 page = *src++;
			if (page != _paf_currentDecodeVideoPage) {
				if (page >= 4) { printf("invalid page %d\n", page); exit(0); }
				uint8 *_esi = _paf_decodeVideoPages + (page << 16);
				uint8 *_edi = _paf_decodeVideoPages + (_paf_currentDecodeVideoPage << 16);
				memcpy(_edi, _esi, 256 * 192);
			}
		}
		break;
	case 4: {
			uint8 *_edi = _paf_decodeVideoPages + _paf_currentDecodeVideoPage * 256 * 256;
			src += 2;
			int size = 256 * 192;
			while (size != 0) {
				int8 _al = *src++;
				if (_al < 0) {
					int count = -_al;
					++count;
					uint8 color = *src++;
					memset(_edi, color, count);
					_edi += count;
					size -= count;
				} else {
					int count = _al + 1;
					memcpy(_edi, src, count);
					_edi += count;
					src += count;
					size -= count;
				}
			}
		}
		break;
	default:
		printf("Invalid code %d (ignored in the original)\n", code & 0xF);
		exit(0);
		break;
	}
	return src - _paf_frameData;
}

static void decode_sound_2205(const uint8 *src) {
	int16 _snd_unkVar40[256 * 2];
	for (int i = 0; i < 256; ++i) {
		int16 val = READ_LE_UINT16(src); src += 2;
		_snd_unkVar40[i] = _snd_unkVar40[256 + i] = val;
	}
	int count = 2205;
	while (count--) {
		uint8 l = *src++;
		int16 lval = _snd_unkVar40[l];
		if (lval < -0x2000) {
			lval = -0x2000;
		} else if (lval > 0x1FFF) {
			lval = 0x1FFF;
		}
		soundWriter->Write((const uint8 *)&lval, 2);

		uint8 r = *src++;
		int16 rval = _snd_unkVar40[256 + r];
		if (rval < -0x2000) {
			rval = -0x2000;
		} else if (rval > 0x1FFF) {
			rval = 0x1FFF;
		}
		soundWriter->Write((const uint8 *)&rval, 2);
	}
} // framesize = 256 * 2 + 2205 * 2 == 4922

#define BUFFER_SIZE_2205 4922

// sox -w -s -r 22050 -c 2 paf.raw paf.wav

static void appendAudioSegment(uint32 dst_offset, const uint8 *src, int bufferSize) {
	memcpy(_paf_audioBuffer + dst_offset, src, bufferSize);
	if (_paf_hdrSoundType == 100) {
//		if (dst_offset == 0x1F000) { // XXX how to dim that ?
		assert((_paf_hdrAudioFrameBlocksCount - 2) * _paf_hdrReadBufferSize == 0x1F000);
		if (dst_offset == (_paf_hdrAudioFrameBlocksCount - 2) * _paf_hdrReadBufferSize) {
//			int buffers_count = 0x1F800 / BUFFER_SIZE_2205;
			int buffers_count = ((_paf_hdrAudioFrameBlocksCount - 1) * _paf_hdrReadBufferSize) / BUFFER_SIZE_2205;
			for (int i = 0; i < buffers_count; ++i) {
				decode_sound_2205(_paf_audioBuffer + i * BUFFER_SIZE_2205);
			}
		}
	}
}

static const uint32 defaultBufferSize = 32 * 1024 * 1024;

bool PAF_Player::open(const char *filename, int num) {
	printf("opening '%s'\n", filename);
	if (_fd.open(filename, "rb")) {
		preload(num);
		uint8 *p = (uint8 *)malloc(defaultBufferSize);
		_fd.read(p, defaultBufferSize);
		_width = 256;
		_height = 192;
		_paf_currentDecodeVideoPage = 0;
		_paf_decodeVideoPages = (uint8 *)malloc(256 * 256 * 4);
		for (int i = 0; i < 256; ++i) {
			_paf_paletteBuffer[i * 3] = _paf_paletteBuffer[i * 3 + 1] = _paf_paletteBuffer[i * 3 + 2] = i * 63 / 256;
		}
		memset(_paf_decodeVideoPages, 0, 256 * 256 * 4);
		_paf_currentDecodeVideoPage = 0;
		memset(_paf_paletteBuffer, 0, 768);
		sysInit(256, 192, filename);

		char buf[256];
		sprintf(buf, "s_%02d", num);
		soundWriter = createWavSoundWriter();
		soundWriter->Open(buf, 22050, 16, 2, true);

		int block_index = 0;
		int src_offset = 0;
		for (int i = 0; i < _paf_animFramesCount; ++i) {
			int blocks_for_frame = (i == 0) ? _paf_hdrPreloadFrameBlocksCount : paf_headerData[i - 1];
			while (blocks_for_frame != 0) {
				int dst_offset = paf_headerDataPtr2[block_index] & ~(1 << 31);
				if (paf_headerDataPtr2[block_index] & (1 << 31)) {
					appendAudioSegment(dst_offset, p + src_offset, 2048);
				} else {
					memcpy(_paf_videoBuffer + dst_offset, p + src_offset, 2048);
				}
				src_offset += 2048;
				++block_index;
				--blocks_for_frame;
			}
			printf("frame %d/%d offset 0x%X blocks_for_frame %d block_index %d\n", i, _paf_animFramesCount, paf_headerDataPtr1[i], blocks_for_frame, block_index);
			decode(_paf_videoBuffer + paf_headerDataPtr1[i]);

			sprintf(buf, "i_%02d_%04d", num, i);
//			ImageWriter *imageWriter = createBmpImageWriter();
			ImageWriter *imageWriter = createPngImageWriter();
			imageWriter->Open(buf, _width, _height);
			imageWriter->Write(_paf_decodeVideoPages + (_paf_currentDecodeVideoPage << 16), _width, _paf_paletteBuffer);
			imageWriter->Close();
			delete imageWriter;

			sysSetPalette(_paf_paletteBuffer, 256);
			assert(_paf_currentDecodeVideoPage >= 0 && _paf_currentDecodeVideoPage < 4);
			sysCopyRect(_paf_decodeVideoPages + (_paf_currentDecodeVideoPage << 16), _width, 0, 0, _width, _height);
			++_paf_currentDecodeVideoPage;
			_paf_currentDecodeVideoPage &= 3;
			if (sysDelay(80)) {
				break;
			}
		}

		soundWriter->Close();
		delete soundWriter;
		return true;
	}
	return false;
}

void PAF_Player::close() {
	sysDestroy();
	_fd.close();
}

void PAF_Player::play() {
}

/*static uint32 SWAP_UINT32(uint32 n) {
	uint32 a = (n & 0xFF000000) >> 24;
	uint32 b = (n & 0x00FF0000) >> 16;
	uint32 c = (n & 0x0000FF00) >> 8;
	uint32 d = (n & 0x000000FF);
	return a | (b << 8) | (c << 16) | (d << 24);
}*/

//void PAF_Player::initLookupTables() {
//	for (int i = 0; i < 256; ++i) {
//		uint32 _ecx = i & 0xC0;
//		uint32 _edx = i & 0x3F;
//		_ecx += _edx;
//		_paf_unk47[i] = (_ecx << 10) + _paf_decodeVideoPages;
//		_paf_unk47[i] = i * 256 * 4 + _paf_decodeVideoPages;
		// (i & 0xC0) >> 6 == page_num
		// (i & 0x3F) * 4 == page start line

//		uint32 _ecx = i & 0x80;
//		uint32 _edx = i & 0x7F;
//		_ecx = _edx + _ecx * 2;
//		_paf_unk50[i] = (_ecx << 1); // (i & 127) * 2 + (i & 128) * 4
//		int dy = ((i & 0x80) != 0) ? 256 * 2 : 0;
//		int dx = (i & 0x7F) * 2;
//		_paf_unk50[i] = dy + dx;
		// (i & 0x80) >> 7 = block2x2_y
		// (i & 0x7F) * 2 = block2x2_x
//	}

//	memset(_paf_unk48, 0, sizeof(_paf_unk48));
//	uint32 _esi = 0;
//	uint32 _edi = 0;
//	do {
//		const uint8 *_ecx = &byte_43E984[(_esi >> 4) * 8];
//		uint8 *_eax = _paf_unk48 + _edi;
//		uint8 _dl = *_ecx++;
//		*_eax++ = _dl;
//		while (_dl != 0) {
//			_dl = *_ecx++;
//			*_eax++ = _dl;
//		}
//		const uint8 *_edx = &byte_43E984[(_esi & 15) * 8];
//		uint8 _cl = *_edx;
//		while (_cl != 0) {
//			*_eax++ = _cl;
//			_cl = _edx[1];
//			++_edx;
//		}
//		_edi += 16;
//		++_esi;
//		*_eax = 1;
//		assert(_eax < _paf_unk48 + sizeof(_paf_unk48));
//	} while (_edi < 256 * 16);

//	for (int i = 0; i < 256; ++i) {
//		uint32 m = (i << 24) | (i << 16) | (i << 8) | i;
//		_paf_remapPalette[i] = m;
//	}

/*	memset(_paf_unk16, 0, sizeof(_paf_unk16));
	uint8 *_edi_p = _paf_unk16;
	uint32 _esi = 0;
	do {
		uint32 _eax = (_esi >> 4) & 15;
		uint32 v = 0;
		if ((_eax & 1) == 0) v |= 0xFF000000;
		if ((_eax & 2) == 0) v |= 0x00FF0000;
		if ((_eax & 4) == 0) v |= 0x0000FF00;
		if ((_eax & 8) == 0) v |= 0x000000FF;
		*(uint32 *)_edi_p = v; _edi_p += 4;

		_eax = _esi & 15;
		v = 0;
		if ((_eax & 1) == 0) v |= 0xFF000000;
		if ((_eax & 2) == 0) v |= 0x00FF0000;
		if ((_eax & 4) == 0) v |= 0x0000FF00;
		if ((_eax & 8) == 0) v |= 0x000000FF;
		*(uint32 *)_edi_p = v; _edi_p += 4;

//		uint32 _eax = (_esi >> 4) & 15;
//		uint32 _ecx = 0;
//		uint32 _edx = 4;
//		do {
//			_ecx <<= 8;
//			if ((_eax & 8) == 0) {
//				_ecx |= 0xFF;
//			}
//			_eax <<= 1;
//			--_edx;
//		} while (_edx != 0);
//		*(uint32 *)_edi_p = SWAP_UINT32(_ecx);
//		_edi_p += 4;
//
//		_eax = _esi & 15;
//		_ecx = 0;
//		_edx = 4;
//		do {
//			_ecx <<= 8;
//			if ((_eax & 8) == 0) {
//				_ecx |= 0xFF;
//			}
//			_eax <<= 1;
//			--_edx;
//		} while (_edx != 0);
//		*(uint32 *)_edi_p = SWAP_UINT32(_ecx);
//		_edi_p += 4;
//		assert(_edi_p <= _paf_unk16 + sizeof(_paf_unk16));

		++_esi;
	} while (_esi < 256);*/
//}

void paf_preloadHelper(File *pf, int count, uint32 *dst) {
	assert((count & 3) == 0);
	for (int i = 0; i < count / 4; ++i) {
		dst[i] = pf->readUint32LE();
	}
	count &= 0x7FF;
	if (count != 0) {
		char temp[0x800];
		pf->read(temp, 0x800 - count);
	}
}

void PAF_Player::preload(int num) {
	int fileOffset = 0;
	if (num != -1) { // packed_file
		assert(num >= 0 && num < 50);
		_fd.seek(num * 4);
		fileOffset = _fd.readUint32LE();
		if (fileOffset == 0) {
			printf("No video found\n");
			exit(0);
		}
		_fd.seek(fileOffset);
	}

	int fileSize = _fd.size();
	uint8 hdr[0x800];
	_fd.read(hdr, 0x800); // 0xBC

	_paf_startOffset = READ_LE_UINT32(hdr + 0xA4);
	printf("_paf_startOffset = 0x%X\n", _paf_startOffset);

	_paf_hdrSoundType = READ_LE_UINT32(hdr + 0x88);
	printf("_paf_hdrSoundType = %d\n", _paf_hdrSoundType);

	_paf_hdrPreloadFrameBlocksCount = READ_LE_UINT32(hdr + 0x9C); // load %d buffers for frame 0
	printf("_paf_hdrPreloadFrameBlocksCount = %d\n", _paf_hdrPreloadFrameBlocksCount);

	_paf_hdrReadBufferSize = READ_LE_UINT32(hdr + 0x98);
	printf("_paf_hdrReadBufferSize = %d\n", _paf_hdrReadBufferSize);

	_paf_animFramesCount = READ_LE_UINT32(hdr + 0x84);
	printf("_paf_animFramesCount = %d\n", _paf_animFramesCount); // total number of frames

	_paf_hdrVideoFrameBlocksCount = READ_LE_UINT32(hdr + 0xA8);
	printf("_paf_hdrVideoFrameBlocksCount = %d\n", _paf_hdrVideoFrameBlocksCount);

	_paf_hdrAudioFrameBlocksCount = READ_LE_UINT32(hdr + 0xAC);
	printf("_paf_hdrAudioFrameBlocksCount = %d\n", _paf_hdrAudioFrameBlocksCount);

	_paf_unkA0 = READ_LE_UINT32(hdr + 0xA0);
	printf("_paf_unkA0 = %d\n", _paf_unkA0);

	assert(_paf_hdrReadBufferSize == 2048);
	assert(_paf_hdrAudioFrameBlocksCount == 0 || _paf_hdrAudioFrameBlocksCount == 64);

	paf_preloadHelper(&_fd, _paf_animFramesCount * 4, paf_headerData);
	int totalNumberOfBuffers = 0;
	for (int i = 0; i < _paf_animFramesCount; ++i) {
//		printf("paf_headerData[%d] = %d\n", i, (int)paf_headerData[i]); // for frame 'i', load %d buffers
		totalNumberOfBuffers += paf_headerData[i];
	}
	printf("totalNumberOfFrameBlocks = %d (size %d bytes)\n", totalNumberOfBuffers, totalNumberOfBuffers * _paf_hdrReadBufferSize);
 	printf("fileSize %d/%d/%d bytes\n", fileSize, _paf_startOffset + (_paf_hdrPreloadFrameBlocksCount + totalNumberOfBuffers) * _paf_hdrReadBufferSize, _paf_startOffset + _paf_unkA0 * _paf_hdrReadBufferSize);

	paf_preloadHelper(&_fd, _paf_animFramesCount * 4, paf_headerDataPtr1);
	for (int i = 0; i < _paf_animFramesCount; ++i) {
//		printf("paf_headerDataPtr1[%d] = 0x%X\n", i, (int)paf_headerDataPtr1[i]); // src offset
	}

//	bool hasSoundData = false;
	paf_preloadHelper(&_fd, _paf_unkA0 * 4, paf_headerDataPtr2);
//	for (int i = 0; i < _paf_unkA0; ++i) {
//		printf("paf_headerDataPtr2[%d] = 0x%X\n", i, (int)paf_headerDataPtr2[i]); // dst offset / frame type
//		if (paf_headerDataPtr2[i] & 0x80000000) {
//			hasSoundData = true;
//		}
//	}

//	if (hasSoundData) {
//		printf("File with sound data !\n");
//	}

	_fd.seek(fileOffset + _paf_startOffset); // XXX
}
