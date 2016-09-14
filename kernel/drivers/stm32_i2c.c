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

#include "frosted.h"
#include <unicore-mx/cm3/nvic.h>

#include <unicore-mx/stm32/dma.h>
#include "unicore-mx/stm32/i2c.h"
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/gpio.h>
#include "device.h"
#include <stdint.h>
#include "cirbuf.h"
#include "dma.h"
#include "i2c.h"
#include "locks.h"

/* Dummy module, for gpio claiming only. */
static struct module mod_i2c = {
    .family = FAMILY_DEV,
    .name = "i2c"
};

typedef enum
{
    I2C_STATE_READY,
    I2C_STATE_ADDRESS,
    I2C_STATE_REGISTER,
    I2C_STATE_READ,
    I2C_STATE_READ_ADDRESS,
    I2C_STATE_READ_DATA,
    I2C_STATE_DATA,
    I2C_STATE_DMA_COMPLETE,
    I2C_STATE_BTF,
    I2C_STATE_ERROR,
}I2C_STATE_t;

typedef enum
{
    I2C_STIM_START,
    I2C_STIM_TIMEOUT,
    I2C_STIM_MASTER_MODE_SELECT,
    I2C_STIM_MASTER_MODE_ADDRESS10,
    I2C_STIM_MASTER_TRANSMITTER_MODE_SELECTED,
    I2C_STIM_MASTER_RECEIVER_MODE_SELECTED,
    I2C_STIM_DMA_COMPLETE_TX,
    I2C_STIM_DMA_COMPLETE_RX,
    I2C_STIM_BTF,
    I2C_STIM_TXE,
}I2C_STIM_t;

struct dev_i2c {
    struct device * dev;
    uint32_t base;
    void (*isr)(struct i2c_slave *sl);
    struct i2c_slave *sl;
    uint32_t ev_irq;
    uint32_t er_irq;
    uint8_t slv_register;
    const struct dma_config * tx_dma_config;
    const struct dma_config * rx_dma_config;
    mutex_t *mutex;

    uint8_t dirn;
    I2C_STATE_t state;
};

#define MAX_I2CS 4

static struct dev_i2c *DEV_I2C[MAX_I2CS] = { };

static void state_machine(struct dev_i2c *i2c, I2C_STIM_t stim);

/*****************************
    MASTER_MODE_SELECT                                   SR2: BUSY|MSL              SR1: SB
    MASTER_MODE_ADDRESS10                           SR2: BUSY|MSL               SR1: ADD10
    MASTER_TRANSMITTER_MODE_SELECTED     SR2: BUSY|MSL|TRA      SR1: TXE|ADDR
    MASTER_RECEIVER_MODE_SELECTED           SR2: BUSY|MSL               SR1: ADDR
*****************************/
static void i2c_ev(struct dev_i2c * i2c)
{
    /* MASTER_MODE_SELECT */
    if(    ((I2C_SR1(i2c->base) & (I2C_SR1_SB)) == (I2C_SR1_SB)) &&
            ((I2C_SR2(i2c->base) & (I2C_SR2_BUSY | I2C_SR2_MSL)) == (I2C_SR2_BUSY | I2C_SR2_MSL))   )
        state_machine(i2c, I2C_STIM_MASTER_MODE_SELECT);
    /* MASTER_MODE_ADDRESS10 */
    else if(    ((I2C_SR1(i2c->base) & (I2C_SR1_ADD10)) == (I2C_SR1_ADD10)) &&
            ((I2C_SR2(i2c->base) & (I2C_SR2_BUSY | I2C_SR2_MSL)) == (I2C_SR2_BUSY | I2C_SR2_MSL))   )
        state_machine(i2c, I2C_STIM_MASTER_MODE_ADDRESS10);
    /* MASTER_TRANSMITTER_MODE_SELECTED */
    else if(    ((I2C_SR1(i2c->base) & (I2C_SR1_TxE | I2C_SR1_ADDR)) == (I2C_SR1_TxE | I2C_SR1_ADDR)) &&
            ((I2C_SR2(i2c->base) & (I2C_SR2_BUSY | I2C_SR2_MSL | I2C_SR2_TRA)) == (I2C_SR2_BUSY | I2C_SR2_MSL | I2C_SR2_TRA))   )
        state_machine(i2c, I2C_STIM_MASTER_TRANSMITTER_MODE_SELECTED);
    /* MASTER_RECEIVER_MODE_SELECTED */
    else if(    ((I2C_SR1(i2c->base) & (I2C_SR1_ADDR)) == (I2C_SR1_ADDR)) &&
            ((I2C_SR2(i2c->base) & (I2C_SR2_BUSY | I2C_SR2_MSL)) == (I2C_SR2_BUSY | I2C_SR2_MSL))   )
        state_machine(i2c, I2C_STIM_MASTER_RECEIVER_MODE_SELECTED);
    else if(    (I2C_SR1(i2c->base) & (I2C_SR1_BTF)) == (I2C_SR1_BTF)   )
        state_machine(i2c, I2C_STIM_BTF);
    else if(    (I2C_SR1(i2c->base) & (I2C_SR1_TxE)) == (I2C_SR1_TxE)   )
        state_machine(i2c, I2C_STIM_TXE);

}


