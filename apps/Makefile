# this makefile is to compile apps as a separate bFLT executable
#
#

# PATH CONFIG
FROSTED_PATH=..
PREFIX:=$(PWD)/build

# APPLICATION CONFIG
APPS-y += init_bflt.o fresh.o
#APPS-y += tsh.o stubs.o
#APPS-y += simple-c-shell.o stubs.o

# TOOLCHAIN CONFIG
CROSS_COMPILE?=arm-none-eabi-
CC:=$(CROSS_COMPILE)gcc
AS:=$(CROSS_COMPILE)as
AR:=$(CROSS_COMPILE)ar

# COMPILER FLAGS
CFLAGS+=-I$(FROSTED_PATH)/kernel/libopencm3/include -I$(FROSTED_PATH)/kernel -I$(FROSTED_PATH)/include -I$(FROSTED_PATH)/include/libc -I$(FROSTED_PATH)/newlib/include
CFLAGS+=-mthumb -mlittle-endian -mthumb-interwork -fno-builtin -ffunction-sections -mcpu=cortex-m3 -nostartfiles
CFLAGS+=-DCORE_M3 -DKLOG_LEVEL=6 -D__frosted__
# GOTPIC CFLAGS
CFLAGS+=-fPIC -mword-relocations -mlong-calls -fno-common -msingle-pic-base -mpic-register=r9 -mno-pic-data-is-text-relative
#debugging
CFLAGS+=-ggdb

# LINKER FLAGS
LDFLAGS:=-L$(FROSTED_PATH)/build/lib -L$(FROSTED_PATH)/newlib/lib -gc-sections --specs=nano.specs
LDFLAGS+=-fPIC -mword-relocations -mlong-calls -fno-common -Wl,-elf2flt

all: apps.bflt

apps.bflt: $(FROSTED_PATH)/build/lib/libfrosted.a $(APPS-y)
	$(CC) -o $@  $(APPS-y) -Telf2flt.ld -lfrosted $(FROSTED_PATH)/newlib/lib/libc.a $(FROSTED_PATH)/newlib/lib/libg.a -lfrosted -Wl,-Map,apps.map  $(LDFLAGS) $(CFLAGS) $(EXTRA_CFLAGS)

clean:
	@rm -f *.o
	@rm -f apps.bflt
	@echo Clean!
