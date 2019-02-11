/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
 */

#include <getopt.h>
#include <sys/stat.h>

#include "3p/inih/ini.h"

#include "game.h"
#include "paf.h"
#include "util.h"
#include "systemstub.h"

static const char *_configIni = "hode.ini";

static const char *_usage =
	"HODe - Heart Of Darkness Interpreter\n"
	"Usage: %s [OPTIONS]...\n"
	"  --datapath=PATH   Path to data files (default '.')\n"
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

static const char *_defaultDataPath = ".";

static const char *_levelNames[] = {
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

static bool configBool(const char *value) {
	return strcasecmp(value, "true") == 0 || (strlen(value) == 2 && (value[0] == 't' || value[0] == '1'));
}

static int handleConfigIni(void *userdata, const char *section, const char *name, const char *value) {
	Game *g = (Game *)userdata;
	// fprintf(stdout, "config.ini: section '%s' name '%s' value '%s'\n", section, name, value);
	if (strcmp(section, "engine") == 0) {
		if (strcmp(name, "disable_paf") == 0) {
			g->_paf->_skipCutscenes = configBool(value);
		} else if (strcmp(name, "disable_mst") == 0) {
			g->_mstLogicDisabled = configBool(value);
		} else if (strcmp(name, "disable_sss") == 0) {
		}
	} else if (strcmp(section, "display") == 0) {
		if (strcmp(name, "scale_factor") == 0) {
			const int scale = atoi(value);
			_system->setScaler(0, scale);
		} else if (strcmp(name, "scale_algorithm") == 0) {
			_system->setScaler(value, 0);
		} else if (strcmp(name, "gamma") == 0) {
			_system->setGamma(atof(value));
		} else if (strcmp(name, "grayscale") == 0) {
			_system->setPaletteScale(configBool(value));
		}
	}
	return 0;
}

int main(int argc, char *argv[]) {
	char *dataPath = 0;
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
				for (int i = 0; _levelNames[i]; ++i) {
					if (strcmp(_levelNames[i], optarg) == 0) {
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
			fprintf(stdout, "%s\n", _usage);
			return -1;
		}
	}
	_system = SystemStub_SDL_create();
	atexit(exitMain);
	g_debugMask = 0; //kDebug_GAME | kDebug_RESOURCE | kDebug_SOUND;
	Game *g = new Game(_system, dataPath ? dataPath : _defaultDataPath);
	ini_parse(_configIni, handleConfigIni, g);
	g->mainLoop(level, checkpoint);
	delete g;
	free(dataPath);
	return 0;
}
