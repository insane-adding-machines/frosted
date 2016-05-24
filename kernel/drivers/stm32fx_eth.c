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

#include <pico_device.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/syscfg.h>
#include <libopencm3/ethernet/mac.h>
#include <libopencm3/ethernet/phy.h>
#include "libopencm3/cm3/nvic.h"

#define dbg(...)

#define ETH_MAX_FRAME    (1524)                 /* Round to multiple of 4 bytes! */
#define ETH_IRQ_PRIO        (1)

/* FIXME: Put in board config */
#define BOARD_PHY_RMII                          /* Whether the board uses RMII or MII */
#define BOARD_phy_addr      PHY_LAN8710A_ID     /* The PHY ID to be detected on one of the PHY addresses */

/* Some known PHY-identifiers */
#define PHY_KSZ8021_ID    0x00221556
#define PHY_KS8721_ID     0x00221610
#define PHY_DP83848I_ID   0x20005C90
#define PHY_LAN8710A_ID   0x0007C0F1
#define PHY_DM9161_ID     0x0181B8A0
#define PHY_AM79C875_ID   0x00225540
#define PHY_STE101P_ID    0x00061C50

struct dev_eth {
    struct pico_device dev;
    uint32_t rx_prod;
    uint32_t rx_cons;
    uint8_t mac_addr[6];
    uint8_t phy_addr;
};

static struct dev_eth * dev_eth_stm = NULL;
static const uint8_t default_mac[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};

static uint32_t eth_smi_get_phy_divider(void)
{
    uint32_t hclk = rcc_ahb_frequency;

#ifdef STM32F4
    /* CSR Clock between 150-168 MHz */ 
    if (hclk >= 150000000)
        return ETH_MACMIIAR_CR_HCLK_DIV_102;    
#endif

    /* CSR Clock between 100-150 MHz */ 
    if (hclk >= 100000000)
        return ETH_MACMIIAR_CR_HCLK_DIV_62;

    /* CSR Clock between 60-100 MHz */ 
    if (hclk >= 60000000)
        return ETH_MACMIIAR_CR_HCLK_DIV_42;

    /* CSR Clock between 35-60 MHz */ 
    if (hclk >= 35000000)
        return ETH_MACMIIAR_CR_HCLK_DIV_26;

    /* CSR Clock between 20-35 MHz */
    if (hclk >= 20000000)
        return ETH_MACMIIAR_CR_HCLK_DIV_16;

    dbg("STM32_HCLK below minimum frequency for ETH operations (20MHz)\n");
    return 0;
}

static int8_t find_phy(uint32_t clk_div)
{
    uint32_t phy;

    for (phy = 0; phy < 31; phy++)
    {
        ETH_MACMIIDR = (phy << 6) | clk_div;
        if ( (eth_smi_read(phy, PHY_REG_ID1) == (BOARD_phy_addr >> 16)) &&
            ((eth_smi_read(phy, PHY_REG_ID2) & 0xFFF0) == (BOARD_phy_addr & 0xFFF0)) )
            return (int8_t)phy;
    }
    /* PHY not detected */
    return -1;
}

// XXX: Remove extra memcpy
static uint8_t temp_rx_buf[ETH_MAX_FRAME];
static int stm_eth_poll(struct pico_device *dev, int loop_score)
{
    uint32_t rx_len = 0;
    /* add eth_rx_peek, then dynamically alloc + zerocopy */
    while (eth_rx(temp_rx_buf, &rx_len, ETH_MAX_FRAME))
    {
        pico_stack_recv(dev, temp_rx_buf, rx_len);
        rx_len = 0;
        loop_score--;
    }
    return loop_score;
}

static int stm_eth_send(struct pico_device *dev, void * buf, int len)
{
    if (eth_tx(buf, len))
        return len;
    else
        return 0;
}

/* link state -> 0 == DOWN, 1 == UP */
static int stm_eth_link_state(struct pico_device *dev)
{
    struct dev_eth *stm = (struct dev_eth *)dev;
    /* test link state */
    if (phy_link_isup(stm->phy_addr))
        return 1;
    else
        return 0;
}

/**
 * Description: Low level MAC initialization.
 * Parameters:  phy_addr    Pointer a phy_addr variable
 *              clk_div     Phy Clock Divider
 */
