#include "frosted.h"
#include <stdint.h>
#include "ioctl.h"

#ifdef LM3S
#endif
#ifdef STM32F4
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/nvic.h>
#define GPIO_CLOCK_ENABLE(P, E)       switch(P){    \
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
                                                                
#define SET_INPUT(P, M, D,I)               gpio_mode_setup(P, M, D, I);

#define SET_OUTPUT(P, M, I, O, S)     gpio_mode_setup(P, M, GPIO_PUPD_NONE, I);   \
                                                                gpio_set_output_options(P, O, S, I);

#define SET_AF(P, M, A, I)                   gpio_mode_setup(P, M, GPIO_PUPD_NONE, I);  \
                                                                gpio_set_af(P, A, I);


void exti_isr(uint32_t exti_base)
{
    exti_reset_request(exti_base);
    /* What next? */
}
void exti0_isr(void)
{
    exti_isr(EXTI0);
}
void exti1_isr(void)
{
    exti_isr(EXTI1);
}
void exti2_isr(void)
{
    exti_isr(EXTI2);
}
void exti3_isr(void)
{
    exti_isr(EXTI3);
}
void exti4_isr(void)
{
    exti_isr(EXTI4);
}
void exti9_5_isr(void)
{

}
void exti15_10_isr(void)
{
}
#endif

#ifdef LPC17XX
#include <libopencm3/lpc17xx/gpio.h>
#define GPIO_CLOCK_ENABLE(C, E) 

#define SET_INPUT(P, M, D,I)              gpio_mode_setup(P, M, D, I);    \
                                                               gpio_set_af(P, GPIO_AF0, I);

#define SET_OUTPUT(P, M, I, O, S)     gpio_mode_setup(P, M, GPIO_PUPD_NONE, I);   \
                                                                gpio_set_af(P, GPIO_AF0, I);
#define SET_AF(P, M, A, I)
#endif

#include "gpio.h"

static int gpio_subsys_initialized = 0;

static struct module mod_devgpio = {
};


static int gpio_check_fd(int fd, struct fnode **fno)
{
    *fno = task_filedesc_get(fd);
    
    if (!fno)
        return -1;
    if (fd < 0)
        return -1;

    if ((*fno)->owner != &mod_devgpio)
        return -1;

    return 0;
}


static int devgpio_write(int fd, const void *buf, unsigned int len);

/* Use static state for now. Future drivers can have multiple structs for this. */
static int gpio_pid = 0;
static mutex_t *gpio_mutex;

void GPIO_Handler(void)
{

    /* If a process is attached, resume the process */
    if (gpio_pid > 0) 
        task_resume(gpio_pid);
}

static int devgpio_write(int fd, const void *buf, unsigned int len)
{
    struct fnode *fno;
    const struct gpio_addr *a;
    char *arg = (char *)buf;

    if (gpio_check_fd(fd, &fno) != 0)
        return -1;
    a = fno->priv;

    if (arg[0] == '1') {
        gpio_set(a->port, a->pin);
        return 1;
    } else if (arg[0] == '0') {
        gpio_clear(a->port, a->pin);
        return 1;
    } else {
        return -1;
    }
}

static int devgpio_ioctl(int fd, const uint32_t cmd, void *arg)
{
    struct fnode *fno;
    const struct gpio_addr *a;
    if (gpio_check_fd(fd, &fno) != 0)
        return -1;
    a = fno->priv;
    if (cmd == IOCTL_GPIO_ENABLE) {
//        gpio_mode_setup(a->port, GPIO_MODE_INPUT,GPIO_PUPD_NONE, a->pin);
    }
    if (cmd == IOCTL_GPIO_DISABLE) {
//        gpio_mode_setup(a->port, GPIO_MODE_INPUT,GPIO_PUPD_NONE, a->pin);
    }
    if (cmd == IOCTL_GPIO_SET_INPUT) {
        /* user land commanded input defaults to no pull up/down*/
        SET_INPUT(a->port, GPIO_MODE_INPUT, GPIO_PUPD_NONE, a->pin)
    }
    if (cmd == IOCTL_GPIO_SET_OUTPUT) {
        SET_OUTPUT(a->port, GPIO_MODE_OUTPUT, a->pin, a->optype, a->speed)
    }
    if (cmd == IOCTL_GPIO_SET_PULLUPDOWN) {
        /* Setting pullup/down implies an input */
        SET_INPUT(a->port, GPIO_MODE_INPUT, *((uint32_t*)arg), a->pin)
    }
    if (cmd == IOCTL_GPIO_SET_ALT_FUNC) {
         gpio_set_af(a->port, *((uint32_t*)arg), a->pin);
    }
    return 0;
}

