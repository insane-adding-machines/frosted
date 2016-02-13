/*  
 *      This file is part of frosted.
 *
 *      frosted is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License version 2, as 
 *      published by the free Software Foundation.
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
 *      Authors: Daniele Lacamera, Maxime Vincent
 *
 */  
#include "frosted.h"
#include <sys/vfs.h>
#include "libopencmsis/core_cm3.h"
#include "libopencm3/cm3/systick.h"
#include "bflt.h"
#include "null.h"

#ifdef CONFIG_PICOTCP
# include "pico_stack.h"
void socket_in_ini(void);
#else
# define pico_stack_init() do{}while(0)
# define socket_in_init()  do{}while(0)
#endif

#ifdef CONFIG_PICOTCP_LOOP
struct pico_device *pico_loop_create(void);
#else
# define pico_loop_create() NULL
#endif

#define IDLE() while(1){do{}while(0);}

/* The following needs to be defined by
 * the application code
 */
void (*init)(void *arg) = (void (*)(void*))(FLASH_ORIGIN + APPS_ORIGIN);

void hard_fault_handler(void)
{
    volatile uint32_t hfsr = SCB_HFSR;
    //volatile uint32_t bfsr = SCB_BFSR;
    volatile uint32_t afsr = SCB_AFSR;
    volatile uint32_t bfar = SCB_BFAR;
    //volatile uint32_t ufsr = SCB_UFSR;
    volatile uint32_t mmfar = SCB_MMFAR;
    while(1);
}

void mem_manage_handler(void)
{
#   define ARM_CFSR (*(volatile uint32_t *)(0xE000ED28))
#   define ARM_MMFAR (*(volatile uint32_t *)(0xE000ED34))
    volatile uint32_t address = 0xFFFFFFFF;
    volatile uint32_t instruction = 0xFFFFFFFF;
    uint32_t *top_stack;

    if ((ARM_CFSR & 0x80)!= 0) {
        address = ARM_MMFAR;
        asm volatile ("mrs %0, psp" : "=r" (top_stack));
        instruction = *(top_stack - 1);
    }

    if (task_segfault(address, instruction, MEMFAULT_ACCESS) < 0)
        while(1);
}

void bus_fault_handler(void)
{
    while(1);
}

void usage_fault_handler(void)
{
    while(1);
}

void machine_init(struct fnode * dev);

static void hw_init(void)
{
    machine_init(fno_search("/dev"));
    SysTick_Config(CONFIG_SYS_CLOCK / 1000);
}

int frosted_init(void)
{
    extern void * _k__syscall__;
    int xip_mounted;

    vfs_init();
    devnull_init(fno_search("/dev"));

    /* Set up system */

    /* ktimers must be enabled before systick */
    ktimer_init();

    hw_init();
    mpu_init();
            
    syscalls_init();

    memfs_init();
    xipfs_init();
    sysfs_init();

    vfs_mount(NULL, "/mem", "memfs", 0, NULL);
    xip_mounted = vfs_mount((char *)init, "/bin", "xipfs", 0, NULL);

    vfs_mount(NULL, "/sys", "sysfs", 0, NULL);

    kernel_task_init();

    /* kernel is now _cur_task, open filedesc for kprintf */
    kprintf_init();
    /* 
    kprintf("\r\n\n\nFrosted kernel version 16.01. (GCC version %s, built %s)\r\n", __VERSION__, __TIMESTAMP__);
    */

#ifdef UNIX    
    socket_un_init();
#endif

    frosted_scheduler_on();
    return xip_mounted;
}

static void ktimer_tcpip(uint32_t time, void *arg);

static void tasklet_tcpip(void *arg)
{
#ifdef CONFIG_PICOTCP
    irq_off();
    pico_stack_tick();
    ktimer_add(1, ktimer_tcpip, NULL);
    irq_on();
#endif
}

static void ktimer_tcpip(uint32_t time, void *arg)
{
    tasklet_add(tasklet_tcpip, NULL);
}


static const char init_path[] = "/bin/init";
static const char *const init_args[2] = { init_path, NULL };

void frosted_kernel(int xipfs_mounted)
{
    if (xipfs_mounted == 0)
    {
        struct fnode *fno = fno_search(init_path);
        void * memptr;
        size_t mem_size;
        size_t stack_size;
        uint32_t got_loc;
        if (!fno) {
            /* PANIC: Unable to find /bin/init */
            while(1 < 2);
        }

        if (fno->owner && fno->owner->ops.exe) {
            void *start = NULL;
            uint32_t pic;

            start = fno->owner->ops.exe(fno, (void *)init_args, &pic);
            task_create(start, (void *)init_args, 2, pic);
        }
    } else {
        /* Create "init" task */
        kprintf("Starting Init task\r\n");
        if (task_create(init, (void *)0, 2, 0) < 0)
            IDLE();
    }

    ktimer_add(1000, ktimer_tcpip, NULL);

    pico_stack_init();
    pico_loop_create();
    socket_in_init();
#ifdef CONFIG_USBETH
    usb_ethernet_init();
#endif

    while(1) {
        check_tasklets();
    }
}

/* OS entry point */
void main(void) 
{
    int xipfs_mounted;
    xipfs_mounted = frosted_init();
    frosted_kernel(xipfs_mounted); /* never returns */
}

