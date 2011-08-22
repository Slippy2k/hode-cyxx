/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009 Gregory Montoir
 */


struct GameMstUnk1 {
	int unk0x20;
	Task *t; // unk0xC4
};

#define SIZEOF_GameMstUnk1 256

struct GameMstUnk2 {
	void *unk0;
	LvlObject *o; // 4
	GameMstUnk1 *unk8;
	int unk0x10;
	int xPos; // 14
	int yPos; // 18
	int xMstPos; // 1C
	int yMstPos; // 20
};

#define SIZEOF_GameMstUnk2 64

struct Task {
	const uint8_t *codeData;
	Task *prev, *next; // 4,8
	uint8_t *dataPtr;
	GameMstUnk2 *unk10;
	int16_t localVars[16];
	uint8_t flags;
	uint8_t runningState;
	int16_t delay;
	uint32_t unk0x38; // mstFlags
	int (Game::*run)(Task *t);
	Task *child;
};







	GameMstUnk1 _gameMstUnkTable1[32];
	GameMstUnk2 _gameMstUnkTable2[64];

	int _mstPosXmin, _mstPosXmax, _mstPosYmin, _mstPosYmax;
	MovingOpcodeState _gameMstMovingState[8];
	int _gameMstMovingStateCount;
	uint8_t _gameMstUnk9;
	int _gameMstUnk27;
	int _gameMstUnk3;
	int _gameMstUnk4;
	int _gameMstUnk5;
	int _gameMstUnk6;
	// TODO: struct MstHdr
	int _resMstHeader0x08;
	int _resMstHeader0x0C;
	int _resMstHeader0x10;
	int _resMstHeader0x14;
	int _resMstHeader0x18;
	int _resMstHeader0x1C;
	int _resMstHeader0x20;
	int _resMstHeader0x24;
	int _resMstHeader0x28;
	int _resMstHeader0x2C;
	int _resMstHeader0x30;
	int _resMstHeader0x34;
	int _resMstHeader0x38;
	int _resMstHeader0x3C;
	int _resMstHeader0x40;
	int _resMstHeader0x44;
	int _resMstHeader0x48;
	int _resMstHeader0x4C;
	int _resMstHeader0x50;
	int _resMstHeader0x54;
	int _resMstHeader0x58;
	int _resMstHeader0x5C;
	int _resMstHeader0x60;
	int _resMstHeader0x64;
	int _resMstHeader0x68;
	int _resMstHeader0x6C;
	int _resMstHeader0x70;
	int _resMstHeader0x74;
	int _resMstHeader0x78;
	int _resMstHeader0x7C;
	int _resMstHeader0x80;
	uint8_t *_resMstReadBuffer;
	const uint8_t *_resMstPointsData;
	uint8_t *_resMstUnk34;
	uint8_t *_resMstUnk35;
	uint8_t *_resMstUnk36;
	MstScreenInitCode *_resMstUnk37; // screenInitCode
	uint8_t *_resMstCodeData_screenInit;
	uint8_t *_resMstUnk38; // MstScreenAreaCode[]
	uint8_t *_resMstUnk39;
	uint8_t *_resMstUnk40; // MstScreenAreaCode*[]
	uint8_t *_resMstUnk41;
	uint8_t *_resMstUnk42;
	uint8_t *_resMstUnk43;
	uint8_t *_resMstUnk44;
	uint8_t *_resMstUnk45;
	uint8_t *_resMstUnk46;
	uint8_t *_resMstUnk47;
	uint8_t *_resMstUnk48;
	uint8_t *_gameMstHeightMapData;
	uint8_t *_resMstUnk49;
	uint8_t *_resMstUnk50;
	uint8_t *_resMstUnk51;
	uint8_t *_resMstUnk52;
	uint8_t *_resMstUnk53;
	uint8_t *_resMstUnk63;
	uint8_t *_resMstUnk54;
	uint8_t *_resMstUnk55;
	uint8_t *_resMstUnk56;
	uint8_t *_resMstUnk57;
	uint8_t *_resMstUnk58;
	uint8_t *_resMstUnk59;
	uint8_t *_resMstUnk60;
	uint8_t *_resMstUnk61;
	uint8_t *_resMstOpcodeData;
	uint8_t *_resMstUnk62;
	Task _tasksTable[128];


	void clearGameMstUnk1(GameMstUnk1 *m);
	void clearGameMstUnk2(GameMstUnk2 *m);


void Game::clearGameMstUnk1(GameMstUnk1 *m) {
#if 0
	m->unk0 = 0; // le32
	_eax = m->unk10; // le32
	if (_eax) {
		_eax->unk30 = 0; // le32
	}
#endif
	for (int i = 0; i < 64; ++i) {
		if (_gameMstUnkTable2[i].unk0 && _gameMstUnkTable2[i].unk8 == m) {
			_gameMstUnkTable2[i].unk8 = 0;
		}
	}
}

void Game::clearGameMstUnk2(GameMstUnk2 *m) {
#if 0
	m->unk0 = 0; // le32
	_ecx = m->unk4; // le32
	if (_ecx) {
		_ecx->unk30 = 0; // le32
	}
#endif
}

void Game::game_unk43(Task *t) {
	GameMstUnk2 *m = t->unk10;
	LvlObject *o = m->o;
	m->xPos = o->xPos + o->posTable[7].x;
	m->yPos = o->yPos + o->posTable[7].y;
	m->xMstPos = m->xPos + (int32)READ_LE_UINT32(_resMstPointsData + o->data0x2E08 * 8 + 0);
	m->yMstPos = m->yPos + (int32)READ_LE_UINT32(_resMstPointsData + o->data0x2E08 * 8 + 4);
}

void Game::shuffleMstUnk43(MstUnk43 *p) {
	for (int i = 0; i < (int)p->count2; ++i) {
		_resMstUnk43[p->offset2 + i] &= 0x7F;
	}
	shuffleArray(_resMstUnk43 + p->offset2, p->count2);
}

