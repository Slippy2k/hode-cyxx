
#include "game.h"
#include "resource.h"
#include "util.h"

void Game::removeSoundObject(SssObject *so) {
	so->soundBits = 0;
	if ((so->flags & 1) != 0) {
		SssObject *next = so->nextPtr;
		SssObject *prev = so->prevPtr;
		if (next) {
			next->prevPtr = prev;
		}
		if (prev) {
			prev->nextPtr = next;
		} else {
			_sssObjectsList1 = next;
		}

		--_snd_fadeVolumeCounter;
		if (_sssObjectsList3 != so) {
			if (_snd_fadeVolumeCounter >= _snd_volumeMin) {
				return;
			}
			if (_sssObjectsList3 == 0) {
				return;
			}
		}
		_sssObjectsList3 = 0;
		if (_snd_fadeVolumeCounter < _snd_volumeMin) {
			return;
		}
		SssObject *current = _sssObjectsList1;
		if (current != 0) {
			SssObject *_ecx = 0;
			do {
				if (_ecx && _ecx->unk9 > current->unk9) {
					_ecx = current;
					_sssObjectsList3 = _ecx;
				}
			} while ((current = current->nextPtr) != 0);
		}
		return;
	}
	SssObject *next = so->nextPtr;
	SssObject *prev = so->prevPtr;
	if (next) {
		next->prevPtr = prev;
	}
	if (prev) {
		prev->nextPtr = next;
	} else {
		_sssObjectsList2 = next;
	}
}

