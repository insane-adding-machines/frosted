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

#ifndef  _TN_PORT_CM4F_H_
#define  _TN_PORT_CM4F_H_

#if !defined TN_SUPPORT_FPU
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

#if defined (__ICCARM__)    // IAR ARM

#include <intrinsics.h>
#define align_attr_start
#define align_attr_end

#ifndef INLINE_FORCED
#define INLINE_FORCED   _Pragma("inline=forced")
#endif

#elif defined (__GNUC__)    //-- GNU Compiler

#define align_attr_start
#define align_attr_end     __attribute__((aligned(0x8)))

#ifndef INLINE_FORCED
#define INLINE_FORCED   static inline __attribute__ ((always_inline))
#endif

INLINE_FORCED
void
__disable_interrupt(void)
{
    __asm volatile("cpsid I;");
}

INLINE_FORCED
void
__enable_interrupt(void)
{
    __asm volatile("cpsie I;");
}

INLINE_FORCED
unsigned long
__get_PRIMASK(void)
{
    unsigned long ret;

    __asm volatile
    (
        "mrs    %0, PRIMASK;" : "=r"(ret)
    );
    return ret;
}

INLINE_FORCED
void
__set_PRIMASK(unsigned long mask)
{
    __asm volatile
    (
        "msr    PRIMASK, %0;" : : "r"(mask)
    );
}

INLINE_FORCED
void
__ISB(void)
{
    __asm volatile  ("isb;");
}

INLINE_FORCED
void
__DSB(void)
{
    __asm volatile  ("dsb;");
}

INLINE_FORCED
unsigned long
__CLZ(unsigned long val)
{
    unsigned long ret;

    __asm volatile
    (
        "clz    %0, %1;" : "=r"(ret) : "r"(val)
    );
    return ret;
}
#elif defined ( __CC_ARM )   //-- Keil

#ifndef INLINE_FORCED
#define INLINE_FORCED   __forceinline
#endif

#define align_attr_start  __align(8)
#define align_attr_end

INLINE_FORCED
void
__disable_interrupt(void)
{
    __asm
    {
        cpsid I
    }
}

INLINE_FORCED
void
__enable_interrupt(void)
{
    __asm
    {
        cpsie I
    }
}

INLINE_FORCED
unsigned long
__get_PRIMASK(void)
{
    unsigned long ret;

    __asm
    {
      mrs   ret, PRIMASK
    }
    return ret;
}

INLINE_FORCED
void
__set_PRIMASK(unsigned long mask)
{
    __asm
    {
      msr  PRIMASK, mask
    }
}

INLINE_FORCED
void
__ISB(void)
{
    __asm
    {
     isb
    }
}

INLINE_FORCED
void
__DSB(void)
{
    __asm
    {
         dsb
    }
}

INLINE_FORCED
unsigned long
__CLZ(unsigned long val)
{
    unsigned long ret;

    __asm
    {
         clz ret, val
    }
    return ret;
}

#else

  #error "Unknown compiler"

#endif

#if (TN_SUPPORT_FPU == 1)
#define  TN_MIN_STACK_SIZE         75      //--  +34 dwords for FPU context
#define  TN_TIMER_STACK_SIZE       (TN_MIN_STACK_SIZE+1)
#define  TN_IDLE_STACK_SIZE        (TN_MIN_STACK_SIZE+1)
#else
#define  TN_TIMER_STACK_SIZE       64
#define  TN_IDLE_STACK_SIZE        48
#define  TN_MIN_STACK_SIZE         40      //--  +20 for exit func when ver GCC > 4
#endif

#define  TN_BITS_IN_INT            32

#define  TN_ALIG                   sizeof(void*)

#define  MAKE_ALIG(a)  ((sizeof(a) + (TN_ALIG-1)) & (~(TN_ALIG-1)))

#if (TN_SUPPORT_FPU == 1)
#define  TN_PORT_STACK_EXPAND_AT_EXIT  17
#else
#define  TN_PORT_STACK_EXPAND_AT_EXIT  16
#endif

  //----------------------------------------------------

#define  TN_NUM_PRIORITY        TN_BITS_IN_INT  //-- 0..31  Priority 0 always is used by timers task

#define  TN_WAIT_INFINITE       0xFFFFFFFF
#define  TN_FILL_STACK_VAL      0xFFFFFFFF
#define  TN_INVALID_VAL         0xFFFFFFFF

#if (TN_SUPPORT_FPU == 2)
#pragma pack(push, 4)
typedef struct _TN_FPU
{
    unsigned long   f32[32];
    unsigned long   scr;

} TN_FPU;
#pragma pack(pop)
#endif

    //-- Assembler functions prototypes

#ifdef __cplusplus
extern "C"  {
#endif

#define tn_switch_context()
  void  tn_switch_context_exit(void);

INLINE_FORCED
void
tn_switch_context_request(void)
{
    volatile unsigned long *cortex_icsr;

    cortex_icsr = (volatile unsigned long *) 0xE000ED04;
    *cortex_icsr = (1<<28);
}

INLINE_FORCED
unsigned long
tn_cpu_save_sr(void)
{
    unsigned long  rc = __get_PRIMASK();
    __disable_interrupt();
    return rc;
}
#define tn_cpu_restore_sr(sr)  __set_PRIMASK(sr)

INLINE_FORCED
int tn_inside_int_cortex(void)
{
    volatile unsigned long *cortex_icsr;

    cortex_icsr = (volatile unsigned long *) 0xE000ED04;
    return (*cortex_icsr & 0x1FF) ? 1 : 0;
}

#define tn_int_enter()
#define tn_int_exit()

  void tn_start_exe(void);
  int  tn_chk_irq_disabled(void);

#ifdef USE_ASM_FFS
INLINE_FORCED
unsigned long
ffs_asm(unsigned long value)
{
    value = value & (0 - value);
    value = 32 - __CLZ(value);
    return value;
}
#endif

  void tn_arm_disable_interrupts(void);
  void tn_arm_enable_interrupts(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

    //-- Interrupt processing   - processor specific

#define  TN_INTSAVE_DATA_INT     int tn_save_status_reg = 0;
#define  TN_INTSAVE_DATA         int tn_save_status_reg = 0;
#define  tn_disable_interrupt()  tn_save_status_reg = tn_cpu_save_sr()
#define  tn_enable_interrupt()   tn_cpu_restore_sr(tn_save_status_reg)

#define  tn_idisable_interrupt() tn_save_status_reg = tn_cpu_save_sr()
#define  tn_ienable_interrupt()  tn_cpu_restore_sr(tn_save_status_reg)

#define  TN_CHECK_INT_CONTEXT               if(!tn_inside_int_cortex()) return TERR_WCONTEXT;
#define  TN_CHECK_INT_CONTEXT_NORETVAL      if(!tn_inside_int_cortex()) return;
#define  TN_CHECK_NON_INT_CONTEXT           if(tn_inside_int_cortex()) return TERR_WCONTEXT;
#define  TN_CHECK_NON_INT_CONTEXT_NORETVAL  if(tn_inside_int_cortex()) return ;

#endif // _TN_PORT_CM4F_H_

