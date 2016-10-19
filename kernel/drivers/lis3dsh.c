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
#include "device.h"
#include <stdint.h>
#include "ioctl.h"
#include "gpio.h"
#include "exti.h"
#include "dma.h"
#include "spi.h"
#include <unicore-mx/stm32/spi.h>

/* LIS3DSH registers addresses */
#define ADD_REG_WHO_AM_I				0x0F
#define ADD_REG_CTRL_4					0x20
#define ADD_REG_OUT_X_L					0x28
#define ADD_REG_OUT_X_H					0x29
#define ADD_REG_OUT_Y_L					0x2A
#define ADD_REG_OUT_Y_H					0x2B
#define ADD_REG_OUT_Z_L					0x2C
#define ADD_REG_OUT_Z_H					0x2D

/* WHO AM I register default value */
#define UC_WHO_AM_I_DEFAULT_VALUE		0x3F

/* ADD_REG_CTRL_4 register configuration value:
 * X,Y,Z axis enabled and 400Hz of output data rate */
#define UC_ADD_REG_CTRL_4_CFG_VALUE		0x77

/* Sensitivity for 2G range [mg/digit] */
#define SENS_2G_RANGE_MG_PER_DIGIT		((float)0.06)

/* LED threshold value in mg */
#define LED_TH_MG				(1000)	/* 1000mg (1G) */

/* set read single command. Attention: command must be 0x3F at most */
#define SET_READ_SINGLE_CMD(x)			(x | 0x80)
/* set read multiple command. Attention: command must be 0x3F at most */
#define SET_READ_MULTI_CMD(x)			(x | 0xC0)
/* set write single command. Attention: command must be 0x3F at most */
#define SET_WRITE_SINGLE_CMD(x)			(x & (~(0xC0)))
/* set write multiple command. Attention: command must be 0x3F at most */
#define SET_WRITE_MULTI_CMD(x)			(x & (~(0x80))	\
						 x |= 0x40)

enum dev_lis3dsh_state {
    LIS3DSH_IDLE = 0,
    LIS3DSH_READING,
    LIS3DSH_COMPLETE
};

struct dev_lis3dsh {
    struct spi_slave sl; /* First argument, for inheritance */
    struct device * dev;
    enum dev_lis3dsh_state state;
    const struct gpio_config *pio_cs;
    uint8_t spi_obuf[8];
    uint8_t spi_ibuf[8];
};

struct lis3dsh_ctrl_reg
{
    uint8_t reg;
    uint8_t data;
};

static struct dev_lis3dsh LIS3DSH;

static int devlis3dsh_read(struct fnode *fno, void *buf, unsigned int len);
static int devlis3dsh_close(struct fnode *fno);

static struct module mod_devlis3dsh = {
    .family = FAMILY_FILE,
    .name = "lis3dsh",
    .ops.open = device_open,
    .ops.read = devlis3dsh_read,
    .ops.close = devlis3dsh_close,
};

#if 0
static int devlis3dsh_read(struct fnode *fno, void *buf, unsigned int len)
{
    struct dev_lis3dsh *lis3dsh = FNO_MOD_PRIV(fno, &mod_devlis3dsh);
    if (len <= 0)
        return len;

    if (!lis3dsh)
        return -1;

    switch (lis3dsh->state) {
        case LIS3DSH_IDLE:
            lis3dsh->spi_obuf[0] = 0x28;
            gpio_clear(lis3dsh->pio_cs->base, lis3dsh->pio_cs->pin);
            devspi_xfer(&lis3dsh->sl, lis3dsh->spi_obuf, lis3dsh->spi_ibuf, 7);
            gpio_set(lis3dsh->pio_cs->base, lis3dsh->pio_cs->pin);
            lis3dsh->dev->pid = scheduler_get_cur_pid();
            lis3dsh->state = LIS3DSH_READING;
            task_suspend();
            return SYS_CALL_AGAIN;
        case LIS3DSH_READING:
            task_suspend();
            return SYS_CALL_AGAIN;
        case LIS3DSH_COMPLETE:
            memcpy(buf, lis3dsh->spi_ibuf, len);
            lis3dsh->state = LIS3DSH_IDLE;
            return len;
    }
    return len;
}
#endif


