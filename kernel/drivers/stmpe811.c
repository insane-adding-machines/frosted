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
 *      Authors: brabo
 *
 */


#include "frosted.h"
#include "device.h"
#include <stdint.h>
#include "ioctl.h"
#include "stmpe811.h"
#include "gpio.h"
#include "exti.h"
#include "dma.h"
#include "i2c.h"

#define STMPE811_I2C_ADDR       0x82
#define STMPE811_CHIP_ID        0x00    /* STMPE811 Device identification */

/* Registers */
#define STMPE811_CHIP_ID        0x00    /* STMPE811 Device identification */
#define STMPE811_ID_VER         0x02    /* STMPE811 Revision number */
#define STMPE811_SYS_CTRL1      0x03    /* Reset control */
#define STMPE811_SYS_CTRL2      0x04    /* Clock control */
#define STMPE811_SPI_CFG        0x08    /* SPI interface configuration */
#define STMPE811_INT_CTRL       0x09    /* Interrupt control register */
#define STMPE811_INT_EN         0x0A    /* Interrupt enable register */
#define STMPE811_INT_STA        0x0B    /* Interrupt status register */
#define STMPE811_GPIO_EN        0x0C    /* GPIO interrupt enable register */
#define STMPE811_GPIO_INT_STA   0x0D    /* GPIO interrupt status register */
#define STMPE811_ADC_INT_EN     0x0E    /* ADC interrupt enable register */
#define STMPE811_ADC_INT_STA    0x0F    /* ADC interface status register */
#define STMPE811_GPIO_SET_PIN   0x10    /* GPIO set pin register */
#define STMPE811_GPIO_CLR_PIN   0x11    /* GPIO clear pin register */
#define STMPE811_MP_STA         0x12    /* GPIO monitor pin state register */
#define STMPE811_GPIO_DIR       0x13    /* GPIO direction register */
#define STMPE811_GPIO_ED        0x14    /* GPIO edge detect register */
#define STMPE811_GPIO_RE        0x15    /* GPIO rising edge register */
#define STMPE811_GPIO_FE        0x16    /* GPIO falling edge register */
#define STMPE811_GPIO_AF        0x17    /* Alternate function register */
#define STMPE811_ADC_CTRL1      0x20    /* ADC control */
#define STMPE811_ADC_CTRL2      0x21    /* ADC control */
#define STMPE811_ADC_CAPT       0x22    /* To initiate ADC data acquisition */
#define STMPE811_ADC_DATA_CHO   0x30    /* ADC channel 0 */
#define STMPE811_ADC_DATA_CH1   0x32    /* ADC channel 1 */
#define STMPE811_ADC_DATA_CH2   0x34    /* ADC channel 2 */
#define STMPE811_ADC_DATA_CH3   0x36    /* ADC channel 3 */
#define STMPE811_ADC_DATA_CH4   0x38    /* ADC channel 4 */
#define STMPE811_ADC_DATA_CH5   0x3A    /* ADC channel 5 */
#define STMPE811_ADC_DATA_CH6   0x3C    /* ADC channel 6 */
#define STMPE811_ADC_DATA_CH7   0x3E    /* ADC channel 7 */
#define STMPE811_TSC_CTRL       0x40    /* 4-wire tsc setup */
#define STMPE811_TSC_CFG        0x41    /* Tsc configuration */
#define STMPE811_WDW_TR_X       0x42    /* Window setup for top right X */
#define STMPE811_WDW_TR_Y       0x44    /* Window setup for top right Y */
#define STMPE811_WDW_BL_X       0x46    /* Window setup for bottom left X */
#define STMPE811_WDW_BL_Y       0x48    /* Window setup for bottom left Y */
#define STMPE811_FIFO_TH        0x4A    /* FIFO level to generate interrupt */
#define STMPE811_FIFO_STA       0x4B    /* Current status of FIFO */
#define STMPE811_FIFO_SIZE      0x4C    /* Current filled level of FIFO */
#define STMPE811_TSC_DATA_X     0x4D    /* Data port for tsc data access */
#define STMPE811_TSC_DATA_Y     0x4F    /* Data port for tsc data access */
#define STMPE811_TSC_DATA_Z     0x51    /* Data port for tsc data access */
#define STMPE811_TSC_DATA_XYZ   0x52    /* Data port for tsc data access */
#define STMPE811_TSC_FRACTION_Z 0x56    /* Touchscreen controller FRACTION_Z */
#define STMPE811_TSC_DATA       0x57    /* Data port for tsc data access */
#define STMPE811_TSC_I_DRIVE    0x58    /* Touchscreen controller drivel */
#define STMPE811_TSC_SHIELD     0x59    /* Touchscreen controller shield */
#define STMPE811_TEMP_CTRL      0x60    /* Temperature sensor setup */
#define STMPE811_TEMP_DATA      0x61    /* Temperature data access port */
#define STMPE811_TEMP_TH        0x62    /* Threshold for temp controlled int */

