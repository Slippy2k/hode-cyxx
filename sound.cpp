
#include "game.h"
#include "resource.h"
#include "util.h"

void Game::removeSoundObject(SssObject *so) {
	so->unk0 = 0;
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
	_sndVolumeChanged = so->sssUnk1Ptr[0x30];
	if ((so->flags & 4) == 0) {
loc_42B179:
		if (so->unk0 == 0) {
			if (so->codeDataStage1) {
				so->codeDataStage1 = executeSssCode(so);
			}
			if (so->unk0 == 0) {
				return;
			}
			if (so->codeDataStage4) {
				so->codeDataStage4 = executeSssCode(so);
			}
			if (so->unk0 == 0) {
				return;
			}
			goto flag_case0;
		} else {
42B1B9:
			if (so->codeDataStage1) {
				so->codeDataStage1 = executeSssCode(so);
			}
			if (so->unk0 == 0) {
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
			if (so->unk0 == 0) {
				return;
			}
			if (so->codeDataStage3) {
				so->codeDataStage3 = executeSssCode(so);
			}
			if (so->codeDataStage4) {
				so->codeDataStage4 = executeSssCode(so);
			}
			if (so->unk0 == 0) {
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
			_ebx = _currentSoundObject;
			_currentSoundObject = so->soundDataPtr;
			prepareSound(ebp, var4);
			_currentSoundObject = _ebx;
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
				// _ebp = currentSoundObject;
				// _currentSoundObject = so->soundDataPtr;
				prepareSound(READ_LE_UINT16(code + 2), code[1], so->flags0);
				// _currentSoundObject = _ebp;
				code += 4;
				if (so->unk0 == 0) {
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
					if (so->unk0) {
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
				if (so->unk0 == 0) {
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

void Game::prepareSound(int a, int b, int c) {
}
