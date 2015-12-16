/**
 * HardFault_HandlerAsm:
 * Alternative Hard Fault handler to help debug the reason for a fault.
 * To use, edit the vector table to reference this function in the HardFault vector
 * This code is suitable for Cortex-M3 and Cortex-M0 cores
 */

void HardFault_HandlerC(unsigned long *hardfault_args);

// Use the 'naked' attribute so that C stacking is not used.
__attribute__((naked))
void HardFault_Handler(void)
{
    /*
     * Get the appropriate stack pointer, depending on our mode,
     * and use it as the parameter to the C handler. This function
     * will never return
     */
    __asm("TST LR, #4           \n"
          "ITE EQ               \n"
          "MRSEQ R0, MSP        \n"
          "MRSNE R0, PSP        \n"
          "B HardFault_HandlerC \n"
           );
}

// Use the 'naked' attribute so that C stacking is not used.
__attribute__((naked))
void UsageFault_Handler(void)
{
    /*
     * Get the appropriate stack pointer, depending on our mode,
     * and use it as the parameter to the C handler. This function
     * will never return
     */
    __asm("TST LR, #4           \n"
          "ITE EQ               \n"
          "MRSEQ R0, MSP        \n"
          "MRSNE R0, PSP        \n"
          "B HardFault_HandlerC \n"
           );
}

/**
 * HardFaultHandler_C:
 * This is called from the HardFault_HandlerAsm with a pointer the Fault stack
 * as the parameter. We can then read the values from the stack and place them
 * into local variables for ease of reading.
 * We then read the various Fault Status and Address Registers to help decode
 * cause of the fault.
 * The function ends with a BKPT instruction to force control back into the debugger
 */
volatile unsigned long stacked_r0 ;
volatile unsigned long stacked_r1 ;
volatile unsigned long stacked_r2 ;
volatile unsigned long stacked_r3 ;
volatile unsigned long stacked_r12 ;
volatile unsigned long stacked_lr ;
volatile unsigned long stacked_pc ;
volatile unsigned long stacked_psr ;
volatile unsigned long _CFSR ;
volatile unsigned long _HFSR ;
volatile unsigned long _DFSR ;
volatile unsigned long _AFSR ;
volatile unsigned long _BFAR ;
volatile unsigned long _MMAR ;

void HardFault_HandlerC(unsigned long *hardfault_args){
    stacked_r0 = ((unsigned long)hardfault_args[0]) ;
    stacked_r1 = ((unsigned long)hardfault_args[1]) ;
    stacked_r2 = ((unsigned long)hardfault_args[2]) ;
    stacked_r3 = ((unsigned long)hardfault_args[3]) ;
    stacked_r12 = ((unsigned long)hardfault_args[4]) ;
    stacked_lr = ((unsigned long)hardfault_args[5]) ;
    stacked_pc = ((unsigned long)hardfault_args[6]) ;
    stacked_psr = ((unsigned long)hardfault_args[7]) ;

        // Configurable Fault Status Register
        // Consists of MMSR, BFSR and UFSR
    _CFSR = (*((volatile unsigned long *)(0xE000ED28))) ;   
                                            
    // Hard Fault Status Register
    _HFSR = (*((volatile unsigned long *)(0xE000ED2C))) ;

    // Debug Fault Status Register
    _DFSR = (*((volatile unsigned long *)(0xE000ED30))) ;

    // Auxiliary Fault Status Register
    _AFSR = (*((volatile unsigned long *)(0xE000ED3C))) ;

    // Read the Fault Address Registers. These may not contain valid values.
    // Check BFARVALID/MMARVALID to see if they are valid values
    // MemManage Fault Address Register
    _MMAR = (*((volatile unsigned long *)(0xE000ED34))) ;
    // Bus Fault Address Register
    _BFAR = (*((volatile unsigned long *)(0xE000ED38))) ;

    __asm("BKPT #0") ; // Break into the debugger

}

