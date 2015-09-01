//
//  Project:         TNKernel RTOS port for ARM(c) Cortex-M4F(c)
//
//  File:            tn_port_asm.s
//  Author:          Copyright (c) 2004-2009 Yuri Tiomkin
//                   Copyright (c) 2008 Sergey Koshkin
//                   Copyright (c) 2007-2013 Vyacheslav Ovsiyenko
//
//  Compiled Using:  CodeSourcery GCC 4.7.2 and IAR 6.50.2
//
//_____________________________________________________________________________
//
//  TNKernel real-time kernel v2.6
//
//  Copyright (c) 2004, 2010 Yuri Tiomkin
//  All rights reserved.
//
//  Permission to use, copy, modify, and distribute this software in source
//  and binary forms and its documentation for any purpose and without fee
//  is hereby granted, provided that the above copyright notice appear
//  in all copies and that both that copyright notice and this permission
//  notice appear in supporting documentation.
//
//  THIS SOFTWARE IS PROVIDED BY THE YURI TIOMKIN AND CONTRIBUTORS "AS IS" AND
//  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL YURI TIOMKIN OR CONTRIBUTORS BE LIABLE
//  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
//  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
//  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
//  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
//  SUCH DAMAGE.
//_____________________________________________________________________________
//
#if !defined ( TN_SUPPORT_FPU )
#error "TN_SUPPORT_FPU must be defined (0, 1, 2 values are allowed)"
#endif
//
// TN_SUPPORT_FPU = 0   - no FPU support, this port operates in the same way as for Cortex-M3
// TN_SUPPORT_FPU = 1   - FPU support based on the hardware lazy context saving
// TN_SUPPORT_FPU = 2   - FPU support based on the context switching on the demand (NOCP exception)
//
#if (TN_SUPPORT_FPU != 0) && (TN_SUPPORT_FPU != 1) && (TN_SUPPORT_FPU != 2)
#error "TN_SUPPORT_FPU is invalid (0, 1, 2 values are allowed)"
#endif

#if defined(__IASMARM__)

#pragma language=extended
#if (((__TID__ >> 8) & 0x7F) != 79)
#error This file should only be compiled by ARM-compiler
#endif
//
// IAR 6.50 Release Notes:
//
// Mixing C-style preprocessor macros and assembler-style macros should be avoided. If a C-style macro
// is used for concatenating identifiers in the label field, that label becomes absolute even if it is
// located in a relocatable segment. Use the ordinal macro arguments \1-\9 and \A-\Z instead of the
// symbolic argument names when you concatenate identifiers.
//
iarlabel        MACRO   name
name:
                ENDM

#if (__VER__ >= 6050002)
#define _label(a)       a:
#else
#define _label(a)       iarlabel        a
#endif
#define _title          name
#define _ltorg          ltorg
#define _data           data
#define _thumb          thumb
#define _global         public
#define _extern         extern
#define _thumb_func
#define _equ            =
#define _end            end
#define _sect(sname, txtiar, txtgcc, falign)    section sname:txtiar(falign)
#define _area(sname, param1, param2, param3)
#define _align          alignram
#define _alignram       alignram
#define _alignrom       alignrom
#define _byte           dc8
#define _word           dc32
#define _rept           REPT

//_____________________________________________________________________________
//
#elif defined(__GNUC__)

#define _label(a)       a:
#define _title          .title
#define _ltorg          .ltorg
#define _data
#define _thumb          .thumb
#define _global         .global
#define _extern         .extern
#define _thumb_func     .thumb_func
#define _equ            =
#define _end            .end
#define _sect(sname, txtiar, txtgcc, falign) .sect      sname, txtgcc
#define _area(sname, param1, param2, param3)
#define _align          .align
#define _alignram       .align
#define _alignrom       .align
#define _byte           .byte
#define _word           .word
#define _rept           .rept
#define ENDR            .endr

                .syntax unified

//_____________________________________________________________________________
//
#elif defined( __CC_ARM)

#define _label(a)       a
#define _title          TTL
#define _ltorg          LTORG
#define _data
#define _thumb          THUMB
#define _thumb_func
#define _sect(sname, txtiar, txtgcc, falign)
#define _area(sname, param1, param2, param3)    AREA  sname, param1, param2, param3
#define _align          ALIGN
#define _alignram       ALIGN
#define _alignrom       ALIGN
#define _global         EXPORT
#define _extern         EXTERN
#define _end            END
#define _word           DCD
#define _equ            EQU
#define _rept           WHILE
#define ENDR            WEND

                PRESERVE8
