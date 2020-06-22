#include "system_jump.h"
#include "main.h"

void SystemAppJump(uint32_t APP_ADDRESS)
{
    typedef void (*pFunction)(void);
    pFunction JumpToApplication;

    __HAL_RCC_USART1_FORCE_RESET();
    __HAL_RCC_USART1_RELEASE_RESET();

    __HAL_RCC_USART2_FORCE_RESET();
    __HAL_RCC_USART2_RELEASE_RESET();

    HAL_RCC_DeInit();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    /**
     * Step: Disable all interrupts
     */
    __disable_irq();

    /* ARM Cortex-M Programming Guide to Memory Barrier Instructions.*/
    __DSB();

    __HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH();

    /* Remap is bot visible at once. Execute some unrelated command! */
    __DSB();
    __ISB();

    JumpToApplication = (void (*)(void)) (*((uint32_t *) ((APP_ADDRESS + 4))));

    /* Initialize user application's Stack Pointer */
    __set_MSP(*(__IO uint32_t*) APP_ADDRESS);

    JumpToApplication();
}