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
#include <unicore-mx/stm32/pwr.h>
#include <unicore-mx/stm32/rtc.h>

#define WFE() __asm__ volatile ("wfe")
#if CONFIG_SYS_CLOCK == 48000000
#    define STM32_CLOCK RCC_CLOCK_3V3_48MHZ
#elif CONFIG_SYS_CLOCK == 84000000
#    define STM32_CLOCK RCC_CLOCK_3V3_84MHZ
#elif CONFIG_SYS_CLOCK == 100000000
#    define STM32_CLOCK RCC_CLOCK_3V3_100MHZ
#elif CONFIG_SYS_CLOCK == 120000000
#    define STM32_CLOCK RCC_CLOCK_3V3_120MHZ
#elif CONFIG_SYS_CLOCK == 168000000
#    define STM32_CLOCK RCC_CLOCK_3V3_168MHZ
#else
#   error No valid clock speed selected
#endif


int lowpower_init(void)
{
    rcc_periph_clock_enable(RCC_PWR);
    rcc_periph_clock_enable(RCC_RTC);

    /* Disable write protection in the backup range */
    PWR_CR |= PWR_CR_DBP;

    rtc_unlock();
    nvic_clear_pending_irq(NVIC_RTC_WKUP_IRQ);
    nvic_disable_irq(NVIC_RTC_WKUP_IRQ);
    RTC_ISR &= ~RTC_ISR_WUTF;
    RTC_CR &= ~(RTC_CR_WUTIE | RTC_CR_WUTE);
    rtc_lock();

    return 0;
}


int lowpower_sleep(int stdby, uint32_t interval)
{

    uint32_t rtc_wup;
    uint32_t err;
    if (interval < 1)
        return -1;
    rtc_wup = (interval * 2048) - 1;
    irq_off();
    rcc_periph_clock_enable(RCC_PWR);
    rcc_periph_clock_enable(RCC_RTC);
    nvic_enable_irq(NVIC_RTC_WKUP_IRQ);

    /* Disable write protection in the backup range */
    PWR_CR |= PWR_CR_DBP;

    /* Enable RTC */
    RCC_BDCR |= RCC_BDCR_RTCEN;

    /* Enable LSI */
    rcc_osc_on(RCC_LSI);
    rcc_wait_for_osc_ready(RCC_LSI);

    /* Select LSI as RTC clock source */
    RCC_BDCR &= ~RCC_BDCR_RTCSEL_MASK << RCC_BDCR_RTCSEL_SHIFT;
    RCC_BDCR |= RCC_BDCR_RTCSEL_LSI << RCC_BDCR_RTCSEL_SHIFT;

    /* Enable the EXTI 22 event */
    exti_set_trigger(EXTI22, EXTI_TRIGGER_RISING);
    exti_enable_request(EXTI22);

    /* Set up watchdog timer */
    rtc_unlock();
    RTC_CR &= ~RTC_CR_WUTE;
    while (!(RTC_ISR & RTC_ISR_WUTWF));
    RTC_WUTR = interval - 1;
    RTC_CR |= RTC_CR_OSEL_WAKEUP << RTC_CR_OSEL_SHIFT;
    RTC_CR |= RTC_CR_WUCLKSEL_SPRE;
    RTC_CR |= RTC_CR_WUTE | RTC_CR_WUTIE;
    rtc_lock();
    systick_counter_disable();
    systick_interrupt_disable();
    SCB_SCR |= SCB_SCR_SEVEONPEND;
    SCB_SCR |= SCB_SCR_SLEEPDEEP;
    if (stdby)
        PWR_CR |= PWR_CR_CWUF | PWR_CR_CSBF | PWR_CR_PDDS;
    else
        PWR_CR |= PWR_CR_CWUF | PWR_CR_LPDS | PWR_CR_FPDS;
    irq_on();
    WFE();
    WFE();
    SCB_SCR &= ~SCB_SCR_SLEEPDEEP;
    PWR_CR |= PWR_CR_CWUF;
#ifdef CLOCK_12MHZ
    rcc_clock_setup_hse_3v3(&rcc_hse_12mhz_3v3[STM32_CLOCK]);
#else
    rcc_clock_setup_hse_3v3(&rcc_hse_8mhz_3v3[STM32_CLOCK]);
#endif
    systick_interrupt_enable();
    systick_counter_enable();
    rcc_periph_clock_enable(RCC_PWR);
    rcc_periph_clock_enable(RCC_RTC);
    nvic_enable_irq(NVIC_RTC_WKUP_IRQ);

    /* Disable write protection in the backup range */
    PWR_CR |= PWR_CR_DBP;
    /* Disable RTC */
    RCC_BDCR &= ~RCC_BDCR_RTCEN;
    jiffies += interval * 1000;
    return 0;
}

void rtc_wkup_isr(void)
{

    /* Enable RTC */
    RCC_BDCR |= RCC_BDCR_RTCEN;
    rtc_unlock();
    nvic_clear_pending_irq(NVIC_RTC_WKUP_IRQ);
    nvic_disable_irq(NVIC_RTC_WKUP_IRQ);
    RTC_ISR &= ~RTC_ISR_WUTF;
    RTC_CR &= ~(RTC_CR_WUTIE | RTC_CR_WUTE);
    rtc_lock();
}

