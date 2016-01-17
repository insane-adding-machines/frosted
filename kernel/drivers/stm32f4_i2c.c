#include "frosted.h"
#include "device.h"
#include <stdint.h>
#include "cirbuf.h"
#include "stm32f4_dma.h"
#include "i2c.h"
#include <libopencm3/cm3/nvic.h>

#include <libopencm3/stm32/dma.h>
#include "libopencm3/stm32/i2c.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

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
    i2c_completion completion_fn;
    void * completion_arg;
    uint32_t ev_irq;
    uint32_t er_irq;
    uint8_t slv_address;
    uint8_t slv_register;
    const struct dma_setup * tx_dma_setup;
    const struct dma_setup * rx_dma_setup;

    uint8_t dirn;
    I2C_STATE_t state;
};

#define MAX_I2CS 4

static struct dev_i2c DEV_I2C[MAX_I2CS];

static struct module mod_devi2c = {
    .family = FAMILY_FILE,
    .name = "i2c",
    .ops.open = device_open,
};

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
    if(I2C_SR1(i2c->base) & (I2C_SR1_TIMEOUT) == (I2C_SR1_TIMEOUT)) state_machine(i2c, I2C_STIM_TIMEOUT);
}

static void i2c_rx_dma_complete(struct dev_i2c * i2c)
{
    state_machine(i2c, I2C_STIM_DMA_COMPLETE_RX);
}
static void i2c_tx_dma_complete(struct dev_i2c * i2c)
{
    state_machine(i2c, I2C_STIM_DMA_COMPLETE_TX);
}

#ifdef CONFIG_I2C_1
void i2c1_ev_isr(void)
{
    i2c_ev(&DEV_I2C[0]);
}
void i2c1_er_isr(void)
{
    i2c_er(&DEV_I2C[0]);
}
void dma1_stream0_isr()
{
    dma_clear_interrupt_flags(DMA1, DMA_STREAM0, DMA_LISR_TCIF0);
    i2c_rx_dma_complete(&DEV_I2C[0]);  
}
void dma1_stream6_isr()
{
    dma_clear_interrupt_flags(DMA1, DMA_STREAM6, DMA_LISR_TCIF0);
    i2c_tx_dma_complete(&DEV_I2C[0]);  
}
#endif

static void state_machine(struct dev_i2c *i2c, I2C_STIM_t stim)
{
    volatile uint16_t cr1;
    volatile uint16_t cr2;


    switch(i2c->state)
    {
        case I2C_STATE_READY:
            i2c_peripheral_enable(i2c->base);
            i2c_enable_interrupt(i2c->base, I2C_CR2_ITEVTEN | I2C_CR2_ITERREN | I2C_CR2_ITBUFEN);
            i2c->state = I2C_STATE_ADDRESS;
            i2c_set_dma_last_transfer(i2c->base);
            i2c_send_start(i2c->base);
            break;

        case I2C_STATE_ADDRESS:
            switch(stim)
            {
                case  I2C_STIM_TIMEOUT:
                    break;
                case I2C_STIM_MASTER_MODE_SELECT:
                    i2c->state = I2C_STATE_REGISTER;
                    i2c_send_7bit_address(i2c->base, i2c->slv_address >> 1, 0); 
                    break;
            }
            break;

        case I2C_STATE_REGISTER:
            switch(stim)
                {
                case  I2C_STIM_TIMEOUT:
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
                    break;
                case I2C_STIM_MASTER_MODE_SELECT:
                    i2c->state = I2C_STATE_READ_DATA;
                    i2c_send_7bit_address(i2c->base, i2c->slv_address >> 1, 1); 
                    break;
            }
            break;

        case I2C_STATE_READ_DATA:
            switch(stim)
            {
                case  I2C_STIM_TIMEOUT:
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
                    break;
                case I2C_STIM_DMA_COMPLETE_RX:
                    i2c_disable_dma(i2c->base);
                    i2c_send_stop(i2c->base);
                    i2c_peripheral_disable(i2c->base);
                    i2c->state = I2C_STATE_READY;
                    tasklet_add(i2c->completion_fn, i2c->completion_arg);
//                    i2c->completion_fn(i2c->completion_arg, 0);
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
                    break;
                case I2C_STIM_BTF:
                    i2c_send_stop(i2c->base);
                    i2c_peripheral_disable(i2c->base);
                    i2c->state = I2C_STATE_READY;
                    tasklet_add(i2c->completion_fn, i2c->completion_arg);
//                    i2c->completion_fn(i2c->completion_arg, 0);
                    break;
            }
            break;
        case I2C_STATE_ERROR:

            break;
    }

}