void Game::loadLevelDataMst() {
	SectorFile f;
	char filename[32];
	sprintf(filename, "%s.MST", _resLevelNames[_currentLevel]);
	f.open(filename);

	uint8_t hdr[132];
	f.read(hdr, sizeof(hdr));

	int dataSize = READ_LE_UINT32(hdr + 4);
	_resMstHeader0x08 = READ_LE_UINT32(hdr + 0x08);
	_resMstHeader0x0C = READ_LE_UINT32(hdr + 0x0C);
	_resMstHeader0x10 = READ_LE_UINT32(hdr + 0x10);
	_resMstHeader0x14 = READ_LE_UINT32(hdr + 0x14);
	_resMstHeader0x18 = READ_LE_UINT32(hdr + 0x18);
	_resMstHeader0x1C = READ_LE_UINT32(hdr + 0x1C);
	_resMstHeader0x20 = READ_LE_UINT32(hdr + 0x20);
	_resMstHeader0x24 = READ_LE_UINT32(hdr + 0x24);
	_resMstHeader0x28 = READ_LE_UINT32(hdr + 0x28);
	_resMstHeader0x2C = READ_LE_UINT32(hdr + 0x2C);
	_resMstHeader0x30 = READ_LE_UINT32(hdr + 0x30);
	_resMstHeader0x34 = READ_LE_UINT32(hdr + 0x34);
	_resMstHeader0x38 = READ_LE_UINT32(hdr + 0x38);
	_resMstHeader0x3C = READ_LE_UINT32(hdr + 0x3C);
	_resMstHeader0x40 = READ_LE_UINT32(hdr + 0x40);
	_resMstHeader0x44 = READ_LE_UINT32(hdr + 0x44);
	_resMstHeader0x48 = READ_LE_UINT32(hdr + 0x48);
	_resMstHeader0x4C = READ_LE_UINT32(hdr + 0x4C);
	_resMstHeader0x50 = READ_LE_UINT32(hdr + 0x50);
	_resMstHeader0x54 = READ_LE_UINT32(hdr + 0x54);
	_resMstHeader0x58 = READ_LE_UINT32(hdr + 0x58);
	_resMstHeader0x5C = READ_LE_UINT32(hdr + 0x5C);
	_resMstHeader0x60 = READ_LE_UINT32(hdr + 0x60);
	_resMstHeader0x64 = READ_LE_UINT32(hdr + 0x64);
	_resMstHeader0x68 = READ_LE_UINT32(hdr + 0x68);
	_resMstHeader0x6C = READ_LE_UINT32(hdr + 0x6C);
	_resMstHeader0x70 = READ_LE_UINT32(hdr + 0x70);
	_resMstHeader0x74 = READ_LE_UINT32(hdr + 0x74);
	_resMstHeader0x78 = READ_LE_UINT32(hdr + 0x78);
	_resMstHeader0x7C = READ_LE_UINT32(hdr + 0x7C);
	_resMstHeader0x80 = READ_LE_UINT32(hdr + 0x80);

	_resMstReadBuffer = (uint8_t *)malloc(dataSize + 4);
	f.flush();
	f.read(_resMstReadBuffer, dataSize + 4);

	_resMstPointsData = _resMstReadBuffer;
	uint8_t *p = _resMstReadBuffer; // sizeof == _resMstHeader0x80 * 8
	_resMstUnk34 = p + _resMstHeader0x80 * 8;
	_resMstUnk35 = _resMstUnk34 + _resMstHeader0x08 * 20;
	uint8_t *_ecx = _resMstUnk35 + _resMstHeader0x0C * 16;
//	int _eax = 0;
//	_edi = _ecx;
	for (int i = 0; i < _resMstHeader0x0C; ++i) {
//		_edi[_eax] = _ecx; // le32
		int count = READ_LE_UINT32(_resMstUnk35 + i * 16 + 4);
//		printf("_resMstUnk35 i %d count %d\n", i, count);
		_ecx += count * 4;
//		for (int j = 0; j < count; ++j) { // le32
//		}
		count = READ_LE_UINT32(_resMstUnk35 + i * 16 + 12);
		if (count != 0) { // le32
//			_resMstUnk35[_eax + 8] = _ecx;
//			printf("_resMstUnk35 i %d count2 %d\n", i, count);
			_ecx += count;
			count &= 3;
			if (count != 0) {
				_ecx += 4 - count;
			}
		}
//		_eax += 16;
	}
	_resMstUnk36 = _ecx; // sizeof _resMstHeader0x10 * 12
	_ecx += _resMstHeader0x10 * 12;
	_resMstUnk37 = (MstScreenInitCode *)_ecx;
	_ecx += 8;
	_resMstCodeData_screenInit = _ecx;
	_ecx += _resMstHeader0x14 * 4;
	_resMstUnk38 = _ecx;
	_ecx += _resMstHeader0x18 * 36;
	_resMstUnk39 = _ecx;
	_ecx += _resMstHeader0x1C * 4;
	_resMstUnk40 = _ecx;
	_ecx += _resMstHeader0x80 * 4;
	_resMstUnk41 = _ecx;
	_ecx += _resMstHeader0x80 * 4;
	_resMstUnk42 = _ecx;
//	_eax = _ecx;
//	_edx = _eax;
//	_eax += _resMstHeader0x20 * 16; // p
	p = _ecx + _resMstHeader0x20 * 16;
	for (int i = 0; i < _resMstHeader0x20; ++i) {
		MstUnk42 *m = ((MstUnk42 *)_resMstUnk42) + i;
		m->offset1 = p - _resMstUnk42;
		m->count1 = FROM_LE32(m->count1);
		int count = m->count1;
		p += count * 4;
		m->count2 = FROM_LE32(m->count2);
		if (m->count2 != 0) {
			m->offset2 = p - _resMstUnk42;
			count = m->count2;
			p += count;
			count &= 3;
			if (count != 0) {
				p += 4 - count;
			}
		} else {
			m->offset2 = 0;
		}
	}
	_resMstUnk43 = p; // _edx
	p += _resMstHeader0x24 * 16;
	for (int i = 0; i < _resMstHeader0x24; ++i) {
		MstUnk43 *m = ((MstUnk43 *)_resMstUnk43) + i;
		m->offset1 = p - _resMstUnk43;
		m->count1 = FROM_LE32(m->count1);
		int count = m->count1;
		p += count * 4;
		m->count2 = FROM_LE32(m->count2);
		if (m->count2 != 0) {
			m->offset2 = p - _resMstUnk43;
			count = m->count2;
			p += count;
			count &= 3;
			if (count != 0) {
				p += 4 - count;
			}
		} else {
			m->offset2 = 0;
		}
	}
	_resMstUnk44 = p; //_eax;
	p += _resMstHeader0x28 * 16; // _eax
//	var14 = 0;
	for (int i = 0; i < _resMstHeader0x28; ++i) {
//loc_41625D:
//		_ebx = i * 16;
//		_resMstUnk44[_ebx] = _eax;
		int count = READ_LE_UINT32(_resMstUnk44 + i * 16 + 12); // _ecx = _resMstUnk44[_ebx + 12]; // le32
//		printf("_resMstUnk44 i %d count %d\n", i, count);
		p += count * 104; //var8 = _eax + _ecx * 104; // _edx
//		for (int k = 0; k < _ecx; ++k) { // _edi
//		}
//		_resMstUnk44[_ebx + 4] = var8; // _edx
		p += _resMstHeader0x80 * 4; // _eax = var8 + _resMstHeader0x80 * 4;
		//_edx = _resMstUnk44[_ebx + 12]; // le32
//		if (count > 0) { //_edx > 0) {
//			_edi = 0x60;
//			_esi = _ebx + 0xC;
//			varC = _edi;
		for (int var10 = 0; var10 < count; ++var10) {
//			do {
//loc_4162E4:
			for (int var8 = 0; var8 < 2; ++var8) {
//				var8 = 2;
//				do {
					//_ecx = _resMstUnk44[_ebx]; // le32
//					_ecx[var8 * 4] = _eax; //_ecx[_edi] = _eax;
					//_edx = _resMstUnk44[_esi];
//					_eax += _edx;
					p += count;
//					if ((_edx & 3) != 0) {
//						_edx = _resMstUnk44[_esi];
//						_eax += 4 - (_edx & 3);
//					}
					if ((count & 3) != 0) {
						p += 4 - (count & 3);
					}
//					_edi += 4;
//				} while (--var8 != 0);
//				++var10;
//				_edi += 0x68;
//				varC = _edi;
			}
//			} while (var10 < _resMstUnk44[_esi]);
		}
//loc_41633E:
//		var4 += 16;
//		++var14;
	}
	_resMstUnk45 = p;
	p += _resMstHeader0x2C * 12;

	_resMstUnk46 = p;
	p += _resMstHeader0x30 * 8;
	for (int i = 0; i < _resMstHeader0x30; ++i) {
		MstUnk46 *m = ((MstUnk46 *)_resMstUnk46) + i;
		m->offset = p - _resMstUnk46;
		const int count = m->count = FROM_LE32(m->count);
		p += count * 44;
	}

	_resMstUnk47 = p;
	p += _resMstHeader0x34 * 8;
	for (int i = 0; i < _resMstHeader0x34; ++i) {
		MstUnk47 *m = ((MstUnk47 *)_resMstUnk47) + i;
		m->offset = p - _resMstUnk47;
		const int count = m->count = FROM_LE32(m->count);
		p += count * 20;
	}

//	_eax = _edx;
	_resMstUnk48 = p; // _eax;
	p += _resMstHeader0x38 * 44; // _edx
	for (int i = 0; i < _resMstHeader0x38; ++i) { // var14
		MstUnk48 *m = ((MstUnk48 *)_resMstUnk48) + i;
		for (int j = 0; j < 2; ++j) { // _ebx
			int count = m->count[j] = FROM_LE32(m->count[j]);
			if (count != 0) {
				m->offsets1[j] = p - _resMstUnk48;
				p += count * 4;
				m->offsets2[j] = p - _resMstUnk48;
				p += count * 4;
			}
		}
		m->offsetUnk12 = p - _resMstUnk48; // _resMstUnk48[i * 44 + 12] = p; // _eax[var8 + 12] = _edx;
		uint8_t *q = p;
		int count = m->countUnk12 = FROM_LE32(m->countUnk12);
		p += count * 12;
		for (int j = 0; j < count; ++j) { // _esi
			MstUnk48Unk12 *m12 = ((MstUnk48Unk12 *)q) + j;
			m12->offset = p - _resMstUnk48;
			int count2 = m12->count = FROM_LE32(m12->count);
			assert(count2 == (int)READ_LE_UINT32(q + 8 + j * 12) );
			p += count2 * 28;
		}
	}

	_gameMstHeightMapData = p; //_edx;
	p += _resMstHeader0x3C * 948;

	// 0x1C * 0x20
//	_edi = _edx;
	_resMstUnk49 = p; // _edx;
	p += _resMstHeader0x40 * 24; // ecx = _edx + _resMstHeader0x40 * 24;
	for (int i = 0; i < _resMstHeader0x40; ++i) { // _ebx
//		_eax = _ebx * 0x18;
//		_edi[_eax + 4] = _ecx;
//		_esi = _resMstUnk49[_eax + 8];
		int count = READ_LE_UINT32(_resMstUnk49 + i * 24 + 8);
//		printf("_resMstUnk49 i %d count %d\n", i, count);
//		_edx = _esi * 16;
//		_ecx += _edx;
		p += count * 16;
//		_edx = 0;
		// sizeof=0x10, count=_eax[_edi+8]
		count = READ_LE_UINT32(_resMstUnk49 + i * 24 + 16); // _edx = _edi[_eax + 0x10];
		if (count != 0) { //_edx != 0) {
//			printf("_resMstUnk49 i %d count2 %d\n", i, count);
//			_edi[_eax + 0x0C] = _ecx;
//			_edx = _resMstUnk49[_eax + 0x10];
//			_ecx += _edx;
			p += count;
			if (count & 3) { //_edx & 3) {
				p += (4 - (count & 3)); // _ecx += 4 - (_edx & 3);
			}
		}
	}

	_resMstUnk50 = p;
	p += _resMstHeader0x44 * 8;
	for (int i = 0; i < _resMstHeader0x44; ++i) {
		MstUnk50 *m = ((MstUnk50 *)_resMstUnk50) + i;
		m->offset = p - _resMstUnk50;
		const int count = m->count = FROM_LE32(m->count);
		p += count * 40;
	}

	_resMstUnk51 = p;
	p += _resMstHeader0x48 * 12;
	for (int i = 0; i < _resMstHeader0x48; ++i) {
		MstUnk51 *m = ((MstUnk51 *)_resMstUnk51) + i;
		m->offset = p - _resMstUnk51;
		int count = m->count = FROM_LE32(m->count);
		assert(count == (int)READ_LE_UINT32(_resMstUnk51 + i * 12 + 8) );
		p += count * 36;
	}
	_resMstUnk52 = p; //_ebp;
	p += _resMstHeader0x4C * 4; //_ecx = _ebp + _resMstHeader0x4C * 4;
	_resMstUnk53 = p; //_ecx;
	p += _resMstHeader0x50 * 20; // sizeof==0x14
	_resMstUnk63 = p; //_edx;
	p += _resMstHeader0x54 * 8;
	_resMstUnk54 = p; //_edx;
	p += _resMstHeader0x58 * 8;
	_resMstUnk55 = p; //_edx;
	p += _resMstHeader0x5C * 8;
	_resMstUnk56 = p; //_edx;
	p += _resMstHeader0x60 * 12;
	_resMstUnk57 = p;
	p += _resMstHeader0x64 * 16;
	_resMstUnk58 = p;
	p += _resMstHeader0x68 * 16;
	_resMstUnk59 = p;
	p += _resMstHeader0x6C * 8;
	_resMstUnk60 = p;
	p += _resMstHeader0x70 * 4;
	_resMstUnk61 = p; // unused
	p += _resMstHeader0x74 * 4;
	_resMstOpcodeData = _ecx;
	p += _resMstHeader0x78 * 16;
	_resMstUnk62 = p;
	assert(_resMstHeader0x7C > 0);
//	_edx = _ecx;
//	_eax = _resMstHeader0x7C;
//	--_eax;
//	do {
//		_dl = *_ecx;
//		_ecx += 4;
//		var4 = _edx;
//	} while (--_eax != 0);
//	_ecx = *_esi;
//	_eax = _esi - _resMstReadBuffer;
//	assert(_eax == _ecx);
	const uint8_t *end = _resMstUnk62 + _resMstHeader0x7C * 4;
	assert(READ_LE_UINT32(end) == (uint32)(end - _resMstReadBuffer));

#if 0
	for (var14 = 0; var14 < _resMstHeader0x0C; ++var14) {
		_esi = var14 * 16;
		_eax = READ_LE_UINT32(_resMstUnk35 + _esi + 4);
		for (_edx = 0; _edx < _eax; ++_edx) {
			_eax = _resMstUnk35[_esi];
			_ecx = READ_LE_UINT32(_eax + _edx * 4);
			if (_ecx == 0xFFFFFFFF) {
				_eax[_edx * 4] = 0;
			} else {
				_eax[_edx * 4] = _resMstUnk62 + _ecx * 4;
			}
		}
	}
#endif
	for (int i = 0; i < _resMstHeader0x6C; ++i) {
		MstUnk59 *m = ((MstUnk59 *)_resMstUnk59) + i;
		m->unk0 = FROM_LE32(m->unk0);
		m->codeIndex = FROM_LE32(m->codeIndex); // _resMstUnk62
	}
#if 0
	for (int i = 0; i < _resMstHeader0x70; ++i) {
		uint32_t codeIndex = READ_LE_UINT32(_resMstUnk60 + i * 4);
		_resMstUnk60[i] = (codeIndex == 0xFFFFFFFF) ? 0 : _resMstUnk62 + codeIndex * 4;
	}
	for (int i = 0; i < _resMstHeader0x10; ++i) {
		_eax = READ_LE_UINT32(_resMstUnk36 + i * 12);
		_resMstUnk36[i * 12] = _resMstUnk49 + _eax * 24;
	}
	_eax = READ_LE_UINT32(_resMstUnk37 + 4);
	_resMstUnk37[4] = (_eax == 0xFFFFFFFF) ? 0 : _resMstUnk62 + _eax * 4;
	for (int i = 0; i < _resMstHeader0x14; ++i) {
		int index = READ_LE_UINT32(_resMstCodeData_screenInit + i * 4);
		_resMstCodeData_screenInit[i * 4] = (index == 0xFFFFFFFF) ? 0 : _resMstUnk62 + index * 4;
	}
	for (int i = 0; i < _resMstHeader0x18; ++i) {
		int index = READ_LE_UINT32(_resMstUnk38 + i * 36 + 0x10);
		_resMstUnk38[i * 36 + 0x10] = (index == 0xFFFFFFFF) ? 0 : _resMstUnk38 + index * 36; // next
		index = READ_LE_UINT32(_resMstUnk38 + i * 36 + 0x14);
		_resMstUnk38[i * 36 + 0x14] = (index == 0xFFFFFFFF) ? 0 : _resMstUnk38 + index * 36; // prev
		index = READ_LE_UINT32(_resMstUnk38 + i * 36 + 0x18);
		_resMstUnk38[i * 36 + 0x18] = (index == 0xFFFFFFFF) ? 0 : _resMstUnk38 + index * 36;
		_resMstUnk38[i * 36 + 0x20] = _resMstUnk62 + _resMstUnk38[i * 36 + 0x20] * 4;
	}
	for (int i = 0; i < _resMstHeader0x1C; ++i) {
		int index = READ_LE_UINT32(_resMstUnk39 + i * 4);
		_resMstUnk39[i * 4] = (index == 0xFFFFFFFF) ? 0 : _resMstUnk38 + index * 36;
	}
	for (int i = 0; i < _resMstHeader0x80; ++i) {
		int index = READ_LE_UINT32(_resMstUnk40 + i * 4);
		_resMstUnk40[i * 4] = (index == 0xFFFFFFFF) ? 0 : _resMstUnk38 + index * 36;
	}
	for (int i = 0; i < _resMstHeader0x80; ++i) {
		int index = READ_LE_UINT32(_resMstUnk41 + i * 4);
		_resMstUnk41[i * 4] = (index == 0xFFFFFFFF) ? 0 : _resMstUnk38 + index * 36;
	}
	for (int i = 0; i < _resMstHeader0x20; ++i) {
		int count = READ_LE_UINT32(_resMstUnk42 + i * 16 + 4);
		for (int j = 0; j < count; ++j) {
			uint8_t *ptr = READ_LE_UINT32(resMstUnk42 + i * 16);
			ptr[i] = _resMstUnk46 + ptr[i] * 8; // le32
		}
	}
#endif
	for (int i = 0; i < _resMstHeader0x24; ++i) {
		MstUnk43 *m = ((MstUnk43 *)_resMstUnk43) + i;
		for (int j = 0; j < (int)m->count1; ++j) {
//			uint8_t *ptr = _resMstUnk43 + m->offset1;
//			int index = READ_LE_UINT32(ptr + j * 4);
//			printf("MstUnk43 index %d (%d/%d) MstUnk48 index %d (offset %d)\n", i, j, m->count1, index, m->offset1);
//			WRITE_LE_UINT32(ptr + j * 4, _resMstUnk48 + ptr[i] * 44);
		}
	}
#if 0
	for (int i = 0; i < _resMstHeader0x28; ++i) {
		// _edx == _resMstUnk44
		// _ecx == i * 16
//loc_416BDC:
		int count = READ_LE_UINT32(_resMstUnk44 + i * 16 + 0xC);
		for (int j = 0; count--; j += 104) {
			uint8_t *ptr = READ_LE_UINT32(_resMstUnk44 + i * 16);
			int index = READ_LE_UINT32(ptr + j + 16);
			ptr[j + 16] = (index == 0xFFFFFFFF) ? 0 : _resMstUnk34 + index * 20;
			index = READ_LE_UINT32(ptr + j + 20);
			ptr[j + 20] = (index == 0xFFFFFFFF) ? 0 : _resMstUnk35 + index * 16;
			index = READ_LE_UINT32(ptr + j + 24);
			ptr[j + 24] = (index == 0xFFFFFFFF) ? 0 : _resMstUnk35 + index * 16;
			index = READ_LE_UINT32(ptr + j + 28);
			ptr[j + 28] = (index == 0xFFFFFFFF) ? 0 : _resMstUnk36 + index * 12;
			index = READ_LE_UINT32(ptr + j + 32);
			ptr[j + 32] = (index == 0xFFFFFFFF) ? 0 : _resMstUnk36 + index * 12;
			index = READ_LE_UINT32(ptr + j + 36);
			ptr[j + 36] = (index == 0xFFFFFFFF) ? 0 : _resMstUnk35 + index * 16;
			index = READ_LE_UINT32(ptr + j + 40);
			ptr[j + 40] = (index == 0xFFFFFFFF) ? 0 : _resMstUnk35 + index * 16;
			index = READ_LE_UINT32(ptr + j + 92);
			ptr[j + 92] = (index == 0xFFFFFFFF) ? 0 : _resMstUnk44 + index * 104;
			index = READ_LE_UINT32(ptr + j + 76);
			ptr[j + 76] = (index == 0xFFFFFFFF) ? 0 : _resMstUnk44 + index * 104;
			index = READ_LE_UINT32(ptr + j + 80);
			ptr[j + 80] = (index == 0xFFFFFFFF) ? 0 : _resMstUnk44 + index * 104;
			index = READ_LE_UINT32(ptr + j + 84);
			ptr[j + 84] = (index == 0xFFFFFFFF) ? 0 : _resMstUnk44 + index * 104;
			index = READ_LE_UINT32(ptr + j + 88);
			ptr[j + 88] = (index == 0xFFFFFFFF) ? 0 : _resMstUnk44 + index * 104;
		}
//loc_416DFA:
		for (int j = 0; j < _resMstHeader0x80; ++j) {
			const uint8_t *ptr = READ_LE_UINT32(_resMstUnk44 + i * 16 + 4);
			ptr[j] = (ptr[j] == 0xFFFFFFFF) ? 0 : READ_LE_UINT32(_resMstUnk44 + i * 16) + ptr[j] * 104;
		}
	}
	for (int i = 0; i < _resMstHeader0x2C; ++i) {
		_resMstUnk45[i * 12 + 4] = _resMstUnk62 + _resMstUnk45[i * 12 + 4] * 4;
		_ecx = _resMstUnk45[i * 12 + 8];
		_resMstUnk45[i * 12 + 8] = _ecx == -1 ? 0 : _resMstUnk62 + _resMstUnk45[i * 12 + 8] * 4;
	}
	for (int _esi = 0; _esi < _resMstHeader0x30; ++_esi) {
		int count = _resMstUnk46[i * 8 + 4]; // le32
		for (int _ebx = 0; _ebx < count; ++_ebx) {
loc_416EC6:
			_edx = _ebx * 44;
			_eax = _resMstUnk46[i * 8]; // le32
			_ecx = _eax[_edx]; // le32
			_eax[_edx] = _gameMstHeightMapData + _ecx * 948;

			_ecx = _resMstUnk46[_esi * 8];
			_eax = _ecx[_edx + 0x1C];
			_ecx[_edx + 0x1C] = (_eax == -1) ? 0 : _resMstUnk51 + _eax * 12;

			_eax = _resMstUnk46[_esi * 8];
			_ecx = _eax[_edx + 0x20];
			_eax[_edx + 0x20] = _resMstUnk44 + _ecx * 16;

			_ecx = _resMstUnk46[_esi * 8];
			_ecx[_edx + 0x24] = (_ecx[_edx + 0x24] == -1) ? 0 : _resMstUnk47 + _ecx[_edx + 0x24] * 8;

			_ecx = _resMstUnk46[_esi * 8];
			_ecx[_edx + 0x28] = (_ecx[_edx + 0x28] == -1) ? 0 : _resMstUnk62 + _ecx[_edx + 0x28] * 4;
		}
	}

	for (int _esi = 0; _esi < _resMstHeader0x34; ++_esi) {
		_edi = _resMstUnk47[_esi * 8 + 4]; // le32
		for (int _edx = 0; _edx < _edi; ++_edx) {
			_eax = _resMstUnk47[_esi * 8];
			_ecx[_edx * 0x14 + 0x10] = _resMstUnk62 + _ecx[_edx * 0x14 + 0x10] * 4;
		}
	}

	var14 = 0;
	_edx = 12;
	_ecx = _resMstUnk48;
	for (int _eax = 0; _eax < _resMstHeader0x38; ++_eax) { // var14
		_ebp = 0;
		_eax = _ecx[_edx - 4]; // _resMstUnk48[8];
		_ebx = _edx - 12; // 0
		if (_eax == 0xFFFFFFFF) {
			_ecx[_ebx + 8] = 0;
		} else {
			_ecx[_ebx + 8] = _resMstUnk62 + _eax * 4;
		}
		for (var8 = 0; var8 < _resMstUnk48[_ebx + 16]; ++var8) { // countUnk12
			_eax = 0;
			_esi = _ecx[_edx];
			var4 = 0;
//			_edi = _esi[_eax + 8];
			for (var4 = 0; var4 < _esi[_eax + 8]; ++var4) { // while (var4 < _ebp[_eax + 8])
				_ecx = _ecx[_edx];
				_ecx = _ecx[_eax + 4];
				_edi = _ecx + _esi + 4;
				_ecx = _ecx[_esi + 4];
				if (_ecx == 0xFFFFFFFF) {
					_edi[0] = 0;
				} else {
					_edi[0] = _resMstUnk51 + _ecx * 12;
				}
				_ebx = _resMstUnk62;
				_ecx = _resMstUnk48[_edx];
				_ecx = _ecx[_eax + 4];
				_edi = _ecx[_esi + 16];
				_ecx = _ecx + _esi + 16;
				_edi = _ebx + _edi * 4;
				_ebx = _edx - 12;
				_ecx[0] = _edi;
				_ecx = _resMstUnk48[_edx];
				_ecx = _ecx[_eax + 4];
				_edi = _ecx[_esi + 20];
				_ecx = _ecx + _esi + 20;
				if (_edi == 0xFFFFFFFF) {
					_ecx[0] = 0;
				} else {
					_ecx[0] = _resMstUnk62 + _edi * 4;
				}
				_ecx = _resMstUnk48;
				++var4;
				_esi += 28;
				_ebp = _resMstUnk48[_edx];
			}
			++var8;
			_eax += 12;
			_edi = _ecx[_ebx + 16];
		}
		++var14;
		_edx += 44;
	}
	for (_ebp = 0; _ebp < _resMstHeader0x3C; ++_ebp) {
		for (_edi = 0; _edi < 32; ++_edi) {
			_eax = _ebp * 948 + _edi * 28;
			_edx = READ_LE_UINT32(_gameMstHeightMapData + _eax + 0x14);
			if (_edx == 0xFFFFFFFF) {
				_gameMstHeightMapData[_eax + 0x14] = 0;
			} else {
				_gameMstHeightMapData[_eax + 0x14] = _gameMstHeightMapData + _edx * 948;
			}
			_edx = READ_LE_UINT32(_gameMstHeightMapData + _eax + 0x18);
			if (_edx == 0xFFFFFFFF) {
				_gameMstHeightMapData[_eax + 0x18] = 0;
			} else {
				_gameMstHeightMapData[_eax + 0x18] = _resMstUnk62 + _edx * 4;
			}
		}
	}
	for (_edx = 0; _edx < _resMstHeader0x40; ++_edx) {
		_eax = _edx * 24;
		_esi = READ_LE_UINT32(_resMstUnk49 + _eax);
		_resMstUnk49[_eax] = _gameMstHeightMapData + _esi * 948;
		for (_edi = 0; _edi < READ_LE_UINT32(_resMstUnk49 + _eax + 8); ++_edi) {
			_ebx = i * 16;
			_ecx = READ_LE_UINT32(_resMstUnk49 + _eax + 4) + _ebx;
			_edx = _gameMstHeightMapData + READ_LE_UINT32(_ecx + 4) * 24 + _esi;
		}
	}
	for (_esi = 0; _esi < _resMstHeader0x44; ++_esi) {
		for (_edx = 0; _edx < READ_LE_UINT32(_resMstUnk50 + _esi * 8 + 4); ++_edx) {
			_eax = READ_LE_UINT32(_resMstUnk50 + _esi * 8) + _edx * 40;
			_edi = *_eax;
			*_eax = _resMstUnk62 + _edi * 4;
		}
	}
	for (var14 = 0; var14 < _resMstHeader0x48; ++var14) {
		_eax = _resMstUnk51 + var14 * 12;
		for (_edi = 0; _edi < 36; _edi += 4) {
			for (_esi = 0; _esi < READ_LE_UINT32(_eax + _edx + 8); ++_esi) {
				_ecx = READ_LE_UINT32(_eax + _edx + 4);
				_ebp = READ_LE_UINT32(_resMstUnk50 + _edi + _ebx * 8);
				_eax = _ecx + _esi * 36;
				_ecx = READ_LE_UINT32(_eax);
				_ecx = _ebp + _ecx * 40;
				*_eax = _ecx;
			}
		}
	}
	_eax = 0;
	for (int i = 0; i < _resMstHeader0x30; ++i) {
		const int count = _resMstUnk46[_eax + 4];
		_edi = 0;
		for (int j = 0; j < count; ++j) {
			_edx = *(_resMstUnk46[_eax] + _edi + 0x1C);
			if (_edx != 0) {
				_ecx = _resMstUnk46[_eax];
				_edx = _edx[8];
				_esi = _ecx[_edi + 0x14];
				if (_esi < _edx) {
					_edx = _esi;
				}
				_ecx[_edi + 0x14] = _edx;
			}
			_edi += 0x2C;
		}
		_eax += 8;
	}
#endif
}

