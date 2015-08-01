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
    SysTick_Handler                         // The SysTick handler
};
 
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
