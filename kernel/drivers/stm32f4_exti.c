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

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/nvic.h>

#include "gpio.h"
#include "stm32f4_exti.h"

struct dev_exti {
    struct device * dev;
    uint32_t base;
    uint16_t pin;
    uint32_t exti;
    exti_callback exti_callback_fn;
    void * exti_callback_arg;
};

#define MAX_EXTIS   32      /* FIX THIS! */

static struct dev_exti DEV_EXTI[MAX_EXTIS];

static int devexti_ioctl(struct fnode *fno, const uint32_t cmd, void *arg);

static struct module mod_devexti = {
    .family = FAMILY_FILE,
    .name = "exti",
    .ops.open = device_open,
    .ops.ioctl = devexti_ioctl,
};

struct dev_exti * exti_fno[16];

void exti_register_callback(struct fnode *fno, exti_callback callback_fn, void * callback_arg)
{
    struct dev_exti *exti = (struct dev_exti *)FNO_MOD_PRIV(fno, &mod_devexti);
    if(!exti)
        return;

    exti->exti_callback_fn = callback_fn;
    exti->exti_callback_arg = callback_arg;
}

void exti_isr(uint32_t exti_base, uint32_t exti_idx)
{
    exti_reset_request(exti_base);
    if(exti_fno[exti_idx]->exti_callback_fn)
    {
        tasklet_add(exti_fno[exti_idx]->exti_callback_fn, exti_fno[exti_idx]->exti_callback_arg);
    }
}
void exti0_isr(void)
{
    exti_isr(EXTI0, 0);
}
void exti1_isr(void)
{
    exti_isr(EXTI1, 1);
}
void exti2_isr(void)
{
    exti_isr(EXTI2, 2);
}
void exti3_isr(void)
{
    exti_isr(EXTI3, 3);
}
void exti4_isr(void)
{
    exti_isr(EXTI4, 4);
}
void exti9_5_isr(void)
{
    if(exti_get_flag_status(EXTI5))exti_isr(EXTI5, 5);
    if(exti_get_flag_status(EXTI6))exti_isr(EXTI6, 6);
    if(exti_get_flag_status(EXTI7))exti_isr(EXTI7, 7);
    if(exti_get_flag_status(EXTI8))exti_isr(EXTI8, 8);
    if(exti_get_flag_status(EXTI9))exti_isr(EXTI9, 9);
}
void exti15_10_isr(void)
{
    if(exti_get_flag_status(EXTI10))exti_isr(EXTI10, 10);
    if(exti_get_flag_status(EXTI11))exti_isr(EXTI11, 11);
    if(exti_get_flag_status(EXTI12))exti_isr(EXTI12, 12);
    if(exti_get_flag_status(EXTI13))exti_isr(EXTI13, 13);
    if(exti_get_flag_status(EXTI14))exti_isr(EXTI14, 14);
    if(exti_get_flag_status(EXTI15))exti_isr(EXTI15, 15);
}

int exti_enable(struct fnode * fno, int enable)
{
    struct dev_exti *exti;
    exti = (struct dev_exti *)FNO_MOD_PRIV(fno, &mod_devexti);
    if(exti)
    {
        if(enable)
        {
            exti_enable_request(exti->exti);
        }
        else
        {
            exti_disable_request(exti->exti);
        }
        return 0;
    }
    return -1;
}

static int devexti_ioctl(struct fnode * fno, const uint32_t cmd, void *arg)
{
    (void)arg;
    if (cmd == IOCTL_EXTI_DISABLE) {
        return exti_enable(fno, 0);
    }
    if (cmd == IOCTL_EXTI_ENABLE) {
        return exti_enable(fno, 1);
    }
    return -1;
}


static void exti_fno_init(struct fnode *dev, uint32_t n, const struct exti_addr * addr)
{
    struct dev_exti *e = &DEV_EXTI[n];
    e->dev = device_fno_init(&mod_devexti, addr->name, dev, FL_RDWR, e);
    e->base = addr->base;
    e->pin = addr->pin;
    e->exti_callback_fn = NULL;
    e->exti_callback_arg = 0;
}