MstUnk48 *Game::getMstUnk48FromMstUnk43(MstUnk43 *m, int index) {
	assert(index < (int)m->count1);
	index = READ_LE_UINT32(_resMstUnk43 + m->offset1 + index * 4);
	return ((MstUnk48 *)_resMstUnk48) + index;
}

void Game::game_unk116(Task *t, int y1, int y2, int x1, int x2, int argC, int arg10, int o_flags1, int o_flags2, int arg1C, int arg20, int arg24) {
	// TODO
#if 0
.text:004153D1                 mov     eax, [esp+4+o_flags2]
.text:004153D8                 mov     ebx, ecx
.text:004153DA                 xor     esi, esi
.text:004153DD                 cmp     ax, 0FFFFh
.text:004153E1                 mov     edi, edx
.text:004153E3                 mov     [esp+14h+task], ebx
.text:004153E7                 mov     [esp+14h+o_flags2], eax
#endif
	if (o_flags2 == 0xFFFF) {
#if 0
		_eax = 0;
		if (t->dataPtr) {
			_eax = *(uint32_t *)(t->dataPtr + 16);
		} else if (t->unk10) {
			_eax = *(uint32_t *)(t->unk10 + 4);
		}
		if (_eax != 0) {
			o_flags2 = *(uint16_t *)(_eax + 0x14);
		} else {
			o_flags2 = 0x3001;
		}
#endif
	}
	if (y1 != y2) {
		y1 += _rnd.update() % ABS(y2 - y1 + 1);
	}
	if (x1 != x2) {
		x1 += _rnd.update() % ABS(x2 - x1 + 1);
	}
	if (argC < 0) {
		x2 = _gameResData0x2E08;
	} else {
		x2 = argC;
	}
#if 0
	if (arg1C != -128) { // _ecx
		if (_globalVars[30] > 32) {
			_globalVars[30] = 32;
		}
		int count = 0;
		for (int i = 0; i < 32; ++i) {
			GameMstUnk1 *gms = &_gameMstUnkTable1[i];
			if (gms->unk0 != 0) { // le32
				++count;
			}
		}
		if (count >= _globalVars[30]) {
			return;
		}
		_ebp = arg24;
		if (arg1C < 0) {
			MstUnk42 *m = ((MstUnk42 *)_resMstUnk42) + arg24;
//.text:004154E2                 lea     esi, [eax+ecx]
//			_esi = _resMstUnk42 + m->offset1;
			if (m->count2 == 0) {
//				_ecx = [esi+8]
//				_edx = 0;
//				_al = [ecx+edx]
//				_edx = _eax;
			} else {
loc_4154F7:
//				lea esi, [eax+ecx]
//				_edx = random_update() % _esi->unkC;
//				_al = [ecx+edx]
//				_edx = _eax;
			}
//			goto loc_415518;
		} else {
loc_415510:
			_ecx &= 255;
			_edx = _ecx;
		}
loc_415518:
		int _esi = 0;
		for (int i = 0; i < 32; ++i) {
			GameMonsterState *gms = &_gameMstUnkTable1[i];
			if (gms->unk0 == 0) { // le32
				++_esi;
				goto loc_415539;
			}
		}
		return;
loc_415539:
		_esi = _gameMstUnkTable1 + _esi * 256;

.text:0041554A                 mov     ecx, 8
.text:0041554F                 xor     eax, eax
.text:00415551                 lea     edi, [esi+28h]
.text:00415554                 rep stosd
.text:00415556                 mov     ecx, _resMstUnk42
.text:0041555C                 mov     byte ptr [esi+48h], 1Ch
.text:00415560                 shl     ebp, 4
.text:00415563                 mov     [esi+0A5h], al
.text:00415569                 mov     dword ptr [esi+0ECh], 0FFFFFFFFh
.text:00415573                 mov     [esi+0D0h], eax
.text:00415579                 mov     [esi+0A6h], al
.text:0041557F                 mov     eax, [ecx+ebp]
.text:00415582                 mov     ecx, [eax+edx*4]
.text:00415585                 mov     eax, [esp+14h+arg_20]
.text:00415589                 mov     [esi], ecx
.text:0041558B                 mov     edi, [ecx]
.text:0041558D                 lea     edx, [eax+eax*4]
.text:00415590                 lea     eax, [eax+edx*2]
.text:00415593                 shl     eax, 2
.text:00415596                 mov     edx, eax
.text:00415598                 add     edx, edi
.text:0041559A                 mov     [esi+4], edx
.text:0041559D                 mov     edx, [ecx]
.text:0041559F                 mov     eax, [edx+eax]
.text:004155A2                 mov     [esi+8], eax
.text:004155A5                 mov     eax, [esi+4]
.text:004155A8                 mov     ecx, [eax+0Ch]
.text:004155AB                 mov     [esi+44h], ecx
.text:004155AE                 mov     ecx, [eax+1Ch]
.text:004155B1                 test    ecx, ecx
.text:004155B3                 jnz     short loc_4155B9
.text:004155B5                 and     byte ptr [esi+48h], not 4
.text:004155B9
.text:004155B9 loc_4155B9:                             ; CODE XREF: game_unk116+1E3j
.text:004155B9                 mov     edx, [esp+14h+o_flags2]
.text:004155BD                 mov     ecx, [esi+4]
.text:004155C0                 mov     eax, [esi+8]
.text:004155C3                 mov     ebp, [esp+14h+o_flags1]
.text:004155C7                 mov     ebx, [esp+14h+x2]
.text:004155CB                 push    0               ; directionKeyMask
.text:004155CD                 push    0               ; actionKeyMask
.text:004155CF                 push    edx
.text:004155D0                 mov     dx, [ecx+4]     ; o_anim
.text:004155D4                 mov     cl, [eax+3B0h]  ; num
.text:004155DA                 push    ebp
.text:004155DB                 push    edx
.text:004155DC                 mov     edx, [esp+28h+x1]
.text:004155E0                 push    ecx
.text:004155E1                 mov     cl, [eax+3B1h]  ; type
.text:004155E7                 push    ebx
.text:004155E8                 push    edx
.text:004155E9                 mov     edx, [esp+34h+y2]
.text:004155ED                 call    game_unk115
.text:004155F2                 mov     edi, eax
.text:004155F4                 test    edi, edi
.text:004155F6                 jnz     short loc_41562C
.text:004155F8                 mov     [esi], eax
.text:004155FA                 mov     eax, [esi+10h]
.text:004155FD                 test    eax, eax
.text:004155FF                 jz      short loc_415604
.text:00415601                 mov     [eax+30h], edi
.text:00415604
.text:00415604 loc_415604:                             ; CODE XREF: game_unk116+22Fj
.text:00415604                 mov     eax, (offset _gameMstUnkTable2_sizeof64+8)
.text:00415609
.text:00415609 loc_415609:                             ; CODE XREF: game_unk116+252j
.text:00415609                 mov     ecx, [eax-8]
.text:0041560C                 test    ecx, ecx
.text:0041560E                 jz      short loc_41561A
.text:00415610                 cmp     [eax], esi
.text:00415612                 jnz     short loc_41561A
.text:00415614                 mov     dword ptr [eax], 0
.text:0041561A
.text:0041561A loc_41561A:                             ; CODE XREF: game_unk116+23Ej
.text:0041561A                                         ; game_unk116+242j
.text:0041561A                 add     eax, 64
.text:0041561D                 cmp     eax, offset dword_468168
.text:00415622                 jl      short loc_415609
.text:00415629                 retn    28h
.text:0041562C
.text:0041562C loc_41562C:                             ; CODE XREF: game_unk116+226j
.text:00415631                 mov     [esi+10h], edi
.text:00415634                 cmp     _currentLevel, 7
.text:00415636                 jnz     loc_4156FC
.text:0041563C                 mov     ecx, [esi+8]
.text:0041563F                 cmp     byte ptr [ecx+3B0h], 1Ah
.text:00415646                 jnz     loc_4156FC
.text:0041564C                 mov     edx, [esi+4]
.text:0041564F                 mov     eax, ecx
.text:00415651                 push    0
.text:00415653                 push    0
.text:00415655                 mov     cx, [edx+4]
.text:00415659                 mov     dl, [eax+3B0h]
.text:0041565F                 push    3001h
.text:00415664                 inc     cx
.text:00415666                 push    ebp
.text:00415667                 push    ecx
.text:00415668                 mov     ecx, [esp+28h+x1]
.text:0041566C                 push    edx
.text:0041566D                 mov     edx, [esp+2Ch+y2]
.text:00415671                 push    ebx
.text:00415672                 push    ecx
.text:00415673                 mov     cl, [eax+3B1h]
.text:00415679                 call    game_unk115
.text:0041567E                 test    eax, eax
.text:00415680                 mov     [esi+14h], eax
.text:00415683                 jnz     short loc_4156BF
.text:00415685                 mov     [esi], eax
.text:00415687                 mov     eax, [esi+10h]
.text:0041568A                 test    eax, eax
.text:0041568C                 jz      short loc_415697
.text:0041568E                 mov     edx, eax
.text:00415690                 mov     dword ptr [edx+30h], 0
.text:00415697
.text:00415697 loc_415697:                             ; CODE XREF: game_unk116+2BCj
.text:00415697                 mov     eax, (offset _gameMstUnkTable2_sizeof64+8)
.text:0041569C
.text:0041569C loc_41569C:                             ; CODE XREF: game_unk116+2E5j
.text:0041569C                 mov     ecx, [eax-8]
.text:0041569F                 test    ecx, ecx
.text:004156A1                 jz      short loc_4156AD
.text:004156A3                 cmp     [eax], esi
.text:004156A5                 jnz     short loc_4156AD
.text:004156A7                 mov     dword ptr [eax], 0
.text:004156AD
.text:004156AD loc_4156AD:                             ; CODE XREF: game_unk116+2D1j
.text:004156AD                                         ; game_unk116+2D5j
.text:004156AD                 add     eax, 64
.text:004156B0                 cmp     eax, offset dword_468168
.text:004156B5                 jl      short loc_41569C
.text:004156BC                 retn    28h
.text:004156BF
.text:004156BF loc_4156BF:                             ; CODE XREF: game_unk116+2B3j
.text:004156BF                 mov     eax, [esp+14h+arg_C]
.text:004156C3                 test    eax, eax
.text:004156C5                 jge     short loc_4156E2
.text:004156C7                 mov     eax, [esi+14h]
.text:004156CA                 mov     ecx, _gameMstScreenRefPosX
.text:004156D0                 mov     edx, [eax]
.text:004156D2                 add     edx, ecx
.text:004156D4                 mov     [eax], edx
.text:004156D6                 mov     eax, [esi+14h]
.text:004156D9                 mov     edx, _gameMstScreenRefPosY
.text:004156DF                 add     [eax+4], edx
.text:004156E2
.text:004156E2 loc_4156E2:                             ; CODE XREF: game_unk116+2F5j
.text:004156E2                 mov     eax, [esi+14h]
.text:004156E5                 push    6
.text:004156E7                 mov     dl, 6
.text:004156E9                 mov     dword ptr [eax+30h], 0
.text:004156F0                 mov     ecx, [esi+14h]
.text:004156F3                 push    ecx
.text:004156F4                 mov     ecx, [esi+10h]
.text:004156F7                 call    setLvlObjectPosRelativeToObject
.text:004156FC
.text:004156FC loc_4156FC:                             ; CODE XREF: game_unk116+266j
.text:004156FC                                         ; game_unk116+276j
.text:004156FC                 mov     eax, [esp+14h+o_flags2]
.text:00415700                 and     eax, 0FFFFh
.text:00415705                 mov     [esi+0E8h], eax
.text:0041570B                 and     eax, 1Fh
.text:0041570E                 xor     ebp, ebp
.text:00415710                 mov     dl, _game_unkLookupTable5[eax]
.text:00415716                 mov     [esi+0E6h], dl
.text:0041571C                 mov     [edi+30h], esi
.text:0041571F                 jmp     loc_4157E8

	} else {
loc_415724:
//		int index = 0; // _ebp
		for (int i = 0; i < 64; ++i) {
			if (_gameMstUnkTable2[i].unk0 == 0) {
				_ebp = &_gameMstUnkTable2[i];
				break;
			}
		}
		if (!_ebp) {
			return;
		}
loc_415743:
		_ebp = &_gameMstUnkTable2[index];

.text:00415754                 mov     eax, [esp+14h+arg_24]
.text:00415758                 mov     ecx, _resMstUnk45
.text:0041575E                 lea     eax, [eax+eax*2]
.text:00415761                 lea     eax, [ecx+eax*4]
.text:00415764                 mov     [ebp+0], eax
.text:00415767                 mov     eax, [ebx+0Ch]
.text:0041576A                 test    eax, eax
.text:0041576C                 jz      short loc_415773
.text:0041576E                 mov     [ebp+8], eax
.text:00415771                 jmp     short loc_415789
.text:00415773
.text:00415773 loc_415773:                             ; CODE XREF: game_unk116+39Cj
.text:00415773                 mov     eax, [ebx+10h]
.text:00415776                 test    eax, eax
.text:00415778                 jz      short loc_415782
.text:0041577A                 mov     ecx, [eax+8]
.text:0041577D                 mov     [ebp+8], ecx
.text:00415780                 jmp     short loc_415789
.text:00415782
.text:00415782 loc_415782:                             ; CODE XREF: game_unk116+3A8j
.text:00415782                 mov     dword ptr [ebp+8], 0
.text:00415789
.text:00415789 loc_415789:                             ; CODE XREF: game_unk116+3A1j
.text:00415789                                         ; game_unk116+3B0j
.text:00415789                 mov     eax, [ebp+0]
.text:0041578C                 mov     edi, [esp+14h+o_flags2]
.text:00415790                 push    0
.text:00415792                 xor     ecx, ecx
.text:00415794                 push    0
.text:00415796                 mov     byte ptr [ebp+24h], 0
.text:0041579A                 mov     cl, [eax]
.text:0041579C                 mov     ax, [eax+2]
.text:004157A0                 push    edi
.text:004157A1                 mov     edi, [esp+20h+o_flags1]
.text:004157A5                 push    edi
.text:004157A6                 push    eax
.text:004157A7                 mov     al, cl
.text:004157A9                 and     al, 7Fh
.text:004157AB                 push    eax
.text:004157AC                 mov     eax, [esp+2Ch+x2]
.text:004157B0                 push    eax
.text:004157B1                 push    edx
.text:004157B2                 mov     edx, [esp+34h+y2]
.text:004157B6                 shr     ecx, 7
.text:004157B9                 and     cl, 1
.text:004157BC                 call    game_unk115
.text:004157C1                 mov     edi, eax
.text:004157C3                 test    edi, edi
.text:004157C5                 jnz     short loc_4157E2
.text:004157C7                 mov     [ebp+0], eax
.text:004157CA                 mov     eax, [ebp+4]
.text:004157CD                 test    eax, eax
.text:004157CF                 jz      return
.text:004157D5                 mov     ecx, eax
.text:004157D7                 mov     [ecx+30h], edi
.text:004157DF                 retn    28h
.text:004157E2
.text:004157E2 loc_4157E2:                             ; CODE XREF: game_unk116+3F5j
.text:004157E2                 mov     [ebp+4], edi
.text:004157E5                 mov     [edi+30h], ebp
.text:004157E8

	}
#endif

#if 0
	if (argC < 0) {
		_edi->xPos += _gameMstScreenRefPosX;
		_edi->yPos += _gameMstScreenRefPosY;
	}
	setLvlObjectPosInScreenGrid(_edi, 7);
	if (_ebp) {
		for (int i = 0; i < 128; ++i) {
			if (_tasksTable[i].codeData == 0) {
				goto loc_41584E;
			}
		}
		_ebp->unk0 = 0; // le32
		_eax = _ebp->unk4; // le32
		if (_eax != 0) {
			_eax->unk30 = 0; // le32
		}
		removeLvlObjectNotType2List1(_edi);
		return;

loc_41584E:
		_esi = &_tasksTable[i];
		memset(_esi, 0, sizeof(Task));
		resetTask(_esi, 0x4D2);
		_esi->prev = 0;
		_esi->next = _gameMstResToLoad1Pri;
		if (_gameMstResToLoad1Pri) { // _eax
			_gameMstResToLoad1Pri->prev = _esi;
		}
		_edi = _currentTask;
		_esi->unk10 = _ebp;
		_esi->dataPtr = 0;
		_currentTask = _gameMstResToLoad1Pri = _esi;
.text:004158A0                 mov     [ebp+3Ch], esi
		_eax = _esi->child;
		if (_eax) {
			_eax->codeData = 0;
			_esi->child = 0;
		}
		_esi->codeData = 0;
		_eax = _esi->next;
		_ecx = _esi->prev;
		if (_eax) {
			_eax->prev = _ecx;
		}
		if (_ecx) {
			_ecx->next = _eax;
		} else {
			_gameMstResToLoad1Pri = _eax;
		}
		if (!_gameMstResToLoad1Pri) {
			_gameMstResToLoad1Pri = _esi;
			_esi->next = _esi->prev = 0;
		} else {
			_eax = _gameMstResToLoad1Pri;
			_ecx = _gameMstResToLoad1Pri->next;
			while (_ecx) {
				_eax = _ecx;
				_ecx = _eax->next;
			}
			_eax->next = _esi;
			_esi->next = 0;
			_esi->prev = _eax;
		}
		_esi->codeData = 1234;
		game_unk43(_esi);
.text:0041590A                 mov     ecx, [ebp+0]
.text:0041590D                 mov     edx, [ecx+4]
		resetTask(_esi, _edx);
.text:00415917                 cmp     _currentLevel, 1
.text:0041591E                 jnz     loc_415ADE
.text:00415924                 mov     edx, [ebp+0]
.text:00415927                 cmp     byte ptr [edx], 1Bh
.text:0041592A                 jnz     loc_415ADE
.text:00415930                 mov     ecx, ebp
.text:00415932                 call    game_unk129
.text:00415937                 jmp     loc_415ADE

	}
loc_41593C:

.text:0041593C loc_41593C:                             ; CODE XREF: game_unk116+446j
.text:0041593C                 xor     ecx, ecx
.text:0041593E                 mov     eax, offset _tasksTable
.text:00415943
.text:00415943 loc_415943:                             ; CODE XREF: game_unk116+580j
.text:00415943                 cmp     [eax], ebx
.text:00415945                 jz      short loc_415989
.text:00415947                 add     eax, 44h
.text:0041594A                 inc     ecx
.text:0041594B                 cmp     eax, offset _andyActionKeyMaskAnd
.text:00415950                 jl      short loc_415943
.text:00415952
.text:00415952 loc_415952:                             ; CODE XREF: game_unk116+5C9j
.text:00415952                 mov     eax, [esi+10h]
.text:00415955                 mov     [esi], ebx
.text:00415957                 cmp     eax, ebx
.text:00415959                 jz      short loc_415960
.text:0041595B                 mov     ecx, eax
.text:0041595D                 mov     [ecx+30h], ebx
.text:00415960
.text:00415960 loc_415960:                             ; CODE XREF: game_unk116+589j
.text:00415960                 mov     eax, (offset _gameMstUnkTable2_sizeof64+8)
.text:00415965
.text:00415965 loc_415965:                             ; CODE XREF: game_unk116+5A8j
.text:00415965                 cmp     [eax-8], ebx
.text:00415968                 jz      short loc_415970
.text:0041596A                 cmp     [eax], esi
.text:0041596C                 jnz     short loc_415970
.text:0041596E                 mov     [eax], ebx
.text:00415970
.text:00415970 loc_415970:                             ; CODE XREF: game_unk116+598j
.text:00415970                                         ; game_unk116+59Cj
.text:00415970                 add     eax, 64
.text:00415973                 cmp     eax, offset dword_468168
.text:00415978                 jl      short loc_415965

	removeLvlObjectNotType2List1(_edi);
	return;

.text:00415989 loc_415989:                             ; CODE XREF: game_unk116+575j
.text:00415989                 mov     eax, ecx
.text:0041598B                 shl     eax, 4
.text:0041598E                 add     eax, ecx
.text:00415990                 lea     ebp, _tasksTable.codeData[eax*4]
.text:00415997                 cmp     ebp, ebx
.text:00415999                 jz      short loc_415952
.text:0041599B                 mov     ecx, 11h
.text:004159A0                 xor     eax, eax
.text:004159A2                 mov     edi, ebp
.text:004159A4                 mov     edx, 4D2h
.text:004159A9                 rep stosd
.text:004159AB                 mov     ecx, ebp
.text:004159AD                 call    resetTask
.text:004159B2                 mov     eax, _gameMstResToLoad2Pri
.text:004159B7                 mov     [ebp+4], ebx // prev
.text:004159BC                 mov     [ebp+8], eax // next
.text:004159BA                 cmp     eax, ebx
.text:004159BF                 jz      short loc_4159C4
.text:004159C1                 mov     [eax+4], ebp
.text:004159C4
.text:004159C4 loc_4159C4:                             ; CODE XREF: game_unk116+5EFj
.text:004159C4                 mov     edi, _currentTask
.text:004159CA                 mov     [ebp+Task.dataPtr], esi
.text:004159CD                 mov     [ebp+Task.field_10], ebx
.text:004159D0                 mov     eax, [ebp+Task.child]
.text:004159D5                 mov     _gameMstResToLoad2Pri, ebp
.text:004159DB                 mov     [esi+0C4h], ebp
.text:004159E1                 mov     _currentTask, ebp
.text:004159D3                 cmp     eax, ebx
.text:004159E7                 jz      short loc_4159EE
.text:004159E9                 mov     [eax], ebx
.text:004159EB                 mov     [ebp+Task.child], ebx
.text:004159EE
.text:004159EE loc_4159EE:                             ; CODE XREF: game_unk116+617j
.text:004159EE                 mov     eax, [ebp+Task.prev]
.text:004159F1                 mov     ecx, [ebp+Task.next]
.text:004159F6                 mov     [ebp+Task.codeData], ebx
.text:004159F4                 cmp     eax, ebx
.text:004159F9                 jz      short loc_4159FE
.text:004159FB                 mov     [eax+4], ecx
.text:004159FE
.text:004159FE loc_4159FE:                             ; CODE XREF: game_unk116+629j
.text:004159FE                 cmp     ecx, ebx
.text:00415A00                 jz      short loc_415A07
.text:00415A02                 mov     [ecx+8], eax
.text:00415A05                 jmp     short loc_415A0C
.text:00415A07
.text:00415A07 loc_415A07:                             ; CODE XREF: game_unk116+630j
.text:00415A07                 mov     _gameMstResToLoad2Pri, eax
.text:00415A0C
.text:00415A0C loc_415A0C:                             ; CODE XREF: game_unk116+635j
.text:00415A0C                 mov     eax, _gameMstResToLoad2Pri
.text:00415A11                 cmp     eax, ebx
.text:00415A13                 jnz     short loc_415A23
.text:00415A15                 mov     _gameMstResToLoad2Pri, ebp
.text:00415A1B                 mov     [ebp+Task.prev], ebx
.text:00415A1E                 mov     [ebp+Task.next], ebx
.text:00415A21                 jmp     short loc_415A3C
.text:00415A23
.text:00415A23 loc_415A23:                             ; CODE XREF: game_unk116+643j
.text:00415A23                 mov     ecx, [eax+8]
.text:00415A26                 cmp     ecx, ebx
.text:00415A28                 jz      short loc_415A33
.text:00415A2A
.text:00415A2A loc_415A2A:                             ; CODE XREF: game_unk116+661j
.text:00415A2A                 mov     eax, ecx
.text:00415A2C                 mov     ecx, [eax+8]
.text:00415A2F                 cmp     ecx, ebx
.text:00415A31                 jnz     short loc_415A2A
.text:00415A33
.text:00415A33 loc_415A33:                             ; CODE XREF: game_unk116+658j
.text:00415A33                 mov     [eax+8], ebp
.text:00415A36                 mov     [ebp+Task.prev], ebx
.text:00415A39                 mov     [ebp+Task.next], eax
.text:00415A3C
.text:00415A3C loc_415A3C:                             ; CODE XREF: game_unk116+651j
.text:00415A42                 mov     [ebp+Task.codeData], 1234

.text:00415A3C                 lea     ecx, [esi+0CCh]
.text:00415A49                 call    game_unk66
.text:00415A4E                 lea     ecx, [esi+0C8h]
.text:00415A54                 call    game_unk66
.text:00415A59                 mov     edx, [esi+4]
.text:00415A5C                 mov     eax, [edx+20h]
.text:00415A5F                 mov     edx, [esi+8]
.text:00415A62                 mov     ecx, [eax]
.text:00415A64                 mov     dword ptr [esi+98h], 0FFFFFFFFh
.text:00415A6E                 mov     [esi+0Ch], ecx
.text:00415A71                 mov     al, [edx+3B2h]
.text:00415A77                 test    al, 4
.text:00415A79                 jz      short loc_415A89
.text:00415A7B                 mov     byte ptr [esi+0A8h], 0FFh
.text:00415A82                 mov     byte ptr [esi+0A9h], 0FFh
.text:00415A89
.text:00415A89 loc_415A89:                             ; CODE XREF: game_unk116+6A9j
.text:00415A89                 mov     ecx, ebp
.text:00415A8B                 call    GameMstUnk3

	switch (arg10) {
	case 1:
		game_unk94(_ebp);
		break;
	case 2:
		if (_esi) {
			_esi->unkA6 = 1; // byte
		}
		game_unk95(_ebp, 0);
		break;
	default:
		_esi->unkA5 = 1; // byte
		if (game_unk34(_esi) == 0) {
			game_unk65(_esi);
		}
		game_unk108(_ebp);
		break;
	}
	_currentTask = _edi;
	task->flags &= ~0x80;
#endif
}


