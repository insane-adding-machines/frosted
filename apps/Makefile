# this makefile is to compile apps as a separate bFLT executable
#
#

# PATH CONFIG
FROSTED_PATH=/home/vinz/frosted
PREFIX:=$(PWD)/build

# APPLICATION CONFIG
APPS-y := init_bflt.o
APPS-y += fresh.o

# TOOLCHAIN CONFIG
CROSS_COMPILE?=arm-none-eabi-
CC:=$(CROSS_COMPILE)gcc
AS:=$(CROSS_COMPILE)as
AR:=$(CROSS_COMPILE)ar

# COMPILER FLAGS
CFLAGS+=-I$(FROSTED_PATH)/kernel/libopencm3/include -I$(FROSTED_PATH)/kernel -I$(FROSTED_PATH)/include -I$(FROSTED_PATH)/include/libc 
CFLAGS+=-mthumb -mlittle-endian -mthumb-interwork -fno-builtin -ffunction-sections
CFLAGS+=-fPIC -mword-relocations -mlong-calls -fno-common
CFLAGS+=-DCORE_M3 -DKLOG_LEVEL=6 
#-DSYS_CLOCK=$(SYS_CLOCK)
#debugging
CFLAGS+=-ggdb

# LINKER FLAGS
LDFLAGS:=-L$(FROSTED_PATH)/build/lib -gc-sections --specs=nano.specs 
LDFLAGS+=-fPIC -mword-relocations -mlong-calls -fno-common -Wl,-elf2flt

all: apps.bflt

apps.bflt: $(FROSTED_PATH)/build/lib/libfrosted.a $(APPS-y)
	$(CC) -o $@  $(APPS-y) -Telf2flt.ld -lfrosted -lc -lfrosted -Wl,-Map,apps.map  $(LDFLAGS) $(CFLAGS) $(EXTRA_CFLAGS)

clean:
	@rm -f *.o
	@rm -f apps.bflt
	@echo Clean!