void exti_init(struct fnode * dev,  const struct exti_addr exti_addrs[], int num_extis)
{
    int i;
    uint32_t exti_irq;
    rcc_periph_clock_enable(RCC_SYSCFG);

    for(i=0;i<num_extis;i++)
    {
        if(exti_addrs[i].base == 0)
            continue;

        exti_fno_init(dev, i, &exti_addrs[i]);

        switch(exti_addrs[i].pin)
        {
            case GPIO0:     DEV_EXTI[i].exti = EXTI0;       exti_irq = NVIC_EXTI0_IRQ;  exti_fno[0] = &DEV_EXTI[i];  break;
            case GPIO1:     DEV_EXTI[i].exti = EXTI1;       exti_irq = NVIC_EXTI1_IRQ;  exti_fno[1] = &DEV_EXTI[i];  break;
            case GPIO2:     DEV_EXTI[i].exti = EXTI2;       exti_irq = NVIC_EXTI2_IRQ;  exti_fno[2] = &DEV_EXTI[i];  break;
            case GPIO3:     DEV_EXTI[i].exti = EXTI3;       exti_irq = NVIC_EXTI3_IRQ;  exti_fno[3] = &DEV_EXTI[i];  break;
            case GPIO4:     DEV_EXTI[i].exti = EXTI4;       exti_irq = NVIC_EXTI4_IRQ;  exti_fno[4] = &DEV_EXTI[i];  break;
            case GPIO5:     DEV_EXTI[i].exti = EXTI5;       exti_irq = NVIC_EXTI9_5_IRQ;  exti_fno[5] = &DEV_EXTI[i];  break;
            case GPIO6:     DEV_EXTI[i].exti = EXTI6;       exti_irq = NVIC_EXTI9_5_IRQ;  exti_fno[6] = &DEV_EXTI[i];  break;
            case GPIO7:     DEV_EXTI[i].exti = EXTI7;       exti_irq = NVIC_EXTI9_5_IRQ;  exti_fno[7] = &DEV_EXTI[i];  break;
            case GPIO8:     DEV_EXTI[i].exti = EXTI8;       exti_irq = NVIC_EXTI9_5_IRQ;  exti_fno[8] = &DEV_EXTI[i];  break;
            case GPIO9:     DEV_EXTI[i].exti = EXTI9;       exti_irq = NVIC_EXTI9_5_IRQ;  exti_fno[9] = &DEV_EXTI[i];  break;
            case GPIO10:    DEV_EXTI[i].exti = EXTI10;       exti_irq = NVIC_EXTI15_10_IRQ;  exti_fno[10] = &DEV_EXTI[i];  break;
            case GPIO11:    DEV_EXTI[i].exti = EXTI11;       exti_irq = NVIC_EXTI15_10_IRQ;  exti_fno[11] = &DEV_EXTI[i];  break;
            case GPIO12:    DEV_EXTI[i].exti = EXTI12;       exti_irq = NVIC_EXTI15_10_IRQ;  exti_fno[12] = &DEV_EXTI[i];  break;
            case GPIO13:    DEV_EXTI[i].exti = EXTI13;       exti_irq = NVIC_EXTI15_10_IRQ;  exti_fno[13] = &DEV_EXTI[i];  break;
            case GPIO14:    DEV_EXTI[i].exti = EXTI14;       exti_irq = NVIC_EXTI15_10_IRQ;  exti_fno[14] = &DEV_EXTI[i];  break;
            case GPIO15:    DEV_EXTI[i].exti = EXTI15;       exti_irq = NVIC_EXTI15_10_IRQ;  exti_fno[15] = &DEV_EXTI[i];  break;
            default: break;
        }

        nvic_enable_irq(exti_irq);
        exti_select_source(DEV_EXTI[i].exti, exti_addrs[i].base);
        exti_set_trigger(DEV_EXTI[i].exti, exti_addrs[i].trigger);
        /* Be sure to enable the exti via IOCTL! */
    }
    register_module(&mod_devexti);
}
