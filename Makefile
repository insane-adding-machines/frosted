-include kconfig/.config
-include config.mk

ifeq ($(FRESH),y)
	CFLAGS+=-DCONFIG_FRESH=1
endif

ifeq ($(PRODCONS),y)
	CFLAGS+=-DCONFIG_PRODCONS=1
endif

CROSS_COMPILE?=arm-none-eabi-
CC:=$(CROSS_COMPILE)gcc
AS:=$(CROSS_COMPILE)as
AR:=$(CROSS_COMPILE)ar
CFLAGS+=-mthumb -mlittle-endian -mthumb-interwork -Ikernel/libopencm3/include -Ikernel -DCORE_M3 -Iinclude -fno-builtin -ffreestanding -DKLOG_LEVEL=6 -DSYS_CLOCK=$(SYS_CLOCK)
PREFIX:=$(PWD)/build
LDFLAGS:=-gc-sections -nostartfiles -ggdb -L$(PREFIX)/lib 

#debugging
CFLAGS+=-ggdb

#optimization
#CFLAGS+=-Os

ASFLAGS:=-mcpu=cortex-m3 -mthumb -mlittle-endian -mthumb-interwork -ggdb
APPS-y:= apps/init.o 
APPS-$(FRESH)+=apps/fresh.o


OBJS-y:=kernel/systick.o

# device drivers 
OBJS-$(MEMFS)+= kernel/drivers/memfs.o
OBJS-$(XIPFS)+= kernel/drivers/xipfs.o
CFLAGS-$(MEMFS)+=-DCONFIG_MEMFS

OBJS-$(SYSFS)+= kernel/drivers/sysfs.o
CFLAGS-$(SYSFS)+=-DCONFIG_SYSFS

OBJS-$(DEVNULL)+= kernel/drivers/null.o
CFLAGS-$(DEVNULL)+=-DCONFIG_DEVNULL



OBJS-$(SOCK_UNIX)+= kernel/drivers/socket_un.o
CFLAGS-$(SOCK_UNIX)+=-DCONFIG_SOCK_UNIX

OBJS-$(DEVUART)+= kernel/drivers/uart.o
CFLAGS-$(DEVUART)+=-DCONFIG_DEVUART

OBJS-$(DEVGPIO)+=kernel/drivers/gpio.o
CFLAGS-$(DEVGPIO)+=-DCONFIG_DEVGPIO

OBJS-$(MACH_STM32F407Discovery)+=kernel/$(BOARD)/stm32f407discovery.o 
OBJS-$(MACH_STM32F405Pyboard)+=kernel/$(BOARD)/stm32f405pyboard.o 
OBJS-$(MACH_STM32F4x1Discovery)+=kernel/$(BOARD)/stm32f4x1discovery.o 
OBJS-$(MACH_STM32F429Discovery)+=kernel/$(BOARD)/stm32f429discovery.o 
OBJS-$(MACH_LPC1768MBED)+=kernel/$(BOARD)/lpc1768mbed.o
OBJS-$(MACH_SEEEDPRO)+=kernel/$(BOARD)/lpc1768mbed.o
OBJS-$(MACH_LPC1679XPRESSO)+=kernel/$(BOARD)/lpc1769xpresso.o
OBJS-$(MACH_LM3S6965EVB)+=kernel/$(BOARD)/lm3s6965evb.o

CFLAGS+=$(CFLAGS-y)

SHELL=/bin/bash
APPS_START = 0x20000
PADTO = $$(($(FLASH_ORIGIN)+$(APPS_START)))

all: image.bin

kernel/syscall_table.c: kernel/syscall_table_gen.py
	python2 $^


include/syscall_table.h: kernel/syscall_table.c

.PHONY: FORCE

$(PREFIX)/lib/libkernel.a: FORCE
	make -C kernel

$(PREFIX)/lib/libfrosted.a: FORCE
	make -C libfrosted

image.bin: kernel.elf apps.elf
	export PADTO=`python2 -c "print ( $(KFLASHMEM_SIZE) * 1024) + int('$(FLASH_ORIGIN)', 16)"`;	\
	$(CROSS_COMPILE)objcopy -O binary --pad-to=$$PADTO kernel.elf $@
	#$(CROSS_COMPILE)objcopy -O binary --pad-to=0x40000 apps.elf apps.bin
	#$(CROSS_COMPILE)objcopy -O binary apps/busybox/busybox_unstripped apps.bin
	#cat apps.bin >> $@
	cat apps/busybox/busybox_unstripped >> $@

