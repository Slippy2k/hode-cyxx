
SDL_CFLAGS = `sdl2-config --cflags`
SDL_LIBS = `sdl2-config --libs`

CPPFLAGS = -g -O -Wall $(SDL_CFLAGS) $(DEFINES) -MMD

SRCS = andy.cpp benchmark.cpp fileio.cpp game.cpp level1.cpp level2.cpp level3.cpp level4.cpp \
	lzw.cpp main.cpp monsters.cpp paf.cpp random.cpp resource.cpp \
	scaler.cpp screenshot.cpp sound.cpp staticres.cpp systemstub_sdl.cpp \
	util.cpp video.cpp

SCALERS := scaler_nearest.cpp scaler_xbr.cpp

OBJS = $(SRCS:.cpp=.o) $(SCALERS:.cpp=.o) libxbr-standalone/xbr.o
DEPS = $(SRCS:.cpp=.d) $(SCALERS:.cpp=.d) libxbr-standalone/xbr.d

all: hode

hode: $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(SDL_LIBS)

clean:
	rm -f $(OBJS) $(DEPS)

-include $(DEPS)
