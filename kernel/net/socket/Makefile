CROSS_COMPILE?=arm-none-eabi-
CC=$(CROSS_COMPILE)gcc

CFLAGS+="-I. -I$(PREFIX)/include -DFROSTED -DCORTEX_M3"

all: pico_bsd_sockets.o pico_osal_frosted.o

pico_bsd_socket.o: pico_bsd_socket.c
	$(CC) $(CFLAGS) -c -o $@ $^

pico_osal_frosted.o: pico_osal_frosted.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	@rm -f *.o

