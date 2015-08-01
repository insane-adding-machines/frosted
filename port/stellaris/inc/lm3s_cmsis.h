//*****************************************************************************
//
// lm3s_cmsis.h - CMSIS header file for Luminary Micro LM3S Stellaris
//                microcontroller.s
//
// This file is based on CMSIS specification 1.30 pre2 (13. Oct. 2009)
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

#ifndef __LM3S_CMSIS_H__
#define __LM3S_CMSIS_H__

/*
 * ============================================================================
 * ---------- Interrupt Number Definition -------------------------------------
 * ============================================================================
 */
typedef enum IRQn
{
/******  Cortex-M3 Processor Exceptions Numbers ******************************/
    NonMaskableInt_IRQn     = -14,  /*!< 2 Non Maskable Interrupt            */
    MemoryManagement_IRQn   = -12,  /*!< 4 Cortex-M3 Memory Management Int   */
    BusFault_IRQn           = -11,  /*!< 5 Cortex-M3 Bus Fault Interrupt     */
    UsageFault_IRQn         = -10,  /*!< 6 Cortex-M3 Usage Fault Interrupt   */
    SVCall_IRQn             = -5,   /*!< 11 Cortex-M3 SV Call Interrupt      */
    DebugMonitor_IRQn       = -4,   /*!< 12 Cortex-M3 Debug Monitor Interrupt*/
    PendSV_IRQn             = -2,   /*!< 14 Cortex-M3 Pend SV Interrupt      */
    SysTick_IRQn            = -1,   /*!< 15 Cortex-M3 System Tick Interrupt  */

/******  LM3S Specific Interrupt Numbers *************************************/
    GPIOPortA_IRQn          =  0,   /*!<  GPIO Port A                        */
    GPIOPortB_IRQn          =  1,   /*!<  GPIO Port B                        */
    GPIOPortC_IRQn          =  2,   /*!<  GPIO Port C                        */
    GPIOPortD_IRQn          =  3,   /*!<  GPIO Port D                        */
    GPIOPortE_IRQn          =  4,   /*!<  GPIO Port E                        */
    UART0_IRQn              =  5,   /*!<  UART0                              */
    UART1_IRQn              =  6,   /*!<  UART1                              */
    SSI0_IRQn               =  7,   /*!<  SSI0                               */
    I2C0_IRQn               =  8,   /*!<  I2C0                               */
    PWMFault_IRQn           =  9,   /*!<  PWM Fault                          */
    PWMGen0_IRQn            = 10,   /*!<  PWM Generator 0                    */
    PWMGen1_IRQn            = 11,   /*!<  PWM Generator 1                    */
    PWMGen2_IRQn            = 12,   /*!<  PWM Generator 2                    */
    QEI0_IRQn               = 13,   /*!<  Quadrature Encoder 0               */
    ADCSeq0_IRQn            = 14,   /*!<  ADC Sequence 0                     */
    ADCSeq1_IRQn            = 15,   /*!<  ADC Sequence 1                     */
    ADCSeq2_IRQn            = 16,   /*!<  ADC Sequence 2                     */
    ADCSeq3_IRQn            = 17,   /*!<  ADC Sequence 3                     */
    Watchdog_IRQn           = 18,   /*!<  Watchdog                           */
    Timer0A_IRQn            = 19,   /*!<  Timer 0A                           */
    Timer0B_IRQn            = 20,   /*!<  Timer 0B                           */
    Timer1A_IRQn            = 21,   /*!<  Timer 1A                           */
    Timer1B_IRQn            = 22,   /*!<  Timer 1B                           */
    Timer2A_IRQn            = 23,   /*!<  Timer 2A                           */
    Timer2B_IRQn            = 24,   /*!<  Timer 2B                           */
    Comp0_IRQn              = 25,   /*!<  Comp 0                             */
    Comp1_IRQn              = 26,   /*!<  Comp 1                             */
    Comp2_IRQn              = 27,   /*!<  Comp 2                             */
    SysCtrl_IRQn            = 28,   /*!<  System Control                     */
    FlashCtrl_IRQn          = 29,   /*!<  Flash Control                      */
    GPIOPortF_IRQn          = 30,   /*!<  GPIO Port F                        */
    GPIOPortG_IRQn          = 31,   /*!<  GPIO Port G                        */
    GPIOPortH_IRQn          = 32,   /*!<  GPIO Port H                        */
    USART2_IRQn             = 33,   /*!<  UART2 Rx and Tx                    */
    SSI1_IRQn               = 34,   /*!<  SSI1 Rx and Tx                     */
    Timer3A_IRQn            = 35,   /*!<  Timer 3 subtimer A                 */
    Timer3B_IRQn            = 36,   /*!<  Timer 3 subtimer B                 */
    I2C1_IRQn               = 37,   /*!<  I2C1 Master and Slave              */
    QEI1_IRQn               = 38,   /*!<  Quadrature Encoder 1               */
    CAN0_IRQn               = 39,   /*!<  CAN0                               */
    CAN1_IRQn               = 40,   /*!<  CAN1                               */
    CAN2_IRQn               = 41,   /*!<  CAN2                               */
    Ethernet_IRQn           = 42,   /*!<  Ethernet                           */
    Hibernate_IRQn          = 43,   /*!<  Hibernate                          */
    USB0_IRQn               = 44,   /*!<  USB0                               */
    PWMGen3_IRQn            = 45,   /*!<  PWM Generator 3                    */
    uDMA_IRQn               = 46,   /*!<  uDMA Software Transfer             */
    uDMAErr_IRQn            = 47    /*!<  uDMA Error                         */
} IRQn_Type;


/*
 * ============================================================================
 * ----------- Processor and Core Peripheral Section --------------------------
 * ============================================================================
 */

/* Configuration of the Cortex-M3 Processor and Core Peripherals */
#define __MPU_PRESENT           1   /*!< MPU present or not                  */
#define __NVIC_PRIO_BITS        3   /*!< Number of Bits used for Prio Levels */
#define __Vendor_SysTickConfig  0   /*!< 1 if different SysTick config used  */


#include "core_cm3.h"          /* Cortex-M3 processor and core periphs  */
#include "system_lm3s.h"       /* LM3S Stellaris system init            */


/**
 * @brief  Setup the initial configuration of the microcontroller
 * @param  none
 * @return none
 *
 * Initialize the system clocking according to the user configuration.
 */
extern void SystemInit (void);


/*****************************************************************************/
/*                Device Specific Peripheral registers structures            */
/*****************************************************************************/

/*----------       System Controller (SYSCTL)                     -----------*/
// <g> System Controller (SYSCTL)
typedef struct
{
    __I  uint32_t DID0;             // Device identification register 0
    __I  uint32_t DID1;             // Device identification register 1
    __I  uint32_t DC0;              // Device capabilities register 0
         uint8_t  RESERVED1[4];
    __I  uint32_t DC1;              // Device capabilities register 1
    __I  uint32_t DC2;              // Device capabilities register 2
    __I  uint32_t DC3;              // Device capabilities register 3
    __I  uint32_t DC4;              // Device capabilities register 4
    __I  uint32_t DC5;              // Device capabilities register 5
    __I  uint32_t DC6;              // Device capabilities register 6
    __I  uint32_t DC7;              // Device capabilities register 7
    __I  uint32_t DC8;              // Device capabilities register 8
    __IO uint32_t PBORCTL;          // POR/BOR reset control register
    __IO uint32_t LDOPCTL;          // LDO power control register
         uint8_t  RESERVED2[8];
    __IO uint32_t SRCR0;            // Software reset control reg 0
    __IO uint32_t SRCR1;            // Software reset control reg 1
    __IO uint32_t SRCR2;            // Software reset control reg 2
         uint8_t  RESERVED3[4];
    __I  uint32_t RIS;              // Raw interrupt status register
    __IO uint32_t IMC;              // Interrupt mask/control register
    __IO uint32_t MISC;             // Interrupt status register
    __IO uint32_t RESC;             // Reset cause register
    __IO uint32_t RCC;              // Run-mode clock config register
    __I  uint32_t PLLCFG;           // PLL configuration register
         uint8_t  RESERVED4[4];
    __IO uint32_t GPIOHBCTL;        // GPIO High-Performance Bus Control
#define GPIOHSCTL GPIOHBCTL         // [__IO] GPIO High Speed Control
    __IO uint32_t RCC2;             // Run-mode clock config register 2
         uint8_t  RESERVED6[8];
    __IO uint32_t MOSCCTL;          // Main Oscillator Control
         uint8_t  RESERVED7[128];
    __IO uint32_t RCGC0;            // Run-mode clock gating register 0
    __IO uint32_t RCGC1;            // Run-mode clock gating register 1
    __IO uint32_t RCGC2;            // Run-mode clock gating register 2
         uint8_t  RESERVED8[4];
    __IO uint32_t SCGC0;            // Sleep-mode clock gating reg 0
    __IO uint32_t SCGC1;            // Sleep-mode clock gating reg 1
    __IO uint32_t SCGC2;            // Sleep-mode clock gating reg 2
         uint8_t  RESERVED9[4];
    __IO uint32_t DCGC0;            // Deep Sleep-mode clock gate reg 0
    __IO uint32_t DCGC1;            // Deep Sleep-mode clock gate reg 1
    __IO uint32_t DCGC2;            // Deep Sleep-mode clock gate reg 2
         uint8_t  RESERVED10[24];
    __IO uint32_t DSLPCLKCFG;       // Deep Sleep-mode clock config reg
         uint8_t  RESERVED11[8];
    __IO uint32_t PIOSCCAL;         // Precision Internal Oscillator Calibration
    __I  uint32_t PIOSCSTAT;        // Precision Internal Oscillator Statistics
         uint8_t  RESERVED13[8];
    __IO uint32_t LDOARST;          // LDO reset control register
         uint8_t  RESERVED14[12];
    __IO uint32_t I2SMCLKCFG;       // I2S MCLK Configuration
         uint8_t  RESERVED15[28];
    __I  uint32_t DC9;              // Device capabilities register 9
         uint8_t  RESERVED16[12];
    __I  uint32_t NVMSTAT;          // Non-Volitile Memory Information
} SYSCTL_Type;
// </g>

/*----------       General Purpose Input/Output (GPIO)            -----------*/
// <g> General Purpose Input/Output (GPIO)
typedef struct
{
    __IO uint32_t DATA_Bits[255];   // Bit specific data registers
    __IO uint32_t DATA;             // Data register
    __IO uint32_t DIR;              // Data direction register
    __IO uint32_t IS;               // Interrupt sense register
    __IO uint32_t IBE;              // Interrupt both edges register
    __IO uint32_t IEV;              // Interrupt event register
    __IO uint32_t IM;               // Interrupt mask register
    __I  uint32_t RIS;              // Raw interrupt status register
    __I  uint32_t MIS;              // Masked interrupt status reg
    __O  uint32_t ICR;              // Interrupt clear register
    __IO uint32_t AFSEL;            // Mode control select register
         uint8_t  RESERVED1[220];
    __IO uint32_t DR2R;             // 2ma drive select register
    __IO uint32_t DR4R;             // 4ma drive select register
    __IO uint32_t DR8R;             // 8ma drive select register
    __IO uint32_t ODR;              // Open drain select register
    __IO uint32_t PUR;              // Pull up select register
    __IO uint32_t PDR;              // Pull down select register
    __IO uint32_t SLR;              // Slew rate control enable reg
    __IO uint32_t DEN;              // Digital input enable register
    __IO uint32_t LOCK;             // Lock register
    __IO  uint32_t CR;               // Commit register
    __IO uint32_t AMSEL;            // GPIO Analog Mode Select
    __IO uint32_t PCTL;             // GPIO Port Control
} GPIO_Type;
// </g>

