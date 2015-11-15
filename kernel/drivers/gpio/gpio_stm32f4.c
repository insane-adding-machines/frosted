#include "frosted.h"
#include <stdint.h>
#include "ioctl.h"



extern struct hal_iodev GPIOA;
extern struct hal_iodev GPIOD;
extern struct hal_iodev GPIOG;

static int gpio_subsys_initialized = 0;

extern int stm32_pio_mode(uint32_t port, uint32_t pin, uint32_t mode);
extern int stm32_pio_func(uint32_t port, uint32_t pin, uint32_t func);


static struct module mod_devgpio = {
};

static struct fnode *gpio_3_12 = NULL;
static struct fnode *gpio_3_13 = NULL;
static struct fnode *gpio_3_14 = NULL;
static struct fnode *gpio_3_15 = NULL;

struct __attribute__((packed)) gpio_reg {
    uint32_t mode;          /* Mode register,                    Address offset: 0x00      */
    uint32_t otype;         /* Output type register,             Address offset: 0x04      */
    uint32_t ospeed;        /* Output speed register,            Address offset: 0x08      */
    uint32_t pullupdown;    /* Pull-up/pull-down register,       Address offset: 0x0C      */
    uint32_t data_in;       /* Input data register,              Address offset: 0x10      */
    uint32_t data_out;      /* Output data register,             Address offset: 0x14      */
    uint16_t setr_low;      /* Bit set/reset low register,       Address offset: 0x18      */
    uint16_t setr_hi;       /* Bit set/reset high register,      Address offset: 0x1A      */
    uint32_t lock;          /* Configuration lock register,      Address offset: 0x1C      */
    uint32_t altf[2];       /* Alternate function registers,     Address offset: 0x20-0x24 */
};

struct gpio_addr {
    uint32_t port;
    uint32_t n;
};

static struct gpio_addr a_pio_3_12 = {3, 12};
static struct gpio_addr a_pio_3_13 = {3, 13};
static struct gpio_addr a_pio_3_14 = {3, 14};
static struct gpio_addr a_pio_3_15 = {3, 15};

#define GPIO0 ((struct gpio_reg *)0x40020000) 
#define GPIO1 ((struct gpio_reg *)0x40020400)
#define GPIO2 ((struct gpio_reg *)0x40020800)
#define GPIO3 ((struct gpio_reg *)0x40020C00)
#define GPIO4 ((struct gpio_reg *)0x40021000)
#define GPIO5 ((struct gpio_reg *)0x40021400)
#define GPIO6 ((struct gpio_reg *)0x40021800)
#define GPIO7 ((struct gpio_reg *)0x40021C00)
#define GPIO8 ((struct gpio_reg *)0x40022000) 

#define GPIOREG(x) ((struct gpio_reg *)(0x40020000 + x * 0x400))

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
        GPIOREG(a->port)->setr_hi = (1 << a->n);
        return 1;
    } else if (arg[0] == '0') {
        GPIOREG(a->port)->setr_low = (1 << a->n);
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
        GPIOREG(a->port)->mode &= ~(3 << (a->n * 2));
        GPIOREG(a->port)->mode |= (1 << (a->n * 2));
        GPIOREG(a->port)->otype &= ~(1 << (a->n));
        GPIOREG(a->port)->ospeed |= (3 << (a->n * 2));
        GPIOREG(a->port)->pullupdown &= ~(3 << (a->n * 2));
    }
    if (cmd == IOCTL_GPIO_DISABLE) {
    }
    if (cmd == IOCTL_GPIO_SET_INPUT) {
        GPIOREG(a->port)->mode &= ~(3 << (a->n * 2));
    }
    if (cmd == IOCTL_GPIO_SET_OUTPUT) {
        GPIOREG(a->port)->mode |= (1 << (a->n * 2));
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
        hal_irq_on(GPIOA.irqn);
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
    

    if (!gpio_subsys_initialized) {
        hal_iodev_on(&GPIOD);
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

