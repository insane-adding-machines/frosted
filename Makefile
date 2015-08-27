-include kconfig/.config
#FAMILY?=lpc17xx
#ARCH?=seedpro
FAMILY?=stellaris
ARCH?=stellaris_qemu
CROSS_COMPILE?=arm-none-eabi-
CC:=$(CROSS_COMPILE)gcc
AS:=$(CROSS_COMPILE)as
CFLAGS:=-mcpu=cortex-m3 -mthumb -mlittle-endian -mthumb-interwork -Ikernel -DCORE_M3 -Iinclude -fno-builtin -ffreestanding -DKLOG_LEVEL=6
CFLAGS+=-Iarch/$(ARCH)/inc -Iport/$(FAMILY)/inc
PREFIX:=$(PWD)/build
#LDFLAGS:=-nostartfiles -ggdb -lc -lm -lsysfrosted -L$(PREFIX)/lib
LDFLAGS:=-nostartfiles -ggdb -lc -lm -lnosys -L$(PREFIX)/lib

#debugging
CFLAGS+=-ggdb

#optimization
#CFLAGS+=-Os

ASFLAGS:=-mcpu=cortex-m3 -mthumb -mlittle-endian -mthumb-interwork -ggdb
OBJS-y:=  port/$(FAMILY)/$(FAMILY).o	\
		apps/init.o apps/fresh.o

include port/$(FAMILY)/$(FAMILY).mk

all: image.elf

kernel/syscall_table.c: kernel/syscall_table_gen.py
	python2 $^

include/syscall_table.h: kernel/syscall_table.c

$(PREFIX)/lib/libfrosted.a:
	make -C kernel

$(PREFIX)/lib/libsysfrosted.a:
	make -C sysfrosted

image.elf: $(PREFIX)/lib/libfrosted.a $(PREFIX)/lib/libsysfrosted.a $(OBJS-y)
	$(CC) -o $@   -Wl,--start-group $(PREFIX)/lib/libfrosted.a $(OBJS-y) -Wl,--end-group  -Tport/$(FAMILY)/$(FAMILY).ld  -Wl,-Map,image.map  $(LDFLAGS) $(CFLAGS) $(EXTRA_CFLAGS)

qemu: image.elf
	qemu-system-arm -semihosting -M lm3s6965evb --kernel image.elf -serial stdio -S -s

qemu2: image.elf
	qemu-system-arm -semihosting -M lm3s6965evb --kernel image.elf -serial stdio

menuconfig:
	@$(MAKE) -C kconfig/ menuconfig -f Makefile.frosted

clean:
	@make -C kernel clean
	@make -C sysfrosted clean
	@rm -f $(OBJS-y)
	@rm -f image.elf image.map

