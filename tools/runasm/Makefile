
CC = gcc
CFLAGS = -O -Wall -Wuninitialized -Wno-multichar
NASM = nasm
NASMFLAGS = -f elf32

OBJS = run.o run_i386.o
DEPS = run.d

.SUFFIXES: .asm

run: $(OBJS)
	$(CC) -m32 -o $@ $(OBJS)

.c.o:
	$(CC) -m32 $(CFLAGS) -MMD -c $< -o $*.o

.asm.o:
	$(NASM) $(NASMFLAGS) -o $*.o $(<)

clean:
	rm -f *.o *.d

-include $(DEPS)
