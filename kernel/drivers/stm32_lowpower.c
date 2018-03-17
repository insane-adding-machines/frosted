#include "frosted.h"
#include "unicore-mx/cm3/nvic.h"
#include "unicore-mx/cm3/systick.h"
#include "unicore-mx/cm3/scb.h"
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/gpio.h>
#include <unicore-mx/stm32/timer.h>
#include <unicore-mx/cm3/nvic.h>
#include <unicore-mx/stm32/exti.h>
#include <unicore-mx/cm3/systick.h>

static volatile int sleeping = 0;
static uint32_t psc;
static uint32_t val;
static uint32_t system_clock;
static uint32_t sleeping_interval = 0;
#define SEV() asm volatile ("sev")

int lowpower_sleep(uint32_t interval)
{

    uint32_t clock;
    uint32_t err;
    if (interval < 1000)
        return -1;
    if (interval > 10000)
        interval = 10000;
    psc = 1;
    clock = interval * (CONFIG_SYS_CLOCK / 2000);
    if (sleeping)
        return 0;
    while (psc < 65535) {
        val = clock / psc;
        err = clock % psc;
        if ((val < 65535) && (err == 0)) {
            val--;
            break;
        }
        val = 0;
        psc++;
    }
    if (val == 0)
        return -1;

    system_clock = CONFIG_SYS_CLOCK;
    sleeping_interval = interval;

    irq_off();
    sleeping = 1;
    /* Enable TIM2 clock. */
    rcc_periph_clock_enable(RCC_TIM2);

    /* Enable TIM2 interrupt. */
    nvic_enable_irq(NVIC_TIM2_IRQ);
    timer_disable_counter(TIM2);
    timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT,
            TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

    timer_set_prescaler(TIM2, psc);
    timer_set_period(TIM2, val);
    timer_set_counter(TIM2,1);
    //timer_one_shot_mode(TIM2);

    timer_enable_counter(TIM2);
    timer_enable_irq(TIM2, TIM_DIER_UIE);
    systick_counter_disable();
    systick_interrupt_disable();
    SCB_SCR |= SCB_SCR_SEVEONPEND;
    irq_on();
    return 0;

}

static void do_lowpower_resume(int from_timer)
{
    uint32_t elapsed = timer_get_counter(TIM2);
    uint32_t elapsed_ms = (psc * elapsed) / (system_clock / 2000);

    if (!sleeping) {
        return;
    }
    sleeping = 0;
    nvic_disable_irq(NVIC_TIM2_IRQ);
    rcc_periph_clock_disable(RCC_TIM2);
    timer_disable_irq(TIM2, TIM_DIER_UIE);
    timer_disable_counter(TIM2);
    if (from_timer)
        jiffies += sleeping_interval;
    else
        jiffies += elapsed_ms;
    SEV();
    systick_interrupt_enable();
    systick_counter_enable();
    SCB_SCR &= ~SCB_SCR_SEVEONPEND;
}

void lowpower_resume(void)
{
    return do_lowpower_resume(0);
}

void tim2_isr(void)
{
    if (timer_get_flag(TIM2, TIM_SR_UIF))
        timer_clear_flag(TIM2, TIM_SR_UIF);
    if (sleeping)
        do_lowpower_resume(1);
}
