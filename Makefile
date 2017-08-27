-include kconfig/.config
FROSTED:=$(PWD)
FLASH_ORIGIN?=0x0
CFLAGS+=-DFLASH_ORIGIN=$(FLASH_ORIGIN)

ifneq ($(V),1)
   Q:=@
   #Do not print "Entering directory ...".
   MAKEFLAGS += --no-print-directory
endif



#kernel headers
CFLAGS+=-Ikernel/frosted-headers/include -nostdlib

#drivers headers
CFLAGS+=-Ikernel/drivers

# minimal kernel
OBJS-y+= kernel/frosted.o \
		 kernel/vfs.o \
		 kernel/systick.o \
		 kernel/drivers/device.o \
		 kernel/mpu.o				\
		 kernel/fpb.o				\
		 kernel/string.o			\
		 kernel/sys.o				\
		 kernel/locks.o				\
		 kernel/semaphore.o			\
		 kernel/mutex.o				\
		 kernel/tasklet.o			\
		 kernel/scheduler.o			\
		 kernel/syscall_table.o		\
		 kernel/malloc.o			\
		 kernel/module.o			\
		 kernel/cirbuf.o			\
		 kernel/term.o				\
		 kernel/bflt.o				\
		 kernel/getaddrinfo.o		\
		 kernel/kprintf.o 			\
		 kernel/pipe.o


-include rules/config.mk
include  rules/arch.mk
include  rules/picotcp.mk
include  rules/userspace.mk


# device drivers

OBJS-$(ARCH_STM32F4)+=kernel/drivers/gpio.o \
	kernel/drivers/exti.o

OBJS-$(ARCH_STM32F7)+=kernel/drivers/gpio.o \
	kernel/drivers/exti.o

OBJS-$(ARCH_LPC17XX)+=kernel/drivers/gpio.o

OBJS-$(MEMFS)+= kernel/drivers/memfs.o
OBJS-$(XIPFS)+= kernel/drivers/xipfs.o
CFLAGS-$(MEMFS)+=-DCONFIG_MEMFS

OBJS-$(FATFS)+= kernel/fatfs.o
CFLAGS-$(FATFS)+=-DCONFIG_FATFS
CFLAGS-$(FAT32)+=-DCONFIG_FAT32
CFLAGS-$(FAT16)+=-DCONFIG_FAT16

OBJS-$(SYSFS)+= kernel/drivers/sysfs.o
CFLAGS-$(SYSFS)+=-DCONFIG_SYSFS

OBJS-$(DEVNULL)+= kernel/drivers/null.o
CFLAGS-$(DEVNULL)+=-DCONFIG_DEVNULL

OBJS-$(SOCK_UNIX)+= kernel/drivers/socket_un.o
CFLAGS-$(SOCK_UNIX)+=-DCONFIG_SOCK_UNIX

OBJS-$(PICOTCP)+= kernel/drivers/socket_in.o
CFLAGS-$(PICOTCP)+=-DCONFIG_PICOTCP
CFLAGS-$(CONFIG_PICOTCP_LOOP)+="-DCONFIG_PICOTCP_LOOP"

CFLAGS-$(TCPIP_MEMPOOL_YN)+=-DCONFIG_TCPIP_MEMPOOL=$(TCPIP_MEMPOOL)

OBJS-$(DEVL3GD20)+= kernel/drivers/l3gd20.o
CFLAGS-$(DEVL3GD20)+=-DCONFIG_DEVL3GD20

OBJS-$(DEVLSM303DLHC)+= kernel/drivers/lsm303dlhc.o
CFLAGS-$(DEVLSM303DLHC)+=-DCONFIG_DEVLSM303DLHC

OBJS-$(DEVSTMPE811)+= kernel/drivers/stmpe811.o
CFLAGS-$(DEVSTMPE811)+=-DCONFIG_DEVSTMPE811

OBJS-$(DEVFT5336)+= kernel/drivers/ft5336.o
CFLAGS-$(DEVFT5336)+=-DCONFIG_DEVFT5336

OBJS-$(DEVMCCOG21)+= kernel/drivers/mccog21.o
CFLAGS-$(DEVMCCOG21)+=-DCONFIG_DEVMCCOG21

OBJS-$(DEVSPI)+= kernel/drivers/stm32_spi.o
CFLAGS-$(DEVSPI)+=-DCONFIG_DEVSTM32F4SPI

OBJS-$(DEVLIS3DSH)+= kernel/drivers/lis3dsh.o
CFLAGS-$(DEVLIS3DSH)+=-DCONFIG_DEVLIS3DSH

OBJS-$(DEVSTM32I2C)+= kernel/drivers/stm32_i2c.o
CFLAGS-$(DEVSTM32I2C)+=-DCONFIG_DEVI2C

