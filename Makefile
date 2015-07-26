FAMILY?=lpc17xx
ARCH?=seedpro
#FAMILY?=stellaris
#ARCH?=stellaris_qemu
CROSS_COMPILE?=arm-none-eabi-
CC:=$(CROSS_COMPILE)gcc
AS:=$(CROSS_COMPILE)as
CFLAGS:=-mcpu=cortex-m3 -mthumb -mlittle-endian -mthumb-interwork -DARCH_$(ARCH) -Iarch/$(ARCH)/inc -Ilib/$(FAMILY)/inc -DCORE_M3
LDFLAGS:=-nostartfiles -lc -lm -lrdimon -ggdb

#debugging
CFLAGS+=-ggdb

ASFLAGS:=-mcpu=cortex-m3 -mthumb -mlittle-endian -mthumb-interwork -ggdb
OBJS:=svc.o frosted.o lib/$(FAMILY)/$(FAMILY).o  sys.o systick.o syscall.o timer.o scheduler.o syscall_table.o sbrk.o


include lib/$(FAMILY)/$(FAMILY).mk

all: image.elf

syscall_table.c: syscall_table_gen.py
	python2 $^

syscall_table.h: syscall_table.c

image.elf: syscall_table.h $(OBJS) $(LIBS)
	$(CC) -o $@   -Wl,--start-group  $(OBJS) $(LIBS) -Wl,--end-group  -Tlib/$(FAMILY)/$(FAMILY).ld  -Wl,-Map,image.map  $(LDFLAGS) $(CFLAGS) $(EXTRA_CFLAGS)

qemu: image.elf
	qemu-system-arm -semihosting -M lm3s6965evb --kernel image.elf --serial null -nographic -S -s

clean:
	@rm -f $(OBJS)
	@rm -f image.elf image.map