int i2c_read(struct fnode *fno, i2c_completion completion_fn, void * completion_arg, uint8_t address, uint8_t  slv_register, uint8_t *buf, uint32_t len)
{
    struct dev_i2c *i2c;
    
    if (len <= 0)
        return len;
    
    i2c = (struct dev_i2c *)FNO_MOD_PRIV(fno, &mod_devi2c);
    if (i2c == NULL)
        return -1;

    frosted_mutex_lock(i2c->dev->mutex);
   
    i2c->dirn = 1;
    i2c->slv_address = address;
    i2c->slv_register = slv_register;
    i2c->completion_fn = completion_fn;
    i2c->completion_arg = completion_arg;
    
    init_dma(i2c->rx_dma_setup, (uint32_t)buf, len);
    dma_enable_transfer_complete_interrupt(i2c->rx_dma_setup->base, i2c->rx_dma_setup->stream);
    nvic_set_priority(i2c->rx_dma_setup->irq, 1);
    nvic_enable_irq(i2c->rx_dma_setup->irq);

    state_machine(i2c, I2C_STIM_START);
    return 0;
}

int i2c_write(struct fnode *fno, i2c_completion completion_fn, void * completion_arg, uint8_t address, uint8_t  slv_register, const uint8_t *buf, uint32_t len)
{
    struct dev_i2c *i2c;
    
    if (len <= 0)
        return len;
    
    i2c = (struct dev_i2c *)FNO_MOD_PRIV(fno, &mod_devi2c);
    if (i2c == NULL)
        return -1;

    frosted_mutex_lock(i2c->dev->mutex);

    i2c->dirn = 0;
    i2c->slv_address = address;
    i2c->slv_register = slv_register;
    i2c->completion_fn = completion_fn;
    i2c->completion_arg = completion_arg;

    init_dma(i2c->tx_dma_setup, (uint32_t)buf, len);
    dma_enable_transfer_complete_interrupt(i2c->tx_dma_setup->base, i2c->tx_dma_setup->stream);
    nvic_set_priority(i2c->tx_dma_setup->irq, 1);
    nvic_enable_irq(i2c->tx_dma_setup->irq);

    state_machine(i2c, I2C_STIM_START);
    return 0;
}

static void i2c_fno_init(struct fnode *dev, uint32_t n, const struct i2c_addr * addr)
{
    struct dev_i2c *i = &DEV_I2C[n];
    i->dev = device_fno_init(&mod_devi2c, addr->name, dev, FL_RDWR, i);
    i->base = addr->base;
    i->ev_irq = addr->ev_irq;
    i->er_irq = addr->er_irq;
    i->tx_dma_setup = &addr->tx_dma;
    i->rx_dma_setup = &addr->rx_dma;
    i->state = I2C_STATE_READY;
}

void i2c_init(struct fnode *dev, const struct i2c_addr i2c_addrs[], int num_i2cs)
{
    int i;
    for (i = 0; i < num_i2cs; i++) 
    {
        if (i2c_addrs[i].base == 0)
            continue;
        i2c_fno_init(dev, i, &i2c_addrs[i]);
        rcc_periph_clock_enable(i2c_addrs[i].rcc);
        rcc_periph_clock_enable(i2c_addrs[i].dma_rcc);

        i2c_peripheral_disable(i2c_addrs[i].base);
        i2c_reset(i2c_addrs[i].base);

        i2c_set_clock_frequency(i2c_addrs[i].base, i2c_addrs[i].clock_f);
        if(i2c_addrs[i].fast_mode)  i2c_set_fast_mode(i2c_addrs[i].base);
        else                                    i2c_set_standard_mode(i2c_addrs[i].base);

        i2c_set_trise(i2c_addrs[i].base, i2c_addrs[i].rise_time);
        i2c_set_ccr(i2c_addrs[i].base, i2c_addrs[i].bus_clk_frequency);

        i2c_nack_current(i2c_addrs[i].base);
        i2c_disable_ack(i2c_addrs[i].base);
        i2c_clear_stop(i2c_addrs[i].base);

        nvic_set_priority(i2c_addrs[i].ev_irq, 1);
        nvic_enable_irq(i2c_addrs[i].ev_irq);
        nvic_set_priority(i2c_addrs[i].er_irq, 1);
        nvic_enable_irq(i2c_addrs[i].er_irq);
    }
}