OBJS-$(DEVF4DSP)+=kernel/drivers/stm32f4_dsp.o
CFLAGS-$(DEVF4DSP)+=-DCONFIG_DSP

OBJS-$(DEVF4ETH)+= kernel/drivers/stm32_eth.o
CFLAGS-$(DEVF4ETH)+=-DCONFIG_DEVETH

OBJS-$(DEVF7ETH)+= kernel/drivers/stm32_eth.o
CFLAGS-$(DEVF7ETH)+=-DCONFIG_DEVETH

OBJS-$(DEVLM3SETH)+= kernel/drivers/lm3s_eth.o
CFLAGS-$(DEVLM3SETH)+=-DCONFIG_DEVETH

OBJS-$(DEVUART)+= kernel/drivers/uart.o
CFLAGS-$(DEVUART)+=-DCONFIG_DEVUART

OBJS-$(DEVILI9341)+= kernel/drivers/ili9341.o
CFLAGS-$(DEVILI9341)+=-DCONFIG_DEVILI9341 -DCONFIG_LTDC

OBJS-$(DEVF7DISCOLTDC) += kernel/drivers/stm32f7_ltdc.o
CFLAGS-$(DEVF7DISCOLTDC)+=-DCONFIG_DEVF7DISCOLTDC -DCONFIG_LTDC

OBJS-$(DEVFRAMEBUFFER)+= kernel/framebuffer.o
CFLAGS-$(DEVFRAMEBUFFER)+=-DCONFIG_DEVFRAMEBUFFER
OBJS-$(DEVFBCON)+= kernel/drivers/fbcon.o kernel/fonts/cga_8x8.o kernel/fonts/palette_256_xterm.o
CFLAGS-$(DEVFBCON)+=-DCONFIG_DEVFBCON

OBJS-$(DEVSTM32DMA)+=kernel/drivers/stm32_dma.o
CFLAGS-$(DEVSTM32DMA)+=-DCONFIG_DMA

CFLAGS-$(DEVSTM32USB)+=-DCONFIG_DEVUSB
OBJS-$(DEVSTM32USB)+=kernel/drivers/stm32_usb.o
CFLAGS-$(DEVSTM32USBFS)+=-DCONFIG_DEVUSBFS
CFLAGS-$(DEVSTM32USBHS)+=-DCONFIG_DEVUSBHS

CFLAGS-$(USBFS_HOST)+=-DCONFIG_USBHOST -DCONFIG_USBFSHOST
CFLAGS-$(USBHS_HOST)+=-DCONFIG_USBHOST -DCONFIG_USBHSHOST

OBJS-$(DEVSTM32SDIO)+=kernel/drivers/stm32_sdio.o
CFLAGS-$(DEVSTM32SDIO)+=-DCONFIG_SDIO

OBJS-$(DEVADC)+=kernel/drivers/stm32f4_adc.o
CFLAGS-$(DEVADC)+=-DCONFIG_DEVSTM32F4ADC

OBJS-$(DEVTIM)+=kernel/drivers/stm32f4_tim.o
CFLAGS-$(DEVTIM)+=-DCONFIG_DEVSTM32F4TIM

OBJS-$(DEVRNG)+=kernel/drivers/stm32_rng.o
CFLAGS-$(DEVRNG)+=-DCONFIG_RNG

OBJS-$(DEVFRAND)+=kernel/drivers/stm32_rng.o		\
		 kernel/crypto/misc.o			\
		 kernel/crypto/sha256.o			\
		 kernel/crypto/aes.o			\
		 kernel/drivers/fortuna.o		\
		 kernel/drivers/frand.o
CFLAGS-$(DEVFRAND)+=-DCONFIG_FRAND -DCONFIG_RNG
OBJS-$(STM32F7_SDRAM)+=kernel/drivers/stm32f7_sdram.o
OBJS-$(STM32F4_SDRAM)+=kernel/drivers/stm32f4_sdram.o
CFLAGS-$(STM32F7_SDRAM)+=-DCONFIG_SDRAM
CFLAGS-$(STM32F4_SDRAM)+=-DCONFIG_SDRAM

OBJS-$(DEV_USB_ETH)+=kernel/drivers/devusb_cdc_ecm.o
CFLAGS-$(DEV_USB_ETH)+=-DCONFIG_DEV_USBETH
CFLAGS-$(DEV_USB_ETH)+=-DCONFIG_USB_DEFAULT_IP=\"$(USB_DEFAULT_IP)\"
CFLAGS-$(DEV_USB_ETH)+=-DCONFIG_USB_DEFAULT_NM=\"$(USB_DEFAULT_NM)\"
CFLAGS-$(DEV_USB_ETH)+=-DCONFIG_USB_DEFAULT_GW=\"$(USB_DEFAULT_GW)\"

