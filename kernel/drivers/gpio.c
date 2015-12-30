#include "frosted.h"
#include "device.h"
#include <stdint.h>
#include "ioctl.h"
#include "poll.h"
#ifdef STM32F4
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/nvic.h>
#endif
#ifdef LPC17XX
#include <libopencm3/lpc17xx/nvic.h>
#include <libopencm3/lpc17xx/gpio.h>
#include <libopencm3/lpc17xx/exti.h>
#endif
#include "gpio.h"

struct dev_gpio {
    struct device * dev;
    uint32_t base;
    uint16_t pin;
    exti_callback exti_callback_fn;
    void * exti_callback_arg;
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
#ifdef STM32F4
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


struct dev_gpio * exti_fno[16];

void gpio_register_exti_callback(struct fnode *fno, exti_callback callback_fn, void * callback_arg)
{
    struct dev_gpio *gpio = (struct dev_gpio *)FNO_MOD_PRIV(fno, &mod_devgpio);
    if(!gpio)
        return;

    gpio->exti_callback_fn = callback_fn;
    gpio->exti_callback_arg = callback_arg;
}

void exti_isr(uint32_t exti_base, uint32_t exti_idx)
{
    exti_reset_request(exti_base);
    if(exti_fno[exti_idx]->exti_callback_fn)
    {
        tasklet_add(exti_fno[exti_idx]->exti_callback_fn, exti_fno[exti_idx]->exti_callback_arg);
    }
}
void exti0_isr(void)
{
    exti_isr(EXTI0, 0);
}
void exti1_isr(void)
{
    exti_isr(EXTI1, 1);
}
void exti2_isr(void)
{
    exti_isr(EXTI2, 2);
}
void exti3_isr(void)
{
    exti_isr(EXTI3, 3);
}
void exti4_isr(void)
{
    exti_isr(EXTI4, 4);
}
void exti9_5_isr(void)
{
    if(exti_get_flag_status(EXTI5))exti_isr(EXTI5, 5);
    if(exti_get_flag_status(EXTI6))exti_isr(EXTI6, 6);
    if(exti_get_flag_status(EXTI7))exti_isr(EXTI7, 7);
    if(exti_get_flag_status(EXTI8))exti_isr(EXTI8, 8);
    if(exti_get_flag_status(EXTI9))exti_isr(EXTI9, 9);
}
void exti15_10_isr(void)
{
    if(exti_get_flag_status(EXTI10))exti_isr(EXTI10, 10);
    if(exti_get_flag_status(EXTI11))exti_isr(EXTI11, 11);
    if(exti_get_flag_status(EXTI12))exti_isr(EXTI12, 12);
    if(exti_get_flag_status(EXTI13))exti_isr(EXTI13, 13);
    if(exti_get_flag_status(EXTI14))exti_isr(EXTI14, 14);
    if(exti_get_flag_status(EXTI15))exti_isr(EXTI15, 15);
}
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
        SET_INPUT(gpio->base, GPIO_PUPD_NONE, gpio->pin)
    }
    if (cmd == IOCTL_GPIO_SET_OUTPUT) {
//        SET_OUTPUT(gpio->port, gpio->pin, gpio->optype, gpio->speed)
    }
    if (cmd == IOCTL_GPIO_SET_PULLUPDOWN) {
        /* Setting pullup/down implies an input */
        SET_INPUT(gpio->base, *((uint32_t*)arg), gpio->pin)
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
    g->exti_callback_fn = NULL;
    g->exti_callback_arg = 0;
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
    
        GPIO_CLOCK_ENABLE(gpio_addrs[i].base)
            
        switch(gpio_addrs[i].mode)
        {
            case GPIO_MODE_INPUT:
                SET_INPUT(gpio_addrs[i].base, gpio_addrs[i].pullupdown, gpio_addrs[i].pin)
                break;
            case GPIO_MODE_OUTPUT:
                SET_OUTPUT(gpio_addrs[i].base, gpio_addrs[i].pin, gpio_addrs[i].optype, gpio_addrs[i].speed)
                break;
            case GPIO_MODE_AF:
                SET_AF(gpio_addrs[i].base, GPIO_MODE_AF, gpio_addrs[i].af,  gpio_addrs[i].pin)
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
                            nvic_enable_irq(NVIC_EINT0_IRQ);                    break;
                            break;
                        case GPIOPIN11: 
                            nvic_disable_irq(NVIC_EINT1_IRQ);
                            exti_set_trigger(EXTI1, gpio_addrs[i].trigger);
                            nvic_clear_pending_irq(NVIC_EINT1_IRQ);
                            exti_clear_flag(EXTI1);
                            nvic_enable_irq(NVIC_EINT1_IRQ);                    break;
                            break;
                        case GPIOPIN12: 
                            nvic_disable_irq(NVIC_EINT2_IRQ);
                            exti_set_trigger(EXTI2, gpio_addrs[i].trigger);
                            nvic_clear_pending_irq(NVIC_EINT2_IRQ);
                            exti_clear_flag(EXTI2);
                            nvic_enable_irq(NVIC_EINT2_IRQ);                    break;
                            break;
                        case GPIOPIN13: 
                            nvic_disable_irq(NVIC_EINT3_IRQ);
                            exti_set_trigger(EXTI3, gpio_addrs[i].trigger);
                            nvic_clear_pending_irq(NVIC_EINT3_IRQ);
                            exti_clear_flag(EXTI3);
                            nvic_enable_irq(NVIC_EINT3_IRQ);                    break;
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

        if(gpio_addrs[i].exti)
        {
#ifdef STM32F4
            switch(gpio_addrs[i].pin)
            {
                case GPIO0:     exti = EXTI0;       exti_irq = NVIC_EXTI0_IRQ;  exti_fno[0] = &DEV_GPIO[i];  break;
                case GPIO1:     exti = EXTI1;       exti_irq = NVIC_EXTI1_IRQ;  exti_fno[1] = &DEV_GPIO[i];  break;
                case GPIO2:     exti = EXTI2;       exti_irq = NVIC_EXTI2_IRQ;  exti_fno[2] = &DEV_GPIO[i];  break;
                case GPIO3:     exti = EXTI3;       exti_irq = NVIC_EXTI3_IRQ;  exti_fno[3] = &DEV_GPIO[i];  break;
                case GPIO4:     exti = EXTI4;       exti_irq = NVIC_EXTI4_IRQ;  exti_fno[4] = &DEV_GPIO[i];  break;
                case GPIO5:     exti = EXTI5;       exti_irq = NVIC_EXTI4_IRQ;  exti_fno[5] = &DEV_GPIO[i];  break;
                case GPIO6:     exti = EXTI6;       exti_irq = NVIC_EXTI4_IRQ;  exti_fno[6] = &DEV_GPIO[i];  break;
                case GPIO7:     exti = EXTI7;       exti_irq = NVIC_EXTI4_IRQ;  exti_fno[7] = &DEV_GPIO[i];  break;
                case GPIO8:     exti = EXTI8;       exti_irq = NVIC_EXTI4_IRQ;  exti_fno[8] = &DEV_GPIO[i];  break;
                case GPIO9:     exti = EXTI9;       exti_irq = NVIC_EXTI4_IRQ;  exti_fno[9] = &DEV_GPIO[i];  break;
                case GPIO10:    exti = EXTI10;       exti_irq = NVIC_EXTI4_IRQ;  exti_fno[10] = &DEV_GPIO[i];  break;
                case GPIO11:    exti = EXTI11;       exti_irq = NVIC_EXTI4_IRQ;  exti_fno[11] = &DEV_GPIO[i];  break;
                case GPIO12:    exti = EXTI12;       exti_irq = NVIC_EXTI4_IRQ;  exti_fno[12] = &DEV_GPIO[i];  break;
                case GPIO13:    exti = EXTI13;       exti_irq = NVIC_EXTI4_IRQ;  exti_fno[13] = &DEV_GPIO[i];  break;
                case GPIO14:    exti = EXTI14;       exti_irq = NVIC_EXTI4_IRQ;  exti_fno[14] = &DEV_GPIO[i];  break;
                case GPIO15:    exti = EXTI15;       exti_irq = NVIC_EXTI4_IRQ;  exti_fno[15] = &DEV_GPIO[i];  break;
                /* More to add here 5_9 10_15 */
                default: break;
            }
            
            nvic_enable_irq(exti_irq);
            exti_select_source(exti, gpio_addrs[i].base);
            exti_set_trigger(exti, gpio_addrs[i].trigger);
            exti_enable_request(exti);
#endif
        }
        
    }
    register_module(&mod_devgpio);
}

