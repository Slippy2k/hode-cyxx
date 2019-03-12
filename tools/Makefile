
ALL = bitop decode_sss dump_data dump_staticres hod_extract opcode_map split_paf

all: $(ALL)

bitop: bitop.o
	$(CC) $^ -o $@

decode_sss: decode_sss.o
	$(CXX) $^ -o $@

dump_data: dump_data.o
	$(CC) $^ -o $@

dump_staticres: dump_staticres.o
	$(CC) -o $@ $^ -lz

hod_extract: hod_extract.o
	$(CC) $^ -o $@

opcode_map: opcode_map.o
	$(CC) $^ -o $@

split_paf: split_paf.o
	$(CC) $^ -o $@

clean:
	rm -f *.o $(ALL)
