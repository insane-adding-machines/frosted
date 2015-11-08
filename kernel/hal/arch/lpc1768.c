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
#include "tools.h"
#include "regs.h"

const uint32_t NVIC_PRIO_BITS = 3;

/* PLL/Systick Configured values */
const uint32_t SYS_CLOCK =  96000000;
#define PLL_FCO (16u)       /* Value for 384 MHz as main PLL clock */
#define CPU_CLOCK_DIV (4u)  /* 384 / (3+1) = 96MHz */

const uint32_t UART0_BASE = 0x4000C000;
const uint32_t UART0_IRQn = 5;

/* PLL bit pos */
#define PLL0_FREQ     (16)  /* PLL0 main freq pos */
#define PLL0_STS_ON   (24)  /* PLL0 enable  pos */
#define PLL0_STS_CONN (25)  /* PLL0 connect pos */
#define PLL0_STS_LOCK (26)  /* PLL0 connect pos */

#define PLLE0         (0)
#define PLLC0         (1)

#define PLL1_FREQ     (5)   /* PLL1 main freq pos */
#define PLL1_STS_ON   (8)   /* PLL1 enable  pos */
#define PLL1_STS_CONN (9)   /* PLL1 connect pos */
#define PLL1_STS_LOCK (10)  /* PLL1 connect pos */

/* SCS bit pos */
#define SCS_OSC_RANGE (4)
#define SCS_OSC_ON    (5)
#define SCS_OSC_STAT  (6)

/* Clock Source value */
#define CLOCK_SOURCE_INT  0
#define CLOCK_SOURCE_MAIN 1
#define CLOCK_SOURCE_RTC  2

/* PLL Kickstart sequence */
#define PLL_KICK0 (0xAAUL)
#define PLL_KICK1 (0x55UL)


int hal_board_init(void)
{
    volatile uint32_t scs;
    volatile uint32_t pll0ctrl;
    volatile uint32_t pll0stat;
    /* Check if PLL0 is already connected, if so, disconnect */
    if (GET_REG(SYSREG_SC_PLL0_STAT) & (1<<PLL0_STS_CONN)) {
        pll0ctrl = GET_REG(SYSREG_SC_PLL0_CTRL);
        pll0ctrl &= ~(1 << PLLE0);
        SET_REG(SYSREG_SC_PLL0_CTRL, pll0ctrl);
    }
    
    /* Check if PLL0 is already ON, if so, disable */
    if (GET_REG(SYSREG_SC_PLL0_STAT) & (1<<PLL0_STS_ON)) {
        pll0ctrl = GET_REG(SYSREG_SC_PLL0_CTRL);
        pll0ctrl &= ~(1 << PLLE0);
        SET_REG(SYSREG_SC_PLL0_CTRL, pll0ctrl);
    }

    /* Check if oscillator is enabled, otherwise enable */
    do {
        scs = GET_REG(SYSREG_SC_SCS);
        scs |= (1 << SCS_OSC_ON);
        SET_REG(SYSREG_SC_SCS, scs);
        noop();
    } while ((scs & (1 << SCS_OSC_STAT)) == 0);

    /* Reset CPU Clock divider */
    SET_REG(SYSREG_SC_CCLKSEL, 0u);

    /* Select Main Oscillator as primary clock source */
    SET_REG(SYSREG_SC_CLKSRCSEL, CLOCK_SOURCE_MAIN);

    /* Set oscillator frequency to 384 MHz */
    SET_REG(SYSREG_SC_PLL0_CONF, (PLL_FCO - 1u));
    SET_REG(SYSREG_SC_PLL0_FEED, PLL_KICK0);
    SET_REG(SYSREG_SC_PLL0_FEED, PLL_KICK1);
    SET_REG(SYSREG_SC_PLL0_CTRL, 0x01);
    SET_REG(SYSREG_SC_PLL0_FEED, PLL_KICK0);
    SET_REG(SYSREG_SC_PLL0_FEED, PLL_KICK1);

    /* Enable oscillator */
    pll0ctrl = GET_REG(SYSREG_SC_PLL0_CTRL);
    pll0ctrl |= (1 << PLLE0);
    SET_REG(SYSREG_SC_PLL0_CTRL, pll0ctrl);
    SET_REG(SYSREG_SC_PLL0_FEED, PLL_KICK0);
    SET_REG(SYSREG_SC_PLL0_FEED, PLL_KICK1);


    /* Set correct divider */
    SET_REG(SYSREG_SC_CCLKSEL, (CPU_CLOCK_DIV - 1u));

    /* Wait for lock */
    while ((GET_REG(SYSREG_SC_PLL0_STAT) & PLL0_STS_LOCK) == 0)
        noop();

    /* Connect oscillator */ 
    pll0ctrl = GET_REG(SYSREG_SC_PLL0_CTRL);
    pll0ctrl |= (1 << PLLC0);
    SET_REG(SYSREG_SC_PLL0_CTRL, pll0ctrl);
    SET_REG(SYSREG_SC_PLL0_FEED, PLL_KICK0);
    SET_REG(SYSREG_SC_PLL0_FEED, PLL_KICK1);


    while ((GET_REG(SYSREG_SC_PLL0_STAT) & PLL0_STS_CONN) == 0)
        noop();

    pll0stat = GET_REG(SYSREG_SC_PLL0_STAT);

    noop();
}