/*----------       General Purpose Timer (TIMER)                  -----------*/
// <g> General Purpose Timer (TIMER)
typedef struct
{
    __IO uint32_t CFG;              // Configuration register
    __IO uint32_t TAMR;             // TimerA mode register
    __IO uint32_t TBMR;             // TimerB mode register
    __IO uint32_t CTL;              // Control register
         uint8_t  RESERVED0[8];
    __IO uint32_t IMR;              // Interrupt mask register
    __I  uint32_t RIS;              // Interrupt status register
    __I  uint32_t MIS;              // Masked interrupt status reg
    __O  uint32_t ICR;              // Interrupt clear register
    __IO uint32_t TAILR;            // TimerA interval load register
    __IO uint32_t TBILR;            // TimerB interval load register
    __IO uint32_t TAMATCHR;         // TimerA match register
    __IO uint32_t TBMATCHR;         // TimerB match register
    __IO uint32_t TAPR;             // TimerA prescale register
    __IO uint32_t TBPR;             // TimerB prescale register
    __IO uint32_t TAPMR;            // TimerA prescale match register
    __IO uint32_t TBPMR;            // TimerB prescale match register
    __I  uint32_t TAR;              // TimerA register
    __I  uint32_t TBR;              // TimerB register
    __I  uint32_t TAV;              // GPTM Timer A Value
    __I  uint32_t TBV;              // GPTM Timer B Value
} TIMER_Type;
// </g>

/*----------       Analog to Digital Converter (ADC)              -----------*/
// <g> Analog to Digital Converter (ADC)
typedef struct
{
    __IO uint32_t ACTSS;            // Active sample register
    __I  uint32_t RIS;              // Raw interrupt status register
    __IO uint32_t IM;               // Interrupt mask register
    __IO uint32_t ISC;              // Interrupt status/clear register
    __IO uint32_t OSTAT;            // Overflow status register
    __IO uint32_t EMUX;             // Event multiplexer select reg
    __IO uint32_t USTAT;            // Underflow status register
         uint8_t  RESERVED0[4];
    __IO uint32_t SSPRI;            // Channel priority register
         uint8_t  RESERVED1[4];
    __O  uint32_t PSSI;             // Processor sample initiate reg
         uint8_t  RESERVED2[4];
    __IO uint32_t SAC;              // Sample Averaging Control reg
    __IO uint32_t DCISC;            // ADC Digital Comparator Interrupt Status
                                    // and Clear
    __IO uint32_t CTL;              // ADC Control
         uint8_t  RESERVED3[4];
    __IO uint32_t SSMUX0;           // Multiplexer select 0 register
    __IO uint32_t SSCTL0;           // Sample sequence control 0 reg
    __I  uint32_t SSFIFO0;          // Result FIFO 0 register
    __I  uint32_t SSFSTAT0;         // FIFO 0 status register
    __IO uint32_t SSOP0;            // ADC Sample Sequence 0 Operation
    __IO uint32_t SSDC0;            // ADC Sample Sequence 0 Digital Comparator
                                    // Select
         uint8_t  RESERVED4[8];
    __IO uint32_t SSMUX1;           // Multiplexer select 1 register
    __IO uint32_t SSCTL1;           // Sample sequence control 1 reg
    __I  uint32_t SSFIFO1;          // Result FIFO 1 register
    __I  uint32_t SSFSTAT1;         // FIFO 1 status register
    __IO uint32_t SSOP1;            // ADC Sample Sequence 1 Operation
    __IO uint32_t SSDC1;            // ADC Sample Sequence 1 Digital Comparator
                                    // Select
         uint8_t  RESERVED5[8];
    __IO uint32_t SSMUX2;           // Multiplexer select 2 register
    __IO uint32_t SSCTL2;           // Sample sequence control 2 reg
    __I  uint32_t SSFIFO2;          // Result FIFO 2 register
    __I  uint32_t SSFSTAT2;         // FIFO 2 status register
    __IO uint32_t SSOP2;            // ADC Sample Sequence 2 Operation
    __IO uint32_t SSDC2;            // ADC Sample Sequence 2 Digital Comparator
                                    // Select
         uint8_t  RESERVED6[8];
    __IO uint32_t SSMUX3;           // Multiplexer select 3 register
    __IO uint32_t SSCTL3;           // Sample sequence control 3 reg
    __I  uint32_t SSFIFO3;          // Result FIFO 3 register
    __I  uint32_t SSFSTAT3;         // FIFO 3 status register
    __IO uint32_t SSOP3;            // ADC Sample Sequence 3 Operation
    __IO uint32_t SSDC3;            // ADC Sample Sequence 3 Digital Comparator
                                    // Select
         uint8_t  RESERVED7[72];
    __IO uint32_t TMLB;             // Test mode loopback register
         uint8_t  RESERVED8[3068];
    __IO uint32_t DCRIC;            // ADC Digital Comparator Reset Initial
                                    // Conditions
         uint8_t  RESERVED9[252];
    __IO uint32_t DCCTL0;           // ADC Digital Comparator Control 0
    __IO uint32_t DCCTL1;           // ADC Digital Comparator Control 1
    __IO uint32_t DCCTL2;           // ADC Digital Comparator Control 2
    __IO uint32_t DCCTL3;           // ADC Digital Comparator Control 3
    __IO uint32_t DCCTL4;           // ADC Digital Comparator Control 4
    __IO uint32_t DCCTL5;           // ADC Digital Comparator Control 5
    __IO uint32_t DCCTL6;           // ADC Digital Comparator Control 6
    __IO uint32_t DCCTL7;           // ADC Digital Comparator Control 7
         uint8_t  RESERVED10[32];
    __IO uint32_t DCCMP0;           // ADC Digital Comparator Range 0
    __IO uint32_t DCCMP1;           // ADC Digital Comparator Range 1
    __IO uint32_t DCCMP2;           // ADC Digital Comparator Range 2
    __IO uint32_t DCCMP3;           // ADC Digital Comparator Range 3
    __IO uint32_t DCCMP4;           // ADC Digital Comparator Range 4
    __IO uint32_t DCCMP5;           // ADC Digital Comparator Range 5
    __IO uint32_t DCCMP6;           // ADC Digital Comparator Range 6
    __IO uint32_t DCCMP7;           // ADC Digital Comparator Range 7
} ADC_Type;
// </g>

/*----------       Controller Area Network (CAN)                  -----------*/
// <g> Controller Area Network (CAN)
typedef struct
{
    __IO uint32_t CTL;              // Control register
    __IO uint32_t STS;              // Status register
    __I  uint32_t ERR;              // Error register
    __IO uint32_t BIT;              // Bit Timing register
    __I  uint32_t INT;              // Interrupt register
    __IO uint32_t TST;              // Test register
    __IO uint32_t BRPE;             // Baud Rate Prescaler register
         uint8_t  RESERVED0[4];
    __IO uint32_t IF1CRQ;           // Interface 1 Command Request reg
    __IO uint32_t IF1CMSK;          // Interface 1 Command Mask reg
    __IO uint32_t IF1MSK1;          // Interface 1 Mask 1 register
    __IO uint32_t IF1MSK2;          // Interface 1 Mask 2 register
    __IO uint32_t IF1ARB1;          // Interface 1 Arbitration 1 reg
    __IO uint32_t IF1ARB2;          // Interface 1 Arbitration 2 reg
    __IO uint32_t IF1MCTL;          // Interface 1 Message Control reg
    __IO uint32_t IF1DA1;           // Interface 1 DataA 1 register
    __IO uint32_t IF1DA2;           // Interface 1 DataA 2 register
    __IO uint32_t IF1DB1;           // Interface 1 DataB 1 register
    __IO uint32_t IF1DB2;           // Interface 1 DataB 2 register
         uint8_t  RESERVED1[52];
    __IO uint32_t IF2CRQ;           // Interface 2 Command Request reg
    __IO uint32_t IF2CMSK;          // Interface 2 Command Mask reg
    __IO uint32_t IF2MSK1;          // Interface 2 Mask 1 register
    __IO uint32_t IF2MSK2;          // Interface 2 Mask 2 register
    __IO uint32_t IF2ARB1;          // Interface 2 Arbitration 1 reg
    __IO uint32_t IF2ARB2;          // Interface 2 Arbitration 2 reg
    __IO uint32_t IF2MCTL;          // Interface 2 Message Control reg
    __IO uint32_t IF2DA1;           // Interface 2 DataA 1 register
    __IO uint32_t IF2DA2;           // Interface 2 DataA 2 register
    __IO uint32_t IF2DB1;           // Interface 2 DataB 1 register
    __IO uint32_t IF2DB2;           // Interface 2 DataB 2 register
         uint8_t  RESERVED2[84];
    __I  uint32_t TXRQ1;            // Transmission Request 1 register
    __I  uint32_t TXRQ2;            // Transmission Request 2 register
         uint8_t  RESERVED3[24];
    __I  uint32_t NWDA1;            // New Data 1 register
    __I  uint32_t NWDA2;            // New Data 2 register
         uint8_t  RESERVED4[24];
    __I  uint32_t MSG1INT;          // CAN Message 1 Interrupt Pending
    __I  uint32_t MSG2INT;          // CAN Message 2 Interrupt Pending
         uint8_t  RESERVED5[24];
    __I  uint32_t MSG1VAL;          // CAN Message 1 Valid
    __I  uint32_t MSG2VAL;          // CAN Message 2 Valid
} CAN_Type;
// </g>

/*----------       Analog Comparators (COMP)                      -----------*/
// <g> Analog Comparators (COMP)
typedef struct
{
    __IO uint32_t ACMIS;            // Analog Comparator Masked
                                    // Interrupt Status
    __I  uint32_t ACRIS;            // Analog Comparator Raw Interrupt Status
    __IO uint32_t ACINTEN;          // Analog Comparator Interrupt Enable
         uint8_t  RESERVED0[4];
    __IO uint32_t ACREFCTL;         // Analog Comparator Reference Voltage
                                    // Control
         uint8_t  RESERVED1[12];
    __I  uint32_t ACSTAT0;          // Comp0 status register
    __IO uint32_t ACCTL0;           // Comp0 control register
         uint8_t  RESERVED2[24];
    __I  uint32_t ACSTAT1;          // Comp1 status register
    __IO uint32_t ACCTL1;           // Comp1 control register
         uint8_t  RESERVED3[24];
    __I  uint32_t ACSTAT2;          // Comp2 status register
    __IO uint32_t ACCTL2;           // Comp2 control register
} COMP_Type;
// </g>

