//*****************************************************************************
//
// system_lm3s.c - System initialization file for CMSIS application on a
//                 Luminary Micro Stellaris microcontroller.
//
// Copyright (c) 2009 Luminary Micro, Inc.  All rights reserved.
// Software License Agreement
// 
// Luminary Micro, Inc. (LMI) is supplying this software for use solely and
// exclusively on LMI's microcontroller products.
// 
// The software is owned by LMI and/or its suppliers, and is protected under
// applicable copyright laws.  All rights are reserved.  You may not combine
// this software with "viral" open-source software in order to form a larger
// program.  Any use in violation of the foregoing restrictions may subject
// the user to criminal sanctions under applicable laws, as well as to civil
// liability for the breach of the terms and conditions of this license.
// 
// THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
// OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
// LMI SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
// CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 48 of the Stellaris CMSIS Package.
//
//*****************************************************************************

#include <stdint.h>
#include "lm3s_cmsis.h"
#include "lm3s_config.h"

/*----------------------------------------------------------------------------
  Local functions
 *---------------------------------------------------------------------------*/
__INLINE static uint32_t getOscClk (uint32_t xtal, uint32_t oscSrc);

/*----------------------------------------------------------------------------
  Define clocks
 *----------------------------------------------------------------------------*/
#define XTALM       ( 6000000UL)            /* Main         oscillator freq */
#define XTALI       (12000000UL)            /* Internal     oscillator freq */
#define XTAL30K     (   30000UL)            /* Internal 30K oscillator freq */
#define XTAL32K     (   32768UL)            /* external 32K oscillator freq */

