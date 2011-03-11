
SDL_CFLAGS = `sdl-config --cflags`
SDL_LIBS = `sdl-config --libs`

DEFINES =

CC = g++
CFLAGS = -g -O -Wall -Wuninitialized -Wno-multichar

SRCS = bmp.cpp paf_player.cpp paf_decoder.cpp file.cpp main.cpp png.cpp systemstub_sdl.cpp wav.cpp

OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

pafply: $(OBJS)
	$(CC) -o $@ $^ $(SDL_LIBS) -lpng -lz

%.o: %.cpp
	$(CC) $(CFLAGS) $(SDL_CFLAGS) $(DEFINES) -MMD -c $< -o $@

clean:
	rm -f *.o *.d

-include $(DEPS)
