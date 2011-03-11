/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009 Gregory Montoir
 */

#include "game.h"
#include "util.h"
#include "systemstub.h"

static const char *_usage =
	"HODe - Heart Of Darkness Interpreter\n"
	"Usage: %s [OPTIONS]...\n"
	"  --datapath=PATH   Path to data files (default 'DATA')\n";

static bool parseOption(const char *arg, const char *longCmd, const char **opt) {
	if (strncmp(arg, "--", 2) == 0) {
		if (strncmp(arg + 2, longCmd, strlen(longCmd)) == 0) {
			*opt = arg + 2 + strlen(longCmd);
			return true;
		}
	}
	return false;
}

static bool parseOptionInt(const char *arg, const char *longCmd, int *opt) {
	const char *level;
	if (parseOption(arg, longCmd, &level)) {
		*opt = strtol(level, 0, 10);
		return true;
	}
	return false;
}

static SystemStub *_system = 0;

static void exitMain() {
	if (_system) {
		_system->destroy();
		delete _system;
		_system = 0;
	}
}

#undef main
int main(int argc, char *argv[]) {
	const char *dataPath = "DATA";
	int initialLevel = 0;
	for (int i = 1; i < argc; ++i) {
		bool opt = false;
		if (strlen(argv[i]) >= 2) {
			opt |= parseOption(argv[i], "datapath=", &dataPath);
			opt |= parseOptionInt(argv[i], "level=", &initialLevel);
		}
		if (!opt) {
			printf(_usage, argv[0]);
			return 0;
		}
	}
	_system = SystemStub_SDL_create();
	atexit(exitMain);
	g_debugMask = DEBUG_GAME | DEBUG_RESOURCE;
	Game *g = new Game(_system, dataPath);
	g->mainLoop(initialLevel);
	delete g;
	return 0;
}
