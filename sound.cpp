
#include "game.h"
#include "resource.h"
#include "systemstub.h"
#include "util.h"

enum {
	kFlagPlaying   = 1 << 0,
	kFlagIdle      = 1 << 1, // no PCM
	kFlagNoTrigger = 1 << 2, // no bytecode
};

static const bool kLimitSounds = false; // limit the number of active playing sounds

static const uint8_t _dbVolumeTable[129] = {
	0x00, 0x01, 0x01, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x05, 0x05, 0x06, 0x06,
	0x06, 0x07, 0x07, 0x07, 0x08, 0x08, 0x09, 0x09, 0x09, 0x0A, 0x0A, 0x0B, 0x0B, 0x0C, 0x0C, 0x0C,
	0x0D, 0x0D, 0x0E, 0x0E, 0x0F, 0x0F, 0x10, 0x10, 0x10, 0x11, 0x11, 0x12, 0x12, 0x13, 0x13, 0x14,
	0x14, 0x15, 0x15, 0x16, 0x16, 0x17, 0x17, 0x18, 0x18, 0x19, 0x1A, 0x1A, 0x1B, 0x1B, 0x1C, 0x1C,
	0x1D, 0x1D, 0x1E, 0x1F, 0x1F, 0x20, 0x20, 0x21, 0x22, 0x22, 0x23, 0x23, 0x24, 0x25, 0x25, 0x26,
	0x27, 0x27, 0x28, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2F, 0x30, 0x32, 0x33, 0x35, 0x37, 0x38,
	0x3A, 0x3C, 0x3D, 0x3F, 0x41, 0x43, 0x44, 0x46, 0x48, 0x4A, 0x4C, 0x4E, 0x50, 0x52, 0x54, 0x56,
	0x59, 0x5B, 0x5D, 0x5F, 0x62, 0x64, 0x66, 0x69, 0x6B, 0x6E, 0x70, 0x73, 0x76, 0x78, 0x7B, 0x7E,
	0x80
};

static uint32_t valueSssLut(uint8_t source, uint8_t mask, SssInfo *s) {
	uint32_t value = 0;
	assert(source < 3); // 0,1,2
// 42BA9D

	// bits 20..24
	value = source;
	// bits 24..32
	value |= mask << 4;

	// bits 16..20
	value <<= 4;
	value |= s->unk2 & 15;

	// bits 12..16
	value <<= 4;
	value |= s->unk6;

	// bits 0..12
	value <<= 12;
	value |= (s->sssBankIndex & 0xFFF);

	return value;
}

static bool compareSssLut(uint32_t flags_a, uint32_t flags_b) {
	// (flags_a & 0xFFF00FFF) == (flags_b & 0xFFF00FFF) ?
	if (((flags_a >> 20) & 15) == ((flags_b >> 20) & 15)) { // lut index : 0,1,2 (Andy, monster1, monster2)
		if ((flags_a & 0xFFF) == (flags_b & 0xFFF)) { // lut offset : num _sssBankIndex
			return (flags_a >> 24) == (flags_b >> 24); // bit 0..31, used as a mask lut[][] |= 1 << bit
		}
	}
	return false;
}

static uint32_t *getSssLutPtr(Resource *res, int lut, uint32_t flags) {
	const uint32_t a = (flags >> 20) & 0xF; // 0,1,2
	assert(a < 3);
	const uint32_t b = flags & 0xFFF; // num indexes _sssBankIndex
	assert(b < (uint32_t)res->_sssHdr.dataUnk3Count);
	switch (lut) {
	case 1:
		return &res->_sssLookupTable1[a][b];
	case 2:
		return &res->_sssLookupTable2[a][b];
	case 3:
		return &res->_sssLookupTable3[a][b];
	default:
		error("Invalid sssLut %d", lut);
	}
	return 0;
}

void Game::resetSound() {
	MixerLock ml(&_mix);
	// TODO
	clearSoundObjects();
}

static SssObject *findLowestPrioritySssObject(SssObject *sssObjectsList) {
	SssObject *ret = 0;
	for (SssObject *current = sssObjectsList; current; current = current->nextPtr) {
		if (!ret || ret->currentPriority > current->currentPriority) {
			ret = current;
		}
	}
	return ret;
}

void Game::removeSoundObjectFromList(SssObject *so) {
	so->pcm = 0;
	if ((so->flags & 1) != 0) {

		// remove from linked list1
		SssObject *next = so->nextPtr;
		SssObject *prev = so->prevPtr;
		if (next) {
			next->prevPtr = prev;
		}
		if (prev) {
			prev->nextPtr = next;
		} else {
			assert(so == _sssObjectsList1);
			_sssObjectsList1 = next;
		}
		--_playingSssObjectsCount;

		if (kLimitSounds) {
			// update the least important soundObject
			if (_lowPrioritySssObject == so || (_playingSssObjectsCount < _playingSssObjectsMax && _lowPrioritySssObject)) {
				_lowPrioritySssObject = 0;
				if (_playingSssObjectsCount >= _playingSssObjectsMax && _sssObjectsList1) {
					_lowPrioritySssObject = findLowestPrioritySssObject(_sssObjectsList1);
				}
			}
		}
	} else {

		// remove from linked list2
		SssObject *next = so->nextPtr;
		SssObject *prev = so->prevPtr;
		if (next) {
			next->prevPtr = prev;
		}
		if (prev) {
			prev->nextPtr = next;
		} else {
			assert(so == _sssObjectsList2);
			_sssObjectsList2 = next;
		}
	}
}

