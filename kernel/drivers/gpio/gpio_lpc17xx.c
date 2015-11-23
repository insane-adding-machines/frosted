#include "frosted.h"
#include <stdint.h>
#include "ioctl.h"

#include <libopencm3/lpc17xx/gpio.h>

static int gpio_subsys_initialized = 0;

static struct module mod_devgpio = {
};

static struct fnode *gpio_1_18 = NULL;
static struct fnode *gpio_1_20 = NULL;
static struct fnode *gpio_1_21 = NULL;
static struct fnode *gpio_1_23 = NULL;

struct gpio_addr {
    uint32_t port;
    uint32_t n;
};

static struct gpio_addr a_pio_1_18 = {GPIO1, GPIOPIN18};
static struct gpio_addr a_pio_1_20 = {GPIO1, GPIOPIN20};
static struct gpio_addr a_pio_1_21 = {GPIO1, GPIOPIN21};
static struct gpio_addr a_pio_1_23 = {GPIO1, GPIOPIN23};

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
    }
    if (cmd == IOCTL_GPIO_DISABLE) {
    }
    if (cmd == IOCTL_GPIO_SET_OUTPUT) {
        GPIO1_DIR |= a->n; 
    }
    if (cmd == IOCTL_GPIO_SET_INPUT) {
        GPIO1_DIR &= ~(a->n); 
    }
    if (cmd == IOCTL_GPIO_SET_MODE) {
    }
    if (cmd == IOCTL_GPIO_SET_FUNC) {
    }
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
    
    /* Module initialization */
    if (!gpio_subsys_initialized) {
        gpio_subsys_initialized++;
    }


    gpio_1_18 = fno_create(&mod_devgpio, "gpio_1_18", dev);
    if (gpio_1_18)
        gpio_1_18->priv = &a_pio_1_18;

    gpio_1_20 = fno_create(&mod_devgpio, "gpio_1_20", dev);
    if (gpio_1_20)
        gpio_1_20->priv = &a_pio_1_20;
    
    gpio_1_21 = fno_create(&mod_devgpio, "gpio_1_21", dev);
    if (gpio_1_21)
        gpio_1_21->priv = &a_pio_1_21;
    
    gpio_1_23 = fno_create(&mod_devgpio, "gpio_1_23", dev);
    if (gpio_1_23)
        gpio_1_23->priv = &a_pio_1_23;

    klog(LOG_INFO, "GPIO Driver: KLOG enabled.\n");

    register_module(&mod_devgpio);
}

