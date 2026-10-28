#include "boot_manager.h"
#include "Flash.h"


#include "AES.h"
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



/**
 * @brief 备份区代码保存到运行区
 */

unsigned char AES128key[16] = "123456789abcdefa";
unsigned char AES_IV[16]    = "0102030405123456";

int8_t back_to_app(int32_t app_size)
{ 
    if(app_size <= 0) return -1;
    if(app_size > (BACK_FLASH_ADDR - APP_FLASH_ADDR)) return -1;

    uint32_t app_address  = APP_FLASH_ADDR;
    uint32_t back_address = BACK_FLASH_ADDR;
    
    uint8_t tmep[16];
    uint8_t descryptbuf[16];
    uint8_t current_iv[16]; // 用于存放动态更新的 IV
    uint32_t App_size;

    AES_Init(AES128key);
    memcpy(current_iv, AES_IV, 16); // 初始化 IV
    
    // ---------------- 1. 解密头部 ----------------
    memcpy(tmep, (uint8_t*)back_address, 16);
    back_address += 16; 
    AES_Decrypt(tmep, descryptbuf, 16, current_iv);
    memcpy(current_iv, tmep, 16); // 更新 IV 为第一个密文块

    // 解析 App_size (假设小端序)
    App_size = (descryptbuf[15]<< 24) | (descryptbuf[14]<< 16) | (descryptbuf[13]<< 8) | descryptbuf[12];

    // 校验合法性防止跑飞
    if(App_size == 0 || App_size > (BACK_FLASH_ADDR - APP_FLASH_ADDR)) {
        return -2; // 长度不合法，可能解密错误或头部格式不对
    }

    // ---------------- 2. 擦除 Flash ----------------
    // 注意：请确保 Flash_erase 内部处理了页对齐向上取整的问题
    Flash_erase(app_address, App_size);   

    // ---------------- 3. 循环解密并写入 ----------------
    uint32_t total_blocks = (App_size + 15) / 16; // 向上取整计算块数
    uint32_t remain_size = App_size;

    for(uint32_t i = 0; i < total_blocks; i++)
    {
        memcpy(tmep, (uint8_t*)back_address, 16);
        back_address += 16; 
        
        // 使用当前的 IV 解密
        AES_Decrypt(tmep, descryptbuf, 16, current_iv);
        
        // 更新 IV 为当前密文块，供下一次解密使用
        memcpy(current_iv, tmep, 16);

        // 计算当前块实际要写入的字节数（处理最后一块不是16的倍数的情况）
        uint32_t write_len = (remain_size >= 16) ? 16 : remain_size;

        // 按 4 字节写入（注意处理非4字节对齐的情况）
        for(uint32_t j = 0; j < write_len; j += 4)
        {
            uint32_t word = 0;
            // 如果剩余不足4字节，需要特殊处理，避免读越界
            uint32_t copy_len = (write_len - j >= 4) ? 4 : (write_len - j);
            memcpy(&word, descryptbuf + j, copy_len);

            // 安全写入
            Flash_Write(app_address, word);  

            // 安全校验（使用 memcpy 避免非对齐访问异常）
            uint32_t verify_word = 0;
            memcpy(&verify_word, (void*)app_address, 4);
            if(verify_word != word)
            {
                return -1;
            }
            app_address += 4;
        }
        remain_size -= write_len;
    }
    
    return 0;
}

