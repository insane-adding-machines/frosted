/*

  TNKernel real-time kernel

  Copyright © 2004, 2010 Yuri Tiomkin
  Copyright © 2008 Sergey Koshkin (Cortex - M3 port)
  Copyright © 2007-2013 Vyacheslav Ovsiyenko (Cortex - M4F port)

  All rights reserved.

  Permission to use, copy, modify, and distribute this software in source
  and binary forms and its documentation for any purpose and without fee
  is hereby granted, provided that the above copyright notice appear
  in all copies and that both that copyright notice and this permission
  notice appear in supporting documentation.

  THIS SOFTWARE IS PROVIDED BY THE YURI TIOMKIN AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL YURI TIOMKIN OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  SUCH DAMAGE.

*/

  /* ver 2.6 */

#include "../tn.h"

//----------------------------------------------------------------------------
//   FPU context owning task
//
#if (TN_SUPPORT_FPU == 2)
volatile struct _TN_TCB* tn_fpu_owner_task;
#endif

//----------------------------------------------------------------------------
//   Processor specific routine - here for Cortex-M3/M4F
//
//   sizeof(void*) = sizeof(int)
//----------------------------------------------------------------------------
unsigned int * tn_stack_init(void * task_func,
                             void * stack_start,
                             void * param)
{
   unsigned int * stk;

 //-- filling register's position in the stack - for debugging only

   stk  = (unsigned int *)stack_start;       //-- Load stack pointer

   *stk = 0x01000000L;                       //-- xPSR
   stk--;

   *stk = ((unsigned int)task_func) | 1;     //-- Entry Point (1 for THUMB mode)
   stk--;

   *stk = ((unsigned int)tn_task_exit) | 1;  //-- R14 (LR)    (1 for THUMB mode)
   stk--;

   *stk = 0x12121212L;                       //-- R12
   stk--;

   *stk = 0x03030303L;                       //-- R3
   stk--;

   *stk = 0x02020202L;                       //-- R2
   stk--;

   *stk = 0x01010101L;                       //-- R1
   stk--;

   *stk = (unsigned int)param;               //-- R0 - task's function argument
   stk--;

#if (TN_SUPPORT_FPU == 1)
    //
    // Return to Thread mode, exception return uses  non-floating-point state
    // from the PSP and execution uses PSP after return
    //
    // Actually this code is discarded, placed for illustrative purposes only
    //
   *stk = 0xFFFFFFFDul;
   stk--;
#endif

   *stk = 0x11111111L;                       //-- R11
   stk--;

   *stk = 0x10101010L;                       //-- R10
   stk--;

   *stk = 0x09090909L;                       //-- R9
   stk--;

   *stk = 0x08080808L;                       //-- R8
   stk--;

   *stk = 0x07070707L;                       //-- R7
   stk--;

   *stk = 0x06060606L;                       //-- R6
   stk--;

   *stk = 0x05050505L;                       //-- R5
   stk--;

   *stk = 0x04040404L;                       //-- R4

   return stk;
}

//_____________________________________________________________________________
//
void tn_cpu_int_enable(void)
{
    __enable_interrupt();
}

void tn_cpu_int_disable(void)
{
    __disable_interrupt();
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

