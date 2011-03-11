
#include "intern.h"
#include "staticdata.h"

static const uint32 _res_unpackBitmaskTable[] = { 0, 1, 3, 7, 0xF, 0x1F, 0x3F, 0x7F, 0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF, 0xFFFFFFFF };


static uint32 _res_unpackUnkVar5;
static uint16 _res_unpackUnkVar11[4096];
static uint32 _res_unpackUnkVar4;
static uint8 _res_unpackUnkVar12[8192];
static uint32 _res_unpackUnkVar13;
static const uint8 *_res_unpackBitsData;
static uint32 _res_unpackUnkVar9;
static uint32 _res_unpackBitsMask;
static uint32 _res_unpackUnkVar15;
static uint32 _res_unpackUnkVar7;
static uint32 _res_unpackUnkVar10;
static uint32 _res_unpackBitsCount;

// lzw_decode

int UnpackData(int type, const uint8 *src, uint8 *dst) {
	uint32 _esi, _eax, _ebx, _ecx, _edx, _edi, var8, var10, _ebp_i;
	const uint8 *_ebp;
	uint8 *dst_, *varC, *_ecx_p, *_esi_p, *_edi_p, *_eax_p;

	_esi = type - 1;
	_ebp = src;
	dst_ = dst;
	_res_unpackBitsData = _ebp;
	assert(_esi >= 2 && _esi <= 9);
	_eax = _esi + 1;
	_ecx = _eax;
	_edx = 1 << (_ecx & 0xFF);
	_ecx = _esi;
	_ebx = 0;
	_res_unpackUnkVar7 = _eax;
	_edi = 8;
	var8 = _eax;
	_res_unpackUnkVar9 = _edx;
	_edx = 1 << (_ecx & 0xFF);
	varC = &_res_unpackUnkVar12[8191]; //&_res_unpackUnkVar8;
	_res_unpackUnkVar15 = _edx;
	_ecx = _edx + 1;
	_res_unpackUnkVar5 = _ecx;
	++_ecx;
	_res_unpackUnkVar13 = _res_unpackUnkVar10 = _ecx;
	_ecx = 0;
	++_ebp;
	_res_unpackBitsCount = _ecx;
	_ebx = _ebp[-1];
	_esi = _ebx;
	var10 = _ecx;
	type = _ecx;
	_res_unpackBitsMask = _ebx;
	_res_unpackBitsData = _ebp;
	_res_unpackBitsCount = _edi;
	_res_unpackUnkVar4 = _esi;
	if (_eax > _edi) {
		do {
			_ecx = _edi;
			_ebx = *_ebp++;
			_edi += 8;
			_edx = _ebx;
			_edx <<= (_ecx & 0xFF);
			_res_unpackBitsMask = _ebx;
			_res_unpackBitsData = _ebp;
			_res_unpackBitsCount = _edi;
			_esi |= _edx;
			_res_unpackUnkVar4 = _esi;
		} while (_eax > _edi);
		_edx = _res_unpackUnkVar15;
	}
	assert(_eax <= 12);
	_ecx = _res_unpackBitmaskTable[_eax];
	_edi -= _eax;
	_esi &= _ecx;
	_ecx = _res_unpackUnkVar5;
	_res_unpackBitsCount = _edi;
	_res_unpackUnkVar4 = _esi;
	if (_esi == _ecx) goto end;
	goto loc_405C4F;
loop2:
	_edx = _res_unpackUnkVar15;


	//
	// DECODE MAIN PART A (decode data)
	//


loc_405C4F:
	if (_esi != _edx) goto loc_405DB6;
	_eax = _res_unpackUnkVar13;
	_esi = var8;
	_res_unpackUnkVar10 = _eax;
	_eax = 1;
	_res_unpackUnkVar7 = _ecx = _esi;
	_eax <<= (_ecx & 0xFF);
	_res_unpackUnkVar9 = _eax;
	if (_edi == 0) {
		_edi = 8;
		_ebx = *_ebp++;
		_res_unpackBitsMask = _ebx;
		_res_unpackBitsData = _ebp;
		_res_unpackBitsCount = _edi;
	}
	_ecx = 8;
	_eax = _ebx;
	_ecx -= _edi;
	_eax >>= (_ecx & 0xFF);
	_res_unpackUnkVar4 = _eax;
	if (_esi > _edi) {
		do {
			_ecx = _edi;
			_ebx = *_ebp++;
			_edi += 8;
			_edx = _ebx;
			_edx <<= (_ecx & 0xFF);
			_res_unpackBitsMask = _ebx;
			_res_unpackBitsData = _ebp;
			_res_unpackBitsCount = _edi;
			_eax |= _edx;
			_res_unpackUnkVar4 = _eax;
		} while (_esi > _edi);
		_edx = _res_unpackUnkVar15;
	}
	assert(_esi < 12);
	_ecx = _res_unpackBitmaskTable[_esi];
	_edi -= _esi;
	_eax &= _ecx;
	_res_unpackBitsCount = _edi;
	_res_unpackUnkVar4 = _eax;
	if (_eax == _edx) {
		do {
			if (_edi == 0) {
				_edi = 8;
				_ebx = *_ebp++;
				_res_unpackBitsMask = _ebx;
				_res_unpackBitsData = _ebp;
				_res_unpackBitsCount = _edi;
			}
			_ecx = 8;
			_eax = _ebx;
			_ecx -= _edi;
			_eax >>= (_ecx & 0xFF);
			_res_unpackUnkVar4 = _eax;
			if (_esi > _edi) {
				do {
					_ecx = _edi;
					_ebx = *_ebp++;
					_edi += 8;
					_edx = _ebx << (_ecx & 0xFF);
					_res_unpackBitsMask = _ebx;
					_res_unpackBitsData = _ebp;
					_res_unpackBitsCount = _edi;
					_eax |= _edx;
					_res_unpackUnkVar4 = _eax;
				} while (_esi > _edi);
				_edx = _res_unpackUnkVar15;
			}
			assert(_esi < 12);
			_ecx = _res_unpackBitmaskTable[_esi];
			_edi -= _esi;
			_eax &= _ecx;
			_res_unpackBitsCount = _edi;
			_res_unpackUnkVar4 = _eax;
		} while (_eax == _edx);
	}
//loc_405D83:
	if (_eax == _res_unpackUnkVar5) goto end;
	if (_eax >= _res_unpackUnkVar13) {
		_res_unpackUnkVar4 = _eax = 0;
	}
	_ecx_p = dst_;
	var10 = _eax;
	type = _eax;
	*_ecx_p++ = (_eax & 0xFF);
	dst_ = _ecx_p;
	goto loc_405E79;


	//
	// DECODE MAIN PART B (refill dictionnary)
	//


loc_405DB6:
	_ebp_i = _res_unpackUnkVar10;
	_ecx = _esi;
	if (_esi >= _ebp_i) {
		//_dl = var10 & 0xFF;
		_ecx = type;
		_edi_p = &_res_unpackUnkVar12[8190]; //&_res_unpackUnkVar14;
		_res_unpackUnkVar12[8190] = var10 & 0xFF; //_res_unpackUnkVar14 = _dl;
	} else {
		_edi_p = varC;
	}
	_edx = _res_unpackUnkVar13;
	while (_ecx >= _edx) {
		--_edi_p;
//		printf("pos %d _ecx %d _edx %d _ebx %d\n", _edi_p - &_res_unpackUnkVar12[0], _ecx, _edx, _res_unpackUnkVar11[_ecx]);
		assert(_edi_p >= &_res_unpackUnkVar12[0]);
		*_edi_p = _res_unpackUnkVar12[_ecx];
		_ebx = _res_unpackUnkVar11[_ecx];
		_ecx = _ebx;
	}
	_edx = _res_unpackUnkVar9;
	--_edi_p;
	*_edi_p = (_ecx & 0xFF);
	if (_ebp_i < _edx) {
		_res_unpackUnkVar12[_ebp_i] = _ecx & 0xFF; // XXX ss:
		var10 = _ecx;
		_ecx = type & 0xFFFF; //_cx = READ_LE_UINT16(type);
		type = _esi;
		_res_unpackUnkVar11[_ebp_i] = _ecx; //_res_unpackUnkVar11[_ebp] = _ecx;
//		printf("var11[%d/%d]=%d\n", _ebp_i, _edx, _res_unpackUnkVar11[_ebp_i]);
		++_ebp_i;
		_res_unpackUnkVar10 = _ebp_i;
		if (_ebp_i < _edx) goto loc_405E44;
	}
	if (_eax < 12) {
		_edx <<= 1;
		++_eax;
		_res_unpackUnkVar9 = _edx;
		_res_unpackUnkVar7 = _eax;
	}
loc_405E44:
//	_eax_p = dst_;
	//_ecx = &_res_unpackUnkVar12[8192]; //&_res_unpackUnkVar8;
	//_ecx -= _edi;
	_esi_p = _edi_p;
	_edx = _res_unpackUnkVar4 = &_res_unpackUnkVar12[8191] - _edi_p; //_res_unpackUnkVar4 = _ecx;
	//_edx = _ecx;
//	_edi_p = _eax_p;
	varC = &_res_unpackUnkVar12[8191]; //&_res_unpackUnkVar8;
//	memcpy(_edi_p, _esi_p, _edx);
//	_edi_p += _edx;
//	_esi_p += _edx;
//	_eax_p += _res_unpackUnkVar4;
//	dst_ = _eax_p;
	while (_edx--) { *dst_++ = *_esi_p++; }

	//
	// DECODE END PART
	//


loc_405E79:
	_edi = _res_unpackBitsCount;
	if (_edi == 0) {
		_ebp = _res_unpackBitsData;
		_ebx = *_ebp++;
		_edi = 8;
		_res_unpackBitsMask = _ebx;
		_res_unpackBitsData = _ebp;
		_res_unpackBitsCount = _edi;
	} else {
		_ebp = _res_unpackBitsData;
		_ebx = _res_unpackBitsMask;
	}
	_eax = _res_unpackUnkVar7;
	_ecx = 8;
	_ecx -= _edi;
	_esi = _ebx;
	_esi >>= (_ecx & 0xFF);
	_res_unpackUnkVar4 = _esi;
	while (_eax > _edi) {
		_ecx = _edi;
		_ebx = *_ebp++;
		_edi += 8;
		_edx = _ebx << (_ecx & 0xFF);
		_res_unpackBitsMask = _ebx;
		_res_unpackBitsData = _ebp;
		_res_unpackBitsCount = _edi;
		_esi |= _edx;
		_res_unpackUnkVar4 = _esi;
	}
	assert(_eax <= 12);
	_edx = _res_unpackBitmaskTable[_eax];
	_ecx = _res_unpackUnkVar5;
	_esi &= _edx;
	_edi -= _eax;
	_res_unpackBitsCount = _edi;
	_res_unpackUnkVar4 = _esi;
	if (_esi != _ecx) goto loop2;

end:
//	_eax = dst_;
//	_ecx = dst;
//	_eax -= _ecx;
//	return _eax;
	return dst_ - dst;
}

void writeFile(const char *filename, const uint8 *buf, int bufSize) {
	FILE *fp = fopen(filename, "wb");
	if (fp) {
		fwrite(buf, bufSize, 1, fp);
		fclose(fp);
	}
}

#if 0
static void fix(uint8 *buf, int bufSize) {
	for (int i = 0; i < bufSize; ++i) {
		buf[i] <<= 4;
	}
}
static uint8 decodeBuffer[256 * 192 * 10];
int main(int argc, char *argv[]) {
	int sz1 = UnpackData(9, byte_43D960, decodeBuffer);
	printf("sz1 %d\n", sz1);
	fix(decodeBuffer, sz1);
	writeFile("data_43D960.bin", decodeBuffer, sz1);

	int sz2 = UnpackData(9, byte_43EA78, decodeBuffer);
	printf("sz2 %d\n", sz2);
	fix(decodeBuffer, sz2);
	writeFile("data_43EA78.bin", decodeBuffer, sz2);

	return 0;
}
#endif
