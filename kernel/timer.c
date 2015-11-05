/*  
 *      This file is part of frosted.
 *
 *      frosted is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License version 2, as 
 *      published by the Free Software Foundation.
 *      
 *
 *      frosted is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with frosted.  If not, see <http://www.gnu.org/licenses/>.
 *
 *      Authors: Daniele Lacamera, Maxime Vincent
 *
 */  
#include "frosted.h"

unsigned long timer_val = 0;
 
void TIMER1_IRQHandler(void)
{
#if 0
    if (Chip_TIMER_MatchPending(LPC_TIMER1, 1)) {
        Chip_TIMER_ClearMatch(LPC_TIMER1, 1);
    }
    Chip_TIMER_Disable(LPC_TIMER1);
    NVIC_DisableIRQ(TIMER1_IRQn);
    jiffies+= timer_val;
    SysTick_on();
#endif
}

int Timer_on(unsigned int n)
{
#if 0
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
#endif
    return 0;
}
