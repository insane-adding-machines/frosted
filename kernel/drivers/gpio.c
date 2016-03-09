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
    *((uint8_t*)buf) = gpio_get(gpio->base, gpio->pin) ? '1':'0';
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
                    switch(gpio_addrs[i].pin)
                    {
                        /* LPC17XX only supports EXTI on P2.10 - P2.13
                            Doesn't seem to matter what we do here we always
                            get an interrupt when configuring the EXTI :( */
                        case GPIOPIN10:
                            nvic_disable_irq(NVIC_EINT0_IRQ);
                            exti_set_trigger(EXTI0, gpio_addrs[i].trigger);
                            exti_clear_flag(EXTI0);
                            nvic_clear_pending_irq(NVIC_EINT0_IRQ);
                            nvic_enable_irq(NVIC_EINT0_IRQ);
                            break;
                        case GPIOPIN11:
                            nvic_disable_irq(NVIC_EINT1_IRQ);
                            exti_set_trigger(EXTI1, gpio_addrs[i].trigger);
                            nvic_clear_pending_irq(NVIC_EINT1_IRQ);
                            exti_clear_flag(EXTI1);
                            nvic_enable_irq(NVIC_EINT1_IRQ);
                            break;
                        case GPIOPIN12:
                            nvic_disable_irq(NVIC_EINT2_IRQ);
                            exti_set_trigger(EXTI2, gpio_addrs[i].trigger);
                            nvic_clear_pending_irq(NVIC_EINT2_IRQ);
                            exti_clear_flag(EXTI2);
                            nvic_enable_irq(NVIC_EINT2_IRQ);
                            break;
                        case GPIOPIN13:
                            nvic_disable_irq(NVIC_EINT3_IRQ);
                            exti_set_trigger(EXTI3, gpio_addrs[i].trigger);
                            nvic_clear_pending_irq(NVIC_EINT3_IRQ);
                            exti_clear_flag(EXTI3);
                            nvic_enable_irq(NVIC_EINT3_IRQ);
                            break;
                        default:
                            break;
                    }
                }
#endif
                break;
            case GPIO_MODE_ANALOG:
                gpio_mode_setup(gpio_addrs[i].base, gpio_addrs[i].mode, GPIO_PUPD_NONE, gpio_addrs[i].pin);
                break;
        }
    }

    register_module(&mod_devgpio);
}