void Game::GameMstUpdateMovingStateFromMainObject() {
	if (_andyObject) {
		_gameMstScreenRefPosX = _andyObject->xPos;
		_gameMstScreenRefPosY = _andyObject->yPos;
		const uint8_t *p = _resMstPointsData + _gameResData0x2E08 * 8;
		_gameMstMovingStatePosX = _gameMstScreenRefPosX + READ_LE_UINT32(p + 0);
		_gameMstMovingStatePosY = _gameMstScreenRefPosY + READ_LE_UINT32(p + 4);
		if (_gameMstUnk9) {
assert(0); // code path check
// TODO
//			_gameMstUnk10 = game_unk45(_gameMstUnk10, 0xFE, _gameMstMovingStatePosX, _gameMstMovingStatePosY, _gameMstMovingStatePosX + _andyObject->width - 1, _gameMstMovingStatePosY + _andyObject->height - 1) & 0xFF;
		}
		_gameMstScreenRefPosX += _andyObject->posTable[3].x;
		_gameMstScreenRefPosY += _andyObject->posTable[3].y;
		_gameMstMovingStatePosX += _andyObject->posTable[3].x;
		_gameMstMovingStatePosY += _andyObject->posTable[3].y;
	} else {
		_gameMstScreenRefPosX = 128;
		_gameMstScreenRefPosY = 96;
		_gameMstMovingStatePosX = (int32)READ_LE_UINT32(_resMstPointsData + 0) + 128;
		_gameMstMovingStatePosY = (int32)READ_LE_UINT32(_resMstPointsData + 4) + 96;
	}
	_gameMstMovingStateCount = 0;
	_gameMstMovingState[0].unk0x40 = 0;
	if (!_lvlObjectsList0) {
		if (_plasmaCannonDirection == 0) {
			goto end2;
		}
		_gameMstMovingState[0].unk0x1C = 512;
		_gameMstMovingState[0].unk0x20 = 512;
		_gameMstMovingState[0].unk0x28 = 0;
		_gameMstMovingState[0].unk0x40 = 3;
		_gameMstMovingState[0].unk0x18 = 4;
		_gameMstMovingState[0].xPos = _gameXPosTable[_plasmaCannonFirstIndex] + (int32)READ_LE_UINT32(_resMstPointsData + _gameResData0x2E08 * 8 + 0);
		_gameMstMovingState[0].yPos = _gameYPosTable[_plasmaCannonFirstIndex] + (int32)READ_LE_UINT32(_resMstPointsData + _gameResData0x2E08 * 8 + 4);
		switch (_plasmaCannonDirection - 1) {
		case 0:
			_gameMstMovingState[0].unk0x24 = 6;
			_gameMstMovingStateCount = 1;
			break;
		case 2:
			_gameMstMovingState[0].unk0x24 = 3;
			_gameMstMovingStateCount = 1;
			break;
		case 1:
			_gameMstMovingState[0].unk0x24 = 0;
			_gameMstMovingStateCount = 1;
			break;
		case 5:
			_gameMstMovingState[0].unk0x24 = 4;
			_gameMstMovingStateCount = 1;
			break;
		case 3:
			_gameMstMovingState[0].unk0x24 = 7;
			_gameMstMovingStateCount = 1;
			break;
		case 11:
			_gameMstMovingState[0].unk0x24 = 2;
			_gameMstMovingStateCount = 1;
			break;
		case 7:
			_gameMstMovingState[0].unk0x24 = 5;
			_gameMstMovingStateCount = 1;
			break;
		case 8:
			_gameMstMovingState[0].unk0x24 = 1;
			_gameMstMovingStateCount = 1;
			break;
		default:
			_gameMstMovingStateCount = 1;
			break;
		}
	} else {
		MovingOpcodeState *p = _gameMstMovingState;
		for (LvlObject *o = _lvlObjectsList0; o; o = o->nextPtr) {
			p->unk0x2C = o;
assert(0); // code path check
#if 0
			GameUnkList1 *_eax = (GameUnkList1 *)getLvlObjectDataPtr(o, kObjectDataTypeUnk1);
			p->unk0x28 = _eax;
			assert(_eax);
			if (_eax->unk3 == 0x80) {
				continue; // loc_40F437
			}
			if (_eax->unk4 == 0 && _eax->unk8 == 0) {
				continue; // loc_40F437
			}
			p->unk1C = ABS(READ_LE_UINT32(_eax + 4));
			p->unk20 = ABS(READ_LE_UINT32(_eax + 8));
			p->unk0x24 = _eax->unk1;
			switch (_eax->unk0) {
			case 0: // loc_40F3F1
				p->unk0x40 = 1;
				p->xPos = o->xPos + (int32)READ_LE_UINT32(_resMstPointsData + o->data0x2E08 * 8 + 0) + o->posTable[7].x;
				p->unk0x18 = 3;
				p->yPos = o->yPos + (int32)READ_LE_UINT32(_resMstPointsData + o->data0x2E08 * 8 + 4) + o->posTable[7].y;
				break;
			case 5: // loc_40F3B0
				p->unk0x24 |= 0x80;
				// fall through
			case 4: // loc_40F3B8
				p->unk0x40 = 2;
				p->xPos = o->xPos + (int32)READ_LE_UINT32(_resMstPointsData + o->data0x2E08 * 8 + 0) + o->posTable[7].x;
				p->unk0x18 = 7;
				p->yPos = o->yPos + (int32)READ_LE_UINT32(_resMstPointsData + o->data0x2E08 * 8 + 4) + o->posTable[7].y;
				break;
			default:
				--p;
				--_gameMstMovingStateCount;
				break;
			}
			++p;
			++_gameMstMovingStateCount;
			if (_gameMstMovingStateCount >= 8) {
				break;
			}
#endif
		}
		if (_gameMstMovingStateCount == 0) {
			goto end2;
		}
	}
	for (int i = 0; i < _gameMstMovingStateCount; ++i) {
		MovingOpcodeState *p = &_gameMstMovingState[i];
		p->boundingBox.x2 = p->xPos + p->unk0x18;
		p->boundingBox.x1 = p->xPos - p->unk0x18;
		p->boundingBox.y2 = p->yPos + p->unk0x18;
		p->boundingBox.y1 = p->yPos - p->unk0x18;
	}
	return;

end2:
	_executeMstLogicLastCounter = _executeMstLogicCounter;
	return;
}


