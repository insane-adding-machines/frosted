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
 *      Authors: Daniele Lacamera
 *
 */


#include "fpb.h"
#include <libopencmsis/core_cm3.h>
#define FPB_NUM_CODE2_OFF   12
#define FPB_NUM_LIT_MASK_OFF 8
#define FPB_NUM_CODE1_OFF    4


#define DBG_DHCSR MMIO32(0xE000EDF0)
#define DBG_DEMCR MMIO32(0xE000EDFC)

#define DBG_DHCSR_KEY ((0xA0 << 24) | (0x5F << 16))
#define DBG_DHCSR_HALT (1 << 1)
#define DBG_DHCSR_STEP (1 << 2)


#define DBG_DEMCR_MON_STEP (1 << 18)
#define DBG_DEMCR_MON_PEND (1 << 17)
#define DBG_DEMCR_MON_EN (1 << 16)

#define FPB_REPLACE_LO (1 << 30)
#define FPB_REPLACE_HI (2 << 30)
#define FPB_REPLACE_BOTH (3 << 30)


struct bkpt {
    int pid;
    void *b;
};

struct bkpt bkpt[8];

void debug_monitor_handler(void)
{
    int pid;
    kprintf("TRAP!\r\n");
    /* Exit debug state */
    task_hit_breakpoint(this_task());
    DBG_DHCSR = DBG_DHCSR_KEY;
}

int fpb_setbrk(int pid, void *bpoint, int n)
{
    int i;
    if (n < 0) {
        for (i = 0; i < 8; i++) {
            if (bkpt[i].pid == 0) {
                n = i;
                break;
            }
        }
    }
    if (n < 0)
        return -1;
    bkpt[n].pid = pid;
    bkpt[n].b = bpoint;
    if ((uint32_t)bpoint & 0x01)
        return -1;
    if ((uint32_t)bpoint & 0x02) 
        FPB_COMP[n] = FPB_COMP_ENABLE | (((uint32_t)bpoint) & (0x1FFFFFFC)) | FPB_REPLACE_HI; 
    else
        FPB_COMP[n] = FPB_COMP_ENABLE | (((uint32_t)bpoint) & (0x1FFFFFFC)) | FPB_REPLACE_LO; 
    return n;
}

int fpb_delbrk(int n)
{
    bkpt[n].pid = 0;
    FPB_COMP[n] = 0;
}


int fpb_init(void)
{
    /* Enable Debug Monitor Exception */
    DBG_DEMCR = DBG_DEMCR_MON_EN;
    FPB_CTRL = FPB_CTRL_ENABLE | FPB_CTRL_KEY | (1 << FPB_NUM_CODE2_OFF) | (2 << FPB_NUM_LIT_MASK_OFF);
    nvic_enable_irq(DEBUG_MONITOR_IRQ);
}

