
SDL_CFLAGS = `sdl2-config --cflags`
SDL_LIBS = `sdl2-config --libs`

CXXFLAGS = -g -O -Wall $(SDL_CFLAGS) $(DEFINES)

OBJDIR = obj

SRCS = andy.cpp benchmark.cpp fileio.cpp game.cpp level1.cpp level2.cpp level3.cpp level4.cpp \
	lzw.cpp main.cpp monsters.cpp paf.cpp random.cpp resource.cpp \
	scaler.cpp sound.cpp staticres.cpp systemstub_sdl.cpp \
	util.cpp video.cpp

SCALERS := scaler_nearest.cpp scaler_xbrz.cpp xbrz/xbrz.cpp

OBJS = $(SRCS:.cpp=.o) $(SCALERS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d) $(SCALERS:.cpp=.d)

all: $(OBJDIR) hode

hode: $(addprefix $(OBJDIR)/, $(OBJS))
	$(CXX) $(LDFLAGS) -o $@ $^ $(SDL_LIBS) $(VORBIS_LIBS)

$(OBJDIR):
	mkdir -p $(OBJDIR)/xbrz/

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $@

clean:
	rm -f $(OBJDIR)/*.o $(OBJDIR)/*.d

-include $(addprefix $(OBJDIR)/, $(DEPS))