//_____________________________________________________________________________
//
#else
   #error "Unknown compiler"
#endif

//_____________________________________________________________________________
//
                _title  "tn_port_cm4f"

                _sect(.text, CODE, "ax", 2)
                _area(|.text|, CODE, READONLY, ALIGN=3)
                _thumb
//
// External variables, required by this module
//
                _extern tn_curr_run_task
                _extern tn_next_task_to_run
                _extern tn_system_state
//
// Public functions declared in this file
//
                _global  tn_switch_context_exit
                _global  tn_start_exe
                _global  tn_chk_irq_disabled
                _global  PendSV_Handler

#if (TN_SUPPORT_FPU == 2)
                _extern tn_fpu_owner_task
TN_TNFPU_OFFSET         _equ    4
#endif

//
// Constants needed
//
PR_12_15_ADDR           _equ    0xE000ED20      // System Handlers 12-15 Priority Register Address
PENDS_VPRIORITY         _equ    0x00FF0000      //  PRI_14 (PendSV) priority in the System Handlers 12-15
                                                // Priority Register Address, PendSV priority is minimal (0xFF)
SC_ICSR                 _equ    0xE000ED04      // Interrupt Control and State
SC_CPACR                _equ    0xE000ED88      //
SC_FPDSCR               _equ    0xE000EF3C      //
SC_CFSR                 _equ    0xE000ED28      // Configurable Fault Status
                                                //
bFPDSCR_AHP             _equ    (1<<26)         // Default alternative half-precision control
bFPDSCR_DN              _equ    (1<<25)         // Default NaN mode control
bFPDSCR_FZ              _equ    (1<<24)         // Default flush-to-zero mode control
bCFSR_NOCP              _equ    (1<<19)         // No coprocessor usage fault
bCPACR_FPUEN            _equ    (0xF<<20)       //
                                                //
bICSR_NMIPENDSET        _equ    (1<<31)         // NMI set-pending bit
bICSR_PENDSVSET         _equ    (1<<28)         // PendSV set-pending bit
bICSR_PENDSVCLR         _equ    (1<<27)         // PendSV clear-pending bit
bICSR_PENDSTSET         _equ    (1<<26)         // SysTick exception set-pending bit
bICSR_PENDSTCLR         _equ    (1<<25)         // SysTick exception clear-pending bit
bICSR_ISRPREEMPT        _equ    (1<<23)         //
bICSR_ISRPENDING        _equ    (1<<22)         // Interrupt pending flag, excluding NMI and Faults
bICSR_VECTPENDING_MASK  _equ    0x1FF           //
bICSR_VECTPENDING_SHIFT _equ    12              //
bICSR_RETTOBASE         _equ    (1<<11)         // There are preempted active exceptions
bICSR_VECTACTIVE_MASK   _equ    0x1FF           //
bICSR_VECTACTIVE_SHIFT  _equ    0               //
                                                //
//_____________________________________________________________________________
//
//  Interrups are not enabled (yet)
//
                _sect(.text, CODE, "ax", 2)
                _area(|.text|, CODE, READONLY, ALIGN=3)
                _thumb
                _thumb_func

_label(tn_start_exe)
                ldr     R1, =PR_12_15_ADDR              // Load the System 12-15 Priority Register
                ldr     R0, [R1]                        //
                orr     R0, R0, #PENDS_VPRIORITY        // set PRI_14 (PendSV) to 0xFF - minimal
                str     R0, [R1]                        //
                                                        //
                ldr     R1, =tn_system_state            // Indicate that system has started
                mov     R0, #1                          // 1 -> TN_SYS_STATE_RUNNING
                strb    R0, [R1]                        //
                                                        //
#if (TN_SUPPORT_FPU == 2)                               //
                ldr     R4, =tn_fpu_owner_task          //
                movs    R0, #0                          //
                str     R0, [R4]                        //
                ldr     R4, =SC_CPACR                   //
                str     R0, [R4]                        //
                isb                                     //
                dsb                                     //
#endif                                                  //
                ldr     R1, =tn_curr_run_task           //
                ldr     R2, [R1]                        //
                ldr     R0, [R2]                        // in r0 - new task SP
                                                        //