#define PLL_CLK    (400000000UL)
#define ADC_CLK     (PLL_CLK/25)
#define CAN_CLK     (PLL_CLK/50)



  /* Determine clock frequency according to clock register values */
  #if (RCC2_Val & (1UL<<31))                              /* is rcc2 used ? */
    #if (RCC2_Val & (1UL<<11))                           /* check BYPASS */
              #if   (((RCC2_Val>>4) & 0x07) == 0x0)                  
                #if   (((RCC_Val>>6) & 0x0F) == 0x0)                   
                      #define __CORE_CLK_PRE  1000000UL 
                #elif (((RCC_Val>>6) & 0x0F) == 0x1)                   
                      #define __CORE_CLK_PRE  1843200UL 
                #elif (((RCC_Val>>6) & 0x0F) == 0x2)                   
                      #define __CORE_CLK_PRE  2000000UL 
                #elif (((RCC_Val>>6) & 0x0F) == 0x3)                   
                      #define __CORE_CLK_PRE  2457600UL 
                #elif (((RCC_Val>>6) & 0x0F) == 0x4)                   
                      #define __CORE_CLK_PRE  3579545UL 
                #elif (((RCC_Val>>6) & 0x0F) == 0x5)                   
                      #define __CORE_CLK_PRE  3686400UL 
                #elif (((RCC_Val>>6) & 0x0F) == 0x6)                   
                      #define __CORE_CLK_PRE  4000000UL 
                #elif (((RCC_Val>>6) & 0x0F) == 0x7)                   
                      #define __CORE_CLK_PRE  4096000UL 
                #elif (((RCC_Val>>6) & 0x0F) == 0x8)                   
                      #define __CORE_CLK_PRE  4915200UL 
                #elif (((RCC_Val>>6) & 0x0F) == 0x9)                   
                      #define __CORE_CLK_PRE  5000000UL 
                #elif (((RCC_Val>>6) & 0x0F) == 0xA)                   
                      #define __CORE_CLK_PRE  5120000UL 
                #elif (((RCC_Val>>6) & 0x0F) == 0xB)                   
                      #define __CORE_CLK_PRE  6000000UL 
                #elif (((RCC_Val>>6) & 0x0F) == 0xC)                   
                      #define __CORE_CLK_PRE  6144000UL 
                #elif (((RCC_Val>>6) & 0x0F) == 0xD)                   
                      #define __CORE_CLK_PRE  7372800UL 
                #elif (((RCC_Val>>6) & 0x0F) == 0xE)                   
                      #define __CORE_CLK_PRE  8000000UL 
                #else                                   
                      #define __CORE_CLK_PRE  8192000UL 
                #endif                                  
              #elif (((RCC2_Val>>4) & 0x07) == 0x1)                  
                  #define __CORE_CLK_PRE  XTALI         
              #elif (((RCC2_Val>>4) & 0x07) == 0x2)                  
                  #define __CORE_CLK_PRE  (XTALI/4)     
              #else                                     
                  #define __CORE_CLK_PRE  XTAL30K       
              #endif
    #else
      #define __CORE_CLK_PRE   PLL_CLK
    #endif
    #if (RCC_Val & (1UL<<22))                            /* check USESYSDIV */
      #if (RCC2_Val & (1UL<<11))
        #define __CORE_CLK  (__CORE_CLK_PRE / (((RCC2_Val>>23) & (0x3F)) + 1))
      #else
        #define __CORE_CLK  (__CORE_CLK_PRE / (((RCC2_Val>>23) & (0x3F)) + 1) / 2)
      #endif
    #else
      #define __CORE_CLK  __CORE_CLK_PRE
    #endif
  #else
    #if (RCC_Val & (1UL<<11))                           /* check BYPASS */
              #if   (((RCC_Val>>4) & 0x03) == 0x0)                  
                #if   (((RCC_Val>>6) & 0x0F) == 0x0)                   
                      #define __CORE_CLK_PRE  1000000UL 
                #elif (((RCC_Val>>6) & 0x0F) == 0x1)                   
                      #define __CORE_CLK_PRE  1843200UL 
                #elif (((RCC_Val>>6) & 0x0F) == 0x2)                   
                      #define __CORE_CLK_PRE  2000000UL 
                #elif (((RCC_Val>>6) & 0x0F) == 0x3)                   
                      #define __CORE_CLK_PRE  2457600UL 
                #elif (((RCC_Val>>6) & 0x0F) == 0x4)                   
                      #define __CORE_CLK_PRE  3579545UL 
                #elif (((RCC_Val>>6) & 0x0F) == 0x5)                   
                      #define __CORE_CLK_PRE  3686400UL 
                #elif (((RCC_Val>>6) & 0x0F) == 0x6)                   
                      #define __CORE_CLK_PRE  4000000UL 
                #elif (((RCC_Val>>6) & 0x0F) == 0x7)                   
                      #define __CORE_CLK_PRE  4096000UL 
                #elif (((RCC_Val>>6) & 0x0F) == 0x8)                   
                      #define __CORE_CLK_PRE  4915200UL 
                #elif (((RCC_Val>>6) & 0x0F) == 0x9)                   
                      #define __CORE_CLK_PRE  5000000UL 
                #elif (((RCC_Val>>6) & 0x0F) == 0xA)                   
                      #define __CORE_CLK_PRE  5120000UL 
                #elif (((RCC_Val>>6) & 0x0F) == 0xB)                   
                      #define __CORE_CLK_PRE  6000000UL 
                #elif (((RCC_Val>>6) & 0x0F) == 0xC)                   
                      #define __CORE_CLK_PRE  6144000UL 
                #elif (((RCC_Val>>6) & 0x0F) == 0xD)                   
                      #define __CORE_CLK_PRE  7372800UL 
                #elif (((RCC_Val>>6) & 0x0F) == 0xE)                   
                      #define __CORE_CLK_PRE  8000000UL 
                #else                                   
                      #define __CORE_CLK_PRE  8192000UL 
                #endif                                  
              #elif (((RCC_Val>>4) & 0x03) == 0x1)                  
                  #define __CORE_CLK_PRE  XTALI         
              #elif (((RCC_Val>>4) & 0x03) == 0x2)                  
                  #define __CORE_CLK_PRE  (XTALI/4)     
              #else                                     
                  #define __CORE_CLK_PRE  XTAL30K       
              #endif
    #else
      #define __CORE_CLK_PRE   PLL_CLK
    #endif
    #if (RCC_Val & (1UL<<22))                            /* check USESYSDIV */
      #if (RCC_Val & (1UL<<11))                          /* check BYPASS */
        #define __CORE_CLK  (__CORE_CLK_PRE / (((RCC_Val>>23) & (0x0F)) + 1))
      #else
        #define __CORE_CLK  (__CORE_CLK_PRE / (((RCC_Val>>23) & (0x0F)) + 1) / 2)
      #endif
    #else
      #define __CORE_CLK  __CORE_CLK_PRE
    #endif
  #endif


/*----------------------------------------------------------------------------
  Clock Variable definitions
 *----------------------------------------------------------------------------*/
uint32_t SystemCoreClock         = __CORE_CLK;/*!< System Clock Frequency (Core Clock)*/
//uint32_t SystemFrequency_PwmClk  = XTALI;     /*!< PWM clock                           */
//uint32_t SystemFrequency_AdcClk  = ADC_CLK;   /*!< ADC clock                           */
//uint32_t SystemFrequency_CanClk  = CAN_CLK;   /*!< CAN clock                           */

/**
 * Update the system coreclock
 *
 * @param  none
 * @return none
 *
 * @brief  Update the system coreclock variable.
 */