static int mac_init(uint8_t * phy_addr, uint32_t clk_div)
{
    int8_t phy_detect;

    /* Enable SYSCFG clock */
    rcc_periph_clock_enable(RCC_SYSCFG);

    /* MAC clocks stopped to configure RMII .*/
    rcc_periph_clock_disable(RCC_ETHMAC);
    rcc_periph_clock_disable(RCC_ETHMACTX);
    rcc_periph_clock_disable(RCC_ETHMACRX);

    /* XXX FIXME */
    #define SYSCFG_PMC_MII_RMII_SEL         ((uint32_t)0x00800000) /*!<Ethernet PHY interface selection */
    #if defined(BOARD_PHY_RMII)
      SYSCFG_PMC |= SYSCFG_PMC_MII_RMII_SEL;
    #else
      SYSCFG_PMC &= ~SYSCFG_PMC_MII_RMII_SEL;
    #endif

    /* Enable RCC clocks: Eth, GPIO */
    rcc_periph_clock_enable(RCC_ETHMAC);
    rcc_periph_clock_enable(RCC_ETHMACTX);
    rcc_periph_clock_enable(RCC_ETHMACRX);

    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_GPIOE);

    rcc_osc_on(RCC_PLL);
    rcc_wait_for_osc_ready(RCC_PLL);

    /* Reset of the MAC core.*/
    rcc_periph_reset_pulse(RST_ETHMAC);

    /* MAC clocks temporary activation.*/
    rcc_periph_clock_enable(RCC_ETHMAC);
    rcc_periph_clock_enable(RCC_ETHMACTX);
    rcc_periph_clock_enable(RCC_ETHMACRX);

    /* PHY address setup.*/
    phy_detect = find_phy(clk_div);
    *phy_addr = (uint8_t)phy_detect;
    if (phy_detect == -1)   /* Detect PHY address */
        return -1;          /* PHY not found */

    /* PHY Soft Reset */
    eth_smi_write(*phy_addr, PHY_REG_BCR, PHY_REG_BCR_RESET);
    while (eth_smi_read(*phy_addr, PHY_REG_BCR) & PHY_REG_BCR_RESET) {};

    /* ETH DMA Soft Reset */
    ETH_DMABMR |= ETH_DMABMR_SR;
    while(ETH_DMABMR & ETH_DMABMR_SR) {};

    return 0;
}

int stm_eth_init(void)
{
    int8_t phy_addr;
    uint8_t * descriptors;
    uint32_t clk_div = eth_smi_get_phy_divider();
    const char ipstr[] = CONFIG_ETH_DEFAULT_IP;
    const char nmstr[] = CONFIG_ETH_DEFAULT_NM;
    const char gwstr[] = CONFIG_ETH_DEFAULT_GW;
    struct pico_ip4 default_ip, default_nm, default_gw, zero;

    pico_string_to_ipv4(ipstr, &default_ip.addr);
    pico_string_to_ipv4(nmstr, &default_nm.addr);
    pico_string_to_ipv4(gwstr, &default_gw.addr);
    

    dev_eth_stm = kalloc(sizeof(struct dev_eth));
    if (!dev_eth_stm)
        return -1;
    memset(dev_eth_stm, 0, sizeof(struct dev_eth));
    mac_init(&dev_eth_stm->phy_addr, clk_div);

    /* DMA soft-reset */
    ETH_DMABMR |= ETH_DMABMR_SR;
    while(ETH_DMABMR & ETH_DMABMR_SR) {};

    eth_init(phy_addr, clk_div); /* does a phy_reset */
    eth_set_mac((uint8_t*)default_mac);

    /* Initialize descriptors */
    /* sizes must be multiple of 4 bytes! buffer must be 4 byte aligned */
    descriptors = kalloc(2 * 2 * ETH_MAX_FRAME + 2 * 16); /* size of buffers + size of descriptors */
    if (!descriptors)
        return -1;
    eth_desc_init(descriptors, 2, 2, ETH_MAX_FRAME, ETH_MAX_FRAME, false);

    /* set pico function pointers */
    dev_eth_stm->dev.poll = stm_eth_poll;
    dev_eth_stm->dev.send = stm_eth_send;
    dev_eth_stm->dev.link_state = stm_eth_link_state;

    if (pico_device_init(&dev_eth_stm->dev,"eth0", default_mac) < 0) {
        kfree(dev_eth_stm);
        return -1;
    }
    /* Set address/netmask */
    pico_ipv4_link_add(&dev_eth_stm->dev, default_ip, default_nm);
    /* Set default gateway */
    if (default_gw.addr)
        pico_ipv4_route_add(zero, zero, default_gw, 1, NULL);

    /* Enabling required interrupt sources.*/
    eth_irq_ack_pending(ETH_DMASR);
    eth_irq_disable(0xFFFF); /* Disable all */
    eth_irq_enable(ETH_DMAIER_NISE | ETH_DMAIER_RIE | ETH_DMAIER_TIE);
    nvic_set_priority(NVIC_ETH_IRQ, ETH_IRQ_PRIO);
    nvic_enable_irq(NVIC_ETH_IRQ);

    eth_start();

    return 0;
}

void eth_isr(void)
{
    uint32_t bits = ETH_DMASR;

    /* Clear all bits */
    eth_irq_ack_pending(ETH_DMASR);

    if (bits & ETH_DMASR_RS)
    {
        /* Receive Status bit set */
        dev_eth_stm->rx_prod++;
    }
}

