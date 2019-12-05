
ALL = bitop decode_pe.exe decode_mst decode_sss dump_data opcode_map

all: $(ALL)

bitop: bitop.o
	$(CC) $^ -o $@

decode_pe.exe: decode_pe.cpp
	i686-w64-mingw32-g++ -Wall $^ -o $@ -lversion

decode_mst: decode_mst.o
	$(CXX) $^ -o $@

decode_sss: decode_sss.o
	$(CXX) $^ -o $@

dump_data: dump_data.o
	$(CC) $^ -o $@ -lz

opcode_map: opcode_map.o
	$(CC) $^ -o $@

clean:
	rm -f *.o $(ALL)
