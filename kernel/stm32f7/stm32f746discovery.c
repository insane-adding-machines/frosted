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
#include "unicore-mx/cm3/systick.h"
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/usart.h>
#include <unicore-mx/cm3/nvic.h>
#include <unicore-mx/stm32/f7/usart.h>
#include <unicore-mx/stm32/i2c.h>
#include <unicore-mx/stm32/dma.h>

#if CONFIG_SYS_CLOCK == 216000000
#else
#error No valid clock speed selected for STM32F729 Discovery
#endif

#include "gpio.h"
#include "uart.h"
#include "rng.h"
#include "sdram.h"
#include "sdio.h"
#include "framebuffer.h"
#include "ltdc.h"
#include "usb.h"
#include "eth.h"
#include "i2c.h"

static const struct gpio_config gpio_led0 = {
    .base=GPIOI,
    .pin=GPIO1,
    .mode=GPIO_MODE_OUTPUT,
    .optype=GPIO_OTYPE_PP,
    .name="led0"
};

static const struct gpio_config gpio_button = {
    .base=GPIOI,
    .pin=GPIO11,
    .mode=GPIO_MODE_INPUT,
    .optype=GPIO_OTYPE_PP,
    .pullupdown=GPIO_PUPD_PULLUP,
    .name="button"
};

