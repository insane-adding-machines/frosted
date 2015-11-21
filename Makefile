-include kconfig/.config

ifeq ($(ARCH_SEEEDPRO),y)
	CPU=cortex-m
	BOARD=lpc1768
	RAM_BASE=0x10000000
	CFLAGS+=-DSEEEDPRO -mcpu=cortex-m3
endif

ifeq ($(ARCH_QEMU),y)
	CPU=cortex-m
	BOARD=lm3s
	CFLAGS+=-DSTELLARIS -mcpu=cortex-m3
endif

ifeq ($(ARCH_STM32F4),y)
	CPU=cortex-m
	BOARD=stm32f4
	CFLAGS+=-DSTM32F4 -mcpu=cortex-m4 -mfloat-abi=soft
	FLASH_SIZE=1024K
	FLASH_ORIGIN=0x08000000
endif

ifeq ($(FRESH),y)
	CFLAGS+=-DCONFIG_FRESH=1
endif

ifeq ($(PRODCONS),y)
	CFLAGS+=-DCONFIG_PRODCONS=1
endif


RAM_BASE?=0x20000000
FLASH_ORIGIN?=0x0
FLASH_SIZE?=256K
CFLAGS+=-DFLASH_ORIGIN=$(FLASH_ORIGIN)

CROSS_COMPILE?=arm-none-eabi-
CC:=$(CROSS_COMPILE)gcc
AS:=$(CROSS_COMPILE)as
CFLAGS+=-mthumb -mlittle-endian -mthumb-interwork -Ikernel -DCORE_M3 -Iinclude -fno-builtin -ffreestanding -DKLOG_LEVEL=6
PREFIX:=$(PWD)/build
LDFLAGS:=-gc-sections -nostartfiles -ggdb -L$(PREFIX)/lib 

#debugging
CFLAGS+=-ggdb

#optimization
#CFLAGS+=-Os

ASFLAGS:=-mcpu=cortex-m3 -mthumb -mlittle-endian -mthumb-interwork -ggdb
APPS-y:= apps/init.o 
APPS-$(FRESH)+=apps/fresh.o


OBJS-y:=

# device drivers 
OBJS-$(MEMFS)+= kernel/drivers/memfs.o
CFLAGS-$(MEMFS)+=-DCONFIG_MEMFS

OBJS-$(SYSFS)+= kernel/drivers/sysfs.o
CFLAGS-$(SYSFS)+=-DCONFIG_SYSFS

OBJS-$(DEVNULL)+= kernel/drivers/null.o
CFLAGS-$(DEVNULL)+=-DCONFIG_DEVNULL



OBJS-$(SOCK_UNIX)+= kernel/drivers/socket_un.o
CFLAGS-$(SOCK_UNIX)+=-DCONFIG_SOCK_UNIX

OBJS-$(DEVUART)+= kernel/drivers/uart.o
CFLAGS-$(DEVUART)+=-DCONFIG_DEVUART

OBJS-$(GPIO_SEEEDPRO)+=kernel/drivers/gpio/gpio_seeedpro.o
CFLAGS-$(GPIO_SEEEDPRO)+=-DCONFIG_GPIO_SEEEDPRO

OBJS-$(GPIO_STM32F4)+=kernel/drivers/gpio/gpio_stm32f4.o
CFLAGS-$(GPIO_STM32F4)+=-DCONFIG_GPIO_STM32F4

CFLAGS+=$(CFLAGS-y)

SHELL=/bin/bash
APPS_START = 0x20000
PADTO = $$(($(FLASH_ORIGIN)+$(APPS_START)))

all: image.bin

kernel/syscall_table.c: kernel/syscall_table_gen.py
	python2 $^


include/syscall_table.h: kernel/syscall_table.c

$(PREFIX)/lib/libkernel.a:
	make -C kernel

$(PREFIX)/lib/libfrosted.a:
	make -C libfrosted

image.bin: kernel.elf apps.elf
	$(CROSS_COMPILE)objcopy -O binary --pad-to=$(PADTO) kernel.elf $@
	$(CROSS_COMPILE)objcopy -O binary apps.elf apps.bin
	cat apps.bin >> $@


apps/apps.ld: apps/apps.ld.in
	export KMEM_SIZE_B=`expr $(KMEM_SIZE) \* 1024`; \
	export KMEM_SIZE_B_HEX=`printf 0x%X $$KMEM_SIZE_B`;	\
	cat $^ | sed -e "s/__FLASH_ORIGIN/$(FLASH_ORIGIN)/g" | \
			 sed -e "s/__FLASH_SIZE/$(FLASH_SIZE)/g" | \
			 sed -e "s/__RAM_BASE/$(RAM_BASE)/g" |\
			 sed -e "s/__KMEM_SIZE/` printf 0x%x $$KMEM_SIZE_B_HEX`/g" \
			 >$@


apps.elf: $(PREFIX)/lib/libfrosted.a $(APPS-y) apps/apps.ld
	$(CC) -o $@  $(APPS-y) -Tapps/apps.ld -lfrosted -lc -lfrosted -Wl,-Map,apps.map  $(LDFLAGS) $(CFLAGS) $(EXTRA_CFLAGS)

kernel/libopencm3/lib/libopencm3_$(BOARD).a:
	make -C kernel/libopencm3

$(PREFIX)/lib/libkernel.a: kernel/libopencm3/lib/libopencm3_$(BOARD).a

kernel.elf: $(PREFIX)/lib/libkernel.a $(OBJS-y) kernel/libopencm3/lib/libopencm3_$(BOARD).a
	$(CC) -o $@   -Tkernel/hal/arch/$(BOARD).ld -Wl,--start-group $^ -Wl,--end-group \
		-Wl,-Map,kernel.map  $(LDFLAGS) $(CFLAGS) $(EXTRA_CFLAGS)

qemu: image.bin 
	qemu-system-arm -semihosting -M lm3s6965evb --kernel image.bin -serial stdio -S -gdb tcp::3333

qemu2: image.bin
	qemu-system-arm -semihosting -M lm3s6965evb --kernel image.bin -serial stdio

menuconfig:
	@$(MAKE) -C kconfig/ menuconfig -f Makefile.frosted

clean:
	@make -C kernel clean
	@make -C kernel/libopencm3 clean
	@make -C libfrosted clean
	@rm -f $(OBJS-y)
	@rm -f *.map *.bin *.elf
	@rm -f apps/apps.ld
	@find . |grep "\.o" | xargs -x rm -f

