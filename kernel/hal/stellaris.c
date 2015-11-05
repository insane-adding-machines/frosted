/*  
 *      This file is part of frosted.
 *
 *      frosted is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, either version 3 of the License, or
 *      (at your option) any later version.
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
#include "tools.h"
#include "regs.h"

const uint32_t NVIC_PRIO_BITS = 3;

/* PLL/Systick Configured values */
const uint32_t SYS_CLOCK = 50000000;
#define RCC_SYSDIV_VAL        (3)
#define RCC_PWMDIV_VAL        (7)
#define RCC_XTAL_VAL          (14)


/* Register values */
#define RCC_DEFAULT  (0x078E3AD1)
#define RCC2_DEFAULT (0x07802810)

/* RCC1 bit pos */
#define RCC_SYSDIV    23
#define RCC_USESYSDIV 22
#define RCC_USEPWMDIV 20
#define RCC_PWMDIV    17
#define RCC_OFF       13
#define RCC_BYPASS    11
#define RCC_XTAL      6
#define RCC_IOSCDIS   1

/* RCC2 bit pos */
#define RCC2_USERRCC2  31
#define RCC2_SYSDIV    23
#define RCC2_OFF       13
#define RCC2_BYPASS    11

/* RIS bit pos */
#define RIS_PLLLRIS    6


const uint32_t UART0_BASE = 0x4000C000;
const uint32_t UART0_IRQn = 5;


int hal_board_init(void)
{
    uint32_t rcc = RCC_DEFAULT;
    uint32_t rcc2 = RCC2_DEFAULT;

    /* Initials: set default values */

    SET_REG(SYSREG_SC_RCC,  rcc);
    SET_REG(SYSREG_SC_RCC2, rcc2);
    noop();

    /* Stage 1: Reset Oscillators and select configured values */

    rcc = (RCC_SYSDIV_VAL << RCC_SYSDIV) | (RCC_PWMDIV_VAL << RCC_PWMDIV) | (RCC_XTAL_VAL << RCC_XTAL) |
            (1 << RCC_USEPWMDIV);

    rcc2 = (RCC_SYSDIV_VAL << RCC2_SYSDIV); 

    rcc  |= (1 << RCC_BYPASS) | (1 << RCC_OFF);
    rcc2 |= (1 << RCC2_BYPASS) | (1 << RCC2_OFF);

    SET_REG(SYSREG_SC_RCC,  rcc);
    SET_REG(SYSREG_SC_RCC2, rcc2);
    noop();

    /* Stage 2: Power up oscillators */
    rcc  &= ~(1 << RCC_OFF);
    rcc2 &= ~(1 << RCC2_OFF);
    SET_REG(SYSREG_SC_RCC,  rcc);
    SET_REG(SYSREG_SC_RCC2, rcc2);
    noop();

    /* Stage 3: Set USESYSDIV */
    rcc |= (1 << RCC_BYPASS) | (1 << RCC_USESYSDIV);
    SET_REG(SYSREG_SC_RCC,  rcc);
    noop();

    /* Stage 4: Wait for PLL raw interrupt */
    while ((GET_REG((SYSREG_SC_RIS)) & (1 << RIS_PLLLRIS)) == 0)
        noop();

    /* Stage 5: Disable bypass */
    rcc  &= ~(1 << RCC_BYPASS);
    rcc2 &= ~(1 << RCC2_BYPASS);
    SET_REG(SYSREG_SC_RCC,  rcc);
    SET_REG(SYSREG_SC_RCC2, rcc2);
    noop();

}


void Reset_Handler(void);
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

extern int main(void);
extern unsigned int __StackTop; /* provided by linker script */
 
extern unsigned long _etext;
extern unsigned long _data;
extern unsigned long _edata;
extern unsigned long _bss;
extern unsigned long _ebss;
 
void Reset_Handler(void)
{
    unsigned long *pulSrc, *pulDest;
    unsigned char * bssDest;
 
    /*
     * Copy the data segment initializers from flash to SRAM.
     */
    pulSrc = &_etext;
    pulDest = &_data; 

    while(pulDest < &_edata)
    {
        *pulDest++ = *pulSrc++;
    }

    /*
     * Zero-init the BSS section
     */
    bssDest = (unsigned char *)&_bss;

    while(bssDest < (unsigned char *)&_ebss)
    {
        *bssDest++ = 0u;
    }
 
    /*
     * Call the kernel  entry point.
     */
    main();
}
 
