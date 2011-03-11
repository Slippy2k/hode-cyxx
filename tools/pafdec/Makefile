
DEFINES =

CC = g++
CFLAGS = -g -O -Wall -Wuninitialized -Wno-multichar

SRCS = bmp.cpp paf_decoder.cpp file.cpp main.cpp png.cpp wav.cpp

OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

paf_decoder: $(OBJS)
	$(CC) -o $@ $^ $(SDL_LIBS) -lpng -lz

%.o: %.cpp
	$(CC) $(CFLAGS) $(DEFINES) -MMD -c $< -o $@

clean:
	rm -f *.o *.d

-include $(DEPS)
