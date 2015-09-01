-include kconfig/.config

ifeq ($(ARCH_SEEDPRO),y)
	FAMILY=lpc17xx
	BOARD=seedpro
	CFLAGS+=-DSEEDPRO
endif

ifeq ($(ARCH_QEMU),y)
	FAMILY=stellaris
	BOARD=stellaris_qemu
	CFLAGS+=-DSTELLARIS
endif

ifeq ($(ARCH_STM32F4DISCOVERY),y)
	FAMILY=stm32f4
	BOARD=stm32f4discovery
	CFLAGS+=-DSTM32F4DISCOVERY -DSTM32F407xx
endif

ifeq ($(FRESH),y)
	CFLAGS+=-DCONFIG_FRESH=1
endif

ifeq ($(PRODCONS),y)
	CFLAGS+=-DCONFIG_PRODCONS=1
endif


-include board/$(BOARD)/layout.conf
FLASH_ORIGIN?=0x0
FLASH_SIZE?=256K
CFLAGS+=-DFLASH_ORIGIN=$(FLASH_ORIGIN)

CROSS_COMPILE?=arm-none-eabi-
CC:=$(CROSS_COMPILE)gcc
AS:=$(CROSS_COMPILE)as
CFLAGS+=-mcpu=cortex-m3 -mthumb -mlittle-endian -mthumb-interwork -Ikernel -DCORE_M3 -Iinclude -fno-builtin -ffreestanding -DKLOG_LEVEL=6
CFLAGS+=-Iboard/$(BOARD)/inc -Ifamily/$(FAMILY)/inc -Lfamily/$(FAMILY)/
PREFIX:=$(PWD)/build
LDFLAGS:=-gc-sections -nostartfiles -ggdb -L$(PREFIX)/lib 

#debugging
CFLAGS+=-ggdb

#optimization
#CFLAGS+=-Os

ASFLAGS:=-mcpu=cortex-m3 -mthumb -mlittle-endian -mthumb-interwork -ggdb
OBJS-y:=  family/$(FAMILY)/$(FAMILY).o	
APPS-y:= \
		apps/init.o apps/fresh.o

include family/$(FAMILY)/$(FAMILY).mk

all: image.bin

kernel/syscall_table.c: kernel/syscall_table_gen.py
	python2 $^

include/syscall_table.h: kernel/syscall_table.c

$(PREFIX)/lib/libkernel.a:
	make -C kernel

$(PREFIX)/lib/libfrosted.a:
	make -C libfrosted

image.bin: kernel.elf apps.elf
	$(CROSS_COMPILE)objcopy -O binary --pad-to=0x20000 kernel.elf $@
	$(CROSS_COMPILE)objcopy -O binary apps.elf apps.bin
	cat apps.bin >> $@


apps/apps.ld: apps/apps.ld.in
	cat $^ | sed -e "s/__FLASH_ORIGIN/$(FLASH_ORIGIN)/g" | sed -e "s/__FLASH_SIZE/$(FLASH_SIZE)/g" >$@


apps.elf: $(PREFIX)/lib/libfrosted.a $(APPS-y) apps/apps.ld
	$(CC) -o $@  $(APPS-y) -Tapps/apps.ld -lfrosted -lc -lfrosted -Wl,-Map,apps.map  $(LDFLAGS) $(CFLAGS) $(EXTRA_CFLAGS)


kernel.elf: $(PREFIX)/lib/libkernel.a $(OBJS-y)
	$(CC) -o $@   -Tfamily/$(FAMILY)/$(FAMILY).ld $(OBJS-y) -lkernel -Wl,-Map,kernel.map  $(LDFLAGS) $(CFLAGS) $(EXTRA_CFLAGS)

qemu: image.bin 
	qemu-system-arm -semihosting -M lm3s6965evb --kernel image.bin -serial stdio -S -gdb tcp::3333

qemu2: image.bin
	qemu-system-arm -semihosting -M lm3s6965evb --kernel image.bin -serial stdio

menuconfig:
	@$(MAKE) -C kconfig/ menuconfig -f Makefile.frosted

clean:
	@make -C kernel clean
	@make -C libfrosted clean
	@rm -f $(OBJS-y)
	@rm -f *.map *.bin *.elf
	@rm -f apps/apps.ld
	@find . |grep "\.o" | xargs -x rm -f

