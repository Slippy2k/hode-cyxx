
#include <queue>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "lzw.h"
#include "screenshot.h"

static const int MAX_OFFSETS = 1024;

static uint8_t _buffer[256 * 192];
static uint8_t _buffer2[256 * 192 + 1024];
static uint8_t _palette[256 * 3];

static uint32_t offsets[MAX_OFFSETS];

static void convertVgaColors() {
	for (int i = 0; i < 256 * 3; ++i) {
		_palette[i] = (_palette[i] << 2) | (_palette[i] & 3);
	}
}

static void decodeBitmap(FILE *fp, uint32_t offset, const char *name, int screen, int state) {
	int count;

	if (screen == 0 || screen == 4 || screen == 9 || screen == 18 || screen == 19) {
		if (state == 0) {
			fseek(fp, offset - 0x60C, SEEK_SET);
			fread(_palette, 1, sizeof(_palette), fp);
			convertVgaColors();
		}
	} else {
		for (int i = 0; i < 256; ++i) {
			_palette[3 * i] = _palette[3 * i + 1] = _palette[3 * i + 2] = i;
		}
	}

	fseek(fp, offset, SEEK_SET);
	fread(_buffer, 1, sizeof(_buffer), fp);
	count = decodeLZW(_buffer, _buffer2);
	fprintf(stdout, "bitmap size %d offset 0x%x name %s\n", count, offset, name);
	saveBMP(name, _buffer2, _palette, 256, 192);
}

static const int kMaxScreens = 40;

enum {
	kPosTopScreen    = 0,
	kPosRightScreen  = 1,
	kPosBottomScreen = 2,
	kPosLeftScreen   = 3
};

struct LevelScreen {
	uint8_t num;
	int x, y;
};

static const int kLevelMapMaxW = 128;
static const int kLevelMapMaxH = 128;

static void GenerateLevelMap(const uint8_t *data, int count) {
	bool visited[kMaxScreens];
	for (int i = 0; i < count; ++i) {
		visited[i] = false;
	}

	std::queue<LevelScreen> q;

	uint8_t levelMap[kLevelMapMaxH][kLevelMapMaxW];
	memset(levelMap, 255, sizeof(levelMap));

	int xmin, xmax;
	int ymin, ymax;

	LevelScreen ls;
	ls.num = 0;
	ls.x = xmin = xmax = kLevelMapMaxW / 2;
	ls.y = ymin = ymax = kLevelMapMaxH / 2;
	q.push(ls);

	while (!q.empty()) {
		const int screenNum = q.front().num;
		const int x = q.front().x;
		const int y = q.front().y;
		q.pop();
		if (visited[screenNum]) {
			continue;
		}
		if (x < xmin) {
			xmin = x;
		} else if (x > xmax) {
			xmax = x;
		}
		if (y < ymin) {
			ymin = y;
		} else if (y > ymax) {
			ymax = y;
		}
		levelMap[y][x] = screenNum;
		visited[screenNum] = true;
		const int top    = data[screenNum * 4 + kPosTopScreen];
		const int right  = data[screenNum * 4 + kPosRightScreen];
		const int bottom = data[screenNum * 4 + kPosBottomScreen];
		const int left   = data[screenNum * 4 + kPosLeftScreen];
		if (top != 255) {
			assert(y >= 0);
			ls.num = top;
			ls.x = x;
			ls.y = y - 1;
			q.push(ls);
		}
		if (bottom != 255) {
			assert(y < kLevelMapMaxH);
			ls.num = bottom;
			ls.x = x;
			ls.y = y + 1;
			q.push(ls);
		}
		if (left != 255) {
			assert(x >= 0);
			ls.num = left;
			ls.x = x - 1;
			ls.y = y;
			q.push(ls);
		}
		if (right != 255) {
			assert(x < kLevelMapMaxW);
			ls.num = right;
			ls.x = x + 1;
			ls.y = y;
			q.push(ls);
		}
	}
	fprintf(stdout, "levelMap %d %d\n", (ymax - ymin + 1), (xmax - xmin + 1));
	for (int y = ymin; y <= ymax; ++y) {
		for (int x = xmin; x <= xmax; ++x) {
			if (levelMap[y][x] == 255) {
				fprintf(stdout, "    ");
			} else {
				fprintf(stdout, " %02x ", levelMap[y][x]);
			}
		}
		fprintf(stdout, "\n");
	}
}

// ~/Data/heart_of_darkness/DATA_saturn/00000001
int main(int argc, char *argv[]) {
	int count = 0;
	FILE *fp = fopen(argv[1], "rb");
	if (fp) {
		const char *sep = strrchr(argv[1], '/');
		if (sep && strcmp(sep + 1, "00000002") == 0) {
			fseek(fp, 4, SEEK_SET);
			count = fgetc(fp);
			fread(_buffer, 1, 4 * count, fp);
			GenerateLevelMap(_buffer, count);
			goto end;
		}
/*
		// matches DOS demo
		decodeBitmap(fp, 0x616);
		decodeBitmap(fp, 0x6646);
		decodeBitmap(fp, 0x9D9CA);
		decodeBitmap(fp, 0xAF514);
*/

		// find 0x00 0xAD 0x00
		fseek(fp, 0, SEEK_SET);
		do {
			if (fgetc(fp) == 0) {
				uint8_t code1 = fgetc(fp);
				uint8_t code2 = fgetc(fp);
				if ((code1 == 0xAD || code1 == 0xBD) && code2 == 0) {
					assert(count < MAX_OFFSETS);
					offsets[count++] = ftell(fp) - 1;
				}
			}
		} while (!feof(fp));
		fprintf(stdout, "Found %d offsets\n", count);
		{
			char name[32];
			int screenNum = 0;
			int stateNum = 0;
			uint32_t prev = 0;
			for (int i = 0; i < count; ++i) {
				if (i != 0) {
					// fprintf(stdout, "diff 0x%x screen %d\n", offsets[i] - prev, screenNum);
					if (offsets[i] - prev < 0x8000) {
						++stateNum;
					} else {
						++screenNum;
						stateNum = 0;
					}
				}
				snprintf(name, sizeof(name), "hod_%02d_%02d.bmp", screenNum, stateNum);
				decodeBitmap(fp, offsets[i], name, screenNum, stateNum);
				prev = offsets[i];
			}
		}
end:
		fclose(fp);
	}
	return 0;
}