#include "game.h"
#include "resource.h"

void Game::resetMstCode() {
	if (_mstLogicDisabled) {
		return;
	}
	_mstGlobalFlags = 0;
	for (int i = 0; i < 32; ++i) {
		clearGameMstUnk1(&_gameMstUnkTable1[i]);
	}
	for (int i = 0; i < 64; ++i) {
		clearGameMstUnk2(&_gameMstUnkTable2[i]);
	}
	clearLvlObjectsList1();
	for (int i = 0; i < _resMstHeader0x18; ++i) {
		MstScreenAreaCode *p = ((MstScreenAreaCode *)_resMstUnk38) + i;
		p->unk0x1D = 1;
	}
#if 0
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 32; ++j) {
			byte_466F60[32 * i + j] = j;
		}
		for (int j = 0; j < 64; ++j) {
			const int index1 = _rnd.update() & 31;
			const int index2 = _rnd.update() & 31;
			SWAP(byte_466F60[32 * i + index1], byte_466F60[32 * i + index2]);
		}
	}
#endif
	_rnd.initTable();
#if 0
	for (int i = 0; i < _resMstHeader0x40; ++i) { // var8
		const int max = READ_LE_UINT32(_resMstUnk49 + i * 24 + 16);
		if (max != 0) {
			ptr = READ_LE_UINT32(_resMstUnk49 + i * 24 + 12);
			for (int k = 0; k < max * 2; ++k) {
				const int index1 = _rnd.update() % max;
				const int index2 = _rnd.update() % max;
				SWAP(ptr[index1], ptr[index2]);
			}
		}
	}
	for (int i = 0; i < _resMstHeader0x0C; ++i) { // var8
		const int max = READ_LE_UINT32(_resMstUnk35 + i * 16 + 12);
		if (max != 0) {
			ptr = READ_LE_UINT32(_resMstUnk35 + i * 16 + 8);
			for (int k = 0; k < max * 2; ++k) {
				const int index1 = _rnd.update() % max;
				const int index2 = _rnd.update() % max;
				SWAP(ptr[index1], ptr[index2]);
			}
		}
	}
#endif
	for (int i = 0; i < _resMstHeader0x24; ++i) {
		MstUnk43 *m = ((MstUnk43 *)_resMstUnk43) + i;
		const int count = m->count2;
		if (count != 0) {
			for (int k = 0; k < count; ++k) {
				_resMstUnk43[m->offset2 + k] &= 0x7F;
			}
			for (int k = 0; k < count * 2; ++k) {
				const int index1 = _rnd.update() % count;
				const int index2 = _rnd.update() % count;
				SWAP(_resMstUnk43[m->offset2 + index1], _resMstUnk43[m->offset2 + index2]);
			}
		}
	}
#if 0
	_gameMstUnk1 = 0xFFFFFF00;
	_gameMstUnk2 = 0xFFFFFF00;
#endif
	memset(_gameMstUnkTable1, 0, sizeof(_gameMstUnkTable1));
	memset(_gameMstUnkTable2, 0, sizeof(_gameMstUnkTable2));
	memset(_globalVars, 0, sizeof(_globalVars));
	memset(_tasksTable, 0, sizeof(_tasksTable));
	_gameMstUnk3 = -1;
	_gameMstUnk4 = -1;
	_gameMstUnk5 = -1;
	_gameMstUnk6 = -1;
	_executeMstLogicCounter = 0;
	_executeMstLogicLastCounter = 0;
#if 0
	_gameMstUnk8 = 0;
	_gameMstUnk9 = 0;
	_gameMstUnk10 = 0xFF;
	_gameMstUnk11 = 0;
	_gameMstUnk12 = 0;
	_gameMstUnk13 = 0;
	_gameMstUnk14 = 0xFF;
	_gameMstUnk15 = 256;
	_gameMstUnk16 = 256;
	_gameMstUnk17 = 0;
	_gameMstUnk18 = 0;
	_gameMstUnk19 = 0xFF;
#endif
	_gameMstLogicHelper1TestValue = 0;
	_gameMstLogicHelper1TestMask = 0xFFFFFFFF;
//	_gameMstEq0 = 0;
	_tasksListTail = 0;
	_gameMstResToLoad2Pri = 0;
	_gameMstResToLoad1Pri = 0;
