/* ARM Cortex-M interrupt table */

void Reset_Handler(void);               /* Reset vector                 */
void NMI_Handler(void);                 /* NMI Handler                  */
void HardFault_HandlerAsm(void);        /* Hard Fault Handler           */
void MemManage_Handler(void);           /* MPU Fault Handler            */
void BusFault_Handler(void);            /* Bus Fault Handler            */
void UsageFault_Handler(void);          /* Usage Fault Handler          */
void SVC_Handler(void);                 /* SVCall Handler               */
void DebugMon_Handler(void);            /* Debug Monitor Handler        */
void PendSV_Handler(void);              /* PendSV Handler               */
void SysTick_Handler(void);             /* SysTick Handler              */

/* External Interrupts */
void DAC_IRQHandler(void);              /* 16 D/A Converter */
void DMA_IRQHandler(void);              /* 18 General Purpose DMA */
void ETH_IRQHandler(void);              /* 21 Ethernet */
void SDIO_IRQHandler(void);             /* 22 SD/MMC */
void LCD_IRQHandler(void);              /* 23 LCD */
void USB0_IRQHandler(void);             /* 24 USB0*/
void USB1_IRQHandler(void);             /* 25 USB1*/
void SCT_IRQHandler(void);              /* 26 State Configurable Timer*/
void RIT_IRQHandler(void);              /* 27 Repetitive Interrupt Timer*/
void TIMER0_IRQHandler(void);           /* 28 Timer0*/
void TIMER1_IRQHandler(void);           /* 29 Timer1*/
void TIMER2_IRQHandler(void);           /* 30 Timer2*/
void TIMER3_IRQHandler(void);           /* 31 Timer3*/
void MCPWM_IRQHandler(void);            /* 32 Motor Control PWM*/
void ADC0_IRQHandler(void);             /* 33 A/D Converter 0*/
void I2C0_IRQHandler(void);             /* 34 I2C0*/
void I2C1_IRQHandler(void);             /* 35 I2C1*/
void ADC1_IRQHandler(void);             /* 37 A/D Converter 1*/
void SSP0_IRQHandler(void);             /* 38 SSP0*/
void SSP1_IRQHandler(void);             /* 39 SSP1*/
void UART0_IRQHandler(void);            /* 40 UART0*/
void UART1_IRQHandler(void);            /* 41 UART1*/
void UART2_IRQHandler(void);            /* 42 UART2*/
void UART3_IRQHandler(void);            /* 43 UART3*/
void I2S0_IRQHandler(void);             /* 44 I2S*/
void I2S1_IRQHandler(void);             /* 45 AES Engine*/
void SPIFI_IRQHandler(void);            /* 46 SPI Flash Interface*/
void SGPIO_IRQHandler(void);            /* 47 SGPIO*/
void GPIO0_IRQHandler(void);            /* 48 GPIO0*/
void GPIO1_IRQHandler(void);            /* 49 GPIO1*/
void GPIO2_IRQHandler(void);            /* 50 GPIO2*/
void GPIO3_IRQHandler(void);            /* 51 GPIO3*/
void GPIO4_IRQHandler(void);            /* 52 GPIO4*/
void GPIO5_IRQHandler(void);            /* 53 GPIO5*/
void GPIO6_IRQHandler(void);            /* 54 GPIO6*/
void GPIO7_IRQHandler(void);            /* 55 GPIO7*/
void GINT0_IRQHandler(void);            /* 56 GINT0*/
void GINT1_IRQHandler(void);            /* 57 GINT1*/
void EVRT_IRQHandler(void);             /* 58 Event Router*/
void CAN1_IRQHandler(void);             /* 59 C_CAN1*/
void VADC_IRQHandler(void);             /* 61 VADC*/
void ATIMER_IRQHandler(void);           /* 62 ATIMER*/
void RTC_IRQHandler(void);              /* 63 RTC*/
void WDT_IRQHandler(void);              /* 65 WDT*/
void CAN0_IRQHandler(void);             /* 67 C_CAN0*/
void QEI_IRQHandler(void);              /* 68 QEI*/
