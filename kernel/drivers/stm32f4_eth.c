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
 *      Authors:
 *
 */
 
#include <stdint.h>
#include "frosted.h"
#include "gpio.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/ethernet/mac.h>
#include <libopencm3/ethernet/phy.h>

const struct gpio_addr gpio_eth[] = {
    {.base=GPIOA, .pin=GPIO2, .mode=GPIO_MODE_OUTPUT, .optype=GPIO_OTYPE_PP, .name=NULL}, // MDIO
    {.base=GPIOC, .pin=GPIO1, .mode=GPIO_MODE_OUTPUT, .optype=GPIO_OTYPE_PP, .name=NULL}, // MDC
    {.base=GPIOA, .pin=GPIO1, .mode=GPIO_MODE_INPUT, .name=NULL},                         // RMII REF CLK IN
    {.base=GPIOA, .pin=GPIO7, .mode=GPIO_MODE_INPUT, .name=NULL},                         // RMII CRS DV
    {.base=GPIOB, .pin=GPIO10,.mode=GPIO_MODE_INPUT, .name=NULL},                         // RMII RXER
    {.base=GPIOC, .pin=GPIO4, .mode=GPIO_MODE_INPUT, .name=NULL},                         // RMII RXD0
    {.base=GPIOC, .pin=GPIO5, .mode=GPIO_MODE_INPUT, .name=NULL},                         // RMII RXD1
    {.base=GPIOC, .pin=GPIO5, .mode=GPIO_MODE_INPUT, .name=NULL},                         // RMII RXD1
    {.base=GPIOB, .pin=GPIO11,.mode=GPIO_MODE_OUTPUT, .optype=GPIO_OTYPE_PP, .name=NULL}, // RMII TXEN
    {.base=GPIOB, .pin=GPIO12,.mode=GPIO_MODE_OUTPUT, .optype=GPIO_OTYPE_PP, .name=NULL}, // RMII TXD0
    {.base=GPIOB, .pin=GPIO13,.mode=GPIO_MODE_OUTPUT, .optype=GPIO_OTYPE_PP, .name=NULL}, // RMII TXD1
    {.base=GPIOA, .pin=GPIO8, .mode=GPIO_MODE_OUTPUT, .optype=GPIO_OTYPE_PP, .name=NULL}, // MCO out
    {.base=GPIOC, .pin=GPIO0, .mode=GPIO_MODE_OUTPUT, .optype=GPIO_OTYPE_PP, .name=NULL}, // PHI RESET
};


void stm32f4_eth_init(void)
{
    gpio_init(NULL, gpio_eth, sizeof(gpio_eth) / sizeof(struct gpio_addr));
	rcc_periph_clock_enable(RCC_ETHMAC);
	rcc_periph_clock_enable(RCC_ETHMACTX);
	rcc_periph_clock_enable(RCC_ETHMACRX);


	gpio_set(GPIOC, GPIO0);
	//rcc_set_pll3_multiplication_factor(RCC_CFGR_PLLMUL_PLL_CLK_MUL10);
	rcc_osc_on(RCC_PLL);
	rcc_wait_for_osc_ready(RCC_PLL);
	rcc_set_mco(RCC_CFGR_MCO_PLL3);
	rcc_periph_reset_pulse(RST_ETHMAC);
	phy_hard_reset();
	eth_init(3);
	eth_set_mac(1, mymac);




}
