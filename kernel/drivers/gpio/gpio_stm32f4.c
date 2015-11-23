#include "frosted.h"
#include <stdint.h>
#include "ioctl.h"

#if defined(STM32F4)
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#endif

static int gpio_subsys_initialized = 0;

static struct module mod_devgpio = {
};

static struct fnode *gpio_3_12 = NULL;
static struct fnode *gpio_3_13 = NULL;
static struct fnode *gpio_3_14 = NULL;
static struct fnode *gpio_3_15 = NULL;

struct gpio_addr {
    uint32_t port;
    uint32_t n;
    uint32_t rcc;
};

static struct gpio_addr a_pio_3_12 = {GPIOD, GPIO12, RCC_GPIOD};
static struct gpio_addr a_pio_3_13 = {GPIOD, GPIO13, RCC_GPIOD};
static struct gpio_addr a_pio_3_14 = {GPIOD, GPIO14, RCC_GPIOD};
static struct gpio_addr a_pio_3_15 = {GPIOD, GPIO15, RCC_GPIOD};

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
    struct gpio_addr *a;
    char *arg = (char *)buf;

    if (gpio_check_fd(fd, &fno) != 0)
        return -1;
    a = fno->priv;

    if (arg[0] == '1') {
        gpio_set(a->port, a->n);
        return 1;
    } else if (arg[0] == '0') {
        gpio_clear(a->port, a->n);
        return 1;
    } else {
        return -1;
    }
}

static int devgpio_ioctl(int fd, const uint32_t cmd, void *arg)
{
    struct fnode *fno;
    struct gpio_addr *a;
    if (gpio_check_fd(fd, &fno) != 0)
        return -1;
    a = fno->priv;
    if (cmd == IOCTL_GPIO_ENABLE) {
        rcc_periph_clock_enable(a->rcc);
        gpio_mode_setup(a->port, GPIO_MODE_INPUT,GPIO_PUPD_NONE, a->n);
    }
    if (cmd == IOCTL_GPIO_DISABLE) {
        gpio_mode_setup(a->port, GPIO_MODE_INPUT,GPIO_PUPD_NONE, a->n);
    }
    if (cmd == IOCTL_GPIO_SET_INPUT) {
        gpio_mode_setup(a->port, GPIO_MODE_INPUT,GPIO_PUPD_NONE, a->n);
    }
    if (cmd == IOCTL_GPIO_SET_OUTPUT) {
        gpio_set_output_options(a->port, 0, 0, a->n);
        gpio_mode_setup(a->port, GPIO_MODE_OUTPUT,GPIO_PUPD_NONE, a->n);
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

void devgpio_init(struct fnode *dev)
{
    gpio_mutex = frosted_mutex_init();
    mod_devgpio.family = FAMILY_FILE;
    mod_devgpio.ops.open = devgpio_open;
    mod_devgpio.ops.read = devgpio_read; 
    mod_devgpio.ops.poll = devgpio_poll;
    mod_devgpio.ops.write = devgpio_write;
    mod_devgpio.ops.ioctl = devgpio_ioctl;
    

    if (!gpio_subsys_initialized) {
//        hal_iodev_on(&GPIOD);
        gpio_subsys_initialized++;
    }


    gpio_3_12 = fno_create(&mod_devgpio, "gpio_3_12", dev);
    if (gpio_3_12)
        gpio_3_12->priv = &a_pio_3_12;

    gpio_3_13 = fno_create(&mod_devgpio, "gpio_3_13", dev);
    if (gpio_3_13)
        gpio_3_13->priv = &a_pio_3_13;

    gpio_3_14 = fno_create(&mod_devgpio, "gpio_3_14", dev);
    if (gpio_3_14)
        gpio_3_14->priv = &a_pio_3_14;

    gpio_3_15 = fno_create(&mod_devgpio, "gpio_3_15", dev);
    if (gpio_3_15)
        gpio_3_15->priv = &a_pio_3_15;

    klog(LOG_INFO, "GPIO Driver: KLOG enabled.\n");
    register_module(&mod_devgpio);
}

