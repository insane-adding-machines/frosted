#include <stdint.h>

void BSP_Init(void)
{
    // set the system clock to be HSE
    SystemClock_Config();
//    HAL_Init();
//    HAL_SYSTICK_Config(SystemCoreClock / 1000); // HAL_Init sets systick wrong, so do it again
}

void BSP_SysTick_Hook(void)
{
//    HAL_IncTick();
}

void BSP_Reset(void)
{
    // TODO: Not implemented yet
}