#if (TN_SUPPORT_FPU == 1)                               //
                adds    R0, R0, #4                      // advance for LR placeholder
#endif                                                  //
                ldmia   R0!, {R4-R11}                   //
                msr     PSP, R0                         //
                                                        //
                ldr     R1, =SC_ICSR                    // Trigger PendSV exception
                ldr     R0, =bICSR_PENDSVSET            // for context switching
                str     R0, [R1]                        //
                                                        //
                movs    R0, #0                          // Clear FPCA bit (if any) also
                msr     CONTROL, R0                     // Use the MSP before PendSv switch
                isb                                     //
                cpsie   IF                              // Enable core interrupts
                b       .                               // Never reach this

//_____________________________________________________________________________
//
                _thumb                                  //
                _thumb_func                             //
                                                        //
_label(tn_switch_context_exit)                          // Thanks to Sergey Koshkin
                                                        //
                                                        // At the interrupt request below,
                                                        // the PSP will be used to store
                                                        // manually saved 8 registers (R4-R11)
                ldr     R1, =tn_curr_run_task           //
                ldr     R2, [R1]                        //
                                                        //
#if (TN_SUPPORT_FPU == 2)                               //
                ldr     R4, =tn_fpu_owner_task          //
                ldr     R5, [R4]                        //
                cmp     R5, R2                          //
                bne     Not_Owner                       //
                movs    R0, #0                          //
                str     R0, [R4]                        //
                ldr     R4, =SC_CPACR                   //
                str     R0, [R4]                        //
                isb                                     //
                dsb                                     //
                                                        //
_label(Not_Owner)                                       //
#endif                                                  //
                ldr     R0, [R2]                        //
                                                        // Preset PSP stack
#if (TN_SUPPORT_FPU == 1)                               //
                adds    R0, R0, #9*4                    // 9 manually saved registers
#else                                                   //
                add     R0, R0, #8*4                    // 8 manually saved registers
#endif                                                  //
                msr     PSP, R0                         // Now we have only hardware-stored frame
                                                        // in the process stack
                                                        //
                                                        // Preset MSP stack
                mrs     R0,  MSP                        // The next interrupt will save the frame
                adds    R0,  R0, #8*4                   // in the handler stack, but the PendSV
                msr     MSP, R0                         // always pulls the frame from PSP stack
                                                        // on its return
                movs    R0, #0                          //
                msr     CONTROL, R0                     // FPCA bit is cleared as well
                isb                                     //
                                                        //
                ldr     R1, =SC_ICSR                    // Trigger PendSV exception
                ldr     R0, =bICSR_PENDSVSET            //
                str     R0,  [R1]                       //
                cpsie   I                               // Enable core interrupts
                b       .                               // Should never reach
                                                        // PendSV handler returns to
                _ltorg                                  // the address in the PSP stack frame
//_____________________________________________________________________________
//
                _sect(.psvhandler, CODE, "ax", 4)       //
                _area(PSVHANDLER, CODE, READONLY, ALIGN=4)
                _alignrom 4                             //
                _thumb                                  //
                _thumb_func                             //
                                                        //
_label(PendSV_Handler)                                  //
                cpsid   I                               // Disable core interrupts
                ldr     R3, =tn_curr_run_task           // in R3 - =tn_curr_run_task
                ldr     R1, [R3]                        // in R1 - tn_curr_run_task
                ldr     R2, =tn_next_task_to_run        //
                ldr     R2, [R2]                        // in R2 - tn_next_task_to_run
                cmp     R1, R2                          //
                beq     exit_context_switch             //
                                                        //
                mrs       R0, PSP                       //
#if (TN_SUPPORT_FPU == 1)                               //
                tst       LR, #0x10                     // if FPU context is active
                it        eq                            // we should save its registers
                vstmdbeq  R0!, {S16-S31}                // on the process stack
                stmdb     R0!, {R4-R11, LR}             //
#else
                stmdb   R0!, {R4-R11}                   //
#endif                                                  //
                str     R0, [R1]                        //  save own SP in TCB
                str     R2, [R3]                        // in R3 - =tn_curr_run_task
                ldr     R0, [R2]                        // in R0 - new task SP
                                                        //
