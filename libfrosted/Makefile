include ../kconfig/.config

ifeq ($(ARCH_SEEEDPRO),y)
	FAMILY=lpc17xx 
	ARCH=seedpro
	CFLAGS+=-DSEEEDPRO
endif

ifeq ($(ARCH_QEMU),y)
	FAMILY=stellaris
	ARCH=stellaris_qemu
	CFLAGS+=-DLM3S
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
		newlib_syscalls.o \
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
		dup.o		\
		dup2.o		\
		open.o		\
		read.o		\
		stat.o		\
		unlink.o	\
		wait.o		\
		write.o		\
		mem_init.o  \
		malloc.o	\
		free.o		\
		mkdir.o		\
		chdir.o		\
		getppid.o	\
		sleep.o		\
		getcwd.o	\
		opendir.o		\
		readdir.o		\
		closedir.o		\
		sem_init.o		\
		semaphore.o			\
		ioctl.o				\
		mutex_init.o		\
		mutex.o				\
		socket.o			\
		accept.o	\
		bind.o		\
		connect.o	\
		getsockopt.o		\
		listen.o	\
		recvfrom.o	\
		sendto.o	\
		setsockopt.o		\
		shutdown.o			\
		thread_create.o		\
		thread_join.o		\


CFLAGS+=$(CFLAGS-y)


all: $(OBJS-y)
	@echo -e "\t[AR] $(PREFIX)/lib/$(LIBNAME)"
	@mkdir -p $(PREFIX)/lib
	@$(AR) cru $(PREFIX)/lib/$(LIBNAME) $(OBJS-y)

clean:
	@rm -f $(OBJS-y)
	@rm -f $(PREFIX)/lib/$(LIBNAME)

