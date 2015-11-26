#include "frosted.h"
#include <stdint.h>
#include "ioctl.h"

#ifdef LM3S
#endif
#ifdef STM32F4
#include <libopencm3/stm32/gpio.h>
#define CLOCK_ENABLE(C)                 rcc_periph_clock_enable(C);

#define SET_INPUT(P, M, D,I)               gpio_mode_setup(P, M, D, I);

#define SET_OUTPUT(P, M, I, O, S)     gpio_mode_setup(P, M, GPIO_PUPD_NONE, I);   \
                                                                gpio_set_output_options(P, O, S, I);
#endif

#ifdef LPC17XX
#include <libopencm3/lpc17xx/gpio.h>
#define CLOCK_ENABLE(C) 

#define SET_INPUT(P, M, D,I)              gpio_mode_setup(P, M, D, I);    \
                                                               gpio_set_af(P, GPIO_AF0, I);

#define SET_OUTPUT(P, M, I, O, S)     gpio_mode_setup(P, M, GPIO_PUPD_NONE, I);   \
                                                                gpio_set_af(P, GPIO_AF0, I);
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
//        gpio_mode_setup(a->port, GPIO_MODE_INPUT,GPIO_PUPD_NONE, a->pin);
    }
    if (cmd == IOCTL_GPIO_SET_OUTPUT) {
//        gpio_set_output_options(a->port, 0, 0, a->pin);
//        gpio_mode_setup(a->port, GPIO_MODE_OUTPUT,GPIO_PUPD_NONE, a->pin);
    }
    if (cmd == IOCTL_GPIO_SET_MODE) {
        /* TODO: Set pullup/down or int on edges... */
    }
    if (cmd == IOCTL_GPIO_SET_FUNC) {
    }
        /* TODO: Set alternate function for pin */
    return 0;
}

static int devgpio_read(int fd, void *buf, unsigned int len)
{
    int out;
    volatile int len_available = 1;
    char *ptr = (char *)buf;

    if (len_available < len) {
        gpio_pid = scheduler_get_cur_pid();
        task_suspend();
        out = SYS_CALL_AGAIN;
        frosted_mutex_unlock(gpio_mutex);
    } else {
        /* GPIO: get current value */
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
    struct fnode *node;

    struct module * devgpio = devgpio_init(dev);

    for(i=0;i<num_gpios;i++)
    {
        CLOCK_ENABLE(gpio_addrs[i].rcc)

        switch(gpio_addrs[i].mode)
        {
            case GPIO_MODE_INPUT:
                SET_INPUT(gpio_addrs[i].port, gpio_addrs[i].mode, gpio_addrs[i].pullupdown, gpio_addrs[i].pin)
                break;
            case GPIO_MODE_OUTPUT:
                SET_OUTPUT(gpio_addrs[i].port, gpio_addrs[i].mode, gpio_addrs[i].pin, gpio_addrs[i].optype, gpio_addrs[i].speed)
                break;
            case GPIO_MODE_AF:
                gpio_mode_setup(gpio_addrs[i].port, gpio_addrs[i].mode, GPIO_PUPD_NONE, gpio_addrs[i].pin);
                gpio_set_af(gpio_addrs[i].port, gpio_addrs[i].af, gpio_addrs[i].pin);
                break;
            case GPIO_MODE_ANALOG:
                gpio_mode_setup(gpio_addrs[i].port, gpio_addrs[i].mode, GPIO_PUPD_NONE, gpio_addrs[i].pin);
                break;
        }
         
        if(gpio_addrs[i].name)
        {
            node = fno_create(devgpio, gpio_addrs[i].name, dev);
            if (node)
                node->priv = &gpio_addrs[i];
        }
    }
    register_module(devgpio);
}

