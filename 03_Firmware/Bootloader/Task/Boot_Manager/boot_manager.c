#include "boot_manager.h"
#include "Flash.h"
#include "W25Qxx_handler.h"
#include "elog.h""
#include "key.h"


#include "AES.h"
#include "AT24Cxx_Driver.h"
#include "ymodem.h"


extern key_handler_t  key_handler;
extern key_need_arg_t key_need_arg;

extern st_W25Q_Handler s_st_W25Q_Handler_1;

uint32_t payload_app_size;
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



unsigned char AES128key[16] = "123456789abcdefa";
unsigned char AES_IV[16]    = "0102030405123456";
uint16_t w25qxx_len = 16;
uint8_t w25q64xx_data[4096] = {0};

int8_t back_to_app(int32_t app_size)
{
    if(app_size <= 0) return -1;
    if(app_size > (BACK_FLASH_ADDR - APP_FLASH_ADDR)) return -1;

    uint32_t app_address = APP_FLASH_ADDR;
    uint32_t remaining = (uint32_t)app_size;

    Flash_erase(app_address, app_size);

    while(remaining > 0)
    {
        // 读取 B 区数据
        W25Q64_ReadData(w25q64xx_data, &w25qxx_len);

        // 检查读取长度
        if(w25qxx_len == 0 ||
           w25qxx_len > sizeof(w25q64xx_data))
        {
            return -1;
        }

        // 本次实际需要写入的字节数
        uint32_t write_len =
            (w25qxx_len > remaining) ? remaining : w25qxx_len;

        // 非最后一个数据块必须满足 4 字节对齐
        if(write_len < remaining && (write_len % 4 != 0))
        {
            return -1;
        }

        for(uint32_t j = 0; j < write_len; j += 4)
        {
            uint32_t word = 0xFFFFFFFF;

            uint32_t copy_len =
                (write_len - j >= 4) ? 4 : (write_len - j);

            memcpy(&word, w25q64xx_data + j, copy_len);

            // 写入内部 Flash
            Flash_Write(app_address, word);

            // 校验
            uint32_t verify_word = 0;
            memcpy(&verify_word, (void *)app_address, 4);

            if(verify_word != word)
            {
                return -1;
            }

            app_address += 4;
        }

        // 更新剩余数据量
        remaining -= write_len;
    }

    return 0;
}

int8_t back_to_w25Qxx_back(int32_t app_size)
{ 
    if(app_size <= 0) return -1;
    if(app_size > (BACK_FLASH_ADDR - APP_FLASH_ADDR)) return -1;

    uint32_t app_address  = APP_FLASH_ADDR;
    //uint32_t back_address = BACK_FLASH_ADDR;
    
    uint8_t descryptbuf[16];
    uint8_t current_iv[16]; // 用于存放动态更新的 IV
    uint32_t App_size;
    uint8_t *p_data =w25q64xx_data;

    AES_Init(AES128key);
    memcpy(current_iv, AES_IV, 16); // 初始化 IV
    
    // ---------------- 1. 解密头部 ----------------
   // memcpy(p_data, (uint8_t*)back_address, 16);

    W25Q64_ReadData(p_data, &w25qxx_len);
    AES_Decrypt(p_data, descryptbuf, 16, current_iv);
    memcpy(current_iv, p_data, 16); // 更新 IV 为第一个密文块
    p_data+=16;

    // 解析 App_size (假设小端序)
    App_size = (descryptbuf[15]<< 24) | (descryptbuf[14]<< 16) | (descryptbuf[13]<< 8) | descryptbuf[12];

    // 校验合法性防止跑飞
    if(App_size == 0 || App_size > (BACK_FLASH_ADDR - APP_FLASH_ADDR)) {
        return -2; // 长度不合法，可能解密错误或头部格式不对
    }

    payload_app_size = App_size;
    // ---------------- 3. 循环解密并写入 ----------------
    uint32_t total_blocks = (App_size + 15) / 16; // 向上取整计算块数
    uint32_t remain_size = App_size;

    s_st_W25Q_Handler_1.write_databuf_index = 0;
    s_st_W25Q_Handler_1.write_index = 0;
    s_st_W25Q_Handler_1.write_sector_index = 128;


    for(uint32_t i = 0; i < total_blocks; i++)
    {
        //当前缓冲区数据如果用完了就再次读取
        if(w25qxx_len<= (p_data - w25q64xx_data) )
        {

            p_data =w25q64xx_data;
            W25Q64_ReadData(p_data, &w25qxx_len);
        }
        
        // 使用当前的 IV 解密
        AES_Decrypt(p_data, descryptbuf, 16, current_iv);
        // 更新 IV 为当前密文块，供下一次解密使用
        memcpy(current_iv,p_data, 16);
        p_data+=16; 
        // 计算当前块实际要写入的字节数（处理最后一块不是16的倍数的情况）
        uint32_t write_len = (remain_size >= 16) ? 16 : remain_size;

        W25Q64_WriteData(descryptbuf,write_len);
        remain_size -= write_len;
    }
    
    W25Q64_WriteData_End();
    return 0;
}


