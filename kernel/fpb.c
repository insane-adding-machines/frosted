#include "frosted.h"
#include <libopencmsis/core_cm3.h>
#define __ARM_ARCH_7M__ 1
#define __ARM_ARCH_7EM__ 1
#include <cm3/fpb.h>

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





#if 0
void debug_monitor_handler(void)
{
    kprintf("TRAP!\r\n");
    jiffies+=10000000;
//    FPB_COMP[0] = 0;
    /* Exit debug state */
    DBG_DHCSR = DBG_DHCSR_KEY;

}

int fpb_setbrk(void *bpoint)
{
    FPB_COMP[0] = FPB_COMP_ENABLE | (((uint32_t)bpoint) & (0x1FFFFFFC)) | FPB_REPLACE_BOTH;
}
#endif


int fpb_init(void)
{
#if 0
    /* Enable Debug Monitor Exception */
    DBG_DEMCR = DBG_DEMCR_MON_EN;
    FPB_CTRL = FPB_CTRL_ENABLE | FPB_CTRL_KEY | (1 << FPB_NUM_CODE2_OFF) | (2 << FPB_NUM_LIT_MASK_OFF);
    nvic_enable_irq(DEBUG_MONITOR_IRQ);

    fpb_setbrk(fno_unlink);
#endif
}

