
CC = gcc
CFLAGS = -O -Wall -Wuninitialized -Wno-multichar
NASM = nasm
NASMFLAGS = -f elf32

OBJS = lut.o lut_i386.o
DEPS = lut.d

.SUFFIXES: .asm

lut: $(OBJS)
	$(CC) -m32 -o $@ $(OBJS)

.c.o:
	$(CC) -m32 $(CFLAGS) -MMD -c $< -o $*.o

.asm.o:
	$(NASM) $(NASMFLAGS) -o $*.o $(<)

clean:
	rm -f *.o *.d

-include $(DEPS)
