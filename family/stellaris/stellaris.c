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