#if (TN_SUPPORT_FPU == 1)                               //
                ldmia     R0!, {R4-R11, LR}             // restore the high GPRs
                tst       LR, #0x10                     // look whether there is the
                it        eq                            // active FPU context
                vldmiaeq  R0!, {S16-S31}                //
                                                        //
#elif (TN_SUPPORT_FPU == 2)                             //
                ldr     R4, =tn_fpu_owner_task          // in R4 - FPU context owner TCB
                ldr     R4, [R4]                        //
                subs    R1, R1, R4                      //
                itt     eq                              // we are leaving the owning task
                ldreq   R5, =SC_CPACR                   // owner == current, disable FPU
                streq   R1, [R5]                        //
                subs    R4, R4, R2                      // we are entering the owning task
                ittt    eq                              // owner == next_to_run, enable FPU
                ldreq   R5, =SC_CPACR                   //
                moveq   R3, #bCPACR_FPUEN               //
                streq   R3, [R5]                        //
                ldmia   R0!, {R4-R11}                   //
#else                                                   //
                                                        // no FPU support
                ldmia   R0!, {R4-R11}                   //
#endif                                                  //
                msr     PSP, R0                         //
                                                        //
_label(exit_context_switch)                             // bugfix: immediately completion of
                                                        // tn_switch_context_exit()
                orr     LR, LR, #0x04                   // Force to new process PSP
                cpsie   I                               // Enable core interrupts
                bx      LR                              //
                _ltorg                                  //

#if (TN_SUPPORT_FPU == 2)
//_____________________________________________________________________________
//
//  NOCP - no coprocessor exception handler. Actually it processes the UsageFault
//  exception, look whether it's been raised by floating point instruction.
//  If the FPU access caused the exception:
//      - enable access to FPU by setting CPACR bits
//      - save current FPU context in the owning task TCB
//      - load the FPU for current task from its TCB
//      - resume the execution
//
//  If the reason for exeption is not FPU access, the default handler called
//
                _sect(.nocphandler, CODE, "ax", 4)      //
                _area(NOCPHANDLER, CODE, READONLY, ALIGN=4)
                _alignrom 4                             //
                _global tn_port_nocp_handler            //
                _extern bsp_thunk_handler               //
                _thumb                                  //
                _thumb_func                             //
                                                        //
_label(tn_port_nocp_handler)                            //
                                                        //
                cpsid   I                               //
                tst     LR, #0x08                       // exception comes from
                beq     Default_UF                      // handler mode
                                                        //
                ldr     R0, =SC_CFSR                    //
                ldr     R1, [R0]                        //
                ands    R1, R1, #bCFSR_NOCP             //
                beq     Default_UF                      // reset the NOCP bit
                str     R1, [R0]                        //
                                                        //
                ldr     R0, =SC_CPACR                   //
                ldr     R1, =bCPACR_FPUEN               //
                str     R1, [R0]                        //
                dsb                                     //
                                                        //
                ldr     R0, =tn_fpu_owner_task          //
                ldr     R1, =tn_curr_run_task           //
                ldr     R2, [R0]                        //
                ldr     R3, [R1]                        //
                subs    R1, R3, R2                      //
                cbz     R1, Resume_Exec                 //
                                                        //
                ands    R2, R2, R2                      // save the FPU context
                itttt   ne                              //
                addne   R2, R2, #TN_TNFPU_OFFSET        //
                vstmiane R2!, {S0-S31}                  //
                vmrsne  R1, FPSCR                       //
                strne   R1, [R2]                        //
                                                        //
                str     R3, [R0]                        //
                adds    R3, R3, #TN_TNFPU_OFFSET        //
                vldmia  R3!, {S0-S31}                   //
                ldr     R1, [R3]                        //
                vmsr    FPSCR, R1                       //
                                                        //
_label(Resume_Exec)                                     //
                cpsie   I                               //
                bx      LR                              //
                                                        //
_label(Default_UF)                                      //
                cpsie   I                               //
                b       bsp_thunk_handler               //
                _ltorg                                  //
#endif                                                  //
                                                        //
//_____________________________________________________________________________
//
                _thumb                                  //
                _thumb_func                             //
                                                        //
_label(tn_chk_irq_disabled)                             //
                                                        //
                mrs     R0, PRIMASK                     //
                bx      LR                              //
//_____________________________________________________________________________
//
                _end