#define STMPE811_SYS_CTRL2_ADC_OFF      (1 << 0)
#define STMPE811_SYS_CTRL2_TSC_OFF      (1 << 1)
#define STMPE811_SYS_CTRL2_GPIO_OFF     (1 << 2)
#define STMPE811_SYS_CTRL2_TS_OFF       (1 << 3)

#define STMPE811_TEMP_CTRL_EN           (1 << 0)
#define STMPE811_TEMP_CTRL_ACQ          (1 << 1)
#define STMPE811_TEMP_DATA_MSB_MASK     0x03

#define STMPE811_TSC_CTRL_EN            (1 << 0)

#define STMPE811_INT_EN_TOUCH_DET       (1 << 0)
#define STMPE811_INT_EN_FIFO_TH         (1 << 1)
#define STMPE811_INT_EN_FIFO_OFLOW      (1 << 2)
#define STMPE811_INT_EN_FIFO_FULL       (1 << 3)
#define STMPE811_INT_EN_FIFO_EMPTY      (1 << 4)
#define STMPE811_INT_EN_TEMP_SENS       (1 << 5)
#define STMPE811_INT_EN_ADC             (1 << 6)
#define STMPE811_INT_EN_GPIO            (1 << 7)

#define STMPE811_SYS_CTRL1_SOFT_RESET   (1 << 1)

#define STMPE811_FIFO_STA_TOUCH_DET     (1 << 0)

#define STMPE811_INT_CTRL_GLOBAL_INT    (1 << 0)

enum stmpe811_state {
    STMPE811_STATE_IDLE = 0,
    STMPE811_STATE_BUSY,
    STMPE811_STATE_TX_RDY,
    STMPE811_STATE_RX_RDY,
    STMPE811_STATE_RDY,
    STMPE811_STATE_RD,
    STMPE811_STATE_INIT,
};

enum insta {
    STMPE811_INIT_1,
    STMPE811_INIT_2,
    STMPE811_INIT_3,
    STMPE811_INIT_4,
    STMPE811_INIT_5,
    STMPE811_INIT_6,
    STMPE811_INIT_7,
    STMPE811_INIT_8,
    STMPE811_INIT_9,
    STMPE811_INIT_10,
    STMPE811_INIT_11,
    STMPE811_INIT_12,
    STMPE811_INIT_13,
    STMPE811_INIT_14,
    STMPE811_INIT_15,
    STMPE811_INIT_16,
    STMPE811_INIT_17,
    STMPE811_INIT_18,
    STMPE811_INIT_19,
    STMPE811_INIT_20,
    STMPE811_INIT_21,
    STMPE811_INIT_22,
    STMPE811_INIT_23,
    STMPE811_INIT_24,
    STMPE811_INIT_25,
    STMPE811_INIT_26,
    STMPE811_INIT_27,
    STMPE811_INIT_28,
    STMPE811_INIT_29,
    STMPE811_INIT_30,
    STMPE811_INIT_31,
    STMPE811_INIT_32,
    STMPE811_INIT_33,
    STMPE811_INIT_34,
    STMPE811_INIT_35,
    STMPE811_INIT_36,
    STMPE811_INIT_37,
    STMPE811_INIT_38,
    STMPE811_INIT_39,
    STMPE811_INIT_40,
    STMPE811_INIT_41,
    STMPE811_INIT_42,
    STMPE811_INIT_43,
    STMPE811_INIT_44,
    STMPE811_INIT_45,
    STMPE811_INIT_46,
    STMPE811_INIT_47,
    STMPE811_INIT_48,
    STMPE811_INIT_49,
    STMPE811_INIT_50,
    STMPE811_INIT_51,
    STMPE811_INIT_52,
    STMPE811_INIT_53,
    STMPE811_INIT_54,
    STMPE811_INIT_55,
    STMPE811_INIT_56,
    STMPE811_INIT_57,
};

