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
#include <unicore-mx/cm3/systick.h>
#include <unicore-mx/cm3/nvic.h>

#include "uart.h"
#include "gpio.h"
#include "sdio.h"
#include "dsp.h"
#include "usb.h"
#include "i2c.h"
#include "spi.h"


static const struct i2c_config i2c_configs[] = {
#ifdef CONFIG_I2C1
    {
        .idx  = 1,
        .base = I2C1,
        .ev_irq = NVIC_I2C1_EV_IRQ,
        .er_irq = NVIC_I2C1_ER_IRQ,
        .rcc = RCC_I2C1,
        .dma_rcc = RCC_DMA2,

        .clock_f = I2C_CR2_FREQ_36MHZ,
        .fast_mode = 1,
        .rise_time = 11,
        .bus_clk_frequency = 10,

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
        .pio_scl = {
            .base=GPIOB,
            .pin=GPIO8,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF4,
            .speed=GPIO_OSPEED_50MHZ,
            .optype=GPIO_OTYPE_OD,
            .pullupdown = GPIO_PUPD_NONE
        },
        .pio_sda = {
            .base=GPIOB,
            .pin=GPIO9,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF4,
            .speed=GPIO_OSPEED_50MHZ,
            .optype=GPIO_OTYPE_OD,
            .pullupdown = GPIO_PUPD_NONE
        },
    },
#endif
};
#define NUM_I2CS (sizeof(i2c_configs) / sizeof(struct i2c_config))

static const struct spi_config spi_configs[] = {
#ifdef CONFIG_SPI_1
    {
        .idx  = 1,
        .base = SPI1,
        .irq = NVIC_SPI1_IRQ,
        .rcc = RCC_SPI1,
        .baudrate = 0, /* TODO: SPI baudrate */
        .polarity = 1,
        .phase = 1,
        .rx_only = 0,
        .bidir_mode = 0,
        .dff_16 = 0,
        .enable_software_slave_management = 1,
        .send_msb_first = 1,

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
        .dma_rcc = RCC_DMA1,
        .pio_sck = {
            .base=GPIOA,
            .pin=GPIO5,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF5,
            .optype=GPIO_OTYPE_PP,
            .speed=GPIO_OSPEED_100MHZ,
        },
        .pio_mosi = {
            .base=GPIOA,
            .pin=GPIO6,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF5,
            .optype=GPIO_OTYPE_PP,
            .speed=GPIO_OSPEED_100MHZ,
        },
        .pio_miso = {
            .base=GPIOA,
            .pin=GPIO7,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF5,
            .optype=GPIO_OTYPE_PP,
            .speed=GPIO_OSPEED_100MHZ,
        },
    },
#endif
};
#define NUM_SPIS (sizeof(spi_configs) / sizeof(struct spi_config))

static const struct uart_config uart_configs[] = {
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

        .pio_rx = {
            .base=GPIOA,
            .pin=GPIO9,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF7,
            .pullupdown=GPIO_PUPD_NONE,
        },

        .pio_tx = {
            .base=GPIOA,
            .pin=GPIO10,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF7,
            .speed=GPIO_OSPEED_25MHZ,
            .optype=GPIO_OTYPE_PP,
        },
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

        .pio_rx = {
            .base=GPIOA,
            .pin=GPIO2,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF7,
            .pullupdown=GPIO_PUPD_NONE,
        },

        .pio_tx = {
            .base=GPIOA,
            .pin=GPIO3,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF7,
            .speed=GPIO_OSPEED_25MHZ,
            .optype=GPIO_OTYPE_PP,
        },
    },
#endif
#ifdef CONFIG_USART_3
    {
        .devidx = 3,
        .base = USART3,
        .irq = NVIC_USART3_IRQ,
        .rcc = RCC_USART3,
        .baudrate = 115200,
        .stop_bits = USART_STOPBITS_1,
        .data_bits = 8,
        .parity = USART_PARITY_NONE,
        .flow = USART_FLOWCONTROL_NONE,
        .pio_rx = {
            .base=GPIOD,
            .pin=GPIO8,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF7,
            .pullupdown=GPIO_PUPD_NONE,
        },
        .pio_tx = {
            .base=GPIOD,
            .pin=GPIO9,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF7,
            .speed=GPIO_OSPEED_25MHZ,
            .optype=GPIO_OTYPE_PP,
        },
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

        .pio_rx = {
            .base=GPIOC,
            .pin=GPIO6,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF8,
            .pullupdown=GPIO_PUPD_NONE,
        },
        .pio_tx = {
            .base=GPIOC,
            .pin=GPIO7,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF8,
            .speed=GPIO_OSPEED_25MHZ,
            .optype=GPIO_OTYPE_PP,
        },
    },
#endif
};
#define NUM_UARTS (sizeof(uart_configs) / sizeof(struct uart_config))

int machine_init(void)
{
    int i;
    /* Clock */
    //rcc_clock_setup_hse_3v3(&rcc_hse_8mhz_3v3[STM32_CLOCK]);

    /* Uarts */
    for (i = 0; i < NUM_UARTS; i++) {
        uart_create(&uart_configs[i]);
    }

    /* I2Cs */
    for (i = 0; i < NUM_I2CS; i++) {
        i2c_create(&i2c_configs[i]);
    }

    /* SPIs */
    for (i = 0; i < NUM_SPIS; i++) {
        devspi_create(&spi_configs[i]);
    }

    return 0;
}
