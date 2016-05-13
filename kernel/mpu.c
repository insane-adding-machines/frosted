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
#include "cm3/mpu.h"
#include "libopencmsis/core_cm3.h"
#include "stm32/tools.h"

#define MPUSIZE_1K      (0x09 << 1)
#define MPUSIZE_2K      (0x0a << 1)
#define MPUSIZE_4K      (0x0b << 1)
#define MPUSIZE_8K      (0x0c << 1)
#define MPUSIZE_16K     (0x0d << 1)
#define MPUSIZE_32K     (0x0e << 1)
#define MPUSIZE_64K     (0x0f << 1)
#define MPUSIZE_128K    (0x10 << 1)
#define MPUSIZE_256K    (0x11 << 1)
#define MPUSIZE_512K    (0x12 << 1)
#define MPUSIZE_1M      (0x13 << 1)
#define MPUSIZE_2M      (0x14 << 1)
#define MPUSIZE_4M      (0x15 << 1)
#define MPUSIZE_8M      (0x16 << 1)
#define MPUSIZE_16M     (0x17 << 1)
#define MPUSIZE_32M     (0x18 << 1)
#define MPUSIZE_64M     (0x19 << 1)
#define MPUSIZE_128M    (0x1a << 1)
#define MPUSIZE_256M    (0x1b << 1)
#define MPUSIZE_512M    (0x1c << 1)
#define MPUSIZE_1G      (0x1d << 1)
#define MPUSIZE_2G      (0x1e << 1)
#define MPUSIZE_4G      (0x1f << 1)
#define MPUSIZE_ERR     (0xFFFFFFFFu)


#define FLASH_START       (0x00000000)
#define RAM_START         (0x20000000)
#define DEV_START         (0x40000000)
#define EXTRAM_START      (0xC0000000)
#define EXTFLASH_START    (0x80000000)
#define EXTDEV_START      (0xA0000000)
#define REG_START         (0xE0000000)

uint32_t mpu_size(uint32_t size)
{
    switch(size) {
        case (1 * 1024):
            return MPUSIZE_1K;
        case (2 * 1024):
            return MPUSIZE_2K;
        case (4 * 1024):
            return MPUSIZE_4K;
        case (8 * 1024):
            return MPUSIZE_8K;
        case (16 * 1024):
            return MPUSIZE_16K;
        case (32 * 1024):
            return MPUSIZE_32K;
        case (64 * 1024):
            return MPUSIZE_64K;
        case (128 * 1024):
            return MPUSIZE_128K;
        case (256 * 1024):
            return MPUSIZE_256K;
        case (512 * 1024):
            return MPUSIZE_512K;
        case (1 * 1024 * 1024):
            return MPUSIZE_1M;
        case (2 * 1024 * 1024):
            return MPUSIZE_2M;
        case (4 * 1024 * 1024):
            return MPUSIZE_4M;
        case (8 * 1024 * 1024):
            return MPUSIZE_8M;
        default:
            return MPUSIZE_ERR;
    }
}


static uint32_t mpu_bits = 0;

int mpu_present(void)
{
    mpu_bits = MPU_TYPE;
    if (mpu_bits != 0)
        return 1;
    return 0;
}

int mpu_enable(void)
{
    if (!mpu_bits)
        return -1;
    MPU_CTRL = MPU_CTRL_ENABLE; //| MPU_CTRL_PRIVDEFENA;
    return 0;
}

int mpu_disable(void)
{
    if (!mpu_bits)
        return -1;
    MPU_CTRL = 0;
    return 0;
}

static void mpu_select(uint32_t region)
{
    MPU_RNR = region;
}

void mpu_setattr(int region, uint32_t attr)
{
    mpu_select(region);
    MPU_RASR = attr;
}

void mpu_setaddr(int region, uint32_t addr)
{
    mpu_select(region);
    MPU_RBAR = addr;
}


void mpu_init(void)
{
    if (!mpu_present())
        return;
    irq_off();

    /* User area: prio 0, from start */
    mpu_setaddr(0, 0);              /* Userspace memory block   0x00000000 (1G) - Internal flash is an exception of this */
    mpu_setattr(0, MPUSIZE_1G | MPU_RASR_ENABLE | MPU_RASR_ATTR_SCB | MPU_RASR_ATTR_AP_PRW_URW);

    mpu_setaddr(1, EXTRAM_START);   /* External RAM bank        0x60000000 (512M) */
    mpu_setattr(1, MPUSIZE_512M   | MPU_RASR_ENABLE | MPU_RASR_ATTR_SCB | MPU_RASR_ATTR_AP_PRW_URW);

    /* Read-only sectors */
    mpu_setaddr(2, FLASH_START);    /* Internal Flash           0x00000000 - 0x0FFFFFFF (256M) */
    mpu_setattr(2, MPUSIZE_256M | MPU_RASR_ENABLE | MPU_RASR_ATTR_SCB | MPU_RASR_ATTR_AP_PRO_URO);

    /* System (No user access) */
    mpu_setaddr(3, RAM_START);      /* Kernel memory            0x20000000 (CONFIG_KRAM_SIZE KB) */
    mpu_setattr(3, mpu_size(CONFIG_KRAM_SIZE << 10) | MPU_RASR_ENABLE | MPU_RASR_ATTR_SCB | MPU_RASR_ATTR_AP_PRW_UNO);

    /* Priority 4 reserved for task stack exception in kernel memory */

    mpu_setaddr(5, DEV_START);      /* Peripherals              0x40000000 (512MB)*/
    mpu_setattr(5, MPUSIZE_1G | MPU_RASR_ENABLE | MPU_RASR_ATTR_S | MPU_RASR_ATTR_B | MPU_RASR_ATTR_AP_PRW_UNO);
    mpu_setaddr(6, EXTDEV_START);   /* External Peripherals     0xA0000000 (1GB)   */
    mpu_setattr(6, MPUSIZE_1G | MPU_RASR_ENABLE | MPU_RASR_ATTR_S | MPU_RASR_ATTR_B | MPU_RASR_ATTR_AP_PRW_UNO);
    mpu_setaddr(7, REG_START);      /* System Level             0xE0000000 (256MB) */
    mpu_setattr(7, MPUSIZE_256M | MPU_RASR_ENABLE | MPU_RASR_ATTR_S | MPU_RASR_ATTR_B | MPU_RASR_ATTR_AP_PRW_UNO);

    SCB_SHCSR |= SCB_SHCSR_MEMFAULTENA;
    mpu_enable();
    irq_on();
}

void mpu_task_on(void *stack)
{
    mpu_disable();
    mpu_setaddr(4, (int)(stack + 20));
    mpu_setattr(4, mpu_size(CONFIG_TASK_STACK_SIZE) | MPU_RASR_ENABLE | MPU_RASR_ATTR_SCB | MPU_RASR_ATTR_AP_PRW_URW);
    mpu_enable();
}