/*----------       External Peripheral Interface (EPI)            -----------*/
// <g> External Peripheral Interface (EPI)
typedef struct
{
    __IO uint32_t CFG;              // EPI Configuration
    __IO uint32_t BAUD;             // EPI Main Baud Rate
         uint8_t  RESERVED0[8];
    __IO uint32_t GPCFG;            // EPI General Purpose Configuration
#define HB16CFG   GPCFG             // EPI Host-Bus 16 Configuration
#define HB8CFG    GPCFG             // EPI Host-Bus 8 Mode Configuration
#define SDRAMCFG  GPCFG             // EPI SDRAM Mode Configuration
    __IO uint32_t GPCFG2;           // EPI General-Purpose Configuration 2
#define HB16CFG2  GPCFG2            // EPI Host-Bus 16 Configuration 2
#define HB8CFG2   GPCFG2            // EPI Host-Bus 8 Configuration 2
         uint8_t  RESERVED7[4];
    __IO uint32_t ADDRMAP;          // EPI Address Map
    __IO uint32_t RSIZE0;           // EPI Read Size 0
    __IO uint32_t RADDR0;           // EPI Read Address 0
    __IO uint32_t RPSTD0;           // EPI Non-Blocking Read Data 0
         uint8_t  RESERVED8[4];
    __IO uint32_t RSIZE1;           // EPI Read Size 1
    __IO uint32_t RADDR1;           // EPI Read Address 1
    __IO uint32_t RPSTD1;           // EPI Non-Blocking Read Data 1
         uint8_t  RESERVED9[36];
    __I  uint32_t STAT;             // EPI Status
         uint8_t  RESERVED10[8];
    __I  uint32_t RFIFOCNT;         // EPI Read FIFO Count
    __I  uint32_t READFIFO;         // EPI Read FIFO
    __I  uint32_t READFIFO1;        // EPI Read FIFO Alias 1
    __I  uint32_t READFIFO2;        // EPI Read FIFO Alias 2
    __I  uint32_t READFIFO3;        // EPI Read FIFO Alias 3
    __I  uint32_t READFIFO4;        // EPI Read FIFO Alias 4
    __I  uint32_t READFIFO5;        // EPI Read FIFO Alias 5
    __I  uint32_t READFIFO6;        // EPI Read FIFO Alias 6
    __I  uint32_t READFIFO7;        // EPI Read FIFO Alias 7
         uint8_t  RESERVED11[368];
    __IO uint32_t FIFOLVL;          // EPI FIFO Level Selects
    __I  uint32_t WFIFOCNT;         // EPI Write FIFO Count
         uint8_t  RESERVED12[8];
    __IO uint32_t IM;               // EPI Interrupt Mask
    __I  uint32_t RIS;              // EPI Raw Interrupt Status
    __I  uint32_t MIS;              // EPI Masked Interrupt Status
    __IO uint32_t EISC;             // EPI Error Interrupt Status and Clear
} EPI_Type;
// </g>

/*----------       Ethernet Controller (MAC)                      -----------*/
// <g> Ethernet Controller (MAC)
typedef struct
{
    __IO uint32_t RIS;              // (__I) Ethernet MAC Raw Interrupt Status
#define IACK RIS                    // (__O)Interrupt Acknowledge Register
    __IO uint32_t IM;               // Interrupt Mask Register
    __IO uint32_t RCTL;             // Receive Control Register
    __IO uint32_t TCTL;             // Transmit Control Register
    __IO uint32_t DATA;             // Data Register
    __IO uint32_t IA0;              // Individual Address Register 0
    __IO uint32_t IA1;              // Individual Address Register 1
    __IO uint32_t THR;              // Threshold Register
    __IO uint32_t MCTL;             // Management Control Register
    __IO uint32_t MDV;              // Management Divider Register
         uint8_t  RESERVED1[4];
    __IO uint32_t MTXD;             // Management Transmit Data Reg
    __IO uint32_t MRXD;             // Management Receive Data Reg
    __I  uint32_t NP;               // Number of Packets Register
    __IO uint32_t TR;               // Transmission Request Register
    __IO uint32_t TS;               // Timer Support Register
    __IO uint32_t LED;              // Ethernet MAC LED Encoding
    __IO uint32_t MDIX;             // MDIX Register
} MAC_Type;
// </g>

/*----------       Flash Memory Controller (FLASH)                -----------*/
// <g> Flash Memory Controller (FLASH)
typedef struct
{
    __IO uint32_t FMA;              // Memory address register
    __IO uint32_t FMD;              // Memory data register
    __IO uint32_t FMC;              // Memory control register
    __I  uint32_t FCRIS;            // Raw interrupt status register
    __IO uint32_t FCIM;             // Interrupt mask register
    __IO uint32_t FCMISC;           // Interrupt status register
         uint8_t  RESERVED1[8];
    __IO uint32_t FMC2;             // Flash Memory Control 2
         uint8_t  RESERVED2[12];
    __IO uint32_t FWBVAL;           // Flash Write Buffer Valid
         uint8_t  RESERVED3[204];
    __IO uint32_t FWBN;             // Flash Write Buffer Register n
         uint8_t  RESERVED4[4076];
    __IO uint32_t RMCTL;            // ROM Control
    __I  uint32_t RMVER;            // ROM Version Register
         uint8_t  RESERVED5[56];
    __IO uint32_t FMPRE;            // FLASH read protect register
    __IO uint32_t FMPPE;            // FLASH program protect register
         uint8_t  RESERVED6[8];
    __IO uint32_t USECRL;           // uSec reload register
         uint8_t  RESERVED7[140];
    __IO uint32_t USERDBG;          // User Debug
         uint8_t  RESERVED8[12];
    __IO uint32_t USERREG0;         // User Register 0
    __IO uint32_t USERREG1;         // User Register 1
    __IO uint32_t USERREG2;         // User Register 2
    __IO uint32_t USERREG3;         // User Register 3
         uint8_t  RESERVED9[16];
    __IO uint32_t FMPRE0;           // FLASH read protect register 0
    __IO uint32_t FMPRE1;           // FLASH read protect register 1
    __IO uint32_t FMPRE2;           // FLASH read protect register 2
    __IO uint32_t FMPRE3;           // FLASH read protect register 3
         uint8_t  RESERVED10[496];
    __IO uint32_t FMPPE0;           // FLASH program protect register 0
    __IO uint32_t FMPPE1;           // FLASH program protect register 1
    __IO uint32_t FMPPE2;           // FLASH program protect register 2
    __IO uint32_t FMPPE3;           // FLASH program protect register 3
} FLASH_Type;
// </g>

/*----------       Hibernation Module (HIB)                       -----------*/
// <g> Hibernation Module (HIB)
typedef struct
{
    __I  uint32_t RTCC;             // Hibernate RTC counter
    __IO uint32_t RTCM0;            // Hibernate RTC match 0
    __IO uint32_t RTCM1;            // Hibernate RTC match 1
    __IO uint32_t RTCLD;            // Hibernate RTC load
    __IO uint32_t CTL;              // Hibernate RTC control
    __IO uint32_t IM;               // Hibernate interrupt mask
    __I  uint32_t RIS;              // Hibernate raw interrupt status
    __I  uint32_t MIS;              // Hibernate masked interrupt stat
    __IO uint32_t IC;               // Hibernate interrupt clear
    __IO uint32_t RTCT;             // Hibernate RTC trim
         uint8_t  RESERVED1[8];
    __IO uint32_t DATA[64];         // Hibernate data area
} HIB_Type;
// </g>

/*-------- Inter-Integrated Circuit Controller Master (I2C_MASTER) ----------*/
// <g> Inter-Integrated Circuit Controller Master (I2C_MASTER)
typedef struct
{
    __IO uint32_t MSA;              // I2C Master Slave Address
    __IO uint32_t MCS;              // I2C Master Control/Status
    __IO uint32_t MDR;              // I2C Master Data
    __IO uint32_t MTPR;             // I2C Master Timer Period
    __IO uint32_t MIMR;             // I2C Master Interrupt Mask
    __I  uint32_t MRIS;             // I2C Master Raw Interrupt Status
    __I  uint32_t MMIS;             // I2C Master Masked Interrupt Status
    __O  uint32_t MICR;             // I2C Master Interrupt Clear
    __IO uint32_t MCR;              // I2C Master Configuration
} I2C_MASTER_Type;
// </g>

/*--------- Inter-Integrated Circuit Controller Slave (I2C_SLAVE) -----------*/
// <g> Inter-Integrated Circuit Controller Slave (I2C_SLAVE)
typedef struct
{
    __IO uint32_t SOAR;             // I2C Slave Own Address
    __I  uint32_t SCSR;             // I2C Slave Control/Status
    __IO uint32_t SDR;              // I2C Slave Data
    __IO uint32_t SIMR;             // I2C Slave Interrupt Mask
    __I  uint32_t SRIS;             // I2C Slave Raw Interrupt Status
    __I  uint32_t SMIS;             // I2C Slave Masked Interrupt Status
    __O  uint32_t SICR;             // I2C Slave Interrupt Clear
} I2C_SLAVE_Type;
// </g>

/*----------       Inter-Integrated Circuit Sound (I2S)           -----------*/
// <g> Inter-Integrated Circuit Sound (I2S)
typedef struct
{
    __O  uint32_t TXFIFO;           // I2S Transmit FIFO Data
    __IO uint32_t TXFIFOCFG;        // I2S Transmit FIFO Configuration
    __IO uint32_t TXCFG;            // I2S Transmit Module Configuration
    __IO uint32_t TXLIMIT;          // I2S Transmit FIFO Limit
    __IO uint32_t TXISM;            // I2S Transmit Interrupt Status and Mask
         uint8_t  RESERVED0[4];
    __I  uint32_t TXLEV;            // I2S Transmit FIFO Level
         uint8_t  RESERVED1[2020];
    __I  uint32_t RXFIFO;           // I2S Receive FIFO Data
    __IO uint32_t RXFIFOCFG;        // I2S Receive FIFO Configuration
    __IO uint32_t RXCFG;            // I2S Receive Module Configuration
    __IO uint32_t RXLIMIT;          // I2S Receive FIFO Limit
    __IO uint32_t RXISM;            // I2S Receive Interrupt Status and Mask
         uint8_t  RESERVED2[4];
    __I  uint32_t RXLEV;            // I2S Receive FIFO Level
         uint8_t  RESERVED3[996];
    __IO uint32_t CFG;              // I2S Module Configuration
         uint8_t  RESERVED4[12];
    __IO uint32_t IM;               // I2S Interrupt Mask
    __I  uint32_t RIS;              // I2S Raw Interrupt Status
    __I  uint32_t MIS;              // I2S Masked Interrupt Status
    __O  uint32_t IC;               // I2S Interrupt Clear
} I2S_Type;
// </g>