void Game::updateSoundObject(SssObject *so) {
	_sssUpdatedObjectsTable[so->num] = true;
	_sssObjectsChanged = so->filter->unk30;
	if ((so->flags & 4) == 0) {
//42B179:
		if (so->pcm == 0) {
			if (so->codeDataStage1) {
				so->codeDataStage1 = executeSssCode(so, so->codeDataStage1);
			}
			if (so->pcm == 0) {
				return;
			}
			if (so->codeDataStage4) {
				so->codeDataStage4 = executeSssCode(so, so->codeDataStage4);
			}
			if (so->pcm == 0) {
				return;
			}
		} else {
//42B1B9:
			if (so->codeDataStage1) {
				so->codeDataStage1 = executeSssCode(so, so->codeDataStage1);
			}
			if (so->pcm == 0) {
				return;
			}
			if (so->panningPtr) {
				const int panning = getSoundObjectPanning(so);
				if (panning != so->panning) {
					so->panning = panning;
					_sssObjectsChanged = true;
				}
			} else {
				if (so->codeDataStage2) {
					so->codeDataStage2 = executeSssCode(so, so->codeDataStage2);
				}
			}
			if (so->pcm == 0) {
				return;
			}
			if (so->codeDataStage3) {
				so->codeDataStage3 = executeSssCode(so, so->codeDataStage3);
			}
			if (so->codeDataStage4) {
				so->codeDataStage4 = executeSssCode(so, so->codeDataStage4);
			}
			if (so->pcm == 0) {
				return;
			}
			if (_sssObjectsChanged && (so->flags & 1) != 0) {
				setSoundObjectPanning(so);
			}
		}
	} else if ((so->flags & 1) != 0) {
		if (so->panningPtr) {
			const int panning = getSoundObjectPanning(so);
			if (panning != so->panning) {
				so->panning = panning;
				_sssObjectsChanged = true;
				setSoundObjectPanning(so);
			}
		} else if (_sssObjectsChanged) {
			setSoundObjectPanning(so);
		}
	}
	if (so->pcmFramesCount != 0) {
		--so->pcmFramesCount;
		if ((so->flags & 2) == 0) {
			++so->currentPcmFrame;
		}
	} else {
		const int _edi = so->flags0;
		removeSoundObjectFromList(so);
		if (so->nextSoundNum != -1) {
			LvlObject *tmp = _currentSoundLvlObject;
			_currentSoundLvlObject = so->lvlObject;
			createSoundObject(so->nextSoundNum, so->nextSoundCounter, _edi);
			_currentSoundLvlObject = tmp;
			return;
		}
		const uint32_t mask = 1 << (_edi >> 24);
		uint32_t *sssLut2 = getSssLutPtr(_res, 2, _edi);
		if ((*sssLut2 & mask) == 0) {
			return;
		}
		for (SssObject *so = _sssObjectsList1; so; so = so->nextPtr) {
			if (compareSssLut(so->flags0, _edi)) {
                                *sssLut2 &= ~mask;
                                return;
			}
		}
		for (SssObject *so = _sssObjectsList2; so; so = so->nextPtr) {
			if (compareSssLut(so->flags0, _edi)) {
                                *sssLut2 &= ~mask;
                                return;
			}
		}
	}
}

void Game::sssOp12_removeSounds2(int num, uint8_t lut, uint8_t c) {
	assert(lut < 3);
	assert(num < _res->_sssHdr.dataUnk3Count);
	assert(c < 32);
	const uint32_t mask = (1 << c);
	_res->_sssLookupTable1[lut][num] &= ~mask;
	for (SssObject *so = _sssObjectsList1; so; so = so->nextPtr) {
		if (so->bankIndex == 0 && ((so->flags1 >> 20) & 15) == lut && (so->flags1 >> 24) == c) {
			so->codeDataStage3 = 0;
			if (so->codeDataStage4 == 0) {
				removeSoundObjectFromList(so);
			}
			so->nextSoundNum = -1;
			so->delayCounter = -2;
		}
	}
	for (SssObject *so = _sssObjectsList2; so; so = so->nextPtr) {
		if (so->bankIndex == 0 && ((so->flags1 >> 20) & 15) == lut && (so->flags1 >> 24) == c) {
			so->codeDataStage3 = 0;
			if (so->codeDataStage4 == 0) {
				removeSoundObjectFromList(so);
			}
			so->nextSoundNum = -1;
			so->delayCounter = -2;
		}
	}
	while (_sssObjectsCount > 0 && _sssObjectsTable[_sssObjectsCount - 1].pcm == 0) {
		--_sssObjectsCount;
	}
}

void Game::sssOp16_resumeSound(SssObject *so) {
	if ((so->flags & 2) != 0) {
		SssObject *next = so->nextPtr;
		SssObject *prev = so->prevPtr;
		SssPcm *pcm = so->pcm;
		so->pcm = 0;
		if (next) {
			next->prevPtr = prev;
		}
		if (prev) {
			prev->nextPtr = next;
		} else {
			assert(so == _sssObjectsList2);
			_sssObjectsList2 = next;
		}
		so->pcm = pcm;
		so->flags &= ~2;
		prependSoundObjectToList(so);
	}
}

void Game::sssOp17_pauseSound(SssObject *so) {
	if ((so->flags & 2) == 0) {
		SssPcm *pcm = so->pcm;
		SssObject *prev = so->prevPtr;
		SssObject *next = so->nextPtr;
		so->pcm = 0;
		if ((so->flags & 1) != 0) {
			if (next) {
				next->prevPtr = prev;
			}
			if (prev) {
				prev->nextPtr = next;
			} else {
				assert(so == _sssObjectsList1);
				_sssObjectsList1 = next;
			}
			--_playingSssObjectsCount;

			if (kLimitSounds) {
				// update the least important sound object
				if (so == _lowPrioritySssObject || (_playingSssObjectsCount < _playingSssObjectsMax && _lowPrioritySssObject)) {
					_lowPrioritySssObject = 0;
					if (_playingSssObjectsCount >= _playingSssObjectsMax && _sssObjectsList1) {
						_lowPrioritySssObject = findLowestPrioritySssObject(_sssObjectsList1);
					}
				}
			}
		} else {
			if (next) {
				next->prevPtr = prev;
			}
			if (prev) {
				prev->nextPtr = next;
			} else {
				assert(so == _sssObjectsList2);
				_sssObjectsList2 = next;
			}
		}
		so->pcm = pcm;
		so->flags = (so->flags & ~1) | 2;
		prependSoundObjectToList(so);
	}
}

