#include "frosted.h"
#include <stdint.h>
#include "ioctl.h"

extern struct hal_iodev GPIO;
static int gpio_subsys_initialized = 0;


static struct module mod_devgpio = {
};

static struct fnode *gpio_1_18 = NULL;
static struct fnode *gpio_1_20 = NULL;
static struct fnode *gpio_1_21 = NULL;
static struct fnode *gpio_1_23 = NULL;

struct gpio_reg {
    uint32_t dir;
    uint32_t _res[3];
    uint32_t mask;
    uint32_t pin;
    uint32_t set;
    uint32_t clr;
};

struct gpio_addr {
    uint32_t port;
    uint32_t n;
};

static struct gpio_addr a_pio_1_18 = {1, 18};
static struct gpio_addr a_pio_1_20 = {1, 20};
static struct gpio_addr a_pio_1_21 = {1, 21};
static struct gpio_addr a_pio_1_23 = {1, 23};

struct gpio_reg *GPIOREG;

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
        GPIOREG[a->port].set |= (1 << a->n);
        return 1;
    } else if (arg[0] == '0') {
        GPIOREG[a->port].clr |= (1 << a->n);
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
        GPIOREG[a->port].mask &= ~(1 << a->n);
    }
    if (cmd == IOCTL_GPIO_DISABLE) {
        GPIOREG[a->port].mask |= (1 << a->n);
    }
    if (cmd == IOCTL_GPIO_SET_OUTPUT) {
        GPIOREG[a->port].dir |= (1 << a->n);
    }
    if (cmd == IOCTL_GPIO_SET_INPUT) {
        GPIOREG[a->port].dir &= ~(1 << a->n);
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
        hal_irq_on(GPIO.irqn);
        mutex_unlock(gpio_mutex);
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
    gpio_mutex = mutex_init();
    mod_devgpio.family = FAMILY_FILE;
    mod_devgpio.ops.open = devgpio_open;
    mod_devgpio.ops.read = devgpio_read; 
    mod_devgpio.ops.poll = devgpio_poll;
    mod_devgpio.ops.write = devgpio_write;
    mod_devgpio.ops.ioctl = devgpio_ioctl;
    
    /* Module initialization */
    GPIOREG = (struct gpio_reg *)GPIO.base;

    if (!gpio_subsys_initialized) {
        hal_iodev_on(&GPIO);
        gpio_subsys_initialized++;
    }


    gpio_1_18 = fno_create(&mod_devgpio, "gpio_1_18", dev);
    if (gpio_1_18)
        gpio_1_18->priv = &a_pio_1_18;

    GPIOREG[1].dir |= (1<<18);

    gpio_1_20 = fno_create(&mod_devgpio, "gpio_1_20", dev);
    if (gpio_1_20)
        gpio_1_20->priv = &a_pio_1_20;
    
    GPIOREG[1].dir |= (1<<18);

    gpio_1_21 = fno_create(&mod_devgpio, "gpio_1_21", dev);
    if (gpio_1_21)
        gpio_1_21->priv = &a_pio_1_21;
    
    GPIOREG[1].dir |= (1<<18);

    gpio_1_23 = fno_create(&mod_devgpio, "gpio_1_23", dev);
    if (gpio_1_23)
        gpio_1_23->priv = &a_pio_1_23;

    GPIOREG[1].dir |= (1<<18);


    /* Kernel printf associated to devgpio_write */
    klog_set_write(devgpio_write);

    klog(LOG_INFO, "GPIO Driver: KLOG enabled.\n");

    register_module(&mod_devgpio);
}

