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
 *      Authors:
 *
 */

#include "frosted.h"
#include "device.h"
#include <stdint.h>
#include "ioctl.h"
#include "poll.h"

#if defined(STM32F4) || defined(STM32F7)
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/nvic.h>
#endif
#ifdef STM32F7
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/nvic.h>
#endif
#ifdef LPC17XX
#include <libopencm3/lpc17xx/nvic.h>
#include <libopencm3/lpc17xx/gpio.h>
#include <libopencm3/lpc17xx/exti.h>
#endif
#ifdef LPC43XX
#include <libopencm3/lpc43xx/gpio.h>
#include <libopencm3/lpc43xx/scu.h>
#include <libopencm3/cm3/common.h>
#endif


#ifdef PYBOARD
# define LED0 "gpio_1_13"
# define LED1 "gpio_1_14"
# define LED2 "gpio_1_15"
# define LED3 "gpio_1_4"
#elif defined (STM32F7)
# define LED0 "gpio_9_1"
# define LED1 ""
# define LED2 ""
# define LED3 ""
#elif defined (STM32F4)
# if defined (F429DISCO)
#  define LED0 "gpio_6_13"
#  define LED1 "gpio_6_14"
# else
#  define LED0 "gpio_3_12"
#  define LED1 "gpio_3_13"
#endif
# define LED2 "gpio_3_14"
# define LED3 "gpio_3_15"
#elif defined (LPC43XX)
#  define LED0 "gpio_4_1"
#  define LED1 "gpio_4_2"
# define LED2 ""
# define LED3 ""
#elif defined (LPC17XX)
#if 0
/*LPCXpresso 1769 */
# define LED0 "/dev/gpio_0_22"
# define LED1 "/dev/null"
# define LED2 "/dev/null"
# define LED3 "/dev/null"
#else
/* mbed 1768 */
# define LED0 "gpio_1_18"
# define LED1 "gpio_1_20"
# define LED2 "gpio_1_21"
# define LED3 "gpio_1_23"
#endif
#else
# define LED0 "null"
# define LED1 "null"
# define LED2 "null"
# define LED3 "null"
#endif
#include "gpio.h"

struct dev_gpio {
    struct device * dev;
    uint32_t base;
    uint16_t pin;
};

#define MAX_GPIOS   32      /* FIX THIS! */

static struct dev_gpio DEV_GPIO[MAX_GPIOS];

static int devgpio_write(struct fnode *fno, const void *buf, unsigned int len);
static int devgpio_ioctl(struct fnode *fno, const uint32_t cmd, void *arg);
static int devgpio_read (struct fnode *fno, void *buf, unsigned int len);
static int devgpio_poll (struct fnode *fno, uint16_t events, uint16_t *revents);


static struct module mod_devgpio = {
    .family = FAMILY_FILE,
    .name = "gpio",
    .ops.open = device_open,
    .ops.read = devgpio_read,
    .ops.poll = devgpio_poll,
    .ops.write = devgpio_write,
    .ops.ioctl = devgpio_ioctl,
};

#ifdef LM3S
#endif
#if defined(STM32F4) || defined(STM32F7)
#define GPIO_CLOCK_ENABLE(P)       switch(P){    \
                                                                case GPIOA:rcc_periph_clock_enable(RCC_GPIOA);  break;  \
                                                                case GPIOB:rcc_periph_clock_enable(RCC_GPIOB);  break;  \
                                                                case GPIOC:rcc_periph_clock_enable(RCC_GPIOC);  break;  \
                                                                case GPIOD:rcc_periph_clock_enable(RCC_GPIOD);  break;  \
                                                                case GPIOE:rcc_periph_clock_enable(RCC_GPIOE);  break;  \
                                                                case GPIOF:rcc_periph_clock_enable(RCC_GPIOF);  break;  \
                                                                case GPIOG:rcc_periph_clock_enable(RCC_GPIOG);  break;  \
                                                                case GPIOH:rcc_periph_clock_enable(RCC_GPIOH);  break;  \
                                                                case GPIOI:rcc_periph_clock_enable(RCC_GPIOI);  break;  \
                                                                case GPIOJ:rcc_periph_clock_enable(RCC_GPIOJ);  break;  \
                                                                case GPIOK:rcc_periph_clock_enable(RCC_GPIOK);  break;  \
                                                                }

#define SET_INPUT(P, D, I)               gpio_mode_setup(P, GPIO_MODE_INPUT, D, I);

#define SET_OUTPUT(P, I, O, S)     gpio_mode_setup(P, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, I);   \
                                                          gpio_set_output_options(P, O, S, I);