/*****************************
    BERR             I2C_SR1_BERR
    AF                  I2C_SR1_AF
    OVR               I2C_SR1_OVR
    PECERR          I2C_SR1_PECERR
    TIMEOUT        I2C_SR1_TIMEOUT
    SMBALERT    I2C_SR1_SMBALERT
*****************************/
static void i2c_er(struct dev_i2c * i2c)
{
    if (!i2c)
        return;
    if(I2C_SR1(i2c->base) & (I2C_SR1_TIMEOUT) == (I2C_SR1_TIMEOUT))
        state_machine(i2c, I2C_STIM_TIMEOUT);
}

static void i2c_rx_dma_complete(struct dev_i2c * i2c)
{
    if (!i2c)
        return;
    state_machine(i2c, I2C_STIM_DMA_COMPLETE_RX);
}
static void i2c_tx_dma_complete(struct dev_i2c * i2c)
{
    if (!i2c)
        return;
    state_machine(i2c, I2C_STIM_DMA_COMPLETE_TX);
}

#ifdef CONFIG_I2C1
void i2c1_ev_isr(void)
{
    i2c_ev(DEV_I2C[1]);
}
void i2c1_er_isr(void)
{
    i2c_er(DEV_I2C[1]);
}
void dma1_stream0_isr()
{
    dma_clear_interrupt_flags(DMA1, DMA_STREAM0, DMA_LISR_TCIF0);
    i2c_rx_dma_complete(DEV_I2C[1]);
}
void dma1_stream6_isr()
{
    dma_clear_interrupt_flags(DMA1, DMA_STREAM6, DMA_LISR_TCIF0);
    i2c_tx_dma_complete(DEV_I2C[1]);
}
#endif


static void restart_state_machine(struct dev_i2c *i2c)
{
    mutex_unlock(i2c->mutex);
    i2c->state = I2C_STATE_READY;
}

static void state_machine(struct dev_i2c *i2c, I2C_STIM_t stim)
{
    volatile uint16_t cr1;
    volatile uint16_t cr2;


    switch(i2c->state)
    {
        case I2C_STATE_READY:
            i2c_peripheral_enable(i2c->base);
            i2c_enable_interrupt(i2c->base, I2C_CR2_ITEVTEN | I2C_CR2_ITERREN);
            i2c->state = I2C_STATE_ADDRESS;
            i2c_set_dma_last_transfer(i2c->base);
            i2c_send_start(i2c->base);
            break;

        case I2C_STATE_ADDRESS:
            switch(stim)
            {
                case  I2C_STIM_TIMEOUT:
                    restart_state_machine(i2c);
                    break;
                case I2C_STIM_MASTER_MODE_SELECT:
                    i2c->state = I2C_STATE_REGISTER;
                    i2c_send_7bit_address(i2c->base, i2c->sl->address >> 1, 0);
                    break;
            }
            break;

        case I2C_STATE_REGISTER:
            switch(stim)
                {
                case  I2C_STIM_TIMEOUT:
                    restart_state_machine(i2c);
                    break;
                case I2C_STIM_MASTER_TRANSMITTER_MODE_SELECTED:
                    if(i2c->dirn)
                    {
                        i2c->state = I2C_STATE_READ;
                    }
                    else
                    {
                        i2c->state = I2C_STATE_DATA;
                    }
                    I2C_DR(i2c->base) = i2c->slv_register;
                    break;
                }
            break;

        case I2C_STATE_READ:
            switch(stim)
            {
                case  I2C_STIM_TIMEOUT:
                    restart_state_machine(i2c);
                    break;
                case I2C_STIM_TXE:
                    i2c->state = I2C_STATE_READ_ADDRESS;
                    i2c_send_start(i2c->base);
                    break;
            }
            break;

        case I2C_STATE_READ_ADDRESS:
            switch(stim)
            {
                case  I2C_STIM_TIMEOUT:
                    restart_state_machine(i2c);
                    break;
                case I2C_STIM_MASTER_MODE_SELECT:
                    i2c->state = I2C_STATE_READ_DATA;
                    i2c_send_7bit_address(i2c->base, i2c->sl->address >> 1, 1);
                    break;
            }
            break;

        case I2C_STATE_READ_DATA:
            switch(stim)
            {
                case  I2C_STIM_TIMEOUT:
                    restart_state_machine(i2c);
                    break;
                case I2C_STIM_MASTER_RECEIVER_MODE_SELECTED:
                    i2c->state = I2C_STATE_DMA_COMPLETE;
                    i2c_enable_dma(i2c->base);
                    break;
            }
            break;

        case I2C_STATE_DATA:
            switch(stim)
            {
                case  I2C_STIM_TIMEOUT:
                    restart_state_machine(i2c);
                    break;
                case I2C_STIM_TXE:
                    i2c->state = I2C_STATE_DMA_COMPLETE;
                    i2c_enable_dma(i2c->base);
                    break;
            }
            break;

        case I2C_STATE_DMA_COMPLETE:
            switch(stim)
            {
                case  I2C_STIM_TIMEOUT:
                    restart_state_machine(i2c);
                    break;
                case I2C_STIM_DMA_COMPLETE_RX:
                    i2c_disable_dma(i2c->base);
                    i2c_send_stop(i2c->base);
                    i2c_peripheral_disable(i2c->base);
                    i2c->isr(i2c->sl);
                    restart_state_machine(i2c);
                    break;
                case I2C_STIM_DMA_COMPLETE_TX:
                    i2c_disable_dma(i2c->base);
                    i2c->state = I2C_STATE_BTF;
                    break;
            }
            break;

        case I2C_STATE_BTF:
            switch(stim)
            {
                case  I2C_STIM_TIMEOUT:
                    restart_state_machine(i2c);
                    break;
                case I2C_STIM_BTF:
                    i2c_send_stop(i2c->base);
                    i2c_peripheral_disable(i2c->base);
                    i2c->isr(i2c->sl);
                    restart_state_machine(i2c);
                    break;
            }
            break;
        case I2C_STATE_ERROR:
            restart_state_machine(i2c);
            break;
    }
}

