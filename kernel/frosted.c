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
#include "libopencmsis/core_cm3.h"
#include "libopencm3/cm3/systick.h"

#define IDLE() while(1){do{}while(0);}

/* The following needs to be defined by
 * the application code
 */
void (*init)(void *arg) = (void (*)(void*))(FLASH_ORIGIN + APPS_ORIGIN);
uint8_t * flt_file = (void (*)(void*))(FLASH_ORIGIN + APPS_ORIGIN);

static int (*_klog_write)(int, const void *, unsigned int) = NULL;
    

void klog_set_write(int (*wr)(int, const void *, unsigned int))
{
    _klog_write = wr;
}

int klog_write(int file, char *ptr, int len)
{
    if (_klog_write) {
        _klog_write(file, ptr, len);
    }
    return len;
}

void hard_fault_handler(void)
{
    /*
    volatile uint32_t hfsr = GET_REG(SYSREG_HFSR);
    volatile uint32_t bfsr = GET_REG(SYSREG_BFSR);
    volatile uint32_t bfar = GET_REG(SYSREG_BFAR);
    volatile uint32_t afsr = GET_REG(SYSREG_AFSR);
    */
    while(1);
}

void mem_manage_handler(void)
{
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

void frosted_init(void)
{
    extern void * _k__syscall__;
    volatile void * vector = &_k__syscall__;
    (void)vector;

    vfs_init();
    devnull_init(fno_search("/dev"));

    /* Set up system */

    /* ktimers must be enabled before systick */
    ktimer_init();

    hw_init();
            
    syscalls_init();

    memfs_init();
    xipfs_init();
    sysfs_init();

    vfs_mount(NULL, "/mem", "memfs", 0, NULL);
    vfs_mount(NULL, "/bin", "xipfs", 0, NULL);
    vfs_mount(NULL, "/sys", "sysfs", 0, NULL);

    kernel_task_init();

#ifdef UNIX    
    socket_un_init();
#endif

    frosted_scheduler_on();
}

static void tasklet_test(void *arg)
{
    klog(LOG_INFO, "Tasklet executed\n");
}

static void ktimer_test(uint32_t time, void *arg)
{
    tasklet_add(tasklet_test, NULL);
}


void frosted_kernel(void)
{
    /* Load init from BFLT */
    void * memptr;
    size_t mem_size;
    //bflt_load(flt_file, &memptr, &mem_size, &init);

    /* Create "init" task */
    klog(LOG_INFO, "Starting Init task\n");
    //if (task_create(init + 0x60 + 0x40, (void *)0, 2) < 0)
    //if (task_create(flt_file + 0x60 + 0x40, (void *)0, 2) < 0)
    if (task_create(init, (void *)0, 2) < 0) /* reset THUMB bit */
        IDLE();

    ktimer_add(1000, ktimer_test, NULL);

    while(1) {
        check_tasklets();
    }
}

/* OS entry point */
void main(void) 
{
    frosted_init();
    frosted_kernel(); /* never returns */
}

