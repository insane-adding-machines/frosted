/*  
 *      This file is part of frosted.
 *
 *      frosted is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License version 2, as 
 *      published by the Free Software Foundation.
 *      
 *
 *      frosted is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with frosted.  If not, see <http://www.gnu.org/licenses/>.
 *
 *      Authors: Daniele Lacamera, Maxime Vincent
 *
 */  
#include "frosted.h"
#include "libopencm3/cm3/systick.h"
#include <libopencm3/stm32/rcc.h>
#include "libopencm3/cm3/nvic.h"
#include <libopencm3/stm32/dma.h>

#ifdef CONFIG_DEVUART
#include "libopencm3/stm32/usart.h"
#include "uart.h"
#endif

#ifdef CONFIG_DEVGPIO
#include <libopencm3/stm32/gpio.h>
#include "gpio.h"
#endif

#ifdef CONFIG_DEVF4EXTI
#include <libopencm3/stm32/exti.h>
#include "stm32f4_exti.h"
#endif

#ifdef CONFIG_DEVSTM32F4DMA
#include "stm32f4_dma.h"
#endif

#ifdef CONFIG_DEVSPI
#include <libopencm3/stm32/spi.h>
#include "spi.h"
#endif

#ifdef CONFIG_DEVL3GD20
#include "l3gd20.h"
#endif

#ifdef CONFIG_DEVI2C
#include <libopencm3/stm32/i2c.h>
#include "i2c.h"
#endif

#ifdef CONFIG_DEVLSM303DLHC
#include "lsm303dlhc.h"
#endif

#ifdef CONFIG_DEVADC
#include <libopencm3/stm32/adc.h>
#include "adc.h"
#endif