/*----------       Pulse Width Modulation (PWM)                   -----------*/
// <g> Pulse Width Modulation (PWM)
typedef struct
{
    __IO uint32_t CTL;              // PWM Master Control register
    __IO uint32_t SYNC;             // PWM Time Base Sync register
    __IO uint32_t ENABLE;           // PWM Output Enable register
    __IO uint32_t INVERT;           // PWM Output Inversion register
    __IO uint32_t FAULT;            // PWM Output Fault register
    __IO uint32_t INTEN;            // PWM Interrupt Enable register
    __I  uint32_t RIS;              // PWM Interrupt Raw Status reg
    __IO uint32_t ISC;              // PWM Interrupt Status register
    __I  uint32_t STATUS;           // PWM Status register
    __IO uint32_t FAULTVAL;         // PWM Fault Condition Value
    __IO uint32_t ENUPD;            // PWM Enable Update
         uint8_t  RESERVED0[20];
    __IO uint32_t GEN0_CTL;            // PWM0 Control
    __IO uint32_t GEN0_INTEN;          // PWM0 Interrupt and Trigger Enable
    __I  uint32_t GEN0_RIS;            // PWM0 Raw Interrupt Status
    __IO uint32_t GEN0_ISC;            // PWM0 Interrupt Status and Clear
    __IO uint32_t GEN0_LOAD;           // PWM0 Load
    __I  uint32_t GEN0_COUNT;          // PWM0 Counter
    __IO uint32_t GEN0_CMPA;           // PWM0 Compare A
    __IO uint32_t GEN0_CMPB;           // PWM0 Compare B
    __IO uint32_t GEN0_GENA;           // PWM0 Generator A Control
    __IO uint32_t GEN0_GENB;           // PWM0 Generator B Control
    __IO uint32_t GEN0_DBCTL;          // PWM0 Dead-Band Control
    __IO uint32_t GEN0_DBRISE;         // PWM0 Dead-Band Rising-Edge Delay
    __IO uint32_t GEN0_DBFALL;         // PWM0 Dead-Band Falling-Edge-Delay
    __IO uint32_t GEN0_FLTSRC0;        // PWM0 Fault Source 0
    __IO uint32_t GEN0_FLTSRC1;        // PWM0 Fault Source 1
    __IO uint32_t GEN0_MINFLTPER;      // PWM0 Minimum Fault Period
    __IO uint32_t GEN1_CTL;            // PWM1 Control
    __IO uint32_t GEN1_INTEN;          // PWM1 Interrupt Enable
    __I  uint32_t GEN1_RIS;            // PWM1 Raw Interrupt Status
    __IO uint32_t GEN1_ISC;            // PWM1 Interrupt Status and Clear
    __IO uint32_t GEN1_LOAD;           // PWM1 Load
    __I  uint32_t GEN1_COUNT;          // PWM1 Counter
    __IO uint32_t GEN1_CMPA;           // PWM1 Compare A
    __IO uint32_t GEN1_CMPB;           // PWM1 Compare B
    __IO uint32_t GEN1_GENA;           // PWM1 Generator A Control
    __IO uint32_t GEN1_GENB;           // PWM1 Generator B Control
    __IO uint32_t GEN1_DBCTL;          // PWM1 Dead-Band Control
    __IO uint32_t GEN1_DBRISE;         // PWM1 Dead-Band Rising-Edge Delay
    __IO uint32_t GEN1_DBFALL;         // PWM1 Dead-Band Falling-Edge-Delay
    __IO uint32_t GEN1_FLTSRC0;        // PWM1 Fault Source 0
    __IO uint32_t GEN1_FLTSRC1;        // PWM1 Fault Source 1
    __IO uint32_t GEN1_MINFLTPER;      // PWM1 Minimum Fault Period
    __IO uint32_t GEN2_CTL;            // PWM2 Control
    __IO uint32_t GEN2_INTEN;          // PWM2 InterruptEnable
    __I  uint32_t GEN2_RIS;            // PWM2 Raw Interrupt Status
    __IO uint32_t GEN2_ISC;            // PWM2 Interrupt Status and Clear
    __IO uint32_t GEN2_LOAD;           // PWM2 Load
    __I  uint32_t GEN2_COUNT;          // PWM2 Counter
    __IO uint32_t GEN2_CMPA;           // PWM2 Compare A
    __IO uint32_t GEN2_CMPB;           // PWM2 Compare B
    __IO uint32_t GEN2_GENA;           // PWM2 Generator A Control
    __IO uint32_t GEN2_GENB;           // PWM2 Generator B Control
    __IO uint32_t GEN2_DBCTL;          // PWM2 Dead-Band Control
    __IO uint32_t GEN2_DBRISE;         // PWM2 Dead-Band Rising-Edge Delay
    __IO uint32_t GEN2_DBFALL;         // PWM2 Dead-Band Falling-Edge-Delay
    __IO uint32_t GEN2_FLTSRC0;        // PWM2 Fault Source 0
    __IO uint32_t GEN2_FLTSRC1;        // PWM2 Fault Source 1
    __IO uint32_t GEN2_MINFLTPER;      // PWM2 Minimum Fault Period
    __IO uint32_t GEN3_CTL;            // PWM3 Control
    __IO uint32_t GEN3_INTEN;          // PWM3 Interrupt and Trigger Enable
    __I  uint32_t GEN3_RIS;            // PWM3 Raw Interrupt Status
    __IO uint32_t GEN3_ISC;            // PWM3 Interrupt Status and Clear
    __IO uint32_t GEN3_LOAD;           // PWM3 Load
    __I  uint32_t GEN3_COUNT;          // PWM3 Counter
    __IO uint32_t GEN3_CMPA;           // PWM3 Compare A
    __IO uint32_t GEN3_CMPB;           // PWM3 Compare B
    __IO uint32_t GEN3_GENA;           // PWM3 Generator A Control
    __IO uint32_t GEN3_GENB;           // PWM3 Generator B Control
    __IO uint32_t GEN3_DBCTL;          // PWM3 Dead-Band Control
    __IO uint32_t GEN3_DBRISE;         // PWM3 Dead-Band Rising-Edge Delay
    __IO uint32_t GEN3_DBFALL;         // PWM3 Dead-Band Falling-Edge-Delay
    __IO uint32_t GEN3_FLTSRC0;        // PWM3 Fault Source 0
    __IO uint32_t GEN3_FLTSRC1;        // PWM3 Fault Source 1
    __IO uint32_t GEN3_MINFLTPER;      // PWM3 Minimum Fault Period
         uint8_t  RESERVED1[1728];
    __IO uint32_t GEN0_FLTSEN;         // PWM0 Fault Pin Logic Sense
    __IO uint32_t GEN0_FLTSTAT0;       // PWM0 Fault Status 0
    __IO uint32_t GEN0_FLTSTAT1;       // PWM0 Fault Status 1
         uint8_t  RESERVED2[116];
    __IO uint32_t GEN1_FLTSEN;         // PWM1 Fault Pin Logic Sense
    __IO uint32_t GEN1_FLTSTAT0;       // PWM1 Fault Status 0
    __IO uint32_t GEN1_FLTSTAT1;       // PWM1 Fault Status 1
         uint8_t  RESERVED3[116];
    __IO uint32_t GEN2_FLTSEN;         // PWM2 Fault Pin Logic Sense
    __IO uint32_t GEN2_FLTSTAT0;       // PWM2 Fault Status 0
    __IO uint32_t GEN2_FLTSTAT1;       // PWM2 Fault Status 1
         uint8_t  RESERVED4[116];
    __IO uint32_t GEN3_FLTSEN;         // PWM3 Fault Pin Logic Sense
    __IO uint32_t GEN3_FLTSTAT0;       // PWM3 Fault Status 0
    __IO uint32_t GEN3_FLTSTAT1;       // PWM3 Fault Status 1
} PWM_Type;
// </g>

/*----------       Pulse Width Modulation Generator (PWMGEN)      -----------*/
// <g> Pulse Width Modulation Generator (PWMGEN)
typedef struct
{
    __IO uint32_t CTL;                 // PWM0 Control
    __IO uint32_t INTEN;               // PWM0 Interrupt and Trigger Enable
    __I  uint32_t RIS;                 // PWM0 Raw Interrupt Status
    __IO uint32_t ISC;                 // PWM0 Interrupt Status and Clear
    __IO uint32_t LOAD;                // PWM0 Load
    __I  uint32_t COUNT;               // PWM0 Counter
    __IO uint32_t CMPA;                // PWM0 Compare A
    __IO uint32_t CMPB;                // PWM0 Compare B
    __IO uint32_t GENA;                // PWM0 Generator A Control
    __IO uint32_t GENB;                // PWM0 Generator B Control
    __IO uint32_t DBCTL;               // PWM0 Dead-Band Control
    __IO uint32_t DBRISE;              // PWM0 Dead-Band Rising-Edge Delay
    __IO uint32_t DBFALL;              // PWM0 Dead-Band Falling-Edge-Delay
    __IO uint32_t FLTSRC0;             // PWM0 Fault Source 0
    __IO uint32_t FLTSRC1;             // PWM0 Fault Source 1
    __IO uint32_t MINFLTPER;           // PWM0 Minimum Fault Period
         uint8_t  RESERVED1[1920];
    __IO uint32_t FLTSEN;              // PWM0 Fault Pin Logic Sense
    __IO uint32_t FLTSTAT0;            // PWM0 Fault Status 0
    __IO uint32_t FLTSTAT1;            // PWM0 Fault Status 1
} PWMGEN_Type;
// </g>

/*----------       Quadrature Encoded Input (QEI)                 -----------*/
// <g> Quadrature Encoded Input (QEI)
typedef struct
{
    __IO uint32_t CTL;                  // Configuration and control reg
    __I  uint32_t STAT;                 // Status register
    __IO uint32_t POS;                  // Current position register
    __IO uint32_t MAXPOS;               // Maximum position register
    __IO uint32_t LOAD;                 // Velocity timer load register
    __I  uint32_t TIME;                 // Velocity timer register
    __I  uint32_t COUNT;                // Velocity pulse count register
    __I  uint32_t SPEED;                // Velocity speed register
    __IO uint32_t INTEN;                // Interrupt enable register
    __I  uint32_t RIS;                  // Raw interrupt status register
    __IO uint32_t ISC;                  // Interrupt status register
} QEI_Type;
// </g>

/*----------       Synchronous Serial Interface (SSI)             -----------*/
// <g> Synchronous Serial Interface (SSI)
typedef struct
{
    __IO uint32_t CR0;              // Control register 0
    __IO uint32_t CR1;              // Control register 1
    __IO uint32_t DR;               // Data register
    __I  uint32_t SR;               // Status register
    __IO uint32_t CPSR;             // Clock prescale register
    __IO uint32_t IM;               // Int mask set and clear register
    __I  uint32_t RIS;              // Raw interrupt register
    __I  uint32_t MIS;              // Masked interrupt register
    __O  uint32_t ICR;              // Interrupt clear register
    __IO uint32_t DMACTL;           // SSI DMA Control
} SSI_Type;
// </g>

/*----------       Asynchronous Serial (UART)                     -----------*/
// <g> Asynchronous Serial (UART)
typedef struct
{
    __IO uint32_t DR;               // Data Register
    __IO uint32_t RSR;              // Receive Status Register (read)
#define ECR RSR                     // Error Clear Register (write)
         uint8_t  RESERVED1[16];
    __I  uint32_t FR;               // Flag Register (read only)
         uint8_t  RESERVED2[4];
    __IO uint32_t ILPR;             // UART IrDA Low-Power Register
    __IO uint32_t IBRD;             // Integer Baud Rate Divisor Reg
    __IO uint32_t FBRD;             // Fractional Baud Rate Divisor Reg
    __IO uint32_t LCRH;             // UART Line Control
    __IO uint32_t CTL;              // Control Register
    __IO uint32_t IFLS;             // Interrupt FIFO Level Select Reg
    __IO uint32_t IM;               // Interrupt Mask Set/Clear Reg
    __I  uint32_t RIS;              // Raw Interrupt Status Register
    __I  uint32_t MIS;              // Masked Interrupt Status Register
    __O  uint32_t ICR;              // Interrupt Clear Register
    __IO uint32_t DMACTL;           // UART DMA Control
         uint8_t  RESERVED3[68];
    __IO uint32_t LCTL;             // UART LIN Control
    __I  uint32_t LSS;              // UART LIN Snap Shot
    __I  uint32_t LTIM;             // UART LIN Timer
} UART_Type;
// </g>

/*----------       DMA Controller (UDMA)                          -----------*/
// <g> DMA Controller (UDMA)
typedef struct
{
    __I  uint32_t STAT;             // DMA Status
    __O  uint32_t CFG;              // DMA Configuration
    __IO uint32_t CTLBASE;          // DMA Channel Control Base Pointer
    __I  uint32_t ALTBASE;          // DMA Alternate Channel Control Base
                                    // Pointer
    __I  uint32_t WAITSTAT;         // DMA Channel Wait on Request Status
    __O  uint32_t SWREQ;            // DMA Channel Software Request
    __IO uint32_t USEBURSTSET;      // DMA Channel Useburst Set
    __O  uint32_t USEBURSTCLR;      // DMA Channel Useburst Clear
    __IO uint32_t REQMASKSET;       // DMA Channel Request Mask Set
    __O  uint32_t REQMASKCLR;       // DMA Channel Request Mask Clear
    __IO uint32_t ENASET;           // DMA Channel Enable Set
    __O  uint32_t ENACLR;           // DMA Channel Enable Clear
    __IO uint32_t ALTSET;           // DMA Channel Primary Alternate Set
    __O  uint32_t ALTCLR;           // DMA Channel Primary Alternate Clear
    __IO uint32_t PRIOSET;          // DMA Channel Priority Set
    __O  uint32_t PRIOCLR;          // DMA Channel Priority Clear
         uint8_t  RESERVED1[12];
    __IO uint32_t ERRCLR;           // DMA Bus Error Clear
         uint8_t  RESERVED2[1200];
    __IO uint32_t CHALT;            // DMA Channel Alternate Select
} UDMA_Type;
// </g>

