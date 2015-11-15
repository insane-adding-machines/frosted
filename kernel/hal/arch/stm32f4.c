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
#include "stm32f4.h"


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

#define RCC_CR_HSI_POS    (0)
#define RCC_CR_HSEON_VAL  (0x00010000)
#define RCC_CR_HSERDY_VAL (0x00020000)
#define RCC_CR_PLLON_VAL  (0x01000000)
#define RCC_CR_PLLRDY_VAL (0x02000000)

#define RCC_CR_RESET_HSE_CSS_PLL_MASK        (0xFEF6FFFF)
#define RCC_CR_RESET_HSE_BYP_MASK            (0xFFFBFFFF)

#define RCC_CFGR_RESET_MASK                  (0xF8FF0000)
#define RCC_CFGR_CLOCKSEL_MASK  (0x00000003)
#define RCC_CFGR_SELECT_PLL     (0x00000002)
#define RCC_CFGR_SWS_MASK       (0x0C)
#define RCC_CFGR_SWS_PLL_USED   (0x08)

#define FLASH_ACR_MASK          (0x0F)
#define FLASH_LATENCY           (5)

#define PLL_CONFIG_VAL          (0x28405408)

#define RCC_PRE0_MASK   (0x000000F0)
#define RCC_PRE1_MASK   (0x00001C00)
#define RCC_PRE2_MASK   (0x0000E000)
#define HWCLOCK_NODIV (0)
#define HWCLOCK_DIV2  (0x1000)
#define HWCLOCK_DIV4  (0x1400)
#define VOLTREG_MODE (0x0000C000)


static void clock_on(uint32_t clk)
{
    uint32_t port = clk / 32;
    uint32_t pin = clk % 32;
    uint32_t temp = GET_REG(SYSREG_RCC_PERIPH_ENABLE(port));
    temp |= (1 << pin);
    SET_REG(SYSREG_RCC_PERIPH_ENABLE(port), temp);
}

#define UPDATE_REG(reg, mask, val) { \
    uint32_t temp_val_reg = GET_REG(reg) & (~mask); \
    temp_val_reg |= val; \
    SET_REG(reg, temp_val_reg); \
    } 

int hal_board_init(void)
{
    uint32_t tmp;
    /* Set HSION bit */
    UPDATE_REG(SYSREG_RCC_CR, 1 << RCC_CR_HSI_POS, 1 << RCC_CR_HSI_POS);

    /* Reset CFGR register flags */
    tmp = GET_REG(SYSREG_RCC_CFGR) & RCC_CFGR_RESET_MASK;
    SET_REG(SYSREG_RCC_CFGR, tmp);

    /* Reset HSEON, CSSON and PLLON bits */
    tmp = GET_REG(SYSREG_RCC_CR) & RCC_CR_RESET_HSE_CSS_PLL_MASK;
    SET_REG(SYSREG_RCC_CR, tmp);

    /* Reset HSEBYP bit */
    tmp = GET_REG(SYSREG_RCC_CR) & RCC_CR_RESET_HSE_BYP_MASK;
    SET_REG(SYSREG_RCC_CR, tmp);

    /* Disable all interrupts */
    SET_REG(SYSREG_RCC_CIR, 0u);
    noop();


    /* Set up Oscillator */
    clock_on(CHIP_CLOCK_PWR);
    SET_REG(SYSREG_PWR_CR, VOLTREG_MODE);

    while (GET_REG(SYSREG_RCC_BDCR) != 0)
        noop();

    /* Enable HSE */
    UPDATE_REG(SYSREG_RCC_CR, RCC_CR_HSEON_VAL, RCC_CR_HSEON_VAL);
    while((GET_REG(SYSREG_RCC_CR) & RCC_CR_HSERDY_VAL) == 0)
        noop();
    
    tmp = 0x28405408; /* TODO: Figure out correct bit mapping, using hardcoded value for now */


    SET_REG(SYSREG_RCC_PLLCFGR, PLL_CONFIG_VAL);

    UPDATE_REG(SYSREG_RCC_CR, RCC_CR_PLLON_VAL, RCC_CR_PLLON_VAL);
    while((GET_REG(SYSREG_RCC_CR) & RCC_CR_PLLRDY_VAL) == 0)
        noop();


    UPDATE_REG(SYSREG_FLASH_ACR, FLASH_ACR_MASK, FLASH_LATENCY);

    UPDATE_REG(SYSREG_RCC_CFGR, RCC_CFGR_CLOCKSEL_MASK, RCC_CFGR_SELECT_PLL);

    while ((GET_REG(SYSREG_RCC_CFGR) & RCC_CFGR_SWS_MASK) != RCC_CFGR_SWS_PLL_USED)
        noop();

    /* Set divisor for HWCLOCK */
    tmp = GET_REG(SYSREG_RCC_CFGR) & (~ RCC_PRE0_MASK);
    tmp |= HWCLOCK_NODIV;
    SET_REG(SYSREG_RCC_CFGR, tmp);

    /* Set divisor for APB1 */
    tmp = GET_REG(SYSREG_RCC_CFGR) & (~ RCC_PRE1_MASK);
    tmp |= HWCLOCK_DIV4;
    SET_REG(SYSREG_RCC_CFGR, tmp);
    
    /* Set divisor for APB2 */
    tmp = GET_REG(SYSREG_RCC_CFGR) & (~ RCC_PRE2_MASK);
    tmp |= HWCLOCK_DIV2 << 3;
    SET_REG(SYSREG_RCC_CFGR, tmp);
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

