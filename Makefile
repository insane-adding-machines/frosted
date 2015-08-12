#FAMILY?=lpc17xx
#ARCH?=seedpro
FAMILY?=stellaris
ARCH?=stellaris_qemu
CROSS_COMPILE?=arm-none-eabi-
CC:=$(CROSS_COMPILE)gcc
AS:=$(CROSS_COMPILE)as
CFLAGS:=-mcpu=cortex-m3 -mthumb -mlittle-endian -mthumb-interwork -DARCH_$(ARCH) -Iarch/$(ARCH)/inc -Iport/$(FAMILY)/inc -Ikernel -DCORE_M3 -Iinclude -fno-builtin -ffreestanding -DKLOG_LEVEL=6
LDFLAGS:=-nostartfiles -ggdb -lc -lm -lnosys 
PREFIX=$(PWD)/build

#debugging
CFLAGS+=-ggdb

ASFLAGS:=-mcpu=cortex-m3 -mthumb -mlittle-endian -mthumb-interwork -ggdb
OBJS:=  port/$(FAMILY)/$(FAMILY).o	\
		apps/test.o

include port/$(FAMILY)/$(FAMILY).mk

all: image.elf

kernel/syscall_table.c: kernel/syscall_table_gen.py
	python2 $^

kernel/syscall_table.h: kernel/syscall_table.c

$(PREFIX)/build/lib/libfrosted.a:
	make -C kernel

image.elf: $(PREFIX)/build/lib/libfrosted.a $(OBJS) $(LIBS)
	$(CC) -o $@   -Wl,--start-group $(PREFIX)/lib/libfrosted.a $(OBJS) $(LIBS) -Wl,--end-group  -Tport/$(FAMILY)/$(FAMILY).ld  -Wl,-Map,image.map  $(LDFLAGS) $(CFLAGS) $(EXTRA_CFLAGS)

qemu: image.elf
	#qemu-system-arm -semihosting -M lm3s6965evb --kernel image.elf -nographic -S -s
	qemu-system-arm -semihosting -M lm3s6965evb --kernel image.elf -serial stdio -S -s

clean:
	@make -C kernel clean
	@rm -f $(OBJS)
	@rm -f image.elf image.map

