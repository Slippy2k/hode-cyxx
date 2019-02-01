
SDL_CFLAGS = `sdl2-config --cflags`
SDL_LIBS = `sdl2-config --libs`

CPPFLAGS += -g -O -Wall -Wpedantic $(SDL_CFLAGS) $(DEFINES) -MMD

SRCS = andy.cpp benchmark.cpp fileio.cpp game.cpp \
	level1_rock.cpp level2_fort.cpp level3_pwr1.cpp level4_isld.cpp \
	level5_lava.cpp level6_pwr2.cpp level7_lar1.cpp level9_dark.cpp \
	lzw.cpp main.cpp monsters.cpp paf.cpp random.cpp resource.cpp \
	scaler.cpp screenshot.cpp sound.cpp staticres.cpp systemstub_sdl.cpp \
	util.cpp video.cpp

SCALERS := scaler_nearest.cpp scaler_xbr.cpp

OBJS = $(SRCS:.cpp=.o) $(SCALERS:.cpp=.o) 3p/libxbr-standalone/xbr.o
DEPS = $(SRCS:.cpp=.d) $(SCALERS:.cpp=.d) 3p/libxbr-standalone/xbr.d

all: hode

hode: $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(SDL_LIBS)

clean:
	rm -f $(OBJS) $(DEPS)

-include $(DEPS)
