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
#include "tools.h"
#include "regs.h"

const uint32_t NVIC_PRIO_BITS = 3;

/* PLL/Systick Configured values */
const uint32_t SYS_CLOCK =  96000000;
#define PLL_FCO (16u)       /* Value for 384 MHz as main PLL clock */
#define CPU_CLOCK_DIV (3u)  /* 384 / 3 = 96MHz */

const uint32_t UART0_BASE = 0x4000C000;
const uint32_t UART0_IRQn = 5;

/* PLL bit pos */
#define PLL0_FREQ     (16)  /* PLL0 main freq pos */
#define PLL0_STS_ON   (24)  /* PLL0 enable  pos */
#define PLL0_STS_CONN (25)  /* PLL0 connect pos */
#define PLL0_STS_LOCK (26)  /* PLL0 connect pos */

#define PLL1_FREQ     (5)   /* PLL1 main freq pos */
#define PLL1_STS_ON   (8)   /* PLL1 enable  pos */
#define PLL1_STS_CONN (9)   /* PLL1 connect pos */
#define PLL1_STS_LOCK (10)  /* PLL1 connect pos */

/* SCS bit pos */
#define SCS_OSC_RANGE (4)
#define SCS_OSC_ON    (5)
#define SCS_OSC_STAT  (6)

/* Clock Source value */
#define CLOCK_SOURCE_INT  0
#define CLOCK_SOURCE_MAIN 1
#define CLOCK_SOURCE_RTC  2

/* PLL Kickstart sequence */
#define PLL_KICK0 (0xAAUL)
#define PLL_KICK1 (0x55UL)


int hal_board_init(void)
{
    volatile uint32_t scs;
    volatile uint32_t pll0ctrl;
    /* Check if PLL0 is already connected, if so, disconnect */
    if (GET_REG(SYSREG_SC_PLL0_STAT) & (1<<PLL0_STS_CONN)) {
        pll0ctrl = GET_REG(SYSREG_SC_PLL0_CTRL);
        pll0ctrl &= ~(1 << PLL0_STS_CONN);
        SET_REG(SYSREG_SC_PLL0_CTRL, pll0ctrl);
    }
    
    /* Check if PLL0 is already ON, if so, disable */
    if (GET_REG(SYSREG_SC_PLL0_STAT) & (1<<PLL0_STS_ON)) {
        pll0ctrl = GET_REG(SYSREG_SC_PLL0_CTRL);
        pll0ctrl &= ~(1 << PLL0_STS_ON);
        SET_REG(SYSREG_SC_PLL0_CTRL, pll0ctrl);
    }

    /* Check if oscillator is enabled, otherwise enable */
    do {
        scs = GET_REG(SYSREG_SC_SCS);
        scs |= (1 << SCS_OSC_ON);
        SET_REG(SYSREG_SC_SCS, scs);
        noop();
    } while ((scs & (1 << SCS_OSC_STAT)) == 0);

    /* Reset CPU Clock divider */
    SET_REG(SYSREG_SC_CCLKSEL, 0u);

    /* Select Main Oscillator as primary clock source */
    SET_REG(SYSREG_SC_CLKSRCSEL, CLOCK_SOURCE_MAIN);

    /* Set oscillator frequency to 384 MHz */
    SET_REG(SYSREG_SC_PLL0_CONF, (PLL_FCO - 1u));

    /* Enable oscillator */
    SET_REG(SYSREG_SC_PLL0_CTRL, 1);

    /* Enable oscillator */
    pll0ctrl = GET_REG(SYSREG_SC_PLL0_CTRL);
    pll0ctrl |= (1 << PLL0_STS_ON);
    SET_REG(SYSREG_SC_PLL0_CTRL, pll0ctrl);

    /* Kickstart oscillator */
    SET_REG(SYSREG_SC_PLL0_FEED, PLL_KICK0);
    SET_REG(SYSREG_SC_PLL0_FEED, PLL_KICK1);

    /* Set correct divider */
    SET_REG(SYSREG_SC_CCLKSEL, CPU_CLOCK_DIV);

    /* Wait for lock */
    while ((GET_REG(SYSREG_SC_PLL0_STAT) & PLL0_STS_LOCK) == 0)
        noop();

    /* Connect oscillator */ 
    pll0ctrl = GET_REG(SYSREG_SC_PLL0_CTRL);
    pll0ctrl |= (1 << PLL0_STS_CONN);
    SET_REG(SYSREG_SC_PLL0_CTRL, pll0ctrl);

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
    volatile uint32_t hf = GET_REG(SYSREG_HFSR);
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

/** Default (weak) handler for external IRQs **/
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

void __attribute__((weak)) TIMER3_IRQHandler(void)
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

void __attribute__((weak)) UART3_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) PWM1_IRQHandler(void)
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

void __attribute__((weak)) SPI_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) SSP0_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) SSP1_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) PLL0_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) RTC_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) EINT0_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) EINT1_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) EINT2_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) EINT3_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) ADC_IRQHandler(void)
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

void __attribute__((weak)) CAN_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) DMA_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) I2S_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) ENET_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) RIT_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) MCPWM_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) QEI_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) PLL1_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) USBActivity_IRQHandler(void)
{
    while(1);
}

void __attribute__((weak)) CANActivity_IRQHandler(void)
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
    
    /* Board specific external IRQ handlers */ 
    WDT_IRQHandler,                 
    TIMER0_IRQHandler,              
    TIMER1_IRQHandler,              
    TIMER2_IRQHandler,              
    TIMER3_IRQHandler,              
    UART0_IRQHandler,               
    UART1_IRQHandler,               
    UART2_IRQHandler,               
    UART3_IRQHandler,               
    PWM1_IRQHandler,                
    I2C0_IRQHandler,                
    I2C1_IRQHandler,                
    I2C2_IRQHandler,                
    SPI_IRQHandler,                 
    SSP0_IRQHandler,                
    SSP1_IRQHandler,                
    PLL0_IRQHandler,                
    RTC_IRQHandler,                 
    EINT0_IRQHandler,               
    EINT1_IRQHandler,               
    EINT2_IRQHandler,               
    EINT3_IRQHandler,               
    ADC_IRQHandler,                 
    BOD_IRQHandler,                 
    USB_IRQHandler,                 
    CAN_IRQHandler,                 
    DMA_IRQHandler,                 
    I2S_IRQHandler,                 
    ENET_IRQHandler,                
    RIT_IRQHandler,                 
    MCPWM_IRQHandler,               
    QEI_IRQHandler,                 
    PLL1_IRQHandler,                
    USBActivity_IRQHandler,         
    CANActivity_IRQHandler,         
};