apps/apps.ld: apps/apps.ld.in
	export KMEM_SIZE_B=`python2 -c "print '0x%X' % ( $(KFLASHMEM_SIZE) * 1024)"`;	\
	export AMEM_SIZE_B=`python2 -c "print '0x%X' % ( ($(RAM_SIZE) - $(KRAMMEM_SIZE)) * 1024)"`;	\
	export KFLASHMEM_SIZE_B=`python2 -c "print '0x%X' % ( $(KFLASHMEM_SIZE) * 1024)"`;	\
	export AFLASHMEM_SIZE_B=`python2 -c "print '0x%X' % ( ($(FLASH_SIZE) - $(KFLASHMEM_SIZE)) * 1024)"`;	\
	export KRAMMEM_SIZE_B=`python2 -c "print '0x%X' % ( $(KRAMMEM_SIZE) * 1024)"`;	\
	cat $^ | sed -e "s/__FLASH_ORIGIN/$(FLASH_ORIGIN)/g" | \
			 sed -e "s/__KFLASHMEM_SIZE/$$KFLASHMEM_SIZE_B/g" | \
			 sed -e "s/__AFLASHMEM_SIZE/$$AFLASHMEM_SIZE_B/g" | \
			 sed -e "s/__RAM_BASE/$(RAM_BASE)/g" |\
			 sed -e "s/__KRAMMEM_SIZE/$$KRAMMEM_SIZE_B/g" |\
			 sed -e "s/__AMEM_SIZE/$$AMEM_SIZE_B/g" \
			 >$@


apps.elf: $(PREFIX)/lib/libfrosted.a $(APPS-y) apps/apps.ld
	$(CC) -o $@  $(APPS-y) -Tapps/apps.ld -lfrosted -lc -lfrosted -Wl,-Map,apps.map  $(LDFLAGS) $(CFLAGS) $(EXTRA_CFLAGS)

kernel/libopencm3/lib/libopencm3_$(BOARD).a:
	make -C kernel/libopencm3 $(OPENCM3FLAGS)

$(PREFIX)/lib/libkernel.a: kernel/libopencm3/lib/libopencm3_$(BOARD).a

kernel/$(BOARD)/$(BOARD).ld: kernel/$(BOARD)/$(BOARD).ld.in
	export KRAMMEM_SIZE_B=`python2 -c "print '0x%X' % ( $(KRAMMEM_SIZE) * 1024)"`;	\
	export KFLASHMEM_SIZE_B=`python2 -c "print '0x%X' % ( $(KFLASHMEM_SIZE) * 1024)"`;	\
	cat $^ | sed -e "s/__FLASH_ORIGIN/$(FLASH_ORIGIN)/g" | \
			 sed -e "s/__KFLASHMEM_SIZE/$$KFLASHMEM_SIZE_B/g" | \
			 sed -e "s/__RAM_BASE/$(RAM_BASE)/g" |\
			 sed -e "s/__KRAMMEM_SIZE/$$KRAMMEM_SIZE_B/g" \
			 >$@

kernel.elf: $(PREFIX)/lib/libkernel.a $(OBJS-y) kernel/libopencm3/lib/libopencm3_$(BOARD).a kernel/$(BOARD)/$(BOARD).ld
	$(CC) -o $@   -Tkernel/$(BOARD)/$(BOARD).ld -Wl,--start-group $(PREFIX)/lib/libkernel.a $(OBJS-y) kernel/libopencm3/lib/libopencm3_$(BOARD).a -Wl,--end-group \
		-Wl,-Map,kernel.map  $(LDFLAGS) $(CFLAGS) $(EXTRA_CFLAGS)
	
apps/busybox/busybox: busybox

busybox:
	CROSS_COMPILE=arm-none-eabi- make -C apps/busybox

qemu: image.bin 
	qemu-system-arm -semihosting -M lm3s6965evb --kernel image.bin -serial stdio -S -gdb tcp::3333

qemu2: image.bin
	qemu-system-arm -semihosting -M lm3s6965evb --kernel image.bin -serial stdio

menuconfig:
	@$(MAKE) -C kconfig/ menuconfig -f Makefile.frosted

libclean:
	@make -C kernel/libopencm3 clean

clean:
	rm -f  kernel/$(BOARD)/$(BOARD).ld
	@make -C kernel clean
	@make -C libfrosted clean
	@rm -f $(OBJS-y)
	@rm -f *.map *.bin *.elf
	@rm -f apps/apps.ld
	@rm -f kernel/$(BOARD)/$(BOARD).ld
	@find . |grep "\.o" | xargs -x rm -f

