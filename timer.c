#include "frosted.h"

unsigned long timer_val = 0;
 
void TIMER1_IRQHandler(void)
{
    if (Chip_TIMER_MatchPending(LPC_TIMER1, 1)) {
        Chip_TIMER_ClearMatch(LPC_TIMER1, 1);
    }
    Chip_TIMER_Disable(LPC_TIMER1);
    NVIC_DisableIRQ(TIMER1_IRQn);
    jiffies+= timer_val;
    SysTick_on();
}

int Timer_on(unsigned int n)
{
    unsigned long timerFreq = Chip_Clock_GetPeripheralClockRate(SYSCTL_PCLK_TIMER1);
    timer_val = n;
    Chip_TIMER_Reset(LPC_TIMER1);
    Chip_TIMER_MatchEnableInt(LPC_TIMER1, 1);
    Chip_TIMER_SetMatch(LPC_TIMER1, 1, (timerFreq / 1000) * n);
    Chip_TIMER_ResetOnMatchEnable(LPC_TIMER1, 1);
    Chip_TIMER_Enable(LPC_TIMER1);

    NVIC_EnableIRQ(TIMER1_IRQn);
    NVIC_ClearPendingIRQ(TIMER1_IRQn);
    SysTick_off();
    return 0;
}