CFLAGS-$(DEVF4ETH)+=-DCONFIG_DEV_ETH
CFLAGS-$(DEVF4ETH)+=-DCONFIG_ETH_DEFAULT_IP=\"$(ETH_DEFAULT_IP)\"
CFLAGS-$(DEVF4ETH)+=-DCONFIG_ETH_DEFAULT_NM=\"$(ETH_DEFAULT_NM)\"
CFLAGS-$(DEVF4ETH)+=-DCONFIG_ETH_DEFAULT_GW=\"$(ETH_DEFAULT_GW)\"
CFLAGS-$(DEVF7ETH)+=-DCONFIG_DEV_ETH
CFLAGS-$(DEVF7ETH)+=-DCONFIG_ETH_DEFAULT_IP=\"$(ETH_DEFAULT_IP)\"
CFLAGS-$(DEVF7ETH)+=-DCONFIG_ETH_DEFAULT_NM=\"$(ETH_DEFAULT_NM)\"
CFLAGS-$(DEVF7ETH)+=-DCONFIG_ETH_DEFAULT_GW=\"$(ETH_DEFAULT_GW)\"
CFLAGS-$(DEVLM3SETH)+=-DCONFIG_DEV_ETH
CFLAGS-$(DEVLM3SETH)+=-DCONFIG_ETH_DEFAULT_IP=\"$(ETH_DEFAULT_IP)\"
CFLAGS-$(DEVLM3SETH)+=-DCONFIG_ETH_DEFAULT_NM=\"$(ETH_DEFAULT_NM)\"
CFLAGS-$(DEVLM3SETH)+=-DCONFIG_ETH_DEFAULT_GW=\"$(ETH_DEFAULT_GW)\"

OBJS-$(MACH_STM32F407Discovery)+=kernel/$(BOARD)/stm32f407discovery.o
OBJS-$(MACH_STM32F405Pyboard)+=kernel/$(BOARD)/stm32f405pyboard.o
OBJS-$(MACH_STM32F4x1Discovery)+=kernel/$(BOARD)/stm32f4x1discovery.o
OBJS-$(MACH_STM32F429Discovery)+=kernel/$(BOARD)/stm32f429discovery.o
OBJS-$(MACH_STM32F446Nucleo)+=kernel/$(BOARD)/stm32f446nucleo.o
OBJS-$(MACH_STM32F746Discovery)+=kernel/$(BOARD)/stm32f746discovery.o
OBJS-$(MACH_STM32F769Discovery)+=kernel/$(BOARD)/stm32f769discovery.o
OBJS-$(MACH_STM32F746Nucleo144)+=kernel/$(BOARD)/stm32f746nucleo-144.o
OBJS-$(MACH_LPC1768MBED)+=kernel/$(BOARD)/lpc1768mbed.o
OBJS-$(MACH_SEEEDPRO)+=kernel/$(BOARD)/lpc1768mbed.o
OBJS-$(MACH_LPC1679XPRESSO)+=kernel/$(BOARD)/lpc1769xpresso.o
OBJS-$(MACH_LM3S6965EVB)+=kernel/$(BOARD)/lm3s6965evb.o
OBJS-$(MACH_LM3SVIRT)+=kernel/$(BOARD)/lm3s6965evb.o


LIB-y:=
LIB-$(PICOTCP)+=$(PREFIX)/lib/libpicotcp.a
LIB-y+=kernel/unicore-mx/lib/libucmx_$(BOARD).a
OBJS-$(PICOTCP)+=kernel/net/pico_lock.o

CFLAGS+=$(CFLAGS-y)

SHELL=/bin/bash
all: image.bin

kernel/syscall_table.c: kernel/syscall_table_gen.py
	@python2 $^

$(PREFIX)/lib/libpicotcp.a:
	echo $(BUILD_PICO)
	$(BUILD_PICO)
	@pwd

.PHONY: FORCE st-flash

st-flash: image.bin
	st-flash write image.bin 0x08000000

kernel.img: kernel.elf
	@export PADTO=`python2 -c "print ( $(KFLASHMEM_SIZE) * 1024) + int('$(FLASH_ORIGIN)', 16)"`;	\
	$(CROSS_COMPILE)objcopy -O binary --pad-to=$$PADTO kernel.elf $@

apps.img: $(USERSPACE)
	@make -C $(USERSPACE) FROSTED=$(PWD) FAMILY=$(FAMILY) ARCH=$(ARCH)

image.bin: kernel.img apps.img
	cat kernel.img apps.img > $@

kernel/unicore-mx/lib/libucmx_$(BOARD).a:
	make -C kernel/unicore-mx FP_FLAGS="-mfloat-abi=soft" PREFIX=arm-frosted-eabi TARGETS=$(UNICOREMX_TARGET)

