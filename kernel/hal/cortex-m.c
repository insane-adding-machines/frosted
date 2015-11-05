/*  
 *      This file is part of frosted.
 *
 *      frosted is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, either version 3 of the License, or
 *      (at your option) any later version.
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
#include "tools.h"
#include "regs.h"
#include "system.h"

extern const uint32_t SYS_CLOCK;


#define TMR_CTL_COUNTFLAG 16
#define TMR_CTL_CLKSOURCE 2
#define TMR_CTL_TICKINT   1
#define TMR_CTL_ENABLE    0

#define AIRCR_KEY           16
#define AIRCR_KEY_WR_VALUE  0x05FA0000
#define AIRCR_KEY_MASK      0xFFFF0000
#define AIRCR_ENDIANESS     15
#define AIRCR_PRIGRP        8
#define AIRCR_PRIGRP_MASK   0x00000700
#define AIRCR_SYSRESETREQ   2


void hal_irqprio_config(void)
{
    uint32_t aircr = GET_REG(SYSREG_AIRCR);
    aircr &= ~(AIRCR_KEY_MASK);
    aircr &= ~(AIRCR_PRIGRP_MASK);
    aircr |= AIRCR_KEY_WR_VALUE;
    aircr |= ((NVIC_PRIO_BITS << AIRCR_PRIGRP) & AIRCR_PRIGRP_MASK);
    SET_REG(SYSREG_AIRCR, aircr);
}


void hal_irq_set_prio(uint32_t irqn, uint32_t prio)
{
    if ((irqn & INT_SYS) == INT_SYS) {
        uint32_t prio_reg_n, prio_reg_pos;
        irqn &= ~INT_SYS;
        prio_reg_n = (irqn - 4) / 4;
        prio_reg_pos = (irqn % 4);
        SET_REG(SYSREG_SHPR(prio_reg_n), (0x0F & (prio << (8 - NVIC_PRIO_BITS))) << (8 * prio_reg_pos));
    } else {
        SET_REG(SYSREG_NVIC_IPRR(irqn), (0x0F & (prio << (8 - NVIC_PRIO_BITS))));
    }
}

void hal_irq_on(uint32_t irqn)
{
    SET_REG(SYSREG_NVIC_ISER(irqn >> 5u), (1u << (irqn & 0x1f)));
}

void hal_irq_off(uint32_t irqn)
{
    SET_REG(SYSREG_NVIC_ICER(irqn >> 5u), (1u << (irqn & 0x1f)));
}

void hal_irq_set_pending(uint32_t irqn)
{
    SET_REG(SYSREG_NVIC_ISPR(irqn >> 5u), (1u << (irqn & 0x1f)));
}

void hal_irq_clear_pending(uint32_t irqn)
{
    SET_REG(SYSREG_NVIC_ICPR(irqn >> 5u), (1u << (irqn & 0x1f)));
}

uint32_t hal_irq_get_pending(uint32_t irqn)
{
    uint32_t ret = (GET_REG(SYSREG_NVIC_ISPR(irqn >> 5u)) & (1u << (irqn & 0x1f)))? 1u : 0u;
    return ret;
}

uint32_t hal_irq_get_active(uint32_t irqn)
{
    uint32_t ret = (GET_REG(SYSREG_NVIC_IABR(irqn >> 5u)) & (1u << (irqn & 0x1f)))? 1u : 0u;
    return ret;
}


#define MAX_TICKS ((1 << 24) - 1)
void hal_systick_config(uint32_t ticks)
{
    ticks--;

    if (ticks > MAX_TICKS)
        while(1);; /* panic */

    /* Set RELOAD value */
    SET_REG(SYSREG_TMR_RVR, ticks);

    /* Set systick interrupt priority */
    hal_irq_set_prio(IRQN_TCK, (1 << NVIC_PRIO_BITS) - 1);

    /* Reset CURRENT value */
    SET_REG(SYSREG_TMR_CVR, 0U);

    /* Set control parameters */
    SET_REG(SYSREG_TMR_CSR, (1 << TMR_CTL_CLKSOURCE) | (1 << TMR_CTL_TICKINT) | (1 << TMR_CTL_ENABLE));
}

void hal_systick_stop(void)
{
    SET_REG(SYSREG_TMR_CSR, 0);
}

