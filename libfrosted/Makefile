include ../kconfig/.config
include ../config.mk

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
CFLAGS+=-I. -I../include -I../newlib/include
#Freestanding options
CFLAGS+=-fno-builtin
CFLAGS+=-ffreestanding
#Debugging
CFLAGS+=-ggdb
ASFLAGS:=-mcpu=cortex-m3 -mthumb -mlittle-endian -mthumb-interwork -ggdb

OBJS-y+=						\
		accept.o				\
		bind.o					\
		closedir.o				\
		connect.o				\
		chdir.o					\
		chroot.o				\
		close.o					\
		dup.o					\
		dup2.o					\
		errno.o					\
		exec.o					\
		exit.o					\
		fork.o					\
		free.o					\
		fstat.o					\
		getcwd.o				\
		getlogin.o				\
		getpid.o				\
		getppid.o				\
		getsockopt.o			\
		grp.o					\
		ioctl.o					\
		isatty.o				\
		kill.o					\
		link.o					\
		lseek.o					\
		listen.o				\
		malloc.o				\
		mkdir.o					\
		mutex_init.o			\
		mutex.o					\
		mount.o					\
		newlib_syscalls.o		\
		open.o					\
		opendir.o				\
		pipe.o					\
		poll.o					\
		printf.o				\
		pwd.o					\
		read.o					\
		readdir.o				\
		recvfrom.o				\
		stat.o					\
		sem_init.o				\
		semaphore.o				\
		socket.o				\
		sendto.o				\
		setsockopt.o			\
		sleep.o					\
		shutdown.o				\
		syscall.o				\
		syscall_table.o			\
		svc.o	    			\
		ttyname.o				\
		thread_create.o			\
		thread_join.o			\
		tcsetattr.o				\
		unlink.o				\
		umount.o				\
		uid.o					\
		wait.o					\
		waitpid.o				\
		write.o					\
		posix/collate_error.o   \
		posix/collcmp.o			\
		posix/execl.o			\
		posix/execle.o			\
		posix/execlp.o			\
		posix/execv.o			\
		posix/execve.o			\
		posix/execvp.o			\
		posix/glob.o			\


CFLAGS+=$(CFLAGS-y)


all: $(OBJS-y)
	@echo -e "\t[AR] $(PREFIX)/lib/$(LIBNAME)"
	@mkdir -p $(PREFIX)/lib
	@$(AR) cru $(PREFIX)/lib/$(LIBNAME) $(OBJS-y)

clean:
	@rm -f syscall_table.c
	@rm -f $(OBJS-y)
	@rm -f $(PREFIX)/lib/$(LIBNAME)

