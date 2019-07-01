
ALL = bitop decode_mst decode_sss dump_data dump_staticres opcode_map

all: $(ALL)

bitop: bitop.o
	$(CC) $^ -o $@

decode_mst: decode_mst.o
	$(CXX) $^ -o $@

decode_sss: decode_sss.o
	$(CXX) $^ -o $@

dump_data: dump_data.o
	$(CC) $^ -o $@

dump_staticres: dump_staticres.o
	$(CC) -o $@ $^ -lz

opcode_map: opcode_map.o
	$(CC) $^ -o $@

clean:
	rm -f *.o $(ALL)