/*----------       DMA Channel Control Structure (UDMA_CTRL)      -----------*/
// <g> DMA Channel Control Structure (UDMA_CTRL)
typedef struct
{
    __IO uint32_t SRCENDP;          // DMA Channel Source Address End Pointer
    __IO uint32_t DSTENDP;          // DMA Channel Destination Address End
                                    // Pointer
    __IO uint32_t CHCTL;            // DMA Channel Control Word
} UDMA_CTRL_Type;
// </g>

/*----------       Universal Serial Bus Controller (USB)          -----------*/
// <g> Universal Serial Bus Controller (USB)
typedef struct
{
    __IO uint8_t  FADDR;            // USB Device Functional Address
    __IO uint8_t  POWER;            // USB Power
    __I  uint16_t TXIS;             // USB Transmit Interrupt Status
    __I  uint16_t RXIS;             // USB Receive Interrupt Status
    __IO uint16_t TXIE;             // USB Transmit Interrupt Enable
    __IO uint16_t RXIE;             // USB Receive Interrupt Enable
    __I  uint8_t  IS;               // USB General Interrupt Status
    __IO uint8_t  IE;               // USB Interrupt Enable
    __I  uint16_t FRAME;            // USB Frame Value
    __IO uint8_t  EPIDX;            // USB Endpoint Index
    __IO uint8_t  TEST;             // USB Test Mode
         uint8_t  RESERVED0[16];
    __IO uint32_t FIFO0;            // USB FIFO Endpoint 0
    __IO uint32_t FIFO1;            // USB FIFO Endpoint 1
    __IO uint32_t FIFO2;            // USB FIFO Endpoint 2
    __IO uint32_t FIFO3;            // USB FIFO Endpoint 3
    __IO uint32_t FIFO4;            // USB FIFO Endpoint 4
    __IO uint32_t FIFO5;            // USB FIFO Endpoint 5
    __IO uint32_t FIFO6;            // USB FIFO Endpoint 6
    __IO uint32_t FIFO7;            // USB FIFO Endpoint 7
    __IO uint32_t FIFO8;            // USB FIFO Endpoint 8
    __IO uint32_t FIFO9;            // USB FIFO Endpoint 9
    __IO uint32_t FIFO10;           // USB FIFO Endpoint 10
    __IO uint32_t FIFO11;           // USB FIFO Endpoint 11
    __IO uint32_t FIFO12;           // USB FIFO Endpoint 12
    __IO uint32_t FIFO13;           // USB FIFO Endpoint 13
    __IO uint32_t FIFO14;           // USB FIFO Endpoint 14
    __IO uint32_t FIFO15;           // USB FIFO Endpoint 15
    __IO uint8_t  DEVCTL;           // USB Device Control
         uint8_t  RESERVED1[1];
    __IO uint8_t  TXFIFOSZ;         // USB Transmit Dynamic FIFO Sizing
    __IO uint8_t  RXFIFOSZ;         // USB Receive Dynamic FIFO Sizing
    __IO uint16_t TXFIFOADD;        // USB Transmit FIFO Start Address
    __IO uint16_t RXFIFOADD;        // USB Receive FIFO Start Address
         uint8_t  RESERVED2[18];
    __IO uint8_t  CONTIM;           // USB Connect Timing
    __IO uint8_t  VPLEN;            // USB OTG VBus Pulse Timing
         uint8_t  RESERVED3[1];
    __IO uint8_t  FSEOF;            // USB Full-Speed Last Transaction to End
                                    // of Frame Timing
    __IO uint8_t  LSEOF;            // USB Low-Speed Last Transaction to End of
                                    // Frame Timing
         uint8_t  RESERVED4[1];
    __IO uint8_t  TXFUNCADDR0;      // USB Transmit Functional Address
                                    // Endpoint 0
         uint8_t  RESERVED5[1];
    __IO uint8_t  TXHUBADDR0;       // USB Transmit Hub Address Endpoint 0
    __IO uint8_t  TXHUBPORT0;       // USB Transmit Hub Port Endpoint 0
         uint8_t  RESERVED6[4];
    __IO uint8_t  TXFUNCADDR1;      // USB Transmit Functional Address
                                    // Endpoint 1
         uint8_t  RESERVED7[1];
    __IO uint8_t  TXHUBADDR1;       // USB Transmit Hub Address Endpoint 1
    __IO uint8_t  TXHUBPORT1;       // USB Transmit Hub Port Endpoint 1
    __IO uint8_t  RXFUNCADDR1;      // USB Receive Functional Address
                                    // Endpoint 1
         uint8_t  RESERVED8[1];
    __IO uint8_t  RXHUBADDR1;       // USB Receive Hub Address Endpoint 1
    __IO uint8_t  RXHUBPORT1;       // USB Receive Hub Port Endpoint 1
    __IO uint8_t  TXFUNCADDR2;      // USB Transmit Functional Address
                                    // Endpoint 2
         uint8_t  RESERVED9[1];
    __IO uint8_t  TXHUBADDR2;       // USB Transmit Hub Address Endpoint 2
    __IO uint8_t  TXHUBPORT2;       // USB Transmit Hub Port Endpoint 2
    __IO uint8_t  RXFUNCADDR2;      // USB Receive Functional Address
                                    // Endpoint 2
         uint8_t  RESERVED10[1];
    __IO uint8_t  RXHUBADDR2;       // USB Receive Hub Address Endpoint 2
    __IO uint8_t  RXHUBPORT2;       // USB Receive Hub Port Endpoint 2
    __IO uint8_t  TXFUNCADDR3;      // USB Transmit Functional Address
                                    // Endpoint 3
         uint8_t  RESERVED11[1];
    __IO uint8_t  TXHUBADDR3;       // USB Transmit Hub Address Endpoint 3
    __IO uint8_t  TXHUBPORT3;       // USB Transmit Hub Port Endpoint 3
    __IO uint8_t  RXFUNCADDR3;      // USB Receive Functional Address
                                    // Endpoint 3
         uint8_t  RESERVED12[1];
    __IO uint8_t  RXHUBADDR3;       // USB Receive Hub Address Endpoint 3
    __IO uint8_t  RXHUBPORT3;       // USB Receive Hub Port Endpoint 3
    __IO uint8_t  TXFUNCADDR4;      // USB Transmit Functional Address
                                    // Endpoint 4
         uint8_t  RESERVED13[1];
    __IO uint8_t  TXHUBADDR4;       // USB Transmit Hub Address Endpoint 4
    __IO uint8_t  TXHUBPORT4;       // USB Transmit Hub Port Endpoint 4
    __IO uint8_t  RXFUNCADDR4;      // USB Receive Functional Address
                                    // Endpoint 4
         uint8_t  RESERVED14[1];
    __IO uint8_t  RXHUBADDR4;       // USB Receive Hub Address Endpoint 4
    __IO uint8_t  RXHUBPORT4;       // USB Receive Hub Port Endpoint 4
    __IO uint8_t  TXFUNCADDR5;      // USB Transmit Functional Address
                                    // Endpoint 5
         uint8_t  RESERVED15[1];
    __IO uint8_t  TXHUBADDR5;       // USB Transmit Hub Address Endpoint 5
    __IO uint8_t  TXHUBPORT5;       // USB Transmit Hub Port Endpoint 5
    __IO uint8_t  RXFUNCADDR5;      // USB Receive Functional Address
                                    // Endpoint 5
         uint8_t  RESERVED16[1];
    __IO uint8_t  RXHUBADDR5;       // USB Receive Hub Address Endpoint 5
    __IO uint8_t  RXHUBPORT5;       // USB Receive Hub Port Endpoint 5
    __IO uint8_t  TXFUNCADDR6;      // USB Transmit Functional Address
                                    // Endpoint 6
         uint8_t  RESERVED17[1];
    __IO uint8_t  TXHUBADDR6;       // USB Transmit Hub Address Endpoint 6
    __IO uint8_t  TXHUBPORT6;       // USB Transmit Hub Port Endpoint 6
    __IO uint8_t  RXFUNCADDR6;      // USB Receive Functional Address
                                    // Endpoint 6
         uint8_t  RESERVED18[1];
    __IO uint8_t  RXHUBADDR6;       // USB Receive Hub Address Endpoint 6
    __IO uint8_t  RXHUBPORT6;       // USB Receive Hub Port Endpoint 6
    __IO uint8_t  TXFUNCADDR7;      // USB Transmit Functional Address
                                    // Endpoint 7
         uint8_t  RESERVED19[1];
    __IO uint8_t  TXHUBADDR7;       // USB Transmit Hub Address Endpoint 7
    __IO uint8_t  TXHUBPORT7;       // USB Transmit Hub Port Endpoint 7
    __IO uint8_t  RXFUNCADDR7;      // USB Receive Functional Address
                                    // Endpoint 7
         uint8_t  RESERVED20[1];
    __IO uint8_t  RXHUBADDR7;       // USB Receive Hub Address Endpoint 7
    __IO uint8_t  RXHUBPORT7;       // USB Receive Hub Port Endpoint 7
    __IO uint8_t  TXFUNCADDR8;      // USB Transmit Functional Address
                                    // Endpoint 8
         uint8_t  RESERVED21[1];
    __IO uint8_t  TXHUBADDR8;       // USB Transmit Hub Address Endpoint 8
    __IO uint8_t  TXHUBPORT8;       // USB Transmit Hub Port Endpoint 8
    __IO uint8_t  RXFUNCADDR8;      // USB Receive Functional Address
                                    // Endpoint 8
         uint8_t  RESERVED22[1];
    __IO uint8_t  RXHUBADDR8;       // USB Receive Hub Address Endpoint 8
    __IO uint8_t  RXHUBPORT8;       // USB Receive Hub Port Endpoint 8
    __IO uint8_t  TXFUNCADDR9;      // USB Transmit Functional Address
                                    // Endpoint 9
         uint8_t  RESERVED23[1];
    __IO uint8_t  TXHUBADDR9;       // USB Transmit Hub Address Endpoint 9
    __IO uint8_t  TXHUBPORT9;       // USB Transmit Hub Port Endpoint 9
    __IO uint8_t  RXFUNCADDR9;      // USB Receive Functional Address
                                    // Endpoint 9
         uint8_t  RESERVED24[1];
    __IO uint8_t  RXHUBADDR9;       // USB Receive Hub Address Endpoint 9
    __IO uint8_t  RXHUBPORT9;       // USB Receive Hub Port Endpoint 9
    __IO uint8_t  TXFUNCADDR10;     // USB Transmit Functional Address
                                    // Endpoint 10
         uint8_t  RESERVED25[1];
    __IO uint8_t  TXHUBADDR10;      // USB Transmit Hub Address Endpoint 10
    __IO uint8_t  TXHUBPORT10;      // USB Transmit Hub Port Endpoint 10
    __IO uint8_t  RXFUNCADDR10;     // USB Receive Functional Address
                                    // Endpoint 10
         uint8_t  RESERVED26[1];
    __IO uint8_t  RXHUBADDR10;      // USB Receive Hub Address Endpoint 10
    __IO uint8_t  RXHUBPORT10;      // USB Receive Hub Port Endpoint 10
    __IO uint8_t  TXFUNCADDR11;     // USB Transmit Functional Address
                                    // Endpoint 11
         uint8_t  RESERVED27[1];
    __IO uint8_t  TXHUBADDR11;      // USB Transmit Hub Address Endpoint 11
    __IO uint8_t  TXHUBPORT11;      // USB Transmit Hub Port Endpoint 11
    __IO uint8_t  RXFUNCADDR11;     // USB Receive Functional Address
                                    // Endpoint 11
         uint8_t  RESERVED28[1];
    __IO uint8_t  RXHUBADDR11;      // USB Receive Hub Address Endpoint 11
    __IO uint8_t  RXHUBPORT11;      // USB Receive Hub Port Endpoint 11
    __IO uint8_t  TXFUNCADDR12;     // USB Transmit Functional Address
                                    // Endpoint 12
         uint8_t  RESERVED29[1];
    __IO uint8_t  TXHUBADDR12;      // USB Transmit Hub Address Endpoint 12
    __IO uint8_t  TXHUBPORT12;      // USB Transmit Hub Port Endpoint 12
    __IO uint8_t  RXFUNCADDR12;     // USB Receive Functional Address
                                    // Endpoint 12
         uint8_t  RESERVED30[1];
    __IO uint8_t  RXHUBADDR12;      // USB Receive Hub Address Endpoint 12
    __IO uint8_t  RXHUBPORT12;      // USB Receive Hub Port Endpoint 12
    __IO uint8_t  TXFUNCADDR13;     // USB Transmit Functional Address
                                    // Endpoint 13
         uint8_t  RESERVED31[1];
    __IO uint8_t  TXHUBADDR13;      // USB Transmit Hub Address Endpoint 13
    __IO uint8_t  TXHUBPORT13;      // USB Transmit Hub Port Endpoint 13
    __IO uint8_t  RXFUNCADDR13;     // USB Receive Functional Address
                                    // Endpoint 13
         uint8_t  RESERVED32[1];
    __IO uint8_t  RXHUBADDR13;      // USB Receive Hub Address Endpoint 13
    __IO uint8_t  RXHUBPORT13;      // USB Receive Hub Port Endpoint 13
    __IO uint8_t  TXFUNCADDR14;     // USB Transmit Functional Address
                                    // Endpoint 14
         uint8_t  RESERVED33[1];
    __IO uint8_t  TXHUBADDR14;      // USB Transmit Hub Address Endpoint 14
    __IO uint8_t  TXHUBPORT14;      // USB Transmit Hub Port Endpoint 14
    __IO uint8_t  RXFUNCADDR14;     // USB Receive Functional Address
                                    // Endpoint 14
         uint8_t  RESERVED34[1];
    __IO uint8_t  RXHUBADDR14;      // USB Receive Hub Address Endpoint 14
    __IO uint8_t  RXHUBPORT14;      // USB Receive Hub Port Endpoint 14
    __IO uint8_t  TXFUNCADDR15;     // USB Transmit Functional Address
                                    // Endpoint 15
         uint8_t  RESERVED35[1];
    __IO uint8_t  TXHUBADDR15;      // USB Transmit Hub Address Endpoint 15
    __IO uint8_t  TXHUBPORT15;      // USB Transmit Hub Port Endpoint 15
    __IO uint8_t  RXFUNCADDR15;     // USB Receive Functional Address
                                    // Endpoint 15
         uint8_t  RESERVED36[1];
    __IO uint8_t  RXHUBADDR15;      // USB Receive Hub Address Endpoint 15
    __IO uint8_t  RXHUBPORT15;      // USB Receive Hub Port Endpoint 15
         uint8_t  RESERVED37[2];
    __O  uint8_t  CSRL0;            // USB Control and Status Endpoint 0 Low
    __O  uint8_t  CSRH0;            // USB Control and Status Endpoint 0 High
         uint8_t  RESERVED38[4];
    __I  uint8_t  COUNT0;           // USB Receive Byte Count Endpoint 0
         uint8_t  RESERVED39[1];
    __IO uint8_t  TYPE0;            // USB Type Endpoint 0
    __IO uint8_t  NAKLMT;           // USB NAK Limit
         uint8_t  RESERVED40[4];
    __IO uint16_t TXMAXP1;          // USB Maximum Transmit Data Endpoint 1
    __IO uint8_t  TXCSRL1;          // USB Transmit Control and Status
                                    // Endpoint 1 Low
    __IO uint8_t  TXCSRH1;          // USB Transmit Control and Status
                                    // Endpoint 1 High
    __IO uint16_t RXMAXP1;          // USB Maximum Receive Data Endpoint 1
    __IO uint8_t  RXCSRL1;          // USB Receive Control and Status
                                    // Endpoint 1 Low
    __IO uint8_t  RXCSRH1;          // USB Receive Control and Status
                                    // Endpoint 1 High
    __I  uint16_t RXCOUNT1;         // USB Receive Byte Count Endpoint 1
    __IO uint8_t  TXTYPE1;          // USB Host Transmit Configure Type
                                    // Endpoint 1
    __IO uint8_t  TXINTERVAL1;      // USB Host Transmit Interval Endpoint 1
    __IO uint8_t  RXTYPE1;          // USB Host Configure Receive Type
                                    // Endpoint 1
    __IO uint8_t  RXINTERVAL1;      // USB Host Receive Polling Interval
                                    // Endpoint 1
         uint8_t  RESERVED41[2];
    __IO uint16_t TXMAXP2;          // USB Maximum Transmit Data Endpoint 2
    __IO uint8_t  TXCSRL2;          // USB Transmit Control and Status
                                    // Endpoint 2 Low
    __IO uint8_t  TXCSRH2;          // USB Transmit Control and Status
                                    // Endpoint 2 High
    __IO uint16_t RXMAXP2;          // USB Maximum Receive Data Endpoint 2
    __IO uint8_t  RXCSRL2;          // USB Receive Control and Status
                                    // Endpoint 2 Low
    __IO uint8_t  RXCSRH2;          // USB Receive Control and Status
                                    // Endpoint 2 High
    __I  uint16_t RXCOUNT2;         // USB Receive Byte Count Endpoint 2
    __IO uint8_t  TXTYPE2;          // USB Host Transmit Configure Type
                                    // Endpoint 2
    __IO uint8_t  TXINTERVAL2;      // USB Host Transmit Interval Endpoint 2
    __IO uint8_t  RXTYPE2;          // USB Host Configure Receive Type
                                    // Endpoint 2
    __IO uint8_t  RXINTERVAL2;      // USB Host Receive Polling Interval
                                    // Endpoint 2
         uint8_t  RESERVED42[2];
    __IO uint16_t TXMAXP3;          // USB Maximum Transmit Data Endpoint 3
    __IO uint8_t  TXCSRL3;          // USB Transmit Control and Status
                                    // Endpoint 3 Low
    __IO uint8_t  TXCSRH3;          // USB Transmit Control and Status
                                    // Endpoint 3 High
    __IO uint16_t RXMAXP3;          // USB Maximum Receive Data Endpoint 3
    __IO uint8_t  RXCSRL3;          // USB Receive Control and Status
                                    // Endpoint 3 Low
    __IO uint8_t  RXCSRH3;          // USB Receive Control and Status
                                    // Endpoint 3 High
    __I  uint16_t RXCOUNT3;         // USB Receive Byte Count Endpoint 3
    __IO uint8_t  TXTYPE3;          // USB Host Transmit Configure Type
                                    // Endpoint 3
    __IO uint8_t  TXINTERVAL3;      // USB Host Transmit Interval Endpoint 3
    __IO uint8_t  RXTYPE3;          // USB Host Configure Receive Type
                                    // Endpoint 3
    __IO uint8_t  RXINTERVAL3;      // USB Host Receive Polling Interval
                                    // Endpoint 3
         uint8_t  RESERVED43[2];
    __IO uint16_t TXMAXP4;          // USB Maximum Transmit Data Endpoint 4
    __IO uint8_t  TXCSRL4;          // USB Transmit Control and Status
                                    // Endpoint 4 Low
    __IO uint8_t  TXCSRH4;          // USB Transmit Control and Status
                                    // Endpoint 4 High
    __IO uint16_t RXMAXP4;          // USB Maximum Receive Data Endpoint 4
    __IO uint8_t  RXCSRL4;          // USB Receive Control and Status
                                    // Endpoint 4 Low
    __IO uint8_t  RXCSRH4;          // USB Receive Control and Status
                                    // Endpoint 4 High
    __I  uint16_t RXCOUNT4;         // USB Receive Byte Count Endpoint 4
    __IO uint8_t  TXTYPE4;          // USB Host Transmit Configure Type
                                    // Endpoint 4
    __IO uint8_t  TXINTERVAL4;      // USB Host Transmit Interval Endpoint 4
    __IO uint8_t  RXTYPE4;          // USB Host Configure Receive Type
                                    // Endpoint 4
    __IO uint8_t  RXINTERVAL4;      // USB Host Receive Polling Interval
                                    // Endpoint 4
         uint8_t  RESERVED44[2];
    __IO uint16_t TXMAXP5;          // USB Maximum Transmit Data Endpoint 5
    __IO uint8_t  TXCSRL5;          // USB Transmit Control and Status
                                    // Endpoint 5 Low
    __IO uint8_t  TXCSRH5;          // USB Transmit Control and Status
                                    // Endpoint 5 High
    __IO uint16_t RXMAXP5;          // USB Maximum Receive Data Endpoint 5
    __IO uint8_t  RXCSRL5;          // USB Receive Control and Status
                                    // Endpoint 5 Low
    __IO uint8_t  RXCSRH5;          // USB Receive Control and Status
                                    // Endpoint 5 High
    __I  uint16_t RXCOUNT5;         // USB Receive Byte Count Endpoint 5
    __IO uint8_t  TXTYPE5;          // USB Host Transmit Configure Type
                                    // Endpoint 5
    __IO uint8_t  TXINTERVAL5;      // USB Host Transmit Interval Endpoint 5
    __IO uint8_t  RXTYPE5;          // USB Host Configure Receive Type
                                    // Endpoint 5
    __IO uint8_t  RXINTERVAL5;      // USB Host Receive Polling Interval
                                    // Endpoint 5
         uint8_t  RESERVED45[2];
    __IO uint16_t TXMAXP6;          // USB Maximum Transmit Data Endpoint 6
    __IO uint8_t  TXCSRL6;          // USB Transmit Control and Status
                                    // Endpoint 6 Low
    __IO uint8_t  TXCSRH6;          // USB Transmit Control and Status
                                    // Endpoint 6 High
    __IO uint16_t RXMAXP6;          // USB Maximum Receive Data Endpoint 6
    __IO uint8_t  RXCSRL6;          // USB Receive Control and Status
                                    // Endpoint 6 Low
    __IO uint8_t  RXCSRH6;          // USB Receive Control and Status
                                    // Endpoint 6 High
    __I  uint16_t RXCOUNT6;         // USB Receive Byte Count Endpoint 6
    __IO uint8_t  TXTYPE6;          // USB Host Transmit Configure Type
                                    // Endpoint 6
    __IO uint8_t  TXINTERVAL6;      // USB Host Transmit Interval Endpoint 6
    __IO uint8_t  RXTYPE6;          // USB Host Configure Receive Type
                                    // Endpoint 6
    __IO uint8_t  RXINTERVAL6;      // USB Host Receive Polling Interval
                                    // Endpoint 6
         uint8_t  RESERVED46[2];
    __IO uint16_t TXMAXP7;          // USB Maximum Transmit Data Endpoint 7
    __IO uint8_t  TXCSRL7;          // USB Transmit Control and Status
                                    // Endpoint 7 Low
    __IO uint8_t  TXCSRH7;          // USB Transmit Control and Status
                                    // Endpoint 7 High
    __IO uint16_t RXMAXP7;          // USB Maximum Receive Data Endpoint 7
    __IO uint8_t  RXCSRL7;          // USB Receive Control and Status
                                    // Endpoint 7 Low
    __IO uint8_t  RXCSRH7;          // USB Receive Control and Status
                                    // Endpoint 7 High
    __I  uint16_t RXCOUNT7;         // USB Receive Byte Count Endpoint 7
    __IO uint8_t  TXTYPE7;          // USB Host Transmit Configure Type
                                    // Endpoint 7
    __IO uint8_t  TXINTERVAL7;      // USB Host Transmit Interval Endpoint 7
    __IO uint8_t  RXTYPE7;          // USB Host Configure Receive Type
                                    // Endpoint 7
    __IO uint8_t  RXINTERVAL7;      // USB Host Receive Polling Interval
                                    // Endpoint 7
         uint8_t  RESERVED47[2];
    __IO uint16_t TXMAXP8;          // USB Maximum Transmit Data Endpoint 8
    __IO uint8_t  TXCSRL8;          // USB Transmit Control and Status
                                    // Endpoint 8 Low
    __IO uint8_t  TXCSRH8;          // USB Transmit Control and Status
                                    // Endpoint 8 High
    __IO uint16_t RXMAXP8;          // USB Maximum Receive Data Endpoint 8
    __IO uint8_t  RXCSRL8;          // USB Receive Control and Status
                                    // Endpoint 8 Low
    __IO uint8_t  RXCSRH8;          // USB Receive Control and Status
                                    // Endpoint 8 High
    __I  uint16_t RXCOUNT8;         // USB Receive Byte Count Endpoint 8
    __IO uint8_t  TXTYPE8;          // USB Host Transmit Configure Type
                                    // Endpoint 8
    __IO uint8_t  TXINTERVAL8;      // USB Host Transmit Interval Endpoint 8
    __IO uint8_t  RXTYPE8;          // USB Host Configure Receive Type
                                    // Endpoint 8
    __IO uint8_t  RXINTERVAL8;      // USB Host Receive Polling Interval
                                    // Endpoint 8
         uint8_t  RESERVED48[2];
    __IO uint16_t TXMAXP9;          // USB Maximum Transmit Data Endpoint 9
    __IO uint8_t  TXCSRL9;          // USB Transmit Control and Status
                                    // Endpoint 9 Low
    __IO uint8_t  TXCSRH9;          // USB Transmit Control and Status
                                    // Endpoint 9 High
    __IO uint16_t RXMAXP9;          // USB Maximum Receive Data Endpoint 9
    __IO uint8_t  RXCSRL9;          // USB Receive Control and Status
                                    // Endpoint 9 Low
    __IO uint8_t  RXCSRH9;          // USB Receive Control and Status
                                    // Endpoint 9 High
    __I  uint16_t RXCOUNT9;         // USB Receive Byte Count Endpoint 9
    __IO uint8_t  TXTYPE9;          // USB Host Transmit Configure Type
                                    // Endpoint 9
    __IO uint8_t  TXINTERVAL9;      // USB Host Transmit Interval Endpoint 9
    __IO uint8_t  RXTYPE9;          // USB Host Configure Receive Type
                                    // Endpoint 9
    __IO uint8_t  RXINTERVAL9;      // USB Host Receive Polling Interval
                                    // Endpoint 9
         uint8_t  RESERVED49[2];
    __IO uint16_t TXMAXP10;         // USB Maximum Transmit Data Endpoint 10
    __IO uint8_t  TXCSRL10;         // USB Transmit Control and Status
                                    // Endpoint 10 Low
    __IO uint8_t  TXCSRH10;         // USB Transmit Control and Status
                                    // Endpoint 10 High
    __IO uint16_t RXMAXP10;         // USB Maximum Receive Data Endpoint 10
    __IO uint8_t  RXCSRL10;         // USB Receive Control and Status
                                    // Endpoint 10 Low
    __IO uint8_t  RXCSRH10;         // USB Receive Control and Status
                                    // Endpoint 10 High
    __I  uint16_t RXCOUNT10;        // USB Receive Byte Count Endpoint 10
    __IO uint8_t  TXTYPE10;         // USB Host Transmit Configure Type
                                    // Endpoint 10
    __IO uint8_t  TXINTERVAL10;     // USB Host Transmit Interval Endpoint 10
    __IO uint8_t  RXTYPE10;         // USB Host Configure Receive Type
                                    // Endpoint 10
    __IO uint8_t  RXINTERVAL10;     // USB Host Receive Polling Interval
                                    // Endpoint 10
         uint8_t  RESERVED50[2];
    __IO uint16_t TXMAXP11;         // USB Maximum Transmit Data Endpoint 11
    __IO uint8_t  TXCSRL11;         // USB Transmit Control and Status
                                    // Endpoint 11 Low
    __IO uint8_t  TXCSRH11;         // USB Transmit Control and Status
                                    // Endpoint 11 High
    __IO uint16_t RXMAXP11;         // USB Maximum Receive Data Endpoint 11
    __IO uint8_t  RXCSRL11;         // USB Receive Control and Status
                                    // Endpoint 11 Low
    __IO uint8_t  RXCSRH11;         // USB Receive Control and Status
                                    // Endpoint 11 High
    __I  uint16_t RXCOUNT11;        // USB Receive Byte Count Endpoint 11
    __IO uint8_t  TXTYPE11;         // USB Host Transmit Configure Type
                                    // Endpoint 11
    __IO uint8_t  TXINTERVAL11;     // USB Host Transmit Interval Endpoint 11
    __IO uint8_t  RXTYPE11;         // USB Host Configure Receive Type
                                    // Endpoint 11
    __IO uint8_t  RXINTERVAL11;     // USB Host Receive Polling Interval
                                    // Endpoint 11
         uint8_t  RESERVED51[2];
    __IO uint16_t TXMAXP12;         // USB Maximum Transmit Data Endpoint 12
    __IO uint8_t  TXCSRL12;         // USB Transmit Control and Status
                                    // Endpoint 12 Low
    __IO uint8_t  TXCSRH12;         // USB Transmit Control and Status
                                    // Endpoint 12 High
    __IO uint16_t RXMAXP12;         // USB Maximum Receive Data Endpoint 12
    __IO uint8_t  RXCSRL12;         // USB Receive Control and Status
                                    // Endpoint 12 Low
    __IO uint8_t  RXCSRH12;         // USB Receive Control and Status
                                    // Endpoint 12 High
    __I  uint16_t RXCOUNT12;        // USB Receive Byte Count Endpoint 12
    __IO uint8_t  TXTYPE12;         // USB Host Transmit Configure Type
                                    // Endpoint 12
    __IO uint8_t  TXINTERVAL12;     // USB Host Transmit Interval Endpoint 12
    __IO uint8_t  RXTYPE12;         // USB Host Configure Receive Type
                                    // Endpoint 12
    __IO uint8_t  RXINTERVAL12;     // USB Host Receive Polling Interval
                                    // Endpoint 12
         uint8_t  RESERVED52[2];
    __IO uint16_t TXMAXP13;         // USB Maximum Transmit Data Endpoint 13
    __IO uint8_t  TXCSRL13;         // USB Transmit Control and Status
                                    // Endpoint 13 Low
    __IO uint8_t  TXCSRH13;         // USB Transmit Control and Status
                                    // Endpoint 13 High
    __IO uint16_t RXMAXP13;         // USB Maximum Receive Data Endpoint 13
    __IO uint8_t  RXCSRL13;         // USB Receive Control and Status
                                    // Endpoint 13 Low
    __IO uint8_t  RXCSRH13;         // USB Receive Control and Status
                                    // Endpoint 13 High
    __I  uint16_t RXCOUNT13;        // USB Receive Byte Count Endpoint 13
    __IO uint8_t  TXTYPE13;         // USB Host Transmit Configure Type
                                    // Endpoint 13
    __IO uint8_t  TXINTERVAL13;     // USB Host Transmit Interval Endpoint 13
    __IO uint8_t  RXTYPE13;         // USB Host Configure Receive Type
                                    // Endpoint 13
    __IO uint8_t  RXINTERVAL13;     // USB Host Receive Polling Interval
                                    // Endpoint 13
         uint8_t  RESERVED53[2];
    __IO uint16_t TXMAXP14;         // USB Maximum Transmit Data Endpoint 14
    __IO uint8_t  TXCSRL14;         // USB Transmit Control and Status
                                    // Endpoint 14 Low
    __IO uint8_t  TXCSRH14;         // USB Transmit Control and Status
                                    // Endpoint 14 High
    __IO uint16_t RXMAXP14;         // USB Maximum Receive Data Endpoint 14
    __IO uint8_t  RXCSRL14;         // USB Receive Control and Status
                                    // Endpoint 14 Low
    __IO uint8_t  RXCSRH14;         // USB Receive Control and Status
                                    // Endpoint 14 High
    __I  uint16_t RXCOUNT14;        // USB Receive Byte Count Endpoint 14
    __IO uint8_t  TXTYPE14;         // USB Host Transmit Configure Type
                                    // Endpoint 14
    __IO uint8_t  TXINTERVAL14;     // USB Host Transmit Interval Endpoint 14
    __IO uint8_t  RXTYPE14;         // USB Host Configure Receive Type
                                    // Endpoint 14
    __IO uint8_t  RXINTERVAL14;     // USB Host Receive Polling Interval
                                    // Endpoint 14
         uint8_t  RESERVED54[2];
    __IO uint16_t TXMAXP15;         // USB Maximum Transmit Data Endpoint 15
    __IO uint8_t  TXCSRL15;         // USB Transmit Control and Status
                                    // Endpoint 15 Low
    __IO uint8_t  TXCSRH15;         // USB Transmit Control and Status
                                    // Endpoint 15 High
    __IO uint16_t RXMAXP15;         // USB Maximum Receive Data Endpoint 15
    __IO uint8_t  RXCSRL15;         // USB Receive Control and Status
                                    // Endpoint 15 Low
    __IO uint8_t  RXCSRH15;         // USB Receive Control and Status
                                    // Endpoint 15 High
    __I  uint16_t RXCOUNT15;        // USB Receive Byte Count Endpoint 15
    __IO uint8_t  TXTYPE15;         // USB Host Transmit Configure Type
                                    // Endpoint 15
    __IO uint8_t  TXINTERVAL15;     // USB Host Transmit Interval Endpoint 15
    __IO uint8_t  RXTYPE15;         // USB Host Configure Receive Type
                                    // Endpoint 15
    __IO uint8_t  RXINTERVAL15;     // USB Host Receive Polling Interval
                                    // Endpoint 15
         uint8_t  RESERVED55[262];
    __IO uint16_t RQPKTCOUNT1;      // USB Request Packet Count in Block
                                    // Transfer Endpoint 1
         uint8_t  RESERVED56[2];
    __IO uint16_t RQPKTCOUNT2;      // USB Request Packet Count in Block
                                    // Transfer Endpoint 2
         uint8_t  RESERVED57[2];
    __IO uint16_t RQPKTCOUNT3;      // USB Request Packet Count in Block
                                    // Transfer Endpoint 3
         uint8_t  RESERVED58[2];
    __IO uint16_t RQPKTCOUNT4;      // USB Request Packet Count in Block
                                    // Transfer Endpoint 4
         uint8_t  RESERVED59[2];
    __IO uint16_t RQPKTCOUNT5;      // USB Request Packet Count in Block
                                    // Transfer Endpoint 5
         uint8_t  RESERVED60[2];
    __IO uint16_t RQPKTCOUNT6;      // USB Request Packet Count in Block
                                    // Transfer Endpoint 6
         uint8_t  RESERVED61[2];
    __IO uint16_t RQPKTCOUNT7;      // USB Request Packet Count in Block
                                    // Transfer Endpoint 7
         uint8_t  RESERVED62[2];
    __IO uint16_t RQPKTCOUNT8;      // USB Request Packet Count in Block
                                    // Transfer Endpoint 8
         uint8_t  RESERVED63[2];
    __IO uint16_t RQPKTCOUNT9;      // USB Request Packet Count in Block
                                    // Transfer Endpoint 9
         uint8_t  RESERVED64[2];
    __IO uint16_t RQPKTCOUNT10;     // USB Request Packet Count in Block
                                    // Transfer Endpoint 10
         uint8_t  RESERVED65[2];
    __IO uint16_t RQPKTCOUNT11;     // USB Request Packet Count in Block
                                    // Transfer Endpoint 11
         uint8_t  RESERVED66[2];
    __IO uint16_t RQPKTCOUNT12;     // USB Request Packet Count in Block
                                    // Transfer Endpoint 12
         uint8_t  RESERVED67[2];
    __IO uint16_t RQPKTCOUNT13;     // USB Request Packet Count in Block
                                    // Transfer Endpoint 13
         uint8_t  RESERVED68[2];
    __IO uint16_t RQPKTCOUNT14;     // USB Request Packet Count in Block
                                    // Transfer Endpoint 14
         uint8_t  RESERVED69[2];
    __IO uint16_t RQPKTCOUNT15;     // USB Request Packet Count in Block
                                    // Transfer Endpoint 15
         uint8_t  RESERVED70[2];
    __IO uint16_t RXDPKTBUFDIS;     // USB Receive Double Packet Buffer Disable
    __IO uint16_t TXDPKTBUFDIS;     // USB Transmit Double Packet Buffer
                                    // Disable
         uint8_t  RESERVED71[188];
    __IO uint32_t EPC;              // USB External Power Control
    __I  uint32_t EPCRIS;           // USB External Power Control Raw Interrupt Status
    __IO uint32_t EPCIM;            // USB External Power Control Interrupt
                                    // Mask
    __IO uint32_t EPCISC;           // USB External Power Control Interrupt
                                    // Status and Clear
    __I  uint32_t DRRIS;            // USB Device Resume Raw Interrupt Status
    __IO uint32_t DRIM;             // USB Device Resume Interrupt Mask
    __O  uint32_t DRISC;            // USB Device Resume Interrupt Status and
                                    // Clear
    __IO uint32_t GPCS;             // USB General-Purpose Control and Status
         uint8_t  RESERVED72[16];
    __IO uint32_t VDC;              // USB VBUS Droop Control
    __I  uint32_t VDCRIS;           // USB VBUS Droop Control Raw Interrupt
                                    // Status
    __IO uint32_t VDCIM;            // USB VBUS Droop Control Interrupt Mask
    __IO uint32_t VDCISC;           // USB VBUS Droop Control Interrupt
                                    // Status and Clear
         uint8_t  RESERVED73[4];
    __I  uint32_t IDVRIS;           // USB ID Valid Detect Raw Interrupt Status
    __IO uint32_t IDVIM;            // USB ID Valid Detect Interrupt Mask
    __IO uint32_t IDVISC;           // USB ID Valid Detect Interrupt
                                    // Status and Clear
    __IO uint32_t DMASEL;           // USB DMA Select
} USB_Type;
// </g>