void __attribute__((weak)) NMI_Handler(void)
{
    while(1);
}

void __attribute__((weak)) HardFault_Handler(void)
{
    while(1);
}

void __attribute__((weak)) MemManage_Handler(void)
{
    while(1);
}

void __attribute__((weak)) BusFault_Handler(void)
{
    while(1);
}

void __attribute__((weak)) UsageFault_Handler(void)
{
    while(1);
}

void __attribute__((weak)) SVC_Handler(void)
{
    while(1);
}

void __attribute__((weak)) DebugMon_Handler(void)
{
    while(1);
}

void __attribute__((weak)) PendSV_Handler(void)
{
    while(1);
}

void __attribute__((weak)) SysTick_Handler(void)
{
    while(1);
}
 
void __attribute__((weak)) IntDefaultHandler(void)
{
    while(1) {
        ;
    }
}

void __attribute__((weak)) WDT_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) TIMER0_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) TIMER1_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) TIMER2_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) UART0_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) UART1_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) UART2_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) PWMFault_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) PWM0_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) PWM1_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) PWM2_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) I2C0_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) I2C1_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) I2C2_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) SSI0_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) ADC0_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) ADC1_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) ADC2_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) ADC3_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) BOD_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) USB_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) ENET_IRQHandler(void)
{
    while(1);
}

__attribute__ ((section(".isr_vector")))
void (* const g_pfnVectors[])(void) =
{
    (void *)&__StackTop,                    // The initial stack pointer
    Reset_Handler,                          // The reset handler
    NMI_Handler,                            // The NMI handler
    HardFault_Handler,                      // The hard fault handler
    MemManage_Handler,                      // The MPU fault handler
    BusFault_Handler,                       // The bus fault handler
    UsageFault_Handler,                     // The usage fault handler
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    SVC_Handler,                            // SVCall handler
    DebugMon_Handler,                       // Debug monitor handler
    0,                                      // Reserved
    PendSV_Handler,                         // The PendSV handler
    SysTick_Handler,                        // The SysTick handler
    IntDefaultHandler,                      // GPIO Port A
    IntDefaultHandler,                      // GPIO Port B
    IntDefaultHandler,                      // GPIO Port C
    IntDefaultHandler,                      // GPIO Port D
    IntDefaultHandler,                      // GPIO Port E
    UART0_IRQHandler,                       // UART0
    UART1_IRQHandler,                       // UART1
    SSI0_IRQHandler,                        // SSI0
    I2C0_IRQHandler,                        // I2C0
    PWMFault_IRQHandler,                    // PWM Fault
    PWM0_IRQHandler,                        // PWM Generator 0
    PWM1_IRQHandler,                        // PWM Generator 1
    PWM2_IRQHandler,                        // PWM Generator 2
    IntDefaultHandler,                      // QEI0
    ADC0_IRQHandler,                        // ADC Sequence 0
    ADC1_IRQHandler,                        // ADC Sequence 1
    ADC2_IRQHandler,                        // ADC Sequence 2
    ADC3_IRQHandler,                        // ADC Sequence 3
    WDT_IRQHandler,                         // Watchdog timer 0
    TIMER0_IRQHandler,                      // Timer 0A
    TIMER0_IRQHandler,                      // Timer 0B
    TIMER1_IRQHandler,                      // Timer 1A
    TIMER1_IRQHandler,                      // Timer 1B
    TIMER2_IRQHandler,                      // Timer 2A
    TIMER2_IRQHandler,                      // Timer 2B
    IntDefaultHandler,                      // Analog Comparator 0
    IntDefaultHandler,                      // Analog Comparator 1
    IntDefaultHandler,                      // Reserved
    IntDefaultHandler,                      // System Control
    IntDefaultHandler,                      // Flash Memory Control
    IntDefaultHandler,                      // GPIO Port F
    IntDefaultHandler,                      // GPIO Port G
    IntDefaultHandler,                      // Reserved
    UART2_IRQHandler,                       // UART2
    IntDefaultHandler,                      // Reserved
    IntDefaultHandler,                      // Timer 3A
    IntDefaultHandler,                      // Timer 3B
    I2C1_IRQHandler,                        // I2C1
    IntDefaultHandler,                      // QEI1
    IntDefaultHandler,                      // Reserved
    IntDefaultHandler,                      // Reserved
    0,                                      // Reserved
    ENET_IRQHandler,                        // Ethernet
    IntDefaultHandler                       // Hibernate
};