void Game::sssOp4_removeSounds(uint32_t flags) {
	const uint32_t mask = 1 << (flags >> 24);
	*getSssLutPtr(_res, 1, flags) &= ~mask;
	for (SssObject *so = _sssObjectsList1; so; so = so->nextPtr) {
		if ((so->flags1 & 0xFFFF0FFF) == 0) {
			so->codeDataStage3 = 0;
			if (so->codeDataStage4 == 0) {
				removeSoundObjectFromList(so);
			}
			so->nextSoundNum = -1;
			so->delayCounter = -2;
		}
	}
	for (SssObject *so = _sssObjectsList2; so; so = so->nextPtr) {
		if ((so->flags1 & 0xFFFF0FFF) == 0) {
			so->codeDataStage3 = 0;
			if (so->codeDataStage4 == 0) {
				removeSoundObjectFromList(so);
			}
			so->nextSoundNum = -1;
			so->delayCounter = -2;
		}
	}
	while (_sssObjectsCount > 0 && _sssObjectsTable[_sssObjectsCount - 1].pcm == 0) {
		--_sssObjectsCount;
	}
}

const uint8_t *Game::executeSssCode(SssObject *so, const uint8_t *code, bool tempSssObject) {
	while (1) {
		debug(kDebug_SOUND, "executeSssCode() code %d", *code);
		switch (*code) {
		case 0:
			return 0;
		case 2: // add_sound
			if (so->delayCounter >= -1) {
				LvlObject *tmp = _currentSoundLvlObject;
				_currentSoundLvlObject = so->lvlObject;
				createSoundObject(READ_LE_UINT16(code + 2), (int8_t)code[1], so->flags0);
				_currentSoundLvlObject = tmp;
			}
			code += 4;
			if (so->pcm == 0) {
				return code;
			}
			break;
		case 4: { // remove_sound
				const uint8_t _cl = code[1] & 0xF;
				const uint16_t num = READ_LE_UINT16(code + 2);
				uint32_t flags = (so->flags0 & 0xFFF0F000);
				flags |= (_cl << 16) | (num & 0xFFF);
				sssOp4_removeSounds(flags);
				code += 4;
			}
			break;
		case 5: { // seek_forward
				const int32_t frame = READ_LE_UINT32(code + 4);
				if (so->currentPcmFrame < frame) {
					so->currentPcmFrame = frame;
					if (so->pcm) {
						const int16_t *ptr = so->pcm->ptr;
						if (ptr) {
							so->currentPcmPtr = ptr + READ_LE_UINT32(code + 8) / sizeof(int16_t);
						}
					}
				}
				code += 12;
			}
			break;
		case 6: { // repeat_jge
				--so->repeatCounter;
				if (so->repeatCounter < 0) {
					code += 8;
				} else {
					const int32_t offset = READ_LE_UINT32(code + 4);
					code -= offset;
				}
			}
			break;
		case 8: { // seek_backward_delay
				const int32_t frame = READ_LE_UINT32(code + 12);
				if (so->currentPcmFrame > frame) {
					--so->pauseCounter;
					if (so->pauseCounter < 0) {
						code += 16;
						break;
					}
					so->currentPcmFrame = READ_LE_UINT32(code + 8);
					if (so->pcm) {
						const int16_t *ptr = so->pcm->ptr;
						if (ptr) {
							so->currentPcmPtr = ptr + READ_LE_UINT32(code + 4) / sizeof(int16_t);
						}
					}
				}
				return code;
			}
			break;
		case 9: { // modulate_panning
				so->panningModulateCurrent += so->panningModulateDelta;
				const int panning = (so->panningModulateCurrent + 0x8000) >> 16;
				if (panning != so->panning) {
					so->panning = panning;
					_sssObjectsChanged = true;
				}
				--so->panningModulateSteps;
				if (so->panningModulateSteps >= 0) {
					return code;
				}
				code += 4;
			}
			break;
		case 10: { // modulate_volume
				if (so->volumeModulateSteps >= 0) {
					so->volumeModulateCurrent += so->volumeModulateDelta;
					const int volume = (so->volumeModulateCurrent + 0x8000) >> 16;
					if (volume != so->volume) {
						so->volume = volume;
						_sssObjectsChanged = true;
					}
					--so->volumeModulateSteps;
					if (so->volumeModulateSteps >= 0) {
						return code;
					}
				}
				code += 4;
			}
			break;
		case 11: { // set_volume
				if (so->volume != code[1]) {
					so->volume = code[1];
					_sssObjectsChanged = true;
				}
				code += 4;
			}
			break;
		case 12: { // remove_sounds2
				uint32_t _eax =  so->flags1 >> 24;
				uint32_t _edx = (so->flags1 >> 20) & 0xF;
				uint16_t _ecx = READ_LE_UINT16(code + 2);
				sssOp12_removeSounds2(_ecx, _edx, _eax);
				code += 4;
			}
			break;
		case 13: { // init_volume_modulation
				const int count = READ_LE_UINT32(code + 4);
				so->volumeModulateSteps = count - 1;
				const int16_t value = READ_LE_UINT16(code + 2);
				if (value == -1) {
					so->volumeModulateCurrent = so->volume << 16;
				} else {
					assert(value >= 0);
					so->volumeModulateCurrent = value << 16;
					so->volume = value;
					_sssObjectsChanged = true;
				}
				so->volumeModulateDelta = ((code[1] << 16) - so->volumeModulateCurrent) / count;
				return code + 8;
			}
			break;
		case 14: { // init_panning_modulation
				const int count = READ_LE_UINT32(code + 8);
				so->panningModulateSteps = count - 1;
				const int16_t value = READ_LE_UINT16(code + 2);
				if (value == -1) {
					so->panningModulateCurrent = so->panning << 16;
				} else {
					assert(value >= 0);
					so->panningModulateCurrent = value << 16;
					so->panning = value;
					_sssObjectsChanged = true;
				}
				so->panningModulateDelta = ((code[1] << 16) - so->panningModulateCurrent) / count;
				return code + 12;
			}
			break;
		case 16: { // resume_sound
				if (tempSssObject) {
					// 'tempSssObject' is allocated on the stack, it must not be added to the linked list
					warning("Invalid call to .sss opcode 16 with temporary SssObject");
					return 0;
				}
				--so->pauseCounter;
				if (so->pauseCounter >= 0) {
					return code;
				}
				sssOp16_resumeSound(so);
				code += 4;
				if (so->pcm == 0) {
					return code;
				}
				_sssObjectsChanged = true;
			}
			break;
		case 17: { // pause_sound
				if (tempSssObject) {
					// 'tempSssObject' is allocated on the stack, it must not be added to the linked list
					warning("Invalid call to .sss opcode 17 with temporary SssObject");
					return 0;
				}
				sssOp17_pauseSound(so);
				so->pauseCounter = READ_LE_UINT32(code + 4);
				return code + 8;
			}
			break;
		case 18: { // decrement_repeat_counter
				if (so->repeatCounter < 0) {
					so->repeatCounter = READ_LE_UINT32(code + 4);
				}
				code += 8;
			}
			break;
		case 19: { // set_panning
				if (so->panning != code[1]) {
					so->panning = code[1];
					_sssObjectsChanged = true;
				}
				code += 4;
			}
			break;
		case 20: { // set_pause_counter
				so->pauseCounter = READ_LE_UINT16(code + 2);
				code += 4;
			}
			break;
		case 21: { // decrement_delay_counter
				--so->delayCounter;
				if (so->delayCounter >= 0) {
					return code;
				}
				code += 4;
			}
			break;
		case 22: { // set_delay_counter
				so->delayCounter = READ_LE_UINT32(code + 4);
				return code + 8;
			}
			break;
		case 23: { // decrement_volume_modulate_steps
				--so->volumeModulateSteps;
				if (so->volumeModulateSteps >= 0) {
					return code;
				}
				code += 4;
			}
			break;
		case 24: { // set_volume_modulate_steps
				so->volumeModulateSteps = READ_LE_UINT32(code + 4);
				return code + 8;
			}
			break;
		case 25: { // decrement_panning_modulate_steps
				--so->panningModulateSteps;
				if (so->panningModulateSteps >= 0) {
					return code;
				}
				code += 4;
			}
			break;
		case 26: { // set_panning_modulate_steps
				so->panningModulateSteps = READ_LE_UINT32(code + 4);
				return code + 8;
			}
			break;
		case 27: { // seek_backward
				int frame = READ_LE_UINT32(code + 12);
				if (so->currentPcmFrame > frame) {
					so->currentPcmFrame = READ_LE_UINT32(code + 8);
					if (so->pcm) {
						const int16_t *ptr = so->pcm->ptr;
						if (ptr) {
							so->currentPcmPtr = ptr + READ_LE_UINT32(code + 4) / sizeof(int16_t);
						}
					}
				}
				return code;
			}
			break;
		case 28: { // jump
				const int32_t offset = READ_LE_UINT32(code + 4);
				code -= offset;
			}
			break;
		case 29: // end
			so->pcmFramesCount = 0;
			return 0;
		default:
			error("Invalid .sss opcode %d", *code);
			break;
		}
	}
	return code;
}