/*----------       Watchdog Timer (WDT)                           -----------*/
// <g> Watchdog Timer (WDT)
typedef struct
{
    __IO uint32_t LOAD;             // Load register
    __I  uint32_t VALUE;            // Current value register
    __IO uint32_t CTL;              // Control register
    __O  uint32_t ICR;              // Interrupt clear register
    __I  uint32_t RIS;              // Raw interrupt status register
    __I  uint32_t MIS;              // Masked interrupt status register
         uint8_t  RESERVED0[1024];
    __IO uint32_t TEST;             // Test register
         uint8_t  RESERVED1[2020];
    __IO uint32_t LOCK;             // Lock register
} WDT_Type;
// </g>

/******************************************************************************/
/*                              Memory map                                    */
/******************************************************************************/

#define FLASH_BASE          (0x00000000UL)
#define RAM_BASE            (0x20000000UL)
#define PERIPH_BASE         (0x40000000UL)

#define WATCHDOG_BASE           0x40000000  // Watchdog
#define GPIO_PORTA_BASE         0x40004000  // GPIO Port A
#define GPIO_PORTB_BASE         0x40005000  // GPIO Port B
#define GPIO_PORTC_BASE         0x40006000  // GPIO Port C
#define GPIO_PORTD_BASE         0x40007000  // GPIO Port D
#define SSI0_BASE               0x40008000  // SSI0
#define SSI1_BASE               0x40009000  // SSI1
#define UART0_BASE              0x4000C000  // UART0
#define UART1_BASE              0x4000D000  // UART1
#define UART2_BASE              0x4000E000  // UART2
#define I2C0_MASTER_BASE        0x40020000  // I2C0 Master
#define I2C0_SLAVE_BASE         0x40020800  // I2C0 Slave
#define I2C1_MASTER_BASE        0x40021000  // I2C1 Master
#define I2C1_SLAVE_BASE         0x40021800  // I2C1 Slave
#define GPIO_PORTE_BASE         0x40024000  // GPIO Port E
#define GPIO_PORTF_BASE         0x40025000  // GPIO Port F
#define GPIO_PORTG_BASE         0x40026000  // GPIO Port G
#define GPIO_PORTH_BASE         0x40027000  // GPIO Port H
#define PWM_BASE                0x40028000  // PWM
#define PWM_GEN_0_OFFSET        0x00000040  // PWM0 base
#define PWM_GEN_1_OFFSET        0x00000080  // PWM1 base
#define PWM_GEN_2_OFFSET        0x000000C0  // PWM2 base
#define PWM_GEN_3_OFFSET        0x00000100  // PWM3 base
#define QEI0_BASE               0x4002C000  // QEI0
#define QEI1_BASE               0x4002D000  // QEI1
#define TIMER0_BASE             0x40030000  // Timer0
#define TIMER1_BASE             0x40031000  // Timer1
#define TIMER2_BASE             0x40032000  // Timer2
#define TIMER3_BASE             0x40033000  // Timer3
#define ADC_BASE                0x40038000  // ADC
#define COMP_BASE               0x4003C000  // Analog comparators
#define CAN0_BASE               0x40040000  // CAN0
#define CAN1_BASE               0x40041000  // CAN1
#define CAN2_BASE               0x40042000  // CAN2
#define ETH_BASE                0x40048000  // Ethernet
#define MAC_BASE                0x40048000  // Ethernet
#define USB0_BASE               0x40050000  // USB 0 Controller
#define GPIO_PORTA_AHB_BASE     0x40058000  // GPIO Port A (high speed)
#define GPIO_PORTB_AHB_BASE     0x40059000  // GPIO Port B (high speed)
#define GPIO_PORTC_AHB_BASE     0x4005A000  // GPIO Port C (high speed)
#define GPIO_PORTD_AHB_BASE     0x4005B000  // GPIO Port D (high speed)
#define GPIO_PORTE_AHB_BASE     0x4005C000  // GPIO Port E (high speed)
#define GPIO_PORTF_AHB_BASE     0x4005D000  // GPIO Port F (high speed)
#define GPIO_PORTG_AHB_BASE     0x4005E000  // GPIO Port G (high speed)
#define GPIO_PORTH_AHB_BASE     0x4005F000  // GPIO Port H (high speed)
#define HIB_BASE                0x400FC000  // Hibernation Module
#define FLASH_CTRL_BASE         0x400FD000  // FLASH Controller
#define SYSCTL_BASE             0x400FE000  // System Control
#define UDMA_BASE               0x400FF000  // uDMA Controller

