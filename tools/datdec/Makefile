
DEFINES =

CC = g++
CFLAGS = -g -O -Wuninitialized -Wno-multichar

SRCS_LVL = file.cpp lvl.cpp unpack.cpp raw2png.cpp
OBJS_LVL = $(SRCS_LVL:.cpp=.o)
DEPS_LVL = $(SRCS_LVL:.cpp=.d)

SRCS_DAT = file.cpp dat.cpp unpack.cpp raw2png.cpp
OBJS_DAT = $(SRCS_DAT:.cpp=.o)
DEPS_DAT = $(SRCS_DAT:.cpp=.d)

all: lvl_decoder dat_decoder

lvl_decoder: $(OBJS_LVL)
	$(CC) -o $@ $^ -lpng -lz

dat_decoder: $(OBJS_DAT)
	$(CC) -o $@ $^ -lpng -lz

%.o: %.cpp
	$(CC) $(CFLAGS) $(DEFINES) -MMD -c $< -o $@

clean:
	rm -f *.o *.d
	strip *.exe

-include $(DEPS_LVL) $(DEPS_DAT)
