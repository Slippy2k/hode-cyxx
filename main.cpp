/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
 */

#include <getopt.h>
#include <sys/stat.h>

#include "game.h"
#include "util.h"
#include "systemstub.h"

static const char *USAGE =
	"HODe - Heart Of Darkness Interpreter\n"
	"Usage: %s [OPTIONS]...\n"
	"  --datapath=PATH   Path to data files (default 'DATA')\n"
	"  --level=NUM       Start at level NUM\n"
	"  --checkpoint=NUM  Start at checkpoint NUM\n"
;

static SystemStub *_system = 0;

static void exitMain() {
	if (_system) {
		_system->destroy();
		delete _system;
		_system = 0;
	}
}

static const char *kLevelNames[] = {
	"rock",
	"fort",
	"pwr1",
	"isld",
	"lava",
	"pwr2",
	"lar1",
	"lar2",
	"dark",
	0
};

int main(int argc, char *argv[]) {
	const char *dataPath = ".";
	int level = 0;
	int checkpoint = 0;
	if (argc == 2) {
		// data path as the only command line argument
		struct stat st;
		if (stat(argv[1], &st) == 0 && S_ISDIR(st.st_mode)) {
			dataPath = strdup(argv[1]);
		}
	}
	while (1) {
		static struct option options[] = {
			{ "datapath",   required_argument, 0, 1 },
			{ "level",      required_argument, 0, 2 },
			{ "checkpoint", required_argument, 0, 3 },
			{ 0, 0, 0, 0 },
		};
		int index;
		const int c = getopt_long(argc, argv, "", options, &index);
		if (c == -1) {
			break;
		}
		switch (c) {
		case 1:
			dataPath = strdup(optarg);
			break;
		case 2:
			if (optarg[0] >= '0' && optarg[0] <= '9') {
				level = atoi(optarg);
			} else {
				for (int i = 0; kLevelNames[i]; ++i) {
					if (strcmp(kLevelNames[i], optarg) == 0) {
						level = i;
						break;
					}
				}
			}
			break;
		case 3:
			checkpoint = atoi(optarg);
			break;
		default:
			fprintf(stdout, "%s\n", USAGE);
			return -1;
		}
	}
	_system = SystemStub_SDL_create();
	atexit(exitMain);
	g_debugMask = 0; //kDebug_GAME | kDebug_RESOURCE | kDebug_SOUND;
	Game *g = new Game(_system, dataPath ? dataPath : "DATA");
	g->mainLoop(level, checkpoint);
	delete g;
	return 0;
}
