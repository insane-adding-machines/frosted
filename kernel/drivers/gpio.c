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
#include "gpio.h"
#include <stdint.h>
#include "ioctl.h"
#include "poll.h"


#include "sys/frosted-io.h"

#if defined(STM32F4) || defined(STM32F7)
#include <unicore-mx/stm32/gpio.h>
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/cm3/nvic.h>

static inline uint32_t ARCH_GPIO_BASE(int x)
{
    switch(x) {
        case 1:
            return GPIOA;
        case 2:
            return GPIOB;
        case 3:
            return GPIOC;
        case 4:
            return GPIOD;
        case 5:
            return GPIOE;
        case 6:
            return GPIOF;
        case 7:
            return GPIOG;
        case 8:
            return GPIOH;
        case 9:
            return GPIOI;
        case 10:
            return GPIOJ;
        case 11:
            return GPIOK;
        default:
            return 0;
    }
}
#define ARCH_GPIO_PIN(X) (1 << X)
#define ARCH_GPIO_BASE_MAX 6
#define ARCH_GPIO_PIN_MAX 16


#include "stm32_exti.h"
#endif

#ifdef STM32F7
#include <unicore-mx/stm32/gpio.h>
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/cm3/nvic.h>
#endif
#ifdef LPC17XX
#include <unicore-mx/lpc17xx/nvic.h>
#include <unicore-mx/lpc17xx/gpio.h>
#include <unicore-mx/lpc17xx/exti.h>
static inline uint32_t ARCH_GPIO_BASE(int x)
{
    switch(x) {
        case 1:
            return GPIO1;
        case 2:
            return GPIO2;
        case 3:
            return GPIO3;
        case 4:
            return GPIO4;
        default:
            return 0;
    }
}
#define ARCH_GPIO_PIN(X) (1 << X)
#define ARCH_GPIO_BASE_MAX 4
#define ARCH_GPIO_PIN_MAX 32
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

struct dev_gpio *Gpio_list = NULL;

static void gpio_list_add(struct dev_gpio *pio)
{
    pio->next = Gpio_list;
    Gpio_list = pio;
}

static struct dev_gpio *gpio_list_find(uint32_t base, uint32_t pin)
{
    struct dev_gpio *cur = Gpio_list;
    while(cur) {
        if ((cur->base == base) && (cur->pin == pin))
            return cur;
        cur = cur->next;
    }
    return NULL;
}

static void gpio_list_del(struct dev_gpio *old)
{
    struct dev_gpio *cur = Gpio_list;
    struct dev_gpio *last = NULL;
    while(cur) {
        if (cur == old) {
            if (last == NULL)
                Gpio_list = cur->next;
            else
                last->next = cur->next;
            return;
        }
        last = cur;
        cur = cur->next;
    }
}

int gpio_read_list_entry(void **last__, char *buf, int len)
{
    struct dev_gpio **last = (struct dev_gpio **)last__;
    if (*last == NULL)
        *last = Gpio_list;
    else
        *last = (*last)->next;
    /* Line: port pid mode pupd trigger speed owner */
    /* type: u32  u32 s    s    s       u32   s     */
    //snprintf(buf, len, "%lu\t%lu\t%s\t%s\t%s\t%lu\t%s\r\n", 0, 0, "", "", "", 0, "");
    buf[0] = 0;
    return strlen(buf);
}


static int devgpio_write(struct fnode *fno, const void *buf, unsigned int len);
static int devgpio_ioctl(struct fnode *fno, const uint32_t cmd, void *arg);
static int devgpio_read (struct fnode *fno, void *buf, unsigned int len);
static int devgpio_poll (struct fnode *fno, uint16_t events, uint16_t *revents);

static int devgpiomx_ioctl(struct fnode * fno, const uint32_t cmd, void *arg);

static struct module mod_devgpio = {
    .family = FAMILY_FILE,
    .name = "gpio",
    .ops.open = device_open,
    .ops.read = devgpio_read,
    .ops.poll = devgpio_poll,
    .ops.write = devgpio_write,
    .ops.ioctl = devgpio_ioctl,
};

static struct module mod_devgpio_mx = {
    .family = FAMILY_FILE,
    .name = "gpio-mx",
    .ops.ioctl = devgpiomx_ioctl,
};


/****************/
/* HW specifics */
/****************/

/* LM3S */
#ifdef LM3S
#endif



/* STM32 */
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

/* LPC */
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


#ifdef CONFIG_DEVSTM32EXTI
static void gpio_isr(void *arg)
{
    struct dev_gpio *gpio = arg;
    exti_enable(gpio->exti_idx, 0);
    if (gpio->dev->pid > 0) {
        task_resume(gpio->dev->pid);
    }
}

