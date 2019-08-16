
CPPFLAGS += -Wall

hod_decoder: decode_sss.o decode_mst.o fileio.o lzw.o main.o screenshot.o wav.o
	$(CXX) -o $@ $^ -lz

clean:
	rm -f *.o hod_decoder
