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
 *      Authors: Zaerc, brabo
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


/* Frosted device driver hook */
static struct module mod_spi_mmc;

struct dev_sd {
    struct device * dev;
    MMC_CARD card;
};

struct dev_sd SdCard[1]; /* Multiple slots? Make this array bigger */



/* Internal stuff for SPI */
struct dev_spi_mmc {
    struct spi_slave sl; /* First argument, for inheritance */
    struct device * dev;
    const struct gpio_config *pio_cs;
};

struct spi_mmc_ctrl_reg
{
    uint8_t reg;
    uint8_t data;
};

static struct dev_spi_mmc SPI_MMC;

static int devspi_mmc_open(const char *path, int flags);
static int devspi_mmc_read(struct fnode *fno, void *buf, unsigned int len);
static int devspi_mmc_close(struct fnode *fno);




/*
 * For all the SPI-related stuff, lis3dsh.[c|h] is a functional SPI
 * slave driver, good example. To expose the device properly as a
 * block device, checkout stm_sdio.c from line 851. We will also need
 * to expose a block_read and a block_write in the end that accept
 * and handle arguments same as sdio_block_read and sdio_block_write
 * (again in stm32_sdio.c). for open/close
 *
 */




static int devspi_mmc_open(const char *path, int flags)
{
    struct fnode *f = fno_search(path);
    if (!f)
        return -ENOENT;

    /* Do whatever open stuff we need to do right here.. */


    /* Once complete, we need to add and return an FD! */
    return task_filedesc_add(f);
}

static int dev_spi_mmc_read(struct fnode *fno, void *buf, unsigned int len)
{
    struct dev_spi_mmc *spi_mmc = FNO_MOD_PRIV(fno, &mod_spi_mmc);

    if (!spi_mmc)
        return -ENOENT;

    /* Do a read here. Have to see if 1 spi read returns enough
     * to use this as block_read!
     */


     /* Once complete, we need to return number of bytes read (in either case) */
     return len;
}

/* Here we need a write similar to read */

static int devspi_mmc_close(struct fnode *fno)
{
    struct dev_spi_mmc *spi_mmc;
    spi_mmc = FNO_MOD_PRIV(fno, &mod_spi_mmc);

    if (!spi_mmc)
        return -ENOENT;

    /* Do what needs to be done to close.. */


    /* And return 0 to indicate success.. */
    return 0;
}

/* We may also need an ISR.. */

static void spi_mmc_isr(struct spi_slave *sl)
{
    struct dev_spi_mmc *spi_mmc = (struct dev_spi_mmc *)sl;
    task_resume(spi_mmc->dev->task);
}


static void spi_mmc_card_detect(void *arg)
{
    struct fnode *dev = arg;
    char name[4] = "sd0";
    memset(&SdCard[0], 0, sizeof(struct dev_sd));
    SdCard[0].card = stm32_sdio_open();
    if (!SdCard[0].card) {
        return;
    }
    kprintf("Found SD card in microSD slot.\r\n");
    SdCard[0].dev = device_fno_init(&mod_spi_mmc, name, dev, FL_BLK, &SdCard[0]);
}

int spi_mmc_init(struct spi_mmc_config *conf)
{
    SDIO_CARD card;
    struct fnode *devfs;
    memset(&SPI_MMC, 0, sizeof(struct dev_spi_mmc));

    devfs = fno_search("/dev");
    if (!devfs)
        return -ENOENT;

    SPI_MMC.dev = device_fno_init(&mod_spi_mmc, "ins", devfs, FL_RDONLY, &SPI_MMC);
    if (!SPI_MMC.dev)
        return -EFAULT;


    memset(&mod_spi_mmc, 0, sizeof(mod_spi_mmc));
    kprintf("Successfully initialized SPI_MMC module.\r\n");
    strcpy(mod_spi_mmc.name,"spi_mmc");


    //mod_sdio.ops.close = sdio_close;
    mod_spi_mmc.ops.block_read = spi_mmc_block_read;
    mod_spi_mmc.ops.block_write = spi_mmc_block_write;

    /* Populate spi_slave struct */
    SPI_MMC.sl.bus = bus;
    SPI_MMC.sl.isr = spi_mmc_isr;


    register_module(&mod_spi_mmc);


    // need to check out what this is doing... */
    tasklet_add(spi_mmc_card_detect, devfs);
    return 0;
}










///////////////////////////////////////////////////////
//// following functions from lis3dsh.c just as example
///////////////////////////////////////////////////////


/* Function to write a register to LIS3DSH through SPI  */
static void lis3dsh_write_reg(int reg, int data)
{
	/* set CS low */
    gpio_clear(LIS3DSH.pio_cs->base, LIS3DSH.pio_cs->pin);
	spi_xfer(SPI1, SET_WRITE_SINGLE_CMD(reg));
	spi_xfer(SPI1, data);
	/* set CS high */
    gpio_set(LIS3DSH.pio_cs->base, LIS3DSH.pio_cs->pin);
}


/* Function to read a register from LIS3DSH through SPI */
static int lis3dsh_read_reg(int reg)
{
	int reg_value;
	/* set CS low */
    gpio_clear(LIS3DSH.pio_cs->base, LIS3DSH.pio_cs->pin);
	reg_value = spi_xfer(SPI1, SET_READ_SINGLE_CMD(reg));
	reg_value = spi_xfer(SPI1, 0xFF);
	/* set CS high */
    gpio_set(LIS3DSH.pio_cs->base, LIS3DSH.pio_cs->pin);
	return reg_value;
}

static int devlis3dsh_open(const char *path, int flags)
{
    struct fnode *f = fno_search(path);
    volatile int int_reg_value;
    if (!f)
        return -1;
    /* get WHO AM I value */
    int_reg_value = lis3dsh_read_reg(ADD_REG_WHO_AM_I);

    /* if WHO AM I value is the expected one */
    if (int_reg_value == UC_WHO_AM_I_DEFAULT_VALUE) {
        /* set output data rate to 400 Hz and enable X,Y,Z axis */
        lis3dsh_write_reg(ADD_REG_CTRL_4, UC_ADD_REG_CTRL_4_CFG_VALUE_DEFAULT);
        /* verify written value */
        int_reg_value = lis3dsh_read_reg(ADD_REG_CTRL_4);
        /* if written value is different */
        if (int_reg_value != UC_ADD_REG_CTRL_4_CFG_VALUE_DEFAULT) {
            return -EIO;
        }
    } else {
        return -EIO;
    }
    return task_filedesc_add(f);
}

static int devlis3dsh_read(struct fnode *fno, void *buf, unsigned int len)
{
    struct dev_lis3dsh *lis3dsh = FNO_MOD_PRIV(fno, &mod_devlis3dsh);
    uint8_t reg_base = ADD_REG_OUT_X_L;
    uint8_t i;
    volatile int int_reg_value;
    if (len < 6)
        return len;

    if (!lis3dsh)
        return -1;
    for (i = 0; i < 6; i++) {
        ((uint8_t*)buf)[i] = lis3dsh_read_reg(reg_base + i);
    }
    return 6;
}

static int devlis3dsh_close(struct fnode *fno)
{
    struct dev_lis3dsh *lis3dsh;
    lis3dsh = FNO_MOD_PRIV(fno, &mod_devlis3dsh);
    if (!lis3dsh)
        return -1;

    return 0;
}

static void lis3dsh_isr(struct spi_slave *sl)
{
    struct dev_lis3dsh *lis = (struct dev_lis3dsh *)sl;
    task_resume(lis->dev->task);
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