void Game::updateSoundObject(SssObject *so) {
#if 0
	_snd_usedFlagTable[so->num] = 1;
	_sssObjectsChanged = so->sssUnk1Ptr[0x30];
	if ((so->flags & 4) == 0) {
loc_42B179:
		if (so->soundBits == 0) {
			if (so->codeDataStage1) {
				so->codeDataStage1 = executeSssCode(so);
			}
			if (so->soundBits == 0) {
				return;
			}
			if (so->codeDataStage4) {
				so->codeDataStage4 = executeSssCode(so);
			}
			if (so->soundBits == 0) {
				return;
			}
			goto flag_case0;
		} else {
42B1B9:
			if (so->codeDataStage1) {
				so->codeDataStage1 = executeSssCode(so);
			}
			if (so->soundBits == 0) {
				return;
			}
			if (so->volumePtr) {
				const int volume = getSoundObjectVolumeByPos(so);
				if (volume != so->volume) {
					so->volume = volume;
					_sssObjectsChanged = 1;
				}
			} else {
				if (so->codeDataStage2) {
					so->codeDataStage2 = executeSssCode(so);
				}
			}
			if (so->soundBits == 0) {
				return;
			}
			if (so->codeDataStage3) {
				so->codeDataStage3 = executeSssCode(so);
			}
			if (so->codeDataStage4) {
				so->codeDataStage4 = executeSssCode(so);
			}
			if (so->soundBits == 0) {
				return;
			}
			if (_sssObjectsChanged == 0 || (sss->flags & 1) == 0) {
				goto flag_case0;
			}
			goto flag_case1;
	} else if ((so->flags & 1) == 0) {
		goto flag_case0;
	} else if (so->volumePtr) {
		const int volume = getSoundObjectVolumeByPos(so);
		if (volume != so->volume) {
			so->volume = volume;
			_sssObjectsChanged = 1;
			goto flag_case1;
		}
	}
	if (_sssObjectsChanged) {
goto flag_case1:
	} else {
goto flag_case0:
	}

flag_case1:
	updateSoundVolume(so);
flag_case0:
	if (so->unk2C == 0) {
		var4 = ecx = so->unk7C;
		ebp = so->unk78;
		edi = so->flags0;
		removeSoundObject(so);
		if (ebp != -1) {
			LvlObject *tmp = _currentSoundLvlObject;
			_currentSoundLvlObject = so->lvlObject;
			prepareSoundObject(ebp, var4);
			_currentSoundLvlObject = tmp;
		} else {
loc_42B29C:
		_eax = (_edi >> 0x14) & 0xF;
		_ebp = (_edi & 0xFFF);

		_ecx = _sssFlagsTable[_eax * 4];
		_edx = _ecx + _ebp * 4;

		_ecx = _edi >> 0x18;

		_esi = *(uint32_t *)_edx:
		_edi = _ebx >> (_ecx & 255);
		var4 = _edx;

		if ((_esi & _edi) == 0) {
			return;
		}
		_edx = _sssObjectsList1;
		while (_edx) {
			_esi = _edx->flags0;
			_ebx = _esi & 0xFFF;
			if (_ebx == _ebp) {
				_ebx = (_esi >> 0x14) & 0xF;
				if (_ebx == _eax) {
					_esi >>= 0x18;
					if (_esi == _ecx) {
						goto 42B343;
					}
				}
			}
			_edx = _edx->nextPtr;
		}
		_edx = _sssObjectsList2;
		while (_edx) {
			_esi = _edx->flags0;
			_ebx = _esi & 0xFFF;
			if (_ebx == _ebp) {
				_ebx = (_esi >> 0x14) & 0xF;
				if (_ebx == _eax) {
					_esi >>= 0x18;
					if (_esi == _ecx) {
						goto 42B343;
					}
				}
			}
			_edx = _edx->nextPtr;
		}
		*(uint32_t *)eax = (*(uint32_t *)var4) & ~_edi;
		return;
loc_42B343:
		if (_edx) {
			*(uint32_t *)eax = (*(uint32_t *)var4) & ~_edi;
		}
		return;
	}
loc_42B357:
	_esi->unk2C = _eax - 1;
	if ((_esi->flags & 2) == 0) {
		++_esi->unk6C;
	}
#endif
}

const uint8_t *Game::executeSoundCode(SssObject *so, const uint8_t *code) {
	while (1) {
		switch (*code) {
		case 0:
			return 0;
		case 2:
			if (so->unk50 >= -1) {
				LvlObject *tmp = _currentSoundLvlObject;
				_currentSoundLvlObject = so->lvlObject;
				prepareSoundObject(READ_LE_UINT16(code + 2), code[1], so->flags0);
				_currentSoundLvlObject = tmp;
				code += 4;
				if (so->soundBits == 0) {
					return code;
				}
			}
			break;
		case 4:
			// _cl = code[1] & 0xF;
			// _eax = so->flags0 & 0xFF00F000;
			// _edx = (so->flags0 ^ _ebx) & 0x00F00000;
			// _ebx ^= _edx;
			// _dx = READ_LE_UINT16(code + 2);
			// _ebx &= 0xF00000;
			// _edx &= 0xFFF;
			// _eax ^= _ebx;
			// _ecx <<= 0x10;
			// _eax |= _ecx;
			// _eax |= _eax;
			// executeSssCodeOp4(_ecx, _edx, _ebx);
			code += 4;
			break;
		case 5: {
				int32_t _eax = READ_LE_UINT32(code + 4);
				if (so->unk6C < _eax) {
					so->unk6C = _eax;
					if (so->soundBits) {
						_eax = READ_LE_UINT32(code);
						if (_eax != 0) {
							so->unk28 = _eax + READ_LE_UINT32(code + 8);
						}
					}
				}
				code += 12;
			}
			break;
		case 6: {
				--so->counter;
				if (so->counter < 0) {
					code += 8;
				} else {
					uint32_t offset = READ_LE_UINT32(code + 4);
					code -= offset;
				}
			}
			break;
		case 8: {
				int _eax = READ_LE_UINT32(code + 12);
				if (so->unk6C <= _eax) {
					return code;
				}
				--so->unk4C;
				if (so->unk4C < 0) {
					code += 16;
				} else {
					so->unk6C = READ_LE_UINT32(code + 8);
					// TODO: 42AF44
				}
			}
			break;
		case 9: {
				so->unk64 += so->unk68;
				const int volume = (so->unk64 + 0x8000) >> 16;
				if (volume != so->volume) {
					so->volume = volume;
					// _sssObjectsChanged = 1;
				}
				--so->unk58;
				if (so->unk58 >= 0) {
					return code;
				}
				code += 4;
			}
		case 10: {
				// TODO:
			}
			break;
		case 11: {
				if (so->unk18 != code[1]) {
					so->unk18 = code[1];
					// _sssObjectsChanged = 1;
				}
			}
			break;
		case 12: {
				// uint32_t _edx = so->flags1;
				// uint32_t _eax = _edx << 0x18;
				// _edx = (_edx >> 0x14) & 0xF;
				// uint16_t _ecx = READ_LE_UINT16(code + 2);
				// executeSssCodeOp12(_ecx, _edx, _eax);
				code += 4;
			}
			break;
		case 13: {
				// TODO:
			}
			break;
		case 14: {
				// TODO:
			}
			break;
		case 16: {
				--so->unk4C;
				if (so->unk4C >= 0) {
					return code;
				}
				// executeSssCodeOp16(so);
				code += 4;
				if (so->soundBits == 0) {
					return code;
				}
				// _sssObjectsChanged = 1;
			}
			break;
		case 17: {
				// executeSssCodeOp17(so);
				so->unk4C = READ_LE_UINT32(code + 4);
				return code + 8;
			}
			break;
		case 18: {
				if (so->counter < 0) {
					so->counter = READ_LE_UINT32(code + 4);
				}
				code += 8;
			}
			break;
		case 19: {
				if (so->volume != code[1]) {
					so->volume = code[1];
					// _sssObjectsChanged = 1;
				}
			}
			break;
		case 20: {
				so->unk4C = READ_LE_UINT16(code + 2);
				code += 4;
			}
			break;
		case 21: {
				--so->unk50;
				if (so->unk50 >= 0) {
					return code;
				}
				code += 4;
			}
			break;
		case 22: {
				so->unk50 = READ_LE_UINT32(code + 4);
				return code + 8;
			}
			break;
		case 23: {
				--so->unk54;
				if (so->unk54 >= 0) {
					return code;
				}
				code += 4;
			}
			break;
		case 24: {
				so->unk54 = READ_LE_UINT32(code + 4);
				return code + 8;
			}
		case 25: {
				--so->unk58;
				if (so->unk58 >= 0) {
					return code;
				}
				code += 4;
			}
			break;
		case 26: {
				so->unk58 = READ_LE_UINT32(code + 4);
				return code + 8;
			}
			break;
		case 27: {
				int _eax = READ_LE_UINT32(code + 12);
				if (so->unk6C <= _eax) {
					return code;
				}
				so->unk6C = _eax;
				// TODO: goto 42AF44
			}
			break;
		case 28: {
				uint32_t offset = READ_LE_UINT32(code + 4);
				code -= offset;
			}
			break;
		default:
			error("Invalid .sss opcode %d", *code);
			break;
		}
	}
	return code;
}

void Game::prepareSoundObject(int num, int b, int c) {
	if (b > 0) {
		if ((_res->_sssDataUnk3[num].unk1 & 1) != 0) {
			// int var8 = _res->_sssDataUnk3[num].sssUnk4
			if (_res->_sssDataUnk3[num].unk0 <= 0) {
				return; // 0;
			}
// 42B81D
		}
// 42B865:
		// TODO:
	} else {
		SssObject *so = startSoundObject(num, b, c);
		if (so && (_res->_sssDataUnk3[num].unk0 & 4) != 0) {
			so->unk7C = b;
			so->unk78 = num;
		}
	}
}

SssObject *Game::startSoundObject(int a, int b, int c) {
	// TODO
	return 0;
}

void Game::setupSound(SssUnk1 *s, int a, int b) {
	debug(kDebug_SOUND, "setupSound num %d a 0x%x b 0x%x", s->unk0, a, b);
	const int num = _res->_sssDataUnk3[s->unk0].sssUnk4;
	debug(kDebug_SOUND, "sssUnk4 num %d", num);
	SssUnk4 *sssUnk4Ptr = &_res->_sssDataUnk4[num];
	bool found = false;
	for (int i = 0; i < _sssObjectsCount; ++i) {
		SssObject *so = &_sssObjectsTable[i];
		if (so->soundBits != 0 && so->sssUnk4Ptr == sssUnk4Ptr) {
			found = true;
			break;
		}
	}
	const int _ecx = READ_LE_UINT32(sssUnk4Ptr->data + 4);
	const int _eax = (s->unk3 << 16);
	if (_ecx != _eax) {
		if (!found) {
			// (sssUnk4Ptr->data + 4) = (s->unk3 << 16); // int32_t*
		} else {
			// sssUnk4Ptr->data + 0xC = 4; // uint32_t
			// sssUnk4Ptr->data + 0x30 = 1; // uint32_t
			// eax = (s->unk3 << 16) - _ecx
			// cdq; edx &= 3; eax += _edx; eax >>= 2;
			// ssUnk4Ptr->data + 8 = _eax; // uint32_t
		}
	}
// 42B9FD

// 42BBDD
	int _ebp = 0; // TODO
	prepareSoundObject(s->unk0, (int8_t)s->unk2, _ebp);
}

void Game::clearSoundObjects() {
	memset(_sssObjectsTable, 0, sizeof(_sssObjectsTable));
	_sssObjectsList1 = 0;
	_sssObjectsList2 = 0;
	_sssObjectsList3 = 0;
	for (size_t i = 0; i < ARRAYSIZE(_sssObjectsTable); ++i) {
		_sssObjectsTable[i].num = i;
	}
	_sssObjectsCount = 0;
	_snd_fadeVolumeCounter = 0;
	// _snd_mixingBufferSize = 0;
	if (_res->_sssHdr.unk10 != 0) {
		for (int i = 0; i < 3; ++i) {
			// memset(_sssLookupTable1[i], 0, _sssHdr.unk18 * 4);
			// memset(_sssObjectsTable[i].soundBits, 0, _sssHdr.unk18 * 4);
			// memset(_sssLookupTable3[i], 0, _sssHdr.unk18 * 4);
		}
	}
	// TODO:
}

void Game::fadeSoundObject(SssObject *so) {
	if ((so->flags & 2) == 0) {
		_sssObjectsList3 = 0;
		if (_snd_fadeVolumeCounter >= _snd_volumeMin) {
			so = 0;
			SssObject *cur = _sssObjectsList1;
			for (; cur; cur = cur->nextPtr) {
				if (so) {
					if (so->unk9 <= cur->unk9) {
						continue;
					}
				}
				so = cur;
				_sssObjectsList2 = cur;
			}
		}
	}
}
