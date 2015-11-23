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
#if defined STM32F4
#include <libopencm3/stm32/rcc.h>
#   elif defined LPC17XX
#include <libopencm3/lpc17xx/clock.h>
#endif

#define IDLE() while(1){do{}while(0);}

/* The following needs to be defined by
 * the application code
 */
void (*init)(void *arg) = (void (*)(void*))(FLASH_ORIGIN + APPS_ORIGIN);

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


static void hw_init(void)
{
    systick_clear();
#   if defined STM32F4
#       if CONFIG_SYS_CLOCK == 48000000
        rcc_clock_setup_hse_3v3(&hse_8mhz_3v3[CLOCK_3V3_48MHZ]);
#       elif CONFIG_SYS_CLOCK == 84000000
        rcc_clock_setup_hse_3v3(&hse_8mhz_3v3[CLOCK_3V3_84MHZ]);
#       elif CONFIG_SYS_CLOCK == 120000000
        rcc_clock_setup_hse_3v3(&hse_8mhz_3v3[CLOCK_3V3_120MHZ]);
#       elif CONFIG_SYS_CLOCK == 168000000
        rcc_clock_setup_hse_3v3(&hse_8mhz_3v3[CLOCK_3V3_168MHZ]);
#       else
#error No valid clock speed selected
#endif
#   elif defined LPC17XX
#if CONFIG_SYS_CLOCK == 100000000
    /* Enable the external clock */
    CLK_SCS |= 0x20;                
    while((CLK_SCS & 0x40) == 0);
    /* Select external oscilator */
    CLK_CLKSRCSEL = 0x01;   
    /*N = 2 & M = 25*/
    CLK_PLL0CFG = (2<<16) |25;
    /*Feed*/
    CLK_PLL0FEED=0xAA; 
    CLK_PLL0FEED=0x55;
    /*PLL0 Enable */
    CLK_PLL0CON = 1;
    /*Feed*/
    CLK_PLL0FEED=0xAA; 
    CLK_PLL0FEED=0x55;
    /* Divide by 3 */
    CLK_CCLKCFG = 2;
    /* wait until locked */
    while (!(CLK_PLL0STAT & (1<<26)));
    /* see flash accelerator - TBD*/
    /*_FLASHCFG = (_FLASHCFG & 0xFFF) | (4<<12);*/
    /* PLL0 connect */
    CLK_PLL0CON |= 1<<1;
    /*Feed*/
    CLK_PLL0FEED=0xAA; 
    CLK_PLL0FEED=0x55;
    /* PLL0 operational */
#elif CONFIG_SYS_CLOCK == 120000000
#error TBD
#else
#error No valid clock speed selected
#endif
#   elif defined LM3S
        rcc_qemu_init();
#   else
#       error "Unknown architecture. Please add proper HW initialization"    
#   endif

    systick_set_reload(CONFIG_SYS_CLOCK / 1000);
    systick_set_clocksource(STK_CSR_CLKSOURCE);
    systick_counter_enable();
    systick_interrupt_enable();
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

