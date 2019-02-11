
ALL = bitop decode_sss dump_data hod_extract opcode_map split_paf

all: $(ALL)

bitop: bitop.o
	$(CC) $^ -o $@

decode_sss: decode_sss.o
	$(CXX) $^ -o $@

dump_data: dump_data.o
	$(CC) $^ -o $@

hod_extract: hod_extract.o
	$(CC) $^ -o $@

opcode_map: opcode_map.o
	$(CC) $^ -o $@

split_paf: split_paf.o
	$(CC) $^ -o $@

clean:
	rm -f *.o $(ALL)