struct dev_stmpe811 {
    struct i2c_slave i2c; /* As first argument, so isr callbacks will use this as arg */
    struct device *dev;
    unsigned int flags;
    int eidx;
    enum stmpe811_state state;
    volatile uint16_t cnt;
} TS;

static int devstmpe811_read(struct fnode *fno, void *buf, unsigned int len);
static int devstmpe811_close(struct fnode *fno);
static int stmpe811_fno_init(struct dev_stmpe811 *s);

static struct module mod_devstmpe811 = {
    .family = FAMILY_FILE,
    .name = "stmpe811",
    .ops.open = device_open,
    .ops.read = devstmpe811_read,
    .ops.close = devstmpe811_close,
};

static uint8_t buffer;

static void *sys_ctrl1_r(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    i2c_init_read(&stmpe811->i2c, STMPE811_SYS_CTRL1, &buffer, 1);
}

static void *set_softreset(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    buffer |= STMPE811_SYS_CTRL1_SOFT_RESET;

    i2c_init_write(&stmpe811->i2c, STMPE811_SYS_CTRL1, &buffer, 1);
}

static void *unset_softreset(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    buffer &= ~STMPE811_SYS_CTRL1_SOFT_RESET;

    i2c_init_write(&stmpe811->i2c, STMPE811_SYS_CTRL1, &buffer, 1);
}

static void *sys_ctrl2_r(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    i2c_init_read(&stmpe811->i2c, STMPE811_SYS_CTRL2, &buffer, 1);
}

static void *ts_off(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    buffer = STMPE811_SYS_CTRL2_TS_OFF;

    i2c_init_write(&stmpe811->i2c, STMPE811_SYS_CTRL2, &buffer, 1);
}

static void *gpio_off(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    buffer |= STMPE811_SYS_CTRL2_GPIO_OFF;

    i2c_init_write(&stmpe811->i2c, STMPE811_SYS_CTRL2, &buffer, 1);
}

static void *int_en_r(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    i2c_init_read(&stmpe811->i2c, STMPE811_INT_EN, &buffer, 1);
}

static void *enable_fifo_oflow(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    buffer = STMPE811_INT_EN_FIFO_OFLOW;

    i2c_init_write(&stmpe811->i2c, STMPE811_INT_EN, &buffer, 1);
}

static void *enable_fifo_th(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    buffer |= STMPE811_INT_EN_FIFO_TH;

    i2c_init_write(&stmpe811->i2c, STMPE811_INT_EN, &buffer, 1);
}

static void *enable_fifo_touch_det(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    buffer |= STMPE811_INT_EN_TOUCH_DET;

    i2c_init_write(&stmpe811->i2c, STMPE811_INT_EN, &buffer, 1);
}

static void *adc_ctrl1_r(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    buffer = 0x00;

    i2c_init_read(&stmpe811->i2c, STMPE811_ADC_CTRL1, &buffer, 1);
}

static void *set_adc_sample(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    /* set to 80 cycles */
    buffer |= 0x40;

    i2c_init_write(&stmpe811->i2c, STMPE811_ADC_CTRL1, &buffer, 1);
}

static void *set_adc_res(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    /* set adc resolution to 12(?) bit */
    buffer |= 0x08;

    i2c_init_write(&stmpe811->i2c, STMPE811_ADC_CTRL1, &buffer, 1);
}

static void *adc_ctrl2_r(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    buffer = 0x00;

    i2c_init_read(&stmpe811->i2c, STMPE811_ADC_CTRL2, &buffer, 1);
}

static void *set_adc_freq(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    /* set adc clock speed to 3.25 MHz */
    buffer |= 0x01;

    i2c_init_write(&stmpe811->i2c, STMPE811_ADC_CTRL2, &buffer, 1);
}

static void *gpio_af_r(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    buffer = 0x00;

    i2c_init_read(&stmpe811->i2c, STMPE811_GPIO_AF, &buffer, 1);
}

static void *set_gpio_af(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    buffer = 0x00;

    i2c_init_write(&stmpe811->i2c, STMPE811_GPIO_AF, &buffer, 1);
}

static void *tsc_cfg_r(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    buffer = 0x00;

    i2c_init_read(&stmpe811->i2c, STMPE811_TSC_CFG, &buffer, 1);
}

static void *set_tsc(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    buffer = 0x9A;

    i2c_init_write(&stmpe811->i2c, STMPE811_TSC_CFG, &buffer, 1);
}

