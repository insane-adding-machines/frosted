FAMILY?=lpc17xx
ARCH?=seedpro
CROSS_COMPILE?=arm-none-eabi-
CC:=$(CROSS_COMPILE)gcc
AS:=$(CROSS_COMPILE)as
CFLAGS:=-mcpu=cortex-m3 -mthumb -mlittle-endian -mthumb-interwork -DARCH_$(ARCH) -Iarch/$(ARCH)/inc -Ilib/$(FAMILY)/inc -DCORE_M3
LDFLAGS:=-nostartfiles -lc -lm -lrdimon -ggdb

#debugging
CFLAGS+=-ggdb

ASFLAGS:=-mcpu=cortex-m3 -mthumb -mlittle-endian -mthumb-interwork -ggdb
OBJS:=sched.o frosted.o lib/$(FAMILY)/$(FAMILY).o systick.o syscall.o timer.o scheduler.o

include lib/$(FAMILY)/$(FAMILY).mk

all: image.elf

image.elf: $(OBJS) $(LIBS)
	$(CC) -o $@   -Wl,--start-group  $(OBJS) $(LIBS) -Wl,--end-group  -Tlib/$(FAMILY)/$(FAMILY).ld  -Wl,-Map,image.map  $(LDFLAGS) $(CFLAGS) $(EXTRA_CFLAGS)

qemu: image.elf
	sudo qemu-system-arm  -rtc base=localtime -d int,ioport,guest_errors,unimp -M lm3s6965evb -cpu cortex-m3 -m 1 -serial stdio -net nic,model=stellaris -net tap -kernel $< -name stellaris-qemu -s


clean:
	@rm -f $(OBJS)
	@rm -f image.elf image.map

