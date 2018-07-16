
CPPFLAGS += -g

decode_psx: decode_psx.o screenshot.o
	$(CXX) -o $@ $^ -lavcodec -lavutil

clean:
	rm -f *.o decode_psx
