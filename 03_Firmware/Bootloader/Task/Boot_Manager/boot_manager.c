#include "boot_manager.h"


/*! \brief 跳转
 *
 *  Detailed description of the function
 *
 * \param Parameter Parameter description
 * \return Return parameter description
 */
void jump_to_app(void)
{
    uint32_t JumpAddress;
    pFunction Jump_To_Application;

    if (((*(__IO uint32_t *)APP_FLASH_ADDR) & 0x2FFE0000) == 0x20000000)
    {
        /* 1. 关全局中断 */
        __disable_irq();

        /* 2. 停 SysTick（Cortex-M 内核，RCC_DeInit 管不到） */
        SysTick->CTRL = 0;
        SysTick->LOAD = 0;
        SysTick->VAL  = 0;

        /* 3. 清 NVIC 所有使能 + pending，防止遗留中断 */
        for (uint8_t i = 0; i < 8; i++)
        {
            NVIC->ICER[i] = 0xFFFFFFFF;
            NVIC->ICPR[i] = 0xFFFFFFFF;
        }

        /* 4. 关掉 Bootloader 打开的外设时钟（至少 TIM3，最好全关） */
        RCC->APB1ENR = 0;   // TIM3、USART2 等都在 APB1
        RCC->APB2ENR = 0;
        RCC->AHB1ENR = 0;
        RCC->AHB2ENR = 0;
        RCC->AHB3ENR = 0;

        /* 5. 复位 RCC 时钟树（切回 HSI，关 PLL/HSE） */
        RCC_DeInit();

        /* 6. 向量表指向 APP（不要再依赖 APP 侧继承） */
//        SCB->VTOR = APP_FLASH_ADDR;

        /* 7. 设置 MSP + 跳转 */
        __set_MSP(*(__IO uint32_t *) APP_FLASH_ADDR);
        JumpAddress = *(__IO uint32_t *) (APP_FLASH_ADDR + 4);
        Jump_To_Application = (pFunction) JumpAddress;
        Jump_To_Application();
    }
}
