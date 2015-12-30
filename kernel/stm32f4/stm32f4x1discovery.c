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
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/dma.h>

#ifdef CONFIG_DEVUART
#include "libopencm3/stm32/usart.h"
#include "uart.h"
#endif

#ifdef CONFIG_DEVGPIO
#include <libopencm3/stm32/gpio.h>
#include "gpio.h"
#endif

#ifdef CONFIG_DEVSPI
#include <libopencm3/stm32/spi.h>
#include "spi.h"
#endif

#ifdef CONFIG_DEVL3GD20
#include "l3gd20.h"
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
#ifdef CONFIG_DEVL3GD20
            /* Grrr - for some reason this PB is on PA0 and one of the gyro INT lines is on PE0
                We dont support >1 EXTI on the same pin (even if they are in diff banks) */
            {.base=GPIOA, .pin=GPIO0,.mode=GPIO_MODE_INPUT, .pullupdown=GPIO_PUPD_NONE, .name="gpio_0_0"},
#else
            {.base=GPIOA, .pin=GPIO0,.mode=GPIO_MODE_INPUT, .pullupdown=GPIO_PUPD_NONE, .exti=1, .trigger=EXTI_TRIGGER_FALLING, .name="gpio_0_0"},
#endif
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
#ifdef CONFIG_DEVSPI
#ifdef CONFIG_SPI_1
            /* SCK - PA5 MISO - PA6 MOSI - */
            {.base=GPIOA, .pin=GPIO5,.mode=GPIO_MODE_AF,.af=GPIO_AF5, .pullupdown=GPIO_PUPD_NONE, .name=NULL,},
            {.base=GPIOA, .pin=GPIO6,.mode=GPIO_MODE_AF,.af=GPIO_AF5, .pullupdown=GPIO_PUPD_NONE, .name=NULL,},
            {.base=GPIOA, .pin=GPIO7,.mode=GPIO_MODE_AF,.af=GPIO_AF5, .pullupdown=GPIO_PUPD_NONE, .name=NULL,},

#endif
#ifdef CONFIG_DEVL3GD20
            /* CS - PE3 INT1 - PE0 INT2 - PE1 */
            {.base=GPIOE, .pin=GPIO3,.mode=GPIO_MODE_OUTPUT, .speed=GPIO_OSPEED_25MHZ, .optype=GPIO_OTYPE_PP, .name="l3gd20_cs"},
            {.base=GPIOE, .pin=GPIO0,.mode=GPIO_MODE_INPUT, .exti=1, .trigger=EXTI_TRIGGER_FALLING, .name="l3gd20_i1"},
            {.base=GPIOE, .pin=GPIO1,.mode=GPIO_MODE_INPUT, .exti=1, .trigger=EXTI_TRIGGER_FALLING, .name="l3gd20_i2"},
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
            .dma_base = DMA2,
            .dma_rcc = RCC_DMA2,

            .tx_dma_stream = DMA_STREAM3,
            .tx_dma_irq = NVIC_DMA2_STREAM3_IRQ,
            /* Avoid ADC DMA */
            .rx_dma_stream = DMA_STREAM2,
            .rx_dma_irq = NVIC_DMA2_STREAM2_IRQ,
            
        },
#endif
};
#define NUM_SPIS (sizeof(spi_addrs) / sizeof(struct spi_addr))

#ifdef CONFIG_DEVL3GD20
static const struct l3gd20_addr l3gd20_addrs[] = { 
        {
            .spi_name = "spi1",
            .spi_cs_name = "l3gd20_cs",
            .int_1_name = "l3gd20_i1",
            .int_2_name = "l3gd20_i2",
            .name = "l3gd20",
        }
};
#define NUM_L3GD20 (sizeof(l3gd20_addrs)/sizeof(struct l3gd20_addr))
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
            .dma_base = DMA2,
            .dma_rcc = RCC_DMA2,
            .dma_stream = DMA_STREAM0,
            .dma_irq = NVIC_DMA2_STREAM0_IRQ,
        }
};
#define NUM_ADC (sizeof(adc_addrs)/sizeof(struct adc_addr))
#endif


void machine_init(struct fnode * dev)
{
        /* 401 & 411 run 84 and 100 MHz max respectively */
#       if CONFIG_SYS_CLOCK == 48000000
        rcc_clock_setup_hse_3v3(&hse_8mhz_3v3[CLOCK_3V3_48MHZ]);
#       elif CONFIG_SYS_CLOCK == 84000000
        rcc_clock_setup_hse_3v3(&hse_8mhz_3v3[CLOCK_3V3_84MHZ]);
#       else
#error No valid clock speed selected for STM32F4x1 Discovery
#endif

#ifdef CONFIG_DEVGPIO
    gpio_init(dev, gpio_addrs, NUM_GPIOS);
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
#ifdef CONFIG_DEVADC
    adc_init(dev, adc_addrs, NUM_ADC);
#endif
}

