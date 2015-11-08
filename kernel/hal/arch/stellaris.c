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
#include "system.h"

const uint32_t NVIC_PRIO_BITS = 3;

/* PLL/Systick Configured values */
const uint32_t SYS_CLOCK = 50000000;
#define RCC_SYSDIV_VAL        (3)
#define RCC_PWMDIV_VAL        (7)
#define RCC_XTAL_VAL          (14)


/* Register values */
#define RCC_DEFAULT  (0x078E3AD1)
#define RCC2_DEFAULT (0x07802810)

/* RCC1 bit pos */
#define RCC_SYSDIV    23
#define RCC_USESYSDIV 22
#define RCC_USEPWMDIV 20
#define RCC_PWMDIV    17
#define RCC_OFF       13
#define RCC_BYPASS    11
#define RCC_XTAL      6
#define RCC_IOSCDIS   1

/* RCC2 bit pos */
#define RCC2_USERRCC2  31
#define RCC2_SYSDIV    23
#define RCC2_OFF       13
#define RCC2_BYPASS    11

/* RIS bit pos */
#define RIS_PLLLRIS    6

const struct hal_iodev UART0 = { 
    .base = 0x4000C000,
    .irqn = 5,
    .clock_id = 0
};


int hal_board_init(void)
{
    uint32_t rcc = RCC_DEFAULT;
    uint32_t rcc2 = RCC2_DEFAULT;

    /* Initials: set default values */

    SET_REG(SYSREG_SC_RCC,  rcc);
    SET_REG(SYSREG_SC_RCC2, rcc2);
    noop();

    /* Stage 1: Reset Oscillators and select configured values */

    rcc = (RCC_SYSDIV_VAL << RCC_SYSDIV) | (RCC_PWMDIV_VAL << RCC_PWMDIV) | (RCC_XTAL_VAL << RCC_XTAL) |
            (1 << RCC_USEPWMDIV);

    rcc2 = (RCC_SYSDIV_VAL << RCC2_SYSDIV); 

    rcc  |= (1 << RCC_BYPASS) | (1 << RCC_OFF);
    rcc2 |= (1 << RCC2_BYPASS) | (1 << RCC2_OFF);

    SET_REG(SYSREG_SC_RCC,  rcc);
    SET_REG(SYSREG_SC_RCC2, rcc2);
    noop();

    /* Stage 2: Power up oscillators */
    rcc  &= ~(1 << RCC_OFF);
    rcc2 &= ~(1 << RCC2_OFF);
    SET_REG(SYSREG_SC_RCC,  rcc);
    SET_REG(SYSREG_SC_RCC2, rcc2);
    noop();

    /* Stage 3: Set USESYSDIV */
    rcc |= (1 << RCC_BYPASS) | (1 << RCC_USESYSDIV);
    SET_REG(SYSREG_SC_RCC,  rcc);
    noop();

    /* Stage 4: Wait for PLL raw interrupt */
    while ((GET_REG((SYSREG_SC_RIS)) & (1 << RIS_PLLLRIS)) == 0)
        noop();

    /* Stage 5: Disable bypass */
    rcc  &= ~(1 << RCC_BYPASS);
    rcc2 &= ~(1 << RCC2_BYPASS);
    SET_REG(SYSREG_SC_RCC,  rcc);
    SET_REG(SYSREG_SC_RCC2, rcc2);
    noop();

}


int hal_iodev_on(struct hal_iodev *iodev)
{
    return 0;
}

int hal_iodev_off(struct hal_iodev *iodev)
{
    return 0;
}


