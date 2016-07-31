#Common code used to read the .config
ifeq ($(ARCH_LPC17XX),y)
	CPU=cortex-m
	BOARD=lpc17xx
	FLASH_ORIGIN=0x00000000
	RAM_BASE=0x10000000
	ARCH=LPC17XX
	CFLAGS+=-DLPC17XX -mcpu=cortex-m3 -DCONFIG_SRAM_EXTRA
	UNICOREMX_TARGET=lpc17xx
endif

ifeq ($(ARCH_LM3S),y)
	CPU=cortex-m
	BOARD=lm3s
	FLASH_ORIGIN=0x00000000
	RAM_BASE=0x20000000
	SYS_CLOCK=50000000
	ARCH=LM3S
	CFLAGS+=-DLM3S -mcpu=cortex-m3
	UNICOREMX_TARGET=lm3s
endif

ifeq ($(ARCH_STM32F4),y)
	CPU=cortex-m
	BOARD=stm32f4
	FLASH_ORIGIN=0x08000000
	RAM_BASE=0x20000000
	CFLAGS+=-DSTM32F4 -mcpu=cortex-m4 -mfloat-abi=soft
	ARCH=STM32F4
	OPENCM3FLAGS=FP_FLAGS="-mfloat-abi=soft"
	UNICOREMX_TARGET=stm32/f4
endif

ifeq ($(ARCH_STM32F7),y)
	CPU=cortex-m
	BOARD=stm32f7
	FLASH_ORIGIN=0x08000000
	RAM_BASE=0x20000000
	CFLAGS=-DSTM32F7 -mcpu=cortex-m4 -mfloat-abi=soft
	ARCH=STM32F7
	OPENCM3FLAGS=FP_FLAGS="-mfloat-abi=soft"
	UNICOREMX_TARGET=stm32/f7
endif

ifeq ($(MACH_STM32F405Pyboard),y)
	CFLAGS+=-DPYBOARD
endif

ifeq ($(MACH_STM32F429Discovery),y)
	CFLAGS+=-DF429DISCO
endif

ifeq ($(FLASH_SIZE_2MB),y)
	FLASH_SIZE=2048
endif
ifeq ($(FLASH_SIZE_1MB),y)
	FLASH_SIZE=1024
endif
ifeq ($(FLASH_SIZE_512KB),y)
	FLASH_SIZE=512
endif
ifeq ($(FLASH_SIZE_384KB),y)
	FLASH_SIZE=384
endif
ifeq ($(FLASH_SIZE_256KB),y)
	FLASH_SIZE=256
endif
ifeq ($(FLASH_SIZE_128KB),y)
	FLASH_SIZE=128
endif

ifeq ($(RAM_SIZE_320KB),y)
	RAM_SIZE=320
endif
ifeq ($(RAM_SIZE_256KB),y)
	RAM_SIZE=256
endif
ifeq ($(RAM_SIZE_192KB),y)
	RAM_SIZE=192
endif
ifeq ($(RAM_SIZE_128KB),y)
	RAM_SIZE=128
endif
ifeq ($(RAM_SIZE_96KB),y)
	RAM_SIZE=96
endif
ifeq ($(RAM_SIZE_64KB),y)
	RAM_SIZE=64
endif
ifeq ($(RAM_SIZE_32KB),y)
	RAM_SIZE=32
endif
ifeq ($(RAM_SIZE_16KB),y)
	RAM_SIZE=16
endif

ifeq ($(CLK_48MHZ),y)
	SYS_CLOCK=48000000
endif
ifeq ($(CLK_84MHZ),y)
	SYS_CLOCK=84000000
endif
ifeq ($(CLK_100MHZ),y)
	SYS_CLOCK=100000000
endif
ifeq ($(CLK_120MHZ),y)
	SYS_CLOCK=120000000
endif
ifeq ($(CLK_168MHZ),y)
	SYS_CLOCK=168000000
endif
ifeq ($(CLK_180MHZ),y)
	SYS_CLOCK=180000000
endif
ifeq ($(CLK_216MHZ),y)
        SYS_CLOCK=216000000
endif
#USARTs
ifeq ($(USART_0),y)
    CFLAGS+=-DCONFIG_USART_0
endif
ifeq ($(USART_1),y)
    CFLAGS+=-DCONFIG_USART_1
endif
ifeq ($(USART_2),y)
    CFLAGS+=-DCONFIG_USART_2
endif
ifeq ($(USART_3),y)
    CFLAGS+=-DCONFIG_USART_3
endif
ifeq ($(USART_6),y)
    CFLAGS+=-DCONFIG_USART_6
endif
#UARTs
ifeq ($(UART_0),y)
    CFLAGS+=-DCONFIG_UART_1
endif
ifeq ($(UART_1),y)
    CFLAGS+=-DCONFIG_UART_1
endif
ifeq ($(UART_2),y)
    CFLAGS+=-DCONFIG_UART_2
endif
ifeq ($(UART_3),y)
    CFLAGS+=-DCONFIG_UART_3
endif
ifeq ($(UART_4),y)
    CFLAGS+=-DCONFIG_UART_4
endif

#SPIs
ifeq ($(SPI_1),y)
    CFLAGS+=-DCONFIG_SPI_1
endif
ifeq ($(SPI_2),y)
    CFLAGS+=-DCONFIG_SPI_2
endif
ifeq ($(SPI_3),y)
    CFLAGS+=-DCONFIG_SPI_3
endif
ifeq ($(SPI_4),y)
    CFLAGS+=-DCONFIG_SPI_4
endif
ifeq ($(SPI_5),y)
    CFLAGS+=-DCONFIG_SPI_5
endif
ifeq ($(SPI_6),y)
    CFLAGS+=-DCONFIG_SPI_6
endif

#RNG
ifeq ($(DEVRNG),y)
    CFLAGS+=-DCONFIG_RNG
endif

#FRAND
ifeq ($(DEVFRAND),y)
    CFLAGS+=-DCONFIG_FRAND
endif


#I2Cs
ifeq ($(I2C1),y)
    CFLAGS+=-DCONFIG_I2C1
endif

#RNG
ifeq ($(DEVTIM),y)
    CFLAGS+=-DCONFIG_TIM
endif

#LOWPOWER
ifeq ($(LOWPOWER),y)
  CFLAGS+=-DCONFIG_LOWPOWER
endif
ifeq ($(TICKLESS),y)
  CFLAGS+=-DCONFIG_TICKLESS
endif

#PICOTCP
ifeq ($(PICOTCP),y)
  CFLAGS+=-DCONFIG_PICOTCP
endif


APPS_ORIGIN=$$(( $(KFLASHMEM_SIZE) * 1024))
CFLAGS+=-DFLASH_ORIGIN=$(FLASH_ORIGIN)
CFLAGS+=-DAPPS_ORIGIN=$(APPS_ORIGIN)
CFLAGS+=-DCONFIG_KRAM_SIZE=$(KRAMMEM_SIZE)
CFLAGS+=-DCONFIG_SYS_CLOCK=$(SYS_CLOCK)