static int gpio_set_trigger(struct dev_gpio *gpio, uint32_t trigger)
{
    uint32_t val = gpio_get(gpio->base, gpio->pin);
    gpio->trigger = (uint8_t) trigger;
    RESET_TRIGGER_WAITING(gpio);
    if (trigger == GPIO_TRIGGER_TOGGLE) {
        if (val)
            SET_TRIGGER_WAITING(gpio, GPIO_TRIGGER_FALL);
        else
            SET_TRIGGER_WAITING(gpio, GPIO_TRIGGER_RAISE);
    } else {
        SET_TRIGGER_WAITING(gpio, trigger);
    }
    return exti_register(gpio->base, gpio->pin, gpio->trigger, gpio_isr, gpio);
}

#endif

static int devgpiomx_ioctl(struct fnode * fno, const uint32_t cmd, void *arg)
{
    struct dev_gpiomx *gpiomx;
    struct gpio_req *req = arg;
    struct gpio_config addr = { };

    if ((req->base == 0) || (req->pin == 0))
        return -EINVAL;

    if ((req->base > ARCH_GPIO_BASE_MAX) || (req->pin > ARCH_GPIO_PIN_MAX))
        return -EINVAL;

    addr.base = ARCH_GPIO_BASE(req->base);
    addr.pin = ARCH_GPIO_PIN(req->pin);

    if (cmd == IOCTL_GPIOMX_CREATE) {
        if (gpio_list_find(addr.base, addr.pin))
            return -EEXIST;
        gpio_create(&mod_devgpio_mx, &addr );
        return 0;
    }
    if (cmd == IOCTL_GPIOMX_DESTROY) {
        return 0;
    }
    return -EINVAL;
}


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
        return -EEXIST;

    /* (protected GPIOs cannot be reconfigured from userspace) */
    if (IS_PROTECTED(gpio)) {
        return -EPERM;
    }

    if (cmd == IOCTL_GPIO_ENABLE) {
        return 0;
    }
    if (cmd == IOCTL_GPIO_DISABLE) {
        return 0;
    }
    if (cmd == IOCTL_GPIO_SET_INPUT) {
        SET_INPUT(gpio->base, GPIO_PUPD_NONE, gpio->pin);
        return 0;
    }
    if (cmd == IOCTL_GPIO_SET_OUTPUT) {
        SET_OUTPUT(gpio->base, gpio->pin, gpio->optype, gpio->speed)
        return 0;
    }
    if (cmd == IOCTL_GPIO_SET_PULLUPDOWN) {
        /* Setting pullup/down implies an input */
        SET_INPUT(gpio->base, *((uint32_t*)arg), gpio->pin);
        return 0;
    }
    if (cmd == IOCTL_GPIO_SET_ALT_FUNC) {
        gpio_set_af(gpio->base, *((uint32_t*)arg), gpio->pin);
        return 0;
    }
#   ifdef CONFIG_DEVSTM32EXTI
    if (cmd == IOCTL_GPIO_SET_TRIGGER) {
        uint32_t trigger = *((uint32_t *)arg);
        if (trigger > GPIO_TRIGGER_TOGGLE) {
            gpio->trigger = 0;
            return -EINVAL;
        }
        gpio->exti_idx = gpio_set_trigger(gpio, trigger);
        return 0;
    }
#endif
    return -EINVAL;
}

static int devgpio_read(struct fnode * fno, void *buf, unsigned int len)
{
    int out;
    struct dev_gpio *gpio;
    char *ptr = (char *)buf;
    uint8_t val = 0;

    gpio = (struct dev_gpio *)FNO_MOD_PRIV(fno, &mod_devgpio);
    if(!gpio)
        return -1;

    /* GPIO: get current value */
    val = gpio_get(gpio->base, gpio->pin);

#   ifdef CONFIG_DEVSTM32EXTI
    /* Unlock immediately */
    if ((gpio->trigger == GPIO_TRIGGER_NONE) || 
            (val && (gpio->trigger == GPIO_TRIGGER_RAISE)) || (!val && (gpio->trigger == GPIO_TRIGGER_FALL))) {
        *((uint8_t*)buf) = val ? '1':'0';
        return 1;
    } else if (gpio->trigger == GPIO_TRIGGER_TOGGLE) {
        if (TRIGGER_WAITING(gpio) == val) {
            *((uint8_t*)buf) = val ? '1':'0';
            RESET_TRIGGER_WAITING(gpio);
            SET_TRIGGER_WAITING(gpio, (val) ? GPIO_TRIGGER_FALL : GPIO_TRIGGER_RAISE);
            return 1;
        }
    }
    exti_enable(gpio->exti_idx, 1);
    return SYS_CALL_AGAIN;
#   else
    *((uint8_t*)buf) = val ? '1':'0';
    return 1;
#   endif
}