#define SET_AF(P, M, A, I)              gpio_mode_setup(P, M, GPIO_PUPD_NONE, I);  \
                                                         gpio_set_af(P, A, I);


#endif

#ifdef LPC17XX
#define GPIO_CLOCK_ENABLE(C)

#define SET_INPUT(P, D, I)                  gpio_mode_setup(P, GPIO_MODE_INPUT, D, I);    \
                                                               gpio_set_af(P, GPIO_AF0, I);

#define SET_OUTPUT(P, I, O, S)          gpio_mode_setup(P, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, I);   \
                                                                gpio_set_af(P, GPIO_AF0, I);

#define SET_AF(P, M, A, I)                   gpio_mode_setup(P, M, GPIO_PUPD_NONE, I);    \
                                                                gpio_set_af(P, A, I);

void eint_isr(uint32_t exti_base)
{
    /* What next? */
    exti_clear_flag(exti_base);
}
void eint0_isr(void)
{
    eint_isr(EXTI0);
}
void eint1_isr(void)
{
    eint_isr(EXTI1);
}
void eint2_isr(void)
{
    eint_isr(EXTI2);
}
void eint3_isr(void)
{
    eint_isr(EXTI3);
}

#endif

#ifdef LPC43XX
#define GPIO_CLOCK_ENABLE(C)

#define SET_INPUT(P, D, I)

#define SET_OUTPUT(P, I, O, S)

#define SET_AF(P, M, A, I)
#endif

static int devgpio_write(struct fnode * fno, const void *buf, unsigned int len)
{
     struct dev_gpio *gpio;
    char *arg = (char *)buf;

    gpio = (struct dev_gpio *)FNO_MOD_PRIV(fno, &mod_devgpio);
    if(!gpio)
        return -1;

    if (arg[0] == '1') {
        gpio_set(gpio->base, gpio->pin);
        return 1;
    } else if (arg[0] == '0') {
        gpio_clear(gpio->base, gpio->pin);
        return 1;
    } else {
        return -1;
    }
}

static int devgpio_ioctl(struct fnode * fno, const uint32_t cmd, void *arg)
{
     struct dev_gpio *gpio;

    gpio = (struct dev_gpio *)FNO_MOD_PRIV(fno, &mod_devgpio);
    if(!gpio)
        return -1;

#ifndef LPC43XX
    if (cmd == IOCTL_GPIO_ENABLE) {
//        gpio_mode_setup(gpio->port, GPIO_MODE_INPUT,GPIO_PUPD_NONE, gpio->pin);
    }
    if (cmd == IOCTL_GPIO_DISABLE) {
//        gpio_mode_setup(gpio->port, GPIO_MODE_INPUT,GPIO_PUPD_NONE, gpio->pin);
    }
    if (cmd == IOCTL_GPIO_SET_INPUT) {
        /* user land commanded input defaults to no pull up/down*/
        SET_INPUT(gpio->base, GPIO_PUPD_NONE, gpio->pin);
    }
    if (cmd == IOCTL_GPIO_SET_OUTPUT) {
//        SET_OUTPUT(gpio->port, gpio->pin, gpio->optype, gpio->speed)
    }
    if (cmd == IOCTL_GPIO_SET_PULLUPDOWN) {
        /* Setting pullup/down implies an input */
        SET_INPUT(gpio->base, *((uint32_t*)arg), gpio->pin);
    }
    if (cmd == IOCTL_GPIO_SET_ALT_FUNC) {
         gpio_set_af(gpio->base, *((uint32_t*)arg), gpio->pin);
    }
#endif
    return 0;
}

static int devgpio_read(struct fnode * fno, void *buf, unsigned int len)
{
    int out;
    struct dev_gpio *gpio;
    char *ptr = (char *)buf;

    gpio = (struct dev_gpio *)FNO_MOD_PRIV(fno, &mod_devgpio);
    if(!gpio)
        return -1;

    /* GPIO: get current value */
#ifndef LPC43XX
    *((uint8_t*)buf) = gpio_get(gpio->base, gpio->pin) ? '1':'0';
#endif
    return 1;
}


static int devgpio_poll(struct fnode * fno, uint16_t events, uint16_t *revents)
{
    int ret = 0;
    *revents = 0;
    if (events & POLLOUT) {
        *revents |= POLLOUT;
        ret = 1;
    }
    if (events == POLLIN) {
        *revents |= POLLIN;
        ret = 1;
    }
    return ret;
}


static void gpio_fno_init(struct fnode *dev, uint32_t n, const struct gpio_addr * addr)
{
    struct dev_gpio *g = &DEV_GPIO[n];
    g->dev = device_fno_init(&mod_devgpio, addr->name, dev, FL_RDWR, g);
    g->base = addr->base;
    g->pin = addr->pin;
}

