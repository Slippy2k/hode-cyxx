
CPPFLAGS = -g -O -Wuninitialized -Wno-multichar -Wall

SRCS_LVL = file.cpp lvl.cpp unpack.cpp raw2png.cpp
OBJS_LVL = $(SRCS_LVL:.cpp=.o)
DEPS_LVL = $(SRCS_LVL:.cpp=.d)

SRCS_DAT = file.cpp dat.cpp unpack.cpp raw2png.cpp
OBJS_DAT = $(SRCS_DAT:.cpp=.o)
DEPS_DAT = $(SRCS_DAT:.cpp=.d)

all: lvl_decoder dat_decoder

lvl_decoder: $(OBJS_LVL)
	$(CXX) -o $@ $^ -lpng -lz

dat_decoder: $(OBJS_DAT)
	$(CXX) -o $@ $^ -lpng -lz

clean:
	rm -f *.o *.d

-include $(DEPS_LVL) $(DEPS_DAT)
