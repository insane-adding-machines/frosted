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
 *      Authors: Maxime Vincent
 *
 */
 
#include <stdint.h>
#include "frosted.h"
#include "gpio.h"
#include "eth.h"

#include <pico_device.h>
#include <unicore-mx/ethernet/mac.h>
#include <unicore-mx/ethernet/phy.h>
#include "unicore-mx/cm3/nvic.h"

#define dbg(...)

#define ETH_MAX_FRAME    (1524)                 /* Round to multiple of 4 bytes! */
#define ETH_IRQ_PRIO        (1)
#define ETH_IRQMASK_RX (1)                      /* Interrupt Mask for RXINT (bit 0) */

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

static struct module mod_eth = {
    .family = FAMILY_DEV,
    .name = "ethernet",
};

static struct dev_eth * dev_eth = NULL;
static const uint8_t default_mac[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};

static uint8_t temp_rx_buf[ETH_MAX_FRAME];

static int eth_poll(struct pico_device *dev, int loop_score)
{
    uint32_t rx_len = 0;
    pico_lock();
    while (eth_rx(temp_rx_buf, &rx_len, ETH_MAX_FRAME) && (loop_score > 0))
    {
        pico_stack_recv(dev, temp_rx_buf, rx_len);
        rx_len = 0;
        loop_score--;
    }
    pico_unlock();
    return loop_score;
}

static int eth_send(struct pico_device *dev, void * buf, int len)
{
    if (eth_tx(buf, len))
        return len;
    else
        return 0;
}

int pico_eth_start(void)
{
    const char ipstr[] = CONFIG_ETH_DEFAULT_IP;
    const char nmstr[] = CONFIG_ETH_DEFAULT_NM;
    const char gwstr[] = CONFIG_ETH_DEFAULT_GW;
    struct pico_ip4 default_ip, default_nm, default_gw, zero;


    pico_string_to_ipv4(ipstr, &default_ip.addr);
    pico_string_to_ipv4(nmstr, &default_nm.addr);
    pico_string_to_ipv4(gwstr, &default_gw.addr);
    

    dev_eth = kalloc(sizeof(struct dev_eth));
    if (!dev_eth)
        return -1;
    memset(dev_eth, 0, sizeof(struct dev_eth));

    /* set pico function pointers */
    dev_eth->dev.poll = eth_poll;
    dev_eth->dev.send = eth_send;

    if (pico_device_init(&dev_eth->dev,"eth0", default_mac) < 0) {
        kfree(dev_eth);
        return -1;
    }
    /* Set address/netmask */
    pico_ipv4_link_add(&dev_eth->dev, default_ip, default_nm);
    /* Set default gateway */
    if (default_gw.addr)
        pico_ipv4_route_add(zero, zero, default_gw, 1, NULL);

    /* Enabling required interrupt sources.*/

    eth_start();
    return 0;

}

void eth_isr(void)
{
    eth_irq_ack_pending(ETH_IRQMASK_RX);
    frosted_tcpip_wakeup();
}

/* HW initialization */
int ethernet_init(const struct eth_config *conf)
{
    (void)conf;
    eth_init(0, ETH_CLK_50MHZ); /* does a phy_reset */
    nvic_enable_irq(NVIC_ETH_IRQ);
    eth_irq_enable(ETH_IRQMASK_RX);
    eth_set_mac((uint8_t*)default_mac);
    return 0;
}
