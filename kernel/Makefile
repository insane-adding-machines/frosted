include ../kconfig/.config
include ../config.mk
ifeq ($(ARCH_LPC17XX),y)
	CPU=cortex-m
	BOARD=lpc1768
	CFLAGS+=-mcpu=cortex-m3
	ARCH=LPC17XX
endif

ifeq ($(ARCH_LM3S),y)
	CPU=cortex-m
	BOARD=stellaris
	CFLAGS+=-mcpu=cortex-m3
	ARCH=LM3S
	SYS_CLOCK=50000000
endif

ifeq ($(ARCH_STM32F4),y)
	CPU=cortex-m
	BOARD=stm32f4
	CFLAGS+=-DSTM32F4 -mcpu=cortex-m4 -mfloat-abi=soft
	ARCH=STM32F4
endif
FLASH_ORIGIN?=0x0
FLASH_SIZE?=256K
CFLAGS+=-DFLASH_ORIGIN=$(FLASH_ORIGIN)

CROSS_COMPILE?=arm-none-eabi-
CC:=$(CROSS_COMPILE)gcc
AS:=$(CROSS_COMPILE)as
AR:=$(CROSS_COMPILE)ar
LIBNAME:=libkernel.a
PREFIX:=$(PWD)/build
#Target flags
CFLAGS+=-mthumb -mlittle-endian -mthumb-interwork
CFLAGS+=-DCORE_M3 -DBOARD_$(BOARD) -D$(ARCH)
CFLAGS+=-DKLOG_LEVEL=6
CFLAGS+=-DCONFIG_KMEM_SIZE=$(KMEM_SIZE)
#Include paths
CFLAGS+=-I. -I../include -I.. -Ilibopencm3/include/libopencm3 -Ilibopencm3/include
#Freestanding options
CFLAGS+=-fno-builtin
CFLAGS+=-ffreestanding
CFLAGS+=-nostdlib
#Debugging
CFLAGS+=-ggdb
#CFLAGS+=-Os

ASFLAGS:=-mcpu=cortex-m3 -mthumb -mlittle-endian -mthumb-interwork -ggdb


OBJS-y+= \
	svc.o				\
	frosted.o			\
	string.o			\
	sys.o				\
	syscall.o			\
	locks.o				\
	semaphore.o			\
	mutex.o				\
	tasklet.o			\
	scheduler.o			\
	syscall_table.o		\
	malloc.o			\
	module.o			\
	poll.o				\
	cirbuf.o			\
	timer.o
OBJS-y += vfs.o
OBJS-y += syscall_vector.o

CFLAGS+=$(CFLAGS-y)


all: syscall_table.h $(OBJS-y)
	@echo -e "\t[AR] $(PREFIX)/lib/$(LIBNAME)"
	@mkdir -p $(PREFIX)/lib
	$(AR) cr $(PREFIX)/lib/$(LIBNAME) $(OBJS-y)

syscall_table.c: syscall_table_gen.py
	python2 $^

syscall_table.h: syscall_table.c

clean:
	@rm -f $(OBJS-y)
	@rm -f $(PREFIX)/lib/$(LIBNAME)

