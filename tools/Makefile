
ALL = bitop dump_data gen_rename hod_extract opcode_map split_paf

all: $(ALL)

bitop: bitop.o
	$(CC) $^ -o $@

dump_data: dump_data.o
	$(CC) $^ -o $@

gen_rename: gen_rename.o
	$(CC) $^ -o $@

hod_extract: hod_extract.o
	$(CC) $^ -o $@

opcode_map: opcode_map.o
	$(CC) $^ -o $@

split_paf: split_paf.o
	$(CC) $^ -o $@

clean:
	rm -f *.o $(ALL)