/******************************************************************************/
/*                            Peripheral declaration                          */
/******************************************************************************/

#define SYSCTL              ((SYSCTL_Type *)SYSCTL_BASE)

#define GPIOA               ((GPIO_Type *)GPIO_PORTA_BASE)
#define GPIOB               ((GPIO_Type *)GPIO_PORTB_BASE)
#define GPIOC               ((GPIO_Type *)GPIO_PORTC_BASE)
#define GPIOD               ((GPIO_Type *)GPIO_PORTD_BASE)
#define GPIOE               ((GPIO_Type *)GPIO_PORTE_BASE)
#define GPIOF               ((GPIO_Type *)GPIO_PORTF_BASE)
#define GPIOG               ((GPIO_Type *)GPIO_PORTG_BASE)
#define GPIOA_HS            ((GPIO_Type *)GPIO_PORTA_AHB_BASE)
#define GPIOB_HS            ((GPIO_Type *)GPIO_PORTB_AHB_BASE)
#define GPIOC_HS            ((GPIO_Type *)GPIO_PORTC_AHB_BASE)
#define GPIOD_HS            ((GPIO_Type *)GPIO_PORTD_AHB_BASE)
#define GPIOE_HS            ((GPIO_Type *)GPIO_PORTE_AHB_BASE)
#define GPIOF_HS            ((GPIO_Type *)GPIO_PORTF_AHB_BASE)
#define GPIOG_HS            ((GPIO_Type *)GPIO_PORTG_AHB_BASE)

