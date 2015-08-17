#FAMILY?=lpc17xx
#ARCH?=seedpro
FAMILY?=stellaris
ARCH?=stellaris_qemu
CROSS_COMPILE?=arm-none-eabi-
CC:=$(CROSS_COMPILE)gcc
AS:=$(CROSS_COMPILE)as
AR:=$(CROSS_COMPILE)ar
LIBNAME:=libfrosted.a
PREFIX:=$(PWD)/build
#Target flags
CFLAGS+=-mcpu=cortex-m3 -mthumb -mlittle-endian -mthumb-interwork
CFLAGS+=-DCORE_M3 -DARCH_$(ARCH)
CFLAGS+=-DKLOG_LEVEL=6
#Include paths
CFLAGS+=-I. -I../include -I../flibc -I../arch/$(ARCH)/inc -I../port/$(FAMILY)/inc
#Freestanding options
CFLAGS+=-fno-builtin
CFLAGS+=-ffreestanding
#Debugging
CFLAGS+=-ggdb
ASFLAGS:=-mcpu=cortex-m3 -mthumb -mlittle-endian -mthumb-interwork -ggdb

OBJS:=	svc.o			\
	frosted.o			\
	sys.o				\
	systick.o			\
	syscall.o			\
	timer.o				\
	scheduler.o			\
	syscall_table.o		\
	vfs.o				\
	newlib_redirect.o	\
	malloc.o			\
	module.o			\
	poll.o				\
	drivers/memfs.o		\
	drivers/sysfs.o		\
	drivers/null.o		\
	drivers/uart.o 		\
	../flibc/memset.o


all: syscall_table.h $(OBJS)
	@echo -e "\t[AR] $(PREFIX)/lib/$(LIBNAME)"
	@mkdir -p $(PREFIX)/lib
	@$(AR) cru $(PREFIX)/lib/$(LIBNAME) $(OBJS)

syscall_table.c: syscall_table_gen.py
	python2 $^

syscall_table.h: syscall_table.c

clean:
	@rm -f $(OBJS)
	@rm -f $(PREFIX)/lib/$(LIBNAME)

