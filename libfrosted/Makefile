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
CFLAGS+=-I. -I../include
#Freestanding options
CFLAGS+=-fno-builtin
CFLAGS+=-ffreestanding
#Debugging
CFLAGS+=-ggdb
ASFLAGS:=-mcpu=cortex-m3 -mthumb -mlittle-endian -mthumb-interwork -ggdb

OBJS-y+=			\
		close.o		\
		errno.o		\
		_exit.o		\
		fork.o		\
		fstat.o		\
		getpid.o	\
		isatty.o	\
		kill.o		\
		link.o		\
		lseek.o		\
		open.o		\
		read.o		\
		stat.o		\
		unlink.o	\
		wait.o		\
		write.o		\
		malloc.o	\
		free.o		\
		newlib_syscalls.o

CFLAGS+=$(CFLAGS-y)


all: $(OBJS-y)
	@echo -e "\t[AR] $(PREFIX)/lib/$(LIBNAME)"
	@mkdir -p $(PREFIX)/lib
	@$(AR) cru $(PREFIX)/lib/$(LIBNAME) $(OBJS-y)

clean:
	@rm -f $(OBJS-y)
	@rm -f $(PREFIX)/lib/$(LIBNAME)

