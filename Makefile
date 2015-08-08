#FAMILY?=lpc17xx
#ARCH?=seedpro
FAMILY?=stellaris
ARCH?=stellaris_qemu
CROSS_COMPILE?=arm-none-eabi-
CC:=$(CROSS_COMPILE)gcc
AS:=$(CROSS_COMPILE)as
CFLAGS:=-mcpu=cortex-m3 -mthumb -mlittle-endian -mthumb-interwork -DARCH_$(ARCH) -Iarch/$(ARCH)/inc -Iport/$(FAMILY)/inc -Ikernel -DCORE_M3
LDFLAGS:=-nostartfiles -lc -lm -lrdimon -ggdb

#debugging
CFLAGS+=-ggdb

ASFLAGS:=-mcpu=cortex-m3 -mthumb -mlittle-endian -mthumb-interwork -ggdb
OBJS:=	kernel/svc.o			\
	kernel/frosted.o			\
	port/$(FAMILY)/$(FAMILY).o	\
	kernel/sys.o				\
	kernel/systick.o			\
	kernel/syscall.o			\
	kernel/timer.o				\
	kernel/scheduler.o			\
	kernel/syscall_table.o		\
	kernel/vfs.o				\
	kernel/newlib_redirect.o	\
	kernel/malloc.o				\
	kernel/module.o				\
	kernel/drivers/memfs.o		\
	kernel/drivers/null.o		\
	kernel/drivers/uart.o


include port/$(FAMILY)/$(FAMILY).mk

all: image.elf

kernel/syscall_table.c: kernel/syscall_table_gen.py
	python2 $^

kernel/syscall_table.h: kernel/syscall_table.c

image.elf: kernel/syscall_table.h $(OBJS) $(LIBS)
	$(CC) -o $@   -Wl,--start-group  $(OBJS) $(LIBS) -Wl,--end-group  -Tport/$(FAMILY)/$(FAMILY).ld  -Wl,-Map,image.map  $(LDFLAGS) $(CFLAGS) $(EXTRA_CFLAGS)

qemu: image.elf
	#qemu-system-arm -semihosting -M lm3s6965evb --kernel image.elf -nographic -S -s
	qemu-system-arm -semihosting -M lm3s6965evb --kernel image.elf -serial stdio -S -s

clean:
	@rm -f $(OBJS)
	@rm -f image.elf image.map