int i2c_init_read(struct i2c_slave *sl, uint8_t reg, uint8_t *buf, uint32_t len)
{
    struct dev_i2c *i2c;

    if (len <= 0)
        return len;

    i2c = DEV_I2C[sl->bus];
    if (!i2c)
        return -ENOENT;

    if (mutex_trylock(i2c->dev->mutex) < 0)
        return -EBUSY;

    i2c->dirn = 1;
    i2c->slv_register = reg;
    i2c->isr = sl->isr_rx;
    i2c->sl = sl;
    init_dma(i2c->rx_dma_config, (uint32_t)buf, len);
    dma_enable_transfer_complete_interrupt(i2c->rx_dma_config->base, i2c->rx_dma_config->stream);
    nvic_set_priority(i2c->rx_dma_config->irq, 1);
    nvic_enable_irq(i2c->rx_dma_config->irq);
    state_machine(i2c, I2C_STIM_START);
    return 0;
}

int i2c_init_write(struct i2c_slave *sl, uint8_t reg, const uint8_t *buf, uint32_t len)
{
    struct dev_i2c *i2c;

    if (len <= 0)
        return len;

    i2c = DEV_I2C[sl->bus];
    if (!i2c)
        return -ENOENT;

    if (mutex_trylock(i2c->dev->mutex) < 0)
        return -EBUSY;

    i2c->dirn = 0;
    i2c->slv_register = reg;
    i2c->isr = sl->isr_tx;
    i2c->sl = sl;
    init_dma(i2c->tx_dma_config, (uint32_t)buf, len);
    dma_enable_transfer_complete_interrupt(i2c->tx_dma_config->base, i2c->tx_dma_config->stream);
    nvic_set_priority(i2c->tx_dma_config->irq, 1);
    nvic_enable_irq(i2c->tx_dma_config->irq);
    state_machine(i2c, I2C_STIM_START);
    return 0;
}

static int i2c_fno_init(const struct i2c_config *conf, struct dev_i2c *i)
{
    struct fnode *devfs = fno_search("/dev");
    char name[5] = "i2cX";
    if (!devfs)
        return -EFAULT;

    name[3] = '0' + conf->idx;

}

int i2c_create(const struct i2c_config *conf)
{
    struct dev_i2c *i2c = DEV_I2C[conf->idx];

    if (i2c)
        return -EBUSY;
    if (!conf)
        return -EINVAL;
    if (conf->base == 0)
        return -EINVAL;

    i2c = kalloc(sizeof(struct dev_i2c));
    if (!i2c)
        return -ENOMEM;

    gpio_create(&mod_i2c, &conf->pio_sda);
    gpio_create(&mod_i2c, &conf->pio_scl);

    memset(i2c, 0, sizeof(struct dev_i2c));
    rcc_periph_clock_enable(conf->rcc);
    rcc_periph_clock_enable(conf->dma_rcc);



    i2c_peripheral_disable(conf->base);
    i2c_reset(conf->base);


    i2c_set_clock_frequency(conf->base, conf->clock_f);
    if(conf->fast_mode)
        i2c_set_fast_mode(conf->base);
    else
        i2c_set_standard_mode(conf->base);

    i2c_set_trise(conf->base, conf->rise_time);
    i2c_set_ccr(conf->base, conf->bus_clk_frequency);

    i2c_nack_current(conf->base);
    i2c_disable_ack(conf->base);
    i2c_clear_stop(conf->base);

    /* Set up device struct */
    i2c->base = conf->base;
    i2c->ev_irq = conf->ev_irq;
    i2c->er_irq = conf->er_irq;
    i2c->tx_dma_config = &conf->tx_dma;
    i2c->rx_dma_config = &conf->rx_dma;
    i2c->state = I2C_STATE_READY;
    i2c->mutex = mutex_init();

    nvic_set_priority(conf->ev_irq, 1);
    nvic_enable_irq(conf->ev_irq);
    nvic_set_priority(conf->er_irq, 1);
    nvic_enable_irq(conf->er_irq);
    return 0;
}
