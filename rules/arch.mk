
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
CROSS_COMPILE?=arm-none-eabi-
CC:=$(CROSS_COMPILE)gcc
AS:=$(CROSS_COMPILE)as
AR:=$(CROSS_COMPILE)ar
CFLAGS+=-mthumb -mlittle-endian -mthumb-interwork -DCORE_M3 -fno-builtin -ffreestanding -DSYS_CLOCK=$(SYS_CLOCK) -DCORTEX_M3 -DFROSTED
CFLAGS+=-Ikernel/libopencm3/include -Ikernel -Iinclude -Inewlib/include -I.
PREFIX:=$(PWD)/build
LDFLAGS:=-gc-sections -nostartfiles -ggdb -L$(PREFIX)/lib 
CFLAGS+=-mthumb -mlittle-endian -mthumb-interwork
CFLAGS+=-DCORE_M3 -DBOARD_$(BOARD) -D$(ARCH)
CFLAGS+=-DCONFIG_KMEM_SIZE=$(KMEM_SIZE)
CFLAGS+=-DCONFIG_TASK_STACK_SIZE=$(TASK_STACK_SIZE)
CFLAGS-$(KLOG)+=-DCONFIG_KLOG
CFLAGS+=-DCONFIG_KLOG_DEV=\"$(KLOG_DEV)\"
CFLAGS+=$(CFLAGS-y)
#Include paths
CFLAGS+=-Ikernel -Iinclude -I. -Ikernel/libopencm3/include/libopencm3 -Ikernel/libopencm3/include
#Freestanding options
CFLAGS+=-fno-builtin
CFLAGS+=-ffreestanding
CFLAGS+=-nostdlib
#Debugging
CFLAGS+=-ggdb
#CFLAGS+=-Os
ASFLAGS:=-mcpu=cortex-m3 -mthumb -mlittle-endian -mthumb-interwork -ggdb

