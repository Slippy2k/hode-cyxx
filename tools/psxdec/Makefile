
ALL = decode_psx
CPPFLAGS += -g

all: $(ALL)

decode_psx: decode_psx.o screenshot.o libbs/vlc.o libbs/idctfst.o libbs/jfdctint.o libbs/bs.o
	$(CXX) -o $@ $^ -lavcodec -lavutil

clean:
	rm -f *.o $(ALL) libbs/*.o


