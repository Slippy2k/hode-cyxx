
CPPFLAGS += -g

decode_psx: decode_psx.o screenshot.o
	$(CXX) -o $@ $^ -lavcodec -lavutil -lcdio -liso9660

clean:
	rm -f *.o decode_psx
