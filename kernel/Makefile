include ../kconfig/.config
include ../config.mk

FLASH_ORIGIN?=0x0
FLASH_SIZE?=256K
CFLAGS+=-DFLASH_ORIGIN=$(FLASH_ORIGIN)

CROSS_COMPILE?=arm-none-eabi-
CC:=$(CROSS_COMPILE)gcc
AS:=$(CROSS_COMPILE)as
AR:=$(CROSS_COMPILE)ar
LIBNAME:=libkernel.a
PREFIX:=$(PWD)/build

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


###### STACK SIZE #######
ifeq ($(TASK_STACK_SIZE_1K),y)
    TASK_STACK_SIZE=1024
endif
ifeq ($(TASK_STACK_SIZE_2K),y)
    TASK_STACK_SIZE=2048
endif
ifeq ($(TASK_STACK_SIZE_4K),y)
    TASK_STACK_SIZE=4096
endif
ifeq ($(TASK_STACK_SIZE_8K),y)
    TASK_STACK_SIZE=8192
endif
#########################
#Target flags
CFLAGS+=-mthumb -mlittle-endian -mthumb-interwork
CFLAGS+=-DCORE_M3 -DBOARD_$(BOARD) -D$(ARCH)
CFLAGS+=-DCONFIG_KMEM_SIZE=$(KMEM_SIZE)
CFLAGS+=-DCONFIG_TASK_STACK_SIZE=$(TASK_STACK_SIZE)
CFLAGS-$(KLOG)+=-DCONFIG_KLOG
CFLAGS+=-DCONFIG_KLOG_DEV=\"$(KLOG_DEV)\"
CFLAGS+=$(CFLAGS-y)
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
	frosted.o			\
	mpu.o				\
	string.o			\
	sys.o				\
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
	timer.o				\
	term.o				\
	bflt.o				\
	kprintf.o			\
	pipe.o

OBJS-y += vfs.o

CFLAGS+=$(CFLAGS-y)

ifeq ($(PICOTCP),y)
	CFLAGS+=-DCONFIG_PICOTCP -I$(PREFIX)/include -Inet/socket
    PICO_OPTIONS=CROSS_COMPILE=arm-none-eabi- ARCH=cortexm3 RTOS=1 PREFIX=$(PREFIX) \
    		 DHCP_CLIENT=0 DHCP_SERVER=0 MDNS=0 DNS_SD=0 \
    			 OLSR=0 SLAACV4=0 SNTP_CLIENT=0 PPP=0 TFTP=0 \
				 AODV=0 NAT=0 IPFILTER=0 DNSCLIENT=0\
				 EXTRA_CFLAGS="-DFROSTED -I$(PWD)/kernel -I$(PWD)/include -nostdlib"

ifneq ($(CONFIG_PICOTCP_IPV6),y)
  PICO_OPTIONS+=IPV6=0
endif
    BUILD_PICO=make -C net/picotcp $(PICO_OPTIONS)
    BUILD_SOCK=make -C net/socket CFLAGS="$(CFLAGS) -I$(PREFIX)/include -I$(PWD)/kernel -I$(PWD)/include -DFROSTED -DCORTEX_M3 -nostdlib"
endif



all: syscall_table.h $(OBJS-y)
	@echo -e "\t[AR] $(PREFIX)/lib/$(LIBNAME)"
	@$(BUILD_PICO)
	$(BUILD_SOCK)
	@mkdir -p $(PREFIX)/lib
	$(AR) cr $(PREFIX)/lib/$(LIBNAME) $(OBJS-y)

syscall_table.c: syscall_table_gen.py
	python2 $^

picotcp:
	@$(BUILD_PICO)

syscall_table.h: syscall_table.c

clean:
	@rm -f syscall_table.c
	@rm -f $(OBJS-y)
	@rm -f $(PREFIX)/lib/$(LIBNAME)
	@make -C net/socket clean

