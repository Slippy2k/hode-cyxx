
CFLAGS = -g -O -Wall -Wuninitialized -Wno-multichar

SRCS = bmp.cpp paf_decoder.cpp file.cpp main.cpp png.cpp wav.cpp

OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

paf_decoder: $(OBJS)
	$(CXX) -o $@ $^ $(SDL_LIBS) -lpng -lz

clean:
	rm -f *.o *.d

-include $(DEPS)