static void *fifo_th_r(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    buffer = 0x00;

    i2c_init_read(&stmpe811->i2c, STMPE811_FIFO_TH, &buffer, 1);
}

static void *set_fifo_th(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    buffer = 0x01;

    i2c_init_write(&stmpe811->i2c, STMPE811_FIFO_TH, &buffer, 1);
}

static void *fifo_sta_r(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    buffer = 0x00;

    i2c_init_read(&stmpe811->i2c, STMPE811_FIFO_STA, &buffer, 1);
}

static void *set_fifo_sta_touchdet(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    buffer |= STMPE811_FIFO_STA_TOUCH_DET;

    i2c_init_write(&stmpe811->i2c, STMPE811_FIFO_STA, &buffer, 1);
}

static void *unset_fifo_sta_touchdet(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    buffer &= ~STMPE811_FIFO_STA_TOUCH_DET;

    i2c_init_write(&stmpe811->i2c, STMPE811_FIFO_STA, &buffer, 1);
}

static void *tsc_fracz_r(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    buffer = 0x00;

    i2c_init_read(&stmpe811->i2c, STMPE811_TSC_FRACTION_Z, &buffer, 1);
}

static void *set_tsc_fracz(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    buffer |= 0x07;

    i2c_init_write(&stmpe811->i2c, STMPE811_TSC_FRACTION_Z, &buffer, 1);
}

static void *tsc_idrive_r(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    buffer = 0x00;

    i2c_init_read(&stmpe811->i2c, STMPE811_TSC_I_DRIVE, &buffer, 1);
}

static void *set_tsc_idrive(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    buffer |= 0x01;

    i2c_init_write(&stmpe811->i2c, STMPE811_TSC_I_DRIVE, &buffer, 1);
}

static void *tsc_on(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    buffer &= ~STMPE811_SYS_CTRL2_TSC_OFF;

    i2c_init_write(&stmpe811->i2c, STMPE811_SYS_CTRL2, &buffer, 1);
}

static void *tsc_ctrl_r(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    buffer = 0x00;

    i2c_init_read(&stmpe811->i2c, STMPE811_TSC_CTRL, &buffer, 1);
}

static void *enable_tsc(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    buffer |= STMPE811_TSC_CTRL_EN;

    i2c_init_write(&stmpe811->i2c, STMPE811_TSC_CTRL, &buffer, 1);
}

static void *int_sta_r(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    buffer = 0x00;

    i2c_init_read(&stmpe811->i2c, STMPE811_INT_STA, &buffer, 1);
}

static void *set_int_sta_tsc_en(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    buffer = 0xFF;

    i2c_init_write(&stmpe811->i2c, STMPE811_INT_STA, &buffer, 1);
}

static void *int_ctrl_r(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    buffer = 0x00;

    i2c_init_read(&stmpe811->i2c, STMPE811_INT_CTRL, &buffer, 1);
}

static void *set_int_ctrl_global_en(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    buffer |= STMPE811_INT_CTRL_GLOBAL_INT;

    i2c_init_write(&stmpe811->i2c, STMPE811_INT_CTRL, &buffer, 1);
}

static void *gpio_on(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    buffer &= ~STMPE811_SYS_CTRL2_GPIO_OFF;

    i2c_init_write(&stmpe811->i2c, STMPE811_SYS_CTRL2, &buffer, 1);
}

static void *init_end(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    i2c_init_read(&stmpe811->i2c, STMPE811_SYS_CTRL2, &buffer, 1);
    stmpe811_fno_init(stmpe811);
    register_module(&mod_devstmpe811);
    stmpe811->cnt = 0;
    stmpe811->state = STMPE811_STATE_IDLE;
}

struct ts_init {
    enum insta state;
    int (*init)(struct dev_stmpe811 *stmpe811);
};