static int devgpio_read(int fd, void *buf, unsigned int len)
{
    int out;
    struct fnode *fno;
    const struct gpio_addr *a;
    volatile int len_available = 1;
    char *ptr = (char *)buf;

    if (gpio_check_fd(fd, &fno) != 0)
        return -1;
    a = fno->priv;

    if (len_available < len) {
        gpio_pid = scheduler_get_cur_pid();
        task_suspend();
        out = SYS_CALL_AGAIN;
        frosted_mutex_unlock(gpio_mutex);
    } else {
        /* GPIO: get current value */
        *((uint8_t*)buf) = gpio_get(a->port, a->pin) ? '1':'0';
        return 1;
    }
}


static int devgpio_poll(int fd, uint16_t events, uint16_t *revents)
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

static int devgpio_open(const char *path, int flags)
{
    struct fnode *f = fno_search(path);
    if (!f)
        return -1;
    return task_filedesc_add(f); 
}

static struct module *  devgpio_init(struct fnode *dev)
{
    gpio_mutex = frosted_mutex_init();
    mod_devgpio.family = FAMILY_FILE;
    strcpy(mod_devgpio.name,"gpio");
    mod_devgpio.ops.open = devgpio_open;
    mod_devgpio.ops.read = devgpio_read; 
    mod_devgpio.ops.poll = devgpio_poll;
    mod_devgpio.ops.write = devgpio_write;
    mod_devgpio.ops.ioctl = devgpio_ioctl;

    /* Module initialization */
    if (!gpio_subsys_initialized) {
        gpio_subsys_initialized++;
    }

    klog(LOG_INFO, "GPIO Driver: KLOG enabled.\n");
    return &mod_devgpio;
}

void gpio_init(struct fnode * dev,  const struct gpio_addr gpio_addrs[], int num_gpios)
{
    int i;
    uint32_t exti,exti_irq;
    struct fnode *node;

    struct module * devgpio = devgpio_init(dev);

    for(i=0;i<num_gpios;i++)
    {
        GPIO_CLOCK_ENABLE(gpio_addrs[i].port, gpio_addrs[i].exti)
            
        switch(gpio_addrs[i].mode)
        {
            case GPIO_MODE_INPUT:
                SET_INPUT(gpio_addrs[i].port, gpio_addrs[i].mode, gpio_addrs[i].pullupdown, gpio_addrs[i].pin)
                break;
            case GPIO_MODE_OUTPUT:
                SET_OUTPUT(gpio_addrs[i].port, gpio_addrs[i].mode, gpio_addrs[i].pin, gpio_addrs[i].optype, gpio_addrs[i].speed)
                break;
            case GPIO_MODE_AF:
                SET_AF(gpio_addrs[i].port, gpio_addrs[i].mode, gpio_addrs[i].af,  gpio_addrs[i].pin)
                break;
            case GPIO_MODE_ANALOG:
                gpio_mode_setup(gpio_addrs[i].port, gpio_addrs[i].mode, GPIO_PUPD_NONE, gpio_addrs[i].pin);
                break;
        }
#ifdef STM32F4
        if(gpio_addrs[i].exti)
        {
            switch(gpio_addrs[i].pin)
            {
                case GPIO0:     exti = EXTI0;       exti_irq = NVIC_EXTI0_IRQ;  break;
                case GPIO1:     exti = EXTI1;       exti_irq = NVIC_EXTI1_IRQ;  break;
                case GPIO2:     exti = EXTI2;       exti_irq = NVIC_EXTI2_IRQ;  break;
                case GPIO3:     exti = EXTI3;       exti_irq = NVIC_EXTI3_IRQ;  break;
                case GPIO4:     exti = EXTI4;       exti_irq = NVIC_EXTI4_IRQ;  break;
            }
            nvic_enable_irq(exti_irq);
            exti_select_source(exti, gpio_addrs[i].port);
            exti_set_trigger(exti, gpio_addrs[i].trigger);
            exti_enable_request(exti);
        }
#endif         
        if(gpio_addrs[i].name)
        {
            node = fno_create(devgpio, gpio_addrs[i].name, dev);
            if (node)
                node->priv = &gpio_addrs[i];
        }
    }
    register_module(devgpio);
}