SssObject *Game::addSoundObject(SssPcm *pcm, int priority, uint32_t flags_a, uint32_t flags_b) {
	// if (!_sss_enabled) return;
	int minIndex = -1;
	int minPriority = -1;
	for (int i = 0; i < kMaxSssObjects; ++i) {
		if (!_sssObjectsTable[i].pcm) {
// 42A2FA
			minPriority = 0;
			minIndex = i;
			break;
		}
		if (_sssObjectsTable[i].currentPriority < minPriority) {
			minPriority = _sssObjectsTable[i].currentPriority;
			minIndex = i;
		}
	}
// 42A2FE
	if (minIndex == -1) {
		return 0;
	}
	assert(minIndex != -1);
	SssObject *so = &_sssObjectsTable[minIndex];
	if (so->pcm && minPriority >= priority) {
		return 0;
	}
	if (so->pcm) {
		removeSoundObjectFromList(so);
	}
// 42A332
	so->flags1 = flags_a;
	so->currentPriority = priority;
	so->pcm = pcm;
	so->volume = 128;
	so->panning = 64;
	if (pcm->flags & 1) {
		so->stereo = true;
	} else {
		so->stereo = false;
	}
	so->nextSoundNum = -1;
	so->currentPcmFrame = 0;
	so->flags = 0;
	so->pcmFramesCount = pcm->strideCount;
	so->currentPcmPtr = pcm->ptr;
	if (!so->currentPcmPtr) {
		so->flags |= 2;
	}
	so->flags0 = flags_b;
	prependSoundObjectToList(so);
	return so->pcm ? so : 0;
}