static const struct ts_init ts_init[] = {
    { STMPE811_INIT_1, sys_ctrl1_r },
    { STMPE811_INIT_2, set_softreset },
    { STMPE811_INIT_3, unset_softreset },
    { STMPE811_INIT_4, sys_ctrl2_r },
    { STMPE811_INIT_5, ts_off },
    { STMPE811_INIT_6, gpio_off },
    { STMPE811_INIT_7, sys_ctrl2_r },
    { STMPE811_INIT_8, int_en_r },
    { STMPE811_INIT_9, enable_fifo_oflow },
    { STMPE811_INIT_10, enable_fifo_th },
    { STMPE811_INIT_11, enable_fifo_touch_det },
    { STMPE811_INIT_12, int_en_r },
    { STMPE811_INIT_13, adc_ctrl1_r },
    { STMPE811_INIT_14, set_adc_sample },
    { STMPE811_INIT_15, set_adc_res },
    { STMPE811_INIT_16, adc_ctrl1_r },
    { STMPE811_INIT_17, adc_ctrl2_r },
    { STMPE811_INIT_18, set_adc_freq },
    { STMPE811_INIT_19, adc_ctrl2_r },
    { STMPE811_INIT_20, gpio_af_r },
    { STMPE811_INIT_21, set_gpio_af },
    { STMPE811_INIT_22, gpio_af_r },
    { STMPE811_INIT_23, tsc_cfg_r },
    { STMPE811_INIT_24, set_tsc },
    { STMPE811_INIT_25, tsc_cfg_r },
    { STMPE811_INIT_26, fifo_th_r },
    { STMPE811_INIT_27, set_fifo_th },
    { STMPE811_INIT_28, fifo_th_r },
    { STMPE811_INIT_29, fifo_sta_r },
    { STMPE811_INIT_30, set_fifo_sta_touchdet },
    { STMPE811_INIT_31, unset_fifo_sta_touchdet },
    { STMPE811_INIT_32, fifo_sta_r },
    { STMPE811_INIT_33, tsc_fracz_r },
    { STMPE811_INIT_34, set_tsc_fracz },
    { STMPE811_INIT_35, tsc_fracz_r },
    { STMPE811_INIT_36, tsc_idrive_r },
    { STMPE811_INIT_37, set_tsc_idrive },
    { STMPE811_INIT_38, tsc_idrive_r },
    { STMPE811_INIT_39, sys_ctrl2_r },
    { STMPE811_INIT_40, tsc_on },
    { STMPE811_INIT_41, sys_ctrl2_r },
    { STMPE811_INIT_42, tsc_ctrl_r },
    { STMPE811_INIT_43, enable_tsc },
    { STMPE811_INIT_44, tsc_ctrl_r },
    { STMPE811_INIT_45, int_sta_r },
    { STMPE811_INIT_46, set_int_sta_tsc_en },
    { STMPE811_INIT_47, int_sta_r },
    { STMPE811_INIT_48, int_ctrl_r },
    { STMPE811_INIT_49, set_int_ctrl_global_en },
    { STMPE811_INIT_50, int_ctrl_r },
    { STMPE811_INIT_51, sys_ctrl2_r },
    { STMPE811_INIT_52, gpio_on },
    { STMPE811_INIT_53, sys_ctrl2_r },
    { STMPE811_INIT_54, init_end },
};

static uint32_t xy;

static void *ts_read_touch(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    i2c_init_read(&stmpe811->i2c, STMPE811_TSC_CTRL, &buffer, 1);
}


static void *ts_read_x_0(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    if ((buffer & 0x80) == 0) {
        /* no touch, return 0xFFFFFFFF throuhg xy */
        xy = 0xFFFFFFFF;

        exti_enable(stmpe811->eidx, 0);
        stmpe811->state = STMPE811_STATE_RDY;
        stmpe811->cnt = 0;

        if (stmpe811->dev->pid > 0) {
            task_resume(stmpe811->dev->pid);
        }
    } else {
        i2c_init_read(&stmpe811->i2c, STMPE811_TSC_DATA_X, &buffer, 1);
    }
}

static void *ts_read_x_1(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    xy = buffer;

    i2c_init_read(&stmpe811->i2c, (STMPE811_TSC_DATA_X + 1), &buffer, 1);
}

static void *ts_read_y_0(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    xy |= (buffer << 8);

    i2c_init_read(&stmpe811->i2c, STMPE811_TSC_DATA_Y, &buffer, 1);
}

static void *ts_read_y_1(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    xy |= (buffer << 16);

    i2c_init_read(&stmpe811->i2c, (STMPE811_TSC_DATA_Y + 1), &buffer, 1);
}

static void *ts_read_end(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    xy |= (buffer << 24);

    exti_enable(stmpe811->eidx, 0);
    stmpe811->state = STMPE811_STATE_RDY;

    if (stmpe811->dev->pid > 0) {
        task_resume(stmpe811->dev->pid);
    }
}