void gpio_init(struct fnode * dev,  const struct gpio_addr gpio_addrs[], int num_gpios)
{
    int i;
    uint32_t exti,exti_irq;

    for(i=0;i<num_gpios;i++)
    {
        if(gpio_addrs[i].base == 0)
            continue;

        gpio_fno_init(dev, i, &gpio_addrs[i]);

        if (gpio_addrs[i].name) {

            if (strcmp(gpio_addrs[i].name, LED0) == 0)
            {
                char gpio_name[32] = "/dev/";
                strcat(gpio_name, LED0);
                vfs_symlink(gpio_name, "/dev/led0");
            }
            if (strcmp(gpio_addrs[i].name, LED1) == 0)
            {
                char gpio_name[32] = "/dev/";
                strcat(gpio_name, LED1);
                vfs_symlink(gpio_name, "/dev/led1");
            }
            if (strcmp(gpio_addrs[i].name, LED2) == 0)
            {
                char gpio_name[32] = "/dev/";
                strcat(gpio_name, LED2);
                vfs_symlink(gpio_name, "/dev/led2");
            }
            if (strcmp(gpio_addrs[i].name, LED3) == 0)
            {
                char gpio_name[32] = "/dev/";
                strcat(gpio_name, LED3);
                vfs_symlink(gpio_name, "/dev/led3");
            }
        }

#ifndef LPC43XX
        GPIO_CLOCK_ENABLE(gpio_addrs[i].base);

        switch(gpio_addrs[i].mode)
        {
            case GPIO_MODE_INPUT:
                SET_INPUT(gpio_addrs[i].base, gpio_addrs[i].pullupdown, gpio_addrs[i].pin);
                break;
            case GPIO_MODE_OUTPUT:
                SET_OUTPUT(gpio_addrs[i].base, gpio_addrs[i].pin, gpio_addrs[i].optype, gpio_addrs[i].speed);
                break;
            case GPIO_MODE_AF:
                SET_AF(gpio_addrs[i].base, GPIO_MODE_AF, gpio_addrs[i].af,  gpio_addrs[i].pin);
#ifdef LPC17XX
                if((gpio_addrs[i].base == GPIO2) && (gpio_addrs[i].af == GPIO_AF1))
                {
                    /* XXX */
                    /* Insert updated EXTI code for LPC17XX */
                }
#endif
                break;
            case GPIO_MODE_ANALOG:
                gpio_mode_setup(gpio_addrs[i].base, gpio_addrs[i].mode, GPIO_PUPD_NONE, gpio_addrs[i].pin);
                break;
        }
#else
/* GPIO Output PinMux */
#define SCU_PINMUX_LED1     (P4_1) /* GPIO2[1] on P4_1 */
#define SCU_PINMUX_LED2     (P4_2) /* GPIO2[2] on P4_2 */
#define SCU_PINMUX_LED3     (P6_12) /* GPIO2[8] on P6_12 */
/* GPIO Output */
#define PIN_LED1    (BIT1) /* GPIO2[1] on P4_1 */
#define PIN_LED2    (BIT2) /* GPIO2[2] on P4_2 */
#define PIN_LED3    (BIT8) /* GPIO2[8] on P6_12 */
    /* Configure SCU Pin Mux as GPIO */
    scu_pinmux(SCU_PINMUX_LED1, SCU_GPIO_FAST);
    scu_pinmux(SCU_PINMUX_LED2, SCU_GPIO_FAST);
    //scu_pinmux(SCU_PINMUX_LED3, SCU_GPIO_FAST);

    //scu_pinmux(SCU_PINMUX_EN1V8, SCU_GPIO_FAST);

    //scu_pinmux(SCU_PINMUX_BOOT0, SCU_GPIO_FAST);
    //scu_pinmux(SCU_PINMUX_BOOT1, SCU_GPIO_FAST);
    //scu_pinmux(SCU_PINMUX_BOOT2, SCU_GPIO_FAST);
    //scu_pinmux(SCU_PINMUX_BOOT3, SCU_GPIO_FAST);

    /* Configure all GPIO as Input (safe state) */
    GPIO0_DIR = 0;
    GPIO1_DIR = 0;
    GPIO2_DIR = 0;
    GPIO3_DIR = 0;
    GPIO4_DIR = 0;
    GPIO5_DIR = 0;
    GPIO6_DIR = 0;
    GPIO7_DIR = 0;

    /* Configure GPIO as Output */
    GPIO2_DIR |= (PIN_LED1|PIN_LED2); /* Configure GPIO2[1/2/8] (P4_1/2 P6_12) as output. */
#endif
    }

    register_module(&mod_devgpio);
}