kernel/$(BOARD)/$(BOARD).ld: kernel/$(BOARD)/$(BOARD).ld.in
	export KRAMMEM_SIZE_B=`python2 -c "print '0x%X' % ( $(KRAMMEM_SIZE) * 1024)"`;	\
	export KFLASHMEM_SIZE_B=`python2 -c "print '0x%X' % ( $(KFLASHMEM_SIZE) * 1024)"`;	\
	export RAM1_SIZE_B=`python2 -c "print '0x%X' % ( $(RAM1_SIZE) * 1024)"`;	\
	export RAM2_SIZE_B=`python2 -c "print '0x%X' % ( $(RAM2_SIZE) * 1024)"`;	\
	export RAM3_SIZE_B=`python2 -c "print '0x%X' % ( $(RAM3_SIZE) * 1024)"`;	\
	export SDRAM_SIZE_B=`python2 -c "print '0x%X' % ( $(SDRAM_SIZE))"`;	\
	cat $^ | sed -e "s/__FLASH_ORIGIN/$(FLASH_ORIGIN)/g" | \
			 sed -e "s/__KFLASHMEM_SIZE/$$KFLASHMEM_SIZE_B/g" | \
			 sed -e "s/__KRAMMEM_SIZE/$$KRAMMEM_SIZE_B/g" |\
			 sed -e "s/__RAM1_BASE/$(RAM1_BASE)/g" |\
			 sed -e "s/__RAM2_BASE/$(RAM2_BASE)/g" |\
			 sed -e "s/__RAM3_BASE/$(RAM3_BASE)/g" |\
			 sed -e "s/__SDRAM_BASE/$(SDRAM_BASE)/g" |\
			 sed -e "s/__RAM1_SIZE/$$RAM1_SIZE_B/g" |\
			 sed -e "s/__RAM2_SIZE/$$RAM2_SIZE_B/g" |\
			 sed -e "s/__RAM3_SIZE/$$RAM3_SIZE_B/g" |\
			 sed -e "s/__SDRAM_SIZE/$$SDRAM_SIZE_B/g" \
			 >$@

kernel.elf: $(LIB-y) $(OBJS-y) kernel/$(BOARD)/$(BOARD).ld
	@$(CC) -o $@   -Tkernel/$(BOARD)/$(BOARD).ld -Wl,--start-group $(OBJS-y) $(LIB-y) -Wl,--end-group \
		-Wl,-Map,kernel.map  $(LDFLAGS) $(CFLAGS) $(EXTRA_CFLAGS)



qemudbg: image.bin
	qemu-system-arm -M lm3s6965evb --kernel image.bin -nographic -S -gdb tcp::3333

qemu: image.bin
	qemu-system-arm -M lm3s6965evb --kernel image.bin -nographic

qemu2: qemu

qemunet:
	sudo env "PATH=${PATH}" qemu-system-arm -M lm3s6965evb --kernel image.bin -nographic -net nic,vlan=0 -net tap,vlan=0,ifname=frost0

qemunetdbg:
	sudo qemu-system-arm -M lm3s6965evb --kernel image.bin -nographic -S -gdb tcp::3333 -net nic,vlan=0 -net tap,vlan=0,ifname=frost0

qemubridgedbg:
	qemu-system-arm -M lm3s6965evb --kernel image.bin -nographic -S -gdb tcp::3333 -net nic,vlan=0 -net tap,helper=$(HOME)/frosted-qemu/qemu-bin/libexec/qemu-bridge-helper

qemubridge:
	qemu-system-arm -M lm3s6965evb --kernel image.bin -nographic -net nic,vlan=0 -net tap,helper=$(HOME)/frosted-qemu/qemu-bin/libexec/qemu-bridge-helper

menuconfig:
	@$(MAKE) -C kconfig/ menuconfig -f Makefile.frosted

config:
	@$(MAKE) -C kconfig/ config -f Makefile.frosted

defconfig: FORCE
	@test $(TARGET) || (echo "ERROR: you must define a target" && false)
	@cp -i defconfig/$(TARGET).config kconfig/.config

malloc_test:
	@gcc -o malloc.test kernel/malloc.c -DCONFIG_KRAM_SIZE=4


libclean: clean
	@make -C kernel/unicore-mx clean PREFIX=arm-frosted-eabi

clean:
	@rm -f malloc.test
	@rm -f  kernel/$(BOARD)/$(BOARD).ld
	@make -C $(USERSPACE) clean
	@rm -f $(OBJS-y)
	@rm -f *.map *.bin *.elf *.img
	@rm -f kernel/$(BOARD)/$(BOARD).ld
	@find . |grep "\.o" | xargs -x rm -f
	@rm -rf build
	@rm -f tags
	@rm -f kernel/syscall_table.c