#if 0
	_gameMstResToLoad2Num = 0;
	_gameMstResToLoad1Num = 0;
#endif
	if (PTR_OFFS<uint32>(_resMstUnk62, _resMstUnk37->codeData)) {
		_globalVars[31] = FROM_LE32(_resMstUnk37->delay);
	} else {
		_globalVars[31] = -1;
	}
	_gameMstUnk27 = FROM_LE32(_resMstUnk37->delay);
	_globalVars[30] = 0x20;
	for (int i = 0; i < 32; ++i) {
		_gameMstUnkTable1[i].unk0x20 = i;
	}
	for (int i = 0; i < 64; ++i) {
		_gameMstUnkTable2[i].unk0x10 = i;
	}
	GameMstUpdateMovingStateFromMainObject();
	_gameMstObjectRefPointPosY = _gameMstMovingStatePosY;
	_gameMstObjectRefPointPosX = _gameMstMovingStatePosX;
	int offset = 0;
	for (int i = 0; i < _resMstHeader0x3C; ++i) {
		offset += 948;
		_gameMstHeightMapData[offset - 0x20] = _gameMstMovingStatePosX - _gameMstHeightMapData[offset - 0x30];
		_gameMstHeightMapData[offset - 0x1C] = _gameMstMovingStatePosX + _gameMstHeightMapData[offset - 0x30];
		_gameMstHeightMapData[offset - 0x24] = _gameMstHeightMapData[offset - 0x20] - _gameMstHeightMapData[offset - 0x34];
		_gameMstHeightMapData[offset - 0x18] = _gameMstHeightMapData[offset - 0x1C] + _gameMstHeightMapData[offset - 0x34];
		_gameMstHeightMapData[offset - 0x10] = _gameMstMovingStatePosY - _gameMstHeightMapData[offset - 0x30];
		_gameMstHeightMapData[offset - 0x0C] = _gameMstMovingStatePosY + _gameMstHeightMapData[offset - 0x30];
		_gameMstHeightMapData[offset - 0x14] = _gameMstHeightMapData[offset - 0x10] - _gameMstHeightMapData[offset - 0x34];
		_gameMstHeightMapData[offset - 0x08] = _gameMstHeightMapData[offset - 0x0C] + _gameMstHeightMapData[offset - 0x34];
	}
}

void Game::startMstCode() {
	GameMstUpdateMovingStateFromMainObject();
	_gameMstObjectRefPointPosY = _gameMstMovingStatePosY;
	_gameMstObjectRefPointPosX = _gameMstMovingStatePosX;
	int offset = 0;
	for (int i = 0; i < _resMstHeader0x3C; ++i) {
		offset += 948;
		_gameMstHeightMapData[offset - 0x20] = _gameMstMovingStatePosX - _gameMstHeightMapData[offset - 0x30];
		_gameMstHeightMapData[offset - 0x1C] = _gameMstMovingStatePosX + _gameMstHeightMapData[offset - 0x30];
		_gameMstHeightMapData[offset - 0x24] = _gameMstHeightMapData[offset - 0x20] - _gameMstHeightMapData[offset - 0x34];
		_gameMstHeightMapData[offset - 0x18] = _gameMstHeightMapData[offset - 0x1C] + _gameMstHeightMapData[offset - 0x34];
		_gameMstHeightMapData[offset - 0x10] = _gameMstMovingStatePosY - _gameMstHeightMapData[offset - 0x30];
		_gameMstHeightMapData[offset - 0x0C] = _gameMstMovingStatePosY + _gameMstHeightMapData[offset - 0x30];
		_gameMstHeightMapData[offset - 0x14] = _gameMstHeightMapData[offset - 0x10] - _gameMstHeightMapData[offset - 0x34];
		_gameMstHeightMapData[offset - 0x08] = _gameMstHeightMapData[offset - 0x0C] + _gameMstHeightMapData[offset - 0x34];
	}
	if (_currentScreenResourceState < _resMstHeader0x14) {
		const uint8_t *ptr = (const uint8_t *)PTR<uint32>(_resMstUnk62, _resMstCodeData_screenInit, _currentScreenResourceState);
		if (ptr) {
			Task *t = createTask(ptr);
			if (t) {
				while ((this->*(t->run))(t) == 0);
			}
		}
	}
}

void Game::updateMstHeightMapData() {
	int dword_4681DC = _gameMstMovingStatePosX - _gameMstObjectRefPointPosX;
	int dword_4681D8 = _gameMstMovingStatePosY - _gameMstObjectRefPointPosY;
	_gameMstObjectRefPointPosX = _gameMstMovingStatePosX;
	_gameMstObjectRefPointPosY = _gameMstMovingStatePosY;
	if (dword_4681DC == 0 && dword_4681D8 == 0) {
		return;
	}
	int offset = 0;
	for (int i = 0; i < _resMstHeader0x3C; ++i) {
		offset += 948;
		_gameMstHeightMapData[offset - 0x20] = _gameMstMovingStatePosX - _gameMstHeightMapData[offset - 0x30];
		_gameMstHeightMapData[offset - 0x1C] = _gameMstMovingStatePosX + _gameMstHeightMapData[offset - 0x30];
		_gameMstHeightMapData[offset - 0x24] = _gameMstHeightMapData[offset - 0x20] - _gameMstHeightMapData[offset - 0x34];
		_gameMstHeightMapData[offset - 0x18] = _gameMstHeightMapData[offset - 0x1C] + _gameMstHeightMapData[offset - 0x34];
		_gameMstHeightMapData[offset - 0x10] = _gameMstMovingStatePosY - _gameMstHeightMapData[offset - 0x30];
		_gameMstHeightMapData[offset - 0x0C] = _gameMstMovingStatePosY + _gameMstHeightMapData[offset - 0x30];
		_gameMstHeightMapData[offset - 0x14] = _gameMstHeightMapData[offset - 0x10] - _gameMstHeightMapData[offset - 0x34];
		_gameMstHeightMapData[offset - 0x08] = _gameMstHeightMapData[offset - 0x0C] + _gameMstHeightMapData[offset - 0x34];
	}
}

Task *Game::findFreeTask() {
	for (int i = 0; i < 128; ++i) {
		Task *t = &_tasksTable[i];
		if (!t->codeData) {
			return t;
		}
	}
	return 0;
}

Task *Game::createTask(const uint8_t *codeData) {
	for (int i = 0; i < 128; ++i) {
		Task *t = &_tasksTable[i];
		if (!t->codeData) {
			memset(t, 0, sizeof(Task));
			resetTask(t, codeData);
			t->next = 0;
			t->prev = _tasksListTail;
			if (_tasksListTail) {
				_tasksListTail->next = t;
			}
			_tasksListTail = t;
			return t;
		}
	}
	return 0;
}

void Game::resetTask(Task *t, const uint8_t *codeData) {
	t->runningState |= 2;
	t->codeData = codeData;
	t->run = &Game::runTask_main;
	// t->unk0x30 = 0;
#if 0
	_eax = t->dataPtr;
	if (_eax) {
		uint8_t mask = _eax->0xA5(byte);
		if ((mask & 0x88) != 0 && (mask & 0xF0) != 0) {
			if ((mask & 8) != 0) {
				t->flags2 = (t->flags2 & ~0x40) | 0x20;
				_eax->0x48(byte) &= ~0x1C;
			} else if ((mask & 2) != 0) {
				_eax->0x48(byte) |= 8;
				_eax = t->dataPtr;
				_edx = _eax->0x4(le32);
				if (_edx->0x1C(le32)) {
					_eax->0x48(byte) |= 0x04;
				}
				if (_edx->0x24(le32)) {
					_eax->0x48(byte) |= 0x10;
				}
			}
		}
	}
#endif
}

void Game::removeTask(Task **tasksList, Task *t) {
	Task *c = t->child;
	if (c) {
		c->codeData = 0;
		t->child = 0;
	}
	Task *next = t->next;
	t->codeData = 0;
	t = t->prev;
	if (t) {
		t->next = next;
	}
	if (next) {
		next->prev = t;
	} else {
		*tasksList = t;
	}
}

Task *Game::createTaskIfNotPresent(Task *t, int num, const uint8_t *codeData) {
	Task *_eax = _tasksListTail;
	int _ebx = 0;
	if (_eax) {
		Task *_edi;
		do {
			_edi = _eax->prev;
			if ((t->localVars[14] | (t->localVars[15] << 16)) == num) {
				_ebx = 1;
				if (_eax != t) {
					if (!codeData) {
assert(0); // code path check
#if 0
						Task *_esi = _eax->child;
						if (_esi) {
							_esi->codeData = codeData;
							_eax->child = codeData; // =_esi ?
						}
#endif
						Task *_esi = _eax->next;
						_eax->codeData = 0;
						_eax = _eax->prev;
						if (_eax) {
							_eax->next = _esi;
						}
						if (_esi) {
							_esi->prev = _eax;
						} else {
							_tasksListTail = _eax;
						}
					} else {
						_eax->codeData = codeData;
						_eax->run = &Game::runTask_main;
					}
				}
			}
		} while ((_eax = _edi) != 0);
		if (_ebx != 0) {
			return _eax;
		}
		_eax = &_tasksTable[0];
	}
	for (; _eax < &_tasksTable[128]; ++_eax) {
		t = _eax;
		if (!t->codeData) {
			memset(t, 0, sizeof(Task));
			resetTask(t, codeData);
			t->next = 0;
			t->prev = _tasksListTail;
			if (_tasksListTail) {
				_tasksListTail->next = 0;
			}
			_tasksListTail = t;
			t->localVars[14] = num & 0xFFFF;
			t->localVars[15] = num >> 16;
			return t;
		}
	}
	return 0;
}

template <typename T>
static bool compareOp(int op, T num1, T num2) { // _esi, _eax
	switch (op) {
	case 0:
		return num1 != num2;
	case 1:
		return num1 == num2;
	case 2:
		return num1 >= num2;
	case 3:
		return num1 > num2;
	case 4:
		return num1 <= num2;
	case 5:
		return num1 < num2;
	case 6:
		return (num1 & num2) == 0;
	case 7:
		return (num1 | num2) == 0;
	case 8:
		return (num1 ^ num2) == 0;
	default:
		printf("compareOp unhandled op %d\n", op);
		assert(0);
		break;
	}
	return false;
}

template <typename T>
static void arithOp(int op, T *p, int num) {
	switch (op) {
	case 0:
		*p = num;
		break;
	case 1:
		*p += num;
		break;
	case 2:
		*p -= num;
		break;
	case 3:
		*p *= num;
		break;
	case 4:
		if (num != 0) {
			*p /= num;
		}
		break;
	case 5:
		*p <<= num;
		break;
	case 6:
		*p >>= num;
		break;
	case 7:
		*p &= num;
		break;
	case 8:
		*p |= num;
		break;
	case 9:
		*p ^= num;
		break;
	default:
		printf("arithOp unhandled op %d\n", op);
		assert(0);
		break;
	}
}

int Game::getTaskVar(Task *t, int type, int index) {
	int num = 0;
	switch (type) {
	case 1:
		num = index;
		break;
	case 2:
		num = t->localVars[index];
		break;
	case 3:
		num = _globalVars[index];
		break;
	case 4:
		num = getSpecialTaskVar(index, t);
		break;
	case 5:
		assert(0);
#if 0
		uint8_t *p = t->0x10(le32);
		if (p) {
			_ecx = p->0x8(le32)
		} else {
			_ecx = t->dataPtr;
		}
		if (p) {
			num = _ecx->0x28+index*4(le32);
		}
		break;
#endif
	}
	return num;
}

int Game::getSpecialTaskVar(int index, Task *t) const {
	switch (index) {
	case 0:
		return _gameMstScreenRefPosX;
	case 1:
		return _gameMstScreenRefPosY;
	case 2:
		return _gameMstMovingStatePosX;
	case 3:
		return _gameMstMovingStatePosY;
	case 4:
		return _gameResData0x2E08;
	case 5:
		return _res->_screensState[_gameResData0x2E08].s0;
	case 22:
		return _mstOp54Counter;
	case 23:
		return _andyObject->actionKeyMask;
	case 24:
		return _andyObject->directionKeyMask;
	case 31:
		return _executeMstLogicCounter;
	case 32:
		return _executeMstLogicCounter - _executeMstLogicLastCounter;
	case 34:
		return _currentScreenResourceState;
	case 35:
		return _gameCurrentData0x2E08Num;
	default:
		printf("getSpecialTaskVar unhandled index %d\n", index);
		assert(0);
		break;
	}
	return 0;
}