#ifdef CONFIG_DEVGPIO
static const struct gpio_addr gpio_addrs[] = { 
            {.base=GPIOD, .pin=GPIO12,.mode=GPIO_MODE_OUTPUT, .optype=GPIO_OTYPE_PP, .name="gpio_3_12"},
            {.base=GPIOD, .pin=GPIO13,.mode=GPIO_MODE_OUTPUT, .optype=GPIO_OTYPE_PP, .name="gpio_3_13"},
            {.base=GPIOD, .pin=GPIO14,.mode=GPIO_MODE_OUTPUT, .optype=GPIO_OTYPE_PP, .name="gpio_3_14"},
            {.base=GPIOD, .pin=GPIO15,.mode=GPIO_MODE_OUTPUT, .optype=GPIO_OTYPE_PP, .name="gpio_3_15"},
            {.base=GPIOA, .pin=GPIO0,.mode=GPIO_MODE_INPUT, .pullupdown=GPIO_PUPD_NONE, .name=NULL},
#ifdef CONFIG_DEVUART
#ifdef CONFIG_USART_1
            {.base=GPIOA, .pin=GPIO9,.mode=GPIO_MODE_AF,.af=GPIO_AF7, .pullupdown=GPIO_PUPD_NONE, .name=NULL,},
            {.base=GPIOA, .pin=GPIO10,.mode=GPIO_MODE_AF,.af=GPIO_AF7, .speed=GPIO_OSPEED_25MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
#endif
#ifdef CONFIG_USART_2
            {.base=GPIOA, .pin=GPIO2,.mode=GPIO_MODE_AF,.af=GPIO_AF7, .pullupdown=GPIO_PUPD_NONE, .name=NULL,},
            {.base=GPIOA, .pin=GPIO3,.mode=GPIO_MODE_AF,.af=GPIO_AF7, .speed=GPIO_OSPEED_25MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
#endif
#ifdef CONFIG_USART_6
            /* DO NOT use, pins are wired as SPI bus - see schematic */
            {.base=GPIOC, .pin=GPIO6,.mode=GPIO_MODE_AF,.af=GPIO_AF8, .pullupdown=GPIO_PUPD_NONE, .name=NULL,},
            {.base=GPIOC, .pin=GPIO7,.mode=GPIO_MODE_AF,.af=GPIO_AF8, .speed=GPIO_OSPEED_25MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
#endif
#endif

#ifdef CONFIG_DEVI2C
#ifdef CONFIG_I2C_1
            {.base=GPIOB, .pin=GPIO6,.mode=GPIO_MODE_AF,.af=GPIO_AF4, .speed=GPIO_OSPEED_50MHZ, .optype=GPIO_OTYPE_OD, .pullupdown=GPIO_PUPD_PULLUP, .name=NULL,},
            {.base=GPIOB, .pin=GPIO9,.mode=GPIO_MODE_AF,.af=GPIO_AF4, .speed=GPIO_OSPEED_50MHZ, .optype=GPIO_OTYPE_OD, .pullupdown=GPIO_PUPD_PULLUP, .name=NULL,},
#ifdef CONFIG_DEVLSM303DLHC
            /* Depends on I2C_1 : DRDY - PE2 Int1 - E4 Int2 - E5 */
            {.base=GPIOE, .pin=GPIO2,.mode=GPIO_MODE_INPUT, .pullupdown=GPIO_PUPD_NONE, .name=NULL},
            {.base=GPIOE, .pin=GPIO4,.mode=GPIO_MODE_INPUT, .pullupdown=GPIO_PUPD_NONE, .name=NULL},
            {.base=GPIOE, .pin=GPIO5,.mode=GPIO_MODE_INPUT, .pullupdown=GPIO_PUPD_NONE, .name=NULL},
#endif
#endif
#endif

#ifdef CONFIG_DEVSPI
#ifdef CONFIG_SPI_1
            /* SCK - PA5 MISO - PA6 MOSI - */
            {.base=GPIOA, .pin=GPIO5,.mode=GPIO_MODE_AF,.af=GPIO_AF5, .pullupdown=GPIO_PUPD_NONE, .name=NULL,},
            {.base=GPIOA, .pin=GPIO6,.mode=GPIO_MODE_AF,.af=GPIO_AF5, .pullupdown=GPIO_PUPD_NONE, .name=NULL,},
            {.base=GPIOA, .pin=GPIO7,.mode=GPIO_MODE_AF,.af=GPIO_AF5, .pullupdown=GPIO_PUPD_NONE, .name=NULL,},
#ifdef CONFIG_DEVL3GD20
            /* Depends on SPI_1 :  CS - PE3 INT1 - PE0 INT2 - PE1 */
            {.base=GPIOE, .pin=GPIO3,.mode=GPIO_MODE_OUTPUT, .speed=GPIO_OSPEED_25MHZ, .optype=GPIO_OTYPE_PP, .name="l3gd20_cs"},
            {.base=GPIOE, .pin=GPIO0,.mode=GPIO_MODE_INPUT, .pullupdown=GPIO_PUPD_NONE, .name=NULL},
            {.base=GPIOE, .pin=GPIO1,.mode=GPIO_MODE_INPUT, .pullupdown=GPIO_PUPD_NONE, .name=NULL},
#endif
#endif
#endif

#ifdef CONFIG_DEVADC
            {.base=GPIOA, .pin=GPIO1,.mode=GPIO_MODE_ANALOG, .pullupdown=GPIO_PUPD_NONE, .name=NULL},
            {.base=GPIOB, .pin=GPIO0,.mode=GPIO_MODE_ANALOG, .pullupdown=GPIO_PUPD_NONE, .name=NULL},
            {.base=GPIOB, .pin=GPIO1,.mode=GPIO_MODE_ANALOG, .pullupdown=GPIO_PUPD_NONE, .name=NULL},
#endif
};
#define NUM_GPIOS (sizeof(gpio_addrs) / sizeof(struct gpio_addr))
#endif

#ifdef CONFIG_DEVUART
static const struct uart_addr uart_addrs[] = { 
#ifdef CONFIG_USART_1
        {   .devidx = 1,
            .base = USART1, 
            .irq = NVIC_USART1_IRQ, 
            .rcc = RCC_USART1, 
            .baudrate = 115200,
            .stop_bits = USART_STOPBITS_1,
            .data_bits = 8,
            .parity = USART_PARITY_NONE,
            .flow = USART_FLOWCONTROL_NONE,
        },
#endif
#ifdef CONFIG_USART_2
        { 
            .devidx = 2,
            .base = USART2, 
            .irq = NVIC_USART2_IRQ, 
            .rcc = RCC_USART2, 
            .baudrate = 115200,
            .stop_bits = USART_STOPBITS_1,
            .data_bits = 8,
            .parity = USART_PARITY_NONE,
            .flow = USART_FLOWCONTROL_NONE,
        },
#endif
#ifdef CONFIG_USART_6
        { 
            .devidx = 6,
            .base = USART6, 
            .irq = NVIC_USART6_IRQ, 
            .rcc = RCC_USART6, 
            .baudrate = 115200,
            .stop_bits = USART_STOPBITS_1,
            .data_bits = 8,
            .parity = USART_PARITY_NONE,
            .flow = USART_FLOWCONTROL_NONE,
        },
#endif
};
#define NUM_UARTS (sizeof(uart_addrs) / sizeof(struct uart_addr))
#endif

#ifdef CONFIG_DEVF4EXTI
static const struct exti_addr exti_addrs[] = { 
#ifdef CONFIG_DEVL3GD20
            /* INT1 - PE0 INT2 - PE1 */
            {.base=GPIOE, .pin=GPIO0, .trigger=EXTI_TRIGGER_RISING, .name="l3gd20_i1"},
            {.base=GPIOE, .pin=GPIO1, .trigger=EXTI_TRIGGER_RISING, .name="l3gd20_i2"},
#else
            {.base=GPIOA, .pin=GPIO0, .trigger=EXTI_TRIGGER_FALLING, .name="user_pb"},
#endif
#ifdef CONFIG_DEVLSM303DLHC
            {.base=GPIOE, .pin=GPIO2, .trigger=EXTI_TRIGGER_RISING, .name="lsm303_drdy"},
            {.base=GPIOE, .pin=GPIO4, .trigger=EXTI_TRIGGER_RISING, .name="lsm303_i1"},
            {.base=GPIOE, .pin=GPIO5, .trigger=EXTI_TRIGGER_RISING, .name="lsm303_i2"},
#endif
};
#define NUM_EXTIS (sizeof(exti_addrs) / sizeof(struct exti_addr))
#endif

#ifdef CONFIG_DEVSPI
static const struct spi_addr spi_addrs[] = { 
#ifdef CONFIG_SPI_1
        {
            .base = SPI1, 
            .irq = NVIC_SPI1_IRQ, 
            .rcc = RCC_SPI1,

            /* Move this to the upper driver */
            .baudrate_prescaler = SPI_CR1_BR_FPCLK_DIV_256,
            .clock_pol = 1,
            .clock_phase = 1,
            .rx_only = 0,
            .bidir_mode = 0,
            .dff_16 = 0,
            .enable_software_slave_management = 1,
            .send_msb_first = 1,
            /* End move */
            
            .name = "spi1",
            .tx_dma = {
                .base = DMA2,
                .stream = DMA_STREAM3,
                .channel = DMA_SxCR_CHSEL_3,
                .psize =  DMA_SxCR_PSIZE_8BIT,
                .msize = DMA_SxCR_MSIZE_8BIT,
                .dirn = DMA_SxCR_DIR_MEM_TO_PERIPHERAL,
                .prio = DMA_SxCR_PL_MEDIUM,
                .paddr =  (uint32_t) &SPI_DR(SPI1),
                .irq = 0,
            },
            .rx_dma = {
                .base = DMA2,
                .stream = DMA_STREAM2,
                .channel = DMA_SxCR_CHSEL_3,
                .psize =  DMA_SxCR_PSIZE_8BIT,
                .msize = DMA_SxCR_MSIZE_8BIT,
                .dirn = DMA_SxCR_DIR_PERIPHERAL_TO_MEM,
                .prio = DMA_SxCR_PL_VERY_HIGH,
                .paddr =  (uint32_t) &SPI_DR(SPI1),
                .irq = NVIC_DMA2_STREAM2_IRQ,
            },
           
            .dma_rcc = RCC_DMA2,
            
        },
#endif
};
#define NUM_SPIS (sizeof(spi_addrs) / sizeof(struct spi_addr))

#ifdef CONFIG_DEVL3GD20
static const struct l3gd20_addr l3gd20_addrs[] = { 
        {
            .name = "l3gd20",
            .spi_name = "/dev/spi1",
            .spi_cs_name = "/dev/l3gd20_cs",
            .int_1_name = "/dev/l3gd20_i1",
            .int_2_name = "/dev/l3gd20_i2",
        }
};
#define NUM_L3GD20 (sizeof(l3gd20_addrs)/sizeof(struct l3gd20_addr))
#endif
#endif


#ifdef CONFIG_DEVI2C
static const struct i2c_addr i2c_addrs[] = { 
#ifdef CONFIG_I2C_1
        {
            .base = I2C1,
            .ev_irq = NVIC_I2C1_EV_IRQ,
            .er_irq = NVIC_I2C1_ER_IRQ,
            .rcc = RCC_I2C1,
            
            .clock_f = I2C_CR2_FREQ_36MHZ,
            .fast_mode = 1,
            .rise_time = 11,
            .bus_clk_frequency = 10,

            .name = "i2c1",
            .tx_dma = {
                .base = DMA1,
                .stream = DMA_STREAM6,
                .channel = DMA_SxCR_CHSEL_1,
                .psize =  DMA_SxCR_PSIZE_8BIT,
                .msize = DMA_SxCR_MSIZE_8BIT,
                .dirn = DMA_SxCR_DIR_MEM_TO_PERIPHERAL,
                .prio = DMA_SxCR_PL_MEDIUM,
                .paddr =  (uint32_t) &I2C_DR(I2C1),
                .irq = NVIC_DMA1_STREAM6_IRQ,
            },
            .rx_dma = {
                .base = DMA1,
                .stream = DMA_STREAM0,
                .channel = DMA_SxCR_CHSEL_1,
                .psize =  DMA_SxCR_PSIZE_8BIT,
                .msize = DMA_SxCR_MSIZE_8BIT,
                .dirn = DMA_SxCR_DIR_PERIPHERAL_TO_MEM,
                .prio = DMA_SxCR_PL_VERY_HIGH,
                .paddr =  (uint32_t) &I2C_DR(I2C1),
                .irq = NVIC_DMA1_STREAM0_IRQ,
            },
                    
            .dma_rcc = RCC_DMA1,
        },
#endif
};
#define NUM_I2CS (sizeof(i2c_addrs) / sizeof(struct i2c_addr))

#ifdef CONFIG_DEVLSM303DLHC
static const struct lsm303dlhc_addr lsm303dlhc_addrs[] = {
        {
            .name = "lsm303acc",
            .i2c_name = "/dev/i2c1",
            .int_1_name = "/dev/lsm303_i1",
            .int_2_name = "/dev/lsm303_i2",
            .drdy_name = NULL,
            .address = 0x32,
        },
        {
            .name = "lsm303mag",
            .i2c_name = "/dev/i2c1",
            .int_1_name = NULL,
            .int_2_name = NULL,
            .drdy_name = "/dev/lsm303_drdy",
            .address = 0x3C,
        },
};
#define NUM_LSM303DLHC (sizeof(lsm303dlhc_addrs)/sizeof(struct lsm303dlhc_addr))
#endif
#endif


#ifdef CONFIG_DEVADC
static const struct adc_addr adc_addrs[] = { 
        {
            .base = ADC1, 
            .irq = NVIC_ADC_IRQ, 
            .rcc = RCC_ADC1,
            .name = "adc",
            .channel_array = {1, 8, 9},     /*ADC_IN1,8,9 on PA1, PB0, PB1 */
            .num_channels = 3,
            /* Use DMA to fetch sample data */
            .dma = {
                .base = DMA2,
                .stream = DMA_STREAM0,
                .channel = DMA_SxCR_CHSEL_1,
                .psize =  DMA_SxCR_PSIZE_16BIT,
                .msize = DMA_SxCR_MSIZE_16BIT,
                .dirn = DMA_SxCR_DIR_PERIPHERAL_TO_MEM,
                .prio = DMA_SxCR_PL_HIGH,
                .paddr =  (uint32_t) &ADC_DR(ADC1),
                .irq = NVIC_DMA2_STREAM0_IRQ,
            },
            .dma_rcc = RCC_DMA2,
        }
};
#define NUM_ADC (sizeof(adc_addrs)/sizeof(struct adc_addr))
#endif


void machine_init(struct fnode * dev)
{
        /* 401 & 411 run 84 and 100 MHz max respectively */
#       if CONFIG_SYS_CLOCK == 48000000
        rcc_clock_setup_hse_3v3(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_48MHZ]);
#       elif CONFIG_SYS_CLOCK == 84000000
        rcc_clock_setup_hse_3v3(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_84MHZ]);
#       else
#error No valid clock speed selected for STM32F4x1 Discovery
#endif

#ifdef CONFIG_DEVGPIO
    gpio_init(dev, gpio_addrs, NUM_GPIOS);
#endif
#ifdef CONFIG_DEVF4EXTI
    exti_init(dev, exti_addrs, NUM_EXTIS);
#endif
#ifdef CONFIG_DEVUART
    uart_init(dev, uart_addrs, NUM_UARTS);
#endif
#ifdef CONFIG_DEVSPI
    spi_init(dev, spi_addrs, NUM_SPIS);
#ifdef CONFIG_DEVL3GD20
    l3gd20_init(dev, l3gd20_addrs, NUM_L3GD20);
#endif
#endif
#ifdef CONFIG_DEVI2C
    i2c_init(dev, i2c_addrs, NUM_I2CS);
#ifdef CONFIG_DEVLSM303DLHC
    lsm303dlhc_init(dev, lsm303dlhc_addrs, NUM_LSM303DLHC);
#endif
#endif
#ifdef CONFIG_DEVADC
    adc_init(dev, adc_addrs, NUM_ADC);
#endif
}

