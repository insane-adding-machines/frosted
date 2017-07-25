
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
CROSS_COMPILE?=arm-frosted-eabi-
CC:=$(CROSS_COMPILE)gcc
AS:=$(CROSS_COMPILE)as
AR:=$(CROSS_COMPILE)ar
CFLAGS+=-mthumb -mlittle-endian -mthumb-interwork -DCORE_M3 -fno-builtin -ffreestanding -DSYS_CLOCK=$(SYS_CLOCK) -DCORTEX_M3 -DFROSTED
CFLAGS+=-Ikernel/unicore-mx/include -Ikernel -Iinclude -I.
PREFIX:=$(PWD)/build
LDFLAGS:=-gc-sections -nostartfiles -L$(PREFIX)/lib 
CFLAGS+=-mthumb -mlittle-endian -mthumb-interwork
CFLAGS+=-DCORE_M3 -DBOARD_$(BOARD) -D$(ARCH) -mcpu=$(MCPU)
CFLAGS+=-DCONFIG_KMEM_SIZE=$(KMEM_SIZE)
CFLAGS+=-DCONFIG_TASK_STACK_SIZE=$(TASK_STACK_SIZE)

# KERNEL DEBUG
CFLAGS-$(KLOG)+=-DCONFIG_KLOG
CFLAGS+=-DCONFIG_KLOG_SIZE=$(KLOG_SIZE)
CFLAGS-$(HARDFAULT_DBG)+=-DCONFIG_HARDFAULT_DBG
CFLAGS-$(MEMFAULT_DBG)+=-DCONFIG_EXTENDED_MEMFAULT
CFLAGS-$(STRACE)+=-DCONFIG_SYSCALL_TRACE

#PTHREADS
CFLAGS-$(PTHREADS)+=-DCONFIG_PTHREADS

# IPC
CFLAGS-$(PIPE)+=-DCONFIG_PIPE
CFLAGS-$(SIGNALS)+=-DCONFIG_SIGNALS

#MPU
CFLAGS-$(MPU)+=-DCONFIG_MPU

#GCC Optimizations
CFLAGS-$(GDB_CFLAG)+=-ggdb3
CFLAGS-$(OPTIMIZE_NONE)+=-O0
CFLAGS-$(OPTIMIZE_SIZE)+=-Os
CFLAGS-$(OPTIMIZE_PERF)+=-O3

CFLAGS+=$(CFLAGS-y)
#Include paths
CFLAGS+=-Ikernel -Iinclude -I. -Ikernel/unicore-mx/include/unicore-mx -Ikernel/unicore-mx/include
#Freestanding options
CFLAGS+=-fno-builtin
CFLAGS+=-ffreestanding
CFLAGS+=-nostdlib
ASFLAGS+=-mthumb -mlittle-endian -mthumb-interwork -ggdb -mcpu=$(MCPU)