void Game::prependSoundObjectToList(SssObject *so) {
	if (!so->pcm || !so->pcm->ptr) {
		so->flags = (so->flags & ~1) | 2;
	}
	if (so->flags & 2) {
		debug(kDebug_SOUND, "Adding so %p to list2 flags 0x%x", so, so->flags);
		if (1) {
			for (SssObject *current = _sssObjectsList2; current; current = current->nextPtr) {
				if (current == so) {
					warning("SoundObject %p already in _sssObjectsList2", so);
					return;
				}
			}
		}
		so->prevPtr = 0;
		so->nextPtr = _sssObjectsList2;
		if (_sssObjectsList2) {
			_sssObjectsList2->prevPtr = so;
		}
		_sssObjectsList2 = so;
	} else {
		debug(kDebug_SOUND, "Adding so %p to list1 flags 0x%x", so, so->flags);
		if (1) {
			for (SssObject *current = _sssObjectsList1; current; current = current->nextPtr) {
				if (current == so) {
					warning("SoundObject %p already in _sssObjectsList1", so);
					return;
				}
			}
		}
// 429185
		SssObject *stopSo = so; // _edi
		if (so->pcm && so->pcm->ptr) {
			if (kLimitSounds && _playingSssObjectsCount >= _playingSssObjectsMax) {
				if (so->currentPriority > _lowPrioritySssObject->currentPriority) {

					stopSo = _lowPrioritySssObject;
					SssObject *next = _lowPrioritySssObject->nextPtr; // _edx
					SssObject *prev = _lowPrioritySssObject->prevPtr; // _ecx

					so->nextPtr = next;
					so->prevPtr = prev;

					if (next) {
						next->prevPtr = so;
					}
					if (prev) {
						prev->nextPtr = so;
					} else {
						assert(so == _sssObjectsList1);
						_sssObjectsList1 = so;
					}
// 429281
					_lowPrioritySssObject = findLowestPrioritySssObject(_sssObjectsList1);
				}
			} else {
// 4291E8
				stopSo = 0;

				++_playingSssObjectsCount;
				so->nextPtr = _sssObjectsList1;
				so->prevPtr = 0;
				if (_sssObjectsList1) {
					_sssObjectsList1->prevPtr = so;
				}
				_sssObjectsList1 = so;
// 42920F
				if (kLimitSounds) {
					if (_playingSssObjectsCount < _playingSssObjectsMax) {
						_lowPrioritySssObject = 0;
					} else {
						if (_lowPrioritySssObject == 0) {
							_lowPrioritySssObject = findLowestPrioritySssObject(_sssObjectsList1);
						} else {
// 429269
							if (so->currentPriority < _lowPrioritySssObject->currentPriority) {
								_lowPrioritySssObject = so;
							}
						}
					}
				}
			}
// 4292BE
			so->flags |= 1;
		}
// 4292C2
		if (stopSo) {
			stopSo->flags &= ~1;
			stopSo->pcm = 0;
			updateSoundObjectLut2(stopSo->flags0);
		}
	}
// 4292DF
	if (so->num >= _sssObjectsCount) {
		_sssObjectsCount = so->num + 1;
	}
}

void Game::updateSoundObjectLut2(uint32_t flags) {
	uint32_t mask = 1 << (flags >> 24);
	uint32_t *sssLut = getSssLutPtr(_res, 2, flags);
	if ((*sssLut & mask) != 0) {
		for (SssObject *so = _sssObjectsList1; so; so = so->nextPtr) {
			if (compareSssLut(so->flags0, flags)) {
				return;
			}
			so = so->nextPtr;
		}
		for (SssObject *so = _sssObjectsList2; so; so = so->nextPtr) {
			if (compareSssLut(so->flags0, flags)) {
				return;
			}
		}
// 42AC87
		*sssLut &= ~mask;
	}
}

SssObject *Game::createSoundObject(int num, int b, int flags) {
	debug(kDebug_SOUND, "createSoundObject num %d b %d c 0x%x", num, b, flags);
	SssObject *ret = 0;
	if (b < 0) {
		SssBank *bank = &_res->_sssBanksData[num];
		if ((bank->flags & 1) != 0) {
			int firstSampleIndex = bank->firstSampleIndex;
			if (bank->count <= 0) {
				return 0;
			}
			assert(firstSampleIndex >= 0 && firstSampleIndex < _res->_sssHdr.samplesDataCount);
			SssSample *sample = &_res->_sssSamplesData[firstSampleIndex];
// 42B81D
			int framesCount = 0;
			for (int i = 0; i < bank->count; ++i) {
				if (sample->pcm != 0xFFFF) {
					SssObject *so = startSoundObject(num, i, flags);
					if (so && so->pcmFramesCount >= framesCount) {
						framesCount = so->pcmFramesCount;
						ret = so;
					}
				}
				++sample;
			}
		}
// 42B865
		uint32_t _eax = 1 << (_rnd.update() & 31);
		SssUnk6 *unk6 = &_res->_sssDataUnk6[num];
		if ((unk6->unk10 & _eax) == 0) {
			if (_eax > unk6->unk10) {
				do {
					_eax >>= 1;
				} while ((unk6->unk10 & _eax) == 0);
			} else {
// 42B8A8
				do {
					_eax <<= 1;
				} while ((unk6->unk10 & _eax) == 0);
			}
		}
// 42B8AE
		b = 0;
		if (bank->count > 0) {
			do {
				if ((unk6->unk0[b] & _eax) != 0) {
					break;
				}
			} while (++b < bank->count);
		}
// 42B8C7
		if ((bank->flags & 2) != 0) {
			unk6->unk10 &= ~unk6->unk0[b];
			if (unk6->unk10 == 0 && bank->count > 0) {
				int i = 0;
				do {
					unk6->unk10 |= unk6->unk0[i];
				} while (++i < bank->count);
			}
		}
// 42B8E9
		ret = startSoundObject(num, b, flags);
		if (ret && (_res->_sssBanksData[num].flags & 4) != 0) {
			ret->nextSoundNum = num;
			ret->nextSoundCounter = -1;
		}
	} else {
		ret = startSoundObject(num, b, flags);
		if (ret && (_res->_sssBanksData[num].flags & 4) != 0) {
			ret->nextSoundNum = num;
			ret->nextSoundCounter = b;
		}
	}
	return ret;
}