int Game::runTask_main(Task *t) {
	int ret = 0;
	t->runningState &= ~2;
	const uint8_t *p = t->codeData; // _ebx
	do {
//		printf("runTask_main opcode %d _runTaskOpcodesCount %d offset %d\n", *p, _runTaskOpcodesCount, p - _resMstUnk62);
		assert(p >= _resMstUnk62 && p < _resMstUnk62 + _resMstHeader0x7C * 4);
		assert(((p - t->codeData) & 3) == 0);
		switch (*p) {
		case 0: {
/*				uint8_t *p = t->dataPtr;
				if (p) {
					if ((p->0xA6(byte) & 2) == 0) {
						p = p->0x10(le32)
					}
				} else {
					p = t->0x10(le32);
					if (p) {
						p = p->0x4(le32);
					}
				}
				if (p) {
					p->0x20(byte) = 0;
					p->0x21(byte) = 0;
				}*/
			}
			// fall-through
		case 1: {
				const int index = (int16)READ_LE_UINT16(p + 2);
				int num = getTaskVar(t, p[1], index);
				assert(p[1] < 5);
				t->delay = num;
				if (num > 0) {
					if (p[0] == 0) {
						t->run = &Game::runTask_waitDelayInput;
						ret = 1;
					} else {
						t->run = &Game::runTask_waitDelay;
						ret = 1;
					}
				}
			}
			break;
		case 2: { // 2
				const int num = (int16)READ_LE_UINT16(p + 2);
				const uint8_t var = _resMstUnk56[num * 12 + 8];
//				_ecx = _resMstUnk56[num * 12];
				switch ((var >> 4) & 0xF) {
				default:
					printf("unhandled runTask op case 2 num %d/%d var %d\n", num, _resMstHeader0x60, var);
					break;
				}
			}
			break;
//		case 8: // 3
		case 3: // 3
			if (t->dataPtr) {
				printf("runTask_main() skipping opcode %d\n", *p);
			}
			break;
//		case 14: // 9
//			// fall-through
//		case 4: // 4
//			printf("runTask_main() skipping opcode %d\n", *p);
//			break;
//		case 5: // 5
//			printf("runTask_main() skipping opcode %d\n", *p);
//			break;
//		case 16: // 11
//			// fall-through
//		case 6: // 6
//			printf("runTask_main() skipping opcode %d\n", *p);
//			break;
//		case 22: // 12
//			// fall-through
//		case 7: // 7
//		case 12: // 7
//			printf("runTask_main() skipping opcode %d\n", *p);
//			break;
//		case 24: // 14
//			t->flags |= 1 << p[1];
//			break;
		case 26: // 16
			_mstGlobalFlags &= ~(1 << p[1]);
			break;
//		case 27: // 17
//			t->flags &= ~(1 << p[1]);
//			break;
		case 30: // 20
			t->delay = 3;
			t->unk0x38 = 1 << p[1];
			if ((_mstGlobalFlags & t->unk0x38) == 0) {
//				if (t->dataPtr) {
//				} else if (t->unk10) {
//				}
				t->run = &Game::runTask_mstDelay;
				ret = 1;
			}
			break;
		case 33: { // 23
				const int offset = READ_LE_UINT16(p + 2);
				p = _resMstUnk62 + offset * 4 - 4;
			}
			break;
		case 35: { // 24
				const int num = READ_LE_UINT16(p + 2);
				flagMstCodeForPos(num, 1);
			}
			break;
		case 42: { // 29
				const int num = p[1];
				++_globalVars[num];
			}
			break;
		case 44: { // 31
				const int num = p[1];
				--t->localVars[num];
			}
			break;
//		case 144: { // 43
//				const int num = getSpecialTaskVar(p[2], t);
//				arithOp(p[0] - 137, &t->localVars[p[1]], num);
//			}
//			break;
		case 177: // 47
		case 178: // 48
		case 179: { // 47
				const int num = (int16)READ_LE_UINT16(p + 2);
				assert(p[1] < 40);
				arithOp(p[0] - 177, &_globalVars[p[1]], num);
			}
			break;
		case 198: { // 50
				Task *c = findFreeTask();
				assert(c);
				memcpy(c, t, sizeof(Task));
				c->codeData = p + 4;
				const int index = READ_LE_UINT16(p + 2);
				t->codeData = (const uint8_t *)PTR<uint32>(_resMstUnk62, _resMstUnk60, index);
				assert(t->codeData != 0);
				t->runningState &= ~2;
				t->child = c;
				p = t->codeData - 4;
			}
			break;
		case 202:
			runTask_main_op54();
			break;
		case 204: { // 56
				const int num = p[1];
				const int index = READ_LE_UINT16(p + 2);
				ret = runTask_main_op56(t, num, index);
			}
			break;
		case 217: { // 64
				const int num = READ_LE_UINT16(p + 2);
				if (_gameMstUnk3 != num) {
					_gameMstUnk3 = num;
					shuffleMstUnk43(((MstUnk43 *)_resMstUnk43) + num);
//					_mstOp54Counter = 0;
				}
			}
			break;
		case 218: { // 65
				const int num = READ_LE_UINT16(p + 2);
				if (_gameMstUnk4 != num) {
					_gameMstUnk4 = _gameMstUnk5 = num;
					shuffleMstUnk43(((MstUnk43 *)_resMstUnk43) + num);
//					_null_var33 = 0;
				}
			}
			break;
		case 223: { // 67
				const int num = READ_LE_UINT16(p + 2); // _eax
				MstUnk53 *m = ((MstUnk53 *)_resMstUnk53) + num; // _esi
				int var8 = m->unk16;
				int var1C = getTaskVar(t, (var8 >> 16) & 15, m->index1);
				int var20 = getTaskVar(t, (var8 >> 12) & 15, m->index2);
				int var14 = getTaskVar(t, (var8 >>  8) & 15, m->index3); // _ebx
				int _edi  = getTaskVar(t, (var8 >>  4) & 15, m->index4);
				int _eax  = getTaskVar(t,  var8        & 15, m->index5);
				if (var1C > var20) {
					SWAP(var20, var1C);
				}
				if (var14 > _edi) {
					SWAP(var14, _edi);
				}
//				if (t->unk10) {
//				} else if (t->dataPtr) {
//				} else
					int _ecx = 0;

				if (_eax <= -2 && _ecx) {
assert(0); // code path check
#if 0
.text:00413651                 test    byte ptr [ecx+12h], 10h
.text:00413655                 jz      short loc_41367F
.text:00413657                 mov     edx, [esp+34h+var_1C]
.text:0041365B                 mov     ebx, [esp+34h+var_20]
.text:0041365F                 mov     [esp+34h+var_4], edx
.text:00413663                 mov     edx, [ecx]
.text:00413665                 mov     [esp+34h+var_8], edx
.text:00413669                 sub     edx, ebx
.text:0041366B                 mov     ebx, [esp+34h+var_4]
.text:0041366F                 mov     [esp+34h+var_1C], edx
.text:00413673                 mov     edx, [esp+34h+var_8]
.text:00413677                 sub     edx, ebx
.text:00413679                 mov     [esp+34h+var_20], edx
.text:0041367D                 jmp     short loc_413695
.text:0041367F
.text:0041367F loc_41367F:                             ; CODE XREF: runTask_main+1885j
.text:0041367F                 mov     edx, [ecx]
.text:00413681                 mov     ebx, [esp+34h+var_1C]
.text:00413685                 add     ebx, edx
.text:00413687                 mov     [esp+34h+var_1C], ebx
.text:0041368B                 mov     ebx, [esp+34h+var_20]
.text:0041368F                 add     ebx, edx
.text:00413691                 mov     [esp+34h+var_20], ebx
.text:00413695
.text:00413695 loc_413695:                             ; CODE XREF: runTask_main+18ADj
.text:00413695                 mov     edx, [ecx+4]
.text:00413698                 mov     ebx, [esp+34h+var_14]
.text:0041369C                 add     ebx, edx
.text:0041369E                 add     edi, edx
.text:004136A0                 cmp     eax, 0FFFFFFFEh
.text:004136A3                 jge     short loc_4136C3
.text:004136A5                 movsx   eax, word ptr [ecx+54h]
.text:004136A9                 mov     edx, [esp+34h+var_1C]
.text:004136AD                 add     edx, eax
.text:004136AF                 mov     [esp+34h+var_1C], edx
.text:004136B3                 mov     edx, [esp+34h+var_20]
.text:004136B7                 add     edx, eax
.text:004136B9                 movsx   eax, word ptr [ecx+56h]
.text:004136BD                 mov     [esp+34h+var_20], edx
.text:004136C1                 jmp     short loc_4136DF
.text:004136C3
.text:004136C3 loc_4136C3:                             ; CODE XREF: runTask_main+18D3j
.text:004136C3                 movsx   eax, word ptr [ecx+58h]
.text:004136C7                 mov     edx, [esp+34h+var_1C]
.text:004136CB                 add     edx, eax
.text:004136CD                 mov     [esp+34h+var_1C], edx
.text:004136D1                 mov     edx, [esp+34h+var_20]
.text:004136D5                 add     edx, eax
.text:004136D7                 movsx   eax, word ptr [ecx+5Ah]
.text:004136DB                 mov     [esp+34h+var_20], edx
.text:004136DF
.text:004136DF loc_4136DF:                             ; CODE XREF: runTask_main+18F1j
.text:004136DF                 add     ebx, eax
.text:004136E1                 add     edi, eax
.text:004136E3                 xor     eax, eax
.text:004136E5                 mov     al, [ecx+8]
#endif
				}
				if (_eax < -1) {
					_eax = -1;
				} else if ((uint32)_eax >= (uint32)_resMstHeader0x80) {
					_eax = _resMstHeader0x80 - 1;
				}
				int _cl = p[0];
				if (_cl == 0xE0) {
assert(0); // code path check
#if 0
					byte_464D78 = m->unk8;
					byte_468251 = m->unk9;
					word_462A4C = m->unk12;
					_gameMstUnk1 = var1C;
					_gameMstUnk2 = var20;
					_gameMstUnk12 = _ebx;
					_gameMstUnk13 = _edi;
					_gameMstUnk14 = _al;
#endif
				} else if (_cl == 0xE1) {
assert(0); // code path check
#if 0
					byte_468162 = m->unk8;
					byte_468161 = m->unk9;
					word_464D6C = m->unk12;
					_gameMstUnk15 = var1C;
					_gameMstUnk16 = var20;
					_gameMstUnk17 = _ebx;
					_gameMstUnk18 = _edi;
					byte_468160 = _al;
#endif
				} else {
					t->flags |= 0x80;
					if (_cl == 0xDE || _cl == 0xDC) {
						if (_eax != -1) {
							if (_eax == _gameResData0x2E08) {
								break;
							}
						} else {
							if (var1C >= -_gameMstScreenRefPosX && var1C <= 255 - _gameMstScreenRefPosX) {
								break;
							}
						}
					}
//					game_unk116(t, var1C, var20, var14, _edi, _eax, m->unk8, m->unk9, m->unk12, m->unk11, 0, m->unk14);
				}
			}
			break;
		case 227: { // 69
				const int num = READ_LE_UINT16(p + 2); // _eax
				MstUnk54 *m = ((MstUnk54 *)_resMstUnk54) + num; // _esi
				int _esi = getTaskVar(t,  m->unk5       & 15, m->unk0);
				int _eax = getTaskVar(t, (m->unk5 >> 4) & 15, m->unk2);
				if (compareOp(m->unk4, _esi, _eax)) {
					t->codeData = (const uint8_t *)PTR_OFFS<uint32>(_resMstUnk62, m->codeIndex);
				}
			}
			break;
		case 231: // 71
			printf("runTask_main() skipping opcode %d\n", *p);
			break;
		case 234: // 72
			printf("runTask_main() skipping opcode %d\n", *p);
			break;
		case 239: { // 76
				const int index = READ_LE_UINT16(p + 2);
				const uint8_t *codeData = (const uint8_t *)PTR<uint32>(_resMstUnk62, _resMstUnk60, index);
				assert(codeData != 0);
				createTask(codeData);
			}
			break;
		case 240: { // 77
				const int index = READ_LE_UINT16(p + 2);
				MstUnk59 *m = ((MstUnk59 *)_resMstUnk59) + index;
				createTaskIfNotPresent(t, m->unk0, (const uint8_t *)PTR_OFFS<uint32>(_resMstUnk62, m->codeIndex));
			}
			break;
		case 242: // 78
			if (t->child) {
				printf("runTask_main() skipping opcode %d 00413F21\n", *p);
			} else if (t->dataPtr) {
				printf("runTask_main() skipping opcode %d 00413D0C\n", *p);
//			} else if (t->field_10) { // loc_413E74
//				game_unk78(t, &_gameMstResToLoad1Pri);
			} else {
				if ((t->runningState & 1) != 0 && _globalVars[31] == 0) {
					_globalVars[31] = _gameMstUnk27;
				}
				removeTask(&_tasksListTail, t);
				ret = 1;
			}
			break;
		default:
			printf("runTask_main() unhandled opcode %d\n", *p);
//			assert(0);
			break;
		}
		p += 4;
		if ((t->runningState & 2) != 0) {
			t->runningState &= ~2;
			p = t->codeData; // _ebx
		}
		++_runTaskOpcodesCount;
	} while (_runTaskOpcodesCount <= 128 && ret == 0);
	if (t->codeData) {
		t->codeData = p;
	}
	return 1;
}

void Game::runTask_main_op54() {
	if (_gameMstUnk6 != -1) {
		return;
	}
	MstUnk43 *_esi = 0;
	if ((_mstGlobalFlags & 0x20000000) != 0) {
		if (_gameMstUnk5 == -1) {
			return;
		}
		_esi = ((MstUnk43 *)_resMstUnk43) + _gameMstUnk5;
	} else {
		if (_gameMstUnk3 == -1) {
			return;
		}
		_esi = ((MstUnk43 *)_resMstUnk43) + _gameMstUnk3;
		_gameMstUnk5 = _gameMstUnk4;
	}
	int x = _gameMstScreenRefPosX;
	if (x > 255) {
		x = 255;
	}
	if (_gameMstScreenRefPosX < 0) {
		_mstPosXmin = x;
		_mstPosXmax = 255 + x;
	} else {
		_mstPosXmin = -x;
		_mstPosXmax = 255 - x;
	}
	int y = _gameMstScreenRefPosY;
	if (y > 191) {
		y = 191;
	}
	if (_gameMstScreenRefPosY < 0) {
		_mstPosYmin = y;
		_mstPosYmax = 191 + y;
	} else {
		_mstPosYmin = -y;
		_mstPosYmax = 191 - y;
	}
	printf("runTask_main_op54() skipping call to game_unk47\n");
//	game_unk47();
	if (_esi->count2 == 0) {
#if 0
.text:0041E30F                 mov     eax, [esi+0] // MstUnk48
.text:0041E311                 mov     ecx, [eax]
.text:0041E313                 mov     al, [ecx+4]
		_ecx = READ_LE_UINT32(_resMstUnk43 + _esi->offset1);

		MstUnk48 *m48 = getMstUnk48FromMstUnk43(m, 0);
		_al = m48->unk4;

		_al = _ecx[4];
		if (_al == 0) {
			if (game_unk59(_ecx, 0) != 0) {
				_edx = 0;
				goto loc_41E365;
			}
			goto loc_41E36E;
		} else {
			_edi = _rnd.update() & 1;
.text:0041E330                 mov     eax, [esi]
.text:0041E335                 mov     ecx, [eax]
			if (game_unk59(_ecx, _edi) != 0) {
.text:0041E342                 mov     ecx, [esi]
.text:0041E346                 mov     ecx, [ecx]
				if (game_unk62(_ecx, _edi) != 0) {
					goto loc_41E36E;
				}
			}
.text:0041E351                 mov     eax, [esi]
.text:0041E358                 mov     ecx, [eax]
			_edi ^= 1;
			if (game_unk59(_ecx, _edi) != 0) {
				_edx = _edi;
				goto loc_41E365;
			}
			goto loc_41E36E;
		}
loc_41E365:
.text:0041E365                 mov     ecx, [esi]
.text:0041E367                 mov     ecx, [ecx]
.text:0041E369                 call    game_unk62
loc_41E36E:
#endif
		if (_gameMstUnk6 == -1) {
			++_mstOp54Counter;
		}
		if (_mstOp54Counter <= 16) {
			return;
		}
		_mstOp54Counter = 0;
		if (_esi->count2 != 0) {
			shuffleMstUnk43(_esi);
		}
		return;
	}
	int var4 = 0;
	memset(_mstOp54Table, 0, sizeof(_mstOp54Table));
	int i = 0; // _ebp
	if (_esi->count2 > 0) {
		do {
			uint8_t *t = _resMstUnk43 + _esi->offset2 + i;
			if ((*t & 0x80) != 0) {
				continue;
			}
			var4 = 1;
			int index = *t & 0x7F;
			if (_mstOp54Table[index] != 0) {
				continue;
			}
			_mstOp54Table[index] = 1;
			printf("runTask_main_op54() _esi->count2 %d i %d index %d\n", (int)_esi->count2, i, index);

#if 0
.text:0041E412                 mov     ecx, [esi]
.text:0041E414                 mov     ecx, [ecx+ebx*4]
.text:0041E417                 mov     al, [ecx+4]
			MstUnk48 *m48 = getMstUnk48FromMstUnk43(m, _ebx);
			_al = m48->unk4;
			if (_al == 0) {
				if (game_unk59(_ecx, 0) == 0) {
					continue;
				}
.text:0041E429                 mov     eax, [esi]
.text:0041E42D                 mov     ecx, [eax+ebx*4]
				if (game_unk62(_ecx, 0) == 0) {
					continue;
				}
				break;
			} else {
				_edi = _rnd.update() & 1;
.text:0041E440                 mov     ecx, [esi]
.text:0041E447                 mov     ecx, [ecx+ebx*4]
				if (game_unk59(_ecx, _edi) != 0) {
					if (game_unk62(_ecx, _edi) != 0)
						break;
					}
				}
				_edi ^= 1;
.text:0041E465                 mov     ecx, [esi]
.text:0041E46C                 mov     ecx, [ecx+ebx*4]
				if (game_unk59(_ecx, _edi) != 0) {
					if (game_unk62(_ecx, _edi) != 0) {
						break;
					}
				}
#endif
		} while (++i < (int)_esi->count2);
	}
//loc_41E494:
	if (_gameMstUnk6 != -1) {
		_resMstUnk43[_esi->offset2 + i] |= 0x80;
	} else {
		if (var4) {
			++_mstOp54Counter;
			if (_mstOp54Counter <= 16) {
				return;
			}
		}
		_mstOp54Counter = 0;
		if (_esi->count2 != 0) {
			shuffleMstUnk43(_esi);
		}
	}
}

