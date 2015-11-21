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
 *      Authors: Daniele Lacamera, Maxime Vincent
 *
 */  
#include "frosted.h"
#include "libopencm3/cm3/systick.h"

#define IDLE() while(1){do{}while(0);}

/* The following needs to be defined by
 * the application code
 */
void (*init)(void *arg) = (void (*)(void*))(FLASH_ORIGIN + 0x20000); /* Expected init process text at 128 KB */

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

static void hw_init(void)
{
#   if defined STM32F4
        clock_setup_hse_3v3(&hse_8mhz_3v3[CLOCK_3V3_168MHZ]);
#   elif defined LPC17XX
#   elif defined LM3S
        rcc_qemu_init();
#   else
#       error "Unknown architecture. Please add proper HW initialization"    
#   endif
    systick_set_clocksource(1);
    systick_interrupt_enable();
    systick_counter_enable();

}

void frosted_init(void)
{
    extern void * _k__syscall__;
    volatile void * vector = &_k__syscall__;
    (void)vector;

    /* Set up system */
    hw_init();
            
    ktimer_init();
    syscalls_init();
    vfs_init();
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
    /* Create "init" task */
    klog(LOG_INFO, "Starting Init task\n");
    if (task_create(init, (void *)0, 2) < 0)
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

