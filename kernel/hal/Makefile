CC=gcc
CPU?=cortex-m
BOARD?=stellaris
CFLAGS+=-ggdb

all: libsysreg.a


libsysreg.a: regs.o $(CPU).o $(BOARD).o tools.o
	ar cr  $@ $^

test: libsysreg.a test.o



clean:
	rm -f *.o *.a regs.c regs.h