SssObject *Game::startSoundObject(int num, int b, uint32_t flags) {
	debug(kDebug_SOUND, "startSoundObject num %d b %d flags 0x%x", num, b, flags);

	SssBank *bank = &_res->_sssBanksData[num];
	int sampleNum = bank->firstSampleIndex + b;
	debug(kDebug_SOUND, "startSoundObject sample %d", sampleNum);
	assert(sampleNum >= 0 && sampleNum < _res->_sssHdr.samplesDataCount);
	SssSample *sample = &_res->_sssSamplesData[sampleNum];

	// original loads the PCM data in a seperate thread
	_res->loadSssPcm(_res->_sssFile, sample->pcm);

	if (sample->unk2 != 0) {
// 42B64C
		SssFilter *filter = &_res->_sssFilters[bank->sssFilter];
		int _ecx = CLIP(filter->unk24 + sample->unk6, 0, 7); // priority
// 42B67F
		uint32_t flags1 = flags & 0xFFF0F000;
		flags1 |= (num & 0xFFF);

		SssPcm *pcm = &_res->_sssPcmTable[sample->pcm];
		SssObject *so = addSoundObject(pcm, _ecx, flags1, flags);
		if (so) {
			if (sample->codeOffset1 == kNone && sample->codeOffset2 == kNone && sample->codeOffset3 == kNone && sample->codeOffset4 == kNone) {
				so->flags |= 4;
			}
			so->codeDataStage1 = (sample->codeOffset1 == kNone) ? 0 : _res->_sssCodeData + sample->codeOffset1;
			so->codeDataStage2 = (sample->codeOffset2 == kNone) ? 0 : _res->_sssCodeData + sample->codeOffset2;
			so->codeDataStage3 = (sample->codeOffset3 == kNone) ? 0 : _res->_sssCodeData + sample->codeOffset3;
			so->codeDataStage4 = (sample->codeOffset4 == kNone) ? 0 : _res->_sssCodeData + sample->codeOffset4;
			so->lvlObject = _currentSoundLvlObject;
			so->repeatCounter = -1;
			so->pauseCounter = -1;
			so->delayCounter = -1;
			so->volumeModulateSteps = -100;
			so->panningModulateSteps = -1;
			so->volumeModulateDelta = 0;
			so->panningModulateDelta = 0;
			so->flags0 = flags;
			so->pcmFramesCount = sample->unk2;
			so->bankIndex = num;
			so->priority = sample->unk6;
			so->filter = filter;
			so->volume = sample->unk4;
			so->panning = sample->unk7;
			if (sample->unk7 == 0xFF) {
				if (_currentSoundLvlObject) {
					_currentSoundLvlObject->sssObject = so;
					so->panningPtr = &_snd_masterPanning;
					so->panning = getSoundObjectPanning(so);
				} else {
					so->panningPtr = 0;
					so->panning = 64;
				}
			} else {
// 42B7A5
				so->panningPtr = 0;
			}
// 42B7C3
			setSoundObjectPanning(so);
			if (so->pcm) {
				updateSoundObject(so);
				return so;
			}
		}
		return 0;
	}

	SssObject tmpObj;
	memset(&tmpObj, 0, sizeof(tmpObj));
	tmpObj.flags0 = flags;
	tmpObj.flags1 = flags;
	tmpObj.bankIndex = num;
	tmpObj.repeatCounter = -1;
	tmpObj.pauseCounter = -1;
	tmpObj.lvlObject = _currentSoundLvlObject;
	tmpObj.panningPtr = 0;
	debug(kDebug_SOUND, "startSoundObject dpcm %d", sample->pcm);
	tmpObj.pcm = &_res->_sssPcmTable[sample->pcm];
	if (sample->codeOffset1 != kNone) {
		const uint8_t *code = _res->_sssCodeData + sample->codeOffset1;
		executeSssCode(&tmpObj, code, true);
	}

	const uint32_t mask = 1 << (flags >> 24);
	uint32_t *sssLut2 = getSssLutPtr(_res, 2, flags);
	if ((*sssLut2 & mask) != 0) {
		for (SssObject *so = _sssObjectsList1; so; so = so->nextPtr) {
			if (compareSssLut(so->flags0, flags)) {
				*sssLut2 &= ~mask;
				return 0;
			}
			so = so->nextPtr;
		}
		for (SssObject *so = _sssObjectsList2; so; so = so->nextPtr) {
			if (compareSssLut(so->flags0, flags)) {
				*sssLut2 &= ~mask;
				return 0;
			}
		}
		*sssLut2 &= ~mask;
	}
	return 0;
}

void Game::playSoundObject(SssInfo *s, int lut, int bits) {
	debug(kDebug_SOUND, "playSoundObject num %d lut 0x%x bits 0x%x", s->sssBankIndex, lut, bits);
	if (_sssDisabled) {
		return;
	}
	const int num = _res->_sssBanksData[s->sssBankIndex].sssFilter;
	debug(kDebug_SOUND, "sssFilter num %d", num);
	SssFilter *filter = &_res->_sssFilters[num];
	bool found = false;
	for (int i = 0; i < _sssObjectsCount; ++i) {
		SssObject *so = &_sssObjectsTable[i];
		if (so->pcm != 0 && so->filter == filter) {
			found = true;
			break;
		}
	}
	int _ecx = filter->unk4;
	int _eax = ((int8_t)s->unk3) << 16;
	if (_ecx != _eax) {
		if (!found) {
			filter->unk4 = _eax; // int32_t
		} else {
			filter->unkC = 4; // uint32_t
			filter->unk30 = 1; // uint32_t
			_eax = ((s->unk3 << 16) - _ecx) / 4;
			filter->unk8 = _eax; // uint32_t
		}
	}
// 42B9FD
	_eax = ((int8_t)s->unk5) << 16;
	_ecx = filter->unk14;
	if (_ecx != _eax) {
		if (!found) {
			filter->unk14 = _eax; // int32_t
		} else {
			filter->unk1C = 4;
			filter->unk30 = 1;
			_eax = ((s->unk5 << 16) - _ecx) / 4;
			filter->unk18 = _eax;
		}
	}
// 42BA37
	_eax = (int8_t)s->unk4;
	const int scale = filter->unk24;
	if (scale != _eax) {
		filter->unk24 = _eax;
		for (int i = 0; i < _sssObjectsCount; ++i) {
			SssObject *so = &_sssObjectsTable[i];
			if (so->pcm != 0 && so->filter == filter) {
				int _al = filter->unk24 & 255;
				_al += so->priority;
				so->currentPriority = _al < 0 ? 0 : _al;
				if (so->currentPriority > 7) {
					so->currentPriority = 7;
					setLowPrioritySoundObject(so);
				}
			}
		}
	}
// 42BA9D
	const uint32_t _ebp = valueSssLut(lut, bits, s);
	const uint8_t _al = s->unk6;
	_ecx = s->sssBankIndex;
	if (_al & 2) {
		const uint32_t mask = 1 << (_ebp >> 24);
		uint32_t *sssLut3 = _res->_sssLookupTable3[(_ebp >> 20) & 15] + _ecx;
		*sssLut3 |= mask;
		uint32_t *sssLut2 = _res->_sssLookupTable2[(_ebp >> 20) & 15] + _ecx;
		if (*sssLut2 & mask) {
			return;
		}
		*sssLut2 |= mask;
// 42BB26
	} else if (_al & 1) {
		const uint32_t mask = 1 << (_ebp >> 24);
		uint32_t *sssLut1 = _res->_sssLookupTable1[(_ebp >> 20) & 15] + _ecx;
		if (*sssLut1 & mask) {
			return;
		}
		*sssLut1 |= mask;
// 42BB60
	} else if (_al & 4) {
		for (SssObject *so = _sssObjectsList1; so; so = so->nextPtr) {
			if (compareSssLut(so->flags0, _ebp)) {
				goto prepare;
			}
		}
		for (SssObject *so = _sssObjectsList2; so; so = so->nextPtr) {
			if (compareSssLut(so->flags0, _ebp)) {
				goto prepare;
			}
		}
		return;
	}
// 42BBDD
prepare:
	createSoundObject(s->sssBankIndex, s->unk2, _ebp);
}

