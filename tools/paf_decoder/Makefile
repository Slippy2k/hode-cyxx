
CPPFLAGS += -Wall -g

SRCS = bmp.cpp paf_decoder.cpp file.cpp main.cpp png.cpp wav.cpp

OBJS = $(SRCS:.cpp=.o)

paf_decoder: $(OBJS)
	$(CXX) -o $@ $^ $(SDL_LIBS) -lpng -lz

clean:
	rm -f $(OBJS)