int8_t App_to_w25Qxx_back(int32_t app_size)
{ 
    if(app_size <= 0) return -1;
    if(app_size > (BACK_FLASH_ADDR - APP_FLASH_ADDR)) return -1;

    uint32_t app_address  = APP_FLASH_ADDR;
    //uint32_t back_address = BACK_FLASH_ADDR;
    
    uint8_t descryptbuf[16];
    uint8_t current_iv[16]; // 用于存放动态更新的 IV
    uint32_t App_size;
    uint8_t *p_data =w25q64xx_data;

    AES_Init(AES128key);
    memcpy(current_iv, AES_IV, 16); // 初始化 IV
    
    // ---------------- 1. 解密头部 ----------------
   // memcpy(p_data, (uint8_t*)back_address, 16);

    W25Q64_ReadData(p_data, &w25qxx_len);
    AES_Decrypt(p_data, descryptbuf, 16, current_iv);
    memcpy(current_iv, p_data, 16); // 更新 IV 为第一个密文块
    p_data+=16;

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
        //当前缓冲区数据如果用完了就再次读取
        if(w25qxx_len<= (p_data - w25q64xx_data) )
        {

            p_data =w25q64xx_data;
            W25Q64_ReadData(p_data, &w25qxx_len);
        }
        
        // 使用当前的 IV 解密
        AES_Decrypt(p_data, descryptbuf, 16, current_iv);
        // 更新 IV 为当前密文块，供下一次解密使用
        memcpy(current_iv,p_data, 16);
        p_data+=16; 
        // 计算当前块实际要写入的字节数（处理最后一块不是16的倍数的情况）
        uint32_t write_len = (remain_size >= 16) ? 16 : remain_size;

        W25Q64_WriteData(descryptbuf,write_len);
        remain_size -= write_len;
    }
    
    return 0;
}
/**
 * @brief ota状态机处理 
 */

uint8_t temp_app_data[1024] = {0};
void Ota_SateManger(void)
{

    uint8_t ota_status    = NO_APP_NEED_TO_UPDATE;
    uint8_t temp_data[4]  = {0};
    int32_t app_size      = 0;
    int32_t temp_app_size = 0;
    ee_ReadBytes((uint8_t*)&ota_status,0x00,1);

#if 1
    ota_status = DOWNLOAD;
#endif
    switch(ota_status)
    {
      case NO_APP_NEED_TO_UPDATE:
          //检测按键是否按下
          if(KEY_HANDLER_OK == key_handler.key_scan(&key_handler))
          {
                ota_status = DOWNLOAD;
          }

          jump_to_app();
          ota_status = DOWNLOAD;

          break;
      case APP_DOWNLOADING:
         elog_e("ERROR","Download false!!!");
         //合法就跳转
         jump_to_app();
         //不合法就跳转到下载状态
         ota_status = DOWNLOAD;
          break;
      case APP_REQ_UPDATE:
          ee_ReadBytes(temp_data,0x00,4);
          break;
      case DOWNLOAD:
          //进入下载模式 
        elog_i("INFO", "Start Download to back flash.......");            
        //接收数据并写入A区间
        temp_app_size = Ymodem_Receive(temp_app_data);

        //解密并写入B区间
        //配置W25Qxx从A区读取,解密并写入B区间
        

        if( 0!=back_to_w25Qxx_back(temp_app_size))
        {
            elog_e("INFO", "Parse False!!");            
            return;
        }
        
        //判断是不是第一次写入App,不是就备份
        if (((*(__IO uint32_t *)APP_FLASH_ADDR) & 0x2FFE0000) == 0x20000000)
        {
            //开始备份
        }       

        //将B区间代码写入片上Flash中
        
        s_st_W25Q_Handler_1.read_index = 0;
        s_st_W25Q_Handler_1.read_sector_index = 128;

        if( 0 != back_to_app(payload_app_size))
        {
            elog_i("INFO", "B Are to App");            
            return;
        }
        jump_to_app();

          break;
      default:
          break;

    }
}