void Game::clearSoundObjects() {
	memset(_sssObjectsTable, 0, sizeof(_sssObjectsTable));
	_sssObjectsList1 = 0;
	_sssObjectsList2 = 0;
	_lowPrioritySssObject = 0;
	for (int i = 0; i < kMaxSssObjects; ++i) {
		_sssObjectsTable[i].num = i;
	}
	_sssObjectsCount = 0;
	_playingSssObjectsCount = 0;
	_mix._mixingQueueSize = 0;
	if (_res->_sssHdr.infosDataCount != 0) {
		const int size = _res->_sssHdr.dataUnk3Count * 4;
		for (int i = 0; i < 3; ++i) {
			memset(_res->_sssLookupTable1[i], 0, size);
			memset(_res->_sssLookupTable2[i], 0, size);
			memset(_res->_sssLookupTable3[i], 0, size);
		}
	}
	memset(_sssUpdatedObjectsTable, 0, sizeof(_sssUpdatedObjectsTable));
	if (_res->_sssFilters) {
		memset(_res->_sssFilters, 0, _res->_sssHdr.dataUnk2Count * sizeof(SssFilter));
		if (_res->_sssHdr.dataUnk2Count != 0) {
			for (int i = 0; i < _res->_sssHdr.dataUnk2Count; ++i) {
				const int a = _res->_sssDataUnk2[i].unk0 << 16;
				_res->_sssFilters[i].unk4 = a;
				_res->_sssFilters[i].unk0 = a;
				const int b = _res->_sssDataUnk2[i].unk1 << 16;
				_res->_sssFilters[i].unk14 = b;
				_res->_sssFilters[i].unk10 = b;
				const int c = _res->_sssDataUnk2[i].unk2 << 16;
				_res->_sssFilters[i].unk24 = c;
				_res->_sssFilters[i].unk20 = c;
			}
		}
	}
}

void Game::setLowPrioritySoundObject(SssObject *so) {
	if ((so->flags & 2) == 0) {
		if (kLimitSounds) {
			_lowPrioritySssObject = 0;
			if (_playingSssObjectsCount >= _playingSssObjectsMax && _sssObjectsList1) {
				_lowPrioritySssObject = findLowestPrioritySssObject(_sssObjectsList1);
			}
		}
	}
}

int Game::getSoundObjectPanning(SssObject *so) const {
	LvlObject *obj = so->lvlObject;
	if (obj) {
		switch (obj->type) {
		case 8:
		case 2:
		case 0:
			if (obj->screenNum == _currentLeftScreen) {
				return -1;
			}
			if (obj->screenNum == _currentRightScreen) {
				return 129;
			}
			if (obj->screenNum == _currentScreen || (_currentLevel == kLvl_lar2 && obj->spriteNum == 27) || (_currentLevel == kLvl_isld && obj->spriteNum == 26)) {
				const int dist = (obj->xPos + obj->width / 2) / 2;
				return CLIP(dist, 0, 128);
			}
			// fall-through
		default:
			return -2;
		}
	}
	return so->panning;
}

void Game::setSoundObjectPanning(SssObject *so) {
	if ((so->flags & 2) == 0 && so->volume != 0 && _snd_masterVolume != 0) {
		int volume = ((so->filter->unk4 >> 16) * so->volume) >> 7; // indexes decibel volume table
		int _esi = 0;
		if (so->panningPtr) {
			int _eax = CLIP(so->priority + so->filter->unk24, 0, 7); // priority
			if (so->panning == -2) {
				volume = 0;
				_esi = 64;
				_eax = 0;
			} else {
				_esi = CLIP(so->panning, 0, 128);
				volume >>= 2; // _edi
				_eax /= 2;
			}
			if (so->currentPriority != _eax) {
				so->currentPriority = _eax;
				_lowPrioritySssObject = 0;
				if (kLimitSounds && _playingSssObjectsCount >= _playingSssObjectsMax && _sssObjectsList1) {
					_lowPrioritySssObject = findLowestPrioritySssObject(_sssObjectsList1);
				}
			}

		} else {
// 429076
			_esi = CLIP(so->panning + (so->filter->unk14 >> 16), 0, 128); // panning ?
		}
// 429094
		if (so->pcm == 0) {
			return;
		}
		if (volume >= (int)ARRAYSIZE(_dbVolumeTable)) {
			warning("Out of bounds volume %d (filter %d volume %d)", volume, so->filter->unk4, so->volume);
			volume = ARRAYSIZE(_dbVolumeTable) - 1;
		}
		int _edx = _dbVolumeTable[volume];
		int _edi = _esi;
		int _eax = _edx << 7;
		switch (_edi) {
		case 0: // full left
			so->panL = _eax;
			so->panR = 0;
			so->panType = 2;
			break;
		case 64: // center
			_eax /= 2;
			so->panL = _eax;
			so->panR = _eax;
			so->panType = 3;
			break;
		case 128: // full right
			so->panL = 0;
			so->panR = _eax;
			so->panType = 1;
			break;
		default:
			_edx *= _esi;
			so->panR = _edx;
			so->panL = _eax - _edx;
			so->panType = 4;
			break;
		}
// 4290DF
		so->panR = (so->panR * _snd_masterVolume + 64) >> 7;
		so->panL = (so->panL * _snd_masterVolume + 64) >> 7;
	} else {
		so->panL = 0;
		so->panR = 0;
		so->panType = 0;
	}
}