static int devgpio_poll(struct fnode * fno, uint16_t events, uint16_t *revents)
{
    int ret = 0;
    struct dev_gpio *gpio = (struct dev_gpio *)FNO_MOD_PRIV(fno, &mod_devgpio);
    if(!gpio)
        return -EEXIST;
    *revents = 0;
    if (events & POLLOUT) {
        *revents |= POLLOUT;
        ret = 1;
    }
    if (events == POLLIN) {
        uint8_t val;
        val = gpio_get(gpio->base, gpio->pin);
        if ((gpio->trigger == GPIO_TRIGGER_NONE)  || (val && (gpio->trigger == GPIO_TRIGGER_RAISE)) || (!val && (gpio->trigger == GPIO_TRIGGER_FALL))) {
            *revents |= POLLIN;
            ret = 1;
        }
    }
    return ret;
}


static void gpio_fno_init(struct dev_gpio *g, const char *name)
{
    struct fnode *devfs = fno_search("/dev");
    if (!devfs)
        return;

    g->dev = device_fno_init(&mod_devgpio, name, devfs, FL_RDWR, g);
    g->base = g->base;
    g->pin = g->pin;
    if (strcmp(name, LED0) == 0)
    {
        char gpio_name[32] = "/dev/";
        strcat(gpio_name, LED0);
        vfs_symlink(gpio_name, "/dev/led0");
    }
    if (strcmp(name, LED1) == 0)
    {
        char gpio_name[32] = "/dev/";
        strcat(gpio_name, LED1);
        vfs_symlink(gpio_name, "/dev/led1");
    }
    if (strcmp(name, LED2) == 0)
    {
        char gpio_name[32] = "/dev/";
        strcat(gpio_name, LED2);
        vfs_symlink(gpio_name, "/dev/led2");
    }
    if (strcmp(name, LED3) == 0)
    {
        char gpio_name[32] = "/dev/";
        strcat(gpio_name, LED3);
        vfs_symlink(gpio_name, "/dev/led3");
    }
}

int gpio_create(struct module *mod, const struct gpio_config *gpio_config)
{
    struct dev_gpio *gpio;
    if(gpio_config->base == 0)
        return -EINVAL;
    gpio = kalloc(sizeof(struct dev_gpio));
    if (!gpio)
        return -ENOMEM;

    memset(gpio, 0, sizeof(struct gpio_config));
    gpio->base = gpio_config->base;
    gpio->pin = gpio_config->pin;
    gpio->mode = gpio_config->mode;
    gpio->af = gpio_config->af;
    gpio->speed = gpio_config->speed;
    gpio->optype = gpio_config->optype;
    gpio->pullupdown = gpio_config->pullupdown;
    if (gpio_config->name) {
        gpio_fno_init(gpio, gpio_config->name);
    }

    if (mod != &mod_devgpio_mx) {
        gpio->flags |= GPIO_FL_PROTECTED;
    }

    if (mod)
        gpio->owner = mod;
    else
        gpio->owner = &mod_devgpio;
    
    GPIO_CLOCK_ENABLE(gpio_config->base);

    switch(gpio_config->mode)
    {
        case GPIO_MODE_INPUT:
            SET_INPUT(gpio_config->base, gpio_config->pullupdown, gpio_config->pin);
            break;
        case GPIO_MODE_OUTPUT:
            SET_OUTPUT(gpio_config->base, gpio_config->pin, gpio_config->optype, gpio_config->speed);
            break;
        case GPIO_MODE_AF:
            SET_AF(gpio_config->base, GPIO_MODE_AF, gpio_config->af,  gpio_config->pin);
            break;
        case GPIO_MODE_ANALOG:
            gpio_mode_setup(gpio_config->base, gpio_config->mode, GPIO_PUPD_NONE, gpio_config->pin);
            break;
    }
    gpio_list_add(gpio);
    return 0;
}

int gpio_init(void)
{
    static struct device *gpiomx_dev = NULL;
    struct fnode *devfs = fno_search("/dev");
    if (!devfs)
        return -ENOENT;

    gpiomx_dev = device_fno_init(&mod_devgpio_mx, "gpiomx", devfs, FL_RDWR, &mod_devgpio_mx);

    register_module(&mod_devgpio);
    register_module(&mod_devgpio_mx);
    return 0;
}

