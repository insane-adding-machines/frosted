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


const uint32_t NVIC_PRIO_BITS = 4;
#if 0
/* Serial port UART0 */
const struct hal_iodev UART0 = { 
    .base = (uint32_t *)0x4000C000,
    .irqn = 5,
    .clock_id = CHIP_CLOCK_UART0
};

/* Serial port UART1 */
const struct hal_iodev UART1 = { 
    .base = (uint32_t *)0x40010000,
    .irqn = 6,
    .clock_id = CHIP_CLOCK_UART1
};

/* Serial port UART3 */
const struct hal_iodev UART3 = { 
    .base = (uint32_t *)0x4009C000,
    .irqn = 8,
    .clock_id = CHIP_CLOCK_UART3
};

/* GPIO controller */
const struct hal_iodev GPIO = { 
    .base = (uint32_t *)0x2009C000,
    .irqn = 0,
    .clock_id = CHIP_CLOCK_GPIO
};
#endif

/* PLL/Systick Configured values */
const uint32_t SYS_CLOCK =  160000000;

#define HSI_POS (0)
#define HSEBYP_POS (18)
#define CR_HSE_CSS_PLL_OFF (0xFEF6FFFF)
#define PLL_RESET_VAL (0x24003010)


int hal_board_init(void)
{
    uint32_t tmp;
    /* Set HSION bit */
    tmp = GET_REG(SYSREG_RCC_CR);
    tmp |= (1 << HSI_POS);
    SET_REG(SYSREG_RCC_CR, tmp);

    /* Reset CFGR register */
    SET_REG(SYSREG_RCC_CFGR, 0u);

    /* Reset HSEON, CSSON and PLLON bits */
    SET_REG(SYSREG_RCC_CR, CR_HSE_CSS_PLL_OFF);

    /* Reset PLLCFGR register */
    SET_REG(SYSREG_RCC_PLLCFGR, PLL_RESET_VAL);

    /* Reset HSEBYP bit */
    tmp = GET_REG(SYSREG_RCC_CR);
    tmp &= ~(1 << HSEBYP_POS);
    SET_REG(SYSREG_RCC_CR, tmp);

    /* Disable all interrupts */
    SET_REG(SYSREG_RCC_CIR, 0u);
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

