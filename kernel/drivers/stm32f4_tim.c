#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/cm3/systick.h>
#include "frosted.h"

static uint32_t tim2_value = 0;
void cputimer_start(uint32_t val)
{
    uint32_t period = val * (CONFIG_SYS_CLOCK / 2000);

    tim2_value = val;
    /* Enable TIM2 clock. */
    rcc_periph_clock_enable(RCC_TIM2);

    /* Enable TIM2 interrupt. */
    nvic_enable_irq(NVIC_TIM2_IRQ);

    /* Reset TIM2 peripheral. */
    timer_reset(TIM2);

    /* Timer global mode:
     *   * - No divider
     *       * - Alignment edge
     *           * - Direction up
     *               */
    timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT,
            TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

    /* Enable preload. */
    timer_disable_preload(TIM2);

    //timer_set_prescaler(TIM2, 48000);

    timer_set_period(TIM2, period);

    timer_one_shot_mode(TIM2);

    /* Counter enable. */
    timer_enable_counter(TIM2);

    /* Enable commutation interrupt. */
    timer_enable_irq(TIM2, TIM_DIER_UIE);
}

void tim2_isr(void)
{
    volatile uint32_t elapsed_time = 0;
    if (timer_get_flag(TIM2, TIM_SR_UIF)) {

        /* Clear compare interrupt flag. */
        timer_clear_flag(TIM2, TIM_SR_UIF);

        /*
         *       * Get current timer value to calculate next
         *               * compare register value.
         *                       */
        elapsed_time = timer_get_counter(TIM2);
        jiffies += tim2_value;
        systick_interrupt_enable();
    }
}