static const struct uart_config uart_configs[] = {
#ifdef CONFIG_DEVUART
#ifdef CONFIG_USART_1
    {
        .devidx = 1,
        .base = USART1,
        .irq = NVIC_USART1_IRQ,
        .rcc = RCC_USART1,
        .baudrate = 115200,
        .stop_bits = USART_STOPBITS_1,
        .data_bits = 8,
        .parity = USART_PARITY_NONE,
        .flow = USART_FLOWCONTROL_NONE,
        .pio_tx = {
            .base=GPIOB,
            .pin=GPIO7,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF7,
            .speed=GPIO_OSPEED_25MHZ,
            .optype=GPIO_OTYPE_PP,
        },
        .pio_rx = {
            .base=GPIOA,
            .pin=GPIO9,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF7,
            .pullupdown=GPIO_PUPD_NONE
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
        .pio_tx = {
            .base=GPIOC,
            .pin=GPIO6,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF8,
            .speed=GPIO_OSPEED_25MHZ,
            .optype=GPIO_OTYPE_PP,
        },
        .pio_rx = {
            .base=GPIOC,
            .pin=GPIO7,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF8,
            .pullupdown=GPIO_PUPD_NONE
        },
    },
#endif
#endif
};
#define NUM_UARTS (sizeof(uart_configs) / sizeof(struct uart_config))


/* Setup GPIO Pins for SDIO:
   PC8 - PC11 - DAT0 thru DAT3
   PC12 - CLK
   PD2 - CMD
*/
struct sdio_config sdio_conf = {
    .pio_dat0 = {
        .base=GPIOC,
        .pin=GPIO8,
        .mode=GPIO_MODE_AF,
        .speed=GPIO_OSPEED_100MHZ,
        .af = GPIO_AF12,
        .optype=GPIO_OTYPE_PP,
        .pullupdown=GPIO_PUPD_PULLUP

    },
    .pio_dat1 = {
        .base=GPIOC,
        .pin=GPIO9,
        .mode=GPIO_MODE_AF,
        .af = GPIO_AF12,
        .speed=GPIO_OSPEED_100MHZ,
        .optype=GPIO_OTYPE_PP,
        .pullupdown=GPIO_PUPD_PULLUP
    },
    .pio_dat2 = {
        .base=GPIOC,
        .pin=GPIO10,
        .af = GPIO_AF12,
        .mode=GPIO_MODE_AF,
        .speed=GPIO_OSPEED_100MHZ,
        .optype=GPIO_OTYPE_PP,
        .pullupdown=GPIO_PUPD_PULLUP
    },
    .pio_dat3 = {
        .base=GPIOC,
        .pin=GPIO11,
        .af = GPIO_AF12,
        .mode=GPIO_MODE_AF,
        .speed=GPIO_OSPEED_100MHZ,
        .optype=GPIO_OTYPE_PP,
        .pullupdown=GPIO_PUPD_PULLUP
    },
    .pio_clk = {
        .base=GPIOC,
        .pin=GPIO12,
        .mode=GPIO_MODE_AF,
        .af = GPIO_AF12,
        .speed=GPIO_OSPEED_100MHZ,
        .optype=GPIO_OTYPE_PP,
        .pullupdown=GPIO_PUPD_PULLUP
    },
    .pio_cmd = {
        .base=GPIOD,
        .pin=GPIO2,
        .mode=GPIO_MODE_AF,
        .af = GPIO_AF12,
        .speed=GPIO_OSPEED_100MHZ,
        .optype=GPIO_OTYPE_PP,
        .pullupdown=GPIO_PUPD_PULLUP
    },
    .card_detect_supported = 1,
    /* STM37 has an additional card-detect pin on PC13 */
    .pio_cd = {
        .base=GPIOC,
        .pin=GPIO13,
        .mode=GPIO_MODE_INPUT,
        .pullupdown=GPIO_PUPD_PULLUP
    }
};

struct gpio_config stm32eth_mii_pins[] = {
    {.base=GPIOA, .pin=GPIO1, .mode=GPIO_MODE_AF, .af=GPIO_AF11},                        // RMII REF CLK
    {.base=GPIOA, .pin=GPIO2, .mode=GPIO_MODE_AF, .optype=GPIO_OTYPE_PP, .af=GPIO_AF11}, // MDIO
    {.base=GPIOA, .pin=GPIO7, .mode=GPIO_MODE_AF, .af=GPIO_AF11},                        // RMII CRS DV
    {.base=GPIOG, .pin=GPIO2, .mode=GPIO_MODE_AF, .af=GPIO_AF11},                        // RMII RXER
    {.base=GPIOG, .pin=GPIO11,.mode=GPIO_MODE_AF, .optype=GPIO_OTYPE_PP, .af=GPIO_AF11}, // RMII TXEN
    {.base=GPIOG, .pin=GPIO13,.mode=GPIO_MODE_AF, .optype=GPIO_OTYPE_PP, .af=GPIO_AF11}, // RMII TXD0
    {.base=GPIOG, .pin=GPIO14,.mode=GPIO_MODE_AF, .optype=GPIO_OTYPE_PP, .af=GPIO_AF11}, // RMII TXD1
    {.base=GPIOC, .pin=GPIO1, .mode=GPIO_MODE_AF, .optype=GPIO_OTYPE_PP, .af=GPIO_AF11}, // MDC
    {.base=GPIOC, .pin=GPIO4, .mode=GPIO_MODE_AF, .af=GPIO_AF11},                        // RMII RXD0
    {.base=GPIOC, .pin=GPIO5, .mode=GPIO_MODE_AF, .af=GPIO_AF11},                        // RMII RXD1
};

static struct eth_config eth_config = {
    .pio_mii = stm32eth_mii_pins,
    .n_pio_mii = sizeof(stm32eth_mii_pins)/sizeof(struct gpio_config),
    .pio_phy_reset = {},
    .has_phy_reset = 0
};

static struct usb_pio_config_fs pio_usbfs = {
    .pio_vbus = {
        .base=GPIOA,
        .pin=GPIO8,
        .mode=GPIO_MODE_AF,
        .af=GPIO_AF10,
        .pullupdown=GPIO_PUPD_NONE,
    },
    .pio_dm =   {
        .base=GPIOA,
        .pin=GPIO11,
        .mode=GPIO_MODE_AF,
        .af=GPIO_AF10,
        .pullupdown=GPIO_PUPD_NONE,
    },
    .pio_dp =   {
        .base=GPIOA,
        .pin=GPIO12,
        .mode=GPIO_MODE_AF,
        .af=GPIO_AF10,
        .pullupdown=GPIO_PUPD_NONE,
    }
};

static struct usb_pio_config_hs pio_usbhs = {
    .ulpi_data = {
        { .base=GPIOA, .pin=GPIO3, .mode=GPIO_MODE_AF, .af=GPIO_AF10, .pullupdown=GPIO_PUPD_NONE,  .optype=GPIO_OTYPE_PP, .speed=GPIO_OSPEED_100MHZ},
        { .base=GPIOB, .pin=GPIO0, .mode=GPIO_MODE_AF, .af=GPIO_AF10, .pullupdown=GPIO_PUPD_NONE , .optype=GPIO_OTYPE_PP, .speed=GPIO_OSPEED_100MHZ},
        { .base=GPIOB, .pin=GPIO1, .mode=GPIO_MODE_AF, .af=GPIO_AF10, .pullupdown=GPIO_PUPD_NONE , .optype=GPIO_OTYPE_PP, .speed=GPIO_OSPEED_100MHZ},
        { .base=GPIOB, .pin=GPIO10, .mode=GPIO_MODE_AF, .af=GPIO_AF10, .pullupdown=GPIO_PUPD_NONE, .optype=GPIO_OTYPE_PP, .speed=GPIO_OSPEED_100MHZ},
        { .base=GPIOB, .pin=GPIO11, .mode=GPIO_MODE_AF, .af=GPIO_AF10, .pullupdown=GPIO_PUPD_NONE, .optype=GPIO_OTYPE_PP, .speed=GPIO_OSPEED_100MHZ},
        { .base=GPIOB, .pin=GPIO12, .mode=GPIO_MODE_AF, .af=GPIO_AF10, .pullupdown=GPIO_PUPD_NONE, .optype=GPIO_OTYPE_PP, .speed=GPIO_OSPEED_100MHZ},
        { .base=GPIOB, .pin=GPIO13, .mode=GPIO_MODE_AF, .af=GPIO_AF10, .pullupdown=GPIO_PUPD_NONE, .optype=GPIO_OTYPE_PP, .speed=GPIO_OSPEED_100MHZ},
        { .base=GPIOB, .pin=GPIO5, .mode=GPIO_MODE_AF, .af=GPIO_AF10, .pullupdown=GPIO_PUPD_NONE , .optype=GPIO_OTYPE_PP, .speed=GPIO_OSPEED_100MHZ},
    },
    .ulpi_clk  = { .base=GPIOA, .pin=GPIO5, .mode=GPIO_MODE_AF, .af=GPIO_AF10, .optype=GPIO_OTYPE_PP, .pullupdown=GPIO_PUPD_NONE, .speed=GPIO_OSPEED_100MHZ},
    .ulpi_dir  = { .base=GPIOC, .pin=GPIO2, .mode=GPIO_MODE_AF, .af=GPIO_AF10, .optype=GPIO_OTYPE_PP, .pullupdown=GPIO_PUPD_NONE, .speed=GPIO_OSPEED_100MHZ},
    .ulpi_next = { .base=GPIOH, .pin=GPIO4, .mode=GPIO_MODE_AF, .af=GPIO_AF10, .optype=GPIO_OTYPE_PP, .pullupdown=GPIO_PUPD_NONE, .speed=GPIO_OSPEED_100MHZ},
    .ulpi_step = { .base=GPIOC, .pin=GPIO0, .mode=GPIO_MODE_AF, .af=GPIO_AF10, .optype=GPIO_OTYPE_PP, .pullupdown=GPIO_PUPD_NONE, .speed=GPIO_OSPEED_100MHZ}
};

static struct usb_config usb_fs_guest = {
    .dev_type = USB_DEV_FS,
    .otg_mode = USB_MODE_GUEST,
    .pio.fs = &pio_usbfs
};

static struct usb_config usb_hs_guest = {
    .dev_type = USB_DEV_HS,
    .otg_mode = USB_MODE_GUEST,
    .pio.hs = &pio_usbhs
};

static struct usb_config usb_hs_host = {
    .dev_type = USB_DEV_HS,
    .otg_mode = USB_MODE_HOST,
    .pio.hs = &pio_usbhs
};

/** I2C: 
 *    Touchscreen:
 *    INT = PI13
 *    CLK = PI14
 *    R0  = PI15
 *    SCL = PH7
 *    SDA = PH8
 *    VSYNC = PI10
 *    HSYNC = PI11
 *    Address = 01110000
 */
static const struct i2c_config i2c_configs[] = {
#ifdef CONFIG_I2C1
    {
        .idx  = 1,
        .base = I2C1,
        .ev_irq = NVIC_I2C1_EV_IRQ,
        .er_irq = NVIC_I2C1_ER_IRQ,
        .rcc = RCC_I2C1,

        .clock_f = I2C_CR2_FREQ_42MHZ,
        .fast_mode = 1,
        .rise_time = 11,
        .bus_clk_frequency = 10,

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
        .dma_rcc = RCC_DMA1,
        .pio_scl = {
            .base=GPIOB,
            .pin=GPIO8,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF4,
            .speed=GPIO_OSPEED_50MHZ,
            .optype=GPIO_OTYPE_OD,
            .pullupdown=GPIO_PUPD_PULLUP
        },
        .pio_sda = {
            .base=GPIOB,
            .pin=GPIO9,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF4,
            .speed=GPIO_OSPEED_50MHZ,
            .optype=GPIO_OTYPE_OD,
            .pullupdown=GPIO_PUPD_PULLUP
        },
    },
#endif
#ifdef CONFIG_I2C3
    {
        .idx  = 3,
        .base = I2C3,
        .ev_irq = NVIC_I2C3_EV_IRQ,
        .er_irq = NVIC_I2C3_ER_IRQ,
        .rcc = RCC_I2C3,

        .clock_f = I2C_CR2_FREQ_42MHZ,
        .fast_mode = 1,
        .rise_time = 11,
        .bus_clk_frequency = 10,

        .rx_dma = {
            .base = DMA1,
            .stream = DMA_STREAM2,
            .channel = DMA_SxCR_CHSEL_3,
            .psize =  DMA_SxCR_PSIZE_8BIT,
            .msize = DMA_SxCR_MSIZE_8BIT,
            .dirn = DMA_SxCR_DIR_PERIPHERAL_TO_MEM,
            .prio = DMA_SxCR_PL_VERY_HIGH,
            .paddr =  (uint32_t) &I2C_DR(I2C3),
            .irq = NVIC_DMA1_STREAM2_IRQ,
        },
        .tx_dma = {
            .base = DMA1,
            .stream = DMA_STREAM4,
            .channel = DMA_SxCR_CHSEL_3,
            .psize =  DMA_SxCR_PSIZE_8BIT,
            .msize = DMA_SxCR_MSIZE_8BIT,
            .dirn = DMA_SxCR_DIR_MEM_TO_PERIPHERAL,
            .prio = DMA_SxCR_PL_MEDIUM,
            .paddr =  (uint32_t) &I2C_DR(I2C3),
            .irq = NVIC_DMA1_STREAM4_IRQ,
        },
        .dma_rcc = RCC_DMA1,
        .pio_scl = {
            .base=GPIOH,
            .pin=GPIO7,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF4,
            .speed=GPIO_OSPEED_50MHZ,
            .optype=GPIO_OTYPE_OD,
            .pullupdown=GPIO_PUPD_PULLUP
        },
        .pio_sda = {
            .base=GPIOH,
            .pin=GPIO8,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF4,
            .speed=GPIO_OSPEED_50MHZ,
            .optype=GPIO_OTYPE_OD,
            .pullupdown=GPIO_PUPD_PULLUP
        },
    },
#endif
};
#define NUM_I2CS (sizeof(i2c_configs) / sizeof(struct i2c_config))

extern int ft5336_init(uint32_t);
extern int mccog21_init(uint32_t);

int machine_init(void)
{
    int i = 0;
    rcc_clock_setup_hse_3v3(&rcc_hse_25mhz_3v3);

    /* usb_init(&usb_hs_host); TODO */ 

    gpio_create(NULL, &gpio_led0);
    gpio_create(NULL, &gpio_button);
    /* UARTS */
    for (i = 0; i < NUM_UARTS; i++) {
        uart_create(&uart_configs[i]);
    }
    /* I2Cs */
    for (i = 0; i < NUM_I2CS; i++) {
        i2c_create(&i2c_configs[i]);
    }
    rng_create(1, RCC_RNG);
    sdio_conf.rcc_reg = (uint32_t *)&RCC_APB2ENR;
    sdio_conf.rcc_en  = RCC_APB2ENR_SDMMC1EN;
    sdio_init(&sdio_conf);
    /* Initialize USB OTG guest in full-speed mode */
    usb_init(&usb_fs_guest);


    ethernet_init(&eth_config);


    #ifdef CONFIG_DEVMCCOG21
        mccog21_init(1);
    #endif
    #ifdef CONFIG_DEVFT5336
       ft5336_init(3); /* FT5336 touch screen on I2C-3 */
    #endif
    return 0;
}

