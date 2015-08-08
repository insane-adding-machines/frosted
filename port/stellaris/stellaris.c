//*****************************************************************************
//
// startup.c - Startup code for use with GNU tools.
//
//*****************************************************************************

//*****************************************************************************
//
// Forward declaration of the default fault handlers.
//
//*****************************************************************************
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

//*****************************************************************************
//
// The entry point for the application.
//
//*****************************************************************************
extern int main(void);
extern unsigned int __StackTop; /* provided by linker script */
 
 
//*****************************************************************************
//
// The following are constructs created by the linker, indicating where the
// the "data" and "bss" segments reside in memory.  The initializers for the
// "data" segment resides immediately following the "text" segment.
//
//*****************************************************************************
extern unsigned long _etext;
extern unsigned long _data;
extern unsigned long _edata;
extern unsigned long _bss;
extern unsigned long _ebss;
 
//*****************************************************************************
//
// This is the code that gets called when the processor first starts execution
// following a reset event.  Only the absolutely necessary set is performed,
// after which the application supplied entry() routine is called. 
//
//*****************************************************************************
void Reset_Handler(void)
{
    unsigned long *pulSrc, *pulDest;
    unsigned char * bssDest;
 
    //
    // Copy the data segment initializers from flash to SRAM.
    //
    pulSrc = &_etext;
    pulDest = &_data; 

    while(pulDest < &_edata)
    {
        *pulDest++ = *pulSrc++;
    }

    //
    // Zero-init the BSS section
    //
    bssDest = (unsigned char *)&_bss;

    while(bssDest < (unsigned char *)&_ebss)
    {
        *bssDest++ = 0u;
    }
 
    //
    // Call the application's entry point.
    //
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
 
//*****************************************************************************
//
// This is the code that gets called when the processor receives an unexpected
// interrupt.  This simply enters an infinite loop, preserving the system state
// for examination by a debugger.
//
//*****************************************************************************
void __attribute__((weak)) IntDefaultHandler(void)
{
    //
    // Go into an infinite loop.
    //
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

//*****************************************************************************
//
// The vector table.  Note that the proper constructs must be placed on this to
// ensure that it ends up at physical address 0x0000.0000.
//
//*****************************************************************************
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
};