void Game::expireSoundObjects(uint32_t flags) {
	const uint32_t mask = 1 << (flags >> 24);
	uint32_t *sssLut1 = getSssLutPtr(_res, 1, flags);
	*sssLut1 &= ~mask;
	uint32_t *sssLut2 = getSssLutPtr(_res, 2, flags);
	*sssLut2 &= ~mask;
	for (SssObject *so = _sssObjectsList1; so; so = so->nextPtr) {
		if ((so->flags & 0xFFFF0FFF) == 0) {
			so->codeDataStage3 = 0;
			if (so->codeDataStage4 == 0) {
				removeSoundObjectFromList(so);
			}
			so->nextSoundNum = -1;
			so->delayCounter = -2;
		}
	}
	for (SssObject *so = _sssObjectsList2; so; so = so->nextPtr) {
		if ((so->flags & 0xFFFF0FFF) == 0) {
			so->codeDataStage3 = 0;
			if (so->codeDataStage4 == 0) {
				removeSoundObjectFromList(so);
			}
			so->nextSoundNum = -1;
			so->delayCounter = -2;
		}
	}
	while (_sssObjectsCount > 0 && _sssObjectsTable[_sssObjectsCount - 1].pcm == 0) {
		--_sssObjectsCount;
	}
}

void Game::mixSoundObjects17640(bool flag) {
	for (int i = 0; i < _res->_sssHdr.dataUnk2Count; ++i) {
		_res->_sssFilters[i].unk30 = 0;

		if (_res->_sssFilters[i].unkC != 0) {
			--_res->_sssFilters[i].unkC;
		}
		_res->_sssFilters[i].unk4 += _res->_sssFilters[i].unk8;
		_res->_sssFilters[i].unk30 = 1;

		if (_res->_sssFilters[i].unk1C != 0) {
			--_res->_sssFilters[i].unk1C;
		}
		_res->_sssFilters[i].unk14 += _res->_sssFilters[i].unk18;
		_res->_sssFilters[i].unk30 = 1;
	}
// 42B426
	for (int i = 0; i < _sssObjectsCount; ++i) {
		SssObject *so = &_sssObjectsTable[i];
		if (so->pcm && !_sssUpdatedObjectsTable[i]) {
			if (flag) {
				const int mask = 1 << (so->flags1 >> 24);
				if ((*getSssLutPtr(_res, 2, so->flags1) & mask) != 0) {
					if ((*getSssLutPtr(_res, 3, so->flags1) & mask) == 0) {
						expireSoundObjects(so->flags1);
					}
				}
			}
			updateSoundObject(so);
		}
	}
// 42B4B2
	memset(_sssUpdatedObjectsTable, 0, sizeof(_sssUpdatedObjectsTable));
	if (flag) {
		_res->clearSssLookupTable3();
	}
	queueSoundObjectsPcmStride();
}

void Game::queueSoundObjectsPcmStride() {
	for (SssObject *so = _sssObjectsList1; so; so = so->nextPtr) {
		const SssPcm *pcm = so->pcm;
		if (pcm != 0) {
			const int16_t *ptr = pcm->ptr;
			if (!ptr) {
				continue;
			}
			if (so->currentPcmPtr < ptr) {
				continue;
			}
			//const int16_t *end = ptr + pcm->strideCount * pcm->strideSize / sizeof(int16_t);
			const int16_t *end = ptr + _res->getSssPcmSize(pcm) / sizeof(int16_t);
			if (so->currentPcmPtr >= end) {
				continue;
			}
			if (pcm->strideSize != 2276 && pcm->strideSize != 4040) {
				warning("Ignore sample strideSize %d", pcm->strideSize);
				continue;
			}
			if (_mix._mixingQueueSize >= Mixer::kPcmChannels) {
				warning("MixingQueue overflow %d", _mix._mixingQueueSize);
				break;
			}
			_mix._mixingQueue[_mix._mixingQueueSize].ptr = so->currentPcmPtr;
			_mix._mixingQueue[_mix._mixingQueueSize].end = end;
			_mix._mixingQueue[_mix._mixingQueueSize].panL = so->panL;
			_mix._mixingQueue[_mix._mixingQueueSize].panR = so->panR;
			_mix._mixingQueue[_mix._mixingQueueSize].panType = so->panType;
			_mix._mixingQueue[_mix._mixingQueueSize].stereo = so->stereo;
			const int strideSize = (pcm->strideSize - 256 * sizeof(int16_t));
			assert(strideSize == 1764 || strideSize == 3528); // words
			//pcm->currentPcmPtr += pcm->strideSize;
			so->currentPcmPtr += strideSize;
			++_mix._mixingQueueSize;
		}
	}
}

void Game::stopSoundObjectsByPcm(SssObject **sssObjectsList, int num) {
	bool found = false;
	SssPcm *pcm = &_res->_sssPcmTable[num];
	SssObject *current = *sssObjectsList;
	while (current) {
		if (current->pcm == pcm) {
			SssObject *prev = current->prevPtr; // _ecx
			SssObject *next = current->nextPtr; // _esi
			if (next) {
				next->prevPtr = prev;
			}
			if (prev) {
				prev->nextPtr = next;
			} else {
				assert(current == *sssObjectsList);
				*sssObjectsList = next;
			}
			if (sssObjectsList == &_sssObjectsList1) {
				--_playingSssObjectsCount;
				if (kLimitSounds) {
					if (current == _lowPrioritySssObject || (_playingSssObjectsCount < _playingSssObjectsMax && _lowPrioritySssObject)) {
						found = true;
					}
				}
			}
// 429576
			updateSoundObjectLut2(current->flags);
			current = next;
		} else {
// 429583
			current = current->nextPtr;
		}
	}
	if (kLimitSounds) {
		if (found) {
			_lowPrioritySssObject = 0;
			if (_playingSssObjectsCount >= _playingSssObjectsMax && _sssObjectsList1) {
				_lowPrioritySssObject = findLowestPrioritySssObject(_sssObjectsList1);
			}
		}
	}
}
