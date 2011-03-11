
CC = gcc
CFLAGS = -O -Wall -Wuninitialized -Wno-multichar
NASM = nasm
NASMFLAGS = -f elf

OBJS = run.o run_i386.o
DEPS = run.d

.SUFFIXES: .asm

run: $(OBJS)
	$(CC) -o $@ $(OBJS)

.c.o:
	$(CC) $(CFLAGS) -MMD -c $< -o $*.o

.asm.o:
	$(NASM) $(NASMFLAGS) -o $*.o $(<)

clean:
	rm -f *.o *.d

-include $(DEPS)
