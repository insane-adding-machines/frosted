include ../kconfig/.config

ifeq ($(ARCH_SEEDPRO),y)
	FAMILY=lpc17xx 
	ARCH=seedpro
endif

ifeq ($(ARCH_QEMU),y)
	FAMILY=stellaris
	ARCH=stellaris_qemu
endif

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
CFLAGS+=-I. -I../include -I../arch/$(ARCH)/inc -I../port/$(FAMILY)/inc
#Freestanding options
CFLAGS+=-fno-builtin
CFLAGS+=-ffreestanding
#Debugging
CFLAGS+=-ggdb
ASFLAGS:=-mcpu=cortex-m3 -mthumb -mlittle-endian -mthumb-interwork -ggdb

OBJS-y+=	svc.o			\
	frosted.o			\
	string.o			\
	sys.o				\
	systick.o			\
	syscall.o			\
	locks.o				\
	semaphore.o			\
	mutex.o				\
	tasklet.o			\
	scheduler.o			\
	syscall_table.o		\
	interrupts.o		\
	newlib_redirect.o	\
	malloc.o			\
	module.o			\
	poll.o				\
	timer.o
OBJS-y += vfs.o
OBJS-y += syscall_vector.o

OBJS-$(MEMFS)+= drivers/memfs.o
CFLAGS-$(MEMFS)+=-DCONFIG_MEMFS

OBJS-$(SYSFS)+= drivers/sysfs.o
CFLAGS-$(SYSFS)+=-DCONFIG_SYSFS

OBJS-$(DEVNULL)+= drivers/null.o
CFLAGS-$(DEVNULL)+=-DCONFIG_DEVNULL

OBJS-$(DEVUART)+= drivers/uart.o
CFLAGS-$(DEVUART)+=-DCONFIG_DEVUART

OBJS-$(SOCK_UNIX)+= drivers/socket_un.o
CFLAGS-$(SOCK_UNIX)+=-DCONFIG_SOCK_UNIX

CFLAGS+=$(CFLAGS-y)


all: syscall_table.h $(OBJS-y)
	@echo -e "\t[AR] $(PREFIX)/lib/$(LIBNAME)"
	@mkdir -p $(PREFIX)/lib
	@$(AR) cru $(PREFIX)/lib/$(LIBNAME) $(OBJS-y)

syscall_table.c: syscall_table_gen.py
	python2 $^

syscall_table.h: syscall_table.c

clean:
	@rm -f $(OBJS-y)
	@rm -f $(PREFIX)/lib/$(LIBNAME)