int Game::runTask_main_op56(Task *t, int num, int index) {
	switch (num) {
	case 12: {
			const int type1 = READ_LE_UINT32(_resMstOpcodeData + index * 16 + 12);
			const int index1 = READ_LE_UINT32(_resMstOpcodeData + index * 16);
			int num1 = getTaskVar(t, (type1 >> 4) & 0xF, index1);
			const int type2 = READ_LE_UINT32(_resMstOpcodeData + index * 16 + 12);
			const int index2 = READ_LE_UINT32(_resMstOpcodeData + index * 16 + 4);
			int num2 = getTaskVar(t, (type2 >> 4) & 0xF, index2);
			displayHintScreen(num1, num2);
		}
		break;
	default:
		printf("runTask_main_op56() unhandled opcode %d\n", num);
//		assert(0);
		break;
	}
	return 0;
}

int Game::runTask_waitDelayInput(Task *t) {
	--t->delay;
	if (t->delay == 0) {
		t->run = &Game::runTask_main;
		printf("runTask_waitDelayInput unhandled task dataPtr\n");
#if 0
		LvlObject *ptr = 0;
		if ((p = t->dataPtr) != 0) {
			ptr = p->0x10(le32);
		} else {
			GameMstUnk2 *_ecx = t->unk10;
			if (!_ecx) {
				return 0;
			}
			ptr = _ecx->o;
		}
		if (ptr) {
			ptr->directionKeyMask = 0;
			ptr->actionKeyMask = 0;
		}
#endif
		return 0;
	}
	return 1;
}

int Game::runTask_waitDelay(Task *t) {
	--t->delay;
	if (t->delay == 0) {
		t->run = &Game::runTask_main;
		return 0;
	}
	return 1;
}

int Game::runTask_mstDelay(Task *t) {
	const int i = t->unk0x38;
	switch (t->delay) {
	case 1:
		if (i == 0) {
			return 1;
		}
		break;
	case 2:
		if ((t->flags & (1 << i)) == 0) {
			return 1;
		}
		break;
	case 3:
		if ((_mstGlobalFlags & (1 << i)) == 0) {
			return 1;
		}
		break;
	case 4:
		if (getAndyTaskVar(i, t) == 0) {
			return 1;
		}
		break;
	case 5:
		// unk10,dataPtr
		return 1;
	default:
		return 1;
	}
//loc_410A41:
	t->run = &Game::runTask_main;
//	if (t->dataPtr) {
//	} else if (t->unk10) {
//	}
	return 0;
}

int Game::runTask_idle(Task *t) {
	return 1;
}

int Game::getAndyTaskVar(int num, Task *t) {
	if ((num & 0x80) != 0) {
		num &= 0x7F;
//		_eax = (1 << num) & _gameMstEq0;
		return 0;
	}
	switch (num) {
	default:
		printf("Game::getAndyTaskVar() num %d\n", num);
		assert(0);
	}
	return 0;
}

MstScreenAreaCode *Game::findMstCodeForPos(int num, int xPos, int yPos) {
	MstScreenAreaCode *p = PTR<MstScreenAreaCode>(_resMstUnk38, _resMstUnk40, num);
	for (; p; p = (MstScreenAreaCode *)PTR_OFFS<MstScreenAreaCode>(_resMstUnk38, FROM_LE32(p->next))) {
		if (p->x1 <= xPos && p->x2 >= xPos && p->unk0x1D != 0 && p->y1 <= yPos && p->y2 >= yPos) {
			return p;
		}
	}
	return 0;
}

void Game::flagMstCodeForPos(int num, uint8_t value) {
	MstScreenAreaCode *p = PTR<MstScreenAreaCode>(_resMstUnk38, _resMstUnk39, num);
	for (; p; p = (MstScreenAreaCode *)PTR_OFFS<MstScreenAreaCode>(_resMstUnk38, FROM_LE32(p->unk0x18))) {
		p->unk0x1D = value;
	}
}

void Game::executeMstCodeHelper2() {
	GameMstUpdateMovingStateFromMainObject();
	updateMstHeightMapData();
//	for (Task *t = _gameMstResToLoad2Pri; t = t->prev) {
//		GameMstUnk3();
//	}
}

int Game::executeMstCodeHelper3(Task *t) {
	// TODO
	return 1;
}

int Game::executeMstCodeHelper4(Task *t) {
	// TODO
	return 1;
}

void Game::executeMstCode() {
	_andyActionKeyMaskAnd = 0xFF;
	_andyDirectionKeyMaskAnd = 0xFF;
	_andyActionKeyMaskOr = 0;
	_andyDirectionKeyMaskOr = 0;
	if (_mstLogicDisabled) {
		return;
	}
	++_executeMstLogicCounter;
	if ((_gameMstLogicHelper1TestValue & _gameMstLogicHelper1TestMask) != 0) {
assert(0); // code path check
//		_gameMstUnk34 = 0;
//		executeMstCodeHelper1();
		_gameMstLogicHelper1TestValue = 0;
	}
	for (int i = 0; i < 8; ++i) {
		_gameMstMovingState[i].unk0x28 = 0;
		_gameMstMovingState[i].unk0x30 = 0;
		_gameMstMovingState[i].unk0x3C = 0x1000000;
	}
	executeMstCodeHelper2();
	if (_globalVars[31] > 0) {
		--_globalVars[31];
		if (_globalVars[31] == 0) {
			Task *t = createTask(PTR_OFFS<uint32>(_resMstUnk62, _resMstUnk37->codeData));
			if (t) {
				t->runningState = 1;
			}
		}
	}
	MstScreenAreaCode *ac;
	while ((ac = findMstCodeForPos(_gameResData0x2E08, _gameMstMovingStatePosX, _gameMstMovingStatePosY)) != 0) {
		flagMstCodeForPos(ac->unk0x1C, 0);
		createTask(_resMstUnk62 + ac->codeData * 4);
	}
	if (_gameCurrentData0x2E08Num != _gameResData0x2E08) {
		_gameCurrentData0x2E08Num = _gameResData0x2E08;
	}
	for (Task *t = _tasksListTail; t; t = t->prev) {
		_runTaskOpcodesCount = 0;
		while ((this->*(t->run))(t) == 0);
	}
	for (int i = 0; i < _gameMstMovingStateCount; ++i) {
//		_eax = &_gameMstMovingState[i].field_30;
		if (_gameMstMovingState[i].unk0x30 == 0) {
			continue;
		}
		assert(0); // code path check
#if 0
		_ecx = *_eax;
		_esi = _gameMstMovingState[i].field_30-0x8(le32);
		_edx = _ecx+0x44(le32);
		if (_esi != _edx) {
			if (_edx > 0) {
				if (_eax->0x10(byte) == 2) {
					_edx -= 4;
					_ecx->0x44(le32) = _edx;
					_ecx = *_eax(le32);
					if (_ecx->0x44(le32) < 0) {
						_ecx->0x44(le32) = 0;
					}
				} else {
					--_edx;
					_ecx->0x44(le32) = _edx;
				}
				_ecx = _eax-0x8(le32)
				_ecx[3] = 0x80;
				_ecx->0x0C(le32) = READ_LE_UINT16(_eax + 4);
				_ecx->0x10(le32) = READ_LE_UINT16(_eax + 8);
				_edx = *_eax(le32);
				_ecx->0x1C(le32) = READ_LE_UINT32(_edx + 0x10);
			} else if (_edx == -2) {
				_ecx[3] = 0x80;
				_ecx->0x0C(le32) = READ_LE_UINT16(_eax + 4);
				_ecx->0x10(le32) = READ_LE_UINT16(_eax + 8);
			}
		} else {
			if (_edx > 0) {
				--_edx;
				_ecx->0x44(le32) = _edx;
				_plasmaCannonLastIndex1 = _eax->0x11(byte);
			} else if (_edx == -2) {
				_plasmaCannonLastIndex1 = _eax->0x11(byte);
			} else {
				_plasmaCannonLastIndex1 = 0;
			}
		}
#endif
	}
	for (Task *t = _gameMstResToLoad2Pri; t; t = t->prev) {
		_runTaskOpcodesCount = 0;
		if (executeMstCodeHelper3(t) == 0) {
			while ((this->*(t->run))(t) == 0);
		}
	}
	for (Task *t = _gameMstResToLoad1Pri; t; t = t->prev) {
		_runTaskOpcodesCount = 0;
		if (executeMstCodeHelper4(t) == 0) {
			while ((this->*(t->run))(t) == 0);
		}
	}
}

void Game::prependTaskToList(Task **tasksList, Task *t) {
	Task *current = *tasksList;
	if (!current) {
		current->prev = current->next = 0;
	} else {
		Task *prev = current->prev;
		if (prev) {
			do {
				current = prev;
				prev = current->prev;
			} while (prev);
		}
		current->prev = t;
		t->prev = 0;
		t->next = current;
	}
}

int Game::game_unk62(MstUnk48 *m, uint8_t num) {
//	_ebp = m;
//	_ebx = num;
	m->unk5 = num;
	const uint8_t *codeData = PTR_OFFS<uint32>(_resMstUnk62, m->codeData);
	if (codeData) {
		Task *t = createTask(codeData); // _esi
		if (!t) {
			return 0;
		}
		while ((this->*(t->run))(t) == 0);
	}
#if 0
	_gameMstUnk6 = m - _resMstUnk48; // index
	dword_464AB4 = 0;
//	var4 = var8 = 0;
	for (int i = 0; i < m->countUnk12; ++i) { // var4 < _eax ; ++var4; var8 += 12;
.text:0041D8B1                 mov     edx, [ebp+0Ch]
.text:0041D8B4                 mov     eax, [esp+18h+var_8]
.text:0041D8B8                 mov     edi, [edx+eax+4]
.text:0041D8BC                 mov     al, [edi+1Bh]
.text:0041D8BF                 cmp     al, 0FFh
.text:0041D8C1                 jz      loc_41D949
.text:0041D8C7                 and     eax, 0FFh
.text:0041D8CC                 mov     [edi+19h], bl
.text:0041D8CF                 shl     eax, 8
.text:0041D8D2                 add     eax, offset _gameMstUnkTable1
.text:0041D8D7                 mov     esi, eax
.text:0041D8D9                 mov     cl, [esi+48h]
.text:0041D8DC                 mov     [esi+18h], edi
.text:0041D8DF                 or      cl, 40h
.text:0041D8E2                 mov     [esi+48h], cl
.text:0041D8E5                 mov     cl, [esi+0A5h]
.text:0041D8EB                 and     cl, 8Ah
.text:0041D8EE                 or      cl, 0Ah
.text:0041D8F1                 mov     [esi+0A5h], cl
.text:0041D8F7                 mov     ecx, esi
.text:0041D8F9                 call    game_unk65
.text:0041D8FE                 mov     eax, _gameMstResToLoad2Pri
.text:0041D903                 mov     esi, [esi+0C4h]
.text:0041D909                 test    eax, eax
.text:0041D90B                 jz      short loc_41D939
.text:0041D90D                 mov     ecx, [eax+8]
.text:0041D910                 cmp     eax, esi
.text:0041D912                 jz      short loc_41D921
.text:0041D914 loc_41D914:
.text:0041D914                 test    ecx, ecx
.text:0041D916                 mov     eax, ecx
.text:0041D918                 jz      short loc_41D939
.text:0041D91A                 mov     ecx, [ecx+8]
.text:0041D91D                 cmp     eax, esi
.text:0041D91F                 jnz     short loc_41D914
.text:0041D921 loc_41D921:

		removeTask(&_gameMstResToLoad2Pri, _esi);
		prependTaskToList(&_gameMstResToLoad2Pri, _esi);

.text:0041D939 loc_41D939:
.text:0041D939                 mov     edx, [edi+10h]
.text:0041D93C                 mov     ecx, esi
.text:0041D93E                 call    resetTask

		++dword_464AB4;

.text:0041D949 loc_41D949:

	}
#endif
	return 1;
}