#define TIMER0              ((TIMER_Type *)TIMER0_BASE)
#define TIMER1              ((TIMER_Type *)TIMER1_BASE)
#define TIMER2              ((TIMER_Type *)TIMER2_BASE)
#define TIMER3              ((TIMER_Type *)TIMER3_BASE)

#define ADC                 ((ADC_Type *)ADC_BASE)

#define COMP                ((COMP_Type *)COMP_BASE)

#define CAN0                ((CAN_Type *)CAN0_BASE)
#define CAN1                ((CAN_Type *)CAN1_BASE)
#define CAN2                ((CAN_Type *)CAN2_BASE)

#define ETH                 ((MAC_Type *)ETH_BASE)

#define FLASH_CTRL          ((FLASH_Type *)FLASH_CTRL_BASE)

#define HIB                 ((HIB_Type *)HIB_BASE)

#define I2C0_MASTER         ((I2C_MASTER_Type *)I2C0_MASTER_BASE)
#define I2C0_SLAVE          ((I2C_SLAVE_Type *)I2C0_SLAVE_BASE)
#define I2C1_MASTER         ((I2C_MASTER_Type *)I2C1_MASTER_BASE)
#define I2C1_SLAVE          ((I2C_SLAVE_Type *)I2C1_SLAVE_BASE)

#define PWM                 ((PWM_Type *)PWM_BASE)
#define PWMGEN0             ((PWMGEN_Type *)(PWM_BASE + PWM_GEN_0_OFFSET))
#define PWMGEN1             ((PWMGEN_Type *)(PWM_BASE + PWM_GEN_1_OFFSET))
#define PWMGEN2             ((PWMGEN_Type *)(PWM_BASE + PWM_GEN_2_OFFSET))
#define PWMGEN3             ((PWMGEN_Type *)(PWM_BASE + PWM_GEN_3_OFFSET))

#define QEI0                ((QEI_Type *)QEI0_BASE)
#define QEI1                ((QEI_Type *)QEI1_BASE)

#define SSI0                ((SSI_Type *)SSI0_BASE)
#define SSI1                ((SSI_Type *)SSI1_BASE)

#define UART0               ((UART_Type *)UART0_BASE)
#define UART1               ((UART_Type *)UART1_BASE)
#define UART2               ((UART_Type *)UART2_BASE)

#define UDMA                ((UDMA_Type *)UDMA_BASE)

#define USB0                ((USB_Type *)USB0_BASE)

#define WDT                 ((WDT_Type *)WATCHDOG_BASE)

#endif  /* __LM3_CMSIS_H__ */