void SystemCoreClockUpdate (void)            /* Get Core Clock Frequency      */
{
  uint32_t rcc, rcc2;

  /* Determine clock frequency according to clock register values */
  rcc  = SYSCTL->RCC;
  rcc2 = SYSCTL->RCC2;

  if (rcc2 & (1UL<<31)) {                             /* is rcc2 is used ? */
    if (rcc2 & (1UL<<11)) {                           /* check BYPASS */
      SystemCoreClock = getOscClk (((rcc>>6) & 0x0F),((rcc2>>4) & 0x07));
    } else {
      SystemCoreClock = PLL_CLK;
    }
    if (rcc & (1UL<<22)) {                            /* check USESYSDIV */
      if (rcc2 & (1UL<<11)) {
        SystemCoreClock = SystemCoreClock / (((rcc2>>23) & (0x3F)) + 1);
      } else {
        SystemCoreClock = SystemCoreClock / (((rcc2>>23) & (0x3F)) + 1) / 2;
      }
    }
  } else {
//    if (RCC_Val & (1UL<<11)) {                            /* check BYPASS */
    if (rcc & (1UL<<11)) {                            /* check BYPASS */ /* Simulation does not work at theis point */
      SystemCoreClock = getOscClk (((rcc>>6) & 0x0F),((rcc>>4) & 0x03));
    } else {
      SystemCoreClock = PLL_CLK;
    }
    if (rcc & (1UL<<22)) {                            /* check USESYSDIV */
      if (rcc & (1UL<<11)) {                          /* check BYPASS */ /* Simulation does not work at theis point */
//      if (RCC_Val & (1UL<<11)) {                          /* check BYPASS */
        SystemCoreClock = SystemCoreClock / (((rcc>>23) & (0x0F)) + 1);
      } else {
        SystemCoreClock = SystemCoreClock / (((rcc>>23) & (0x0F)) + 1) / 2;
      }
    }
  }

}


/**
 * Initialize the system
 *
 * @param  none
 * @return none
 *
 * @brief  Setup the microcontroller system.
 *         Initialize the System and update the SystemFrequency variable.
 */
void SystemInit (void) {
#if (CLOCK_SETUP)                       /* Clock Setup                        */
  uint32_t i;

  SYSCTL->RCC2 = 0x07802810;    /* set default value */
  SYSCTL->RCC  = 0x078E3AD1;    /* set default value */

  SYSCTL->RCC  = (RCC_Val  | (1UL<<11) | (1UL<<13)) & ~(1UL<<22); /* set value with BYPASS, PWRDN set, USESYSDIV reset */
  SYSCTL->RCC2 = (RCC2_Val | (1UL<<11) | (1UL<<13));              /* set value with BYPASS, PWRDN set */
  for (i = 0; i < 1000; i++);   /* wait a while */

  SYSCTL->RCC  = (RCC_Val  | (1UL<<11)) & ~(1UL<<22);             /* set value with BYPASS, USESYSDIV reset */
  SYSCTL->RCC2 = (RCC2_Val | (1UL<<11));                          /* set value with BYPASS */
  for (i = 0; i < 1000; i++);   /* wait a while */

  SYSCTL->RCC  = (RCC_Val  | (1<<11));                            /* set value with BYPASS */

  if ( (((RCC_Val  & (1UL<<13)) == 0) && ((RCC2_Val & (1UL<<31)) == 0)) ||
       (((RCC2_Val & (1UL<<13)) == 0) && ((RCC2_Val & (1UL<<31)) != 0))   ) {
    while ((SYSCTL->RIS & (1UL<<6)) != (1UL<<6));                 /* wait until PLL is locked */
  }

  SYSCTL->RCC  = (RCC_Val);                                       /* set value */
  SYSCTL->RCC2 = (RCC2_Val);                                      /* set value */
  for (i = 0; i < 10000; i++);   /* wait a while */

#endif

}

/*----------------------------------------------------------------------------
  Get the OSC clock
 *----------------------------------------------------------------------------*/
__INLINE static uint32_t getOscClk (uint32_t xtal, uint32_t oscSrc) {
  uint32_t oscClk = XTALI;

  switch (oscSrc) {                      /* switch OSCSRC */
    case 0:                              /* MOSC Main oscillator */
      switch (xtal) {                    /* switch XTAL */
        case 0x0:
          oscClk = 1000000UL;
          break;
        case 0x1:
          oscClk = 1843200UL;
          break;
        case 0x2:
          oscClk = 2000000UL;
          break;
        case 0x3:
          oscClk = 2457600UL;
          break;
        case 0x4:
          oscClk = 3579545UL;
          break;
        case 0x5:
          oscClk = 3686400UL;
          break;
        case 0x6:
          oscClk = 4000000UL;
          break;
        case 0x7:
          oscClk = 4096000UL;
          break;
        case 0x8:
          oscClk = 4915200UL;
          break;
        case 0x9:
          oscClk = 5000000UL;
          break;
        case 0xA:
          oscClk = 5120000UL;
          break;
        case 0xB:
          oscClk = 6000000UL;
          break;
        case 0xC:
          oscClk = 6144000UL;
          break;
        case 0xD:
          oscClk = 7372800UL;
          break;
        case 0xE:
          oscClk = 8000000UL;
          break;
        case 0xF:
          oscClk = 8192000UL;
          break;
       }
      break;
    case 1:                         /* IOSC Internal oscillator */
      oscClk = XTALI;
      break;
    case 2:                         /* IOSC/4 Internal oscillator/4 */
      oscClk = XTALI/4;
      break;
    case 3:                         /* 30kHz internal oscillator  */
      oscClk = XTAL30K;
      break;
  }

  return oscClk;
}