/* Function to write a register to LIS3DSH through SPI  */
static void lis3dsh_write_reg(int reg, int data)
{
	/* set CS low */
    irq_off();
	gpio_clear(GPIOE, GPIO3);
	/* discard returned value */
	spi_xfer(SPI1, SET_WRITE_SINGLE_CMD(reg));
	spi_xfer(SPI1, data);
	/* set CS high */
	gpio_set(GPIOE, GPIO3);
    irq_on();
}


/* Function to read a register from LIS3DSH through SPI */
static int lis3dsh_read_reg(int reg)
{
	int reg_value;
    irq_off();
	/* set CS low */
	gpio_clear(GPIOE, GPIO3);
	reg_value = spi_xfer(SPI1, SET_READ_SINGLE_CMD(reg));
	reg_value = spi_xfer(SPI1, 0xFF);
	/* set CS high */
	gpio_set(GPIOE, GPIO3);
    irq_on();

	return reg_value;
}


static int devlis3dsh_read(struct fnode *fno, void *buf, unsigned int len)
{
    struct dev_lis3dsh *lis3dsh = FNO_MOD_PRIV(fno, &mod_devlis3dsh);
    volatile int int_reg_value;
    if (len <= 0)
        return len;

    if (!lis3dsh)
        return -1;
	
    int_reg_value = lis3dsh_read_reg(0x0d);
    int_reg_value = lis3dsh_read_reg(0x0e);
	/* get WHO AM I value */
	int_reg_value = lis3dsh_read_reg(ADD_REG_WHO_AM_I);

	/* if WHO AM I value is the expected one */
	if (int_reg_value == UC_WHO_AM_I_DEFAULT_VALUE) {
		/* set output data rate to 400 Hz and enable X,Y,Z axis */
		lis3dsh_write_reg(ADD_REG_CTRL_4, UC_ADD_REG_CTRL_4_CFG_VALUE);
		/* verify written value */
		int_reg_value = lis3dsh_read_reg(ADD_REG_CTRL_4);
		/* if written value is different */
		if (int_reg_value != UC_ADD_REG_CTRL_4_CFG_VALUE) {
			/* ERROR: stay here... */
			while (1);
		}
	} else {
		/* ERROR: stay here... */
		while (1);
	}
    return len;
}

static int devlis3dsh_close(struct fnode *fno)
{
    struct dev_lis3dsh *lis3dsh;
    lis3dsh = FNO_MOD_PRIV(fno, &mod_devlis3dsh);
    if (!lis3dsh)
        return -1;
    lis3dsh->state = LIS3DSH_IDLE;

    return 0;
}

static void lis3dsh_isr(struct spi_slave *sl)
{
    struct dev_lis3dsh *lis = (struct dev_lis3dsh *)sl;
    switch(lis->state) {
        case LIS3DSH_READING:
            lis->state = LIS3DSH_COMPLETE;
            task_resume(lis->dev->pid);
            break;
    }
}


int lis3dsh_init(uint8_t bus, const struct gpio_config *lis3dsh_cs)
{
    struct fnode *devfs;
    memset(&LIS3DSH, 0, sizeof(struct dev_lis3dsh));

    devfs = fno_search("/dev");
    if (!devfs)
        return -EFAULT;

    LIS3DSH.dev = device_fno_init(&mod_devlis3dsh, "ins", devfs, FL_RDONLY, &LIS3DSH);
    if (!LIS3DSH.dev)
        return -EFAULT;

    /* Populate spi_slave struct */
    LIS3DSH.sl.bus = bus;
    LIS3DSH.sl.isr = lis3dsh_isr;
    LIS3DSH.pio_cs = lis3dsh_cs;
    register_module(&mod_devlis3dsh);
    gpio_create(&mod_devlis3dsh, lis3dsh_cs);
    gpio_set(LIS3DSH.pio_cs->base, LIS3DSH.pio_cs->pin);
    return 0;
}