enum ts_read_state {
    TS_READ_TOUCH,
    TS_READ_X_0,
    TS_READ_X_1,
    TS_READ_Y_0,
    TS_READ_Y_1,
};

struct ts_read {
    enum ts_read_state state;
    int (*call)(struct dev_stmpe811 *stmpe811);
};

static const struct ts_read ts_read[] = {
    { TS_READ_X_0, ts_read_x_0 },
    { TS_READ_X_0, ts_read_x_0 },
    { TS_READ_X_1, ts_read_x_1 },
    { TS_READ_Y_0, ts_read_y_0 },
    { TS_READ_Y_1, ts_read_y_1 },
};
/* I2C operation callbacks, executed in IRQ context, and with I2C mutex held. */
static void stmpe811_tx_isr(struct i2c_slave *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    if (stmpe811->state == STMPE811_STATE_INIT) {
        void (*next_f)(void *) = ts_init[stmpe811->cnt++].init;
        tasklet_add(next_f, stmpe811);
    }
}

static void stmpe811_rx_isr(struct i2c_slave *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;

    if (stmpe811->state == STMPE811_STATE_INIT) {
        void (*next_f)(void *) = ts_init[stmpe811->cnt++].init;
        tasklet_add(next_f, stmpe811);
    }

    if (stmpe811->state == STMPE811_STATE_RD) {
        void (*next_f)(void *) = ts_read[stmpe811->cnt++].call;
        tasklet_add(next_f, stmpe811);
    }
}

static void ts_isr(void *arg)
{
    struct dev_stmpe811 *stmpe811 = (struct dev_stmpe811 *)arg;
    stmpe811->state = STMPE811_STATE_BUSY;
    i2c_init_read(&stmpe811->i2c, STMPE811_TSC_DATA_X, &buffer, 1);
}

static int devstmpe811_read(struct fnode *fno, void *buf, unsigned int len)
{
    struct dev_stmpe811 *stmpe811;

    if (len == 0)
        return len;

    if (len != 4)
        return -EINVAL;

    stmpe811 = (struct dev_stmpe811 *)FNO_MOD_PRIV(fno, &mod_devstmpe811);

    if (!stmpe811)
        return EINVAL;

    if (stmpe811->state == STMPE811_STATE_IDLE) {
        if ((stmpe811->dev->pid) && (stmpe811->dev->pid != scheduler_get_cur_pid())) {
            return -EBUSY;
        }
        stmpe811->dev->pid = scheduler_get_cur_pid();

        task_suspend();
        exti_enable(stmpe811->eidx, 1);
        return SYS_CALL_AGAIN;
    } else if (stmpe811->state == STMPE811_STATE_RDY) {
        memcpy(buf, &xy, 4);
        stmpe811->dev->pid = 0;
        stmpe811->state = STMPE811_STATE_IDLE;
        return sizeof(uint32_t);
    }
}

static int devstmpe811_close(struct fnode *fno)
{
    return 0;
}


static int stmpe811_fno_init(struct dev_stmpe811 *s)
{
    static int num_ts = 0;
    char name[4] = "ts";
    struct fnode *devfs = fno_search("/dev");
    if (!devfs)
        return -ENOENT;


    name[2] =  '0' + num_ts++;
    s->dev = device_fno_init(&mod_devstmpe811, name, devfs, FL_RDONLY, s);
    s->dev->pid = -1;
    return 0;

}

int stmpe811_init(struct ts_config *ts)
{
    struct dev_stmpe811 *stmpe811;
    stmpe811 = &TS;
    memset(stmpe811, 0, sizeof(struct dev_stmpe811));

    gpio_create(&mod_devstmpe811, &ts->gpio);

    stmpe811->eidx = exti_register(ts->gpio.base, ts->gpio.pin, GPIO_TRIGGER_RAISE, ts_isr, &ts->gpio);

    /* Populate i2c_slave struct */
    stmpe811->i2c.bus = ts->bus;
    stmpe811->i2c.address = STMPE811_I2C_ADDR;
    stmpe811->i2c.isr_tx = stmpe811_tx_isr;
    stmpe811->i2c.isr_rx = stmpe811_rx_isr;
    stmpe811->state = STMPE811_STATE_INIT;
    stmpe811->cnt = 0;

    i2c_init_read(&stmpe811->i2c, (STMPE811_CHIP_ID + 1), &buffer, 1);

    return 0;
}
