
// lava_hod

#include "game.h"
#include "lzw.h"
#include "video.h"

void Game::callLevel_initialize_lava() {
	_shakeShadowBuffer = (uint8_t *)malloc(256 * 192 + 256);
	decodeLZW(_levelOpStage3ImageData1, _shakeShadowBuffer);
	memcpy(_shakeShadowBuffer + 256 * 192, _shakeShadowBuffer, 256);
}
